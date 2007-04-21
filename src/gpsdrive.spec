#
#
#
Summary: gpsdrive is a GPS based navigation tool 
Name: gpsdrive
Version: 2.10pre2
Release: 1
Copyright: GPL
Group: Tools 
Source: %{name}-%{version}.tar.gz
Vendor: Fritz Ganter <ganter@ganter.at>
Packager: Fritz Ganter <ganter@ganter.at>
BuildRoot: %{_tmppath}/%{name}-root
%define _prefix /usr
%description
Gpsdrive is a map-based navigation system. It displays your position on a zoomable map provided from a NMEA-capable GPS receiver. The maps are autoselected for the best resolution, depending of your position, and the displayed image can be zoomed. Maps can be downloaded from the Internet with one mouse click. The program provides information about speed, direction, bearing, arrival time, actual position, and target position. Speech output is also available. MySQL is supported.
See http://www.gpsdrive.de for new releases.
%prep
%setup


%build
export CFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS="$RPM_OPT_FLAGS"
./configure --prefix=%{_prefix} --mandir=%{_mandir}
make
%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install-strip

%clean
if [ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ]; then
rm -rf $RPM_BUILD_ROOT
fi
rm -rf %{_builddir}/%{name}-%{version}

%files
%defattr (-,root,root)
%doc GPS-receivers INSTALL AUTHORS COPYING  TODO README LEEME LISEZMOI README.FreeBSD README.gpspoint2gspdrive FAQ.gpsdrive FAQ.gpsdrive.fr  README.SQL create.sql  NMEA.txt wp2sql README.kismet  LISEZMOI.kismet LISEZMOI.SQL
%doc %{_mandir}/de/man1/gpsdrive.1.gz
%doc %{_mandir}/es/man1/gpsdrive.1.gz
%doc %{_mandir}/man1/gpsdrive.1.gz

%{_libdir}/*
%{_bindir}/*

%dir %{_prefix}/share/gpsdrive
%{_prefix}/share/gpsdrive/gpsdrivesplash.png
%{_prefix}/share/gpsdrive/gpsdrivemini.png
%{_prefix}/share/gpsdrive/friendsicon.png
%{_prefix}/share/gpsdrive/gpsicon.png
%{_prefix}/share/gpsdrive/gpsiconbt.png
%{_prefix}/share/gpsdrive/gpsdriveanim.gif
%{_prefix}/share/gpsdrive/top_GPSWORLD.jpg
%{_prefix}/share/locale/*/LC_MESSAGES/*
%{_prefix}/share/gpsdrive/AUTHORS
%{_prefix}/share/gpsdrive/CREDITS
%{_prefix}/share/gpsdrive/FAQ.gpsdrive
%{_prefix}/share/gpsdrive/FAQ.gpsdrive.fr
%{_prefix}/share/gpsdrive/GPS-receivers
%{_prefix}/share/gpsdrive/LEEME
%{_prefix}/share/gpsdrive/LISEZMOI
%{_prefix}/share/gpsdrive/NMEA.txt
%{_prefix}/share/gpsdrive/README
%{_prefix}/share/gpsdrive/README.FreeBSD
%{_prefix}/share/gpsdrive/README.SQL
%{_prefix}/share/gpsdrive/README.gpspoint2gspdrive
%{_prefix}/share/gpsdrive/README.kismet
%{_prefix}/share/gpsdrive/TODO
%{_prefix}/share/gpsdrive/README.nasamaps
%{_prefix}/share/gpsdrive/create.sql
%{_prefix}/share/gpsdrive/wp2sql
%{_datadir}/applications/gpsdrive.desktop
%{_prefix}/share/pixmaps/gpsicon.png
