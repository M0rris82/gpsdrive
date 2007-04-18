# Makefile.am for scripts/perl_lib

gpsdrivedir = $(datadir)/gpsdrive

bin_SCRIPTS= \
	import/osmtrackfilter/osmtrackfilter.pl
	planet-mirror/planet-mirror.pl \
	osm-extract/osm-subset.pl \
	simplify/simplify.pl \
	osm2csv/osm2csv.pl \
	planet.osm/C/UTF8sanitizer

UTF8sanitizerdir = $(bindir)
UTF8sanitizer_PROGRAMS = UTF8sanitizer
UTF8sanitizer_SOURCES = planet.osm/C/UTF8sanitizer.c
#PRGS = planet.osm/C/UTF8sanitizer.c

EXTRA_DIST = $(bin_SCRIPTS) \
	$(PRGS) \
	osm2csv/create_all.sh \
	import/csv2osm/csv2osm.pl\
	import/coast_import/coast_upload.pl\
	import/coast_import/osm.pm\
	import/srtm2osm/merge.pl\
	import/srtm2osm/mkcntr.pl\
	import/srtm2osm/extractdata.pl\
	import/osmtrackfilter/osmtrackfilter.pl\
	export/osm2singleline_per_tag/osm2singleline_per_tag.pl\
	export/CityKML/run.pl\
	export/CityKML/download_specific.pl\
	export/CityKML/download.pl\
	export/CityKML/package.pl\
	export/osm2csv/osm2csv.pl\
	export/osmgarminmap/feature-list-csv2xml.pl\
	export/osm2kml/osm2kml.py\
	osm-extract/planetosm-excerpt-tags.pl\
	osm-extract/polygons/extract-germany.pl\
	osm-extract/polygons/extract-polygon.pl\
	osm-extract/osm-subset.pl\
	filter/osm-length/osm-length-2.pl\
	filter/simplify/simplify.pl\
	planet.osm/perl/planetosm-to-db.pl\
	planet.osm/perl/reorder-ways.pl\
	planet.osm/perl/check_osm.pl\
	planet.osm/perl/planetcount.pl\
	planet.osm/python/utf8osmchecker.py
