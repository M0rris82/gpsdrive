#!/bin/bash
# ============================================ 
# Run tests on Gpsdrive gpsfetchmap.pl


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

