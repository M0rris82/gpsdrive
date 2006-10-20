#!/bin/sh
echo
echo --------------- Rebuilding icons.txt ---------------
echo
./rebuild-icons.txt.sh
echo
echo -------- Creating database entries for POIs --------
echo
geoinfo.pl --fill-defaults
echo
echo ------------ Rebuilding poi_types.html -------------
echo
./rebuild-poi_types.html.pl
echo

