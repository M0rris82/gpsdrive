dnl configure.ac for Gpsdrive
dnl Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>
dnl $Id$

AC_INIT(gpsdrive, 2.10pre3-cvs-20060112, ganter@ganter.at)
AC_CONFIG_SRCDIR(foo.c)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_SUBST(FRIENDSSERVERVERSION,'2')

AC_CONFIG_SRCDIR(src/gpsdrive.c)
AC_CONFIG_HEADER(config.h)

ALL_LINGUAS="de es fr it da nl tr de_AT hu sk sv no pt_BR gr ja"



AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([debian/Makefile])
AC_CONFIG_FILES([intl/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_CONFIG_FILES([man/de/Makefile])
AC_CONFIG_FILES([man/es/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([scripts/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([Documentation/Makefile])



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
AC_PROG_YACC

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


UNAMEP=`uname -p`
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

dnl ************************
dnl Check for standard headers
dnl ************************

AC_HEADER_STDC
AC_CHECK_SOCKLEN_T

dnl ************************
dnl Checks for libraries
dnl ************************

PKG_CHECK_MODULES(PKGCONFIG, gtk+-2.0 >= 2.0.6  gthread-2.0  libart-2.0)
LIBS="$LIBS $PKGCONFIG_LIBS"
CFLAGS="$CFLAGS $PKGCONFIG_CFLAGS"

AC_PATH_PROG(PCRE_CONFIG, pcre-config, no)
if test "x$PCRE_CONFIG" = "xno" ; then
	AC_MSG_ERROR(pcre-config not found please install libpcre3-dev or similar)
else 
	LIBS="$LIBS `$PCRE_CONFIG --libs`"
	CFLAGS="$CFLAGS `$PCRE_CONFIG --cflags`"
fi


localedir='${prefix}/share/locale'
AC_SUBST(localedir)

dnl ************************
dnl check dynamic loading flags
dnl ************************
AC_LTDL_DLLIB
AC_LTDL_DLSYM_USCORE
# putting this here gets the option enabled, but program segfaults
# putting a non-conditional CFLAGS="$CFLAGS -Ddlsym..." early in
# configure.ac works correctly, but putting the two AC_ and the
# conditional CFLAGS= does not.
#if test x"$libltdl_cv_need_uscore" = xyes; then
#    AC_SUBST(DLSYM_CFLAGS,'-Ddlsym=dlsym_prepend_underscore') 
#fi
 
dnl Checks select argument types
AC_FUNC_SELECT_ARGTYPES
AC_HEADER_TIME

dnl ************************
dnl Program locations
dnl ************************

AC_PREFIX_DEFAULT("/usr/local")

if test "${prefix}" == "NONE"; then
	prefix=${ac_default_prefix}
	AC_SUBST(prefix)
fi

pkgdatadir=${datadir}/${PACKAGE}/
AC_SUBST(pkgdatadir)

dnl *******************************
dnl   GETTEXT
dnl *******************************

AM_GNU_GETTEXT_VERSION(0.10.40)
AM_GNU_GETTEXT
                                                                                
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


AC_ARG_ENABLE(teleatlas,
AC_HELP_STRING([--enable-teleatlas],[enable street navigation using Teleatlas maps, needs gpsnavlib. Ignore this option until announced at www.gpsdrive.cc ]),[
	case "${enableval}" in
		no) echo "disable street navigation support" ;;
		yes)  echo "enable street navigation using Teleatlas maps, needs gpsnavlib" 
			AC_SUBST(USETELEATLAS,'-DUSETELEATLAS') 
			enableteleatlas=true ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-teleatlas) ;;
	esac
])
AM_CONDITIONAL(ENABLETELEATLAS, test x$enableteleatlas = xtrue)

dnl Manually configure DBUS until we figure out a 
dnl distro-independent was to check for both libraries and headers
AC_ARG_ENABLE(dbus,
    AC_HELP_STRING([--enable-dbus],
                [enable DBUS support. No checks are performed, you are responsible for having it installed =)]),
    [ac_dbus=$enableval], [ac_dbus=no])
AC_MSG_CHECKING([for DBUS support])
if test x"$ac_dbus" == "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE([DBUS_ENABLE], 1, [DBUS support])
    # Older versions of atotools barf and die on this.
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

dnl Setup install path root for perl.
dnl Yep, its ebil, but I don't know better...
#PERL_PACKAGE_DIR=`perl -V:installsitearch | sed "s/installsitearch='//" | sed "s/';//"`
# default on debian should be: /usr/share/perl5/
PERL_VENDORLIB=`perl -V:vendorlib | sed "s,vendorlib=',," | sed "s/';//"`
#PERL_PACKAGE_DIR=`echo ${PERL_VENDORLIB} | sed "s,/usr,-${prefix}-,"`
PERL_PACKAGE_DIR=`echo ${PERL_VENDORLIB} | sed "s,/usr/share,${datadir},"`
AC_SUBST(PERL_PACKAGE_DIR)

dnl ************************
dnl crypt.h and libcrypt
dnl ************************
AC_CHECK_HEADERS(crypt.h)
AC_CHECK_LIB(crypt,crypt)

dnl ************************
dnl GNU getopt
dnl ************************
AC_CHECK_DECLS(getopt)

dnl ************************
dnl check dynamic loading flags
dnl ************************
AC_LTDL_DLLIB
AC_LTDL_DLSYM_USCORE
# putting this here gets the option enabled, but program segfaults
# putting a non-conditional CFLAGS="$CFLAGS -Ddlsym..." early in
# configure.ac works correctly, but putting the two AC_ and the
# conditional CFLAGS= does not.
#if test x"$libltdl_cv_need_uscore" = xyes; then
#    AC_SUBST(DLSYM_CFLAGS,'-Ddlsym=dlsym_prepend_underscore') 
#fi

CFLAGS="$CFLAGS -I\$(srcdir)/mysql"

CFLAGS="$CFLAGS $OPT_CFLAGS"
CXXFLAGS="$CXXFLAGS $OPT_CFLAGS"

AC_OUTPUT
