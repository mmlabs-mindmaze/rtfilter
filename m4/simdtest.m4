# AC_SUPPORT_INSTR_SET(INSTRSET, SEARCH-FLAGS, [INCLUDES], [CODES],
#                      [ISFLAGS], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------
# Search for support of a instruction set, if it's not already available.
AC_DEFUN([AC_SUPPORT_INSTR_SET],
[AS_VAR_PUSHDEF([ac_Search], [ac_cv_search_support_$1])dnl
AC_CACHE_CHECK([for support of $1], [ac_Search],
[ac_support_instr_search_save_CFLAGS=$CFLAGS;
AC_LANG_CONFTEST([AC_LANG_PROGRAM([$3],
                                  [$4])])
for ac_flag in '' $2; do
  if test -z "$ac_flag"; then
    ac_res="none required"
  else
    ac_res=$ac_flag
    CFLAGS="$ac_flag $ac_support_instr_search_save_CFLAGS"
  fi
  AC_LINK_IFELSE([], [AS_VAR_SET([ac_Search], [$ac_res])])
  AS_VAR_SET_IF([ac_Search], [break])
done
AS_VAR_SET_IF([ac_Search], , [AS_VAR_SET([ac_Search], [no])])
rm conftest.$ac_ext
CFLAGS=$ac_support_instr_search_save_CFLAGS])
AS_VAR_COPY([ac_res], [ac_Search])
AS_IF([test "$ac_res" != no],
      [test "$ac_res" = "none required" || $1_CFLAGS="$ac_res"
      $5], [$6])
AS_VAR_POPDEF([ac_Search])dnl
])


# AC_SUPPORT_SSE([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                [TEST-OTHER-CFLAG], SEARCH-OTHER-CFLAGS)
# --------------------------------------------------------
# Search for support SSE compiler flag if supported
AC_DEFUN([AC_SUPPORT_SSE],
[AS_VAR_IF($3, [yes], [test_sse_cflags="-msse $4"], [test_sse_cflags=""])
 AC_SUPPORT_INSTR_SET(SSE, $test_sse_cflags, [#include <xmmintrin.h>],
       [[__m128 a, b, c;
        a = _mm_set1_ps(1.0f);
  	b = _mm_set1_ps(0.0f);
	c = _mm_add_ps(a, b);]],
       [AC_DEFINE([SUPPORT_SSE_SET], [1], [Define to built SSE support])
	AC_SUBST([SSE_CFLAGS])
	$1], [$2])])
])


# AC_SUPPORT_SSE2([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                 [TEST-OTHER-CFLAG], SEARCH-OTHER-CFLAGS)
# --------------------------------------------------------
# Search for support SSE2 compiler flag if supported
AC_DEFUN([AC_SUPPORT_SSE2],
[AS_VAR_IF($3, [yes], [test_sse2_cflags="-msse2 $4"], [test_sse2_cflags=""])
 AC_SUPPORT_INSTR_SET(SSE2, $test_sse2_cflags, [#include <emmintrin.h>],
       [[__m128d a, b, c;
        a = _mm_set1_pd(1.0);
  	b = _mm_set1_pd(0.0);
	c = _mm_add_pd(a, b);]],
       [AC_DEFINE([SUPPORT_SSE2_SET], [1], [Define to built SSE2 support])
        AC_SUBST([SSE2_CFLAGS])
	$1], [$2])])
])


# AC_SUPPORT_SSE3([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
#                 [TEST-OTHER-CFLAG], SEARCH-OTHER-CFLAGS)
# --------------------------------------------------------
# Search for support SSE3 compiler flag if supported
AC_DEFUN([AC_SUPPORT_SSE3],
[AS_VAR_IF($3, [yes], [test_sse3_cflags="-msse3 $4"], [test_sse3_cflags=""])
 AC_SUPPORT_INSTR_SET(SSE3, $test_sse3_cflags, [#include <pmmintrin.h>],
       [[__m128 a, b, c;
        a = _mm_set1_ps(1.0f);
  	b = _mm_movehdup_ps(a);
	c = _mm_add_ps(a, b);]],
       [AC_DEFINE([SUPPORT_SSE3_SET], [1], [Define to built SSE3 support])
        AC_SUBST([SSE3_CFLAGS])
	$1], [$2])])
])


# AC_CHECK_CPUID([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# --------------------------------------------------------
# Check that the host can use cpuid intrinsics
AC_DEFUN([AC_CHECK_CPUID],
 [AC_REQUIRE([AC_PROG_CC])
  AC_MSG_CHECKING([for cpuid instruction])
  AC_TRY_COMPILE(
        [#include <cpuid.h>],
        [__get_cpuid(1, 0, 0, 0, 0);],
        gl_cv_cc_cpuid=yes, gl_cv_cc_cpuid=no)
  AS_IF([test "$gl_cv_cc_cpuid" != no],
        [AC_DEFINE(HAVE_CPUID, [1], [Define to 1 if cpuid can be used])
         $1], [$2])
  AC_MSG_RESULT([$gl_cv_cc_cpuid])
 ])

