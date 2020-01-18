# - Try to find LIBMPQ
#
# Once done this will define
#
#  LIBMPQ_FOUND - System has LIBMPQ
#  LIBMPQ_INCLUDE_DIR - The LIBMPQ include directory
#  LIBMPQ_LIBRARIES - The libraries needed to use LIBMPQ
#  LIBMPQ_DEFINITIONS - Compiler switches required for using LIBMPQ

# Copyright (c) 2011, Ni Hui <shuizhuyuanluo@126.com>
# Based off FindLibXml2.cmake from CMake 2.6.4 by Alexander Neundorf <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_LIBMPQ libmpq QUIET)
SET(LIBMPQ_DEFINITIONS ${PC_LIBMPQ_CFLAGS_OTHER})

FIND_PATH(LIBMPQ_INCLUDE_DIR NAMES libmpq/mpq.h
    HINTS
    ${PC_LIBMPQ_INCLUDEDIR}
    ${PC_LIBMPQ_INCLUDE_DIRS}
)

FIND_LIBRARY(LIBMPQ_LIBRARIES NAMES mpq
    HINTS
    ${PC_LIBMPQ_LIBDIR}
    ${PC_LIBMPQ_LIBRARY_DIRS}
)

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBMPQ_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBMPQ DEFAULT_MSG LIBMPQ_LIBRARIES LIBMPQ_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBMPQ_INCLUDE_DIR LIBMPQ_LIBRARIES)
