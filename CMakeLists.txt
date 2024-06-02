cmake_minimum_required(VERSION 3.24)

project (Vau CXX C) # check for C/C++ compiler

# The name of our project is "Vau". CMakeLists files in this project can 
# refer to the root source directory of the project as ${Vau_SOURCE_DIR} and 
# to the root binary directory of the project as ${Vau_BINARY_DIR}. 

set (CMAKE_CXX_STANDARD 11)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CMAKE_MODULE_PATH ${Vau_SOURCE_DIR}/devel)


### --------------------------------------------------------------------
### Define some basic config variables
### --------------------------------------------------------------------

set (VERSION_MAJOR "2")
set (VERSION_MINOR "1")
set (VERSION_BUILD "2")
set (TEXMACS_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD})

set (TEXMACS_SOURCES ${Vau_SOURCE_DIR})

set (DEVEL_VERSION ${TEXMACS_VERSION})
set (DEVEL_RELEASE "1")
set (STABLE_VERSION ${TEXMACS_VERSION})
set (STABLE_RELEASE "1")
set (TEXMACS_REVISION ${TEXMACS_VERSION})

### --------------------------------------------------------------------
### Options
### --------------------------------------------------------------------

option (PDFHUMMUS_NO_TIFF "Disable TIFF support in PDF Hummus" ON)
option (PDFHUMMUS_NO_DCT "Disable DCT support in PDF Hummus" ON)
option (PDFHUMMUS_NO_PNG "Disable PNG support in PDF Hummus" ON)
option (USE_FREETYPE "use Freetype" ON)
option (LINKED_FREETYPE "linked Freetype" ON)
option (MUPDF_RENDERER "Enable MuPDF" ON)

### --------------------------------------------------------------------
### Include standard modules
### --------------------------------------------------------------------

include (CheckFunctionExists)
#include (CheckLibraryExists)
include (CheckIncludeFile)
include (CheckTypeSize)
#include (FindPkgConfig)
#include (CheckCSourceCompiles)
#include (FindX11)


### --------------------------------------------------------------------
### Check for standard functionalities
### --------------------------------------------------------------------

check_type_size (FILE HAVE_FILE)
check_type_size (intptr_t HAVE_INTPTR_T)
check_type_size (time_t HAVE_TIME_T)
check_include_file (inttypes.h HAVE_INTTYPES_H)
check_include_file (memory.h HAVE_MEMORY_H)
check_include_file (stdlib.h HAVE_STDLIB_H)
check_include_file (strings.h HAVE_STRINGS_H)
check_include_file (string.h HAVE_STRING_H)
check_include_file (sys/stat.h HAVE_SYS_STAT_H)
check_include_file (unistd.h HAVE_UNISTD_H)
#check_include_file (X11/Xlib.h HAVE_X11_XLIB_H)
#check_include_file (X11/Xutil.h HAVE_X11_XUTIL_H)
check_include_file (pty.h HAVE_PTY_H)
check_include_file (util.h HAVE_UTIL_H)
check_function_exists (gettimeofday HAVE_GETTIMEOFDAY)

check_type_size ("void*" SIZE_OF_VOIDP)

set (CONFIG_WORD_LENGTH "4")
set (CONFIG_WORD_LENGTH_INC "3")
set (CONFIG_WORD_MASK "0xfffffffc")
set (CONFIG_MAX_FAST "260 // WORD_LENGTH more than power of 2")

set (CONFIG_HOST_OS "linux-gnu") # TODO: should not be hard-coded
set (CONFIG_HOST_VENDOR "pc") # TODO: should not be hard-coded
set (CONFIG_HOST_CPU ${CMAKE_SYSTEM_PROCESSOR})

if(EMSCRIPTEN)
  message (STATUS "Final adjustments for WASM host")
  set (CONFIG_OS "WASM")
  set (CONFIG_OS_SUFFIX "wasm")
  set (CONFIG_HOST_OS "wasm-emscripten")
  set (CONFIG_HOST_VENDOR "pc")
  set (CONFIG_HOST_CPU ${CMAKE_SYSTEM_PROCESSOR})
  set (SIZE_OF_VOIDP 8) ## 32 bit
  set (HAVE_TIME_T TRUE)
