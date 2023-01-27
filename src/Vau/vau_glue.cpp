
/******************************************************************************
* MODULE     : vau_glue.cpp
* DESCRIPTION: Glue to Scheme
* COPYRIGHT  : (C) 2023  Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/


#include "vau_glue.hpp"

#include "promise.hpp"
#include "tree.hpp"
#include "drd_mode.hpp"
#include "tree_search.hpp"
#include "modification.hpp"
#include "patch.hpp"

#include "boxes.hpp"
#include "universal.hpp"
#include "convert.hpp"
#include "file.hpp"
#include "locale.hpp"
#include "iterator.hpp"
#include "Freetype/tt_tools.hpp"


/******************************************************************************
* Assertion support (partly obsolete)
******************************************************************************/

/* The SCM_EXPECT macros provide branch prediction hints to the
   compiler.  To use only in places where the result of the expression
   under "normal" circumstances is known.  */
#ifdef __GNUC__
# define TMSCM_EXPECT    __builtin_expect
#else
# define TMSCM_EXPECT(_expr, _value) (_expr)
#endif

#define TMSCM_LIKELY(_expr)    TMSCM_EXPECT ((_expr), 1)
#define TMSCM_UNLIKELY(_expr)  TMSCM_EXPECT ((_expr), 0)

#define TMSCM_ASSERT(_cond, _arg, _pos, _subr)                    \
  do { if (TMSCM_UNLIKELY (!(_cond)))                             \
     s7_wrong_type_arg_error (tm_s7, _subr, _pos, _arg, "some other thing"); } while (0)

// old unused stuff

#define TMSCM_ARG1 1
#define TMSCM_ARG2 2
#define TMSCM_ARG3 3
#define TMSCM_ARG4 4
#define TMSCM_ARG5 5
#define TMSCM_ARG6 6
#define TMSCM_ARG7 7
#define TMSCM_ARG8 8
#define TMSCM_ARG9 9
#define TMSCM_ARG10 10

/*******************************************************************************/

#define TMSCM_CONVERSION_2(TYPE, TYPE_CHECK) \
  template<> tmscm tmscm_from<TYPE> (TYPE p) { return TYPE##_to_tmscm (p); } \
  template<> TYPE tmscm_to<TYPE> (tmscm p) { return tmscm_to_##TYPE (p); } \
  template<> void tmscm_check<TYPE> (tmscm p, int arg, const char *fname) { \
    TMSCM_ASSERT (tmscm_is_##TYPE_CHECK (p), p, arg, fname); \
  }

#define TMSCM_CONVERSION(TYPE) TMSCM_CONVERSION_2(TYPE, TYPE) \
  template<> bool tmscm_is<TYPE> (tmscm p) { return tmscm_is_##TYPE (p); }

list<glue_function> glue_function_rep::glue_functions;

template<typename S0, S0 f, typename T0, typename ... Ts>
  const char *tm_glue<T0 (Ts ...), S0, f>::__name;

bool tmscm_is_object (tmscm o) { return true; } // no check

TMSCM_CONVERSION(int)
TMSCM_CONVERSION(double)
TMSCM_CONVERSION(bool)
TMSCM_CONVERSION(string)
TMSCM_CONVERSION(object)


// scheme_tree is a typedef and does not play well with C++ templates
// so we need to define a real type scheme_tree_t to marshall correctly
// data across the Scheme boundary
// same happens for content_wrap (see below)

class scheme_tree_t {
  scheme_tree t;
public:
  template <typename T> scheme_tree_t (T a) : t (a) {}
//  scheme_tree_t (string s) : t (s) {}
  operator tree () { return t; }
};

tmscm
blackboxP (tmscm t) {
  bool b= tmscm_is_blackbox (t);
  return bool_to_tmscm (b);
}

#if 0
template<class T> tmscm box_to_tmscm (T o) {
  return blackbox_to_tmscm (close_box<T> (o)); }
template<class T> T tmscm_to_box (tmscm obj) {
  return open_box<T>(tmscm_to_blackbox (obj));  }
template<class T> tmscm cmp_box (tmscm o1, tmscm o2) {
  return bool_to_tmscm (tmscm_to_box<T> (o1) == tmscm_to_box<T> (o2)); }
template<class T> tmscm boxP (tmscm t) {
  bool b= tmscm_is_blackbox (t) &&
          (type_box (blackboxvalue(t)) == type_helper<T>::id);
  return bool_to_tmscm (b);
}
#endif

/******************************************************************************
* Basic assertions
******************************************************************************/

#define TMSCM_ASSERT_STRING(s,arg,rout) \
TMSCM_ASSERT (tmscm_is_string (s), s, arg, rout)
#define TMSCM_ASSERT_BOOL(flag,arg,rout) \
TMSCM_ASSERT (tmscm_is_bool (flag), flag, arg, rout)
#define TMSCM_ASSERT_INT(i,arg,rout) \
TMSCM_ASSERT (tmscm_is_int (i), i, arg, rout);
#define TMSCM_ASSERT_DOUBLE(i,arg,rout) \
  TMSCM_ASSERT (tmscm_is_double (i), i, arg, rout);
//TMSCM_ASSERT (SCM_REALP (i), i, arg, rout);
#define TMSCM_ASSERT_URL(u,arg,rout) \
TMSCM_ASSERT (tmscm_is_url (u) || tmscm_is_string (u), u, arg, rout)
#define TMSCM_ASSERT_MODIFICATION(m,arg,rout) \
TMSCM_ASSERT (tmscm_is_modification (m), m, arg, rout)
#define TMSCM_ASSERT_PATCH(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_patch (p), p, arg, rout)
#define TMSCM_ASSERT_BLACKBOX(t,arg,rout) \
TMSCM_ASSERT (tmscm_is_blackbox (t), t, arg, rout)
#define TMSCM_ASSERT_SYMBOL(s,arg,rout) \
  TMSCM_ASSERT (tmscm_is_symbol (s), s, arg, rout)
//TMSCM_ASSERT (SCM_NFALSEP (tmscm_symbol_p (s)), s, arg, rout)

#define TMSCM_ASSERT_OBJECT(a,b,c)
// no check

/******************************************************************************
* Tree labels
******************************************************************************/

#define TMSCM_ASSERT_TREE_LABEL(p,arg,rout) TMSCM_ASSERT_SYMBOL(p,arg,rout)

tmscm
tree_label_to_tmscm (tree_label l) {
  string s= as_string (l);
  return symbol_to_tmscm (s);
}

tree_label
tmscm_to_tree_label (tmscm p) {
  string s= tmscm_to_symbol (p);
  return make_tree_label (s);
}

TMSCM_CONVERSION_2(tree_label, symbol)

/******************************************************************************
* Trees
******************************************************************************/

#define TMSCM_ASSERT_TREE(t,arg,rout) TMSCM_ASSERT (tmscm_is_tree (t), t, arg, rout)


bool
tmscm_is_tree (tmscm u) {
  return (tmscm_is_blackbox (u) &&
         (type_box (tmscm_to_blackbox(u)) == type_helper<tree>::id));
}

tmscm
tree_to_tmscm (tree o) {
  return blackbox_to_tmscm (close_box<tree> (o));
}

tree
tmscm_to_tree (tmscm obj) {
  return open_box<tree>(tmscm_to_blackbox (obj));
}

TMSCM_CONVERSION (tree)

tmscm
treeP (tmscm t) {
  bool b= tmscm_is_blackbox (t) &&
          (type_box (tmscm_to_blackbox(t)) == type_helper<tree>::id);
  return bool_to_tmscm (b);
}

tree
coerce_string_tree (string s) {
  return s;
}

string
coerce_tree_string (tree t) {
  return as_string (t);
}

tree
tree_ref (tree t, int i) {
  return t[i];
}

tree
tree_set (tree t, int i, tree u) {
  t[i]= u;
  return u;
}

tree
tree_range (tree t, int i, int j) {
  return t(i,j);
}

tree
tree_append (tree t1, tree t2) {
  return t1 * t2;
}

bool
tree_active (tree t) {
  path ip= obtain_ip (t);
  return is_nil (ip) || last_item (ip) != DETACHED;
}

tree
tree_child_insert (tree t, int pos, tree x) {
  //cout << "t= " << t << "\n";
  //cout << "x= " << x << "\n";
  int i, n= N(t);
  tree r (t, n+1);
  for (i=0; i<pos; i++) r[i]= t[i];
  r[pos]= x;
  for (i=pos; i<n; i++) r[i+1]= t[i];
  return r;
}

/******************************************************************************
* Document modification routines
******************************************************************************/

extern tree the_et;

tree
tree_assign (tree r, tree t) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    assign (reverse (ip), copy (t));
    return subtree (the_et, reverse (ip));
  }
  else {
    assign (r, copy (t));
    return r;
  }
}

tree
tree_insert (tree r, int pos, tree t) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    insert (reverse (path (pos, ip)), copy (t));
    return subtree (the_et, reverse (ip));
  }
  else {
    insert (r, pos, copy (t));
    return r;
  }
}

tree
tree_remove (tree r, int pos, int nr) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    remove (reverse (path (pos, ip)), nr);
    return subtree (the_et, reverse (ip));
  }
  else {
    remove (r, pos, nr);
    return r;
  }
}

tree
tree_split (tree r, int pos, int at) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    split (reverse (path (at, pos, ip)));
    return subtree (the_et, reverse (ip));
  }
  else {
    split (r, pos, at);
    return r;
  }
}

tree
tree_join (tree r, int pos) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    join (reverse (path (pos, ip)));
    return subtree (the_et, reverse (ip));
  }
  else {
    join (r, pos);
    return r;
  }
}

tree
tree_assign_node (tree r, tree_label op) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    assign_node (reverse (ip), op);
    return subtree (the_et, reverse (ip));
  }
  else {
    assign_node (r, op);
    return r;
  }
}

tree
tree_insert_node (tree r, int pos, tree t) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    insert_node (reverse (path (pos, ip)), copy (t));
    return subtree (the_et, reverse (ip));
  }
  else {
    insert_node (r, pos, copy (t));
    return r;
  }
}

tree
tree_remove_node (tree r, int pos) {
  path ip= copy (obtain_ip (r));
  if (ip_attached (ip)) {
    remove_node (reverse (path (pos, ip)));
    return subtree (the_et, reverse (ip));
  }
  else {
    remove_node (r, pos);
    return r;
  }
}

/******************************************************************************
* Scheme trees
******************************************************************************/

#define TMSCM_ASSERT_SCHEME_TREE(p,arg,rout)

tmscm
scheme_tree_to_tmscm (scheme_tree t) {
//  scheme_tree_t t= tw.t;
  if (is_atomic (t)) {
    string s= t->label;
    if (s == "#t") return tmscm_true ();
    if (s == "#f") return tmscm_false ();
    if (is_int (s)) return int_to_tmscm (as_int (s));
    if (is_quoted (s))
      return string_to_tmscm (scm_unquote (s));
    //if ((N(s)>=2) && (s[0]=='\42') && (s[N(s)-1]=='\42'))
    //return string_to_tmscm (s (1, N(s)-1));
    if (N(s) >= 1 && s[0] == '\'') return symbol_to_tmscm (s (1, N(s)));
    return symbol_to_tmscm (s);
  }
  else {
    int i;
    tmscm p= tmscm_null ();
    for (i=N(t)-1; i>=0; i--)
      p= tmscm_cons (scheme_tree_to_tmscm (t[i]), p);
    return p;
  }
}

scheme_tree
tmscm_to_scheme_tree (tmscm p) {
  if (tmscm_is_list (p)) {
    tree t (TUPLE);
    while (!tmscm_is_null (p)) {
      t << tmscm_to_scheme_tree (tmscm_car (p));
      p= tmscm_cdr (p);
    }
    return t;
  }
  if (tmscm_is_symbol (p)) return tmscm_to_symbol (p);
  if (tmscm_is_string (p)) return scm_quote (tmscm_to_string (p));
  //if (tmscm_is_string (p)) return "\"" * tmscm_to_string (p) * "\"";
  if (tmscm_is_int (p)) return as_string ((int) tmscm_to_int (p));
  if (tmscm_is_bool (p)) return (tmscm_to_bool (p)? string ("#t"): string ("#f"));
  if (tmscm_is_tree (p)) return tree_to_scheme_tree (tmscm_to_tree (p));
  return "?";
}

tmscm
scheme_tree_t_to_tmscm (scheme_tree_t t) {
  return scheme_tree_to_tmscm ((tree)t);
}

scheme_tree_t tmscm_to_scheme_tree_t (tmscm obj) {
  return tree (tmscm_to_scheme_tree (obj));
}

template<> tmscm tmscm_from<scheme_tree_t> (scheme_tree_t p) { return scheme_tree_t_to_tmscm (p); }
template<> scheme_tree_t tmscm_to<scheme_tree_t> (tmscm p) { return tmscm_to_scheme_tree_t (p); }
template<> bool tmscm_is<scheme_tree_t> (tmscm p) { return true; } //FIXME: let's do better.
template<> void tmscm_check<scheme_tree_t> (tmscm p, int arg, const char* fname) { } //FIXME: let's do better.

/******************************************************************************
* Content
******************************************************************************/

bool
tmscm_is_content (tmscm p) {
  if (tmscm_is_string (p) || tmscm_is_tree (p)) return true;
  else if (!tmscm_is_pair (p) || !tmscm_is_symbol (tmscm_car (p))) return false;
  else {
    for (p= tmscm_cdr (p); !tmscm_is_null (p); p= tmscm_cdr (p))
      if (!tmscm_is_content (tmscm_car (p))) return false;
    return true;
  }
}

#define content tree
#define TMSCM_ASSERT_CONTENT(p,arg,rout) \
   TMSCM_ASSERT (tmscm_is_content (p), p, arg, rout)
#define content_to_tmscm tree_to_tmscm

tree
tmscm_to_content (tmscm p) {
  if (tmscm_is_string (p)) return tmscm_to_string (p);
  if (tmscm_is_tree (p)) return tmscm_to_tree (p);
  if (tmscm_is_pair (p)) {
    if (!tmscm_is_symbol (tmscm_car (p))) return "?";
    tree t (make_tree_label (tmscm_to_symbol (tmscm_car (p))));
    p= tmscm_cdr (p);
    while (!tmscm_is_null (p)) {
      t << tmscm_to_content (tmscm_car (p));
      p= tmscm_cdr (p);
    }
    return t;
  }
  return "?";
}

