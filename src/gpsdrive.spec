#
#
#
Summary: gpsdrive is a GPS based navigation tool 
Name: gpsdrive
Version: 2.11
Release: 1
#Copyright: GPL
License: GPL
Group: Tools 
Source: %{name}-%{version}.tar.gz
Vendor: Fritz Ganter <ganter@ganter.at>
Packager: Joerg Ostertag <gpsdrive@ostertag.name>
BuildRoot: %{_tmppath}/%{name}-root
BuildRequires:  cmake, xorg-x11-devel, gtk2-devel, glib-devel, libgmodule-2_0-0, pango-devel, gpsd-devel
BuildRequires:  gdk-pixbuf-devel, cairo-devel, atk-devel, libxml2-devel, libcurl-devel
BuildRequires:  boost-devel
# Avoid Error: src/mapnik.cpp:212: error: 'class boost::filesystem::basic_directory_entry<boost::filesystem::basic_path<std::basic_string<char, std::char_traits<char>, std::allocator<char> >, boost::filesystem::path_traits> >' has no member named 'leaf'
#BuildRequires: libboost_serialization1_36_0
#BuildRequires: libboost_filesystem1_36_0
#BuildRequires: libboost_iostreams1_36_0
#BuildRequires: libboost_signals1_36_0
#BuildRequires: libboost_system1_36_0

BuildRequires:  gcc-c++
BuildRequires:  mapnik-devel, postgresql-devel
BuildRequires:  sqlite3-devel, speechd-devel
BuildRequires:  intltool

# otherwise i get strange errors in the includefile xxx in database.c
BuildRequires:  gnu-inetlib

# to avoid error: /usr/include/mapnik/plugin.hpp:32:18: error: ltdl.h: No such file or directory
BuildRequires:  libtool

%define _prefix /usr
%description
Gpsdrive is a map-based navigation system. It displays your position on a zoomable map provided from a NMEA-capable GPS receiver. The maps are autoselected for the best resolution, depending of your position, and the displayed image can be zoomed. Maps can be downloaded from the Internet with one mouse click. The program provides information about speed, direction, bearing, arrival time, actual position, and target position. Speech output is also available. MySQL is supported.
See http://www.gpsdrive.de for new releases.
%prep
%setup


%build
export CMAKE_CXX_COMPILER=/usr/bin/cmake
export CFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS="$RPM_OPT_FLAGS"

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DPERL_SITELIB=/usr/share/perl5 \
      -DMAN_INSTALL_DIR=/usr/share/man \
      -DINFO_INSTALL_DIR=/usr/share/info \
      -DDATADIR=/usr/share \
      .. 

make

%install
rm -rf $RPM_BUILD_ROOT
cd build
make DESTDIR=$RPM_BUILD_ROOT install

%clean
if [ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ]; then
rm -rf $RPM_BUILD_ROOT
fi
rm -rf %{_builddir}/%{name}-%{version}

%files
%defattr (-,root,root)
#%doc GPS-receivers INSTALL AUTHORS COPYING  TODO README LEEME LISEZMOI README.FreeBSD README.gpspoint2gspdrive FAQ.gpsdrive FAQ.gpsdrive.fr  README.SQL create.sql  NMEA.txt wp2sql README.kismet  LISEZMOI.kismet LISEZMOI.SQL
%doc %{_mandir}/de/man1/gpsdrive.1.gz
%doc %{_mandir}/es/man1/gpsdrive.1.gz
%doc %{_mandir}/man1/gpsdrive.1.gz

#%{_libdir}/*
%{_bindir}/*

%dir %{_prefix}/share/gpsdrive
#%{_prefix}/share/gpsdrive/gpsdrivesplash.png
#%{_prefix}/share/gpsdrive/gpsdrivemini.png
#%{_prefix}/share/gpsdrive/friendsicon.png
#%{_prefix}/share/gpsdrive/gpsicon.png
#%{_prefix}/share/gpsdrive/gpsiconbt.png
#%{_prefix}/share/gpsdrive/gpsdriveanim.gif
#%{_prefix}/share/gpsdrive/top_GPSWORLD.jpg
#%{_prefix}/share/locale/*/LC_MESSAGES/*
#%{_prefix}/share/gpsdrive/AUTHORS
#%{_prefix}/share/gpsdrive/CREDITS
#%{_prefix}/share/gpsdrive/FAQ.gpsdrive
#%{_prefix}/share/gpsdrive/FAQ.gpsdrive.fr
#%{_prefix}/share/gpsdrive/GPS-receivers
#%{_prefix}/share/gpsdrive/LEEME
#%{_prefix}/share/gpsdrive/LISEZMOI
#%{_prefix}/share/gpsdrive/NMEA.txt
#%{_prefix}/share/gpsdrive/README
#%{_prefix}/share/gpsdrive/README.FreeBSD
#%{_prefix}/share/gpsdrive/README.SQL
#%{_prefix}/share/gpsdrive/README.gpspoint2gspdrive
#%{_prefix}/share/gpsdrive/README.kismet
#%{_prefix}/share/gpsdrive/TODO
#%{_prefix}/share/gpsdrive/README.nasamaps
#%{_prefix}/share/gpsdrive/create.sql
#%{_prefix}/share/gpsdrive/wp2sql
#%{_datadir}/applications/gpsdrive.desktop
#%{_prefix}/share/pixmaps/gpsicon.png