endif(EMSCRIPTEN)

if (SIZE_OF_VOIDP)
  message (STATUS "Check if we are on a 64-bits computer")
  if (${SIZE_OF_VOIDP} EQUAL 8)
    set (CONFIG_WORD_LENGTH "8")
    set (CONFIG_WORD_LENGTH_INC "7")
    set (CONFIG_WORD_MASK "0xfffffffffffffff8")
    set (CONFIG_MAX_FAST "264 // WORD_LENGTH more than power of 2")
  endif (${SIZE_OF_VOIDP} EQUAL 8)
else ()
  set (SIZE_OF_VOIDP 4)
endif () 


### --------------------------------------------------------------------
###  Test for Libraries
### --------------------------------------------------------------------

if (EMSCRIPTEN)

else (EMSCRIPTEN)
#  find_package (PkgConfig REQUIRED)
  find_package (PNG)
  find_package (Iconv)
  find_package (ZLIB)
  find_package (JPEG)
  #pkg_check_modules (Guile REQUIRED guile-1.8 gmp IMPORTED_TARGET)
  find_package (Freetype REQUIRED)
  find_package (Iconv REQUIRED)
  #include(CMakePrintHelpers)
  #cmake_print_variables(MUPDF_INCLUDE_DIR MUPDF_THIRD_LIBRARY_RELEASE MUPDF_LIBRARY_RELEASE)
endif (EMSCRIPTEN)

find_package (MuPDF REQUIRED)

### --------------------------------------------------------------------
### Generate configure headers
### --------------------------------------------------------------------

set (DEBUG_ASSERT 1)
set (PDF_RENDERER 1)
#set (SIZEOF_VOID_P 8)
set (STDC_HEADERS 1)
set (CONFIG_STD_SETENV "#define STD_SETENV")

set (tm_orig ${Vau_SOURCE_DIR})
set (CONFIG_USER $ENV{USER})
string (TIMESTAMP CONFIG_DATE)
set (tm_devel TeXmacs-${DEVEL_VERSION})
set (tm_stable TeXmacs-${STABLE_VERSION})
set (tm_devel_release ${tm_devel}-${DEVEL_RELEASE})
set (tm_stable_release ${tm_stable}-${STABLE_RELEASE})

set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -include ${Vau_BINARY_DIR}/src/System/config.h")

configure_file (${Vau_SOURCE_DIR}/src/System/config.h.cmake ${Vau_BINARY_DIR}/src/System/config.h)
configure_file (${Vau_SOURCE_DIR}/src/System/tm_configure.hpp.cmake ${Vau_BINARY_DIR}/src/System/tm_configure.hpp)

if (NOT EMSCRIPTEN)
  # we generate Xcode config files only if not configuring Wasm
  configure_file (${Vau_SOURCE_DIR}/platform/macos/Config.xcconfig.cmake ${Vau_SOURCE_DIR}/platform/macos/Config.xcconfig)
endif (NOT EMSCRIPTEN)

### --------------------------------------------------------------------
### the include dirs
### --------------------------------------------------------------------
set (Vau_Include_Dirs
  ${Vau_BINARY_DIR}/src/System
  ${Vau_SOURCE_DIR}/src/Vau
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
  ${Vau_SOURCE_DIR}/src/Scheme/S7
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
#  ${Vau_BINARY_DIR}/src/System/ 
  ${Vau_SOURCE_DIR}/src
  ${Vau_BINARY_DIR}/src/System
)

### --------------------------------------------------------------------
### the main sources
### --------------------------------------------------------------------

