#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([mem], [v0.1], [rfried.dev@gmail.com])
AC_CONFIG_SRCDIR([mem.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CC_C99

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([memset memcpy open close mmap munmap read write])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

