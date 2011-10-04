AC_INIT([ubd],[1.0],[schneider@blinkenlichts.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
CFLAGS="$CFLAGS -g -Wall -Wextra"

AC_CHECK_HEADER(libutil.h,LDFLAGS="$LDFLAGS -lbsd",
  AC_MSG_ERROR(libutil.h not found. Please install libbsd-dev.))
PKG_CHECK_MODULES(GLIB, glib-2.0 > 2.0.0,
  [LIBS="$LIBS $GLIB_LIBS" CFLAGS="$CFLAGS $GLIB_CFLAGS"],
    AC_MSG_ERROR(Cannot find glib: Is glib-config in path?))
PKG_CHECK_MODULES(GIO, gio-2.0 > 2.0.0,
  [LIBS="$LIBS $GIO_LIBS" CFLAGS="$CFLAGS $GIO_CFLAGS"],
    AC_MSG_ERROR(Cannot find gio: Is gio-config in path?))
PKG_CHECK_MODULES(MXML, mxml >= 2.6,
    [LIBS="$LIBS $MXML_LIBS" CFLAGS="$CFLAGS $MXML_CFLAGS"])
PKG_CHECK_MODULES(GTHREAD, gthread-2.0,
    [LIBS="$LIBS $GTHREAD_LIBS" CFLAGS="$CFLAGS $GTHREAD_CFLAGS"])
PKG_CHECK_MODULES(AVAHIGLIB, avahi-glib >= 0.6.22,
    [LIBS="$LIBS $AVAHIGLIB_LIBS" CFLAGS="$CFLAGS $AVAHIGLIB_CFLAGS"])
PKG_CHECK_MODULES(AVAHICLIENT, avahi-client >= 0.6.22,
    [LIBS="$LIBS $AVAHICLIENT_LIBS" CFLAGS="$CFLAGS $AVAHICLIENT_CFLAGS"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
