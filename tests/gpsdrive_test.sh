#!/bin/bash
# ============================================ 
# Run some tests on Gpsdrive


mkdir -p logs

# Option to use virtual Framebuffer.
# This way no gpsdrive is shown on the real X-Display
if echo "$@" | grep -i -e '--use-xvfb'; then
    USE_XVFB="xvfb-run "
fi

if echo "$@" | grep -i -e '--short'; then
    SHORT=true
else
    short=false
fi

if echo "$@" | grep -i -e '--screenshots'; then
    DO_SCREENSHOTS=true
else
    DO_SCREENSHOTS=false
fi


if echo "$@" | grep -i -e '-h'; then
    echo "$0: Run Tests for Gpsdrive"
    echo "   -h             This Help"
    echo "   --use-xvfb     Use Virtual Framebuffer"
    echo "   --short        Do only some basic tests"
    echo "   --screenshots  Do some Screenshots"
    exit
fi

# ------------------------------------------------------------------ GpsDrive
# Test Gpsdrive -T with different Setup
PWD=`pwd`/tests
mkdir -p "$PWD/maps"
mkdir -p "$PWD/tracks"

USER_NAME=`id -u -n`

for LANG in en_US de_DE ; do 
    echo "-------------> check LANG=$LANG"
    for ICON_THEME in square.big square.small classic.big classic.small; do 
	echo "-------------> check icon_theme=$ICON_THEME"
	for USER_INTERFACE in desktop car pda ; do 
	    for MAPNIK in 0 1  ; do 
		echo "------------------> check './build/src/gpsdrive -s -C tests/gpsdriverc -M $USER_INTERFACE -D 1 -T '  mapnik = $MAPNIK"

		perl -p \
		    -e "s,PWD,$PWD,g;" \
		    -e "s/icon_theme = .*/icon_theme = $ICON_THEME/;" \
		    -e "s/mapnik = .*/mapnik = $MAPNIK/" <tests/gpsdriverc-in >tests/gpsdriverc
		cp tests/gpsdriverc tests/gpsdriverc-pre

		# --server-args="-screen 0 1280x1024x16" 
		$USE_XVFB \
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
	    $SHORT && continue
	done || exit 1
    done || exit 1
done || exit 1

# -------------------------------------------- Screenshots
if $DO_SCREENSHOTS; then
    echo "------------> Screenshots...."
    for LANG in en_US de_DE ; do 
	echo "-------------> check LANG=$LANG"
	for ICON_THEME in square.big square.small classic.big classic.small; do 
	    echo "-------------> check icon_theme=$ICON_THEME"
	    for USER_INTERFACE in  desktop car pda ; do 
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

