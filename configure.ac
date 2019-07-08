#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([brica2], [1.0.0], [kotone {at} sfc.keio.ac.jp])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([include/catch.hpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile include/Makefile tests/Makefile])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AX_CXXFLAGS_WARN_ALL
AX_CXX_COMPILE_STDCXX([14])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([dup2 memset])

AC_OUTPUT
