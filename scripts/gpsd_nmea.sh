#/bin/sh
echo "Swithching the local gpsd to NMEA Mode"
echo "N=0" | nc localhost 2947
