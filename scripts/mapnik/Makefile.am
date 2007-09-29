# Makefile.am for scripts

bin_SCRIPTS = gpsdrive_mapnik_gentiles.py

mapnikdir = $(datadir)/mapnik
mapnik_DATA = osm.xml

EXTRA_DIST = setup_z_order.sql gpsdrive_mapnik_gentiles-in.py osm-in.xml 
#CMakeLists.txt

CLEANFILES = osm.xml gpsdrive_mapnik_gentiles.py

dummy:

gpsdrive_mapnik_gentiles.py: gpsdrive_mapnik_gentiles-in.py
	sed 's,@DATA_DIR@,${datadir},g' < $< > $@

osm.xml: osm-in.xml
	sed 's,@DATA_DIR@,${datadir},g' < $< > $@
