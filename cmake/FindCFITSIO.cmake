# Find gnu scientific library CFITSIO (http://www.gnu.org/software/gsl/)
#
# Usage:
# find_package(CFITSIO [REQUIRED] [QUIET] )
#
# Search path variable:
# CFITSIO_ROOT
#
# Once run this will define:
#
# CFITSIO_FOUND       = system has CFITSIO lib
# CFITSIO_LIBRARIES   = full path to the libraries
# CFITSIO_INCLUDE_DIRS = full path to the header files
# CFITSIO_VERSION_STRING - Human readable version number of cfitsio
# CFITSIO_VERSION_MAJOR  - Major version number of cfitsio
# CFITSIO_VERSION_MINOR  - Minor version number of cfitsio
#

if(CFITSIO_ROOT)
    message("CFITSIO_ROOT specified ${CFITSIO_ROOT}")
    find_library(CFITSIO_LIBRARY
        NAMES cfitsio
        PATHS ${CFITSIO_ROOT}
        PATH_SUFFIXES lib lib64
        NO_DEFAULT_PATH
        DOC "CFITSIO library")
    find_path(CFITSIO_INCLUDE_DIR
        NAMES fitsio.h
        PATHS ${CFITSIO_ROOT}
        PATH_SUFFIXES  include include/cfitsio
        NO_DEFAULT_PATH
        DOC "CFITSIO headers")
else(CFITSIO_ROOT)
    find_library(CFITSIO_LIBRARY
        NAMES cfitsio
        HINTS /usr /usr/local
        PATHS /usr /usr/local
        PATH_SUFFIXES lib lib64
        DOC "CFITSIO library")
    find_path(CFITSIO_INCLUDE_DIR
        NAMES fitsio.h
        HINTS /usr /usr/local
        PATHS /usr /usr/local
        PATH_SUFFIXES  include include/cfitsio
        DOC "CFITSIO headers")
endif(CFITSIO_ROOT)

set(CFITSIO_LIBRARIES ${CFITSIO_LIBRARY})
set(CFITSIO_INCLUDE_DIRS ${CFITSIO_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(CFITSIO DEFAULT_MSG CFITSIO_LIBRARIES CFITSIO_INCLUDE_DIRS)

if(CFITSIO_FOUND)

  # Find the version of the cfitsio header
  FILE(READ "${CFITSIO_INCLUDE_DIR}/fitsio.h" FITSIO_H)
  STRING(REGEX REPLACE ".*#define CFITSIO_VERSION[^0-9]*([0-9]+)\\.([0-9]+).*" "\\1.\\2" CFITSIO_VERSION_STRING "${FITSIO_H}")
  STRING(REGEX REPLACE "^([0-9]+)[.]([0-9]+)" "\\1" CFITSIO_VERSION_MAJOR ${CFITSIO_VERSION_STRING})
  STRING(REGEX REPLACE "^([0-9]+)[.]([0-9]+)" "\\2" CFITSIO_VERSION_MINOR ${CFITSIO_VERSION_STRING})
  message(STATUS "found version string ${CFITSIO_VERSION_STRING}")
  message(STATUS "Found CFITSIO ${CFITSIO_VERSION_MAJOR}.${CFITSIO_VERSION_MINOR}: ${CFITSIO_LIBRARIES}")

endif(CFITSIO_FOUND)

mark_as_advanced(CFITSIO_LIBRARIES CFITSIO_INCLUDE_DIRS)
