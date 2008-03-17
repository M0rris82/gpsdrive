#!/bin/bash
# ============================================ 
# Run some tests on Gpsdrive


mkdir -p logs

(
    if  [ ! -d scripts ] ; then 
	echo ""
	echo "!!!!!!!!! WARNING: Directory scripts not found."
	echo "                   Please run from Top Level Directory"
	echo ""
	exit -1 
    fi

    cd ./scripts

    # ------------------------------------------------------------------ geoinfo
    echo "------------------> check geoinfo.pl -h"
    ./geoinfo.pl  -h >/dev/null
    rc=$?
    if [ $rc != 1 ] ; then
	echo "Wrong Exit Code $rc for geoinfo.pl"
	exit 1
    fi
    
    helplines=`./geoinfo.pl  -h | wc -l`
    if [ $helplines -lt 117 ] ; then 
	echo "ERROR Starting geoinfo.pl (only $helplines Lines of Online Help)"
	exit 1
    fi


    # ------------------------------------------------------------------ gpsfetchmap
    echo "------------------> check gpsfetchmap.pl -h"
    ./gpsfetchmap.pl  -h >/dev/null
    rc=$?
    if [ $rc != 1 ] ; then
	echo "Wrong Exit Code $rc for gpsfetchmap.pl"
	exit 1;
    fi
    
    helplines=`./gpsfetchmap.pl  -h | wc -l`
    if [ $helplines -lt 200 ] ; then 
	echo "ERROR Starting gpsfetchmap.pl (only $helpline Lines of Online Help)"
	exit 1
    fi
) || exit 1



# ------------------------------------------------------------------ geoinfo
# No longer needed with the mysqllite files
if false ; then (
    if [ -z "$DBUSER" -o -z "$DBPASS" ] ; then
	echo "!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!! no creation test for Database"
    else
	cd scripts
	echo "------------------> check geoinfo.pl --create-db"
	echo "drop database geoinfotest " | mysql -u $DBUSER -p$DBPASS
	./geoinfo.pl  --db-name=geoinfotest --db-user=$DBUSER --db-password=$DBPASS \
	    --create-db --fill-defaults >../logs/geoinfo_test.txt 2>&1 
	rc=$?
	if [ $rc != 0 ] ; then
	    echo "Wrong Exit Code $rc for geoinfo.pl --create-db"
	    cat ../logs/geoinfo_test.txt
	    exit 1
	fi
    fi
) || exit 1
fi

# ------------------------------------------------------------------ GpsDrive
# Test Gpsdrive -T with different Setup
PWD=`pwd`/tests
mkdir -p "$PWD/maps"

if false; then
echo "------------> Screenshots...."
for LANG in en_US de_DE ; do 
    echo "-------------> check LANG=$LANG"
    for ICON_THEME in square.big square.small classic.big classic.small; do 
	echo "-------------> check icon_theme=$ICON_THEME"
	for USER_INTERFACE in car desktop pda ; do 
	    for MAPNIK in 0 1  ; do 
		perl -p \
		    -e "s,PWD,$PWD,g;s/icon_theme = .*/icon_theme = $ICON_THEME/;s/mapnik = .*/mapnik = $MAPNIK/" <tests/gpsdriverc-in >tests/gpsdriverc
		./build/src/gpsdrive --geometry 800x600 -S tests/ -a -S ./tests -C tests/gpsdriverc  -M $USER_INTERFACE >logs/gpsdrive_test_$LANG.txt 2>&1 

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
		echo "------------------> check './build/src/gpsdrive -T -a -s -D 1 -C tests/gpsdriverc -M $USER_INTERFACE '  mapnik = $MAPNIK"

		perl -p \
		    -e "s,PWD,$PWD,g;s/icon_theme = .*/icon_theme = $ICON_THEME/;s/mapnik = .*/mapnik = $MAPNIK/" <tests/gpsdriverc-in >tests/gpsdriverc
		cp tests/gpsdriverc tests/gpsdriverc-pre

		./build/src/gpsdrive --geometry 800x600 -T -a -s -D 1 -C tests/gpsdriverc -M $USER_INTERFACE >logs/gpsdrive_test_$LANG.txt 2>&1 
		rc=$?

		if [ $rc != 0 ] ; then
		    cat logs/gpsdrive_test_$LANG.txt
		    echo "Error starting gpsdrive -T (rc=$rc)"
		    exit 1;
		fi
		if grep -v \
		    -e 'Gtk-CRITICAL \*\*: gtk_widget_set_sensitive: assertion .GTK_IS_WIDGET .widget.. failed' \
		    -e 'gda_connection_add_event: assertion .GDA_IS_CONNECTION .cnc.. failed' \
		    -e 'cannot open image.*rendering' \
		    -e 'Error: Could not find provider PostgreSQL in the current setup' \
		    -e 'Unknown Config Parameter .*reminder' logs/gpsdrive_test_$LANG.txt | \
		    grep -i -e 'Failed' -e 'ERROR' -e 'CRITICAL'
		    then
		    grep -i -B 3  -e 'Failed' -e 'ERROR'  -e 'CRITICAL' logs/gpsdrive_test_$LANG.txt
		    echo "Found (Error/Failed/CRITICAL) in 'gpsdrive -T output ( LANG=$LANG icon_theme=$ICON_THEME Userinterface=$USER_INTERFACE)'"
		    exit 1;
		fi

		if ! diff -u tests/gpsdriverc-pre tests/gpsdriverc ; then
		    echo "gpsdriverc was modified by test"
		    exit -1 
		fi
	    done || exit 1
	done || exit 1
    done || exit 1
done || exit 1

