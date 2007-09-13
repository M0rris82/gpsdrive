SUBDIRS = de es 

man: gpsdrive.1 garble.1 geo-code.1 geoinfo.1 geo-nearest.1 \
	convert-waypoints.1 create-osm.xml.1 geoinfo.1 gpsdrive-init-db.1 \
	gpsfetchmap.1 gpspoint2gpsdrive.1  install_osm_mapnik.1 poi-manager.1

EXTRA_DIST = gpsdrive.1 garble.1 geo-code.1 geoinfo.1 geo-nearest.1 CMakeLists.txt

install-data-local: convert-waypoints.1 create-osm.xml.1 geoinfo.1 gpsdrive-init-db.1 gpsfetchmap.1 gpspoint2gpsdrive.1  install_osm_mapnik.1 poi-manager.1
	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1/
	$(INSTALL_DATA) $(srcdir)/convert-waypoints.1 $(DESTDIR)$(mandir)/man1/convert-waypoints.1
	$(INSTALL_DATA) $(srcdir)/create-osm.xml.1 $(DESTDIR)$(mandir)/man1/create-osm.xml.1
	$(INSTALL_DATA) $(srcdir)/garble.1 $(DESTDIR)$(mandir)/man1/garble.1
	$(INSTALL_DATA) $(srcdir)/geo-code.1 $(DESTDIR)$(mandir)/man1/geo-code.1
	$(INSTALL_DATA) $(srcdir)/geo-nearest.1 $(DESTDIR)$(mandir)/man1/geo-nearest.1
	$(INSTALL_DATA) $(srcdir)/geoinfo.1 $(DESTDIR)$(mandir)/man1/geoinfo.1
	$(INSTALL_DATA) $(srcdir)/gpsdrive-init-db.1 $(DESTDIR)$(mandir)/man1/gpsdrive-init-db.1
	$(INSTALL_DATA) $(srcdir)/gpsdrive.1 $(DESTDIR)$(mandir)/man1/gpsdrive.1
	$(INSTALL_DATA) $(srcdir)/gpsfetchmap.1 $(DESTDIR)$(mandir)/man1/gpsfetchmap.1
	$(INSTALL_DATA) $(srcdir)/gpspoint2gpsdrive.1 $(DESTDIR)$(mandir)/man1/gpspoint2gpsdrive.1
	$(INSTALL_DATA) $(srcdir)/install_osm_mapnik.1 $(DESTDIR)$(mandir)/man1/install_osm_mapnik.1
	$(INSTALL_DATA) $(srcdir)/poi-manager.1 $(DESTDIR)$(mandir)/man1/poi-manager.1

convert-waypoints.1: ../scripts/convert-waypoints.pl
	pod2man $? >$@ 

create-osm.xml.1: ../scripts/create-osm.xml.pl
	pod2man $? >$@ 

geoinfo.1: ../scripts/geoinfo.pl
	pod2man $? >$@ 

gpsdrive-init-db.1: ../scripts/gpsdrive-init-db.pl
	pod2man $? >$@ 

gpsfetchmap.1: ../scripts/gpsfetchmap.pl
	pod2man $? >$@ 

gpspoint2gpsdrive.1: ../scripts/gpspoint2gpsdrive.pl
	pod2man $? >$@ 

install_osm_mapnik.1: ../scripts/install_osm_mapnik.pl
	pod2man $? >$@ 

poi-manager.1: ../scripts/poi-manager.pl
	pod2man $? >$@ 

