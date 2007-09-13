# Makefile.am for tests
bin_SCRIPTS= $(wildcard *.sh)

tests:
	PERLLIB=./ PATH=./:$$PATH ./gpsdrive_test.sh

EXTRA_DIST = $(bin_SCRIPTS)
