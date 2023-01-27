
/******************************************************************************
* MODULE     : vau_main.cpp
* DESCRIPTION: Entrypoint to Vau
* COPYRIGHT  : (C) 2023  Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#include "scheme.hpp"
#include "vau_stuff.hpp"
#include "vau_buffer.hpp"
#include "vau_editor.hpp"
#include "file.hpp"
#include "merge_sort.hpp"
#include "drd_std.hpp"
#include "convert.hpp"
#include "boot.hpp"
#include "data_cache.hpp"


extern void setup_tex (); // from Plugins/Metafont/tex_init.cpp
extern void init_tex  (); // from Plugins/Metafont/tex_init.cpp

/******************************************************************************
* Subroutines for paths
******************************************************************************/

static url
get_env_path (string which) {
  return url ("$" * which);
}

static void
set_env_path (string which, url val) {
  //cout << which << " := " << val << "\n";
  if ((!is_none (val)) && (val->t != ""))
    set_env (which, as_string (val));
}

static url
get_env_path (string which, url def) {
  url val= get_env_path (which);
  if (is_none (val) || (val->t == "")) {
    set_env_path (which, def);
    return def;
  }
  return val;
}

static void
init_user_dirs () {
  make_dir ("$TEXMACS_HOME_PATH");
  make_dir ("$TEXMACS_HOME_PATH/bin");
  make_dir ("$TEXMACS_HOME_PATH/doc");
  make_dir ("$TEXMACS_HOME_PATH/doc/about");
  make_dir ("$TEXMACS_HOME_PATH/doc/about/changes");
  make_dir ("$TEXMACS_HOME_PATH/fonts");
  make_dir ("$TEXMACS_HOME_PATH/fonts/enc");
  make_dir ("$TEXMACS_HOME_PATH/fonts/error");
  make_dir ("$TEXMACS_HOME_PATH/fonts/pk");
  make_dir ("$TEXMACS_HOME_PATH/fonts/tfm");
  make_dir ("$TEXMACS_HOME_PATH/fonts/truetype");
  make_dir ("$TEXMACS_HOME_PATH/fonts/type1");
  make_dir ("$TEXMACS_HOME_PATH/fonts/unpacked");
  make_dir ("$TEXMACS_HOME_PATH/fonts/virtual");
  make_dir ("$TEXMACS_HOME_PATH/langs");
  make_dir ("$TEXMACS_HOME_PATH/langs/mathematical");
  make_dir ("$TEXMACS_HOME_PATH/langs/mathematical/syntax");
  make_dir ("$TEXMACS_HOME_PATH/langs/natural");
  make_dir ("$TEXMACS_HOME_PATH/langs/natural/dic");
  make_dir ("$TEXMACS_HOME_PATH/langs/natural/hyphen");
  make_dir ("$TEXMACS_HOME_PATH/langs/programming");
  make_dir ("$TEXMACS_HOME_PATH/misc");
  make_dir ("$TEXMACS_HOME_PATH/misc/patterns");
  make_dir ("$TEXMACS_HOME_PATH/misc/pixmaps");
  make_dir ("$TEXMACS_HOME_PATH/misc/themes");
  make_dir ("$TEXMACS_HOME_PATH/packages");
  make_dir ("$TEXMACS_HOME_PATH/plugins");
  make_dir ("$TEXMACS_HOME_PATH/progs");
  make_dir ("$TEXMACS_HOME_PATH/server");
  make_dir ("$TEXMACS_HOME_PATH/styles");
  make_dir ("$TEXMACS_HOME_PATH/system");
  make_dir ("$TEXMACS_HOME_PATH/system/bib");
  make_dir ("$TEXMACS_HOME_PATH/system/cache");
  make_dir ("$TEXMACS_HOME_PATH/system/database");
  make_dir ("$TEXMACS_HOME_PATH/system/database/bib");
  make_dir ("$TEXMACS_HOME_PATH/system/make");
  make_dir ("$TEXMACS_HOME_PATH/system/tmp");
  make_dir ("$TEXMACS_HOME_PATH/texts");
  make_dir ("$TEXMACS_HOME_PATH/users");
  change_mode ("$TEXMACS_HOME_PATH/server", 7 << 6);
  change_mode ("$TEXMACS_HOME_PATH/system", 7 << 6);
  change_mode ("$TEXMACS_HOME_PATH/users", 7 << 6);
  //clean_temp_dirs ();
}

