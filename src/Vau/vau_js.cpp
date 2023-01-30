#include <emscripten/bind.h>

#include "scheme.hpp"


using namespace emscripten;

// Binding code
EMSCRIPTEN_BINDINGS(vau_lib) {
  class_<object>("VauObject")
    ;


  function("null_object", &null_object);  
  function("list_object", select_overload<object (object)>(&list_object));  
  function("symbol_object", &symbol_object);  
  function("call", select_overload<object (string)>(&call)); 

}



