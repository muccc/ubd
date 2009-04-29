AC_INIT([serialtest],[1.0],[schneider@blinkenlichts.net])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
PKG_CHECK_MODULES(GNET, gnet-2.0 > 2.0.0,
  [LIBS="$LIBS $GNET_LIBS" CFLAGS="$CFLAGS $GNET_CFLAGS"],
    AC_MSG_ERROR(Cannot find GNet: Is gnet-config in path?))
        
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT
