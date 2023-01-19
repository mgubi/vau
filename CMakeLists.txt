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
#file (GLOB_RECURSE Vau_Base_SRCS
#  "${Vau_SOURCE_DIR}/src/Data/Drd/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Observers/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/String/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Document/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/History/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Parser/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Tree/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/generic.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Generic/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Texmacs/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Scheme/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Xml/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Data/Convert/Verbatim/*.cpp"
##  "${Vau_SOURCE_DIR}/src/Data/Convert/Tex/*.cpp"
##  "${Vau_SOURCE_DIR}/src/Edit/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Graphics/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Kernel/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Scheme/Scheme/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Scheme/Guile/*.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Misc/data_cache.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Misc/fast_alloc.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Boot/preferences.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Classes/*.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Language/*.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Files/make_file.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Files/tm_ostream.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Files/web_files.cpp"
#  "${Vau_SOURCE_DIR}/src/System/Files/file.cpp"
##  "${Vau_SOURCE_DIR}/src/Texmacs/Data/*.cpp"
##  "${Vau_SOURCE_DIR}/src/Texmacs/Server/*.cpp"
##  "${Vau_SOURCE_DIR}/src/Texmacs/Window/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Typeset/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Vau/*.cpp"
#)
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
#  "${Vau_SOURCE_DIR}/src/Graphics/Renderer/basic_renderer.cpp"
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
  "${Vau_SOURCE_DIR}/src/Scheme/Guile/guile_tm.cpp"
#  "${Vau_SOURCE_DIR}/src/Scheme/Scheme/glue.cpp"
  "${Vau_SOURCE_DIR}/src/Scheme/Scheme/object.cpp"
  "${Vau_SOURCE_DIR}/src/System/Boot/preferences.cpp"
  "${Vau_SOURCE_DIR}/src/System/Classes/tm_timer.cpp"
  "${Vau_SOURCE_DIR}/src/System/Classes/url.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/file.cpp"
  "${Vau_SOURCE_DIR}/src/System/Files/make_file.cpp"
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
  "${Vau_SOURCE_DIR}/src/Vau/vau_main.cpp"
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
#  "${Vau_SOURCE_DIR}/src/Plugins/Pdf/*.cpp"
#  "${Vau_SOURCE_DIR}/src/Plugins/Pdf/*.c"
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


set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -include ${CMAKE_SOURCE_DIR}/src/System/config.h")

set (TeXmacs_All_SRCS ${Vau_Base_SRCS} ${Vau_Std_Plugins_SRCS})

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

