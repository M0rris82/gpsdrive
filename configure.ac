dnl configure.ac for Gpsdrive
dnl Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>
dnl $Id$

dnl #########################################################
dnl Update Log
dnl ===========
dnl Date    	Name	Description
dnl ---------------------------------------------------------
dnl 25/03/2006  DP	Updated detection for GTK
dnl 26/03/2006  DP      Added detection for libglib2.0 (2.8.6-1)
dnl 27/03/2006  DP      Added detection for ATK
dnl 29/03/2006  DP      Added detction for Cairo
dnl 30/03/2006  DP      Added detection for fonfconfig
dnl 04/04/2006  DP      Added detection for libpqxx
dnl 04/04/2006  DP      Added detection for libpng12
dnl 24/04/2006  DP      Send TODO output to Std Error
dnl 08/05/2005  DP	Avoid uname -p error on Debian
dnl 06/06/2006  DP      Lower libpango required min version from 1.10.4 to 1.10.2
dnl 06/06/2006  DP      Lower libglib required min version from 2.8.6 to 2.8.5
dnl 06/06/2006  DP      Remove Requirement for libglib 2.0
dnl 08/06/2006  DP      Fixed typo in error message libcario libcairo.
dnl 10/07/2006  DP      Changed the echo text from libcairo2 to libcairo.
dnl 16/07/2006  DP	Reduced required versions numbers as follows
dnl			libfontconfig 2.3.1 -> 2.2.3
dnl                     libpango 1.10.2 -> 1.10.1
dnl                     libxcursor 1.1.3 -> 1.1.2
dnl #########################################################

