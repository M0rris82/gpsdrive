# Makefile.am for scripts
SUBDIRS =      osm osm/perl_lib mapnik
DIST_SUBDIRS = $(SUBDIRS)

bin_SCRIPTS=	wpcvt wpget gpspoint2gpsdrive.pl geo-nearest geocache2way \
		geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay\
		gpssmswatch gpsfetchmap.pl convert-waypoints.pl \
		geoinfo.pl gpsdrive-init-db.pl wp2sql gpsd_nmea.sh \
		nasaconv.sh \
		start_mysql_as_user.sh poi-manager.pl \
		devpackages-debian.sh  convert-waypoints.pl \
		create_misssing_man_pages.sh

geoinfo:
	PERLLIB=./ PATH=./:$$PATH ./geoinfo.pl --help

gpsdrivedir = $(datadir)/gpsdrive

EXTRA_DIST = $(bin_SCRIPTS) CMakeLists.txt
