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

    *********************************************************************

$Log$
Revision 1.3  2005/04/02 12:10:12  tweety
2005.03.30 by Oddgeir Kvien <oddgeir@oddgeirkvien.com>
Canges made to import a map with one point and enter the scale

Revision 1.2  2005/03/27 00:44:42  tweety
eperated poi_type_list and streets_type_list
and therefor renaming the fields
added drop index before adding one
poi.*: a little bit more error handling
disabling poi and streets if sql is disabled
changed som print statements from \n.... to ...\n
changed some debug statements from debug to mydebug

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.5  2004/02/18 13:24:19  ganter
navigation

Revision 1.4  2004/02/07 15:53:38  ganter
replacing strcpy with g_strlcpy to avoid bufferoverflows

Revision 1.3  2004/02/02 09:03:42  ganter
2.08pre10

Revision 1.2  2004/02/02 07:12:57  ganter
inserted function calcxytopos, key x,y and right mouseclick are now correct in topomaps

Revision 1.1  2004/02/02 03:38:32  ganter
code cleanup

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gpsdrive.h>
#include <config.h>
#include <math.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif



/* variables */
extern gint ignorechecksum, mydebug, debug, mapistopo;
extern gdouble Ra[201];
extern gdouble zero_long, zero_lat, target_long, target_lat, dist;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu,
  int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;
extern gdouble pixelfact, posx, posy, angle_to_destination, direction,
  bearing;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle;
extern gdouble current_long, current_lat, old_long, old_lat, groundspeed,
  milesconv;



/* check NMEA checksum
   ARGS:    NMEA String
   RETURNS: TRUE if Checksumm is ok
 */
gint
checksum (gchar * text)
{
  gchar t[120], t2[10];
  gint i = 1, checksum = 0, j, orig;

  if (ignorechecksum)
    return TRUE;

  strncpy (t, text, 100);
  t[100] = 0;
  j = strlen (t) - 3;
  while (('\0' != t[i]) && (i < j))
    checksum = checksum ^ t[i++];
  g_strlcpy (t2, (t + j + 1),sizeof(t2));
  sscanf (t2, "%X", &orig);
  if (mydebug)
    g_print ("%s\norigchecksum: %X,my:%X\n", t, orig, checksum);

  if (orig == checksum)
    {
      g_strlcpy (text, t,1000);
      return TRUE;
    }
  else
    {
      g_print
	("\n*** NMEA checksum error!\nNMEA: %s\n is: %X, should be: %X\n", t,
	 orig, checksum);
      return FALSE;
    }
}

/* calculates lat and lon for the given position on the screen */
void
calcxytopos (int posx, int posy, gdouble * mylat, gdouble * mylon, gint zoom)
{
  int x, y, px, py;
  gdouble dif, lat, lon;

  x = posx;
  y = posy;
  px = (SCREEN_X_2 - x - xoff) * pixelfact / zoom;
  py = (-SCREEN_Y_2 + y + yoff) * pixelfact / zoom;


  if (mapistopo == FALSE)
    {
      lat = zero_lat - py / (Ra[(int) (100 + current_lat)] * M_PI / 180.0);
      lat = zero_lat - py / (Ra[(int) (100 + lat)] * M_PI / 180.0);
      lon =
	zero_long -
	px / ((Ra[(int) (100 + lat)] * M_PI / 180.0) *
	      cos (M_PI * lat / 180.0));

      dif = lat * (1 - (cos ((M_PI * fabs (lon - zero_long)) / 180.0)));
      lat = lat - dif / 1.5;
      lon =
	zero_long -
	px / ((Ra[(int) (100 + lat)] * M_PI / 180.0) *
	      cos (M_PI * lat / 180.0));
    }
  else
    {
      lat = zero_lat - py / (Ra[(int) (100 + 0)] * M_PI / 180.0);

      lon = zero_long - px / ((Ra[(int) (100 + 0)] * M_PI / 180.0));
    }

  *mylat = lat;
  *mylon = lon;

}

/* calculate xy pos of given lon/lat */ 
void
calcxy (gdouble * posx, gdouble * posy, gdouble lon, gdouble lat, gint zoom)
{
  gdouble dif;

  if (mapistopo == FALSE)
    *posx =
      (Ra[(int) (100 + lat)] * M_PI / 180.0) * cos (M_PI * lat / 180.0) *
      (lon - zero_long);
  else
    *posx = (Ra[(int) (100 + 0.0)] * M_PI / 180.0) * (lon - zero_long);

  *posx = SCREEN_X_2 + *posx * zoom / pixelfact;
  *posx = *posx - xoff;


  if (mapistopo == FALSE)
    {
      *posy = (Ra[(int) (100 + lat)] * M_PI / 180.0) * (lat - zero_lat);
      dif =
	Ra[(int) (100 + lat)] * (1 -
				 (cos ((M_PI * (lon - zero_long)) / 180.0)));
      *posy = *posy + dif / 1.85;
    }
  else
    *posy = (Ra[(int) (100 + lat)] * M_PI / 180.0) * (lat - zero_lat);

  *posy = SCREEN_Y_2 - *posy * zoom / pixelfact;
  *posy = *posy - yoff;
}

void
calcxymini (gdouble * posx, gdouble * posy, gdouble lon, gdouble lat,
	    gint zoom)
{
  gdouble dif;

  if (mapistopo == FALSE)
    *posx =
      (Ra[(int) (100 + lat)] * M_PI / 180.0) * cos (M_PI * lat / 180.0) *
      (lon - zero_long);
  else
    *posx = (Ra[(int) (100 + 0)] * M_PI / 180.0) * (lon - zero_long);

  *posx = 64 + *posx * zoom / (10 * pixelfact);
  *posx = *posx;

  *posy = (Ra[(int) (100 + lat)] * M_PI / 180.0) * (lat - zero_lat);
  if (mapistopo == FALSE)
    {
      dif =
	Ra[(int) (100 + lat)] * (1 -
				 (cos ((M_PI * (lon - zero_long)) / 180.0)));
      *posy = *posy + dif / 1.85;
    }
  *posy = 51 - *posy * zoom / (10 * pixelfact);
  *posy = *posy;
}


