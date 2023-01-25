#[=======================================================================[.rst:
FindMuPDF
------------

Find the MuPDF includes and library.

Imported Targets
^^^^^^^^^^^^^^^^

.. versionadded:: 1.00

This module defines the following :prop_tgt:`IMPORTED` target:

``MuPDF::MuPDF``
  The MuPDF library, if found

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``MUPDF_FOUND``
  true if the Freetype headers and libraries were found
``MUPDF_INCLUDE_DIRS``
  directories containing the MuPDF headers. 
``MUPDF_LIBRARIES``
  the library to link against
``MUPDF_VERSION_STRING``
  the version of freetype found


Hints
^^^^^

The user may set the environment variable ``MUPDF_DIR`` to the root
directory of a Freetype installation.
#]=======================================================================]

# Created by Massimiliano Gubinelli.

set(MUPDF_FIND_ARGS
  HINTS
    ENV MUPDF_DIR
)

find_path(
    MUPDF_INCLUDE_DIR
    pdf.h
    ${MUPDF_FIND_ARGS}
    PATH_SUFFIXES
      include/mupdf
  )
  
  
  if(NOT MUPDF_LIBRARY)
    find_library(MUPDF_LIBRARY
      NAMES
        mupdf
        ${MUPDF_FIND_ARGS}
        PATH_SUFFIXES
        lib
    )
    find_library(MUPDF_THIRD_LIBRARY
      NAMES
        mupdf-third
        ${MUPDF_FIND_ARGS}
        PATH_SUFFIXES
        lib
    )
  else()
    # on Windows, ensure paths are in canonical format (forward slahes):
    file(TO_CMAKE_PATH "${MUPDF_LIBRARY}" MUPDF_LIBRARY)
  endif()
  
  unset(MUPDF_FIND_ARGS)

  # set the user variables
if(MUPDF_INCLUDE_DIR)
  set(MUPDF_INCLUDE_DIRS ${MUPDF_INCLUDE_DIR})
endif()

set(MUPDF_LIBRARIES ${MUPDF_LIBRARY} ${MUPDF_THIRD_LIBRARY})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  MuPDF
  REQUIRED_VARS
    MUPDF_LIBRARIES
    MUPDF_INCLUDE_DIRS
  VERSION_VAR
    MUPDF_VERSION_STRING
)

if(MuPDF_FOUND)
  if(NOT TARGET MuPDF::MuPDF)
    add_library (MuPDF::MuPDF UNKNOWN IMPORTED)
    set_target_properties(MuPDF::MuPDF PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${MUPDF_INCLUDE_DIRS}"
      IMPORTED_LOCATION "${MUPDF_LIBRARY}")
  endif()

  if(NOT TARGET MuPDF::MuPDF-third)
    add_library (MuPDF::MuPDF-third UNKNOWN IMPORTED)
    set_target_properties(MuPDF::MuPDF-third PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${MUPDF_INCLUDE_DIRS}"
      IMPORTED_LOCATION "${MUPDF_THIRD_LIBRARY}")
  endif()
endif()

