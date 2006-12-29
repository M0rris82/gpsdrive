# Makefile.am for tests
bin_SCRIPTS= *.sh

tests:
	PERLLIB=./ PATH=./:$$PATH ./gpsdrive_test.sh

gpsdrivedir = $(datadir)/gpsdrive

EXTRA_DIST = $(bin_SCRIPTS)
