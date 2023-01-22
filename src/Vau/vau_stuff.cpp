
/******************************************************************************
* MODULE     : vau_suff.cpp
* DESCRIPTION: Miscellanea of functions to run Vau
* COPYRIGHT  : (C) 2023  Joris van der Hoeven and Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#include "vau_stuff.hpp"

#include "Kernel/Types/modification.hpp"
#include "tree.hpp"
#include "tree_select.hpp"
#include "analyze.hpp"
#include "picture.hpp"
#include "renderer.hpp"
#include "font.hpp"
#include "widget.hpp"
#include "file.hpp"
#include "vau_editor.hpp"

#include <unistd.h>
#include <sys/stat.h>

/******************************************************************************
* Stuff to implement
******************************************************************************/


void
edit_announce (editor_rep* ed, modification mod) {
  //FIXME: implement
}

void
edit_done (editor_rep* ed, modification mod) {
  //FIXME: implement
}

void
edit_touch (editor_rep* ed, path p) {
  //FIXME: implement
}

extern editor current_editor ();

tree
get_subtree (path p) {
  //FIXME: this function is called by the bridge and need access to the current editor. There should be a better way to do it.
  return current_editor () -> the_subtree (p);
}

bool
gui_interrupted (bool check) {
  return false;
}

char Cork_unaccented[128]= {
  'A', 'A', 'C', 'C', 'D', 'E', 'E', 'G',
  'L', 'L', ' ', 'N', 'N', ' ', 'O', 'R',
  'R', 'S', 'S', 'S', 'T', 'T', 'U', 'U',
  'Y', 'Z', 'Z', 'Z', ' ', 'I', 'd', ' ',
  'a', 'a', 'c', 'c', 'd', 'e', 'e', 'g',
  'l', 'l', ' ', 'n', 'n', ' ', 'o', 'r',
  'r', 's', 's', 's', 't', 't', 'u', 'u',
  'y', 'z', 'z', 'z', ' ', ' ', ' ', ' ',
  'A', 'A', 'A', 'A', 'A', 'A', ' ', 'C',
  'E', 'E', 'E', 'E', 'I', 'I', 'I', 'I',
  'D', 'N', 'O', 'O', 'O', 'O', 'O', ' ',
  ' ', 'U', 'U', 'U', 'U', 'Y', ' ', ' ',
  'a', 'a', 'a', 'a', 'a', 'a', ' ', 'c',
  'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
  'd', 'n', 'o', 'o', 'o', 'o', 'o', ' ',
  ' ', 'u', 'u', 'u', 'u', 'y', ' ', ' '
};

bool
is_verbatim (tree t) {
  return is_compound (t, "cpp-code") || is_compound (t, "mmx-code")   ||
         is_compound (t, "scm-code") || is_compound (t, "shell-code") ||
         is_compound (t, "code")     || is_compound (t, "verbatim")   ||
         is_compound (t, "scilab-code") || is_compound (t, "scala-code") ||
         is_compound (t, "java-code") || is_compound (t, "latex_preview") ||
         is_compound (t, "picture-mixed");
}

picture
native_picture (int w, int h, int ox, int oy) {
  (void) w; (void) h; (void) ox; (void) oy;
  FAILED ("not yet implemented");
  return picture ();
}

renderer
picture_renderer (picture p, double zoomf) {
  (void) p; (void) zoomf;
  FAILED ("not yet implemented");
  return NULL;
}

picture
load_picture (url u, int w, int h, tree eff, int pixel) {
  (void) u; (void) w; (void) h; (void) eff; (void) pixel;
  FAILED ("not yet implemented");
  return picture ();
}

picture
as_native_picture (picture pict) {
  FAILED ("not yet implemented");
  return pict;
}

void
save_picture (url dest, picture p) {
  (void) dest; (void) p;
  FAILED ("not yet implemented");
}


/******************************************************************************
* System functions
******************************************************************************/



int
unix_system (string s) {
  c_string _s (s * " > /dev/null 2>&1");
  int ret= system (_s);
  return ret;
}

int
unix_system (string cmd, string& result) {
  url temp= url_temp ();
  string temp_s= escape_sh (concretize (temp));
  c_string _cmd (cmd * " > " * temp_s * " 2>&1");
  int ret= system (_cmd);
  bool flag= load_string (temp, result, false);
  remove (temp);
  if (flag) result= "";
  return ret;
}

int
unix_system (string cmd, string& result, string& error) {
  url temps= url_temp ();
  url tempe= url_temp ();
  string temp_s= escape_sh (concretize (temps));
  string temp_e= escape_sh (concretize (tempe));
  c_string _cmd (cmd * " > " * temp_s * " 2> " * temp_e);
  int ret= system (_cmd);
  bool flag= load_string (temps, result, false);
  remove (temps);
  if (flag) result= "";
  flag= load_string (tempe, error, false);
  remove (tempe);
  if (flag) error= "";
  return ret;
}

