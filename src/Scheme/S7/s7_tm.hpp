/******************************************************************************
 * MODULE     : s7_tm.hpp
 * DESCRIPTION: Interface to S7
 * COPYRIGHT  : (C) 2020  Joris van der Hoeven and Massimiliano Gubinelli
 *******************************************************************************
 * This software falls under the GNU general public license version 3 or later.
 * It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
 * in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
 ******************************************************************************/

#ifndef S7_TM_H
#define S7_TM_H

#include "tm_configure.hpp"
#include "blackbox.hpp"
#include "array.hpp"

#include "s7.h"

// the type of scheme objects
typedef s7_pointer tmscm;

// the global scheme interpreter state
extern s7_scheme *tm_s7;

bool tmscm_is_blackbox (tmscm obj);
tmscm blackbox_to_tmscm (blackbox b);
blackbox tmscm_to_blackbox (tmscm obj);

inline tmscm tmscm_null () { return s7_nil(tm_s7); }
inline tmscm tmscm_true () { return s7_t(tm_s7); }
inline tmscm tmscm_false () { return s7_f(tm_s7); }
inline void tmscm_set_car (tmscm a, tmscm b) { s7_set_car (a, b); }
inline void tmscm_set_cdr (tmscm a, tmscm b) { s7_set_cdr (a, b); }
	
inline bool tmscm_is_equal (tmscm o1, tmscm o2) { return s7_is_equal(tm_s7, o1, o2); }

inline bool tmscm_is_null (tmscm obj) { return s7_is_null (tm_s7, obj); }
inline bool tmscm_is_pair (tmscm obj) { return s7_is_pair (obj); }
inline bool tmscm_is_list (tmscm obj) { return s7_is_list (tm_s7, obj); }
inline bool tmscm_is_bool (tmscm obj) { return s7_is_boolean (obj); }
inline bool tmscm_is_int (tmscm obj) { return s7_is_integer (obj); }
inline bool tmscm_is_double (tmscm obj) { return s7_is_real (obj); }
inline bool tmscm_is_string (tmscm obj) { return s7_is_string (obj); }
inline bool tmscm_is_symbol (tmscm obj) { return s7_is_symbol (obj); }

inline tmscm tmscm_cons (tmscm obj1, tmscm obj2) { return s7_cons (tm_s7, obj1, obj2); }
inline tmscm tmscm_car (tmscm obj) { return s7_car (obj); }
inline tmscm tmscm_cdr (tmscm obj) { return s7_cdr (obj); }
inline tmscm tmscm_caar (tmscm obj) { return s7_caar (obj); }
inline tmscm tmscm_cadr (tmscm obj) { return s7_cadr (obj); }
inline tmscm tmscm_cdar (tmscm obj) { return s7_cdar (obj); }
inline tmscm tmscm_cddr (tmscm obj) { return s7_cddr (obj); }
inline tmscm tmscm_caddr (tmscm obj) { return s7_caddr (obj); }
inline tmscm tmscm_cadddr (tmscm obj) { return s7_cadddr (obj); }

inline tmscm bool_to_tmscm (bool b) { return (b? s7_t(tm_s7) : s7_f(tm_s7)); }
inline tmscm int_to_tmscm (int i) { return s7_make_integer(tm_s7, i); }
inline tmscm long_to_tmscm (long l) { return s7_make_integer(tm_s7, l);  }
inline tmscm double_to_tmscm (double r) { return s7_make_real(tm_s7, r); }
tmscm string_to_tmscm (string s);
tmscm symbol_to_tmscm (string s);

inline bool tmscm_to_bool (tmscm obj) { return s7_boolean (tm_s7, obj); }
inline int tmscm_to_int (tmscm obj) { return s7_integer (obj); }
inline double tmscm_to_double (tmscm obj) { return s7_real (obj); }
string tmscm_to_string (tmscm obj);
string tmscm_to_symbol (tmscm obj);

tmscm eval_scheme_file (string name);
tmscm eval_scheme (string s);
tmscm call_scheme (tmscm fun);
tmscm call_scheme (tmscm fun, tmscm a1);
tmscm call_scheme (tmscm fun, tmscm a1, tmscm a2);
tmscm call_scheme (tmscm fun, tmscm a1, tmscm a2, tmscm a3);
tmscm call_scheme (tmscm fun, tmscm a1, tmscm a2, tmscm a3, tmscm a4);
tmscm call_scheme (tmscm fun, array<tmscm> a);


/******************************************************************************
 * Gluing
 ******************************************************************************/

// template to adapt the format for function call, from scheme list to C++ arg list
template<typename T> struct Proc {};
template<typename T0, typename ... Ts>
struct Proc<T0 (Ts ...)> {
  template<typename S> struct Arg { tmscm value; Arg (tmscm &args) : value (tmscm_car(args)) { args=tmscm_cdr (args); }  };
  template<T0 (*PROC)(Ts ...)>
  static tmscm proc (s7_scheme*, tmscm args) {
    tmscm res = PROC (Arg<Ts>(args).value ...);
    return (res);
  }
};

//FIXME: check the correct use of p0 and p1 and maybe add documentation
#define tmscm_install_procedure(name, func, args, p0, p1) \
  s7_define_function (tm_s7, name, Proc<decltype(func)>::proc<func>, args, 0, false, "[missing doc]")

#define tmscm_install_procedure_bis(name, func, args, p0, p1) \
  s7_define_function (tm_s7, name, func, args, 0, false, "[missing doc]")

#define TMSCM_UNSPECIFIED (s7_unspecified (tm_s7))

string scheme_dialect ();

#endif // defined S7_TM_H


