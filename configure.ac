# If the library source code has changed at all since the last update, then increment
#      revision.
# If any interfaces have been added, removed, or changed since the last update, increment
#      current, and set revision to 0.
# If any interfaces have been added since the last public release, then increment age.
# If any interfaces have been removed since the last public release, then set age to 0.

m4_define([lib_release],0)
m4_define([lib_current],0)
m4_define([lib_revision],0)
m4_define([lib_age],0)

AC_INIT(rtfilter,[lib_release.lib_current.lib_age.lib_revision])
AC_CONFIG_SRCDIR([src/filter.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config/config.h])
AM_INIT_AUTOMAKE

AC_CANONICAL_HOST
AC_CANONICAL_BUILD


AC_PROG_CC_C99
AM_PROG_CC_C_O
LT_INIT([win32-dll])
AC_SUBST(LIBTOOL_DEPS)

AC_HEADER_STDC
AC_HEADER_TIME
AC_C_CONST
AC_C_VOLATILE


# If on windows, -no-undefined should be added to LDFLAGS to produce a DLL
case $host in
	*win32* | *mingw32* | *cygwin* | *windows*)
		os_support=win32
		AM_LDFLAGS="$AM_LDFLAGS -no-undefined"
		;;
	*)
		os_support=other
		;;
esac

AC_SUBST([RELEASE],[lib_release])
AC_SUBST([CURRENT],[lib_current])
AC_SUBST([REVISION],[lib_revision])
AC_SUBST([AGE],[lib_age])
AC_SUBST(AM_LDFLAGS)

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT

