//LatLong- UTM conversion.cpp
//Lat Long - UTM, UTM - Lat Long conversions
//This file originated from http://www.gpsy.com/gpsinfo/geotoutm/
// converted to C code by Russell Harding Jan, 2004  -- hardingr at cunap dot com


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "LatLong-UTMconversion.h"


/*Reference ellipsoids derived from Peter H. Dana's website- 
  http://www.utexas.edu/depts/grg/gcraft/notes/datum/elist.html
  Department of Geography, University of Texas at Austin
  Internet: pdana@mail.utexas.edu
  3/22/95

  Source
  Defense Mapping Agency. 1987b. DMA Technical Report: Supplement to Department of Defense World Geodetic System
  1984 Technical Report. Part I and II. Washington, DC: Defense Mapping Agency
*/
const double PI = 3.14159265;
const double FOURTHPI = 3.14159265 / 4.0;
const double deg2rad = 3.14159265 / 180.0;
const double rad2deg = 180.0 / 3.14159265;




void
LLtoUTM (const float Lat, const float Long,
	 float *UTMNorthing, float *UTMEasting, int *UTMZone)
{
	//converts lat/long to UTM coords.  Equations from USGS Bulletin 1532 
	//East Longitudes are positive, West longitudes are negative. 
	//North latitudes are positive, South latitudes are negative
	//Lat and Long are in decimal degrees
	//Written by Chuck Gantz- chuck.gantz@globalstar.com

	//  float a = ellipsoid[ReferenceEllipsoid].EquatorialRadius;
	float a = 6378137.0;
	//    float eccSquared = ellipsoid[ReferenceEllipsoid].eccentricitySquared;
	float eccSquared = 0.00669438;
	float k0 = 0.9996;

	float LongOrigin;
	float eccPrimeSquared;
	float N, T, C, A, M;

	//Make sure the longitude is between -180.00 .. 179.9
	float LongTemp = (Long + 180) - (int) ((Long + 180) / 360) * 360 - 180;	// -180.00 .. 179.9;

	float LatRad = Lat * deg2rad;
	float LongRad = LongTemp * deg2rad;
	float LongOriginRad;
	float Test;
	int ZoneNumber;

	ZoneNumber = (int) ((LongTemp + 180) / 6) + 1;

	if (Lat >= 56.0 && Lat < 64.0 && LongTemp >= 3.0 && LongTemp < 12.0)
		ZoneNumber = 32;

	// Special zones for Svalbard
	if (Lat >= 72.0 && Lat < 84.0)
	{
		if (LongTemp >= 0.0 && LongTemp < 9.0)
			ZoneNumber = 31;
		else if (LongTemp >= 9.0 && LongTemp < 21.0)
			ZoneNumber = 33;
		else if (LongTemp >= 21.0 && LongTemp < 33.0)
			ZoneNumber = 35;
		else if (LongTemp >= 33.0 && LongTemp < 42.0)
			ZoneNumber = 37;
	}
	LongOrigin = (ZoneNumber - 1) * 6 - 180 + 3;	//+3 puts origin in middle of zone
	LongOriginRad = LongOrigin * deg2rad;

	//compute the UTM Zone from the latitude and longitude
	//  sprintf(UTMZone, "%f%c", ZoneNumber, UTMLetterDesignator(Lat));

	*UTMZone = ZoneNumber;

	eccPrimeSquared = (eccSquared) / (1 - eccSquared);

	N = a / sqrt (1 - eccSquared * sin (LatRad) * sin (LatRad));
	T = tan (LatRad) * tan (LatRad);
	C = eccPrimeSquared * cos (LatRad) * cos (LatRad);
	A = cos (LatRad) * (LongRad - LongOriginRad);

	M = a * ((1 - eccSquared / 4 - 3 * eccSquared * eccSquared / 64 -
		  5 * eccSquared * eccSquared * eccSquared / 256) * LatRad -
		 (3 * eccSquared / 8 + 3 * eccSquared * eccSquared / 32 +
		  45 * eccSquared * eccSquared * eccSquared / 1024) * sin (2 *
									   LatRad)
		 + (15 * eccSquared * eccSquared / 256 +
		    45 * eccSquared * eccSquared * eccSquared / 1024) *
		 sin (4 * LatRad) -
		 (35 * eccSquared * eccSquared * eccSquared / 3072) * sin (6 *
									   LatRad));

	*UTMEasting = (float) (k0 * N * (A + (1 - T + C) * A * A * A / 6
					 + (5 - 18 * T + T * T + 72 * C -
					    58 * eccPrimeSquared) * A * A *
					 A * A * A / 120) + 500000.0);

	Test = (float) (k0 *
			(M +
			 N * tan (LatRad) * (A * A / 2 +
					     (5 - T + 9 * C +
					      4 * C * C) * A * A * A * A /
					     24 + (61 - 58 * T + T * T +
						   600 * C -
						   330 * eccPrimeSquared) *
					     A * A * A * A * A * A / 720)));
	//  *UTMNorthing = (float)(k0*(M+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
	//                           + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));
	*UTMNorthing = Test;
	if (Lat < 0)
		*UTMNorthing += 10000000.0;	//10000000 meter offset for southern hemisphere
}

char
UTMLetterDesignator (float Lat)
{
	//This routine determines the correct UTM letter designator for the given latitude
	//returns 'Z' if latitude is outside the UTM limits of 84N to 80S
	//Written by Chuck Gantz- chuck.gantz@globalstar.com
	char LetterDesignator;

	if ((84 >= Lat) && (Lat >= 72))
		LetterDesignator = 'X';
	else if ((72 > Lat) && (Lat >= 64))
		LetterDesignator = 'W';
	else if ((64 > Lat) && (Lat >= 56))
		LetterDesignator = 'V';
	else if ((56 > Lat) && (Lat >= 48))
		LetterDesignator = 'U';
	else if ((48 > Lat) && (Lat >= 40))
		LetterDesignator = 'T';
	else if ((40 > Lat) && (Lat >= 32))
		LetterDesignator = 'S';
	else if ((32 > Lat) && (Lat >= 24))
		LetterDesignator = 'R';
	else if ((24 > Lat) && (Lat >= 16))
		LetterDesignator = 'Q';
	else if ((16 > Lat) && (Lat >= 8))
		LetterDesignator = 'P';
	else if ((8 > Lat) && (Lat >= 0))
		LetterDesignator = 'N';
	else if ((0 > Lat) && (Lat >= -8))
		LetterDesignator = 'M';
	else if ((-8 > Lat) && (Lat >= -16))
		LetterDesignator = 'L';
	else if ((-16 > Lat) && (Lat >= -24))
		LetterDesignator = 'K';
	else if ((-24 > Lat) && (Lat >= -32))
		LetterDesignator = 'J';
	else if ((-32 > Lat) && (Lat >= -40))
		LetterDesignator = 'H';
	else if ((-40 > Lat) && (Lat >= -48))
		LetterDesignator = 'G';
	else if ((-48 > Lat) && (Lat >= -56))
		LetterDesignator = 'F';
	else if ((-56 > Lat) && (Lat >= -64))
		LetterDesignator = 'E';
	else if ((-64 > Lat) && (Lat >= -72))
		LetterDesignator = 'D';
	else if ((-72 > Lat) && (Lat >= -80))
		LetterDesignator = 'C';
	else
		LetterDesignator = 'Z';	//This is here as an error flag to show that the Latitude is outside the UTM limits

	return LetterDesignator;
}
