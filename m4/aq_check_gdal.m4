dnl ------------------------------------------------------------------------
dnl Detect GDAL/OGR
dnl
dnl use AQ_CHECK_GDAL to detect GDAL and OGR
dnl it sets:
dnl   GDAL_CFLAGS
dnl   GDAL_LDADD
dnl ------------------------------------------------------------------------

# Check for GDAL and OGR compiler and linker flags

AC_DEFUN([AQ_CHECK_GDAL],
[
AC_ARG_WITH([gdal],
  AC_HELP_STRING([--with-gdal=path],
    [Full path to 'gdal-config' script, e.g. '--with-gdal=/usr/local/bin/gdal-config']),
  [ac_gdal_config_path=$withval])

if test x"$ac_gdal_config_path" = x ; then
  ac_gdal_config_path=`which gdal-config`
fi

ac_gdal_config_path=`dirname $ac_gdal_config_path 2> /dev/null`
AC_PATH_PROG(GDAL_CONFIG, gdal-config, no, $ac_gdal_config_path)

if test x${GDAL_CONFIG} = xno ; then
  AC_MSG_ERROR([gdal-config not found! Supply it with --with-gdal=PATH])
else
  AC_MSG_CHECKING([for OGR in GDAL])
  if test x`$GDAL_CONFIG --ogr-enabled` = "xno" ; then
    AC_MSG_ERROR([GDAL must be compiled with OGR support and currently is not.])
  fi
  AC_MSG_RESULT(yes)
  AC_MSG_CHECKING([GDAL_CFLAGS])
  GDAL_CFLAGS=`$GDAL_CONFIG --cflags`
  AC_MSG_RESULT($GDAL_CFLAGS)

  AC_MSG_CHECKING([GDAL_LDADD])
  GDAL_LDADD=`$GDAL_CONFIG --libs`
  AC_MSG_RESULT($GDAL_LDADD)

  ac_gdalogr_version=`$GDAL_CONFIG --version`
  ac_gdalogr="yes"
fi

AC_SUBST(GDAL_CFLAGS)
AC_SUBST(GDAL_LDADD)
])
