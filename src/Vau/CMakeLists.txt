#file(GLOB_RECURSE SRCS "*.cpp" "*.hpp" "*.h")
target_sources(Vau PRIVATE
        vau_buffer.cpp
        vau_buffer.hpp
        vau_editor.cpp
        vau_editor.hpp
        vau_glue.cpp
        vau_glue.hpp
#        vau_glue-bak-1.cpp
#        vau_glue-bak-1.hpp
        vau_main.cpp
        vau_main.hpp
        vau_stuff.cpp
        vau_stuff.hpp
        )