tmscm
contentP (tmscm t) {
  bool b= tmscm_is_content (t);
  return bool_to_tmscm (b);
}

struct content_wrap {
  tree t;
  inline content_wrap (const tree& x) : t (x) {};
};

template<> content_wrap
tmscm_to<content_wrap> (tmscm p) { return tmscm_to_content (p); }
template<> tmscm
tmscm_from<content_wrap> (content_wrap c) { return tree_to_tmscm (c.t); }
template<> bool
tmscm_is<content_wrap> (tmscm o) { return tmscm_is_content(o); }


/******************************************************************************
* Paths
******************************************************************************/

bool
tmscm_is_path (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_int (tmscm_car (p)) && tmscm_is_path (tmscm_cdr (p));
}

#define TMSCM_ASSERT_PATH(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_path (p), p, arg, rout)

tmscm
path_to_tmscm (path p) {
  if (is_nil (p)) return tmscm_null ();
  else return tmscm_cons (int_to_tmscm (p->item), path_to_tmscm (p->next));
}

path
tmscm_to_path (tmscm p) {
  if (tmscm_is_null (p)) return path ();
  else return path ((int) tmscm_to_int (tmscm_car (p)),
                          tmscm_to_path (tmscm_cdr (p)));
}

TMSCM_CONVERSION (path)

/******************************************************************************
* Observers
******************************************************************************/

#define TMSCM_ASSERT_OBSERVER(o,arg,rout) \
TMSCM_ASSERT (tmscm_is_observer (o), o, arg, rout)


bool
tmscm_is_observer (tmscm o) {
  return (tmscm_is_blackbox (o) &&
         (type_box (tmscm_to_blackbox(o)) == type_helper<observer>::id));
}

tmscm
observer_to_tmscm (observer o) {
  return blackbox_to_tmscm (close_box<observer> (o));
}

static observer
tmscm_to_observer (tmscm obj) {
  return open_box<observer>(tmscm_to_blackbox (obj));
}

tmscm
observerP (tmscm t) {
  bool b= tmscm_is_blackbox (t) &&
  (type_box (tmscm_to_blackbox(t)) == type_helper<observer>::id);
  return bool_to_tmscm (b);
}

TMSCM_CONVERSION (observer)

/******************************************************************************
* Commands
******************************************************************************/

#define TMSCM_ASSERT_COMMAND(o,arg,rout) \
TMSCM_ASSERT (tmscm_is_command (o), o, arg, rout)

bool
tmscm_is_command (tmscm u) {
  return (tmscm_is_blackbox (u) &&
      (type_box (tmscm_to_blackbox(u)) == type_helper<command>::id));
}

static tmscm
command_to_tmscm (command o) {
  return blackbox_to_tmscm (close_box<command> (o));
}

static command
tmscm_to_command (tmscm o) {
  return open_box<command> (tmscm_to_blackbox (o));
}

TMSCM_CONVERSION (command)


/******************************************************************************
* Urls
******************************************************************************/

bool
tmscm_is_url (tmscm u) {
  return (tmscm_is_blackbox (u)
              && (type_box (tmscm_to_blackbox(u)) == type_helper<url>::id))
         || (tmscm_is_string(u));
}

tmscm
url_to_tmscm (url u) {
  return blackbox_to_tmscm (close_box<url> (u));
}

url
tmscm_to_url (tmscm obj) {
  if (tmscm_is_string (obj))
#ifdef OS_MINGW
    return url_system (tmscm_to_string (obj));
#else
  return tmscm_to_string (obj);
#endif
  return open_box<url> (tmscm_to_blackbox (obj));
}

tmscm
urlP (tmscm t) {
  bool b= tmscm_is_url (t);
  return bool_to_tmscm (b);
}

url url_concat (url u1, url u2) { return u1 * u2; }
url url_or (url u1, url u2) { return u1 | u2; }
void string_save (string s, url u) { (void) save_string (u, s); }
string string_load (url u) {
  string s; (void) load_string (u, s, false); return s; }
void string_append_to_file (string s, url u) { (void) append_string (u, s); }
url url_ref (url u, int i) { return u[i]; }

TMSCM_CONVERSION (url)

/******************************************************************************
* Modification
******************************************************************************/

bool
tmscm_is_modification (tmscm m) {
  return (tmscm_is_blackbox (m) &&
    (type_box (tmscm_to_blackbox(m)) == type_helper<modification>::id))
    || (tmscm_is_string (m));
}

tmscm
modification_to_tmscm (modification m) {
  return blackbox_to_tmscm (close_box<modification> (m));
}

modification
tmscm_to_modification (tmscm obj) {
  return open_box<modification> (tmscm_to_blackbox (obj));
}

tmscm
modificationP (tmscm t) {
  bool b= tmscm_is_modification (t);
  return bool_to_tmscm (b);
}

tree
var_apply (tree& t, modification m) {
  apply (t, copy (m));
  return t;
}

tree
var_clean_apply (tree& t, modification m) {
  return clean_apply (t, copy (m));
}

TMSCM_CONVERSION(modification)

/******************************************************************************
* Patch
******************************************************************************/

bool
tmscm_is_patch (tmscm p) {
  return (tmscm_is_blackbox (p) &&
    (type_box (tmscm_to_blackbox(p)) == type_helper<patch>::id))
    || (tmscm_is_string (p));
}

tmscm
patch_to_tmscm (patch p) {
  return blackbox_to_tmscm (close_box<patch> (p));
}

patch
tmscm_to_patch (tmscm obj) {
  return open_box<patch> (tmscm_to_blackbox (obj));
}

tmscm
patchP (tmscm t) {
  bool b= tmscm_is_patch (t);
  return bool_to_tmscm (b);
}

patch
branch_patch (array<patch> a) {
  return patch (true, a);
}

tree
var_clean_apply (tree t, patch p) {
  return clean_apply (copy (p), t);
}

tree
var_apply (tree& t, patch p) {
  apply (copy (p), t);
  return t;
}

TMSCM_CONVERSION(patch)

/******************************************************************************
* Table types
******************************************************************************/

typedef hashmap<string,string> table_string_string;

bool
tmscm_is_table_string_string (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else if (!tmscm_is_pair (p)) return false;
  else {
    tmscm f= tmscm_car (p);
    return tmscm_is_pair (f) &&
    tmscm_is_string (tmscm_car (f)) &&
    tmscm_is_string (tmscm_cdr (f)) &&
    tmscm_is_table_string_string (tmscm_cdr (p));
  }
}

#define TMSCM_ASSERT_TABLE_STRING_STRING(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_table_string_string (p), p, arg, rout)

tmscm
table_string_string_to_tmscm (hashmap<string,string> t) {
  tmscm p= tmscm_null ();
  iterator<string> it= iterate (t);
  while (it->busy ()) {
    string s= it->next ();
    tmscm n= tmscm_cons (string_to_tmscm (s), string_to_tmscm (t[s]));
    p= tmscm_cons (n, p);
  }
  return p;
}

hashmap<string,string>
tmscm_to_table_string_string (tmscm p) {
  hashmap<string,string> t;
  while (!tmscm_is_null (p)) {
    tmscm n= tmscm_car (p);
    t (tmscm_to_string (tmscm_car (n)))= tmscm_to_string (tmscm_cdr (n));
    p= tmscm_cdr (p);
  }
  return t;
}

TMSCM_CONVERSION(table_string_string)


#define tmscm_is_solution tmscm_is_table_string_string
#define TMSCM_ASSERT_SOLUTION(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_solution(p), p, arg, rout)
#define solution_to_tmscm table_string_string_to_tmscm
#define tmscm_to_solution tmscm_to_table_string_string

/******************************************************************************
* Several array types
******************************************************************************/

typedef array<int> array_int;
typedef array<string> array_string;
typedef array<tree> array_tree;
typedef array<url> array_url;
typedef array<patch> array_patch;
typedef array<path> array_path;
typedef array<widget> array_widget;
typedef array<double> array_double;
typedef array<array<double> > array_array_double;
typedef array<array<array<double> > > array_array_array_double;

static bool
tmscm_is_array_int (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_int (tmscm_car (p)) &&
    tmscm_is_array_int (tmscm_cdr (p));
}

#define TMSCM_ASSERT_ARRAY_INT(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_int (p), p, arg, rout)

