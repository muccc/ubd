AC_INIT([ubd],[1.0],[schneider@blinkenlichts.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
CFLAGS="$CFLAGS -Wall -Wextra"

PKG_CHECK_MODULES(GNET, gnet-2.0 > 2.0.0,
  [LIBS="$LIBS $GNET_LIBS" CFLAGS="$CFLAGS $GNET_CFLAGS"],
    AC_MSG_ERROR(Cannot find GNet: Is gnet-config in path?))

PKG_CHECK_MODULES(GIO, gio-2.0 > 2.0.0,
  [LIBS="$LIBS $GIO_LIBS" CFLAGS="$CFLAGS $GIO_CFLAGS"],
    AC_MSG_ERROR(Cannot find gio: Is gio-config in path?))
       
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
