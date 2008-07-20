#!/bin/sh

if echo "$@" | grep -q -e '--help' ; then
    echo "This script connects to the gpsd and switches your GPS to using nmea protocol"
    exit -1
fi

echo "Switching the local gpsd to NMEA Mode"
(echo "N=0"; sleep 1 )  | telnet localhost 2947
