# Makefile.am in main dir
#AUTOMAKE_OPTIONS=dist-bzip2

SUBDIRS =      src man scripts data Documentation debian DSL gentoo po tests
DIST_SUBDIRS = $(SUBDIRS)


# I'm not shure if this is needed
EXTRA_DIST = config.rpath m4/ChangeLog po/Makefile.am po/Makefile.in po/POTFILES \
	cmake/Modules/*.cmake cmake/Modules/CMakeLists.txt \
	cmake/Modules/COPYING-CMAKE-SCRIPTS cmake/CMakeLists.txt \
	config.h.cmake ConfigureChecks.cmake \
	CMakeLists.txt INSTALL.cmake DEVPACKAGES\
	m4/*.m4 \
	intl/*
pkgdata_DATA = depcomp

documentation1dir   = $(datadir)/gpsdrive/Documentation
documentation1_DATA = AUTHORS COPYING INSTALL README NEWS

ACLOCAL_AMFLAGS = -I m4