set (Vau_Base_SRCS
  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/generic.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/indexation.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/input.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/post_convert.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Scheme/from_scheme.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Scheme/to_scheme.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Texmacs/fromtm.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Texmacs/rewrite_equation_number.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Texmacs/totm.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Texmacs/upgradetm.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Verbatim/verbatim.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/cleanhtml.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/fromxml.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/parsehtml.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/parsexml.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/queryxml.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/xml.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Document/new_data.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Document/new_document.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Document/new_style.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Drd/drd_info.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Drd/drd_mode.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Drd/drd_std.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Drd/tag_info.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Drd/vars.cpp"
  "${Vau_SOURCE_DIR}/src/Data/History/archiver.cpp"
  "${Vau_SOURCE_DIR}/src/Data/History/commute.cpp"
  "${Vau_SOURCE_DIR}/src/Data/History/patch.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/edit_observer.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/highlight_observer.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/ip_observer.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/link.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/list_observer.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/tree_addendum.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/tree_pointer.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/tree_position.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Observers/undo_observer.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/escaped_char_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/identifier_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/inline_comment_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/keyword_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/number_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/operator_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/preprocessor_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Parser/string_parser.cpp"
  "${Vau_SOURCE_DIR}/src/Data/String/analyze.cpp"
  "${Vau_SOURCE_DIR}/src/Data/String/base64.cpp"
  "${Vau_SOURCE_DIR}/src/Data/String/converter.cpp"
  "${Vau_SOURCE_DIR}/src/Data/String/fast_search.cpp"
  "${Vau_SOURCE_DIR}/src/Data/String/universal.cpp"
  "${Vau_SOURCE_DIR}/src/Data/String/wencoding.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_analyze.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_brackets.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_correct.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_cursor.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_math_stats.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_modify.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_search.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_select.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_spell.cpp"
  "${Vau_SOURCE_DIR}/src/Data/Tree/tree_traverse.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/bitmap_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph_analyze.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph_distorted.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph_effected.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph_ops.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph_shrink.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph_transforms.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Bitmap_fonts/glyph_unserif.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Colors/colors.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Colors/true_color.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/charmap.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/compound_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/find_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_database.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_guess.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_protrusion.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_scripts.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_select.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_spacing.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_translate.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/font_wide.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/math_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_bbb.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_bold.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_distorted.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_effected.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_extended.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_italic.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_mono.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_rubber.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_smallcaps.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/poor_stretched.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/recolored_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/smart_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/superposed_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/translator.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/virtual_enhance.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Fonts/virtual_font.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Gui/widget.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Handwriting/learn_handwriting.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Handwriting/poly_line.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Handwriting/recognize_handwriting.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Handwriting/smoothen.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Mathematics/math_tree.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Mathematics/test_math.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Pictures/effect.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Pictures/picture.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Pictures/raster_picture.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Pictures/raster_random.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Pictures/scalable.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Renderer/basic_renderer.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Renderer/brush.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Renderer/page_type.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Renderer/pencil.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Renderer/printer.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Renderer/renderer.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Spacial/enlightened.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Spacial/transformed.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Spacial/triangulated.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Types/curve.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Types/curve_extras.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Types/equations.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Types/frame.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Types/grid.cpp"
  "${Vau_SOURCE_DIR}/src/Graphics/Types/point.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Abstractions/basic.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Abstractions/command.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Abstractions/observer.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Abstractions/player.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/array.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/hashfunc.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/hashmap.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/hashmap_extra.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/hashset.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/hashtree.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/iterator.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/list.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Containers/rel_hashmap.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/modification.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/parse_string.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/path.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/rectangles.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/space.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/string.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/tab.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/tree.cpp"
  "${Vau_SOURCE_DIR}/src/Kernel/Types/tree_label.cpp"
#  "${Vau_SOURCE_DIR}/src/Scheme/Guile/guile_tm.cpp"
  "${Vau_SOURCE_DIR}/src/Scheme/S7/s7_tm.cpp"
  "${Vau_SOURCE_DIR}/src/Scheme/S7/s7.c"
#  "${Vau_SOURCE_DIR}/src/Scheme/Scheme/glue.cpp"
  "${Vau_SOURCE_DIR}/src/Scheme/Scheme/object.cpp"
  "${Vau_SOURCE_DIR}/src/Vau/preferences.cpp"
  "${Vau_SOURCE_DIR}/src/System/Classes/tm_timer.cpp"
  "${Vau_SOURCE_DIR}/src/System/Classes/url.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/file.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Files/make_file.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/tm_ostream.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/web_files.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/dictionary.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/fortran_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/hyphenate.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/impl_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/locale.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/math_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/mathemagix_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/packrat_grammar.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/packrat_parser.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/packrat_serializer.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/prog_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/r_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/scheme_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/scilab_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/text_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Language/verb_language.cpp"
  "${Vau_SOURCE_DIR}/src/System/Misc/data_cache.cpp"
  "${Vau_SOURCE_DIR}/src/System/Misc/fast_alloc.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Animate/animate_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Basic/basic_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Basic/boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Basic/rubber_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Basic/stretch_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Basic/text_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/case_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/composite_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/concat_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/decoration_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/math_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/misc_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/script_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/stack_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Composite/superpose_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Graphics/graphics_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Graphics/grid_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Modifier/art_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Modifier/change_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Modifier/highlight_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Boxes/Modifier/modifier_boxes.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_argument.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_auto.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_compound.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_default.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_docrange.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_document.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_eval.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_expand_as.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_formatting.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_gui.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_hidden.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_locus.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_mark.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_rewrite.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_surround.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/bridge_with.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Bridge/typesetter.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_active.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_animate.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_graphics.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_gui.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_inactive.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_macro.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_math.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_post.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concat_text.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Concat/concater.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Env/env.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Env/env_animate.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Env/env_default.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Env/env_exec.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Env/env_inactive.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Env/env_length.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Env/env_semantics.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Format/format.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Format/formatter.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Format/line_item.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Format/page_item.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Line/lazy_gui.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Line/lazy_paragraph.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Line/lazy_typeset.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Line/lazy_vstream.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Line/line_breaker.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Page/columns_breaker.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Page/make_pages.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Page/new_breaker.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Page/page_breaker.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Page/pager.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Page/skeleton.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Page/vpenalty.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Stack/stacker.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Table/cell.cpp"
  "${Vau_SOURCE_DIR}/src/Typeset/Table/table.cpp"
  "${Vau_SOURCE_DIR}/src/Vau/vau_buffer.cpp"
  "${Vau_SOURCE_DIR}/src/Vau/vau_editor.cpp"
  "${Vau_SOURCE_DIR}/src/Vau/vau_glue.cpp"
  "${Vau_SOURCE_DIR}/src/Vau/vau_lib.cpp"
  "${Vau_SOURCE_DIR}/src/Vau/vau_stuff.cpp"
)

