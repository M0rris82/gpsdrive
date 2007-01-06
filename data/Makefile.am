## Makefile.am for data

SUBDIRS =      map-icons icons
DIST_SUBDIRS = $(SUBDIRS)

cfg_filesdir   = $(datadir)/gpsdrive/
cfg_files_DATA = icons.xml map_koord.txt gpsdrive.desktop

pixmapdir   = $(datadir)/gpsdrive/pixmaps
pixmap_DATA = pixmaps/*.png pixmaps/*.gif


####################################################################################
poidir   = $(datadir)/gpsdrive/poi
poi_DATA = \
	poi/cities_germany.txt  poi/cities.txt  poi/germany.txt\
	poi/cities_au.txt poi/shops_au.txt


mapsdir   = $(datadir)/gpsdrive/maps
maps_DATA = \
	maps/*.png \
	maps/*.jpg


appsdir = $(datadir)/applications
apps_DATA = gpsdrive.desktop

EXTRA_DIST= \
	$(cfg_files_DATA) \
	$(pixmap_DATA) $(icons_DATA) \
	\
	$(maps_DATA) $(poi_DATA)  \
	$(apps_DATA) \
	CMakeLists.txt poi/CMakeLists.txt \
	maps/CMakeLists.txt icons/CMakeLists.txt \
	pixmaps/CMakeLists.txt \
	icons/dev-icons.txt \
	mysql/my.cnf \
	poi_examples.gpx \
	README.icons \
	rebuild-all.sh \
	rebuild-icons.txt.sh \
	rebuild-poi_types.html.pl \
	update_icons.pl
