
/******************************************************************************
* MODULE     : vau_suff.hpp
* DESCRIPTION: Miscellanea of functions to run Vau
* COPYRIGHT  : (C) 2023  Joris van der Hoeven and Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#ifndef VAU_STUFF_H
#define VAU_STUFF_H

#include "string.hpp"
#include "url.hpp"

int    system (string s);
int    system (string s, string &r);
int    system (string s, string &r, string& e);
string eval_system (string s);
string var_eval_system (string s);
string get_env (string var);
void   set_env (string var, string with);
void make_dir (url which);
//void change_mode (url u, int mode);
#endif /* VAU_STUFF_H */
