DEFS=@DEFS@ -I. -I$(srcdir) -I.. -DLOCALEDIR=\"${localedir}\" -DDATADIR=\"${datadir}\" -DLIBDIR=\"${libdir}\" -DFRIENDSSERVERVERSION=\"${FRIENDSSERVERVERSION}\" ${NOGARMIN} ${NOPLUGINS} ${USETELEATLAS}

bin_SCRIPTS=	wpcvt wpget gpspoint2gpsdrive.pl geo-nearest geocache2way \
		geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay\
		gpssmswatch gpsfetchmap.pl \
		poi.pl 	

poi:
	PERLLIB=./POI PATH=./POI:$$PATH ./poi.pl --help

EXTRA_DIST = wpcvt wpget gpsfetchmap.pl gpspoint2gpsdrive.pl geo-nearest geocache2way \
	geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay gpssmswatch \
	poi.pl

gpsdrivedir = $(datadir)/gpsdrive


poidir   = /usr/share/perl5/Geo/Gpsdrive/POI
poi_DATA =  POI/*.pm
