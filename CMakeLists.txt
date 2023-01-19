cmake_minimum_required(VERSION 3.24)

project (Vau)

# The name of our project is "Vau". CMakeLists files in this project can 
# refer to the root source directory of the project as ${Vau_SOURCE_DIR} and 
# to the root binary directory of the project as ${Vau_BINARY_DIR}. 

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_INCLUDE_CURRENT_DIR ON)

find_package (PkgConfig REQUIRED)
pkg_check_modules (Guile REQUIRED guile-1.8 gmp IMPORTED_TARGET)
find_package (Freetype REQUIRED)
find_package (Iconv REQUIRED)

#file(GLOB_RECURSE subdirs LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*")

#message(STATUS ${subdirs})

#foreach(subdir ${subdirs})
#    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
#        target_include_directories(Vau PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
#    endif ()
#endforeach()

#add_subdirectory(Data)
#add_subdirectory(Graphics)
#add_subdirectory(Kernel)
#add_subdirectory(Plugins)
#add_subdirectory(System)
#add_subdirectory(Typeset)
#add_subdirectory(Scheme)

### --------------------------------------------------------------------
### the include dirs
### --------------------------------------------------------------------
set (TeXmacs_Include_Dirs
  ${Vau_SOURCE_DIR}/src/Data/Convert
  ${Vau_SOURCE_DIR}/src/Data/Document
  ${Vau_SOURCE_DIR}/src/Data/Drd
  ${Vau_SOURCE_DIR}/src/Data/History
  ${Vau_SOURCE_DIR}/src/Data/Observers
  ${Vau_SOURCE_DIR}/src/Data/Parser
  ${Vau_SOURCE_DIR}/src/Data/String
  ${Vau_SOURCE_DIR}/src/Data/Tree
  ${Vau_SOURCE_DIR}/src/Edit
  ${Vau_SOURCE_DIR}/src/Edit/Editor
  ${Vau_SOURCE_DIR}/src/Edit/Interface
  ${Vau_SOURCE_DIR}/src/Edit/Modify
  ${Vau_SOURCE_DIR}/src/Edit/Process
  ${Vau_SOURCE_DIR}/src/Edit/Replace
  ${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts
  ${Vau_SOURCE_DIR}/src/Graphics/Colors
  ${Vau_SOURCE_DIR}/src/Graphics/Fonts
  ${Vau_SOURCE_DIR}/src/Graphics/Gui
  ${Vau_SOURCE_DIR}/src/Graphics/Handwriting
  ${Vau_SOURCE_DIR}/src/Graphics/Mathematics
  ${Vau_SOURCE_DIR}/src/Graphics/Pictures
  ${Vau_SOURCE_DIR}/src/Graphics/Renderer
  ${Vau_SOURCE_DIR}/src/Graphics/Spacial
  ${Vau_SOURCE_DIR}/src/Graphics/Types
  ${Vau_SOURCE_DIR}/src/Kernel/Abstractions
  ${Vau_SOURCE_DIR}/src/Kernel/Containers
  ${Vau_SOURCE_DIR}/src/Kernel/Types
  ${Vau_SOURCE_DIR}/src/Plugins
  ${Vau_SOURCE_DIR}/src/Plugins/Pdf/LibAesgm
  ${Vau_SOURCE_DIR}/src/Scheme
  ${Vau_SOURCE_DIR}/src/Scheme/Guile
  ${Vau_SOURCE_DIR}/src/Scheme/Scheme
  ${Vau_SOURCE_DIR}/src/Style/Environment
  ${Vau_SOURCE_DIR}/src/Style/Evaluate
  ${Vau_SOURCE_DIR}/src/Style/Memorizer
  ${Vau_SOURCE_DIR}/src/System
  ${Vau_SOURCE_DIR}/src/System/Boot
  ${Vau_SOURCE_DIR}/src/System/Classes
  ${Vau_SOURCE_DIR}/src/System/Files
  ${Vau_SOURCE_DIR}/src/System/Language
  ${Vau_SOURCE_DIR}/src/System/Link
  ${Vau_SOURCE_DIR}/src/System/Misc
  ${Vau_SOURCE_DIR}/src/Texmacs
  ${Vau_SOURCE_DIR}/src/Texmacs/Data
  ${Vau_SOURCE_DIR}/src/Typeset
  ${Vau_SOURCE_DIR}/src/Typeset/Bridge
  ${Vau_SOURCE_DIR}/src/Typeset/Concat
  ${Vau_SOURCE_DIR}/src/Typeset/Page
  ${Vau_SOURCE_DIR}/TeXmacs/include 
  ${Vau_BINARY_DIR}/src/System/ 
  ${Vau_SOURCE_DIR}/src
)


### --------------------------------------------------------------------
### the main sources
### --------------------------------------------------------------------
file (GLOB_RECURSE Vau_Base_SRCS
  "${Vau_SOURCE_DIR}/src/Data/Drd/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/String/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Document/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/History/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Texmacs/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Scheme/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/*.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Verbatim/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Tex/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Edit/*.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/*.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/*.cpp"
  "${Vau_SOURCE_DIR}/src/Scheme/Scheme/*.cpp"
  "${Vau_SOURCE_DIR}/src/Scheme/Guile/*.cpp"
  "${Vau_SOURCE_DIR}/src/System/Misc/data_cache.cpp"
  "${Vau_SOURCE_DIR}/src/System/Misc/fast_alloc.cpp"
  "${Vau_SOURCE_DIR}/src/System/Boot/preferences.cpp"
  "${Vau_SOURCE_DIR}/src/System/Classes/*.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/*.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/make_file.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/tm_ostream.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/file.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/web_files.cpp"
#  "${Vau_SOURCE_DIR}/src/Texmacs/Data/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Texmacs/Server/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Texmacs/Window/*.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/*.cpp"
  "${Vau_SOURCE_DIR}/src/Vau/*.cpp"
  )

file (GLOB_RECURSE Vau_Std_Plugins_SRCS
#  "${Vau_SOURCE_DIR}/src/Plugins/Axel/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Bibtex/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Cairo/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Database/*.cpp"
  "${Vau_SOURCE_DIR}/src/Plugins/Freetype/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Pdf/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Pdf/*.c"
#  "${Vau_SOURCE_DIR}/src/Plugins/Ghostscript/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Imlib2/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Ispell/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Metafont/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/LaTeX_Preview/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Mplayer/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Openssl/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Sqlite3/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Updater/*.cpp"
)


set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -include ${CMAKE_SOURCE_DIR}/src/System/config.h")

#add_subdirectory (Vau)

set (TeXmacs_All_SRCS ${Vau_Base_SRCS} ${Vau_Std_Plugins_SRCS})

#include_directories (${TeXmacs_Include_Dirs})

include_directories (${TeXmacs_Include_Dirs})

add_executable (Vau)

set_target_properties (Vau
        PROPERTIES
        OUTPUT_NAME Vau)

target_link_libraries (Vau PRIVATE PkgConfig::Guile)
target_link_libraries (Vau PRIVATE Freetype::Freetype)
target_link_libraries (Vau PRIVATE Iconv::Iconv)

target_sources (Vau PRIVATE ${TeXmacs_All_SRCS})

target_link_libraries (Vau PRIVATE
        "-framework ApplicationServices"
        "-framework CoreFoundation"
        "-framework Security"
        "-framework Carbon"
        "-framework AppKit"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreGraphics"
        "-framework CoreServices"
        "-framework CoreText"
        "-framework Foundation"
        "-framework ImageIO"
        )

