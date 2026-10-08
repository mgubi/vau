
/******************************************************************************
* MODULE     : vau_lib.cpp
* DESCRIPTION: The Vau library
* COPYRIGHT  : (C) 2023  Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#include "vau_lib.hpp"

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

editor set_current_editor (editor ed) {
  editor old_ed= cur_ed;
  cur_ed= ed;
  return old_ed;
}

editor new_editor (vau_buffer buf) {
  editor ed (buf);
  //ed->init_style("generic");
  ed->set_data (buf->data);
  return ed;
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
  extern bool texmacs_started;
  texmacs_started= true;

  //  setup_tex ();
  init_tex (); // for paths

#ifndef __EMSCRIPTEN__
  extern void test_vau();
  test_vau();
#endif

  cache_memorize ();
  bench_print ();
}


bool use_pdf () { return true; }
bool use_ps () { return true; }

void
init_vau_lib (int argc, char **argv) {
  cout << "Starting Vau" << LF;
#ifdef __EMSCRIPTEN__
  set_env ("TEXMACS_PATH", "/Vau"); //FIXME: this has to point to the installation dir!
  set_env ("TEXMACS_HOME_PATH", "/Vau_Home");
  set_env ("HOME", "/");
#else
  set_env ("TEXMACS_PATH", TEXMACS_SOURCES "/resources"); //FIXME: this has to point to the installation dir!
  set_env ("TEXMACS_HOME_PATH", "$HOME/.Vau");
#endif
  set_env_path ("GUILE_LOAD_PATH", "$TEXMACS_PATH/progs:$GUILE_LOAD_PATH");

  init_user_dirs ();
  
  start_scheme (argc, argv, TeXmacs_main);
//  return 0;
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "MuPDF/mupdf_picture.hpp"

picture cur_pic;

/******************************************************************************
* The interface of the library: what the page (platform/wasm) and the tests
* call. Pages are numbered from 1; a zoom factor of 5 is one pixel per PIXEL.
******************************************************************************/

extern "C" {

// implemented in platform/wasm/mylib.js
extern void vaujs_set_pixmap (unsigned char* p, unsigned int s, int w, int h);

static void
publish_pixmap () {
  // hand the RGBA samples of cur_pic over to JavaScript
#ifdef __EMSCRIPTEN__
  mupdf_picture_rep *pp= (mupdf_picture_rep*)(cur_pic->get_handle());
  unsigned char* samples= fz_pixmap_samples (mupdf_context(), pp->pix);
  vaujs_set_pixmap (samples, pp->pix->w*pp->pix->h*pp->pix->n,
                    pp->get_width(), pp->get_height());
#endif
}

EMSCRIPTEN_KEEPALIVE
void
wasm_init_vau () {
  init_vau_lib (0, NULL);
}

EMSCRIPTEN_KEEPALIVE
int
wasm_open_document (const char *name) {
  // load and typeset a document, return its number of pages (0: failure)
  string s(name);
  cout << "wasm_open_document " << s << LF;
  url u= s;
  if (!exists (u)) return 0;
  vau_buffer buf= concrete_buffer_insist (s);
  set_current_editor (new_editor (buf));
  current_editor ()->typeset_document ("300");
  return current_editor ()->get_nr_pages ();
}

EMSCRIPTEN_KEEPALIVE
int
wasm_get_nr_pages () {
  if (is_nil (current_editor ())) return 0;
  return current_editor ()->get_nr_pages ();
}

EMSCRIPTEN_KEEPALIVE
int
wasm_get_page_width (int page, double zoomf) {
  int w= 0, h= 0;
  if (!is_nil (current_editor ()))
    current_editor ()->get_page_size (page, zoomf, w, h);
  return w;
}

EMSCRIPTEN_KEEPALIVE
int
wasm_get_page_height (int page, double zoomf) {
  int w= 0, h= 0;
  if (!is_nil (current_editor ()))
    current_editor ()->get_page_size (page, zoomf, w, h);
  return h;
}

EMSCRIPTEN_KEEPALIVE
void
wasm_get_page_pixmap (int page) {
  cur_pic= as_native_picture (current_editor ()->get_page_picture (page));
  publish_pixmap ();
}

EMSCRIPTEN_KEEPALIVE
void
wasm_get_view_pixmap (int page, int width, int height, double zoomf,
                      int scroll_x, int scroll_y) {
  cur_pic= as_native_picture (
    current_editor ()->get_view_picture (page, width, height, zoomf,
                                         scroll_x, scroll_y));
  publish_pixmap ();
}

EMSCRIPTEN_KEEPALIVE
unsigned int
wasm_get_page_pixmap_width () {
  return cur_pic->get_width();
}

EMSCRIPTEN_KEEPALIVE
unsigned int
wasm_get_page_pixmap_height () {
  return cur_pic->get_height();
}

EMSCRIPTEN_KEEPALIVE
void
wasm_save_page_png (int page, const char *name) {
  picture pic= current_editor ()->get_page_picture (page);
  save_picture (url_system (string (name)), pic);
}

EMSCRIPTEN_KEEPALIVE
int
wasm_export_pdf (const char *name) {
  // print the document to a PDF file, return 1 if the file is there
  url u= url_system (string (name));
  current_editor ()->print_to_file (u);
  return exists (u)? 1: 0;
}

EMSCRIPTEN_KEEPALIVE
void
wasm_eval (const char *s) {
  eval (s);
}

EMSCRIPTEN_KEEPALIVE
const char*
wasm_eval_to_string (const char *s) {
  // evaluate a Scheme expression, return its value as written by Scheme
  // (the string belongs to the library and lasts until the next call)
  static char* r= NULL;
  if (r != NULL) tm_delete_array (r);
  r= as_charp (object_to_string (eval (s)));
  return r;
}

} // extern "C"



#ifndef __EMSCRIPTEN__

///////////////////////////////////////////////////////////////////////////////

#include <SDL2/SDL.h>

struct gezira_Window_ {
  int width, height;
  SDL_Window *win;
  SDL_Renderer *ren;
  int page;
  double zoomf;
};

typedef struct gezira_Window_ gezira_Window_t;

void
gezira_Window_init (gezira_Window_t *window, int width, int height)
{
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
  }
  window->width = width; window->height = height;
  window->page = 1; window->zoomf = 5.0;
  window->win = SDL_CreateWindow("Hello World!", 100, 100, width, height,
                                 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_ALLOW_HIGHDPI);
  if (window->win == NULL) {
    fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
  }
  
  window->ren = SDL_CreateRenderer(window->win, -1,
                                   SDL_RENDERER_ACCELERATED |
                                   SDL_RENDERER_PRESENTVSYNC);
  if (window->ren == NULL) {
    fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window->win);
  }
}

void
gezira_Window_fini (gezira_Window_t *window)
{
  SDL_DestroyRenderer(window->ren);
  SDL_DestroyWindow(window->win);
}

static SDL_Surface*
get_surface (picture backing_store) {
  fz_pixmap *pix= ((mupdf_picture_rep*)backing_store->get_handle())->pix;
  //snapshot_pixmap (pix);
  unsigned char *samples= fz_pixmap_samples (mupdf_context (), pix);
  int w= fz_pixmap_width (mupdf_context (), pix);
  int h= fz_pixmap_height (mupdf_context (), pix);
  //  fz_keep_pixmap (mupdf_context (), pix);
  SDL_Surface *surf= NULL;
  unsigned char *pixels= tm_new_array<unsigned char>(w*h*4);
  memcpy (pixels, samples, w*h*4);
  // the SDL pixel data is not copied so we need to ensure that the pixmap stays alive.
  surf= SDL_CreateRGBSurfaceWithFormatFrom (pixels, w, h, 32, 4*w,
                                            SDL_PIXELFORMAT_RGBA32); // FIXME: premultiplied?
  return surf;
}


