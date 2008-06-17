#!/bin/bash
# ============================================ 
# Run some tests on Gpsdrive


mkdir -p logs

# ------------------------------------------------------------------ GpsDrive
# Test Gpsdrive -T with different Setup
PWD=`pwd`/tests
mkdir -p "$PWD/maps"

USER_NAME=`id -u -n`
if false; then
echo "------------> Screenshots...."
for LANG in en_US de_DE ; do 
    echo "-------------> check LANG=$LANG"
    for ICON_THEME in square.big square.small classic.big classic.small; do 
	echo "-------------> check icon_theme=$ICON_THEME"
	for USER_INTERFACE in car desktop pda ; do 
	    for MAPNIK in 0 1  ; do 
		
		perl -p \
		    -e "s,PWD,$PWD,g;
                        s/USER/$USER_NAME/g;
                        s/icon_theme = .*/icon_theme = $ICON_THEME/g;
                        s/mapnik = .*/mapnik = $MAPNIK/g;
                        " <tests/gpsdriverc-in >tests/gpsdriverc
		./build/src/gpsdrive --geometry 800x600 -S tests/ -S ./tests -C tests/gpsdriverc  -M $USER_INTERFACE >logs/gpsdrive_test_$LANG.txt 2>&1 

	    done || exit 1
	done || exit 1
    done || exit 1
done || exit 1
fi


for LANG in en_US de_DE ; do 
    echo "-------------> check LANG=$LANG"
    for ICON_THEME in square.big square.small classic.big classic.small; do 
	echo "-------------> check icon_theme=$ICON_THEME"
	for USER_INTERFACE in car desktop pda ; do 
	    for MAPNIK in 0 1  ; do 
		echo "------------------> check './build/src/gpsdrive -s -C tests/gpsdriverc -M $USER_INTERFACE -D 1 -T '  mapnik = $MAPNIK"

		perl -p \
		    -e "s,PWD,$PWD,g;" \
		    -e "s/icon_theme = .*/icon_theme = $ICON_THEME/;" \
		    -e "s/mapnik = .*/mapnik = $MAPNIK/" <tests/gpsdriverc-in >tests/gpsdriverc
		cp tests/gpsdriverc tests/gpsdriverc-pre

		# --server-args="-screen 0 1280x1024x16" 
		xvfb-run \
		    ./build/src/gpsdrive \
		    -s \
		    -C tests/gpsdriverc \
		    -M $USER_INTERFACE \
		    -D 1 \
		    -T >logs/gpsdrive_test_$LANG.txt 2>&1 
		rc=$?

		if [ $rc != 0 ] ; then
		    cat logs/gpsdrive_test_$LANG.txt
		    echo "Error starting gpsdrive -T (rc=$rc)"
		    exit 1;
		fi
		if grep -v \
		    -e 'db_postgis_query: an error occured while trying to read from the database!' \
		    -e 'Gtk-CRITICAL \*\*: gtk_widget_set_sensitive: assertion .GTK_IS_WIDGET .widget.. failed' \
		    -e 'gda_connection_add_event: assertion .GDA_IS_CONNECTION .cnc.. failed' \
		    -e 'Could not find provider PostgreSQL in the current setup' \
		    -e 'cannot open image.*rendering' \
		    -e 'Error: Could not find provider PostgreSQL in the current setup' \
		    -e 'Unknown Config Parameter .*reminder' logs/gpsdrive_test_$LANG.txt | \
		    grep -i -e 'Failed' -e 'ERROR' -e 'CRITICAL' -e 'exception'
		    then
		    echo "Error String Check: -----------------------"
		    grep -color -i -B 3  -A 2 -e 'Failed' -e 'ERROR'  -e 'CRITICAL' -e 'exception' logs/gpsdrive_test_$LANG.txt
		    echo "Found (Error/Failed/CRITICAL) in 'gpsdrive -T output ( LANG=$LANG icon_theme=$ICON_THEME Userinterface=$USER_INTERFACE)'"
		    exit 1;
		fi

		if ! diff -u tests/gpsdriverc-pre tests/gpsdriverc ; then
		    echo "!!!!! gpsdriverc was modified by test"
		    exit -1 
		fi
	    done || exit 1
	done || exit 1
    done || exit 1
done || exit 1