static url
plugin_path (string which) {
  url base= "$TEXMACS_HOME_PATH:/etc/TeXmacs:$TEXMACS_PATH:/usr/share/TeXmacs";
  url search= base * "plugins" * url_wildcard ("*") * which;
  return expand (complete (search, "r"));
}

scheme_tree
plugin_list () {
  bool flag;
  array<string> a= read_directory ("$TEXMACS_PATH/plugins", flag);
  a << read_directory ("/etc/TeXmacs/plugins", flag);
  a << read_directory ("$TEXMACS_HOME_PATH/plugins", flag);
  a << read_directory ("/usr/share/TeXmacs/plugins", flag);
  merge_sort (a);
  int i, n= N(a);
  tree t (TUPLE);
  for (i=0; i<n; i++)
    if ((a[i] != ".") && (a[i] != "..") && ((i==0) || (a[i] != a[i-1])))
      t << a[i];
  return t;
}


/******************************************************************************
* Set additional environment variables
******************************************************************************/

static void
init_env_vars () {
  // Handle binary, library and guile paths for plugins
  url bin_path= get_env_path ("PATH"); // | plugin_path ("bin");
#if defined (OS_MINGW) || defined (OS_MACOS)
  bin_path= bin_path | url ("$TEXMACS_PATH/bin");
//  if (has_user_preference ("manual path"))
//    bin_path= url_system (get_user_preference ("manual path")) | bin_path;
#endif

  set_env_path ("PATH", bin_path);
  url lib_path= get_env_path ("LD_LIBRARY_PATH") | plugin_path ("lib");
  set_env_path ("LD_LIBRARY_PATH", lib_path);

  // Get TeXmacs style and package paths
  url style_root=
    get_env_path ("TEXMACS_STYLE_ROOT",
                  "$TEXMACS_HOME_PATH/styles:$TEXMACS_PATH/styles" | plugin_path ("styles"));
  url package_root=
    get_env_path ("TEXMACS_PACKAGE_ROOT",
                  "$TEXMACS_HOME_PATH/packages:$TEXMACS_PATH/packages" |
                  plugin_path ("packages"));
  url all_root= style_root | package_root;
  url style_path=
    get_env_path ("TEXMACS_STYLE_PATH",
                  search_sub_dirs (all_root));
  url text_root=
    get_env_path ("TEXMACS_TEXT_ROOT",
                  "$TEXMACS_HOME_PATH/texts:$TEXMACS_PATH/texts" |
                  plugin_path ("texts"));
  url text_path=
    get_env_path ("TEXMACS_TEXT_PATH",
                  search_sub_dirs (text_root));

  // Get other data paths
  (void) get_env_path ("TEXMACS_FILE_PATH",text_path | style_path);
  (void) set_env_path ("TEXMACS_DOC_PATH",
                       get_env_path ("TEXMACS_DOC_PATH") |
                       "$TEXMACS_HOME_PATH/doc:$TEXMACS_PATH/doc" |
                       plugin_path ("doc"));
  (void) set_env_path ("TEXMACS_SECURE_PATH",
                       get_env_path ("TEXMACS_SECURE_PATH") |
                       "$TEXMACS_PATH:$TEXMACS_HOME_PATH");
  (void) get_env_path ("TEXMACS_PATTERN_PATH",
                       "$TEXMACS_HOME_PATH/misc/patterns" |
                       url ("$TEXMACS_PATH/misc/patterns") |
                       url ("$TEXMACS_PATH/misc/pictures") |
                       plugin_path ("misc/patterns"));
  (void) get_env_path ("TEXMACS_PIXMAP_PATH",
                       "$TEXMACS_HOME_PATH/misc/pixmaps" |
                       url ("$TEXMACS_PATH/misc/pixmaps/modern/32x32/settings") |
                       url ("$TEXMACS_PATH/misc/pixmaps/modern/32x32/table") |
                       url ("$TEXMACS_PATH/misc/pixmaps/modern/24x24/main") |
                       url ("$TEXMACS_PATH/misc/pixmaps/modern/20x20/mode") |
                       url ("$TEXMACS_PATH/misc/pixmaps/modern/16x16/focus") |
                       url ("$TEXMACS_PATH/misc/pixmaps/traditional/--x17") |
                       plugin_path ("misc/pixmaps"));
  (void) get_env_path ("TEXMACS_DIC_PATH",
                       "$TEXMACS_HOME_PATH/langs/natural/dic" |
                       url ("$TEXMACS_PATH/langs/natural/dic") |
                       plugin_path ("langs/natural/dic"));
  (void) get_env_path ("TEXMACS_THEME_PATH",
                       url ("$TEXMACS_PATH/misc/themes") |
                       url ("$TEXMACS_HOME_PATH/misc/themes") |
                       plugin_path ("misc/themes"));
#ifdef OS_WIN32
  set_env ("TEXMACS_SOURCE_PATH", "");
#else
  set_env ("TEXMACS_SOURCE_PATH", TEXMACS_SOURCES);
#endif
}

