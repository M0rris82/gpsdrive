# Makefile.am for data

cfg_filesdir   = $(datadir)/gpsdrive/
cfg_files_DATA = icons.txt map_koord.txt

pixmapdir   = $(datadir)/gpsdrive/pixmaps
pixmap_DATA = pixmaps/*.png pixmaps/*.gif

iconsdir   = $(datadir)/gpsdrive/icons
icons_DATA = icons/classic/*.png \
	     icons/square.big/*.png \
	     icons/square.big/*/*.png \
	     icons/square.big/*/*/*.png \
	     icons/square.small/*.png  \
	     icons/square.small/*/*.png \
	     icons/index.html

streetsdir   = $(datadir)/gpsdrive/streets
streets_DATA = streets/Autobahnen.txt \
	  streets/Streets_au.txt  streets/Streets.txt

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

EXTRA_DIST=        \
        $(cfg_files_DATA) \
        $(pixmap_DATA) $(icons_DATA) \
        $(maps_DATA) $(poi_DATA) $(streets_DATA) \
        $(apps_DATA)
