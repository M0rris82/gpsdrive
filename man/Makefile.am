SUBDIRS = de es 

man1_MANS = \
	$(srcdir)/geo-code.1 \
	$(srcdir)/geo-nearest.1 \
	$(srcdir)/gpsdrive.1 \
	$(srcdir)/friendsd2.1 \
	$(srcdir)/gpsd_nmea.sh.1 \
	convert-waypoints.pl.1 \
	geoinfo.pl.1 \
	gpsdrive-init-db.pl.1 \
	gpsfetchmap.pl.1 \
	gpspoint2gpsdrive.pl.1 \
	poi-manager.pl.1
dist_man_MANS = $(man1_MANS)

EXTRA_DIST = CMakeLists.txt

convert-waypoints.pl.1: $(srcdir)/../scripts/convert-waypoints.pl
	pod2man $? >$@ 

gpsdrive-init-db.pl.1: $(srcdir)/../scripts/gpsdrive-init-db.pl
	pod2man $? >$@ 

gpsfetchmap.pl.1: $(srcdir)/../scripts/gpsfetchmap.pl
	pod2man $? >$@ 

gpspoint2gpsdrive.pl.1: $(srcdir)/../scripts/gpspoint2gpsdrive.pl
	pod2man $? >$@ 

poi-manager.pl.1: $(srcdir)/../scripts/poi-manager.pl
	pod2man $? >$@ 

geoinfo.pl.1: $(srcdir)/../scripts/geoinfo.pl
	pod2man $? >$@ 
