//LatLong- UTM conversion..h
//definitions for lat/long to UTM and UTM to lat/lng conversions
//This file originated from http://www.gpsy.com/gpsinfo/geotoutm/
// converted to C code by Russell Harding Jan, 2004  -- hardingr at cunap dot com



#ifndef LATLONGCONV
#define LATLONGCONV

void LLtoUTM(const float Lat, const float Long, 
			 float *UTMNorthing, float *UTMEasting, int* UTMZone);
char UTMLetterDesignator(float Lat);

#endif
