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
	install_osm_mapnik.1 \
	poi-manager.1

EXTRA_DIST = CMakeLists.txt

%.1: $(srcdir)/../scripts/%.pl
	pod2man $? >$@ 
