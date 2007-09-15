# Makefile.am for tests
bin_SCRIPTS= gpsdrive_test.sh

tests:
	PERLLIB=./ PATH=./:$$PATH ./gpsdrive_test.sh

TESTS=./gpsdrive_test.sh
EXTRA_DIST=$(bin_SCRIPTS)
