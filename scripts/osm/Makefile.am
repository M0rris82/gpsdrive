# Makefile.am for scripts/perl_lib

gpsdrivedir = $(datadir)/gpsdrive


bin_SCRIPTS= \
	osmfilter/osmfilter.pl \
	planet-mirror/planet-mirror.pl \
	osm-subset/osm-subset.pl \
	osm2csv/osm2csv.pl

Osm_osmfilterdir   = $(PERL_PACKAGE_DIR)/osmfilter
Osm_osmfilterDATA =  osmfilter/*.pl

Osm_planet_mirrordir   = $(PERL_PACKAGE_DIR)/planet-mirror
Osm_planet_mirrorDATA =  planet-mirror/*.pl

Osm_osm_subsetdir   = $(PERL_PACKAGE_DIR)/osm-subset
Osm_osm_subsetDATA =  osm-subset/*.pl

Osm_osm2csvdir   = $(PERL_PACKAGE_DIR)/osm2csv
Osm_osm2csvDATA =  osm2csv/*.pl

#PRGS = planet.osm/C/UTF8sanitizer.c

EXTRA_DIST = \
	$(Osm_osmfilterDATA) \
	$(Osm_planet_mirrorDATA) \
	$(Osm_osm_subsetDATA) \
	$(Osm_osm2csvDATA) 