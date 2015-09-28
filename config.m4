dnl $Id$
dnl config.m4 for extension rlogger

RLOGD_LIBDIR="src"

PHP_ARG_ENABLE(rlogger, whether to enable rlogd support,
[  --enable-rlogger        Enable rlogd support])

PHP_ARG_WITH(rlogd-src-dir,  for rlogd source directory,
[  --with-rlogd-src-dir[=DIR]    Set the path to rlogd source directory.], yes)

if test "$PHP_RLOGGER" != "no"; then
  AC_MSG_CHECKING([for rlogd source location])

  dnl Write more examples of tests here...

  dnl # --with-rlog -> check with-path
  SEARCH_FOR="/src/librlog.h"
  if test -r $PHP_RLOGD_SRC_DIR/$SEARCH_FOR; then # path given as parameter
    RLOGD_SRC_DIR=$PHP_RLOGD_SRC_DIR
  fi

  echo $PHP_RLOGD_SRC_DIR/$SEARCH_FOR

  if test -z "$RLOGD_SRC_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please specify collect path for --with-rlogd-src-dir])
  fi

  dnl # --with-rlog -> add include path
  PHP_ADD_INCLUDE($RLOGD_SRC_DIR/src)

  dnl # --with-rlog -> check for lib and symbol presence
  LIBNAME=rlog # you may want to change this
  LIBSYMBOL=rlog_open # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $RLOGD_SRC_DIR/$RLOGD_LIBDIR, RLOGGER_SHARED_LIBADD)
    AC_DEFINE(HAVE_RLOGLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong rlogd lib version or lib not found])
  ],[
    -L$RLOGD_SRC_DIR/$RLOGD_LIBDIR -lm
  ])

  PHP_SUBST(RLOGGER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(rlogger, rlogger.c, $ext_shared)
fi
