# Makefile.am for data

cfg_filesdir   = $(datadir)/gpsdrive/
cfg_files_DATA = icons.txt map_koord.txt

pixmapdir   = $(datadir)/gpsdrive/pixmaps
pixmap_DATA = pixmaps/*.png pixmaps/*.gif

iconsdir   = $(datadir)/gpsdrive/icons
icons_DATA = icons/*.png icons/*.gif

mapsdir   = $(datadir)/gpsdrive/maps
maps_DATA = maps/top_Africa.jpg   maps/top_Australia.jpg  maps/top_Germany.jpg   maps/top_NorthAmerica.jpg  maps/top_WorldEast.jpg \
	    maps/top_America.jpg  maps/top_Europe.jpg     maps/top_GPSWORLD.jpg  maps/top_SouthAmerica.jpg


appsdir = $(datadir)/applications
apps_DATA = gpsdrive.desktop

EXTRA_DIST=	\
	$(cfg_files_DATA) $(pixmap_DATA) $(icons_DATA) $(maps_DATA) $(apps_DATA)
