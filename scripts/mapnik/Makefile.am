# Makefile.am for scripts
SUBDIRS =      symbols
DIST_SUBDIRS = $(SUBDIRS)

bin_SCRIPTS = gpsdrive_mapnik_gentiles.py

mapnikdir = $(datadir)/mapnik
mapnik_DATA = osm.xml

EXTRA_DIST = setup_z_order.sql gpsdrive_mapnik_gentiles-in.py osm-in.xml 
#CMakeLists.txt

CLEANFILES = osm.xml gpsdrive_mapnik_gentiles.py

dummy:

gpsdrive_mapnik_gentiles.py: gpsdrive_mapnik_gentiles-in.py
	sed 's,@DATA_DIR@,${datadir},' < gpsdrive_mapnik_gentiles-in.py > gpsdrive_mapnik_gentiles.py

osm.xml: osm-in.xml
	sed 's,@DATA_DIR@,${datadir},' < osm-in.xml > osm.xml