/* calculate Earth radius for given lat */
gdouble
calcR (gdouble lat)
{
  gdouble a = 6378.137, r, sc, x, y, z;
  gdouble e2 = 0.08182 * 0.08182; 
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


/* calculate distance to current position */
gdouble
calcdist2 (gdouble longi, gdouble lati)
{
  double a, a1, a2, c, d, dlon, dlat, sa, radiant = M_PI / 180;


  dlon = radiant * (current_long - longi);
  dlat = radiant * (current_lat - lati);

  if ((dlon == 0.0) && (dlat == 0.0))
    return 0.0;

  a1 = sin (dlat / 2);
  a2 = sin (dlon / 2);
  a =
    (a1 * a1) + cos (lati * radiant) * cos (current_lat * radiant) * a2 * a2;
  sa = sqrt (a);
  if (sa <= 1.0)
    c = 2 * asin (sa);
  else
    c = 2 * asin (1.0);
  d = (Ra[(int) (100 + current_lat)] + Ra[(int) (100 + lati)]) * c / 2.0;
  return milesconv * d / 1000.0;
}

/*  same as calcdist2, but much more precise */
gdouble
calcdist (gdouble longi, gdouble lati)
{
  gdouble a = 6378137.0;
  gdouble f = 1.0 / 298.25722210088;
  gdouble glat1, glat2, glon1, glon2;
  gdouble radiant = M_PI / 180;
  gdouble r, tu1, tu2, cu1, su1, cu2, s, baz, faz, x, sx, cx, sy, cy, y;
  gdouble sa, c2a, cz, e, c, d;
  gdouble eps = 0.5e-13;

/*   if (cpuload<10)
 *     {
 *       r = calcdist2 (longi, lati);
 *       return r;
 *     }
 */
  if (((lati - current_lat) == 0.0) && ((longi - current_long) == 0.0))
    return 0.0;

  glat1 = radiant * current_lat;
  glat2 = radiant * lati;
  glon1 = radiant * current_long;
  glon2 = radiant * longi;

  r = 1.0 - f;
  tu1 = r * sin (glat1) / cos (glat1);
  tu2 = r * sin (glat2) / cos (glat2);
  cu1 = 1.0 / sqrt (tu1 * tu1 + 1.0);
  su1 = cu1 * tu1;

  cu2 = 1.0 / sqrt (tu2 * tu2 + 1.0);
  s = cu1 * cu2;
  baz = s * tu2;
  faz = baz * tu1;
  x = glon2 - glon1;

  do
    {
      sx = sin (x);
      cx = cos (x);
      tu1 = cu2 * sx;
      tu2 = baz - su1 * cu2 * cx;
      sy = sqrt (tu1 * tu1 + tu2 * tu2);

      cy = s * cx + faz;
      y = atan2 (sy, cy);
      sa = s * sx / sy;
      c2a = -sa * sa + 1.0;
      cz = faz + faz;

      if (c2a > 0)
	cz = -cz / c2a + cy;
      e = cz * cz * 2.0 - 1.0;
      c = ((-3.0 * c2a + 4.0) * f + 4.0) * c2a * f / 16.0;
      d = x;

      x = ((e * cy * c + cz) * sy * c + y) * sa;
      x = (1.0 - c) * x * f + glon2 - glon1;
    }
  while (fabs (d - x) > eps);

  faz = atan2 (tu1, tu2);
  baz = atan2 (cu1 * sx, baz * cx - su1 * cu2) + M_PI;
  x = sqrt ((1.0 / r / r - 1.0) * c2a + 1.0) + 1.0;
  x = (x - 2.0) / x;
  c = 1.0 - x;
  c = (x * x / 4.0 + 1.0) / c;
  d = (0.375 * x * x - 1.0) * x;
  x = e * cy;
  s = 1.0 - e - e;
  s =
    ((((sy * sy * 4.0 - 3.0) * s * cz * d / 6.0 - x) * d / 4.0 +
      cz) * sy * d + y) * c * a * r;

  return milesconv * s / 1000.0;
}

/* This is an internally used function to create pixmaps. */
GdkPixbuf *
create_pixbuf (const gchar * filename)
{
  gchar pathname[200];
  GdkPixbuf *pixbuf;
  GError *error = NULL;

  if (!filename || !filename[0])
    return NULL;


  g_snprintf (pathname, sizeof (pathname), "%s/gpsdrive/%s", DATADIR,
	      filename);
  if (!pathname)
    {
      g_warning (_("Couldn't find pixmap file: %s"), pathname);
      return NULL;
    }

  pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
  if (!pixbuf)
    {
      fprintf (stderr, "Failed to load pixbuf file: %s: %s\n",
	       pathname, error->message);
    }
  return pixbuf;
}

GtkWidget* create_pixmap(GtkWidget *widget, const gchar     *filename)
{
  gchar pathname[200];
  GtkWidget *pixmap=NULL;
 
  if (!filename || !filename[0])
      return gtk_image_new ();
 
   g_snprintf (pathname, sizeof (pathname), "%s/gpsdrive/%s", DATADIR,
	      filename);

 
  pixmap = gtk_image_new_from_file (pathname);
  if (!pixmap)
    {
      g_warning (_("Couldn't find pixmap file: %s"), pathname);
      return gtk_image_new ();
    }

  return pixmap;
}

