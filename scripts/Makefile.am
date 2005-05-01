DEFS=@DEFS@ -I. -I$(srcdir) -I.. -DLOCALEDIR=\"${localedir}\" -DDATADIR=\"${datadir}\" -DLIBDIR=\"${libdir}\" -DFRIENDSSERVERVERSION=\"${FRIENDSSERVERVERSION}\" ${NOGARMIN} ${NOPLUGINS} ${USETELEATLAS}

#scriptsdir = $(datadir)/gpsdrive/scripts
bin_SCRIPTS=	wpcvt wpget gpspoint2gpsdrive.pl geo-nearest geocache2way \
		geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay\
		gpssmswatch gpsfetchmap.pl \
		poi.pl 	POI/census.pm POI/Kismet.pm POI/DBFuncs.pm POI/mapsource.pm \
		       	POI/Defaults_Cities.pm POI/NGA.pm POI/gpsdrive.pm \
			POI/OpenGeoDB.pm POI/GpsDrive.pm POI/PocketGpsPoi.pm \
			POI/Gps.pm POI/Utils.pm POI/JiGLE.pm POI/WDB.pm

poi:
	PERLLIB=./POI PATH=./POI:$$PATH ./poi.pl --help

EXTRA_DIST = wpcvt wpget gpsfetchmap.pl gpspoint2gpsdrive.pl geo-nearest geocache2way geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay gpssmswatch poi.pl POI/census.pm POI/Kismet.pm POI/DBFuncs.pm POI/mapsource.pm POI/Defaults_Cities.pm POI/NGA.pm POI/gpsdrive.pm POI/OpenGeoDB.pm POI/GpsDrive.pm POI/PocketGpsPoi.pm POI/Gps.pm POI/Utils.pm POI/JiGLE.pm POI/WDB.pm

gpsdrivedir = $(datadir)/gpsdrive


