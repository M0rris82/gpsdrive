# Makefile.am for scripts/perl_lib

gpsdrivedir = $(datadir)/gpsdrive

bin_SCRIPTS= \
	osmfilter/osmfilter.pl \
	planet-mirror/planet-mirror.pl \
	osm-subset/osm-subset.pl \
	simplify/simplify.pl \
	osm2csv/osm2csv.pl \
	planet-osm/C/UTF8sanitizer

UTF8sanitizerdir = $(bindir)
UTF8sanitizer_PROGRAMS = UTF8sanitizer
UTF8sanitizer_SOURCES = planet-osm/C/UTF8sanitizer.c
#PRGS = planet-osm/C/UTF8sanitizer.c

EXTRA_DIST = $(bin_SCRIPTS) \
	$(PRGS) \
	osm2csv/create_all.sh \
	pdf-atlas/create.sh \
	pdf-atlas/Data/readme-shorelines.txt \
	pdf-atlas/Data/readme.txt \
	pdf-atlas/pdf-atlas.pl \
	planet-osm/perl/planetosm-to-db.pl\
	planet-osm/perl/reorder-ways.pl\
	planet-osm/perl/planetosm-excerpt-area.pl\
	planet-osm/perl/planetosm-excerpt-tags.pl\
	planet-osm/perl/extract-germany.pl\
	planet-osm/perl/check_osm.pl\
	planet-osm/perl/planetcount.pl\
	planet-osm/python/utf8osmchecker.py\
	planet-osm/planet2mysql.rb\
	planet-osm/planet.rb\
	planet-osm/java/test-osm2csv.bat\
	planet-osm/java/mysql/loadcsv.sql\
	planet-osm/java/mysql/readme.txt\
	planet-osm/java/src/org/openstreetmap/util/Osm2Csv.java\
	planet-osm/java/build-osm2csv.bat\
	planet-osm/test-planet.osm\
	planet-osm/C/UTF8sanitizer\
	planet-osm/C/Makefile\
	planet-osm/C/UTF8sanitizer.exe\
	planet-osm/C/UTF8sanitizer.c\
	planet-osm/planet.sh\
	planet-osm/php/planet_to_mysql.phps\
	simplify/simplify.pl \
	pdf-atlas/Config/*.txt
