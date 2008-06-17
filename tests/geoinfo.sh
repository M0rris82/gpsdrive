#!/bin/bash
# ============================================ 
# Run some tests on Gpsdrive geoinfo.pl


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
