//
//  vau_glue.cpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 28/12/2022.
//

#include "vau_glue.hpp"

list<glue_function> glue_function_rep::glue_functions;


template<> tmscm tmscm_from<int> (int in) { return SCM_NULL; }
template<> tmscm tmscm_from<bool> (bool in) { return SCM_NULL; }
template<> int tmscm_to<int> (SCM in) { return 0; }


const char *tm_glue_base::name= "none";

bool test () {
  return false;
}

bool test1 (int in1) {
  return false;
}

glue_function glue_decl_test (declare_glue<decltype(test), test> ("test"));
glue_function glue_decl_test1 (declare_glue<decltype(test1), test1> ("test1"));
//DECLARE_GLUE (test);
//DECLARE_GLUE (test1);

