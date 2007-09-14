SUBDIRS = de es 

man1_MANS = \
	$(srcdir)/garble.1 \
	$(srcdir)/geo-code.1 \
	$(srcdir)/geo-nearest.1
	$(srcdir)/geoinfo.1 \
	$(srcdir)/gpsdrive.1 \
	convert-waypoints.1 \
	create-osm.xml.1 \
	gpsdrive-init-db.1 \
	gpsfetchmap.1 \
	gpspoint2gpsdrive.1 \
	poi-manager.1
dist_man_MANS = $(man1_MANS)

EXTRA_DIST = CMakeLists.txt

convert-waypoints.1: $(srcdir)/../scripts/convert-waypoints.pl
	pod2man $? >$@ 

create-osm.xml.1: $(srcdir)/../scripts/create-osm.xml.pl
	pod2man $? >$@ 

gpsdrive-init-db.1: $(srcdir)/../scripts/gpsdrive-init-db.pl
	pod2man $? >$@ 

gpsfetchmap.1: $(srcdir)/../scripts/gpsfetchmap.pl
	pod2man $? >$@ 

gpspoint2gpsdrive.1: $(srcdir)/../scripts/gpspoint2gpsdrive.pl
	pod2man $? >$@ 

poi-manager.1: $(srcdir)/../scripts/poi-manager.pl
	pod2man $? >$@ 
