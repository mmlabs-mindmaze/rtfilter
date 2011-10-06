# - If the library source code has changed at all since the last update,
# then increment revision.
# - If any interfaces have been added, removed, or changed since the last
# update, increment current, and set revision to 0.
# - If any interfaces have been added since the last public release, then
# increment age.
# - If any interfaces have been removed since the last public release, then
# set age to 0.

m4_define([lib_current],1)
m4_define([lib_revision],0)
m4_define([lib_age],0)


# Setup Automake
AC_INIT(rtfilter, [lib_current.lib_revision],[nicolas.bourdaud@epfl.ch])
AC_CONFIG_SRCDIR([src/filter.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config/config.h])

# Initialize Automake
AM_INIT_AUTOMAKE([subdir-objects no-dist-gzip dist-bzip2 color-tests])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

AC_PROG_CC_C99
AM_PROG_CC_C_O
LT_INIT([win32-dll disable-static])
AC_SUBST(LIBTOOL_DEPS)

AC_DEF_HOTSPOT_ATTR
AC_DEF_API_EXPORT_ATTRS
AC_CHECK_FASTMATH_FLAG
AC_USE_SSEMATH

AC_SUPPORT_SSE([has_sse=yes], [has_sse=no])
AC_SUPPORT_SSE2([has_sse2=yes], [has_sse2=no])
AC_SUPPORT_SSE3([has_sse3=yes], [has_sse3=no])

AC_SEARCH_LIBS([cos], [m], [], AC_MSG_ERROR([The math library cannot be found.]))
AC_CHECK_FUNCS([posix_memalign])

# Test optional library
save_LIBS=$LIBS
AC_SEARCH_LIBS([clock_gettime], [rt], [hasclockfn=true], [hasclockfn=false])
OPTION_LIBS=$LIBS
LIBS=$save_LIBS
AC_SUBST([OPTION_LIBS])
AC_SUBST([LIBS])

AM_CONDITIONAL(HAVE_CLOCK_FN, test "x$hasclockfn" = "xtrue")
AM_CONDITIONAL(BUILD_SSE_SUPPORT, test "x$has_sse" = "xyes")
AM_CONDITIONAL(BUILD_SSE2_SUPPORT, test "x$has_sse2" = "xyes")
AM_CONDITIONAL(BUILD_SSE3_SUPPORT, test "x$has_sse3" = "xyes")

AC_SUBST([CURRENT],[lib_current])
AC_SUBST([REVISION],[lib_revision])
AC_SUBST([AGE],[lib_age])

AC_CONFIG_FILES([Makefile src/Makefile src/rtfilter.pc tests/Makefile doc/Makefile])
AC_OUTPUT

