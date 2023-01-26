
/******************************************************************************
* MODULE     : vau_glue.hpp
* DESCRIPTION: Glue to Scheme
* COPYRIGHT  : (C) 2023  Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#ifndef VAU_GLUE_H
#define VAU_GLUE_H

#include "glue.hpp"

// conversion generics
template<typename T0> tmscm tmscm_from (T0 out);
template<typename T0> T0 tmscm_to (tmscm in);
template<typename T0> void tmscm_check (tmscm in, int arg, const char *fname);
template<typename T0> bool tmscm_is (tmscm in);

class glue_function;

typedef tmscm (*FN) (s7_scheme*, tmscm args);

class glue_function_rep : concrete_struct {
  const char *name;
  FN fn;
  int arity;
  static list<glue_function> glue_functions;
protected:
  glue_function_rep (const char *_name, FN _fn, int _ar);
  void instantiate () {
    tmscm_install_procedure_bis (name, fn, arity, 0, 0);
  }
public:
  static void instantiate_all ();
  friend class glue_function;
};

class glue_function {
  CONCRETE(glue_function);
  glue_function (glue_function_rep *_rep) : rep (_rep) {}
};
CONCRETE_CODE(glue_function);

glue_function_rep::glue_function_rep (const char *_name, FN _fn, int _ar)
  : name (_name), fn (_fn), arity (_ar) {
  glue_functions= list<glue_function> (this, glue_functions);
}

// adaptor template class
template<typename T0, typename S0, S0 fn> struct tm_glue  {
  // we do not provide constructor to detect matching errors
};

class scheme_tree_t;



template<typename S0, S0 f, typename T0, typename ... Ts>
struct tm_glue<T0 (Ts ...), S0, f> : public glue_function_rep {
  template<typename A> struct Arg { typedef tmscm Type; };
  template<typename A> struct Res { typedef A Type; };
  template<> struct Res<scheme_tree_t> { typedef scheme_tree Type; };
  template<typename ... As> struct tmscm_check_args {
    int arg;
//    bool v[sizeof...(As)];
    tmscm_check_args (const char *name, typename Arg<As>::Type ... args)
      : arg(0) {
        bool vv[sizeof...(As)]= {(arg++, tmscm_check<As>(args, arg, name), true) ...};
        (void) vv;
    }
  };
  template<> struct tmscm_check_args<> {
    tmscm_check_args (const char *name) {}
  };
  static const char*__name;
  template<typename TT> static typename Res<T0>::Type wrap (Ts ... args) {
    return f (args ...);
  }
  static void wrap_void (Ts ... args) {
    f (args ...);
  }
  template<typename TT> static tmscm func (typename Arg<Ts>::Type ... args) {
    tmscm_check_args<Ts...> check (__name, args...);
    TT out= wrap<TT> (tmscm_to<Ts> (args) ...);
    return tmscm_from<TT> (out);
  }
  template<> static tmscm func<void> (typename Arg<Ts>::Type ... args) {
    tmscm_check_args<Ts...> check (__name, args...);
    wrap_void (tmscm_to<Ts> (args) ...);
    return TMSCM_UNSPECIFIED;
  }
  tm_glue (const char *_name) : glue_function_rep (_name, proc<func<T0> >, sizeof...(Ts)) { __name= _name; }
};

template<typename T0, typename S0, S0 fn> glue_function
declare_glue (const char *_name) {
  return tm_new<tm_glue<T0, S0, fn> > (_name);
}

// to implement unique labels for static variables in DECLARE_GLUE_NAME_TYPE
#define DECLARE_GLUE_CONCAT(a, b) DECLARE_GLUE_CONCAT_INNER(a, b)
#define DECLARE_GLUE_CONCAT_INNER(a, b) a ## b

// declarations macros
#define DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, BASE) \
  glue_function DECLARE_GLUE_CONCAT(glue_decl_##FUNC,__COUNTER__) (declare_glue<TYPE, BASE, FUNC> (NAME));
#define DECLARE_GLUE_NAME_TYPE(FUNC, NAME, TYPE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, decltype(FUNC))
#define DECLARE_GLUE_NAME_BASE(FUNC, NAME, TYPE) DECLARE_GLUE_NAME_TYPE_BASE(FUNC, NAME, TYPE, TYPE)
#define DECLARE_GLUE_NAME(FUNC, NAME) DECLARE_GLUE_NAME_TYPE(FUNC, NAME, decltype(FUNC))
#define DECLARE_GLUE(FUNC) DECLARE_GLUE_NAME(FUNC, #FUNC)

// old stuff
// glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (NAME));
// static glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (#FUNC));

#endif /* VAU_GLUE_H */
