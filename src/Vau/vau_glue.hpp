
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

class glue_function_rep : concrete_struct {
  const char *name;
  s7_function fn;
  int arity;
  static list<glue_function> glue_functions;
protected:
  glue_function_rep (const char *_name, s7_function _fn, int _ar);
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

glue_function_rep::glue_function_rep (const char *_name, s7_function _fn, int _ar)
  : name (_name), fn (_fn), arity (_ar) {
  glue_functions= list<glue_function> (this, glue_functions);
}

// adaptor template class
template<typename T0, typename S0, S0 fn> struct tm_glue  {
  // we do not provide constructor to detect matching errors
};

template<typename S0, S0 f, typename T0, typename ... Ts>
struct tm_glue<T0 (Ts ...), S0, f> : public glue_function_rep {
  static const char*__name;
  template<typename S> struct Arg {
      S value;
      Arg (tmscm &args) : value (tmscm_to<S>(tmscm_car (args)))  { args=tmscm_cdr (args); }
  };
  template<typename ... As> struct Check_args {
    template<typename S>   struct Check {
        bool check;
        Check (tmscm &args, int& n, const char *name) : check(true)   {
            tmscm_check<S>(tmscm_car (args), ++n, name); args=tmscm_cdr (args);
        }
    };
    int arg;
    Check_args (tmscm args, const char *name)
      : arg(0) {
        bool vv[sizeof...(As)]= { Check<As>(args, arg, __name).check ...};
        (void) vv;
    }
  };
  template<typename TT> static tmscm wrap (Ts ... args) {
    TT res= f (args ...);
    return tmscm_from<TT> (res);
  }
  template<> static tmscm wrap<void> (Ts ... args) {
    f (args ...);
    return TMSCM_UNSPECIFIED;
  }
  template<typename TT> static tmscm proc (s7_scheme*, tmscm args) {
    Check_args<Ts ...>(args, __name);
    return  wrap<TT> (Arg<Ts>(args).value ...);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, proc<T0>, sizeof...(Ts)) { __name= _name; }
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

#endif /* VAU_GLUE_H */
