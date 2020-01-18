# - Try to find LIBWEBP
#
# Once done this will define
#
#  LIBWEBP_FOUND - System has LIBWEBP
#  LIBWEBP_INCLUDE_DIR - The LIBWEBP include directory
#  LIBWEBP_LIBRARY - The library needed to use LIBWEBP
#  LIBWEBP_DEFINITIONS - Compiler switches required for using LIBWEBP

# Copyright (c) 2009, Pau Garcia i Quiles <pgquiles@elpauer.org>
# Based off FindLibXml2.cmake from CMake 2.6.4 by Alexander Neundorf <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBWEBP_INCLUDE_DIR AND LIBWEBP_LIBRARIES)
   # in cache already
   SET(LIBWEBP_FIND_QUIETLY TRUE)
ENDIF (LIBWEBP_INCLUDE_DIR AND LIBWEBP_LIBRARIES)

FIND_PATH(LIBWEBP_INCLUDE_DIR webp/decode.h)

FIND_LIBRARY(LIBWEBP_LIBRARY NAMES webp)

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBWEBP_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBWEBP DEFAULT_MSG LIBWEBP_LIBRARY LIBWEBP_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBWEBP_INCLUDE_DIR LIBWEBP_LIBRARIES)
