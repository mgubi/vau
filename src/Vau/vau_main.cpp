//
//  vau_main.cpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 27/12/2022.
//

#include "scheme.hpp"
#include "vau_stuff.hpp"
#include "vau_buffer.hpp"
#include "vau_editor.hpp"
#include "file.hpp"

static void
set_env_path (string which, url val) {
  cout << which << " := " << val << "\n";
  if ((!is_none (val)) && (val->t != ""))
    set_env (which, as_string (val));
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


void
TeXmacs_main (int argc, char** argv) {
  
  initialize_scheme ();
  
  string tm_init_file= "/Users/mgubi/t/vau/src/Vau/init-vau.scm";
  bench_start ("initialize scheme");
  if (exists (tm_init_file)) exec_file (tm_init_file);
  bench_cumul ("initialize scheme");

  string name ("/Users/mgubi/t/svn-src/TeXmacs/examples/texts/accent-test.tm");
  string output ("/Users/mgubi/vau-test.ps");
  tm_buffer buf= concrete_buffer_insist (name);
  editor ed (buf);
  ed->print_to_file (output);
}

int
main(int argc, char **argv) {
  set_env ("TEXMACS_PATH", "/Users/mgubi/t/vau/resources");
  set_env ("TEXMACS_HOME_PATH", "/Users/mgubi/.Vau");
  set_env_path ("GUILE_LOAD_PATH", "$TEXMACS_PATH/progs:$GUILE_LOAD_PATH");

  init_user_dirs ();
  
  the_et     = tuple ();
  the_et->obs= ip_observer (path ());
  //cache_initialize ();
  //bench_start ("initialize texmacs");
  //init_texmacs ();
  //bench_cumul ("initialize texmacs");
  start_scheme (argc, argv, TeXmacs_main);
  return 0;
}
