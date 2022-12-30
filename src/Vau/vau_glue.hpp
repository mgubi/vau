//
//  vau_glue.hpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 28/12/2022.
//

#ifndef VAU_GLUE_H
#define VAU_GLUE_H

#include "glue.hpp"

// conversion generics
template<typename T0> tmscm tmscm_from (T0 out);
template<typename T0> T0 tmscm_to (SCM in);

class glue_function;

class glue_function_rep : concrete_struct {
  const char *name;
  FN fn;
  int arity;
  static list<glue_function> glue_functions;
protected:
  glue_function_rep (const char *_name, FN _fn, int _ar);
  void instantiate () {
    tmscm_install_procedure (name, fn, arity, 0, 0);
  }
public:
  static void instantiate_all ();
  friend class glue_function;
};

class glue_function {
  CONCRETE(glue_function);
  glue_function (glue_function_rep *_rep) : rep(_rep) {}
};
CONCRETE_CODE(glue_function);

glue_function_rep::glue_function_rep (const char *_name, FN _fn, int _ar)
  : name (_name), fn (_fn), arity (_ar) {
  glue_functions= list<glue_function> (this, glue_functions);
}

// adaptor template class
template<typename T0, T0 fn> struct tm_glue  {
  // we do not provide constructor to detect matching errors
};

template<typename T0, T0 f ()>
struct tm_glue<T0 (), f>  : public glue_function_rep {
  static tmscm func () {
    T0 out= f ();
    return tmscm_from<T0> (out);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 0) {}
};

template<typename T0, typename T1, T0 f (T1)>
struct tm_glue<T0 (T1), f> : public glue_function_rep {
  static tmscm func (tmscm arg1) {
    T1 in1= tmscm_to<T1> (arg1);
    T0 out= f (in1);
    return tmscm_from<T0> (out);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 1) {}
};

template<typename T0, typename T1, typename T2, T0 f (T1, T2)>
struct tm_glue<T0 (T1, T2), f> : public glue_function_rep {
  static tmscm func (tmscm arg1, tmscm arg2) {
    T1 in1= tmscm_to<T1> (arg1);
    T2 in2= tmscm_to<T2> (arg2);
    T0 out= f (in1, in2);
    return tmscm_from<T0> (out);
  }
  tm_glue (const char *_name) : glue_function_rep (_name, (FN)func, 2) {}
};

template<typename T0, T0 fn> glue_function
declare_glue (const char *_name) {
  return tm_new<tm_glue<T0, fn> > (_name);
}

// to implement unique labels for static variables in DECLARE_GLUE_NAME_TYPE
#define DECLARE_GLUE_CONCAT(a, b) DECLARE_GLUE_CONCAT_INNER(a, b)
#define DECLARE_GLUE_CONCAT_INNER(a, b) a ## b

// declarations macros
#define DECLARE_GLUE_NAME_TYPE(FUNC, NAME, TYPE) \
  glue_function DECLARE_GLUE_CONCAT(glue_decl_##FUNC,__COUNTER__) (declare_glue<TYPE, FUNC> (NAME));
#define DECLARE_GLUE_NAME(FUNC, NAME) DECLARE_GLUE_NAME_TYPE(FUNC, NAME, decltype(FUNC))
#define DECLARE_GLUE(FUNC) DECLARE_GLUE_NAME(FUNC, #FUNC)

// old stuff
// glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (NAME));
// static glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (#FUNC));

#endif /* VAU_GLUE_H */
