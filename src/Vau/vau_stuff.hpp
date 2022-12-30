//
//  vau_stuff.hpp
//  VauQt
//
//  Created by Massimiliano Gubinelli on 26/12/2022.
//

#ifndef VAU_STUFF_H
#define VAU_STUFF_H

#include "string.hpp"

int    system (string s);
int    system (string s, string &r);
int    system (string s, string &r, string& e);
string eval_system (string s);
string var_eval_system (string s);
string get_env (string var);
void   set_env (string var, string with);


#endif /* VAU_STUFF_H */
