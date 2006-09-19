# Makefile.am for scripts
bin_SCRIPTS=	wpcvt wpget gpspoint2gpsdrive.pl geo-nearest geocache2way \
		geo-code gpssql_backup.sh gpssql_restore.sh gpsreplay\
		gpssmswatch gpsfetchmap.pl \
		geoinfo.pl wp2sql gpsd_nmea.sh \
		nasaconv.sh


geoinfo:
	PERLLIB=./ PATH=./:$$PATH ./geoinfo.pl --help


gpsdrivedir = $(datadir)/gpsdrive


#Geo_OSMdir   = $(PERL_PACKAGE_DIR)/Geo/OSM
#Geo_OSM_DATA =  Geo/OSM/*.pm

Geo_Gpsdrivedir   = $(PERL_PACKAGE_DIR)/Geo/Gpsdrive
Geo_Gpsdrive_DATA =  Geo/Gpsdrive/*.pm

#Utilsdir   = $(PERL_PACKAGE_DIR)/Utils
#Utils_DATA =  Utils/*.pm

#Utils_LWPdir   = $(PERL_PACKAGE_DIR)/Utils/LWP
#Utils_LWP_DATA =  Utils/LWP/*.pm

EXTRA_DIST = $(bin_SCRIPTS)\
	$(Geo_Gpsdrive_DATA)
#	$(Utils_DATA)\
#	$(Utils_LWP_DATA)\
#	$(Geo_OSM_DATA)

