#!/bin/sh
echo "Switching the local gpsd to NMEA Mode"
(echo "N=0"; sleep 1 )  | telnet localhost 2947