AC_INIT(gpsdrive, 2.10svn16234568, package-for-gpsdrive@ostertag.name)
AC_CONFIG_SRCDIR(src/gpsdrive.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_SUBST(FRIENDSSERVERVERSION,'2')
AC_CONFIG_HEADER(config.h)
AC_USE_SYSTEM_EXTENSIONS()

wLL_LINGUAS="da de de_AT es fr hu it ja nl ru sk sv tr"

AC_CONFIG_FILES([DSL/Makefile])
AC_CONFIG_FILES([Documentation/Makefile])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([data/map-icons/Makefile])
AC_CONFIG_FILES([debian/Makefile])
AC_CONFIG_FILES([gentoo/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_CONFIG_FILES([man/de/Makefile])
AC_CONFIG_FILES([man/es/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([scripts/Makefile])
AC_CONFIG_FILES([scripts/mapnik/Makefile])
AC_CONFIG_FILES([scripts/mapnik/symbols/Makefile])
AC_CONFIG_FILES([scripts/osm/Makefile])
AC_CONFIG_FILES([scripts/osm/perl_lib/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/lib_map/Makefile])
AC_CONFIG_FILES([src/util/Makefile])
AC_CONFIG_FILES([tests/Makefile])

AC_CHECK_FUNCS([bzero])
AC_CHECK_FUNCS([floor])
AC_CHECK_FUNCS([gethostbyaddr]) 
AC_CHECK_FUNCS([gethostbyname])
AC_CHECK_FUNCS([gethostname])
AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([inet_ntoa])
AC_CHECK_FUNCS([localeconv])
AC_CHECK_FUNCS([localtime_r])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([nl_langinfo])
AC_CHECK_FUNCS([pow])
AC_CHECK_FUNCS([rint])
AC_CHECK_FUNCS([select])
AC_CHECK_FUNCS([socket])
AC_CHECK_FUNCS([sqrt])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strcspn])
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([strpbrk])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_FUNCS([strstr])
AC_CHECK_FUNCS([strtol])
AC_CHECK_FUNCS([strtoull])
AC_CHECK_FUNCS([tzset])

AC_CHECK_HEADERS([X11/X.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([float.h])
AC_CHECK_HEADERS([langinfo.h])
AC_CHECK_HEADERS([libintl.h])
AC_CHECK_HEADERS([linux/inet.h])
AC_CHECK_HEADERS([netdb.h])
AC_CHECK_HEADERS([netinet/in.h])
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdio_ext.h])
AC_CHECK_HEADERS([sys/ioctl.h])
AC_CHECK_HEADERS([sys/stat.h])
AC_CHECK_HEADERS([sys/termios.h])
AC_CHECK_HEADERS([sys/time.h])
AC_CHECK_HEADERS([sys/timeb.h])
AC_CHECK_HEADERS([sys/types.h])
AC_CHECK_HEADERS([termio.h])
AC_CHECK_HEADERS([termios.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([wchar.h])

AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_CHECK_TYPES([ptrdiff_t])

dnl AM_PATH_GTK_2_0(2.0.6,,AC_ERROR(need at least Gtk+ version 2.0.6))

dnl Following line remvoed 06/06/2006
dnl AM_PATH_GLIB_2_0(2.0.6,, AC_ERROR(need libglib2.0 at least version 2.0.6),gmodule-2.0)

AC_C_VOLATILE

AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRNLEN
AC_FUNC_STRTOD
AC_FUNC_VPRINTF

AC_HEADER_DIRENT
AC_HEADER_STDBOOL

AC_PROG_GCC_TRADITIONAL
AC_PROG_RANLIB
AC_PROG_YACC

AC_CHECK_PROG(PERL,perl,perl)

AC_STRUCT_TM

AC_TYPE_SIGNAL

dnl ************************
dnl Compiler options
dnl ************************
AC_PROG_LIBTOOL
AC_PROG_INSTALL

AC_PROG_CC
AC_ISC_POSIX
AC_C_INLINE
AC_PROG_CXX
AC_HEADER_STDC
AC_PROG_AWK
AC_PROG_LN_S

echo "Using $CC compiler"

if test "$CC" = "gcc" ; then

if $CC -dumpversion|egrep -q -e '^3\.*' -e '^4\.*'; then
echo "GCC ok"
else
echo "*****************************************"
echo "You need a gcc >= 3.x to compile GpsDrive"
echo "*****************************************"
exit
fi

fi
#  ----------------------------------------------------
#  DP 08/05/2006 Debian does not support uname -p 
#  ----------------------------------------------------
OSNAME=`uname -n`
if test "$OSNAME" = "debian"; then
	UNAMEP=unknown
else
	UNAMEP=`uname -p`
fi
#  -----------------------------------------------------

ARCHP=`arch`
CFLAGS="$CFLAGS"

AC_ARG_WITH(debug,
[  --with-debug            compiles with -g],[
        OPT_CFLAGS=""
	CFLAGS=""
	echo "Ignoring optimizing flags; using debugging flags: $OPT_CFLAGS"
])

AC_ARG_ENABLE(auto-optimization, 
		AC_HELP_STRING([--enable-auto-optimization],
				[try to automatically determine the most optimal build options for your architecture]),

		UNAMEP=`uname -p`
		if test "x$UNAMEP" = "xunknown"; then
			UNAMEP=`grep "model name" /proc/cpuinfo | cut -d: -f2`
		fi

echo $UNAMEP
		if $CC -dumpversion|egrep -q "^3\.3.*"; then
# gcc-3.3
			if echo $UNAMEP|grep -q "Intel(R) Pentium(R) 4 CPU"; then
				MARCH=-march=pentium4
			elif echo $UNAMEP|grep -q "Pentium III"; then
				MARCH=-march=pentium3
			elif echo $UNAMEP|grep -q "AMD-K6(tm) 3D"; then
				MARCH=-march=k6-2
			elif echo $UNAMEP|grep -q "Pentium 75 - 200"; then
				MARCH=-march=pentium
			elif echo $UNAMEP|grep -q "Pentium II"; then
				MARCH=-march=pentium2
			elif echo $UNAMEP|grep -q "AMD Athlon(..) XP"; then
				MARCH=-march=athlon-xp
			else
				MARCH=""
			fi
			OPT_CFLAGS="-O3 $MARCH -fomit-frame-pointer -fno-exceptions  -pipe -s -ffast-math -fexpensive-optimizations -falign-functions -falign-loops -funroll-loops -mfpmath=sse"
		elif $CC -dumpversion|egrep -q "^3\.0.*"; then
# gcc-3.0
			OPT_CFLAGS="-O3 -fomit-frame-pointer -fno-exceptions  -pipe -s -ffast-math -fexpensive-optimizations -falign-functions -falign-loops "
		elif $CC -dumpversion | egrep -q "^2\.95.*" ; then
# gcc-2.95
			if echo $UNAMEP|egrep -q "(Pentium III|Pentium II|Pentium\(R\) 4|Athlon)"; then
				MARCH=-march=pentiumpro
			elif echo $UNAMEP|grep -q "AMD-K6"; then
				MARCH=-march=k6
			elif echo $UNAMEP|grep -q "Pentium 75 - 200"; then
				MARCH=-march=pentium
			else 
				MARCH=""
			fi
			OPT_CFLAGS="$MARCH -O3 -fomit-frame-pointer -fno-exceptions  -pipe -s -ffast-math -fexpensive-optimizations"
		else
			echo "warning: compiler is not known: not optimizing!"
		fi
		echo "compiler optimizing flags: $OPT_CFLAGS"
)

AC_ARG_ENABLE(gcc3-optimization, 
		AC_HELP_STRING([--enable-gcc3-optimization=type],
				[gcc3 can optimize for: i386, i486, pentium, pentium-mmx, pentiumpro, pentium2, pentium3, pentium4, k6, k6-2, k6-3, athlon, athlon-tbird, athlon-4, athlon-xp, athlon-mp, winchip-c6, winchip2, c3]),
		if $CC -dumpversion|egrep -q "^3\..*"; then
			OPT_CFLAGS="-O3 -march=$enableval -fomit-frame-pointer -fno-exceptions  -pipe -s -ffast-math -fexpensive-optimizations -falign-functions -falign-loops"
			echo "compiler optimizing flags: $OPT_CFLAGS"
		else
			echo "warning: compiler is not gcc3: not optimizing!"
		fi
)

AC_ARG_ENABLE(gcc2-optimization, 
		AC_HELP_STRING([--enable-gcc2-optimization=type],
				[gcc2 can optimize for: i386, i486, pentium, pentiumpro, k6]),
		if $CC -dumpversion | egrep -q "^2\..*" ; then
			OPT_CFLAGS="-O3 -march=$enableval -fomit-frame-pointer -fno-exceptions  -pipe -s -ffast-math -fexpensive-optimizations"
			echo "compiler optimizing flags: $OPT_CFLAGS"
		else 
			echo "warning: compiler is not gcc2: not optimizing!"
		fi
)

AC_PROG_INSTALL

CFLAGS="$CFLAGS -g -Wall -Wno-format-y2k -pipe $OPT_CFLAGS"

#  ------------------------------------------------------------------
#  Mapnik
#  ------------------------------------------------------------------
AC_MSG_CHECKING([for Mapnik lib support])
#  This is a hack to make AC_TRY_LINK and the compiler think we have a cpp Programm
old_ac_ext=$ac_ext
OLD_CFLAGS=$CFLAGS
ac_ext=cpp
CFLAGS="$CFLAGS -lmapnik"
#  Above is the Hack ;-)
AC_TRY_LINK( [#include <mapnik/map.hpp>], 
	     [using mapnik::Map;mapnik::Map *test_map;], 
	     [ libmapnik_linkable=yes
	       AC_MSG_RESULT([yes] )],
	     [ libmapnik_linkable=no 
	       AC_MSG_RESULT([no])
	     ])
echo "libmapnik_linkable: $libmapnik_linkable"
#  reverse our hack for the rest of configure
ac_ext=$old_ac_ext
CFLAGS=$OLD_CFLAGS
#  ------------------------------------------------------------------

AC_MSG_CHECKING([for Mapnik inclusion])
AC_ARG_ENABLE(mapnik,
	 AC_HELP_STRING([--disable-mapnik],
		        [disable use of mapnik modules]),
	[
	case "${enableval}" in
		yes) AC_MSG_RESULT([yes])
		     with_mapnik=yes
		     AC_SUBST(AMAPNIK,'-DMAPNIK')
		     echo "ENABLED Mapnik support"
		     ;;
		no)  AC_MSG_RESULT([no])
		     with_mapnik=no
		     echo "DISABLED Mapnik support" 
		     ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --disable-mapnik)
		   ;;
	esac
	],
	[ AC_MSG_RESULT([$libmapnik_linkable])
	  with_mapnik=$libmapnik_linkable
	  test x$with_mapnik = xyes && AC_SUBST(AMAPNIK,'-DMAPNIK')
	]
)
AM_CONDITIONAL([WITH_MAPNIK], test x$with_mapnik = xyes)


#  ---------------------------------------------------------------------------
#  GDAL/OGR
#  ---------------------------------------------------------------------------
AC_ARG_ENABLE(gdal,
              AC_HELP_STRING([--enable-gdal],
	                     [enable GDAL support. This might be needed in the future for compilation]),
              [ac_gdal=$enableval], [ac_gdal=no])
AC_MSG_CHECKING([for GDAL support])
if test x"$ac_gdal" == "xyes"; then
    echo "enable GDAL support"
    AQ_CHECK_GDAL
    AM_CONDITIONAL(HAVE_GDAL, test -n $ac_gdal_config_path)
else
    AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([HAVE_GDAL], [test x"$ac_gdal" = x"yes"])


#  ************************
#  Check for standard headers
#  ************************

AC_HEADER_STDC
AC_CHECK_SOCKLEN_T

#  ************************
#  Checks for libraries
#  ************************

#  25/03/2006 DP Added the following line
AM_PATH_GTK_2_0(2.0.6,,AC_ERROR(need at least Gtk+ version 2.0.6))

PKG_CHECK_MODULES(GTK, gtk+-2.0, HAVE_GTK=yes, HAVE_GTK=no)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
if test "$HAVE_GTK" = "no"; then
  AC_MSG_WARN([cannot find GTK+-2.0, that is not good])
fi
AM_CONDITIONAL(HAVE_GTK,[test "$HAVE_GTK" != "no"])
CFLAGS="$CFLAGS -DHAVE_GTK   `pkg-config gtk+-2.0 --cflags`"

PKG_CHECK_MODULES(PKGCONFIG, gtk+-2.0 >= 2.0.6  gthread-2.0  libart-2.0)
LIBS="$LIBS $PKGCONFIG_LIBS"
CFLAGS="$CFLAGS $PKGCONFIG_CFLAGS"
# We do have GK since we just tested on it
#AM_CONDITIONAL(HAVE_GTK, true)

#  --------------------------
#  check for package libxml-2.0
#  --------------------------
PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.6.26)
CFLAGS="$CFLAGS $XML_CFLAGS"
LIBS="$LIBS $XML_LIBS"

#  --------------------------
#  Check for package libglib2.0-0
#  26/03/2006 DP  Added
#  06/06/2006 DP  Reduced from version 2.8.6 yo 2.8.5
#  --------------------------
#  AM_PATH_GLIB_2_0([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]]])
#  MODULES  gmodule, gobject or gthread
AM_PATH_GLIB_2_0(2.8.5, , AC_ERROR(need libglib2.0 at least version 2.8.5))

#  --------------------------
echo -n checking for package libatk1.0-0...
#  27/03/2006 DP Added
#  --------------------------
if $PKG_CONFIG atk --atleast-version=1.10.3
then
   AC_MSG_RESULT([yes])
   dnl AC_CHECK_LIB(atk, main, [], [     dnl TODO need to find a sutibable function name to test for
   dnl echo "Error with test of ATK library not found"
   dnl exit -1
   dnl ])
else
   AC_MSG_ERROR([libatk is missing or your version is to old. Min Version 1.10.3])
fi

#  ---------------------------
echo checking for package libc6 2.3.5-1... TODO >&2
#  ---------------------------

#  ---------------------------
echo checking for package libcairo 1.0.2-3...
#  ---------------------------
if $PKG_CONFIG cairo --atleast-version=1.0.2
then
   AC_MSG_RESULT([yes])
   AC_CHECK_LIB(cairo, cairo_create, [], [
   echo "Error with test of Cairo library not found" 
   exit -1
   ])
else
   AC_MSG_ERROR([libcairo is missing or your version is to old. Min Version 1.0.2])
fi
#  ---------------------------
echo checking for package libexpat1... TODO >&2                   
#  ---------------------------

#  ---------------------------
echo -n checking for package libfontconfig1...
#  ---------------------------
#  DP changed from 2.3.1 to 2.2.3
if $PKG_CONFIG fontconfig --atleast-version=2.2.3
then
   AC_MSG_RESULT([yes])
   AC_CHECK_LIB(fontconfig, FcConfigFilename, [], [
   echo "Error with test of fontconfig library not found"
   exit -1
   ])
else
   AC_MSG_ERROR([libfontconfig is missing or your version is to old. Min Version 2.2.3])
fi		  

#  ---------------------------
echo checking for package libfreetype6... TODO >&2
#  ---------------------------
echo TODO checking for package libgcc1... TODO >&2
#  ---------------------------

#  ---------------------------
echo -n checking for package libpango1 1.10...
#  ---------------------------
if $PKG_CONFIG pango --atleast-version=1.10.1
then
    AC_MSG_RESULT([yes])
    dnl AC_CHECK_LIB(pango,  main, [], [    dnl TODO Need to find a suitable function name to check for
    dnl echo "Error with test of libpango1. library not found"
    dnl exit -1
    dnl ])
else
    AC_MSG_ERROR([libpango1 is missing or your version is to old. Min Version 1.10.1])
fi

#  ---------------------------
echo -n checking for package libpng12-0...
#  ---------------------------
if $PKG_CONFIG libpng12 --atleast-version=1.2.8
then
    AC_MSG_RESULT([yes])
    dnl AC_CHECK_LIB(libpng12, main, [], [   dnl  TODO Need to find a suitable function name to check for
    dnl echo "Error with test of libpng, library not found"
    dnl exit -1
    dnl ])
else
    AC_MSG_ERROR([libpng12 is missing or your version is to old. Min Version 1.2.8])
fi

#  ---------------------------
echo checking for package libstdc++6... TODO >&2
#  ---------------------------
echo checking for package libx11-6...   TODO >&2
#  ---------------------------
echo -n checking for package libxcursor1...
#  ---------------------------
if $PKG_CONFIG xcursor --atleast-version=1.1.2
then
    AC_MSG_RESULT([yes])
    AC_CHECK_LIB(fontconfig, XcursorImageCreate, [], [
    echo "Error with test of xcursor library not found"
    exit -1
    ])
else
    AC_MSG_ERROR([libxcursor1 is missing or your version is to old. Min Version 1.1.2])
fi
#  ----------------------------			


#  ---------------------------
echo checking for package libxext6... TODO >&2
#  ---------------------------
echo checking for package libxi6...   TODO >&2
#  ---------------------------
echo checking for package libxinerama1 1:0.9.0-1... TODO >&2
#  ---------------------------
echo checking for package zlib1g...  TODO >&2
#  ---------------------------
echo checking for package libgdal-grass 1.2.6-1+b1...  TODO >&2
#  ---------------------------
echo checking for package libsrces26-dev 2.6.0-6...   TODO >&2
#  ---------------------------
echo checking for package libmysqlclient14-dev 4.1.15-1... TODO >&2
AC_CHECK_LIB([mysql], [mysql_server_init])

#  ---------------------------
echo checking for package libtiff4-dev 3.7.4-1...  TODO >&2
#  ---------------------------
echo checking for package libjasper-1.701-dev...  TODO >&2
#  ---------------------------
echo checking for package libgeos-dev 2.1.4-2...  TODO >&2
#  ---------------------------
echo checking for package libgdal1-dev 1.2.6-1.3...  TODO >&2
#  ---------------------------
echo checking for package gdal-bin 1.2.6-1.3...  TODO >&2
#  ---------------------------
echo checking for package libgrass 6.0.1-3...  TODO >&2
#  ---------------------------
echo checking for package libgdal1-1.3.2-grass 1.3.2-1...  TODO >&2
#  ---------------------------
echo checking for package libhdf4g-dev 4.1r4-18.1... TODO >&2
#  ---------------------------
echo checking for package libungif4-dev 4.1.4-2...  TODO >&2
#  ---------------------------
echo checking for package netcdfg-dev 3.5.0-7.1...  TODO >&2
#  ---------------------------
echo checking for package libcfitsio-dev 2.510-1... TODO >&2
#  ---------------------------

#  ---------------------------
#  echo -n checking for package libpqxx 2.5.5-2...
#  --------------------------
#  if $PKG_CONFIG libpqxx --atleast-version=2.5.5
dnl then
dnl    AC_MSG_RESULT([yes])
dnl     dnl TODO AC_CHECK_LIB(libpqxx, main, [], [ dnl TODO Need to find a suitable function name to test for
dnl     dnl echo "Error with test of libpqxx library not found"
dnl     dnl exit -1
dnl     dnl ])
dnl else
dnl    AC_MSG_ERROR([libpqxx is missing or your version is to old. Min Version 2.5.5])
dnl fi
dnl --------------------------------------------------------------------------------------

CFLAGS="$CFLAGS -DHAVE_CAIRO `pkg-config cairo --cflags`"

AC_PATH_PROG(PCRE_CONFIG, pcre-config, no)
if test "x$PCRE_CONFIG" = "xno" ; then
	AC_MSG_ERROR(pcre-config not found please install libpcre3-dev or similar)
else 
	LIBS="$LIBS `$PCRE_CONFIG --libs`"
	CFLAGS="$CFLAGS `$PCRE_CONFIG --cflags`"
fi


localedir='${prefix}/share/locale'
AC_SUBST(localedir)

#  ************************
#  check dynamic loading flags
#  ************************
AC_LTDL_DLLIB
AC_LTDL_DLSYM_USCORE
# putting this here gets the option enabled, but program segfaults
# putting a non-conditional CFLAGS="$CFLAGS -Ddlsym..." early in
# configure.ac works correctly, but putting the two AC_ and the
# conditional CFLAGS= does not.
#if test x"$libltdl_cv_need_uscore" = xyes; then
#    AC_SUBST(DLSYM_CFLAGS,'-Ddlsym=dlsym_prepend_underscore') 
#fi
 
#  Checks select argument types
AC_FUNC_SELECT_ARGTYPES
AC_HEADER_TIME

#  ************************
#  Program locations
#  ************************

AC_PREFIX_DEFAULT("/usr/local")

if test "${prefix}" == "NONE"; then
	prefix=${ac_default_prefix}
	AC_SUBST(prefix)
fi

pkgdatadir=${datadir}/${PACKAGE}/
AC_SUBST(pkgdatadir)

#  *******************************
#    GETTEXT
#  *******************************

AM_GNU_GETTEXT([external]) 
AM_GNU_GETTEXT_VERSION([0.16.1])
#AM_GNU_GETTEXT
                                                                                
GETTEXT_PACKAGE=gpsdrive
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE([GETTEXT_PACKAGE],"gpsdrive", this is the gettext domain name)
if test "$GMSGFMT" = "no"; then
        echo "no gmsgfmt - NLS disabled"
        USE_NLS=no
fi
if test "$MSGFMT" = "no" ; then
        echo "no msfmt - NLS disabled"
        USE_NLS=no
fi
if test "$USE_NLS" = "yes" ; then
        AC_DEFINE(ENABLE_NLS)
fi

AC_ARG_ENABLE(garmin,
[  --disable-garmin        compiles without GARMIN protocol support],[
	case "${enableval}" in
		yes) echo "enable GARMIN protocol support" ;;
		no)  echo "disable GARMIN protocol support" 
			AC_SUBST(NOGARMIN,'-DNOGARMIN') 
			disablegarmin=true 
			CXX=$CC ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-garmin) ;;
	esac
])
AM_CONDITIONAL(DISABLEGARMIN, test x$disablegarmin = xtrue)

