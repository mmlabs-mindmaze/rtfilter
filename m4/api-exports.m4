
AC_DEFUN([AC_DEF_API_EXPORT_ATTRS],
[AC_REQUIRE([gl_VISIBILITY])
AC_REQUIRE([AC_CANONICAL_HOST])
case $host in
	*win32* | *mingw32* | *cygwin* | *windows*)
		os_support=win32
		;;
	*)
		os_support=other
		;;
esac
if test $HAVE_VISIBILITY = 1 -a $os_support !=  "win32"; then
     AC_DEFINE(LOCAL_FN, [__attribute__ ((visibility ("hidden")))],
     	[attribute of the non-exported symbols])
     AC_DEFINE(API_EXPORTED, [__attribute__ ((visibility ("default")))],
     	[attribute of the symbols exported in the API])
else
     AC_DEFINE(LOCAL_FN, [], [attribute of the non-exported symbols])
     AC_DEFINE(API_EXPORTED, [], [attribute of the symbols exported in the API])
fi
])

