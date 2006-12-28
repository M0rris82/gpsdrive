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
    if [ $helplines -lt 140 ] ; then 
	echo "ERROR Starting geoinfo.pl (only $helpline Lines of Online Help)"
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


# ------------------------------------------------------------------ GpsDrive
# Test Gpsdrive -T with different Setup
for icon_theme in square.big square.small classic ; do 
    for LANG in en_US de_DE ; do 
	echo "------------------> check 'gpsdrive -T' LANG=$LANG icon_theme=$icon_theme"
	perl -p -i.bak \
	    -e "s/icon_theme = .*/icon_theme = $icon_theme/" ${HOME}/.gpsdrive/gpsdriverc
	#grep icon_theme ${HOME}/.gpsdrive/gpsdriverc
	perl -p -i.bak \
	    -e "s/dbname = geoinfo.*/dbname = geoinfotest/" ${HOME}/.gpsdrive/gpsdriverc

	./src/gpsdrive -S -T -D 1 >logs/gpsdrive_test_$LANG.txt 2>&1 
	rc=$?

	if [ $rc != 0 ] ; then
	    tail logs/gpsdrive_test_$LANG.txt
	    echo "Error starting gpsdrive -T (rc=$rc)"
	    exit 1;
	fi
	if grep -v -e 'Unknown Config Parameter .*reminder' logs/gpsdrive_test_$LANG.txt | \
	    grep -i -e 'Failed' -e 'ERROR'
	    then
	    grep -i -B 3  -e 'Failed' -e 'ERROR'  logs/gpsdrive_test_$LANG.txt
	    echo "Found (Error/Failed) in gpsdrive -T output "
	    exit 1;
	fi
	perl -p -i.bak \
	    -e "s/dbname = geoinfo.*/dbname = geoinfo/" ${HOME}/.gpsdrive/gpsdriverc
    done || exit 1
done || exit 1