editor cur_ed;

editor current_editor () {
  return cur_ed;
}

void
TeXmacs_main (int argc, char** argv) {
  the_et     = tuple ();
  the_et->obs= ip_observer (path ());

  debug_set ("std", true);
  debug_set ("io", true);
  debug_set ("bench", true);
  debug_set ("verbose", true);

  cache_initialize ();
  
  bench_start ("initialize vau");
  //cout << "Initialize -- Succession status table\n";
  init_succession_status_table ();
  //cout << "Initialize -- Succession standard DRD\n";
  init_std_drd ();
  //cout << "Initialize -- User preferences\n";
  load_user_preferences ();
  //cout << "Initialize -- Environment variables\n";
  init_env_vars ();
  bench_cumul ("initialize vau");

  initialize_scheme ();

  bench_start ("initialize scheme");
  string tm_init_file= "$TEXMACS_PATH/progs/init-vau-s7.scm";
  if (exists (tm_init_file)) exec_file (tm_init_file);
  bench_cumul ("initialize scheme");

  //  setup_tex ();
  init_tex (); // for paths

  
  string name ("$TEXMACS_PATH/vau-tests/grassmann-sq-example.tm");
//  string name ("$TEXMACS_PATH/examples/texts/bracket-test.tm");
  string output ("$HOME/vau-test.pdf");
  string image_output  ("$HOME/vau-test.png");

  vau_buffer buf= concrete_buffer_insist (name);

  editor ed (buf);
  cur_ed= ed;
  
  //ed->init_style("generic");
  ed->set_data (buf->data);

  ed->print_to_file (output);
//  ed->get_page_image (image_output, 1, "300");
  
  cur_ed= editor ();
  
  cache_memorize ();
  bench_print ();
}


bool use_pdf () { return true; }
bool use_ps () { return true; }

int
main(int argc, char **argv) {
  cout << "Starting Vau" << LF;
#ifndef __EMSCRIPTEN__
  set_env ("TEXMACS_PATH", TEXMACS_SOURCES "/resources"); //FIXME: this has to point to the installation dir!
  set_env ("TEXMACS_HOME_PATH", "$HOME/.Vau");
#else
  set_env ("TEXMACS_PATH", "/Vau"); //FIXME: this has to point to the installation dir!
  set_env ("TEXMACS_HOME_PATH", "/Home_Vau");
  set_env ("HOME", "/");
#endif
  set_env_path ("GUILE_LOAD_PATH", "$TEXMACS_PATH/progs:$GUILE_LOAD_PATH");

  init_user_dirs ();
  
  start_scheme (argc, argv, TeXmacs_main);
  return 0;
}
