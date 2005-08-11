DEFS=@DEFS@ -I. -I$(srcdir) -I.. -DLOCALEDIR=\"${localedir}\" -DDATADIR=\"${datadir}\" -DLIBDIR=\"${libdir}\" -DFRIENDSSERVERVERSION=\"${FRIENDSSERVERVERSION}\" ${NOGARMIN} ${NOPLUGINS} ${USETELEATLAS}

EXTRA_DIST=	\
	icons.txt \
	map_koord.txt

cfg_filesdir   = $(datadir)/gpsdrive/
cfg_files_DATA = $(EXTRA_DIST)

pixmapdir   = $(datadir)/gpsdrive/pixmaps
pixmap_DATA = pixmaps/*.png pixmaps/*.gif

iconsdir   = $(datadir)/gpsdrive/icons
icons_DATA = icons/*.png icons/*.gif

mapsdir   = $(datadir)/gpsdrive/maps
maps_DATA = maps/top_GPSWORLD.jpg
