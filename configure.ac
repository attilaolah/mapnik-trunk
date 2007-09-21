dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(mapnik, 0.4.0)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LIBTOOL

dnl Check for option to enable debug
AC_MSG_CHECKING(whether to enable debugging)
AC_ARG_ENABLE(debug,
        [  --enable-debug=[no/yes]        enables debug build (default=no)],,
                 enable_debug=no)

dnl Checks for libraries
if [ ! test "x$enable_debug" != "xyes"]; then
  AC_DEFINE(DEBUG, 1, [Define to enable debug build])
  CXXFLAGS="${CXXFLAGS} -g -DDEBUG -DMAPNIK_DEBUG"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi


dnl Check for boost
AX_BOOST_BASE
if test "$succeeded" != "yes" ; then
  exit
fi

AX_BOOST_THREAD
if test "x$link_thread" = "xno"; then
  exit
fi

AX_BOOST_FILESYSTEM
if test "x$link_filesystem" = "xno"; then
  exit
fi

AX_BOOST_REGEX
if test "x$link_regex" = "xno"; then
  exit
fi

dnl only for postgis plugin
AX_LIB_POSTGRESQL
AM_CONDITIONAL(BUILD_POSTGRESQL, test "x$found_postgresql" = "xyes")

AX_CHECK_TIFF
if test "x$link_tiff" = "xno"; then
  exit
fi


AX_CHECK_JPEG
if test "x$link_jpeg" = "xno"; then
  exit
fi

AX_CHECK_PROJ
if test "x$link_proj" = "xno"; then
  exit
fi

AX_CHECK_LTDL
if test "x$link_ltdl" = "xno"; then
  exit
fi

PKG_CHECK_MODULES(PNG, libpng)
AC_SUBST(PNG_CFLAGS)
AC_SUBST(PNG_LIBS)

PKG_CHECK_MODULES(FREETYPE2, freetype2)
AC_SUBST(FREETYPE2_CFLAGS)
AC_SUBST(FREETYPE2_LIBS)

PKG_CHECK_MODULES(AGG, libagg)
AC_SUBST(AGG_CFLAGS)
AC_SUBST(AGG_LIBS)

AC_OUTPUT([
Makefile
include/Makefile
include/mapnik/Makefile
plugins/Makefile
plugins/input/Makefile
plugins/input/gdal/Makefile
plugins/input/postgis/Makefile
plugins/input/raster/Makefile
plugins/input/shape/Makefile
src/Makefile
mapnik.pc
mapnik-uninstalled.pc
])