void
gezira_Window_update (gezira_Window_t *window)
{
  SDL_GetWindowSize (window->win, &window->width, &window->height);
  
  cout << "wasm_get_view_pixmap " << window->page << ", "
       << window->width << ", " << window->height << ", "
       << window->zoomf  << LF;
  cur_pic= as_native_picture (
            current_editor ()->get_view_picture (window->page, window->width*2,
                                                 window->height*2, window->zoomf));
  SDL_Surface *surface= get_surface (cur_pic);
  SDL_Texture* tex= SDL_CreateTextureFromSurface (window->ren, surface);
  SDL_SetTextureBlendMode (tex, SDL_BLENDMODE_NONE);
  SDL_Rect srcrect;
  srcrect.x= 0; srcrect.y= 0;
  srcrect.w= window->width*2; srcrect.h= window->height*2;
  SDL_Rect destrect;
  destrect.x= 0; destrect.y= 0;
  destrect.w= window->width; destrect.h= window->height;
  SDL_RenderClear (window->ren);
  SDL_RenderCopy (window->ren, tex, &srcrect, &srcrect);
  //SDL_RenderCopy (window->ren, tex, NULL, NULL);
  SDL_DestroyTexture (tex);
  unsigned char *p= (unsigned char*)surface->pixels;
  SDL_FreeSurface (surface);
  tm_delete_array (p);
  SDL_RenderPresent (window->ren);
}


void
gezira_Window_loop (gezira_Window_t *window)
{
  SDL_Event event;
  bool quit = false;
  bool redraw = true;

  while (!quit) {
    SDL_PollEvent(&event);

    if (redraw) {
      gezira_Window_update(window);
      redraw = false;
    }

    switch( event.type ){
      case SDL_KEYDOWN: {
        fprintf(stderr, "Key press detected: %c %s\n", (char)event.key.keysym.sym, SDL_GetScancodeName(event.key.keysym.scancode));
        //return (char)event.key.keysym.sym;
        switch(event.key.keysym.sym) {
          case SDLK_MINUS:
            fprintf(stderr, "zoom out\n");
            window->zoomf /= 1.2; redraw = true;
            break;
          case SDLK_EQUALS:
            fprintf(stderr, "zoom in\n");
            window->zoomf *= 1.2; redraw = true;
            break;
          case SDLK_PAGEUP:
            fprintf(stderr, "page up\n");
            window->page -= 1; redraw = true;
            break;
          case SDLK_PAGEDOWN:
            fprintf(stderr, "page down\n");
            window->page += 1; redraw = true;
            break;
          default:
            break;
        }
        }
        break;
        
      case SDL_QUIT:
        fprintf(stderr, "SDL_QUIT\n");
        quit = true;
        break;
        
      default:
        break;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void test_vau() {
  //  string name ("$TEXMACS_PATH/vau-tests/grassmann-sq-example.tm");
  //  string name ("$TEXMACS_PATH/examples/texts/bracket-test.tm");
  //  vau_buffer buf= concrete_buffer_insist (name);
  //  set_current_editor (new_editor (buf));
  //  current_editor ()->typeset_document ("300");
  //  picture pic= current_editor ()->get_page_picture (1);
  //  save_picture ("$HOME/vau-test.png", pic);
  //current_editor()->print_to_file ("$HOME/vau-test.pdf");
  
  // headless test: VAU_TEST_OUTPUT=prefix [VAU_TEST_DOCUMENT=file.tm] writes
  // prefix.png (first page) and prefix.pdf without opening a window
  string test_out= get_env ("VAU_TEST_OUTPUT");
  if (test_out != "") {
    string test_doc= get_env ("VAU_TEST_DOCUMENT");
    if (test_doc == "")
      test_doc= "$TEXMACS_PATH/vau-tests/grassmann-sq-example.tm";
    c_string _test_doc (test_doc);
    wasm_open_document (_test_doc);
    picture pic= current_editor ()->get_page_picture (0);
    save_picture (url_system (test_out * ".png"), pic);
    current_editor ()->print_to_file (url_system (test_out * ".pdf"));
    return;
  }

  wasm_open_document ("$TEXMACS_PATH/vau-tests/grassmann-sq-example.tm");
//  wasm_open_document ("$TEXMACS_PATH/vau-tests/ibp-exponential-example.tm");
  // for (int i=0; i<40; i++) wasm_get_page_pixmap (i);
  //  set_current_editor (editor ());
  gezira_Window_t win;
  gezira_Window_init(&win, 800, 600);
  gezira_Window_update(&win);
  gezira_Window_loop(&win);
  gezira_Window_fini(&win);
}

#endif // !defined __EMSCRIPTEN__
