
/******************************************************************************
* MODULE     : vau_editor.hpp
* DESCRIPTION: Vau editor
* COPYRIGHT  : (C) 2023  Joris van der Hoeven and Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/


#ifndef VAU_EDITOR_H
#define VAU_EDITOR_H
#include "env.hpp"
#include "typesetter.hpp"
//#include "editor.hpp"
#include "hashset.hpp"
#include "new_data.hpp"

#define THE_CURSOR 1
#define THE_FOCUS 2
#define THE_TREE 4
#define THE_ENVIRONMENT 8
#define THE_SELECTION 16
#define THE_DECORATIONS 32
#define THE_EXTENTS 64
#define THE_LOCUS 128
#define THE_MENUS 256
#define THE_FREEZE 512

class vau_buffer_rep;
typedef vau_buffer_rep* vau_buffer;
class document_rep;

class editor_rep: concrete_struct {

protected:

  vau_buffer    buf;  // the underlying buffer
  drd_info     drd;  // the drd for the buffer
  tree&        et;   // all TeXmacs trees
  box          eb;   // box translation of tree
  path         rp;   // path to the root of the document in et
  path         tp;   // path of cursor in tree
#ifdef EXPERIMENTAL
  environment  ste;  // environment for style rewriting
  tree         cct;  // clean copy of the document tree
  memorizer    mem;  // style converted document tree
#endif

protected:
  tree the_style;                         // document style
  hashmap<path,hashmap<string,tree> > cur; // environment at different paths
  hashmap<string,tree> stydef;            // environment after styles
  hashmap<string,tree> pre;               // environment after styles and init
  hashmap<string,tree> init;              // environment changes w.r.t. style
  hashmap<string,tree> fin;               // environment changes w.r.t. doc
  hashmap<string,tree> grefs;             // global references
  edit_env env;                           // the environment for typesetting
  typesetter ttt;                         // the (not) yet typesetted document

protected:
  typesetter           get_typesetter ();
  tree                 get_style ();
  void                 set_style (tree t);
  hashmap<string,tree> get_init ();
  hashmap<string,tree> get_fin ();
  hashmap<string,tree> get_ref ();
  hashmap<string,tree> get_aux ();
  hashmap<string,tree> get_att ();
  void                 set_init (hashmap<string,tree> init= tree ("?"));
  void                 add_init (hashmap<string,tree> init);
  void                 set_fin (hashmap<string,tree> fin);
  void                 set_ref (hashmap<string,tree> ref);
  void                 set_aux (hashmap<string,tree> aux);
  void                 set_att (hashmap<string,tree> att);

public:
  editor_rep (vau_buffer buf2);
  ~editor_rep ();
  void clear_local_info ();

  // from edit_interface
  virtual void notify_change (int changed);
  virtual bool has_changed (int question);
  
  // from edit_graphics
  virtual bool   inside_graphics (bool b=true);

  void     init_update ();
  void     drd_update ();
#ifdef EXPERIMENTAL
  void     environment_update ();
#endif
  tree     get_full_env ();
  bool     defined_at_cursor (string var_name);
  bool     defined_at_init (string var_name);
  bool     defined_in_init (string var_name);
  tree     get_env_value (string var_name, path p);
  tree     get_env_value (string var_name);
  tree     get_init_value (string var_name);
  string   get_env_string (string var_name);
  string   get_init_string (string var_name);
  int      get_env_int (string var_name);
  int      get_init_int (string var_name);
  double   get_env_double (string var_name);
  double   get_init_double (string var_name);
  color    get_env_color (string var_name);
  color    get_init_color (string var_name);
  language get_env_language ();
  int      get_page_count ();
  SI       get_page_width (bool deco);
  SI       get_pages_width (bool deco);
  SI       get_page_height (bool deco);
  SI       get_total_width (bool deco);
  SI       get_total_height (bool deco);
  
  bool     get_save_aux ();
  void     get_data (new_data& data);
  void     set_data (new_data data);
  
  tree     exec (tree t, hashmap<string,tree> env, bool expand_refs= true);
  tree     exec_texmacs (tree t, path p);
  tree     exec_texmacs (tree t);
  tree     exec_verbatim (tree t, path p);
  tree     exec_verbatim (tree t);
  tree     exec_html (tree t, path p);
  tree     exec_html (tree t);
  tree     exec_latex (tree t, path p);
  tree     exec_latex (tree t);
  tree     texmacs_exec (tree t);
  tree     var_texmacs_exec (tree t);

  tree     checkout_animation (tree t);
  tree     commit_animation (tree t);

  void     change_style (tree style);
  tree     get_init_all ();
  void     init_env (string var, tree by);
  void     init_default (string var);
  void     init_style ();
  void     init_style (string style);
  tree     get_ref (string key);
  tree     get_aux (string key);
  tree     get_att (string key);
  void     set_ref (string key, tree im);
  void     set_aux (string key, tree im);
  void     set_att (string key, tree im);
  void     reset_ref (string key);
  void     reset_aux (string key);
  void     reset_att (string key);
  array<string> find_refs (string val, bool global);
  array<string> list_refs (bool global);
  array<string> list_auxs (bool global);
  array<string> list_atts (bool global);

  void     typeset_style_use_cache (tree style);
  void     typeset_preamble ();
  void     typeset_prepare ();
  void     typeset_invalidate_env ();
  void     typeset_exec_until (path p);
  void     typeset_invalidate (path p);
  void     typeset_invalidate_all ();
  void     typeset_invalidate_players (path p, bool reattach);
  void     typeset_sub (SI& x1, SI& y1, SI& x2, SI& y2);
  void     typeset (SI& x1, SI& y1, SI& x2, SI& y2);
  void     typeset_forced ();

  string get_metadata (string kind);
  url get_name ();
  void print_doc (url ps_name, bool to_file, int first, int last);
  void print_to_file (url ps_name, string first="1", string last="1000000");

  tree the_subtree (path p);
  
  friend class editor;

};


class editor {
  CONCRETE_NULL(editor);
public:
  editor (vau_buffer buf);
  inline bool operator == (editor w) { return rep == w.rep; }
  inline bool operator != (editor w) { return rep != w.rep; }
  friend class editor_rep;
};

CONCRETE_NULL_CODE(editor);

editor new_editor (vau_buffer buf);

#endif // defined EDIT_TYPESET_H
