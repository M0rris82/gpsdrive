# Makefile.am for scripts
bin_SCRIPTS=	wpcvt wpget gpspoint2gpsdrive.pl geo-nearest geocache2way \
		geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay\
		gpssmswatch gpsfetchmap.pl \
		geoinfo.pl create.sql


geoinfo:
	PERLLIB=./ PATH=./:$$PATH ./geoinfo.pl --help

EXTRA_DIST = wpcvt wpget gpsfetchmap.pl gpspoint2gpsdrive.pl geo-nearest \
	geocache2way wp2sql \
	geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay gpssmswatch \
	geoinfo.pl create.sql


gpsdrivedir = $(datadir)/gpsdrive


poidir   = /usr/share/perl5/Geo/Gpsdrive
poi_DATA =  Geo/Gpsdrive/*.pm
