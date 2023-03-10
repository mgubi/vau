
/******************************************************************************
* MODULE     : vau_main.cpp
* DESCRIPTION: Vau app entrypoint
* COPYRIGHT  : (C) 2023  Massimiliano Gubinelli
*******************************************************************************
* This software falls under the GNU general public license version 3 or later.
* It comes WITHOUT ANY WARRANTY WHATSOEVER. For details, see the file LICENSE
* in the root directory or <http://www.gnu.org/licenses/gpl-3.0.html>.
******************************************************************************/

#include "vau_lib.hpp"

int
main (int argc, char **argv) {
  init_vau_lib (argc, argv);
  return 0;
}

