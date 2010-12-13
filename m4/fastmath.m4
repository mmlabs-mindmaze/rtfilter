AC_DEFUN([AC_CHECK_FASTMATH_FLAG],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_MSG_CHECKING([for fast math compiler flag])
  fm_save_CFLAGS="$CFLAGS"
  CFLAGS="-ffast-math $CFLAGS"
  FASTMATH_CFLAG=
  AC_TRY_COMPILE(
        [],
        [],
        gl_cv_cc_fastmath=yes,
        gl_cv_cc_fastmath=no)
  CFLAGS="$fm_save_CFLAGS"
  if test $gl_cv_cc_fastmath = yes; then
    FASTMATH_CFLAG="-ffast-math"
  fi
  AC_MSG_RESULT([$gl_cv_cc_fastmath])
  AC_SUBST([FASTMATH_CFLAG])
])


