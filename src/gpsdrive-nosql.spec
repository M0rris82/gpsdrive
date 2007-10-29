#
#
#
Summary: gpsdrive is a GPS based navigation tool 
Name: gpsdrive
Version: 1.31
Release: 1
Copyright: GPL
Group: Tools 
Source: %{name}-%{version}.tar.gz
Vendor: Fritz Ganter <ganter@ganter.at>
Packager: Fritz Ganter <ganter@ganter.at>
BuildRoot: %{_builddir}/%{name}-root
%define _prefix /usr
%description
Gpsdrive is a map-based navigation system. It displays your position on a zoomable map provided from a NMEA-capable GPS receiver. The maps are autoselected for the best resolution, depending of your position, and the displayed image can be zoomed. Maps can be downloaded from the Internet with one mouse click. The program provides information about speed, direction, bearing, arrival time, actual position, and target position. Speech output is also available.
See http://gpsdrive.de for new releases.
%prep
%setup
CFLAGS="$RPM_OPT_FLAGS" ./configure --disable-mysql --prefix=%{_prefix} --mandir=%{_mandir}


%build
make
%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install-strip

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf %{_builddir}/%{name}-%{version}
%files
%defattr (-,root,root)
%doc GPS-receivers INSTALL AUTHORS COPYING  TODO README LEEME LISEZMOI README.FreeBSD  README.gpspoint2gspdrive FAQ.gpsdrive FAQ.gpsdrive.fr  README.SQL create.sql NMEA.txt wp2sql README.kismet
%doc %{_mandir}/de/man1/gpsdrive.1.gz
%doc %{_mandir}/es/man1/gpsdrive.1.gz
%doc %{_mandir}/man1/gpsdrive.1.gz

%{_libdir}/*
%{_bindir}/*

%dir %{_prefix}/share/gpsdrive
%{_prefix}/share/gpsdrive/gpsdrivesplash.png
%{_prefix}/share/gpsdrive/friendsicon.png
%{_prefix}/share/locale/*/LC_MESSAGES/*