#include(CMakePrintHelpers)
#cmake_print_variables(Vau_Base_SRCS)

file (GLOB_RECURSE Vau_Std_Plugins_SRCS
#  "${Vau_SOURCE_DIR}/src/Plugins/Axel/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Bibtex/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Cairo/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Database/*.cpp"
  "${Vau_SOURCE_DIR}/src/Plugins/Freetype/*.cpp"
  "${Vau_SOURCE_DIR}/src/Plugins/Pdf/*.cpp"
  "${Vau_SOURCE_DIR}/src/Plugins/Pdf/*.c"
  "${Vau_SOURCE_DIR}/src/Plugins/MuPDF/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Ghostscript/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Imlib2/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Ispell/*.cpp"
  "${Vau_SOURCE_DIR}/src/Plugins/Metafont/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/LaTeX_Preview/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Mplayer/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Openssl/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Sqlite3/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Updater/*.cpp"
)

set (Vau_All_SRCS ${Vau_Base_SRCS} ${Vau_Std_Plugins_SRCS})

if (NOT EMSCRIPTEN)
  set (Vau_All_SRCS ${Vau_All_SRCS} "${Vau_SOURCE_DIR}/src/Vau/vau_main.cpp")
else (NOT EMSCRIPTEN)
#  set (Vau_All_SRCS ${Vau_All_SRCS} "${Vau_SOURCE_DIR}/src/Vau/vau_js.cpp")
endif (NOT EMSCRIPTEN)