AC_ARG_ENABLE(plugins,
[  --disable-plugins       disable use of plugin modules],[
	case "${enableval}" in
		yes) echo "enable plugin support" ;;
		no)  echo "disable plugin support" 
			AC_SUBST(NOPLUGINS,'-DNOPLUGINS') 
			disableplugins=true ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-plugins) ;;
	esac
])
AM_CONDITIONAL(DISABLEPLUGINS, test x$disableplugins = xtrue)

#  ********************************************
#  Manually configure DBUS until we figure out a 
#  distro-independent was to check for both libraries and headers
#  ********************************************
AC_ARG_ENABLE(dbus,
    AC_HELP_STRING([--enable-dbus],
                [enable DBUS support. No checks are performed, you are responsible for having it installed ]),
    [ac_dbus=$enableval], [ac_dbus=no])
AC_MSG_CHECKING([for DBUS support])
if test x"$ac_dbus" == "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([DBUS_ENABLE], 1, [DBUS support])
    # Older versions of autotools barf and die on this.
    PKG_CHECK_MODULES(DBUS, dbus-1 >= 0.23.4 )
    DBUS_CFLAGS=`pkg-config --cflags dbus-glib-1`
    DBUS_LIBS=`pkg-config --libs dbus-1`
    AC_SUBST(DBUS_CFLAGS)
    AC_SUBST(DBUS_LIBS)
    DBUS_GLIB_LIBS=`pkg-config --libs dbus-glib-1`
    AC_SUBST(DBUS_GLIB_LIBS)
