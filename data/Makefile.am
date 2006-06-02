# Makefile.am for data

cfg_filesdir   = $(datadir)/gpsdrive/
cfg_files_DATA = icons.txt map_koord.txt

pixmapdir   = $(datadir)/gpsdrive/pixmaps
pixmap_DATA = pixmaps/*.png pixmaps/*.gif

iconsdir   = $(datadir)/gpsdrive/icons
icons_DATA = icons/classic/*.png \
	     icons/square.big/*.png \
	     icons/square.big/*/*.png \
	     icons/index.html
#	     icons/square.small/*/*.png 
#	     icons/square.small/*.png 

streetsdir   = $(datadir)/gpsdrive/streets
streets_DATA = streets/Autobahnen.txt \
	  streets/Streets_au.txt  streets/Streets.txt

poidir   = $(datadir)/gpsdrive/poi
poi_DATA = \
	poi/cities_germany.txt  poi/cities.txt  poi/germany.txt\
	poi/cities_au.txt poi/shops_au.txt


mapsdir   = $(datadir)/gpsdrive/maps
maps_DATA = \
           maps/map_BlueBall.png \
           maps/map_LightYellow.png \
           maps/map_White.png \
           maps/map_WhiteBall.png \
           maps/top_Africa.jpg \
           maps/top_America.jpg  \
           maps/top_Australia.jpg \
           maps/top_Europe.jpg \
           maps/top_GPSWORLD.jpg \
           maps/top_Germany.jpg \
           maps/top_GreenBall.png \
           maps/top_LightYellow.png \
           maps/top_NorthAmerica.jpg \
           maps/top_SouthAmerica.jpg \
           maps/top_White.png \
           maps/top_WhiteBall.png \
           maps/top_WorldEast.jpg


appsdir = $(datadir)/applications
apps_DATA = gpsdrive.desktop

EXTRA_DIST=        \
        $(cfg_files_DATA) \
        $(pixmap_DATA) $(icons_DATA) \
        $(maps_DATA) $(poi_DATA) $(streets_DATA) \
        $(apps_DATA)
