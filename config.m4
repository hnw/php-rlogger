dnl $Id$
dnl config.m4 for extension rlog

LIBRLOG_LIBDIR="src"

PHP_ARG_ENABLE(rlog, whether to enable rlog support,
[  --enable-rlog               Enable rlog support])

PHP_ARG_WITH(librlog-dir,  for librlog,
[  --with-librlog-dir[=DIR]    Set the path to librlog install prefix.], yes)

if test "$PHP_RLOG" != "no"; then
  AC_MSG_CHECKING([for librlog location])

  dnl Write more examples of tests here...

  dnl # --with-rlog -> check with-path
  SEARCH_PATH="/usr/local /usr"     # you might want to change this
  SEARCH_FOR="/src/librlog.h"
  if test -r $PHP_LIBRLOG_DIR/$SEARCH_FOR; then # path given as parameter
    LIBRLOG_DIR=$PHP_LIBRLOG_DIR
  else # search default path list
    AC_MSG_CHECKING([for rlog files in default path])
    for i in $SEARCH_PATH ; do
      if test -r $i/$SEARCH_FOR; then
        LIBRLOG_DIR=$i
        AC_MSG_RESULT(found in $i)
      fi
    done
  fi

  if test -z "$LIBRLOG_DIR"; then
    AC_MSG_RESULT([not found])
    AC_MSG_ERROR([Please reinstall the librlog])
  fi

  dnl # --with-rlog -> add include path
  PHP_ADD_INCLUDE($LIBRLOG_DIR/src)

  dnl # --with-rlog -> check for lib and symbol presence
  LIBNAME=rlog # you may want to change this
  LIBSYMBOL=rlog_open # you most likely want to change this 

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LIBRLOG_DIR/$LIBRLOG_LIBDIR, RLOG_SHARED_LIBADD)
    AC_DEFINE(HAVE_RLOGLIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong rlog lib version or lib not found])
  ],[
    -L$LIBRLOG_DIR/$LIBRLOG_LIBDIR -lm
  ])

  PHP_SUBST(RLOG_SHARED_LIBADD)

  PHP_NEW_EXTENSION(rlog, rlog.c, $ext_shared)
fi
