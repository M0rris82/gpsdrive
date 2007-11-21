/***********************************************************************

Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>

Website: www.gpsdrive.de

Disclaimer: Please do not use for navigation. 

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    *********************************************************************/

/*
 * Everything but the necessary functionality for libgpsdrive stripped
 */

#include <stdio.h>
#include <math.h>

#include "map_priv.h"

/* ******************************************************************
 * calculate Earth radius for given lat 
 */
double
calcR2 (double lat)
{
	double a = 6378.137, r, sc, x, y, z;
	double e2 = 0.08182 * 0.08182;
	/* the radius of curvature of an ellipsoidal Earth in the plane of 
	 * the meridian is given by 
	 *
	 * R' = a * (1 - e^2) / (1 - e^2 * (sin(lat))^2)^(3/2) 
	 *
	 * where a is the equatorial radius, 
	 *
	 * b is the polar radius, and 
	 * e is the eccentricity of the ellipsoid = sqrt(1 - b^2/a^2) 
	 * 
	 * a = 6378.137 km (3963 mi) Equatorial radius (surface to center distance) 
	 * b = 6356.752 km (3950 mi) Polar radius (surface to center distance) 
	 * e = 0.08182 Eccentricity
	 */

	lat = lat * M_PI / 180.0;
	sc = sin (lat);
	x = a * (1.0 - e2);
	z = 1.0 - e2 * sc * sc;
	y = pow (z, 1.5);
	r = x / y;

	r = r * 1000.0;
	/*      g_print("\nR=%f",r); */
	return r;
}

/* **********************************************************************
 * Estimate the earth radius for given latitude
 */
double
lat2radius2 (double lat)
{
	if (lat > 90.0)
	{
		lat = lat - 90;
	}
	if (lat < -90.0)
	{
		lat = lat + 90;
	}
	if (lat > 100)
	{
		fprintf (stderr, "ERROR: lat2radius(lat %lf) out of bound\n", lat);
		lat = 100.0;
	};
	if (lat < -100)
	{
		fprintf (stderr, "ERROR: lat2radius(lat %lf) out of bound\n", lat);
		lat = -100.0;
	};
	return calcR2((int) lat);
}

/*  **********************************************************************
 * calculates lat and lon for the given position on the screen 
 *
 * modified version of calcxytopos
 */
void
calcxytopos2 (int posx, int posy, double * mylat, double * mylon,
	      double zero_lat, double zero_long, double mapscale)
{
  int x, y, px, py;
  double dif, lat, lon;
  double current_lat = zero_lat; // ??????

  double pixelfact = mapscale / PIXELFACT;

  x = posx;
  y = posy;
  //px = (MAP_X_2 - x - xoff) * pixelfact / current.zoom;
  //py = (-MAP_Y_2 + y + yoff) * pixelfact / current.zoom;
  px = -posx * pixelfact;
  py = posy * pixelfact;

  lat = zero_lat - py / (lat2radius2 (current_lat) * M_PI / 180.0);
  lat = zero_lat - py / (lat2radius2 (lat) * M_PI / 180.0);

  if (lat > 360)
    lat = lat - 360.0;
  if (lat < -360)
    lat = lat + 360.0;

  lon = zero_long - px / ((lat2radius2 (lat) * M_PI / 180.0) *
			  cos (M_PI * lat / 180.0));

  dif = lat * (1 -
	       (cos ((M_PI * fabs (lon - zero_long)) / 180.0)));
  lat = lat - dif / 1.5;

  if (lat > 360)
    lat = 360.0;
  if (lat < -360)
    lat = -360.0;

  lon = zero_long -
    px / ((lat2radius2 (lat) * M_PI / 180.0) *
	  cos (M_PI * lat / 180.0));

  if (lat > 360)
    lat = 360.0;
  if (lat < -360)
    lat = -360.0;
  if (lon > 180)
    lon = 180.0;
  if (lon < -180)
    lon = -180.0;

  *mylat = lat;
  *mylon = lon;
}

/* ******************************************************************
 * calculate xy pos of given lon/lat
 *
 * modified version of calcxy
 */
void
calcxy2 (double * posx, double * posy, double lat, double lon,
	 double zero_lat, double zero_long, double mapscale)
{
  double dif;

  double pixelfact = mapscale / PIXELFACT;

  *posx = (lat2radius2 (lat) * M_PI / 180.0) * cos (M_PI * lat /
						   180.0) *
    (lon - zero_long);

  *posx = *posx / pixelfact;

  *posy = (lat2radius2 (lat) * M_PI / 180.0) * (lat - zero_lat);
  dif = lat2radius2 (lat) * (1 -
			    (cos
			     ((M_PI * (lon - zero_long)) /
			      180.0)));
  *posy = *posy + dif / 1.85;

  *posy = - *posy / pixelfact;
}