else
    AC_MSG_RESULT([no])
fi

AM_CONDITIONAL([HAVE_DBUS], [test x"$ac_dbus" = x"yes"])


#  ********************************************
#  Perl Path
#  Setup install path root for perl.
#  Yep, its ebil, but I does not know better...
#PERL_PACKAGE_DIR=`perl -V:installsitearch | sed "s/installsitearch='//" | sed "s/';//"`
# default on debian should be: /usr/share/perl5/
PERL_VENDORLIB=`perl -V:vendorlib | sed "s,vendorlib=',," | sed "s/';//"`
#PERL_PACKAGE_DIR=`echo ${PERL_VENDORLIB} | sed "s,/usr,-${prefix}-,"`
PERL_PACKAGE_DIR=`echo ${PERL_VENDORLIB} | sed "s,/usr/share,${datadir},"`
AC_SUBST(PERL_PACKAGE_DIR)

AC_PROG_PERL_MODULES(File::Basename , ,  AC_MSG_ERROR(Need Perl module File::Basename))
AC_PROG_PERL_MODULES(File::Copy , ,  AC_MSG_ERROR(Need Perl module File::Copy))
AC_PROG_PERL_MODULES(File::Path , ,  AC_MSG_ERROR(Need Perl module File::Path))
AC_PROG_PERL_MODULES(File::Temp , ,  AC_MSG_ERROR(Need Perl module File::Temp))
AC_PROG_PERL_MODULES(Getopt::Long , ,  AC_MSG_ERROR(Need Perl module Getopt::Long))
AC_PROG_PERL_MODULES(HTTP::Request , ,  AC_MSG_ERROR(Need Perl module HTTP::Request))
AC_PROG_PERL_MODULES(HTTP::Request::Common , ,  AC_MSG_ERROR(Need Perl module HTTP::Request::Common))
AC_PROG_PERL_MODULES(IO::File , ,  AC_MSG_ERROR(Need Perl module IO::File))
AC_PROG_PERL_MODULES(LWP::Simple , ,  AC_MSG_ERROR(Need Perl module LWP::Simple))
AC_PROG_PERL_MODULES(LWP::UserAgent , ,  AC_MSG_ERROR(Need Perl module LWP::UserAgent))
AC_PROG_PERL_MODULES(Math::Trig , ,  AC_MSG_ERROR(Need Perl module Math::Trig))