add_executable (Vau)
target_sources (Vau PRIVATE ${Vau_All_SRCS})
target_include_directories (Vau PRIVATE ${Vau_Include_Dirs})

target_link_libraries (Vau PRIVATE MuPDF::MuPDF)
target_link_libraries (Vau PRIVATE MuPDF::MuPDF-third)

if (EMSCRIPTEN)
  set_target_properties (Vau
        PROPERTIES
        OUTPUT_NAME Vau-wasm)
  target_compile_options (Vau
    PRIVATE
    "SHELL:-include ${Vau_BINARY_DIR}/src/System/config.h"
    "SHELL:-sUSE_FREETYPE=1" "SHELL:-sUSE_ZLIB=1" "SHELL:-sUSE_LIBPNG=1"
  )
  target_link_options (Vau
    PRIVATE
    "SHELL:--no-entry"
    "SHELL:-sMODULARIZE=1"
    "SHELL:--source-map-base /" 
#   "SHELL:-sDISABLE_EXCEPTION_CATCHING=0"
    "SHELL:-sUSE_FREETYPE=1" "SHELL:-s USE_ZLIB=1" "SHELL:-s USE_LIBPNG=1"
    "SHELL:--preload-file ${Vau_SOURCE_DIR}/resources@/Vau"
    "SHELL:--preload-file ${Vau_SOURCE_DIR}/vau_home@/Vau_Home"
#    "SHELL:--emrun"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sSTACK_SIZE=300000"
    "SHELL:-sASSERTIONS=1"
    "SHELL:--js-library ${Vau_SOURCE_DIR}/platform/wasm/mylib.js"
    "SHELL:-l embind"
    -sNO_EXIT_RUNTIME
    -sEXPORT_NAME='libvau'
    -sEXPORTED_RUNTIME_METHODS=[ccall,cwrap,UTF8ToString,lengthBytesUTF8,stringToUTF8,wasmMemory]
#    "-sEXPORTED_FUNCTIONS=[_main,_malloc,_free,_wasm_open_document,_wasm_get_page_png]"
  )
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options (Vau PRIVATE 
      "SHELL:-s ASSERTIONS=1" "SHELL:-s DEMANGLE_SUPPORT=1" "-g3")
    add_link_options (Vau PRIVATE 
      "SHELL:-s ASSERTIONS=1" "SHELL:-s DEMANGLE_SUPPORT=1"  "-g3" "-gsource-map")
  endif()
else (EMSCRIPTEN)
  set_target_properties (Vau
        PROPERTIES
        OUTPUT_NAME Vau)
  #target_link_libraries (Vau PRIVATE PkgConfig::Guile)
  target_link_libraries (Vau PRIVATE Freetype::Freetype)
  target_link_libraries (Vau PRIVATE Iconv::Iconv)
  target_link_libraries (Vau PRIVATE PNG::PNG)
  target_link_libraries (Vau PRIVATE ZLIB::ZLIB)
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
endif (EMSCRIPTEN)

if (EMSCRIPTEN)
  install (FILES ${Vau_SOURCE_DIR}/platform/wasm/Vau.html
    DESTINATION ${Vau_BINARY_DIR}
    PERMISSIONS OWNER_WRITE OWNER_READ
       GROUP_READ WORLD_READ)
endif (EMSCRIPTEN)



### ---------------------------------------------------------------------
### VSCode Support
### ---------------------------------------------------------------------
foreach (DIR ${Vau_Include_Dirs})
  set (VSCODE_INCLUDE_DIRS "${VSCODE_INCLUDE_DIRS}\"${DIR}\",\n                ")
endforeach (DIR ${Vau_Include_Dirs})

configure_file (${Vau_SOURCE_DIR}/devel/c_cpp_properties.json.in
  ${Vau_SOURCE_DIR}/.vscode/c_cpp_properties.json
  @ONLY
)
