#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(all_distance_sketch, 0.1, eliavbuh@gmail.com)
AM_INIT_AUTOMAKE(all_distance_sketch, 0.1)
AC_OUTPUT(Makefile src/Makefile)
AC_CONFIG_SRCDIR([src/all_distance_sketch.h])

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h, gtest.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday])

# Add links

AC_CONFIG_MACRO_DIRS([./m4])

# Building a lib
AC_PROG_RANLIB
AM_PROG_AR

AC_OUTPUT