# These will only be needed for Building GpsDrive Packages
#AC_PROG_PERL_MODULES(File::Slurp , ,  AC_MSG_ERROR(Need Perl module File::Slurp))
#AC_PROG_PERL_MODULES(WWW::Curl::Easy , ,  AC_MSG_ERROR(Need Perl module WWW::Curl::Easy))
#AC_PROG_PERL_MODULES(WWW::Mechanize , ,  AC_MSG_ERROR(Need Perl module WWW::Mechanize))
#AC_PROG_PERL_MODULES(XML::Parser , ,  AC_MSG_ERROR(Need Perl module XML::Parser))
#AC_PROG_PERL_MODULES(XML::Simple , ,  AC_MSG_ERROR(Need Perl module XML::Simple))
#AC_PROG_PERL_MODULES(XML::Twig , ,  AC_MSG_ERROR(Need Perl module XML::Twig))

#  only for OSM Stuff
#  AC_PROG_PERL_MODULES(Bit::Vector , ,  AC_MSG_ERROR(Need Perl module Bit::Vector))
#  AC_PROG_PERL_MODULES(Math::Polygon , ,  AC_MSG_ERROR(Need Perl module Math::Polygon))
#  AC_PROG_PERL_MODULES(XML::Generator , ,  AC_MSG_ERROR(Need Perl module XML::Generator))
#  AC_PROG_PERL_MODULES(XML::SAX::Machines , ,  AC_MSG_ERROR(Need Perl module XML::SAX::Machines))
#  AC_PROG_PERL_MODULES(XML::SAX::Writer , ,  AC_MSG_ERROR(Need Perl module XML::SAX::Writer))


#  ************************
#  crypt.h and libcrypt
#  ************************
AC_CHECK_HEADERS(crypt.h)
AC_CHECK_LIB(crypt,crypt)

#  ************************
#  GNU getopt
#  ************************
AC_CHECK_DECLS(getopt)

#  ************************
#  check dynamic loading flags
#  ************************
AC_LTDL_DLLIB
AC_LTDL_DLSYM_USCORE
# putting this here gets the option enabled, but program segfaults
# putting a non-conditional CFLAGS="$CFLAGS -Ddlsym..." early in
# configure.ac works correctly, but putting the two AC_ and the
# conditional CFLAGS= does not.
#if test x"$libltdl_cv_need_uscore" = xyes; then
#    AC_SUBST(DLSYM_CFLAGS,'-Ddlsym=dlsym_prepend_underscore') 
#fi

if test -f /usr/include/mysql/mysql.h; then
 CFLAGS="$CFLAGS -I/usr/include/mysql"
else
 CFLAGS="$CFLAGS -Imysql"
fi

CFLAGS="$CFLAGS $OPT_CFLAGS"
CXXFLAGS="$CXXFLAGS $OPT_CFLAGS"

AC_OUTPUT
