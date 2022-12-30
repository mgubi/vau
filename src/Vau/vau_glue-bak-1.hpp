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

struct tm_glue_base {
  static const char *name;
};

// adaptor template class
template<typename T0, T0 fn> struct tm_glue : public tm_glue_base {
  // we do not provide func or arity to be able to detect matching errors
};

template<typename T0, T0 fn ()>
struct tm_glue<T0 (), fn>  : public tm_glue_base {
  static SCM func () {
    return SCM_NULL;
  }
  static const int arity = 0;
};

template<typename T0, typename T1, T0 fn (T1)>
struct tm_glue<T0 (T1), fn> : public tm_glue_base {
  static tmscm func (tmscm arg1) {
    T1 in1= tmscm_to<T1> (arg1);
    T0 out= fn (in1);
    return tmscm_from<T0> (out);
  }
  static const int arity = 1;
};

class glue_function;

struct glue_function_rep : concrete_struct {
  const char *name;
  FN fn;
  int arity;
  glue_function_rep (const char *_name, FN _fn, int _ar)
    : name (_name), fn (_fn), arity (_ar) {}
  void instantiate () {
    tmscm_install_procedure (name, fn, arity, 0, 0);
  }

  static list<glue_function> glue_functions;
  static void instantiate_all ();
};

class glue_function {
  CONCRETE(glue_function);
  glue_function (glue_function_rep *_rep) : rep(_rep) {}
};
CONCRETE_CODE(glue_function);

template<typename T0, T0 fn> glue_function
declare_glue (const char *_name) {
  typedef tm_glue<T0, fn> Glue;
  Glue::name= _name;
  glue_function decl (tm_new<glue_function_rep>(_name, (FN)(Glue::func)  , (Glue::arity)));
  glue_function_rep::glue_functions= list<glue_function> (decl, glue_function_rep::glue_functions);
  return decl;
}

#define DECLARE_GLUE (FUNC) glue_function glue_decl_##FUNC (declare_glue<decltype(FUNC), FUNC> (#FUNC))

#endif /* VAU_GLUE_H */
