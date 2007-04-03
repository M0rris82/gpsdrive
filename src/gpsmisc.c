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

#include <locale.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


/* variables */
extern gint ignorechecksum, mydebug;
extern gdouble zero_lon, zero_lat, target_lon, target_lat, dist;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu,
  int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;
extern gdouble pixelfact, posx, posy, angle_to_destination, direction,
  bearing;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle;
extern gdouble current_lon, current_lat, old_lon, old_lat, milesconv;
static gchar gradsym[] = "\xc2\xb0";
extern gchar local_config_mapdir[500];
gdouble lat2RadiusArray[101];

/* **********************************************************************
 * Build array for earth radii 
 */
void 
init_lat2RadiusArray(){
    int i;
    for (i = 0; i <= 100; i++)
	lat2RadiusArray[i] = calcR (i);
}

/* **********************************************************************
 * Estimate the Earth radius in meter for given lat in Degrees
 */
gdouble
lat2radius (gdouble lat)
{
    lat = fabs(lat);

  // the known undef values
  if ( lat > 999 )
      lat=0;
  
  while (lat > 360)
      {
	  lat = lat - 360.0;
      };
  

  if (lat > 180)
      {
	  if (mydebug > 20)
	      fprintf (stderr, "ERROR: lat2radius(lat %f) out of bound\n", lat);
      };

  if (lat > 90  && lat < 180.0)
      {
	  lat = 180.0 - lat;
      }

  if (lat > 90)
      {
	  if (mydebug > 20)
	      fprintf (stderr, "ERROR: lat2radius(lat %f) still out of bound\n", lat);
	  return 1;
      };

  return lat2RadiusArray[(int) (lat)];
}


/* **********************************************************************
 * Estimate the Earth radius in (meter * PI/180 ) for given lat in Degrees
 */
gdouble
lat2radius_pi_180 (gdouble lat)
{
    return ( lat2radius  (lat) * M_PI / 180.0);
}

/* ******************************************************************
 * calculate Earth radius in meter for given lat in Degrees
 */
gdouble
calcR (gdouble lat)
{
  gdouble a = 6378.137, r, sc, x, y, z;
  gdouble e2 = 0.08182 * 0.08182;
  gdouble lat_pi;
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

  lat_pi = lat * M_PI / 180.0;
  sc = sin (lat_pi);
  x = a * (1.0 - e2);
  z = 1.0 - e2 * sc * sc;
  y = pow (z, 1.5);
  r = x / y;

  r = r * 1000.0;
  // g_print("R(%f)=%f\n",lat,r); 
  return r;
}


/* ******************************************************************
 * calculate distance from (lat/lon) to current position (current_lat/current_lon)
 */
gdouble
calcdist2 (gdouble lon, gdouble lat)
{
  double a, a1, a2, c, d, dlon, dlat, sa, radiant = M_PI / 180;


  dlon = radiant * (current_lon - lon);
  dlat = radiant * (current_lat - lat);

  if ((dlon == 0.0) && (dlat == 0.0))
    return 0.0;

  a1 = sin (dlat / 2);
  a2 = sin (dlon / 2);
  a = (a1 * a1) +
    cos (lat * radiant) * cos (current_lat * radiant) * a2 * a2;
  sa = sqrt (a);
  if (sa <= 1.0)
    c = 2 * asin (sa);
  else
    c = 2 * asin (1.0);
  d = (lat2radius (current_lat) + lat2radius (lat)) * c / 2.0;
  return milesconv * d / 1000.0;
}

/* ******************************************************************
 * same as calcdist2, but much more precise 
 */
gdouble
calc_wpdist (gdouble lon1, gdouble lat1, gdouble lon2, gdouble lat2, gint from_current)
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
   *       r = calcdist2 (lon, lat);
   *       return r;
   *     }
   */
	
	if (from_current)
	{
		lon2 = current_lon;
		lat2 = current_lat;
	}

	if (((lat1 - lat2) == 0.0) && ((lon1 - lon2) == 0.0))
		return 0.0;

	glat1 = radiant * lat2;
	glat2 = radiant * lat1;
	glon1 = radiant * lon2;
	glon2 = radiant * lon1;

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
	s = ((((sy * sy * 4.0 - 3.0) * s * cz * d / 6.0 - x) * d / 4.0 +
		cz) * sy * d + y) * c * a * r;

	return milesconv * s / 1000.0;
}


/* ******************************************************************
 * Convenience Replacement for calcdist (now in calc_wpdist)
 */
gdouble
calcdist (gdouble lon, gdouble lat)
{
	return calc_wpdist (lon, lat, 0, 0, TRUE);
}


/* ******************************************************************
 * This is an internally used function to create pixmaps. 
 */
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
      if (mydebug > 5)
	fprintf (stderr, _("Couldn't find pixmap file: %s"), pathname);
      else
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



/* *****************************************************************************
 * Convert a coordinate to a gchar
 * mode is either LATLON_DMS, LATLON MINDEC or LATLON_DEGDEC 
 * By Oddgeir Kvien, to adopt for 3-way lat/lon display 
 */
