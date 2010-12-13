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
AC_INIT(rtfilter, [lib_current.lib_age.lib_revision],
                  nicolas.bourdaud@epfl.ch)
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

AC_SEARCH_LIBS([cos], [m], [], AC_MSG_ERROR([The math library cannot be found.]))
AC_CHECK_FUNCS([posix_memalign])

AC_SUBST([CURRENT],[lib_current])
AC_SUBST([REVISION],[lib_revision])
AC_SUBST([AGE],[lib_age])

AC_CONFIG_FILES([Makefile src/Makefile src/rtfilter.pc tests/Makefile doc/Makefile])
AC_OUTPUT

