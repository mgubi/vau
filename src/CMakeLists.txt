set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Guile REQUIRED guile-1.8 gmp IMPORTED_TARGET)
find_package(Freetype REQUIRED)
find_package(Iconv REQUIRED)
add_executable(Vau)
set_target_properties(Vau
        PROPERTIES
        OUTPUT_NAME VauQt)
target_link_libraries(Vau PRIVATE PkgConfig::Guile)
target_link_libraries(Vau PRIVATE Freetype::Freetype)
target_link_libraries(Vau PRIVATE Iconv::Iconv)
file(GLOB_RECURSE subdirs LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*")
#message(STATUS ${subdirs})
foreach(subdir ${subdirs})
    if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
        target_include_directories(Vau PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
    endif ()
endforeach()
add_subdirectory(Data)
add_subdirectory(Graphics)
add_subdirectory(Kernel)
add_subdirectory(Plugins)
add_subdirectory(System)
add_subdirectory(Typeset)
add_subdirectory(Scheme)
set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -include ${CMAKE_SOURCE_DIR}/src/System/config.h")
add_subdirectory(Vau)
target_link_libraries(Vau PRIVATE
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