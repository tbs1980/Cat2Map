# Try to find gnu scientific library HEALPix
# See
# http://healpix.sourceforge.net
#
# Usage:
#   find_package(HEALPix [REQUIRED] [QUIET] )
#
# Once run this will define:
#
# HEALPix`_FOUND       = system has HEALPix lib
#
# HEALPix_LIBRARIES   = full path to the libraries
#
# HEALPix_INCLUDE_DIR      = where to find headers
#
# HEALPix_ROOT = search path
# --------------------------------

find_path(HEALPix_INCLUDE_DIR NAMES healpix_base.h PATHS ${HEALPix_ROOT} PATH_SUFFIXES include)

find_library(HEALPix_CXX_LIB NAMES libhealpix_cxx.a PATHS ${HEALPix_ROOT} PATH_SUFFIXES lib)
find_library(HEALPix_CXX_SUPPORT_LIB NAMES libcxxsupport.a PATHS ${HEALPix_ROOT}  PATH_SUFFIXES lib)
find_library(HEALPix_SHARP_LIB NAMES libsharp.a PATHS ${HEALPix_ROOT} PATH_SUFFIXES lib)
find_library(HEALPix_CUTILS_LIB NAMES  libc_utils.a PATHS ${HEALPix_ROOT} PATH_SUFFIXES lib)
find_library(HEALPix_FFTPACK_LIB NAMES libfftpack.a PATHS ${HEALPix_ROOT} PATH_SUFFIXES lib)

set(HEALPix_LIBRARIES ${HEALPix_CXX_LIB}
    ${HEALPix_CXX_SUPPORT_LIB} ${HEALPix_PSHT_LIB}
    ${HEALPix_CUTILS_LIB} ${HEALPix_FFTPACK_LIB})

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(HEALPix DEFAULT_MSG HEALPix_INCLUDE_DIR HEALPix_LIBRARIES)

mark_as_advanced(HEALPix_INCLUDE_DIR HEALPix_LIBRARIES)
