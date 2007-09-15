## Makefile.am for data

SUBDIRS =      map-icons 
DIST_SUBDIRS = $(SUBDIRS)

cfg_filesdir   = $(datadir)/gpsdrive/
cfg_files_DATA = map_koord.txt gpsdrive.desktop

pixmapdir   = $(datadir)/gpsdrive/pixmaps
pixmap_DATA = \
	pixmaps/add_waypoint.png \
	pixmaps/battery.png \
	pixmaps/compass.png \
	pixmaps/friendsicon.png \
	pixmaps/gauge.png \
	pixmaps/gpsdriveanim.gif \
	pixmaps/gpsdrivelogo.png \
	pixmaps/gpsdrivemini.png \
	pixmaps/gpsdrivesplash.png \
	pixmaps/gpsicon.png \
	pixmaps/gpsiconbt.png \
	pixmaps/kismet.png \
	pixmaps/posmarker.png \
	pixmaps/powercharges.png \
	pixmaps/powercord.png \
	pixmaps/star.png \
	pixmaps/targetmarker.png


mapsdir   = $(datadir)/gpsdrive/maps
maps_DATA = \
	maps/map_BlueBall.png \
	maps/map_LightYellow.png \
	maps/map_WhiteBall.png \
	maps/map_White.png \
	maps/top_GreenBall.png \
	maps/top_LightYellow.png \
	maps/top_WhiteBall.png \
	maps/top_White.png \
	maps/top_Africa.jpg \
	maps/top_Americas.jpg \
	maps/top_AsiaEast.jpg \
	maps/top_AsiaMiddleEast.jpg \
	maps/top_Australia.jpg \
	maps/top_BritishIsles.jpg \
	maps/top_Europe.jpg \
	maps/top_Germany.jpg \
	maps/top_GPSWORLD.jpg \
	maps/top_JapanKorea.jpg \
	maps/top_NewZealand.jpg \
	maps/top_NorthAmerica.jpg \
	maps/top_SouthAmerica.jpg \
	maps/top_WorldEast.jpg


appsdir = $(datadir)/applications
apps_DATA = gpsdrive.desktop

EXTRA_DIST= \
	$(cfg_files_DATA) \
	$(pixmap_DATA) $(icons_DATA) \
	\
	$(maps_DATA) $(poi_DATA)  \
	$(apps_DATA) \
	CMakeLists.txt \
	maps/CMakeLists.txt  \
	pixmaps/CMakeLists.txt \
	mysql/my.cnf \
	poi_examples.gpx
