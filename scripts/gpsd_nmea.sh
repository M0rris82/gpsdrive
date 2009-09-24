#!/bin/sh

if echo "$@" | grep -q -e '--help' ; then
    echo "This script connects to Gpsd and switches your GPS to use the"
    echo "  NMEA protocol. Sometimes the GPS can get stuck using, e.g., the"
    echo "  SiRF binary protocol. Note this may not be needed, as modern"
    echo "  versions of Gpsd output pseudo-NMEA sentences when the client"
    echo "  requests 'R' raw-mode."
    exit 0
fi

echo "Switching the GPS connected to the local Gpsd into NMEA Mode ..."
(echo "N=0"; sleep 1 )  | telnet localhost 2947