/* static */ tmscm
array_int_to_tmscm (array<int> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (int_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<int>
tmscm_to_array_int (tmscm p) {
  array<int> a;
  while (!tmscm_is_null (p)) {
    a << ((int) tmscm_to_int (tmscm_car (p)));
    p= tmscm_cdr (p);
  }
  return a;
}

static bool
tmscm_is_array_string (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_string (tmscm_car (p)) &&
    tmscm_is_array_string (tmscm_cdr (p));
}


/* static */ bool
tmscm_is_array_double (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_double (tmscm_car (p)) &&
    tmscm_is_array_double (tmscm_cdr (p));
}

#define TMSCM_ASSERT_ARRAY_DOUBLE(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_double (p), p, arg, rout)

/* static */ tmscm
array_double_to_tmscm (array<double> a) {
  int i, n= N(a);
  tmscm p= tmscm_null();
  for (i=n-1; i>=0; i--) p= tmscm_cons (double_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<double>
tmscm_to_array_double (tmscm p) {
  array<double> a;
  while (!tmscm_is_null (p)) {
    a << ((double) tmscm_to_double (tmscm_car (p)));
    p= tmscm_cdr (p);
  }
  return a;
}

static bool
tmscm_is_array_array_double (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_array_double (tmscm_car (p)) &&
    tmscm_is_array_array_double (tmscm_cdr (p));
}

#define TMSCM_ASSERT_ARRAY_ARRAY_DOUBLE(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_array_double (p), p, arg, rout)

/* static */ tmscm
array_array_double_to_tmscm (array<array_double> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (array_double_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<array_double>
tmscm_to_array_array_double (tmscm p) {
  array<array_double> a;
  while (!tmscm_is_null (p)) {
    a << ((array_double) tmscm_to_array_double (tmscm_car (p)));
    p= tmscm_cdr (p);
  }
  return a;
}

static bool
tmscm_is_array_array_array_double (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_array_array_double (tmscm_car (p)) &&
    tmscm_is_array_array_array_double (tmscm_cdr (p));
}

#define TMSCM_ASSERT_ARRAY_ARRAY_ARRAY_DOUBLE(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_array_array_double (p), p, arg, rout)

/* static */ tmscm
array_array_array_double_to_tmscm (array<array_array_double> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (array_array_double_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<array_array_double>
tmscm_to_array_array_array_double (tmscm p) {
  array<array_array_double> a;
  while (!tmscm_is_null (p)) {
    a << ((array_array_double) tmscm_to_array_array_double (tmscm_car (p)));
    p= tmscm_cdr (p);
  }
  return a;
}

void register_glyph (string s, array_array_array_double gl);
string recognize_glyph (array_array_array_double gl);

#define TMSCM_ASSERT_ARRAY_STRING(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_string (p), p, arg, rout)

/* static */ tmscm
array_string_to_tmscm (array<string> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (string_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<string>
tmscm_to_array_string (tmscm p) {
  array<string> a;
  while (!tmscm_is_null (p)) {
    a << tmscm_to_string (tmscm_car (p));
    p= tmscm_cdr (p);
  }
  return a;
}

static bool
tmscm_is_array_tree (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_tree (tmscm_car (p)) &&
    tmscm_is_array_tree (tmscm_cdr (p));
}

#define TMSCM_ASSERT_ARRAY_TREE(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_tree (p), p, arg, rout)

/* static */ tmscm
array_tree_to_tmscm (array<tree> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (tree_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<tree>
tmscm_to_array_tree (tmscm p) {
  array<tree> a;
  while (!tmscm_is_null (p)) {
    a << tmscm_to_tree (tmscm_car (p));
    p= tmscm_cdr (p);
  }
  return a;
}

static bool
tmscm_is_array_url (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_url (tmscm_car (p)) &&
    tmscm_is_array_url (tmscm_cdr (p));
}


#define TMSCM_ASSERT_ARRAY_URL(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_url (p), p, arg, rout)

/* static */ tmscm
array_url_to_tmscm (array<url> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (url_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<url>
tmscm_to_array_url (tmscm p) {
  array<url> a;
  while (!tmscm_is_null (p)) {
    a << tmscm_to_url (tmscm_car (p));
    p= tmscm_cdr (p);
  }
  return a;
}

static bool
tmscm_is_array_patch (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_patch (tmscm_car (p)) &&
    tmscm_is_array_patch (tmscm_cdr (p));
}

#define TMSCM_ASSERT_ARRAY_PATCH(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_patch (p), p, arg, rout)

/* static */ tmscm
array_patch_to_tmscm (array<patch> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (patch_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<patch>
tmscm_to_array_patch (tmscm p) {
  array<patch> a;
  while (!tmscm_is_null (p)) {
    a << tmscm_to_patch (tmscm_car (p));
    p= tmscm_cdr (p);
  }
  return a;
}

static bool
tmscm_is_array_path (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_path (tmscm_car (p)) &&
    tmscm_is_array_path (tmscm_cdr (p));
}

#define TMSCM_ASSERT_ARRAY_PATH(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_array_path (p), p, arg, rout)

/* static */ tmscm
array_path_to_tmscm (array<path> a) {
  int i, n= N(a);
  tmscm p= tmscm_null ();
  for (i=n-1; i>=0; i--) p= tmscm_cons (path_to_tmscm (a[i]), p);
  return p;
}

/* static */ array<path>
tmscm_to_array_path (tmscm p) {
  array<path> a;
  while (!tmscm_is_null (p)) {
    a << tmscm_to_path (tmscm_car (p));
    p= tmscm_cdr (p);
  }
  return a;
}

TMSCM_CONVERSION(array_int)
TMSCM_CONVERSION(array_string)
TMSCM_CONVERSION(array_tree)
TMSCM_CONVERSION(array_url)
TMSCM_CONVERSION(array_patch)
TMSCM_CONVERSION(array_path)
TMSCM_CONVERSION(array_double)
TMSCM_CONVERSION(array_array_double)
TMSCM_CONVERSION(array_array_array_double)

/******************************************************************************
* List types
******************************************************************************/

typedef list<string> list_string;

bool
tmscm_is_list_string (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_string (tmscm_car (p)) &&
    tmscm_is_list_string (tmscm_cdr (p));
}

#define TMSCM_ASSERT_LIST_STRING(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_list_string (p), p, arg, rout)

tmscm
list_string_to_tmscm (list_string l) {
  if (is_nil (l)) return tmscm_null ();
  return tmscm_cons (string_to_tmscm (l->item),
           list_string_to_tmscm (l->next));
}

list_string
tmscm_to_list_string (tmscm p) {
  if (tmscm_is_null (p)) return list_string ();
  return list_string (tmscm_to_string (tmscm_car (p)),
            tmscm_to_list_string (tmscm_cdr (p)));
}

typedef list<tree> list_tree;

bool
tmscm_is_list_tree (tmscm p) {
  if (tmscm_is_null (p)) return true;
  else return tmscm_is_pair (p) &&
    tmscm_is_tree (tmscm_car (p)) &&
    tmscm_is_list_tree (tmscm_cdr (p));
}

#define TMSCM_ASSERT_LIST_TREE(p,arg,rout) \
TMSCM_ASSERT (tmscm_is_list_tree (p), p, arg, rout)

tmscm
list_tree_to_tmscm (list_tree l) {
  if (is_nil (l)) return tmscm_null ();
  return tmscm_cons (tree_to_tmscm (l->item),
           list_tree_to_tmscm (l->next));
}

list_tree
tmscm_to_list_tree (tmscm p) {
  if (tmscm_is_null (p)) return list_tree ();
  return list_tree (tmscm_to_tree (tmscm_car (p)),
            tmscm_to_list_tree (tmscm_cdr (p)));
}

TMSCM_CONVERSION(list_tree)
TMSCM_CONVERSION(list_string)

/******************************************************************************
* Gluing
******************************************************************************/

#include "boot.hpp"
#include "convert.hpp"
#include "file.hpp"
#include "image_files.hpp"
#include "web_files.hpp"
#include "sys_utils.hpp"
#include "analyze.hpp"
#include "wencoding.hpp"
#include "base64.hpp"
#include "tree_traverse.hpp"
#include "tree_analyze.hpp"
#include "tree_correct.hpp"
#include "tree_modify.hpp"
#include "tree_math_stats.hpp"
#include "Concat/concater.hpp"
#include "converter.hpp"
#include "tm_timer.hpp"
#include "Metafont/tex_files.hpp"
#include "Freetype/tt_file.hpp"
//#include "LaTeX_Preview/latex_preview.hpp"
//#include "Bibtex/bibtex.hpp"
//#include "Bibtex/bibtex_functions.hpp"
#include "link.hpp"
#include "dictionary.hpp"
#include "patch.hpp"
#include "packrat.hpp"
#include "new_style.hpp"
#include "persistent.hpp"
#include "font.hpp"

/******************************************************************************
* Miscellaneous routines for use by glue only
******************************************************************************/

string original_path;

string
get_original_path () {
  return original_path;
}

string
texmacs_version (string which) {
  if (which == "tgz") return TM_DEVEL;
  if (which == "rpm") return TM_DEVEL_RELEASE;
  if (which == "stgz") return TM_STABLE;
  if (which == "srpm") return TM_STABLE_RELEASE;
  if (which == "devel") return TM_DEVEL;
  if (which == "stable") return TM_STABLE;
  if (which == "devel-release") return TM_DEVEL_RELEASE;
  if (which == "stable-release") return TM_STABLE_RELEASE;
  if (which == "revision") return TEXMACS_REVISION;
  return TEXMACS_VERSION;
}

void
set_fast_environments (bool b) {
//  enable_fastenv= b;
}

void
win32_display (string s) {
  cout << s;
  cout.flush ();
}

void
tm_output (string s) {
  cout << s;
  cout.flush ();
}

void
tm_errput (string s) {
  cerr << s;
  cerr.flush ();
}

void
cpp_error () {
  //char *np= 0; *np= 1;
  FAILED ("an error occurred");
}

#if 0
array<int>
get_bounding_rectangle (tree t) {
  editor ed= get_current_editor ();
  rectangle wr= ed -> get_window_extents ();
  path p= reverse (obtain_ip (t));
  selection sel= ed->search_selection (p * start (t), p * end (t));
  SI sz= ed->get_pixel_size ();
  double sf= ((double) sz) / 256.0;
  rectangle selr= least_upper_bound (sel->rs) / sf;
  rectangle r= translate (selr, wr->x1, wr->y2);
  array<int> ret;
  ret << (r->x1) << (r->y1) << (r->x2) << (r->y2);
  //ret << (r->x1/PIXEL) << (r->y1/PIXEL) << (r->x2/PIXEL) << (r->y2/PIXEL);
  return ret;
}
#endif

bool
supports_native_pdf () {
#ifdef PDF_RENDERER
  return true;
#else
  return false;
#endif
}

bool
supports_ghostscript () {
#ifdef USE_GS
  return true;
#else
  return false;
#endif
}

bool
is_busy_versioning () {
  return busy_versioning;
}

array<SI>
get_screen_size () {
  array<SI> r;
  SI w, h;
//FIXME:  gui_root_extents (w, h);
  r << w << h;
  return r;
}

/******************************************************************************
* Redirections
******************************************************************************/

void
cout_buffer () {
  cout.buffer ();
}

string
cout_unbuffer () {
  return cout.unbuffer ();
}

/******************************************************************************
* Adaptor code
******************************************************************************/

//bool is_extension (tree_label t) { return is_extension (tree(t)); }

void
set_font_rules (scheme_tree_t t) {
  tree rules= t;
  int i, n= arity (rules);
  for (i=0; i<n; i++)
    if (arity (rules [i]) == 2) {
      tree l= (tree) rules[i][0];
      tree r= (tree) rules[i][1];
      font_rule (l, r);
    }
}

/******************************************************************************
* Declarations
******************************************************************************/
//string eval_system (string s);

DECLARE_GLUE_NAME_TYPE(texmacs_version,"texmacs-version-release", string (string))
DECLARE_GLUE_NAME_TYPE(version_inf,"version-before?", bool (string, string))
//DECLARE_GLUE_NAME_TYPE(updater_supported,"updater-supported?", bool ())
//DECLARE_GLUE_NAME_TYPE(updater_is_running,"updater-running?", bool ())
//DECLARE_GLUE_NAME_TYPE(updater_check_background,"updater-check-background", bool ())
//DECLARE_GLUE_NAME_TYPE(updater_check_foreground,"updater-check-foreground", bool ())
//DECLARE_GLUE_NAME_TYPE(updater_last_check,"updater-last-check", long ())
//DECLARE_GLUE_NAME_TYPE(updater_set_interval,"updater-set-interval", bool (int))
DECLARE_GLUE_NAME_TYPE(get_original_path,"get-original-path", string ())
DECLARE_GLUE_NAME_TYPE(os_win32,"os-win32?", bool ())
DECLARE_GLUE_NAME_TYPE(os_mingw,"os-mingw?", bool ())
DECLARE_GLUE_NAME_TYPE(os_macos,"os-macos?", bool ())
//DECLARE_GLUE_NAME_TYPE(has_printing_cmd,"has-printing-cmd?", bool ())
DECLARE_GLUE_NAME_TYPE(gui_is_x,"x-gui?", bool ())
DECLARE_GLUE_NAME_TYPE(gui_is_qt,"qt-gui?", bool ())
//DECLARE_GLUE_NAME_TYPE(gui_version,"gui-version", string ())
//DECLARE_GLUE_NAME_TYPE(default_look_and_feel,"default-look-and-feel", string ())
DECLARE_GLUE_NAME_TYPE(default_chinese_font_name,"default-chinese-font", string ())
DECLARE_GLUE_NAME_TYPE(default_japanese_font_name,"default-japanese-font", string ())
DECLARE_GLUE_NAME_TYPE(default_korean_font_name,"default-korean-font", string ())
DECLARE_GLUE_NAME_TYPE(get_retina_factor,"get-retina-factor", int ())
DECLARE_GLUE_NAME_TYPE(get_retina_zoom,"get-retina-zoom", int ())
DECLARE_GLUE_NAME_TYPE(get_retina_icons,"get-retina-icons", int ())
DECLARE_GLUE_NAME_TYPE(get_retina_scale,"get-retina-scale", double ())
DECLARE_GLUE_NAME_TYPE(set_retina_factor,"set-retina-factor", void (int))
DECLARE_GLUE_NAME_TYPE(set_retina_zoom,"set-retina-zoom", void (int))
DECLARE_GLUE_NAME_TYPE(set_retina_icons,"set-retina-icons", void (int))
DECLARE_GLUE_NAME_TYPE(set_retina_scale,"set-retina-scale", void (double))
DECLARE_GLUE_NAME_TYPE(tm_output,"tm-output", void (string))
DECLARE_GLUE_NAME_TYPE(tm_errput,"tm-errput", void (string))
DECLARE_GLUE_NAME_TYPE(win32_display,"win32-display", void (string))
DECLARE_GLUE_NAME_TYPE(cpp_error,"cpp-error", void ())
DECLARE_GLUE_NAME_TYPE(supports_native_pdf,"supports-native-pdf?", bool ())
DECLARE_GLUE_NAME_TYPE(supports_ghostscript,"supports-ghostscript?", bool ())
//DECLARE_GLUE_NAME_TYPE(in_rescue_mode,"rescue-mode?", bool ())
DECLARE_GLUE_NAME_TYPE(scheme_dialect,"scheme-dialect", string ())
DECLARE_GLUE_NAME_TYPE(get_texmacs_path,"get-texmacs-path", url ())
DECLARE_GLUE_NAME_TYPE(get_texmacs_home_path,"get-texmacs-home-path", url ())
//DECLARE_GLUE_NAME_TYPE(plugin_list,"plugin-list", scheme_tree_t ())
DECLARE_GLUE_NAME_TYPE(set_fast_environments,"set-fast-environments", void (bool))
DECLARE_GLUE_NAME_TYPE(tt_font_exists,"font-exists-in-tt?", bool (string))
DECLARE_GLUE_NAME_BASE(eval_system,"eval-system", string (string))
DECLARE_GLUE_NAME_TYPE(var_eval_system,"var-eval-system", string (string))
//DECLARE_GLUE_NAME_BASE(evaluate_system,"evaluate-system", array_string (array_string, array_int, array_string, array_int))
DECLARE_GLUE_NAME_TYPE(get_locale_language,"get-locale-language", string ())
DECLARE_GLUE_NAME_TYPE(get_locale_charset,"get-locale-charset", string ())
DECLARE_GLUE_NAME_TYPE(locale_to_language,"locale-to-language", string (string))
DECLARE_GLUE_NAME_TYPE(language_to_locale,"language-to-locale", string (string))
DECLARE_GLUE_NAME_TYPE(texmacs_time,"texmacs-time", int ())
DECLARE_GLUE_NAME_TYPE(pretty_time,"pretty-time", string (int))
DECLARE_GLUE_NAME_TYPE(mem_used,"texmacs-memory", int ())
DECLARE_GLUE_NAME_BASE(bench_print,"bench-print", void (string))
DECLARE_GLUE_NAME_BASE(bench_print,"bench-print-all", void ())
//DECLARE_GLUE_NAME_TYPE(system_wait,"system-wait", void (string, string))
//DECLARE_GLUE_NAME_TYPE(get_show_kbd,"get-show-kbd", bool ())
//DECLARE_GLUE_NAME_TYPE(set_show_kbd,"set-show-kbd", void (bool))
//DECLARE_GLUE_NAME_TYPE(set_latex_command,"set-latex-command", void (string))
//DECLARE_GLUE_NAME_TYPE(set_bibtex_command,"set-bibtex-command", void (string))
//DECLARE_GLUE_NAME_TYPE(number_latex_errors,"number-latex-errors", int (url))
//DECLARE_GLUE_NAME_TYPE(number_latex_pages,"number-latex-pages", int (url))
string math_symbol_group_wrap (string s) { return math_symbol_group (s); }
array_string math_group_members_wrap (string s) { return math_group_members (s); }
string math_symbol_type_wrap (string s) { return math_symbol_type (s); }
DECLARE_GLUE_NAME_TYPE(math_symbol_group_wrap,"math-symbol-group", string (string))
DECLARE_GLUE_NAME_TYPE(math_group_members_wrap,"math-group-members", array_string (string))
DECLARE_GLUE_NAME_TYPE(math_symbol_type_wrap,"math-symbol-type", string (string))
DECLARE_GLUE_NAME_TYPE(as_command,"object->command", command (object))
DECLARE_GLUE_NAME_TYPE(exec_delayed,"exec-delayed", void (object))
DECLARE_GLUE_NAME_TYPE(exec_delayed_pause,"exec-delayed-pause", void (object))
//DECLARE_GLUE_NAME_TYPE(protected_call,"protected-call", void (object))
DECLARE_GLUE_NAME_TYPE(notify_preferences_booted,"notify-preferences-booted", void ())
DECLARE_GLUE_NAME_TYPE(has_user_preference,"cpp-has-preference?", bool (string))
DECLARE_GLUE_NAME_TYPE(get_user_preference,"cpp-get-preference", string (string, string))
DECLARE_GLUE_NAME_TYPE(set_user_preference,"cpp-set-preference", void (string, string))
DECLARE_GLUE_NAME_TYPE(reset_user_preference,"cpp-reset-preference", void (string))
DECLARE_GLUE_NAME_TYPE(save_user_preferences,"save-preferences", void ())
//DECLARE_GLUE_NAME_TYPE(get_printing_default,"get-default-printing-command", string ())
DECLARE_GLUE_NAME_TYPE(set_input_language,"set-input-language", void (string))
DECLARE_GLUE_NAME_TYPE(get_input_language,"get-input-language", string ())
//DECLARE_GLUE_NAME_TYPE(gui_set_output_language,"set-output-language", void (string))
DECLARE_GLUE_NAME_TYPE(get_output_language,"get-output-language", string ())
DECLARE_GLUE_NAME_BASE(translate,"translate", string (content))
DECLARE_GLUE_NAME_BASE(translate_as_is,"string-translate", string (string))
DECLARE_GLUE_NAME_BASE(translate,"translate-from-to", string (content, string, string))
DECLARE_GLUE_NAME_BASE(tree_translate,"tree-translate", tree (content))
DECLARE_GLUE_NAME_BASE(tree_translate,"tree-translate-from-to", tree (content, string, string))
DECLARE_GLUE_NAME_TYPE(force_load_dictionary,"force-load-translations", void (string, string))
int named_color_wrap (string s) { return named_color(s); }
DECLARE_GLUE_NAME_TYPE(named_color_wrap,"color", int (string))
DECLARE_GLUE_NAME_BASE(get_hex_color,"get-hex-color", string (string))
DECLARE_GLUE_NAME_TYPE(named_color_to_xcolormap,"named-color->xcolormap", string (string))
DECLARE_GLUE_NAME_TYPE(named_rgb_color,"rgba->named-color", string (array_int))
DECLARE_GLUE_NAME_TYPE(get_named_rgb_color,"named-color->rgba", array_int (string))
DECLARE_GLUE_NAME_TYPE(new_author,"new-author", double ())
DECLARE_GLUE_NAME_TYPE(set_author,"set-author", void (double))
DECLARE_GLUE_NAME_BASE(get_author,"get-author", double ())
DECLARE_GLUE_NAME_TYPE(debug_set,"debug-set", void (string, bool))
DECLARE_GLUE_NAME_TYPE(debug_get,"debug-get", bool (string))
DECLARE_GLUE_NAME_TYPE(debug_message,"debug-message", void (string, string))
DECLARE_GLUE_NAME_TYPE(get_debug_messages,"get-debug-messages", tree (string, int))
DECLARE_GLUE_NAME_BASE(clear_debug_messages,"clear-debug-messages", void ())
DECLARE_GLUE_NAME_TYPE(cout_buffer,"cout-buffer", void ())
DECLARE_GLUE_NAME_TYPE(cout_unbuffer,"cout-unbuffer", string ())
DECLARE_GLUE_NAME_TYPE(new_marker,"mark-new", double ())
DECLARE_GLUE_NAME_TYPE(register_glyph,"glyph-register", void (string, array_array_array_double))
DECLARE_GLUE_NAME_TYPE(recognize_glyph,"glyph-recognize", string (array_array_array_double))
DECLARE_GLUE_NAME_TYPE(set_new_fonts,"set-new-fonts", void (bool))
DECLARE_GLUE_NAME_TYPE(get_new_fonts,"new-fonts?", bool ())
DECLARE_GLUE_NAME_TYPE(eqnumber_to_nonumber,"tmtm-eqnumber->nonumber", tree (tree))
DECLARE_GLUE_NAME_TYPE(is_busy_versioning,"busy-versioning?", bool ())
DECLARE_GLUE_NAME_TYPE(players_set_elapsed,"players-set-elapsed", void (tree, double))
DECLARE_GLUE_NAME_TYPE(players_set_speed,"players-set-speed", void (tree, double))
DECLARE_GLUE_NAME_TYPE(apply_effect,"apply-effect", void (content, array_url, url, int, int))
DECLARE_GLUE_NAME_TYPE(tt_font_exists,"tt-exists?", bool (string))
DECLARE_GLUE_NAME_TYPE(tt_dump,"tt-dump", void (url))
DECLARE_GLUE_NAME_TYPE(tt_font_name,"tt-font-name", scheme_tree_t (url))
DECLARE_GLUE_NAME_TYPE(tt_analyze,"tt-analyze", array_string (string))
DECLARE_GLUE_NAME_TYPE(font_database_build,"font-database-build", void (url))
DECLARE_GLUE_NAME_TYPE(font_database_build_local,"font-database-build-local", void ())
DECLARE_GLUE_NAME_TYPE(font_database_extend_local,"font-database-extend-local", void (url))
DECLARE_GLUE_NAME_BASE(font_database_build_global,"font-database-build-global", void ())
DECLARE_GLUE_NAME_TYPE(font_database_build_characteristics,"font-database-build-characteristics", void (bool))
DECLARE_GLUE_NAME_BASE(font_database_build_global,"font-database-insert-global", void (url))
DECLARE_GLUE_NAME_TYPE(font_database_save_local_delta,"font-database-save-local-delta", void ())
DECLARE_GLUE_NAME_TYPE(font_database_load,"font-database-load", void ())
DECLARE_GLUE_NAME_TYPE(font_database_save,"font-database-save", void ())
DECLARE_GLUE_NAME_TYPE(font_database_filter,"font-database-filter", void ())
DECLARE_GLUE_NAME_TYPE(font_database_families,"font-database-families", array_string ())
DECLARE_GLUE_NAME_TYPE(font_database_delta_families,"font-database-delta-families", array_string ())
DECLARE_GLUE_NAME_TYPE(font_database_styles,"font-database-styles", array_string (string))
DECLARE_GLUE_NAME_BASE(font_database_search,"font-database-search", array_string (string, string))
DECLARE_GLUE_NAME_TYPE(font_database_characteristics,"font-database-characteristics", array_string (string, string))
DECLARE_GLUE_NAME_TYPE(font_database_substitutions,"font-database-substitutions", scheme_tree_t (string))
DECLARE_GLUE_NAME_TYPE(family_to_master,"font-family->master", string (string))
DECLARE_GLUE_NAME_TYPE(master_to_families,"font-master->families", array_string (string))
DECLARE_GLUE_NAME_TYPE(master_features,"font-master-features", array_string (string))
DECLARE_GLUE_NAME_TYPE(family_features,"font-family-features", array_string (string))
DECLARE_GLUE_NAME_TYPE(family_strict_features,"font-family-strict-features", array_string (string))
DECLARE_GLUE_NAME_TYPE(style_features,"font-style-features", array_string (string))
DECLARE_GLUE_NAME_BASE(guessed_features,"font-guessed-features", array_string (string, string))
DECLARE_GLUE_NAME_BASE(guessed_distance,"font-guessed-distance", double (string, string, string, string))
DECLARE_GLUE_NAME_BASE(guessed_distance,"font-master-guessed-distance", double (string, string))
DECLARE_GLUE_NAME_BASE(guessed_features,"font-family-guessed-features", array_string (string, bool))
DECLARE_GLUE_NAME_TYPE(characteristic_distance,"characteristic-distance", double (array_string, array_string))
DECLARE_GLUE_NAME_TYPE(trace_distance,"trace-distance", double (string, string, double))
DECLARE_GLUE_NAME_BASE(logical_font,"logical-font-public", array_string (string, string))
DECLARE_GLUE_NAME_TYPE(logical_font_exact,"logical-font-exact", array_string (string, string))
DECLARE_GLUE_NAME_BASE(logical_font,"logical-font-private", array_string (string, string, string, string))
DECLARE_GLUE_NAME_TYPE(get_family,"logical-font-family", string (array_string))
DECLARE_GLUE_NAME_TYPE(get_variant,"logical-font-variant", string (array_string))
DECLARE_GLUE_NAME_TYPE(get_series,"logical-font-series", string (array_string))
DECLARE_GLUE_NAME_TYPE(get_shape,"logical-font-shape", string (array_string))
array_string search_font_wrap (array_string as) { return search_font(as); }
DECLARE_GLUE_NAME_TYPE(search_font_wrap,"logical-font-search", array_string (array_string))
DECLARE_GLUE_NAME_TYPE(search_font_exact,"logical-font-search-exact", array_string (array_string))
DECLARE_GLUE_NAME_TYPE(search_font_families,"search-font-families", array_string (array_string))
DECLARE_GLUE_NAME_TYPE(search_font_styles,"search-font-styles", array_string (string, array_string))
array_string patch_font_wrap (array_string as, array_string as2) { return patch_font(as, as2); }
DECLARE_GLUE_NAME_TYPE(patch_font_wrap,"logical-font-patch", array_string (array_string, array_string))
DECLARE_GLUE_NAME_TYPE(apply_substitutions,"logical-font-substitute", array_string (array_string))
DECLARE_GLUE_NAME_TYPE(main_family,"font-family-main", string (string))
//DECLARE_GLUE_NAME_TYPE(image_to_psdoc,"image->psdoc", string (url))
DECLARE_GLUE_NAME_TYPE(get_control_times,"anim-control-times", array_double (content))
DECLARE_GLUE_NAME_TYPE(tree_to_scheme_tree,"tree->stree", scheme_tree_t (tree))
tree scheme_tree_to_tree_wrap (scheme_tree_t t) { return scheme_tree_to_tree (t); };
DECLARE_GLUE_NAME_TYPE(scheme_tree_to_tree_wrap,"stree->tree", tree (scheme_tree_t))
DECLARE_GLUE_NAME_TYPE(coerce_tree_string,"tree->string", string (tree))
DECLARE_GLUE_NAME_TYPE(coerce_string_tree,"string->tree", tree (string))
tree tm_to_tree (content t) { return tree (t); }
DECLARE_GLUE_NAME_TYPE(tm_to_tree,"tm->tree", tree (content))
DECLARE_GLUE_NAME_BASE(is_atomic,"tree-atomic?", bool (tree))
DECLARE_GLUE_NAME_BASE(is_compound,"tree-compound?", bool (tree))
DECLARE_GLUE_NAME_BASE(L,"tree-label", tree_label (tree))
DECLARE_GLUE_NAME_BASE(A,"tree-children", array_tree (tree))
DECLARE_GLUE_NAME_BASE(N,"tree-arity", int (tree))
DECLARE_GLUE_NAME_TYPE(tree_ref,"tree-child-ref", tree (tree, int))
DECLARE_GLUE_NAME_TYPE(tree_set,"tree-child-set!", tree (tree, int, content))
DECLARE_GLUE_NAME_TYPE(tree_child_insert,"tree-child-insert", tree (content, int, content))
DECLARE_GLUE_NAME_TYPE(obtain_ip,"tree-ip", path (tree))
DECLARE_GLUE_NAME_TYPE(tree_active,"tree-active?", bool (tree))
DECLARE_GLUE_NAME_BASE(strong_equal,"tree-eq?", bool (tree, tree))
DECLARE_GLUE_NAME_TYPE(subtree,"subtree", tree (tree, path))
DECLARE_GLUE_NAME_TYPE(tree_range,"tree-range", tree (tree, int, int))
DECLARE_GLUE_NAME_BASE(copy,"tree-copy", tree (tree))
DECLARE_GLUE_NAME_TYPE(tree_append,"tree-append", tree (tree, tree))
DECLARE_GLUE_NAME_TYPE(right_index,"tree-right-index", int (tree))
DECLARE_GLUE_NAME_TYPE_BASE(is_extension,"tree-label-extension?", bool (tree_label), bool (tree))
DECLARE_GLUE_NAME_TYPE(is_macro,"tree-label-macro?", bool (tree_label))
DECLARE_GLUE_NAME_TYPE(is_parameter,"tree-label-parameter?", bool (tree_label))
DECLARE_GLUE_NAME_TYPE(get_tag_type,"tree-label-type", string (tree_label))
DECLARE_GLUE_NAME_TYPE(is_multi_paragraph,"tree-multi-paragraph?", bool (tree))
DECLARE_GLUE_NAME_TYPE(simplify_correct,"tree-simplify", tree (tree))
DECLARE_GLUE_NAME_BASE(minimal_arity,"tree-minimal-arity", int (tree))
DECLARE_GLUE_NAME_BASE(maximal_arity,"tree-maximal-arity", int (tree))
DECLARE_GLUE_NAME_BASE(correct_arity,"tree-possible-arity?", bool (tree, int))
DECLARE_GLUE_NAME_TYPE(insert_point,"tree-insert_point", int (tree, int))
DECLARE_GLUE_NAME_TYPE(is_dynamic,"tree-is-dynamic?", bool (tree))
DECLARE_GLUE_NAME_TYPE(is_accessible_child,"tree-accessible-child?", bool (tree, int))
DECLARE_GLUE_NAME_TYPE(accessible_children,"tree-accessible-children", array_tree (tree))
DECLARE_GLUE_NAME_TYPE(all_accessible,"tree-all-accessible?", bool (content))
DECLARE_GLUE_NAME_TYPE(none_accessible,"tree-none-accessible?", bool (content))
DECLARE_GLUE_NAME_TYPE(get_name,"tree-name", string (content))
DECLARE_GLUE_NAME_TYPE(get_long_name,"tree-long-name", string (content))
DECLARE_GLUE_NAME_TYPE(get_child_name,"tree-child-name", string (content, int))
DECLARE_GLUE_NAME_TYPE(get_child_long_name,"tree-child-long-name", string (content, int))
DECLARE_GLUE_NAME_TYPE(get_child_type,"tree-child-type", string (content, int))
DECLARE_GLUE_NAME_BASE(get_env_child,"tree-child-env*", tree (content, int, content))
DECLARE_GLUE_NAME_BASE(get_env_child,"tree-child-env", tree (content, int, string, content))
DECLARE_GLUE_NAME_BASE(get_env_descendant,"tree-descendant-env*", tree (content, path, content))
DECLARE_GLUE_NAME_BASE(get_env_descendant,"tree-descendant-env", tree (content, path, string, content))
DECLARE_GLUE_NAME_TYPE(load_inclusion,"tree-load-inclusion", tree (url))
DECLARE_GLUE_NAME_TYPE(tree_as_string,"tree-as-string", string (content))
//DECLARE_GLUE_NAME_TYPE(tree_extents,"tree-extents", tree (content))
DECLARE_GLUE_NAME_BASE(is_empty,"tree-empty?", bool (content))
DECLARE_GLUE_NAME_TYPE(is_multi_line,"tree-multi-line?", bool (content))
DECLARE_GLUE_NAME_TYPE(admits_edit_observer,"tree-is-buffer?", bool (tree))
DECLARE_GLUE_NAME_TYPE(search_sections,"tree-search-sections", array_tree (tree))
DECLARE_GLUE_NAME_BASE(search,"tree-search-tree", array_path (content, content, path, int))
DECLARE_GLUE_NAME_BASE(search,"tree-search-tree-at", array_path (content, content, path, path, int))
//DECLARE_GLUE_NAME_BASE(spell,"tree-spell", array_path (string, content, path, int))
//DECLARE_GLUE_NAME_BASE(spell,"tree-spell-at", array_path (string, content, path, path, int))
//DECLARE_GLUE_NAME_BASE(spell,"tree-spell-selection", array_path (string, content, path, path, path, int))
DECLARE_GLUE_NAME_TYPE(previous_search_hit,"previous-search-hit", array_path (array_path, path, bool))
DECLARE_GLUE_NAME_TYPE(next_search_hit,"next-search-hit", array_path (array_path, path, bool))
DECLARE_GLUE_NAME_TYPE(navigate_search_hit,"navigate-search-hit", array_path (path, bool, bool, bool))
DECLARE_GLUE_NAME_BASE(minimal_arity,"tag-minimal-arity", int (tree_label))
DECLARE_GLUE_NAME_BASE(maximal_arity,"tag-maximal-arity", int (tree_label))
DECLARE_GLUE_NAME_BASE(correct_arity,"tag-possible-arity?", bool (tree_label, int))
DECLARE_GLUE_NAME_TYPE(set_access_mode,"set-access-mode", int (int))
DECLARE_GLUE_NAME_TYPE(get_access_mode,"get-access-mode", int ())
DECLARE_GLUE_NAME_TYPE(tree_assign,"tree-assign", tree (tree, content))
DECLARE_GLUE_NAME_TYPE(tree_insert,"tree-var-insert", tree (tree, int, content))
DECLARE_GLUE_NAME_TYPE(tree_remove,"tree-remove", tree (tree, int, int))
DECLARE_GLUE_NAME_TYPE(tree_split,"tree-split", tree (tree, int, int))
DECLARE_GLUE_NAME_TYPE(tree_join,"tree-join", tree (tree, int))
DECLARE_GLUE_NAME_TYPE(tree_assign_node,"tree-assign-node", tree (tree, tree_label))
DECLARE_GLUE_NAME_TYPE(tree_insert_node,"tree-insert-node", tree (tree, int, content))
DECLARE_GLUE_NAME_TYPE(tree_remove_node,"tree-remove-node", tree (tree, int))
DECLARE_GLUE_NAME_TYPE(correct_node,"cpp-tree-correct-node", void (tree))
DECLARE_GLUE_NAME_TYPE(correct_downwards,"cpp-tree-correct-downwards", void (tree))
DECLARE_GLUE_NAME_TYPE(correct_upwards,"cpp-tree-correct-upwards", void (tree))
DECLARE_GLUE_NAME_TYPE(concat_tokenize,"concat-tokenize-math", array_tree (content))
DECLARE_GLUE_NAME_TYPE(concat_decompose,"concat-decompose", array_tree (content))
DECLARE_GLUE_NAME_TYPE(concat_recompose,"concat-recompose", tree (array_tree))
DECLARE_GLUE_NAME_TYPE(is_with_like,"with-like?", bool (content))
DECLARE_GLUE_NAME_TYPE(with_same_type,"with-same-type?", bool (content, content))
DECLARE_GLUE_NAME_TYPE(with_similar_type,"with-similar-type?", bool (content, content))
DECLARE_GLUE_NAME_TYPE(with_correct,"with-correct", tree (content))
DECLARE_GLUE_NAME_TYPE(superfluous_with_correct,"with-correct-superfluous", tree (content))
DECLARE_GLUE_NAME_TYPE(superfluous_invisible_correct,"invisible-correct-superfluous", tree (content))
DECLARE_GLUE_NAME_TYPE(missing_invisible_correct,"invisible-correct-missing", tree (content, int))
DECLARE_GLUE_NAME_TYPE(automatic_correct,"automatic-correct", tree (content, string))
DECLARE_GLUE_NAME_TYPE(manual_correct,"manual-correct", tree (content))
DECLARE_GLUE_NAME_TYPE(upgrade_brackets,"tree-upgrade-brackets", tree (content, string))
DECLARE_GLUE_NAME_TYPE(upgrade_big,"tree-upgrade-big", tree (content))
DECLARE_GLUE_NAME_TYPE(downgrade_brackets,"tree-downgrade-brackets", tree (content, bool, bool))
DECLARE_GLUE_NAME_TYPE(downgrade_big,"tree-downgrade-big", tree (content))
DECLARE_GLUE_NAME_TYPE(math_status_print,"math-status-print", void ())
DECLARE_GLUE_NAME_TYPE(math_status_reset,"math-status-reset", void ())
DECLARE_GLUE_NAME_TYPE(compile_stats,"math-stats-compile", void (string, content, string))
DECLARE_GLUE_NAME_TYPE(number_occurrences,"math-stats-occurrences", int (string, content))
DECLARE_GLUE_NAME_TYPE(number_in_role,"math-stats-number-in-role", int (string, content))
DECLARE_GLUE_NAME_TYPE(strip,"path-strip", path (path, path))
DECLARE_GLUE_NAME_TYPE(path_inf,"path-inf?", bool (path, path))
DECLARE_GLUE_NAME_TYPE(path_inf_eq,"path-inf-eq?", bool (path, path))
DECLARE_GLUE_NAME_TYPE(path_less,"path-less?", bool (path, path))
DECLARE_GLUE_NAME_TYPE(path_less_eq,"path-less-eq?", bool (path, path))
DECLARE_GLUE_NAME_BASE(start,"path-start", path (content, path))
DECLARE_GLUE_NAME_BASE(end,"path-end", path (content, path))
DECLARE_GLUE_NAME_TYPE(next_valid,"path-next", path (content, path))
DECLARE_GLUE_NAME_TYPE(previous_valid,"path-previous", path (content, path))
DECLARE_GLUE_NAME_TYPE(next_word,"path-next-word", path (content, path))
DECLARE_GLUE_NAME_TYPE(previous_word,"path-previous-word", path (content, path))
DECLARE_GLUE_NAME_TYPE(next_node,"path-next-node", path (content, path))
DECLARE_GLUE_NAME_TYPE(previous_node,"path-previous-node", path (content, path))
DECLARE_GLUE_NAME_TYPE(next_tag,"path-next-tag", path (content, path, scheme_tree_t))
DECLARE_GLUE_NAME_TYPE(previous_tag,"path-previous-tag", path (content, path, scheme_tree_t))
DECLARE_GLUE_NAME_TYPE(next_tag_same_argument,"path-next-tag-same-argument", path (content, path, scheme_tree_t))
DECLARE_GLUE_NAME_TYPE(previous_tag_same_argument,"path-previous-tag-same-argument", path (content, path, scheme_tree_t))
DECLARE_GLUE_NAME_TYPE(next_argument,"path-next-argument", path (content, path))
DECLARE_GLUE_NAME_TYPE(previous_argument,"path-previous-argument", path (content, path))
DECLARE_GLUE_NAME_TYPE(previous_section,"path-previous-section", path (content, path))
DECLARE_GLUE_NAME_TYPE(make_modification,"make-modification", modification (string, path, content))
DECLARE_GLUE_NAME_TYPE(mod_assign,"modification-assign", modification (path, content))
DECLARE_GLUE_NAME_TYPE(mod_insert,"modification-insert", modification (path, int, content))
DECLARE_GLUE_NAME_TYPE(mod_remove,"modification-remove", modification (path, int, int))
DECLARE_GLUE_NAME_TYPE(mod_split,"modification-split", modification (path, int, int))
DECLARE_GLUE_NAME_TYPE(mod_join,"modification-join", modification (path, int))
DECLARE_GLUE_NAME_TYPE(mod_assign_node,"modification-assign-node", modification (path, tree_label))
DECLARE_GLUE_NAME_TYPE(mod_insert_node,"modification-insert-node", modification (path, int, content))
DECLARE_GLUE_NAME_TYPE(mod_remove_node,"modification-remove-node", modification (path, int))
DECLARE_GLUE_NAME_TYPE(mod_set_cursor,"modification-set-cursor", modification (path, int, content))
DECLARE_GLUE_NAME_BASE(get_type,"modification-kind", string (modification))
DECLARE_GLUE_NAME_TYPE(get_path,"modification-path", path (modification))
DECLARE_GLUE_NAME_TYPE(get_tree,"modification-tree", tree (modification))
DECLARE_GLUE_NAME_TYPE(root,"modification-root", path (modification))
DECLARE_GLUE_NAME_BASE(index,"modification-index", int (modification))
DECLARE_GLUE_NAME_TYPE(argument,"modification-argument", int (modification))
DECLARE_GLUE_NAME_BASE(L,"modification-label", tree_label (modification))
DECLARE_GLUE_NAME_BASE(copy,"modification-copy", modification (modification))
DECLARE_GLUE_NAME_BASE(is_applicable,"modification-applicable?", bool (content, modification))
DECLARE_GLUE_NAME_TYPE_BASE(var_clean_apply,"modification-apply", tree (content, modification), tree (tree&, modification))
DECLARE_GLUE_NAME_TYPE_BASE(var_apply,"modification-inplace-apply", tree (tree, modification), tree (tree&, modification))
DECLARE_GLUE_NAME_BASE(invert,"modification-invert", modification (modification, content))
DECLARE_GLUE_NAME_BASE(commute,"modification-commute?", bool (modification, modification))
DECLARE_GLUE_NAME_BASE(can_pull,"modification-can-pull?", bool (modification, modification))
DECLARE_GLUE_NAME_BASE(pull,"modification-pull", modification (modification, modification))
DECLARE_GLUE_NAME_BASE(co_pull,"modification-co-pull", modification (modification, modification))
patch patch_wrap (modification m1, modification m2) { return patch(m1, m2); }
patch patch_wrap (array_patch m) { return patch(m); }
patch patch_wrap (double a, bool b) { return patch(a, b); }
patch patch_wrap (double a, patch b) { return patch(a, b); }
DECLARE_GLUE_NAME_BASE(patch_wrap,"patch-pair", patch (modification, modification))
DECLARE_GLUE_NAME_BASE(patch_wrap,"patch-compound", patch (array_patch))
DECLARE_GLUE_NAME_TYPE(branch_patch,"patch-branch", patch (array_patch))
DECLARE_GLUE_NAME_BASE(patch_wrap,"patch-birth", patch (double, bool))
DECLARE_GLUE_NAME_BASE(patch_wrap,"patch-author", patch (double, patch))
DECLARE_GLUE_NAME_BASE(is_modification,"patch-pair?", bool (patch))
DECLARE_GLUE_NAME_BASE(is_compound,"patch-compound?", bool (patch))
DECLARE_GLUE_NAME_TYPE(is_branch,"patch-branch?", bool (patch))
DECLARE_GLUE_NAME_TYPE(is_birth,"patch-birth?", bool (patch))
DECLARE_GLUE_NAME_TYPE(is_author,"patch-author?", bool (patch))
DECLARE_GLUE_NAME_BASE(N,"patch-arity", int (patch))
DECLARE_GLUE_NAME_BASE(access,"patch-ref", patch (patch, int))
DECLARE_GLUE_NAME_TYPE(get_modification,"patch-direct", modification (patch))
DECLARE_GLUE_NAME_TYPE(get_inverse,"patch-inverse", modification (patch))
DECLARE_GLUE_NAME_TYPE(get_birth,"patch-get-birth", bool (patch))
DECLARE_GLUE_NAME_BASE(get_author,"patch-get-author", double (patch))
DECLARE_GLUE_NAME_BASE(copy,"patch-copy", patch (patch))
DECLARE_GLUE_NAME_BASE(is_applicable,"patch-applicable?", bool (patch, content))
DECLARE_GLUE_NAME_BASE(var_clean_apply,"patch-apply", tree (content, patch))
DECLARE_GLUE_NAME_TYPE_BASE(var_apply,"patch-inplace-apply", tree (tree, patch), tree (tree&, patch))
DECLARE_GLUE_NAME_TYPE(compactify,"patch-compactify", patch (patch))
DECLARE_GLUE_NAME_TYPE(cursor_hint,"patch-cursor-hint", path (patch, content))
DECLARE_GLUE_NAME_BASE(invert,"patch-invert", patch (patch, content))
DECLARE_GLUE_NAME_BASE(commute,"patch-commute?", bool (patch, patch))
DECLARE_GLUE_NAME_BASE(can_pull,"patch-can-pull?", bool (patch, patch))
DECLARE_GLUE_NAME_BASE(pull,"patch-pull", patch (patch, patch))
DECLARE_GLUE_NAME_BASE(co_pull,"patch-co-pull", patch (patch, patch))
DECLARE_GLUE_NAME_TYPE(remove_set_cursor,"patch-remove-set-cursor", patch (patch))
DECLARE_GLUE_NAME_TYPE(does_modify,"patch-modifies?", bool (patch))
DECLARE_GLUE_NAME_TYPE(get_ids,"tree->ids", list_string (tree))
DECLARE_GLUE_NAME_TYPE(get_trees,"id->trees", list_tree (string))
DECLARE_GLUE_NAME_TYPE(get_links,"vertex->links", list_tree (content))
DECLARE_GLUE_NAME_TYPE(tree_pointer_new,"tree->tree-pointer", observer (tree))
DECLARE_GLUE_NAME_TYPE(tree_pointer_delete,"tree-pointer-detach", void (observer))
DECLARE_GLUE_NAME_TYPE(obtain_tree,"tree-pointer->tree", tree (observer))
DECLARE_GLUE_NAME_TYPE(all_link_types,"current-link-types", list_string ())
DECLARE_GLUE_NAME_TYPE(get_locus_rendering,"get-locus-rendering", string (string))
DECLARE_GLUE_NAME_TYPE(set_locus_rendering,"set-locus-rendering", void (string, string))
DECLARE_GLUE_NAME_TYPE(declare_visited,"declare-visited", void (string))
DECLARE_GLUE_NAME_TYPE(has_been_visited,"has-been-visited?", bool (string))
DECLARE_GLUE_NAME_TYPE(set_graphical_value,"graphics-set", void (content, content))
DECLARE_GLUE_NAME_TYPE(has_graphical_value,"graphics-has?", bool (content))
DECLARE_GLUE_NAME_TYPE(get_graphical_value,"graphics-ref", tree (content))
DECLARE_GLUE_NAME_TYPE(graphics_needs_update,"graphics-needs-update?", bool ())
DECLARE_GLUE_NAME_TYPE(graphics_notify_update,"graphics-notify-update", void (content))
DECLARE_GLUE_NAME_BASE(is_double,"cpp-string-number?", bool (string))
DECLARE_GLUE_NAME_TYPE(occurs,"string-occurs?", bool (string, string))
DECLARE_GLUE_NAME_TYPE(count_occurrences,"string-count-occurrences", int (string, string))
DECLARE_GLUE_NAME_BASE(search_forwards,"string-search-forwards", int (string, int, string))
DECLARE_GLUE_NAME_BASE(search_backwards,"string-search-backwards", int (string, int, string))
DECLARE_GLUE_NAME_TYPE(overlapping,"string-overlapping", int (string, string))
DECLARE_GLUE_NAME_BASE(replace,"string-replace", string (string, string, string))
DECLARE_GLUE_NAME_TYPE(find_non_alpha,"string-find-non-alpha", int (string, int, bool))
DECLARE_GLUE_NAME_BASE(is_alpha,"string-alpha?", bool (string))
DECLARE_GLUE_NAME_TYPE(is_locase_alpha,"string-locase-alpha?", bool (string))
DECLARE_GLUE_NAME_TYPE(upcase_first,"upcase-first", string (string))
DECLARE_GLUE_NAME_TYPE(locase_first,"locase-first", string (string))
DECLARE_GLUE_NAME_TYPE(upcase_all,"upcase-all", string (string))
DECLARE_GLUE_NAME_TYPE(locase_all,"locase-all", string (string))
DECLARE_GLUE_NAME_TYPE(string_union,"string-union", string (string, string))
DECLARE_GLUE_NAME_TYPE(string_minus,"string-minus", string (string, string))
DECLARE_GLUE_NAME_TYPE(escape_generic,"escape-generic", string (string))
DECLARE_GLUE_NAME_TYPE(escape_verbatim,"escape-verbatim", string (string))
DECLARE_GLUE_NAME_TYPE(escape_sh,"escape-shell", string (string))
DECLARE_GLUE_NAME_TYPE(cork_to_ascii,"escape-to-ascii", string (string))
DECLARE_GLUE_NAME_TYPE(unescape_guile,"unescape-guile", string (string))
DECLARE_GLUE_NAME_TYPE(scm_quote,"string-quote", string (string))
DECLARE_GLUE_NAME_TYPE(scm_unquote,"string-unquote", string (string))
DECLARE_GLUE_NAME_BASE(trim_spaces_left,"string-trim-spaces-left", string (string))
DECLARE_GLUE_NAME_BASE(trim_spaces_right,"string-trim-spaces-right", string (string))
DECLARE_GLUE_NAME_BASE(trim_spaces,"string-trim-spaces", string (string))
DECLARE_GLUE_NAME_TYPE(downgrade_math_letters,"downgrade-math-letters", string (string))
DECLARE_GLUE_NAME_TYPE(convert,"string-convert", string (string, string, string))
DECLARE_GLUE_NAME_TYPE(encode_base64,"encode-base64", string (string))
DECLARE_GLUE_NAME_TYPE(decode_base64,"decode-base64", string (string))
DECLARE_GLUE_NAME_TYPE(sourcecode_to_cork,"sourcecode->cork", string (string))
DECLARE_GLUE_NAME_TYPE(cork_to_sourcecode,"cork->sourcecode", string (string))
DECLARE_GLUE_NAME_TYPE(utf8_to_cork,"utf8->cork", string (string))
DECLARE_GLUE_NAME_TYPE(cork_to_utf8,"cork->utf8", string (string))
DECLARE_GLUE_NAME_TYPE(utf8_to_t2a,"utf8->t2a", string (string))
DECLARE_GLUE_NAME_TYPE(t2a_to_utf8,"t2a->utf8", string (string))
DECLARE_GLUE_NAME_TYPE(utf8_to_html,"utf8->html", string (string))
DECLARE_GLUE_NAME_TYPE(guess_wencoding,"guess-wencoding", string (string))
DECLARE_GLUE_NAME_TYPE(tm_to_xml_name,"tm->xml-name", string (string))
DECLARE_GLUE_NAME_TYPE(old_tm_to_xml_cdata,"old-tm->xml-cdata", string (string))
DECLARE_GLUE_NAME_TYPE(tm_to_xml_cdata,"tm->xml-cdata", object (string))
DECLARE_GLUE_NAME_TYPE(xml_name_to_tm,"xml-name->tm", string (string))
DECLARE_GLUE_NAME_TYPE(old_xml_cdata_to_tm,"old-xml-cdata->tm", string (string))
DECLARE_GLUE_NAME_TYPE(xml_unspace,"xml-unspace", string (string, bool, bool))
DECLARE_GLUE_NAME_BASE(as_hexadecimal,"integer->hexadecimal", string (int))
DECLARE_GLUE_NAME_BASE(as_hexadecimal,"integer->padded-hexadecimal", string (int, int))
DECLARE_GLUE_NAME_TYPE(from_hexadecimal,"hexadecimal->integer", int (string))
DECLARE_GLUE_NAME_TYPE(tokenize,"cpp-string-tokenize", array_string (string, string))
DECLARE_GLUE_NAME_TYPE(recompose,"cpp-string-recompose", string (array_string, string))
DECLARE_GLUE_NAME_TYPE(differences,"string-differences", array_int (string, string))
DECLARE_GLUE_NAME_TYPE(distance,"string-distance", int (string, string))
DECLARE_GLUE_NAME_TYPE(find_left_bracket,"find-left-bracket", path (path, string, string))
DECLARE_GLUE_NAME_TYPE(find_right_bracket,"find-right-bracket", path (path, string, string))
DECLARE_GLUE_NAME_TYPE(tm_encode,"string->tmstring", string (string))
DECLARE_GLUE_NAME_TYPE(tm_decode,"tmstring->string", string (string))
DECLARE_GLUE_NAME_TYPE(tm_string_length,"tmstring-length", int (string))
DECLARE_GLUE_NAME_TYPE(tm_forward_access,"tmstring-ref", string (string, int))
DECLARE_GLUE_NAME_TYPE(tm_backward_access,"tmstring-reverse-ref", string (string, int))
DECLARE_GLUE_NAME_TYPE(tm_tokenize,"tmstring->list", array_string (string))
DECLARE_GLUE_NAME_TYPE(tm_recompose,"list->tmstring", string (array_string))
DECLARE_GLUE_NAME_TYPE(tm_char_next,"string-next", int (string, int))
DECLARE_GLUE_NAME_TYPE(tm_char_previous,"string-previous", int (string, int))
DECLARE_GLUE_NAME_TYPE(tm_string_split,"tmstring-split", array_string (string))
DECLARE_GLUE_NAME_TYPE(uni_translit,"tmstring-translit", string (string))
DECLARE_GLUE_NAME_TYPE(uni_locase_first,"tmstring-locase-first", string (string))
DECLARE_GLUE_NAME_TYPE(uni_upcase_first,"tmstring-upcase-first", string (string))
DECLARE_GLUE_NAME_TYPE(uni_locase_all,"tmstring-locase-all", string (string))
DECLARE_GLUE_NAME_TYPE(uni_upcase_all,"tmstring-upcase-all", string (string))
DECLARE_GLUE_NAME_TYPE(uni_unaccent_all,"tmstring-unaccent-all", string (string))
DECLARE_GLUE_NAME_TYPE(uni_is_letter,"tmstring-letter?", bool (string))
DECLARE_GLUE_NAME_TYPE(uni_before,"tmstring-before?", bool (string, string))
//DECLARE_GLUE_NAME_BASE(spell_start,"multi-spell-start", void ())
//DECLARE_GLUE_NAME_BASE(spell_done,"multi-spell-done", void ())
//DECLARE_GLUE_NAME_BASE(spell_start,"single-spell-start", string (string))
//DECLARE_GLUE_NAME_BASE(spell_done,"single-spell-done", void (string))
//DECLARE_GLUE_NAME_TYPE(spell_check,"spell-check", tree (string, string))
//DECLARE_GLUE_NAME_TYPE(check_word,"spell-check?", bool (string, string))
//void spell_accept_wrap(string s1, string s2) { spell_accept(s1, s2); }
//DECLARE_GLUE_NAME_TYPE(spell_accept_wrap,"spell-accept", void (string, string))
//DECLARE_GLUE_NAME_TYPE(spell_accept,"spell-var-accept", void (string, string, bool))
//DECLARE_GLUE_NAME_TYPE(spell_insert,"spell-insert", void (string, string))
//DECLARE_GLUE_NAME_TYPE(packrat_define,"packrat-define", void (string, string, tree))
//DECLARE_GLUE_NAME_TYPE(packrat_property,"packrat-property", void (string, string, string, string))
//DECLARE_GLUE_NAME_TYPE(packrat_inherit,"packrat-inherit", void (string, string))
//DECLARE_GLUE_NAME_TYPE(packrat_parse,"packrat-parse", path (string, string, content))
//DECLARE_GLUE_NAME_TYPE(packrat_correct,"packrat-correct?", bool (string, string, content))
//DECLARE_GLUE_NAME_TYPE(packrat_context,"packrat-context", object (string, string, content, path))
//DECLARE_GLUE_NAME_TYPE(initialize_color_decodings,"syntax-read-preferences", void (string))
DECLARE_GLUE_NAME_TYPE(texmacs_document_to_tree,"parse-texmacs", tree (string))
DECLARE_GLUE_NAME_TYPE(tree_to_texmacs,"serialize-texmacs", string (tree))
DECLARE_GLUE_NAME_TYPE(texmacs_to_tree,"parse-texmacs-snippet", tree (string))
DECLARE_GLUE_NAME_TYPE(tree_to_texmacs,"serialize-texmacs-snippet", string (tree))
DECLARE_GLUE_NAME_TYPE(tree_to_scheme,"texmacs->stm", string (tree))
DECLARE_GLUE_NAME_TYPE(scheme_document_to_tree,"stm->texmacs", tree (string))
DECLARE_GLUE_NAME_TYPE(scheme_to_tree,"stm-snippet->texmacs", tree (string))
//DECLARE_GLUE_NAME_TYPE(tree_to_verbatim,"cpp-texmacs->verbatim", string (tree, bool, string))
//DECLARE_GLUE_NAME_TYPE(verbatim_to_tree,"cpp-verbatim-snippet->texmacs", tree (string, bool, string))
//DECLARE_GLUE_NAME_TYPE(verbatim_document_to_tree,"cpp-verbatim->texmacs", tree (string, bool, string))
//tree parse_latex_wrap(string s) { return parse_latex(s); }
//DECLARE_GLUE_NAME_TYPE(parse_latex_wrap,"parse-latex", tree (string))
//tree parse_latex_document_wrap(string s) { return parse_latex_document(s); }
//DECLARE_GLUE_NAME_TYPE(parse_latex_document_wrap,"parse-latex-document", tree (string))
//DECLARE_GLUE_NAME_TYPE(latex_to_tree,"latex->texmacs", tree (tree))
//DECLARE_GLUE_NAME_TYPE(latex_document_to_tree,"cpp-latex-document->texmacs", tree (string, bool))
//DECLARE_GLUE_NAME_TYPE(latex_class_document_to_tree,"latex-class-document->texmacs", tree (string))
//DECLARE_GLUE_NAME_TYPE(tracked_latex_to_texmacs,"tracked-latex->texmacs", tree (string, bool))
//DECLARE_GLUE_NAME_TYPE(conservative_texmacs_to_latex,"conservative-texmacs->latex", string (content, object))
//DECLARE_GLUE_NAME_TYPE(tracked_texmacs_to_latex,"tracked-texmacs->latex", string (content, object))
//DECLARE_GLUE_NAME_TYPE(conservative_latex_to_texmacs,"conservative-latex->texmacs", tree (string, bool))
//DECLARE_GLUE_NAME_TYPE(get_line_number,"get-line-number", int (string, int))
//DECLARE_GLUE_NAME_TYPE(get_column_number,"get-column-number", int (string, int))
//DECLARE_GLUE_NAME_TYPE(try_latex_export,"try-latex-export", tree (content, object, url, url))
//DECLARE_GLUE_NAME_TYPE(parse_xml,"parse-xml", scheme_tree_t (string))
//DECLARE_GLUE_NAME_TYPE(parse_html,"parse-html", scheme_tree_t (string))
//DECLARE_GLUE_NAME_TYPE(parse_bib,"parse-bib", tree (string))
//DECLARE_GLUE_NAME_TYPE(conservative_bib_import,"conservative-bib-import", tree (string, content, string))
//DECLARE_GLUE_NAME_TYPE(conservative_bib_export,"conservative-bib-export", string (content, string, content))
//DECLARE_GLUE_NAME_TYPE(clean_html,"clean-html", tree (content))
//DECLARE_GLUE_NAME_TYPE(tmml_upgrade,"upgrade-tmml", tree (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(upgrade_mathml,"upgrade-mathml", tree (content))
//DECLARE_GLUE_NAME_TYPE(retrieve_mathjax,"retrieve-mathjax", tree (int))
//DECLARE_GLUE_NAME_TYPE(vernac_to_tree,"vernac->texmacs", tree (string))
//DECLARE_GLUE_NAME_TYPE(vernac_document_to_tree,"vernac-document->texmacs", tree (string))
//DECLARE_GLUE_NAME_BASE(compute_keys,"compute-keys-string", array_string (string, string))
//DECLARE_GLUE_NAME_BASE(compute_keys,"compute-keys-tree", array_string (content, string))
//DECLARE_GLUE_NAME_BASE(compute_keys,"compute-keys-url", array_string (url))
//DECLARE_GLUE_NAME_BASE(compute_index,"compute-index-string", scheme_tree_t (string, string))
//DECLARE_GLUE_NAME_BASE(compute_index,"compute-index-tree", scheme_tree_t (content, string))
//DECLARE_GLUE_NAME_BASE(compute_index,"compute-index-url", scheme_tree_t (url))
url url_wrap (url u) { return url(u); }
url url_wrap (string s) { return url(s); }
url url_wrap (string s, string s2) { return url(s, s2); }
DECLARE_GLUE_NAME_BASE(url_wrap,"url->url", url (url))
DECLARE_GLUE_NAME_TYPE(url_root,"root->url", url (string))
DECLARE_GLUE_NAME_BASE(url_wrap,"string->url", url (string))
string as_string_wrap (url u) { return as_string(u); }
DECLARE_GLUE_NAME_TYPE(as_string_wrap,"url->string", string (url))
DECLARE_GLUE_NAME_TYPE_BASE(as_tree,"url->stree", scheme_tree_t (url), scheme_tree (url))
DECLARE_GLUE_NAME_BASE(url_system,"system->url", url (string))
DECLARE_GLUE_NAME_TYPE(as_system_string,"url->system", string (url))
DECLARE_GLUE_NAME_BASE(url_unix,"unix->url", url (string))
DECLARE_GLUE_NAME_TYPE(as_unix_string,"url->unix", string (url))
DECLARE_GLUE_NAME_BASE(url_wrap,"url-unix", url (string, string))
DECLARE_GLUE_NAME_BASE(url_none,"url-none", url ())
DECLARE_GLUE_NAME_BASE(url_wildcard,"url-any", url ())
DECLARE_GLUE_NAME_BASE(url_wildcard,"url-wildcard", url (string))
DECLARE_GLUE_NAME_TYPE(url_pwd,"url-pwd", url ())
DECLARE_GLUE_NAME_BASE(url_parent,"url-parent", url ())
DECLARE_GLUE_NAME_TYPE(url_ancestor,"url-ancestor", url ())
DECLARE_GLUE_NAME_TYPE(url_concat,"url-append", url (url, url))
DECLARE_GLUE_NAME_TYPE(url_or,"url-or", url (url, url))
DECLARE_GLUE_NAME_TYPE(is_none,"url-none?", bool (url))
DECLARE_GLUE_NAME_BASE(is_rooted,"url-rooted?", bool (url))
DECLARE_GLUE_NAME_BASE(is_rooted,"url-rooted-protocol?", bool (url, string))
DECLARE_GLUE_NAME_TYPE(is_rooted_web,"url-rooted-web?", bool (url))
DECLARE_GLUE_NAME_BASE(is_rooted_tmfs,"url-rooted-tmfs?", bool (url))
DECLARE_GLUE_NAME_BASE(is_rooted_tmfs,"url-rooted-tmfs-protocol?", bool (url, string))
DECLARE_GLUE_NAME_TYPE(get_root,"url-root", string (url))
DECLARE_GLUE_NAME_TYPE(unroot,"url-unroot", url (url))
DECLARE_GLUE_NAME_BASE(is_atomic,"url-atomic?", bool (url))
DECLARE_GLUE_NAME_BASE(is_concat,"url-concat?", bool (url))
DECLARE_GLUE_NAME_TYPE(is_or,"url-or?", bool (url))
DECLARE_GLUE_NAME_TYPE(url_ref,"url-ref", url (url, int))
DECLARE_GLUE_NAME_BASE(head,"url-head", url (url))
DECLARE_GLUE_NAME_BASE(tail,"url-tail", url (url))
DECLARE_GLUE_NAME_TYPE(file_format,"url-format", string (url))
DECLARE_GLUE_NAME_TYPE(suffix,"url-suffix", string (url))
DECLARE_GLUE_NAME_BASE(basename,"url-basename", string (url))
DECLARE_GLUE_NAME_BASE(glue,"url-glue", url (url, string))
DECLARE_GLUE_NAME_TYPE(unglue,"url-unglue", url (url, int))
DECLARE_GLUE_NAME_TYPE(relative,"url-relative", url (url, url))
DECLARE_GLUE_NAME_TYPE(expand,"url-expand", url (url))
DECLARE_GLUE_NAME_TYPE(factor,"url-factor", url (url))
DECLARE_GLUE_NAME_TYPE(delta,"url-delta", url (url, url))
DECLARE_GLUE_NAME_TYPE(is_secure,"url-secure?", bool (url))
DECLARE_GLUE_NAME_TYPE(descends,"url-descends?", bool (url, url))
DECLARE_GLUE_NAME_TYPE(complete,"url-complete", url (url, string))
DECLARE_GLUE_NAME_TYPE(resolve,"url-resolve", url (url, string))
DECLARE_GLUE_NAME_TYPE(resolve_in_path,"url-resolve-in-path", url (url))
DECLARE_GLUE_NAME_TYPE(resolve_pattern,"url-resolve-pattern", url (url))
DECLARE_GLUE_NAME_TYPE(exists,"url-exists?", bool (url))
DECLARE_GLUE_NAME_TYPE(exists_in_path,"url-exists-in-path?", bool (url))
DECLARE_GLUE_NAME_TYPE(exists_in_tex,"url-exists-in-tex?", bool (url))
DECLARE_GLUE_NAME_TYPE(concretize_url,"url-concretize*", url (url))
DECLARE_GLUE_NAME_TYPE(concretize,"url-concretize", string (url))
DECLARE_GLUE_NAME_TYPE(sys_concretize,"url-sys-concretize", string (url))
DECLARE_GLUE_NAME_TYPE(materialize,"url-materialize", string (url, string))
DECLARE_GLUE_NAME_TYPE(is_of_type,"url-test?", bool (url, string))
DECLARE_GLUE_NAME_TYPE(is_regular,"url-regular?", bool (url))
DECLARE_GLUE_NAME_TYPE(is_directory,"url-directory?", bool (url))
DECLARE_GLUE_NAME_TYPE(is_symbolic_link,"url-link?", bool (url))
DECLARE_GLUE_NAME_TYPE(is_newer,"url-newer?", bool (url, url))
DECLARE_GLUE_NAME_TYPE(file_size,"url-size", int (url))
//int last_modified_wrap(url u) {return last_modified(u);}
//DECLARE_GLUE_NAME_TYPE(last_modified_wrap,"url-last-modified", int (url))
//url url_temp_wrap () { return url_temp(); }
//DECLARE_GLUE_NAME_TYPE(url_temp_wrap,"url-temp", url ())
//DECLARE_GLUE_NAME_TYPE(url_scratch,"url-scratch", url (string, string, int))
//DECLARE_GLUE_NAME_TYPE(is_scratch,"url-scratch?", bool (url))
//DECLARE_GLUE_NAME_TYPE(web_cache_invalidate,"url-cache-invalidate", void (url))
//DECLARE_GLUE_NAME_TYPE(string_save,"string-save", void (string, url))
//DECLARE_GLUE_NAME_TYPE(string_load,"string-load", string (url))
//DECLARE_GLUE_NAME_TYPE(string_append_to_file,"string-append-to-file", void (string, url))
//DECLARE_GLUE_NAME_BASE(move,"system-move", void (url, url))
//DECLARE_GLUE_NAME_BASE(copy,"system-copy", void (url, url))
//DECLARE_GLUE_NAME_BASE(remove,"system-remove", void (url))
//DECLARE_GLUE_NAME_BASE(mkdir,"system-mkdir", void (url))
//DECLARE_GLUE_NAME_BASE(rmdir,"system-rmdir", void (url))
//DECLARE_GLUE_NAME_TYPE(set_env,"system-setenv", void (string, string))
//DECLARE_GLUE_NAME_TYPE(search_score,"system-search-score", int (url, array_string))
//DECLARE_GLUE_NAME_BASE(system,"system-1", void (string, url))
//DECLARE_GLUE_NAME_BASE(system,"system-2", void (string, url, url))
//DECLARE_GLUE_NAME_TYPE(sys_concretize,"system-url->string", string (url))
//DECLARE_GLUE_NAME_TYPE(grep,"url-grep", url (string, url))
//DECLARE_GLUE_NAME_TYPE(search_file_upwards,"url-search-upwards", url (url, string, array_string))
//DECLARE_GLUE_NAME_TYPE(picture_cache_reset,"picture-cache-reset", void ())
//DECLARE_GLUE_NAME_TYPE(set_file_focus,"set-file-focus", void (url))
//DECLARE_GLUE_NAME_TYPE(get_file_focus,"get-file-focus", url ())
//DECLARE_GLUE_NAME_TYPE(persistent_set,"persistent-set", void (url, string, string))
//DECLARE_GLUE_NAME_TYPE(persistent_reset,"persistent-remove", void (url, string))
//DECLARE_GLUE_NAME_TYPE(persistent_contains,"persistent-has?", bool (url, string))
//DECLARE_GLUE_NAME_TYPE(persistent_get,"persistent-get", string (url, string))
//DECLARE_GLUE_NAME_TYPE(persistent_file_name,"persistent-file-name", url (url, string))
//DECLARE_GLUE_NAME_BASE(keep_history,"tmdb-keep-history", void (url, bool))
//DECLARE_GLUE_NAME_BASE(set_field,"tmdb-set-field", void (url, string, string, array_string, double))
//DECLARE_GLUE_NAME_BASE(get_field,"tmdb-get-field", array_string (url, string, string, double))
//DECLARE_GLUE_NAME_BASE(remove_field,"tmdb-remove-field", void (url, string, string, double))
//DECLARE_GLUE_NAME_TYPE(get_attributes,"tmdb-get-attributes", array_string (url, string, double))
//DECLARE_GLUE_NAME_TYPE(set_entry,"tmdb-set-entry", void (url, string, scheme_tree_t, double))
//DECLARE_GLUE_NAME_TYPE(get_entry,"tmdb-get-entry", scheme_tree_t (url, string, double))
//DECLARE_GLUE_NAME_TYPE(remove_entry,"tmdb-remove-entry", void (url, string, double))
//DECLARE_GLUE_NAME_TYPE(query,"tmdb-query", array_string (url, scheme_tree_t, double, int))
//DECLARE_GLUE_NAME_TYPE(inspect_history,"tmdb-inspect-history", void (url, string))
//DECLARE_GLUE_NAME_TYPE(get_completions,"tmdb-get-completions", array_string (url, string))
//DECLARE_GLUE_NAME_TYPE(get_name_completions,"tmdb-get-name-completions", array_string (url, string))
//DECLARE_GLUE_NAME_TYPE(sqlite3_present,"supports-sql?", bool ())
//DECLARE_GLUE_NAME_TYPE(sql_exec,"sql-exec", scheme_tree_t (url, string))
//DECLARE_GLUE_NAME_TYPE(sql_quote,"sql-quote", string (string))
//DECLARE_GLUE_NAME_TYPE(server_start,"server-start", void ())
//DECLARE_GLUE_NAME_TYPE(server_stop,"server-stop", void ())
//DECLARE_GLUE_NAME_TYPE(server_read,"server-read", string (int))
//DECLARE_GLUE_NAME_TYPE(server_write,"server-write", void (int, string))
//DECLARE_GLUE_NAME_TYPE(server_started,"server-started?", bool ())
//DECLARE_GLUE_NAME_TYPE(client_start,"client-start", int (string))
//DECLARE_GLUE_NAME_TYPE(client_stop,"client-stop", void (int))
//DECLARE_GLUE_NAME_TYPE(client_read,"client-read", string (int))
//DECLARE_GLUE_NAME_TYPE(client_write,"client-write", void (int, string))
//DECLARE_GLUE_NAME_TYPE(enter_secure_mode,"enter-secure-mode", void (int))
//string connection_start_wrap (string name, string session) { return connection_start(name, session); }
//DECLARE_GLUE_NAME_TYPE(connection_start_wrap,"connection-start", string (string, string))
//DECLARE_GLUE_NAME_TYPE(connection_status,"connection-status", int (string, string))
//DECLARE_GLUE_NAME_BASE(connection_write,"connection-write-string", void (string, string, string))
//DECLARE_GLUE_NAME_BASE(connection_write,"connection-write", void (string, string, content))
//DECLARE_GLUE_NAME_TYPE(connection_cmd,"connection-cmd", tree (string, string, string))
//DECLARE_GLUE_NAME_BASE(connection_eval,"connection-eval", tree (string, string, content))
//DECLARE_GLUE_NAME_TYPE(connection_interrupt,"connection-interrupt", void (string, string))
//DECLARE_GLUE_NAME_TYPE(connection_stop,"connection-stop", void (string, string))
//DECLARE_GLUE_NAME_TYPE(printer_widget,"widget-printer", widget (command, url))
//DECLARE_GLUE_NAME_TYPE(color_picker_widget,"widget-color-picker", widget (command, bool, array_tree))
//DECLARE_GLUE_NAME_TYPE(extend_widget,"widget-extend", widget (widget, array_widget))
//DECLARE_GLUE_NAME_TYPE(horizontal_menu,"widget-hmenu", widget (array_widget))
//DECLARE_GLUE_NAME_TYPE(vertical_menu,"widget-vmenu", widget (array_widget))
//DECLARE_GLUE_NAME_TYPE(tile_menu,"widget-tmenu", widget (array_widget, int))
//DECLARE_GLUE_NAME_TYPE(minibar_menu,"widget-minibar-menu", widget (array_widget))
//DECLARE_GLUE_NAME_TYPE(menu_separator,"widget-separator", widget (bool))
//DECLARE_GLUE_NAME_TYPE(menu_group,"widget-menu-group", widget (string, int))
//DECLARE_GLUE_NAME_TYPE(pulldown_button,"widget-pulldown-button", widget (widget, promise_widget))
//DECLARE_GLUE_NAME_TYPE(pullright_button,"widget-pullright-button", widget (widget, promise_widget))
//DECLARE_GLUE_NAME_TYPE(menu_button,"widget-menu-button", widget (widget, command, string, string, int))
//DECLARE_GLUE_NAME_TYPE(toggle_widget,"widget-toggle", widget (command, bool, int))
//DECLARE_GLUE_NAME_TYPE(balloon_widget,"widget-balloon", widget (widget, widget))
//DECLARE_GLUE_NAME_TYPE(empty_widget,"widget-empty", widget ())
//DECLARE_GLUE_NAME_TYPE(text_widget,"widget-text", widget (string, int, int, bool))
//DECLARE_GLUE_NAME_TYPE(input_text_widget,"widget-input", widget (command, string, array_string, int, string))
//DECLARE_GLUE_NAME_TYPE(enum_widget,"widget-enum", widget (command, array_string, string, int, string))
//DECLARE_GLUE_NAME_TYPE(choice_widget,"widget-choice", widget (command, array_string, string))
//DECLARE_GLUE_NAME_TYPE(choice_widget,"widget-choices", widget (command, array_string, array_string))
//DECLARE_GLUE_NAME_TYPE(choice_widget,"widget-filtered-choice", widget (command, array_string, string, string))
//DECLARE_GLUE_NAME_TYPE(tree_view_widget,"widget-tree-view", widget (command, tree, tree))
//DECLARE_GLUE_NAME_TYPE(xpm_widget,"widget-xpm", widget (url))
//DECLARE_GLUE_NAME_TYPE(box_widget,"widget-box", widget (scheme_tree_t, string, int, bool, bool))
//DECLARE_GLUE_NAME_TYPE(glue_widget,"widget-glue", widget (bool, bool, int, int))
//DECLARE_GLUE_NAME_TYPE(glue_widget,"widget-color", widget (content, bool, bool, int, int))
//DECLARE_GLUE_NAME_TYPE(horizontal_list,"widget-hlist", widget (array_widget))
//DECLARE_GLUE_NAME_TYPE(vertical_list,"widget-vlist", widget (array_widget))
//DECLARE_GLUE_NAME_TYPE(division_widget,"widget-division", widget (string, widget))
//DECLARE_GLUE_NAME_TYPE(aligned_widget,"widget-aligned", widget (array_widget, array_widget))
//DECLARE_GLUE_NAME_TYPE(tabs_widget,"widget-tabs", widget (array_widget, array_widget))
//DECLARE_GLUE_NAME_TYPE(icon_tabs_widget,"widget-icon-tabs", widget (array_url, array_widget, array_widget))
//DECLARE_GLUE_NAME_TYPE(user_canvas_widget,"widget-scrollable", widget (widget, int))
//DECLARE_GLUE_NAME_TYPE(resize_widget,"widget-resize", widget (widget, int, string, string, string, string, string, string, string, string))
//DECLARE_GLUE_NAME_TYPE(hsplit_widget,"widget-hsplit", widget (widget, widget))
//DECLARE_GLUE_NAME_TYPE(vsplit_widget,"widget-vsplit", widget (widget, widget))
//DECLARE_GLUE_NAME_TYPE(texmacs_output_widget,"widget-texmacs-output", widget (content, content))
//DECLARE_GLUE_NAME_TYPE(texmacs_input_widget,"widget-texmacs-input", widget (content, content, url))
//DECLARE_GLUE_NAME_TYPE(ink_widget,"widget-ink", widget (command))
//DECLARE_GLUE_NAME_TYPE(refresh_widget,"widget-refresh", widget (string, string))
//DECLARE_GLUE_NAME_TYPE(refreshable_widget,"widget-refreshable", widget (object, string))
//DECLARE_GLUE_NAME_TYPE(as_promise_widget,"object->promise-widget", promise_widget (object))
//DECLARE_GLUE_NAME_TYPE(get_bounding_rectangle,"tree-bounding-rectangle", array_int (tree))
//DECLARE_GLUE_NAME_TYPE(get_widget_size,"widget-size", array_int (widget))
//DECLARE_GLUE_NAME_TYPE(get_texmacs_widget_size,"texmacs-widget-size", array_int (widget))
//DECLARE_GLUE_NAME_TYPE(show_help_balloon,"show-balloon", void (widget, int, int))
//DECLARE_GLUE_NAME_TYPE(get_style_menu,"get-style-menu", object ())
//DECLARE_GLUE_NAME_TYPE(hidden_package,"hidden-package?", bool (string))
//DECLARE_GLUE_NAME_TYPE(get_add_package_menu,"get-add-package-menu", object ())
//DECLARE_GLUE_NAME_TYPE(get_remove_package_menu,"get-remove-package-menu", object ())
//DECLARE_GLUE_NAME_TYPE(get_toggle_package_menu,"get-toggle-package-menu", object ())
//DECLARE_GLUE_NAME_TYPE(windows_refresh,"refresh-now", void (string))
//DECLARE_GLUE_NAME_TYPE(get_screen_size,"get-screen-size", array_int ())
//DECLARE_GLUE_NAME_TYPE(get_all_buffers,"buffer-list", array_url ())
//DECLARE_GLUE_NAME_TYPE(get_current_buffer_safe,"current-buffer-url", url ())
//DECLARE_GLUE_NAME_TYPE(path_to_buffer,"path-to-buffer", url (path))
//DECLARE_GLUE_NAME_TYPE(make_new_buffer,"buffer-new", url ())
//DECLARE_GLUE_NAME_TYPE(rename_buffer,"buffer-rename", void (url, url))
//DECLARE_GLUE_NAME_TYPE(set_buffer_tree,"buffer-set", void (url, content))
//DECLARE_GLUE_NAME_TYPE(get_buffer_tree,"buffer-get", tree (url))
//DECLARE_GLUE_NAME_TYPE(set_buffer_body,"buffer-set-body", void (url, content))
//DECLARE_GLUE_NAME_TYPE(get_buffer_body,"buffer-get-body", tree (url))
//DECLARE_GLUE_NAME_TYPE(set_master_buffer,"buffer-set-master", void (url, url))
//DECLARE_GLUE_NAME_TYPE(get_master_buffer,"buffer-get-master", url (url))
//DECLARE_GLUE_NAME_TYPE(set_title_buffer,"buffer-set-title", void (url, string))
//DECLARE_GLUE_NAME_TYPE(get_title_buffer,"buffer-get-title", string (url))
//DECLARE_GLUE_NAME_TYPE(get_last_save_buffer,"buffer-last-save", int (url))
//DECLARE_GLUE_NAME_TYPE(last_visited,"buffer-last-visited", double (url))
//DECLARE_GLUE_NAME_TYPE(buffer_modified,"buffer-modified?", bool (url))
//DECLARE_GLUE_NAME_TYPE(buffer_modified_since_autosave,"buffer-modified-since-autosave?", bool (url))
//DECLARE_GLUE_NAME_TYPE(pretend_buffer_modified,"buffer-pretend-modified", void (url))
//DECLARE_GLUE_NAME_TYPE(pretend_buffer_saved,"buffer-pretend-saved", void (url))
//DECLARE_GLUE_NAME_TYPE(pretend_buffer_autosaved,"buffer-pretend-autosaved", void (url))
//DECLARE_GLUE_NAME_TYPE(attach_buffer_notifier,"buffer-attach-notifier", void (url))
//DECLARE_GLUE_NAME_TYPE(buffer_has_name,"buffer-has-name?", bool (url))
//DECLARE_GLUE_NAME_TYPE(is_aux_buffer,"buffer-aux?", bool (url))
//DECLARE_GLUE_NAME_TYPE(is_embedded_buffer,"buffer-embedded?", bool (url))
//DECLARE_GLUE_NAME_TYPE(buffer_import,"buffer-import", bool (url, url, string))
//DECLARE_GLUE_NAME_TYPE(buffer_load,"buffer-load", bool (url))
//DECLARE_GLUE_NAME_TYPE(buffer_export,"buffer-export", bool (url, url, string))
//DECLARE_GLUE_NAME_TYPE(buffer_save,"buffer-save", bool (url))
//DECLARE_GLUE_NAME_TYPE(import_loaded_tree,"tree-import-loaded", tree (string, url, string))
//DECLARE_GLUE_NAME_TYPE(import_tree,"tree-import", tree (url, string))
//DECLARE_GLUE_NAME_TYPE(load_inclusion,"tree-inclusion", tree (url))
//DECLARE_GLUE_NAME_TYPE(export_tree,"tree-export", bool (tree, url, string))
//DECLARE_GLUE_NAME_TYPE(load_style_tree,"tree-load-style", tree (string))
//DECLARE_GLUE_NAME_TYPE(focus_on_buffer,"buffer-focus", bool (url))
//DECLARE_GLUE_NAME_TYPE(get_all_views,"view-list", array_url ())
//DECLARE_GLUE_NAME_TYPE(buffer_to_views,"buffer->views", array_url (url))
//DECLARE_GLUE_NAME_TYPE(get_current_view_safe,"current-view-url", url ())
//DECLARE_GLUE_NAME_TYPE(window_to_view,"window->view", url (url))
//DECLARE_GLUE_NAME_TYPE(view_to_buffer,"view->buffer", url (url))
//DECLARE_GLUE_NAME_TYPE(view_to_window,"view->window-url", url (url))
//DECLARE_GLUE_NAME_TYPE(get_new_view,"view-new", url (url))
//DECLARE_GLUE_NAME_TYPE(get_passive_view,"view-passive", url (url))
//DECLARE_GLUE_NAME_TYPE(get_recent_view,"view-recent", url (url))
//DECLARE_GLUE_NAME_TYPE(delete_view,"view-delete", void (url))
//DECLARE_GLUE_NAME_TYPE(window_set_view,"window-set-view", void (url, url, bool))
//DECLARE_GLUE_NAME_TYPE(switch_to_buffer,"switch-to-buffer", void (url))
//DECLARE_GLUE_NAME_TYPE(set_current_drd,"set-drd", void (url))
//DECLARE_GLUE_NAME_TYPE(windows_list,"window-list", array_url ())
//DECLARE_GLUE_NAME_TYPE(get_nr_windows,"windows-number", int ())
//DECLARE_GLUE_NAME_TYPE(get_current_window,"current-window", url ())
//DECLARE_GLUE_NAME_TYPE(buffer_to_windows,"buffer->windows", array_url (url))
//DECLARE_GLUE_NAME_TYPE(window_to_buffer,"window-to-buffer", url (url))
//DECLARE_GLUE_NAME_TYPE(window_set_buffer,"window-set-buffer", void (url, url))
//DECLARE_GLUE_NAME_TYPE(window_focus,"window-focus", void (url))
//DECLARE_GLUE_NAME_TYPE(switch_to_window,"switch-to-window", void (url))
//DECLARE_GLUE_NAME_TYPE(create_buffer,"new-buffer", url ())
//DECLARE_GLUE_NAME_TYPE(new_buffer_in_new_window,"open-buffer-in-window", url (url, content, content))
//DECLARE_GLUE_NAME_TYPE(open_window,"open-window", url ())
//DECLARE_GLUE_NAME_TYPE(open_window,"open-window-geometry", url (content))
//DECLARE_GLUE_NAME_TYPE(clone_window,"clone-window", void ())
//DECLARE_GLUE_NAME_TYPE(kill_buffer,"cpp-buffer-close", void (url))
//DECLARE_GLUE_NAME_TYPE(kill_window,"kill-window", void (url))
//DECLARE_GLUE_NAME_TYPE(kill_current_window_and_buffer,"kill-current-window-and-buffer", void ())
//DECLARE_GLUE_NAME_TYPE(project_attach,"project-attach", void (string))
//DECLARE_GLUE_NAME_TYPE(project_attach,"project-detach", void ())
//DECLARE_GLUE_NAME_TYPE(project_attached,"project-attached?", bool ())
//DECLARE_GLUE_NAME_TYPE(project_get,"project-get", url ())
//DECLARE_GLUE_NAME_TYPE(window_handle,"alt-window-handle", int ())
//DECLARE_GLUE_NAME_TYPE(window_create,"alt-window-create-quit", void (int, widget, string, command))
//DECLARE_GLUE_NAME_TYPE(window_create_plain,"alt-window-create-plain", void (int, widget, string))
//DECLARE_GLUE_NAME_TYPE(window_create_popup,"alt-window-create-popup", void (int, widget, string))
//DECLARE_GLUE_NAME_TYPE(window_create_tooltip,"alt-window-create-tooltip", void (int, widget, string))
//DECLARE_GLUE_NAME_TYPE(window_delete,"alt-window-delete", void (int))
//DECLARE_GLUE_NAME_TYPE(window_show,"alt-window-show", void (int))
//DECLARE_GLUE_NAME_TYPE(window_hide,"alt-window-hide", void (int))
//DECLARE_GLUE_NAME_TYPE(window_get_size,"alt-window-get-size", scheme_tree_t (int))
//DECLARE_GLUE_NAME_TYPE(window_set_size,"alt-window-set-size", void (int, int, int))
//DECLARE_GLUE_NAME_TYPE(window_get_position,"alt-window-get-position", scheme_tree_t (int))
//DECLARE_GLUE_NAME_TYPE(window_set_position,"alt-window-set-position", void (int, int, int))
//DECLARE_GLUE_NAME_TYPE(window_search,"alt-window-search", path (url))
//DECLARE_GLUE_NAME_TYPE(bibtex_present,"supports-bibtex?", bool ())
//DECLARE_GLUE_NAME_TYPE(bibtex_run,"bibtex-run", tree (string, string, url, array_string))
//DECLARE_GLUE_NAME_TYPE(bib_add_period,"bib-add-period", scheme_tree_t (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_locase_first,"bib-locase-first", scheme_tree_t (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_upcase_first,"bib-upcase-first", scheme_tree_t (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_locase,"bib-locase", scheme_tree_t (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_upcase,"bib-upcase", scheme_tree_t (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_default_preserve_case,"bib-default-preserve-case", scheme_tree_t (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_default_upcase_first,"bib-default-upcase-first", scheme_tree_t (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_purify,"bib-purify", string (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_text_length,"bib-text-length", int (scheme_tree_t))
//DECLARE_GLUE_NAME_TYPE(bib_prefix,"bib-prefix", string (scheme_tree_t, int))
//DECLARE_GLUE_NAME_TYPE(bib_empty,"bib-empty?", bool (scheme_tree_t, string))
//DECLARE_GLUE_NAME_TYPE(bib_field,"bib-field", scheme_tree_t (scheme_tree_t, string))
//DECLARE_GLUE_NAME_TYPE(bib_abbreviate,"bib-abbreviate", scheme_tree_t (scheme_tree_t, scheme_tree_t, scheme_tree_t))

DECLARE_GLUE_NAME_TYPE(set_font_rules, "set-font-rules", void (scheme_tree_t))

void
glue_function_rep::instantiate_all () {
  while (!is_nil(glue_functions)) {
    glue_functions->item->instantiate();
    glue_functions= glue_functions->next;
  }
}


void
initialize_glue () {
  tmscm_install_procedure ("tree?", treeP, 1, 0, 0);
  tmscm_install_procedure ("tm?", contentP, 1, 0, 0);
  tmscm_install_procedure ("observer?", observerP, 1, 0, 0);
  tmscm_install_procedure ("url?", urlP, 1, 0, 0);
  tmscm_install_procedure ("modification?", modificationP, 1, 0, 0);
  tmscm_install_procedure ("patch?", patchP, 1, 0, 0);
  tmscm_install_procedure ("blackbox?", blackboxP, 1, 0, 0);
  
  glue_function_rep::instantiate_all ();
}


