# Makefile.am for scripts
bin_SCRIPTS=	wpcvt wpget gpspoint2gpsdrive.pl geo-nearest geocache2way \
		geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay\
		gpssmswatch gpsfetchmap.pl \
		geoinfo.pl create.sql wp2sql gpsd_nmea.sh \
		nasaconv.sh


geoinfo:
	PERLLIB=./ PATH=./:$$PATH ./geoinfo.pl --help


gpsdrivedir = $(datadir)/gpsdrive


poidir   = $(PERL_PACKAGE_DIR)/Geo/Gpsdrive
#poidir   = /usr/share/perl5/Geo/Gpsdrive
poi_DATA =  Geo/Gpsdrive/*.pm

EXTRA_DIST = $(bin_SCRIPTS) $(poi_DATA)

