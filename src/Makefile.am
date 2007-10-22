if HAVE_GDAL
SUBDIRS =      lib_map util
endif
DIST_SUBDIRS = lib_map util

if WITH_MAPNIK
MAPNIK_LIBS=-lmapnik
else
MAPNIK_LIBS=
endif

# TODO: It's a really really bad Idea to hardcode these include Files
DEFS=@DEFS@ -I. -I$(srcdir) -I.. \
	-DLOCALEDIR=\"${localedir}\" -DDATADIR=\"${datadir}\" \
	-DLIBDIR=\"${libdir}\" \
	-DFRIENDSSERVERVERSION=\"${FRIENDSSERVERVERSION}\" \
	${NOGARMIN} ${NOPLUGINS} ${AMAPNIK}\
	-I/usr/include/ \
	-I/usr/local/include \
	-I/opt/boost_1_35/include/boost-1_35 \
	-I/usr/local/include/freetype2 \
	-I/usr/include/freetype2 \
	-I. \
	-L/usr/local/lib

#	-I/usr/include/dbus-1.0/ 


includedir = lib_map

if DISABLEGARMIN
PRG1=
else 
#PRG1=garble
PRG1=
endif

bin_PROGRAMS = $(PRG1) gpsdrive friendsd2 
LIBS=$(DBUS_LIBS) $(DBUS_GLIB_LIBS) -lfreetype $(MAPNIK_LIBS) -lboost_filesystem


if HAVE_DBUS
INCLUDES = $(DBUS_CFLAGS) -DDBUS_API_SUBJECT_TO_CHANGE=1
endif

gpsdrive_LDADD=@LIBS@ $(LIBADD_DL) 

PRGS = 	gpsdrive.c splash.c splash.h gpsdrive_config.c gpsdrive_config.h \
	navigation.c \
	speech_out.c speech_out.h\
	friends.c \
	battery.c track.c poi.c wlan.c waypoint.c draw_grid.c settings.c \
	battery.h track.h poi.h wlan.h waypoint.h gpsdrive.h \
	gpssql.c gpskismet.c gpskismet.h icons.c icons.h \
	gui.c gui.h poi_gui.c poi_gui.h \
	main_gui.c main_gui.h \
	navigation_gui.c settings_gui.c \
	LatLong-UTMconversion.c LatLong-UTMconversion.h \
	gpsnasamap.c gpsmisc.c geometry.c \
	map_handler.c map_handler.h gpsproto.h \
	import_map.c import_map.h\
	routes.c routes.h \
        download_map.c download_map.h \
	map_projection.c \
	speech_strings.c speech_strings.h \
	gps_handler.c gps_handler.h nmea_handler.c nmea_handler.h \
	unit_test.c \
	mapnik.cpp mapnik.h \
	screenshot.c screenshot.h gpx.c gpx.h \
	../config.h gettext.h
#	lib_map/lib_map.a

if DISABLEGARMIN
else
#PRGS += garmin_data.cpp \
#	garmin_serial_unix.cpp garmin_application.cpp garmin_link.cpp \
#	garmin_util.cpp gpsdrivegarble.cpp garmin_legacy.cpp garmin_link.h\
#	garmin_serial_unix.h garmin_application.h garmin_packet.h  garmin_types.h \
#	garmin_command.h garmin_phys.h garmin_util.h garmin_error.h garmin_serial.h\
#	garmin_legacy.h garmin_data.h
endif
gpsdrive_SOURCES= $(PRGS)


if DISABLEGARMIN
else
#garble_SOURCES=	garble.cpp garmin_legacy.cpp garmin_data.cpp \
#		garmin_serial_unix.cpp garmin_application.cpp garmin_link.cpp garmin_util.cpp
endif

friendsd2_SOURCES=friendsd.c
friendsd2_LDADD = @LIBS@ $(LDADD) $(LIBINTL)

EXTRA_DIST = 	gpsdrive.spec gpsdrive-nosql.spec gpsdrive.spec.fc5 CMakeLists.txt
