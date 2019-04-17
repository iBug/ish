AC_INIT([OSH_Shell], [1.0], [iBug@users.noreply.github.com])
AM_INIT_AUTOMAKE([foreign -Wall])

AC_PROG_CC

AC_CHECK_HEADERS([stdio.h stdlib.h string.h unistd.h errno.h sys/wait.h])

AC_TYPE_PID_T
AC_TYPE_SIZE_T

AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd execvp open close pipe dup2])

AC_CONFIG_SRCDIR([.])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
