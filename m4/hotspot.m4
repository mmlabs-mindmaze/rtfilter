AC_DEFUN([AC_DEF_HOTSPOT_ATTR],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_MSG_CHECKING([for hot spot attribute])
  HOTATTR=""
  save_CFLAGS=$CFLAGS
  CFLAGS=-Werror
  AC_TRY_COMPILE(
        [int dummyfunc (void) __attribute__ ((hot));],
        [],
        gl_cv_cc_hotspot=yes,
        gl_cv_cc_hotspot=no)
  if test $gl_cv_cc_hotspot = yes; then
      HOTATTR="__attribute__ ((hot))"
  fi
  CFLAGS=$save_CFLAGS
  AC_MSG_RESULT([$gl_cv_cc_hotspot])
  AC_DEFINE_UNQUOTED([HOTSPOT], [$HOTATTR],
    [Define to the supported attribute to specify a hot spot])
])

