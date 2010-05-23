AC_INIT([ubd],[1.0],[schneider@blinkenlichts.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
CFLAGS="$CFLAGS -Wall -Wextra"
PKG_CHECK_MODULES(GLIB, glib-2.0 > 2.0.0,
  [LIBS="$LIBS $GLIB_LIBS" CFLAGS="$CFLAGS $GLIB_CFLAGS"],
    AC_MSG_ERROR(Cannot find glib: Is glib-config in path?))

PKG_CHECK_MODULES(GIO, gio-2.0 > 2.0.0,
  [LIBS="$LIBS $GIO_LIBS" CFLAGS="$CFLAGS $GIO_CFLAGS"],
    AC_MSG_ERROR(Cannot find gio: Is gio-config in path?))


AC_CHECK_LIB(gthread-2.0, g_thread_init)
AC_CHECK_LIB(gdbm, gdbm_open)
PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.4)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
