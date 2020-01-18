# - Try to find LIBWMF
#
# Once done this will define
#
#  LIBWMF_FOUND - System has LIBWMF
#  LIBWMF_INCLUDE_DIR - The LIBWMF include directory
#  LIBWMF_LIBRARIES - The libraries needed to use LIBWMF
#  LIBWMF_DEFINITIONS - Compiler switches required for using LIBWMF

# Copyright (c) 2009, Pau Garcia i Quiles <pgquiles@elpauer.org>
# Based off FindLibXml2.cmake from CMake 2.6.4 by Alexander Neundorf <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (LIBWMF_INCLUDE_DIR AND LIBWMF_LIBRARIES)
   # in cache already
   SET(LIBWMF_FIND_QUIETLY TRUE)
ENDIF (LIBWMF_INCLUDE_DIR AND LIBWMF_LIBRARIES)

FIND_PATH(LIBWMF_INCLUDE_DIR libwmf/api.h)

FIND_LIBRARY(LIBWMF_wmf_LIBRARY NAMES wmf)

FIND_LIBRARY(LIBWMF_wmflite_LIBRARY NAMES wmflite)

set(LIBWMF_LIBRARIES ${LIBWMF_wmf_LIBRARY} ${LIBWMF_wmflite_LIBRARY})

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBWMF_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBWMF DEFAULT_MSG LIBWMF_LIBRARIES LIBWMF_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBWMF_INCLUDE_DIR LIBWMF_LIBRARIES)
