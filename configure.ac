AC_INIT([ubd],[1.0],[schneider@blinkenlichts.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
CFLAGS="$CFLAGS -g -Wall -Wextra -std=c99"

PKG_CHECK_MODULES(GLIB, glib-2.0 > 2.0.0,
  [LIBS="$LIBS $GLIB_LIBS" CFLAGS="$CFLAGS $GLIB_CFLAGS"],
    AC_MSG_ERROR(Cannot find glib: Is glib-config in path?))
PKG_CHECK_MODULES(GIO, gio-2.0 > 2.0.0,
  [LIBS="$LIBS $GIO_LIBS" CFLAGS="$CFLAGS $GIO_CFLAGS"],
    AC_MSG_ERROR(Cannot find gio: Is gio-config in path?))

PKG_CHECK_MODULES(MXML, mxml >= 2.6,
    [LIBS="$LIBS $MXML_LIBS" CFLAGS="$CFLAGS $MXML_CFLAGS"])

PKG_CHECK_MODULES(JSON, json >= 0.9,
    [LIBS="$LIBS $JSON_LIBS" CFLAGS="$CFLAGS $JSON_CFLAGS"])


#PKG_CHECK_MODULES(GTHREAD, gthread-2.0,
#    [LIBS="$LIBS $GTHREAD_LIBS" CFLAGS="$CFLAGS $GTHREAD_CFLAGS"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