int
system (string s, string& result, string& error) {
#if defined (OS_MINGW)
  int r= qt_system (s, result, error);
#else
  int r= unix_system (s, result, error);
#endif
  return r;
}

int
system (string s, string& result) {
#if defined (OS_MINGW)
  int r= qt_system (s, result);
#else
  int r= unix_system (s, result);
#endif
  return r;
}

int
system (string s) {
  if (DEBUG_STD) debug_shell << s << "\n";
  if (DEBUG_VERBOSE) {
    string result;
    int r= system (s, result);
    debug_shell << result;
    return r;
  }
  else {
#if defined (OS_MINGW)
    // if (starts (s, "convert ")) return 1;
    return qt_system (s);
#else
    return unix_system (s);
#endif
  }
}

string
eval_system (string s) {
  string result;
  (void) system (s, result);
  return result;
}

string
var_eval_system (string s) {
  string r= eval_system (s);
  while ((N(r)>0) && (r[N(r)-1]=='\n' || r[N(r)-1]=='\r')) r= r (0,N(r)-1);
  return r;
}
picture
qt_load_xpm (url file_name) {
  return picture ();
}


string
get_env (string var) {
  c_string _var (var);
  const char* _ret= getenv (_var);
  if (_ret==NULL) {
    if (var == "PWD") return get_env ("HOME");
    return "";
  }
  string ret (_ret);
  return ret;
  // do not delete _ret !
}

void
set_env (string var, string with) {
#if defined(STD_SETENV) && !defined(OS_MINGW)
  c_string _var  (var);
  c_string _with (with);
  setenv (_var, _with, 1);
#else
  char* _varw= as_charp (var * "=" * with);
  (void) putenv (_varw);
  // do not delete _varw !!!
  // -> known memory leak, but solution more complex than it is worth
#endif
}

string
get_crash_report (const char* msg) {
  return "";
}

void qt_clean_picture_cache () {
  
}

/******************************************************************************
* Directory for temporary files
******************************************************************************/

static string main_tmp_dir= "$TEXMACS_HOME_PATH/system/tmp";

void
make_dir (url which) {
  if (is_none(which))
    return ;
  if (!is_directory (which)) {
    make_dir (head (which));
    mkdir (which);
  }
}

static url
url_temp_dir_sub () {
#ifdef OS_MINGW
  static url tmp_dir=
    url_system (main_tmp_dir) * url_system (as_string (time (NULL)));
#else
  static url tmp_dir=
    url_system (main_tmp_dir) * url_system (as_string ((int) getpid ()));
#endif
  return (tmp_dir);
}

url
url_temp_dir () {
  static url u;
  if (u == url_none()) {
    u= url_temp_dir_sub ();
    make_dir (u);
  }
  return u;
}

/******************************************************************************/

url
get_texmacs_path () {
  string tmpath= get_env ("TEXMACS_PATH");
    //FIXME: Why is this?
  while ((N(tmpath)>0) && (tmpath [N(tmpath) - 1] == '/'))
    tmpath= tmpath (0, N(tmpath)-1);
  return url_system (tmpath);
}

url
get_texmacs_home_path () {
  url path= get_env ("TEXMACS_HOME_PATH");
  if (path == "")
    path= url_system ("$HOME/.Vau");
  return path;
}

/******************************************************************************
* Subroutines for the TeXmacs settings
******************************************************************************/
static tree texmacs_settings = tuple ();

string
get_setting (string var, string def) {
  int i, n= N (texmacs_settings);
  for (i=0; i<n; i++)
    if (is_tuple (texmacs_settings[i], var, 1)) {
      return scm_unquote (as_string (texmacs_settings[i][1]));
    }
  return def;
}

void
set_setting (string var, string val) {
  int i, n= N (texmacs_settings);
  for (i=0; i<n; i++)
    if (is_tuple (texmacs_settings[i], var, 1)) {
      texmacs_settings[i][1]= scm_quote (val);
      return;
    }
  texmacs_settings << tuple (var, scm_quote (val));
}

font qt_font (string family, int size, int dpi)
{
  return font ();
}

font get_default_font (bool tt, bool mini, bool bold)
{
  return font ();
}

bool in_presentation_mode () {
  return false;
}

url
get_current_buffer_safe () {
  return url ();
}

void
image_size (url image, int& w, int& h) {
  w= 0; h= 0;
}

void
native_image_size (url image, int& w, int& h) {
  w= 0; h= 0;
}

bool has_image_magick() {
  return false;
}

string imagemagick_cmd() {
  return "";
}

range_set current_alt_selection (string name) {
  return range_set ();
}

void
apply_effect (tree eff, array<url> src, url dest, int w, int h) {
  
}

void clear_imgbox_cache(tree t) {
  
}

widget glue_widget (bool hx, bool vx, SI w, SI h)
{
  return widget ();
}

bool use_which        = false;
bool use_locate       = false;
bool texmacs_started = true;
//hashmap<string,tree> style_tree_cache;
int script_status = 1;  // from sys_util.c

url make_file (int cmd, tree data, array<url> args) {
  //FIXME: stub
  return url();
}



