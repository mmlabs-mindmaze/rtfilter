# CC_WARNFLAGS()
#-------------------------------------------
AC_DEFUN([CC_WARNFLAGS],
[AC_ARG_ENABLE([warn-all],
              [AS_HELP_STRING([--enable-warn-all], [turn on all warnings (default: yes)])],
	      [case $enableval in
                 yes|no|error) ;;
                 *) AC_MSG_ERROR([bad value $enableval for enable-warn-all option]) ;;
               esac
               mm_warnings=$enableval], [mm_warnings=yes])

case $mm_warnings in
	yes) WARNFLAGS="-Wall -Wextra -Wshadow -Wstrict-prototypes -Wmissing-prototypes" ;;
	error) WARNFLAGS="-Wall -Wextra -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Werror" ;;
	no) WARNFLAGS="" ;;
esac

AC_SUBST([WARNFLAGS])
]) #CC_WARNINGS
