DEFS=@DEFS@ -I. -I$(srcdir) -I.. -DLOCALEDIR=\"${localedir}\" -DDATADIR=\"${datadir}\" -DLIBDIR=\"${libdir}\" -DFRIENDSSERVERVERSION=\"${FRIENDSSERVERVERSION}\" ${NOGARMIN} ${NOPLUGINS} ${USETELEATLAS}

# DEFS=@DEFS@ -I. -I$(srcdir) -I.. -DLOCALEDIR=\"${localedir}\" -DDATADIR=\"${datadir}\" -DLIBDIR=\"${libdir}\" -DFRIENDSSERVERVERSION=\"${FRIENDSSERVERVERSION}\" ${NOGARMIN} ${NOPLUGINS} -DG_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED 

includedir = mysql

if DISABLEGARMIN
PRG1=
else 
PRG1=garble
endif
bin_PROGRAMS = $(PRG1) gpsdrive gpsd  friendsd2 
LIBS=

if ENABLETELEATLAS
GPSNAVLIB=-lgpsnavlib
else
GPSNAVLIB=
endif

# gpsdrive_LDADD= $(GLIB_LIBS) $(GTK_LIBS) $(PIXLIBS)
#gpsdrive_LDADD=@LIBS@
gpsdrive_LDADD=@LIBS@ $(LIBADD_DL) $(GPSNAVLIB)
#gpsdrive_LDADD= @XLIBS@
# gpsdrive_CFLAGS=@CFLAGS@
# libgpsd_a_CFLAGS=@CFLAGS@
# CFLAGS=
noinst_LIBRARIES = libgpsd.a
if DISABLEPLUGINS
else
lib_LTLIBRARIES =libfly.la libnautic.la
libfly_la_SOURCES=fly.c
libfly_la_LDFLAGS = -version-info 2:0:0
libnautic_la_SOURCES=nautic.c
libnautic_la_LDFLAGS = -version-info 2:0:0
endif

libgpsd_a_SOURCES=netlib.c nmea_parse.c serial.c  tm.c em.c  \
	gps.h version.h 
gpsd_SOURCES= gpsd.c  gpsd.h nmea.h version.h
gpsd_LDADD= -lgpsd -lm
gpsd_LDFLAGS = -L. 


#bin_SCRIPTS=wpcvt wpget gpspoint2gpsdrive.pl geo-nearest geocache2way geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay gpssmswatch

if DISABLEGARMIN
PRGS = gpsdrive.c splash.c navigation.c speech_out.c friends.c battery.c track.c settings.c  power.h battery.h track.h gpsdrive.h gpssql.c gpskismet.c ../config.h icons.h gettext.h LatLong-UTMconversion.c LatLong-UTMconversion.h compass.h gpsserial.c gpsnasamap.c gpsmisc.c gpsproto.h talogo.h
else 
PRGS = gpsdrive.c splash.c navigation.c speech_out.c friends.c battery.c track.c settings.c  power.h battery.h track.h gpsdrive.h garmin_data.cpp garmin_serial_unix.cpp garmin_application.cpp garmin_link.cpp garmin_util.cpp gpsdrivegarble.cpp garmin_legacy.cpp garmin_link.h    garmin_serial_unix.h garmin_application.h garmin_packet.h  garmin_types.h garmin_command.h garmin_phys.h garmin_util.h garmin_error.h garmin_serial.h gpssql.c gpskismet.c ../config.h icons.h garmin_legacy.h garmin_data.h gettext.h LatLong-UTMconversion.c LatLong-UTMconversion.h compass.h gpsserial.c gpsnasamap.c gpsmisc.c gpsproto.h talogo.h
endif
gpsdrive_SOURCES= $(PRGS)


if DISABLEGARMIN
else
garble_SOURCES=garble.cpp garmin_legacy.cpp garmin_data.cpp garmin_serial_unix.cpp garmin_application.cpp garmin_link.cpp garmin_util.cpp
endif

friendsd2_SOURCES=friendsd.c
friendsd2_LDADD = @LIBS@ $(LDADD) $(LIBINTL)
EXTRA_DIST = mysql wpcvt wpget  gpsfetchmap.pl gpsdrive.spec gpsdrive-nosql.spec map_koord.txt gpsdrivesplash.png gpsicon.png  gpsiconbt.png gpsdrivemini.png friendsicon.png  gpspoint2gpsdrive.pl  geo-nearest geocache2way geo-code gpssql_backup.sh gpssql_restore.sh  top_GPSWORLD.jpg gpsreplay gpssmswatch gpsdriveanim.gif
gpsdrivedir = $(datadir)/gpsdrive
gpsdrive_DATA = gpsdrivesplash.png friendsicon.png top_GPSWORLD.jpg gpsdrivemini.png gpsicon.png  gpsiconbt.png gpsdriveanim.gif

distpixmapdir = $(datadir)/pixmaps
distpixmap_DATA = gpsicon.png