void
coordinate2gchar (gchar * buff, gint buff_size, gdouble pos, gint islat,
		  gint mode)
{
  gint grad, min, minus = FALSE;
  gdouble minf, sec;
  grad = (gint) pos;
  if (pos < 0)
    {
      grad = -grad;
      pos = -pos;
      minus = TRUE;
    }

  minf = (pos - (gdouble) grad) * 60.0;
  min = (gint) minf;
  sec = (minf - (gdouble) min) * 60.0;
  switch (mode)
    {
    case LATLON_DMS:
      if (islat)
	g_snprintf (buff, buff_size, "%d%s%.2d'%05.2f''%c", grad, gradsym,
		    min, sec, (minus) ? 'S' : 'N');
      else
	g_snprintf (buff, buff_size, "%d%s%.2d'%05.2f''%c", grad, gradsym,
		    min, sec, (minus) ? 'W' : 'E');
      break;

    case LATLON_MINDEC:
      if (islat)
	g_snprintf (buff, buff_size, "%d%s%.3f'%c", grad, gradsym, minf,
		    (minus) ? 'S' : 'N');
      else
	g_snprintf (buff, buff_size, "%d%s%.3f'%c", grad, gradsym, minf,
		    (minus) ? 'W' : 'E');
      break;

    case LATLON_DEGDEC:
      if (islat)
	g_snprintf (buff, buff_size, "%8.5f%s%c", pos, gradsym,
		    (minus) ? 'S' : 'N');
      else
	g_snprintf (buff, buff_size, "%8.5f%s%c", pos, gradsym,
		    (minus) ? 'W' : 'E');
      break;
    }
}

/* *****************************************************************************
 * convert a coordinate in an gchar into a gdouble decimal value
 * Sideeffect inside text all , are replaced by .
 */
void
coordinate_string2gdouble (const gchar * intext, gdouble * dec)
{
  gint grad;
  gdouble sec;
  gint min;
  //    gdouble dec = -1002.0;
  gchar s2=' ';
  gdouble fmin;
  gchar text[50];

  g_strlcpy(text,intext,sizeof(text));

  *dec = -1002.0;

  setlocale(LC_NUMERIC, "C");

  // HACK: Fix usage of , and . inside Float-strings
  g_strdelimit (text, ",", '.');

  /*
   * Handle either DegMinSec or MinDec formats
   */
  if (4 == sscanf (text, "%d\xc2\xb0%d'%lf''%c", &grad, &min, &sec, &s2))
    {
      /*   sscanf(s1,"%f",&sec); */
      *dec = grad + min / 60.0 + sec / 3600.0;
    }
  else if (3 == sscanf (text, "%d\xc2\xb0%lf'%c", &grad, &fmin, &s2))
    {
      *dec = grad + fmin / 60.0;
    }
  else if (2 == sscanf (text, "%lf\xc2\xb0%c", dec, &s2))
    {
    }
  else if (1 == sscanf (text, "%lf", dec))
    {
      /* Is already decimal */
    }
  else
    {
      /* TODO: handle bad format gracefully */
      fprintf (stderr,
	       "ERROR BAD FORMAT in coordinate_string2gdouble(%s)-->%f\n",
	       text, *dec);
    }

  if (s2 == 'W')
    *dec *= -1.0;
  if (s2 == 'S')
    *dec *= -1.0;

  if (mydebug > 99)
    fprintf (stderr, "coordinate_string2gdouble('%s')-->%f\n", text, *dec);
}

/* *****************************************************************************
 */
void
checkinput (gchar * text)
{
  if (mydebug > 50)
    fprintf (stderr, "checkinput(%s)\n", text);
  gdouble dec;
  coordinate_string2gdouble (text, &dec);
  g_snprintf (text, 20, "%.6f", dec);
  if (mydebug > 50)
    fprintf (stderr, "checkinput -->'%s'\n", text);
}


/* ******************************************************************
 * Find File in the local directories, User Directories or System Directories
 */
gint
file_location(gchar * filename, gchar *file_location){
    struct stat buf;
    gchar mappath[2048];

    g_snprintf (mappath, sizeof (mappath), "%s%s",  local_config_mapdir, filename);
    g_snprintf (mappath, sizeof (mappath), "data/maps/%s", filename);
    g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/maps/%s", DATADIR, filename);
    g_snprintf (filename, sizeof (filename), "./data/pixmaps/%s", filename);

    g_strlcpy (mappath, local_config_homedir, sizeof (mappath));
    if ( stat (mappath, &buf) ) {
	printf("Success\n");
	//if (buf.st_mtime != waytxtstamp)
    }
    return TRUE;
}

/* ******************************************************************
 * This is an internally used function to create pixmaps. 
 */
GtkWidget *
create_pixmap (GtkWidget * widget, const gchar * filename)
{
  gchar pathname[200];
  GtkWidget *pixmap = NULL;

  if (!filename || !filename[0])
    return gtk_image_new ();

  g_snprintf (pathname, sizeof (pathname), "%s/gpsdrive/%s", DATADIR,
	      filename);


  pixmap = gtk_image_new_from_file (pathname);
  if (!pixmap)
    {
      if (mydebug > 5)
	fprintf (stderr, _("Couldn't find pixmap file: %s"), pathname);
      else
	g_warning (_("Couldn't find pixmap file: %s"), pathname);
      return gtk_image_new ();
    }

  return pixmap;
}
