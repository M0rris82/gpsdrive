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
 * This file contains the refactored drawing functions from gpsdrive.c
 */

#include "map_priv.h"

void
draw_zoom_scale2 (MapGC *mgc, int width, int height,
		  MapSettings *settings,
		  GDALDatasetH dataset, char* path,
		  double zoom, double pixelsize)
{
  double milesconv = 1.0;

  int pixels;
  int m;
  char txt[100];
  pixels = 141 / milesconv;
  //m = mapscale / (20 * zoom);

  //m = scale2zoom(dataset, path, 1, pixelsize);
  double xmpp, ympp, mpp;
  metersperpixel(dataset, path, 0, &xmpp, &ympp);
  m = pixels * (xmpp + ympp) / 2 / zoom;

  if (m < 1000)
    {
      if (!settings->nauticflag)
	snprintf (txt, sizeof (txt), "%d%s", m,
		  (settings->milesflag) ? "yrds" : "m");
      else
	snprintf (txt, sizeof (txt), "%.3f%s", m / 1000.0,
		  (settings->milesflag) ? "mi" : ((settings->metricflag) ? "km"
						    : "nmi"));

      if (!settings->metricflag)
	pixels = pixels * milesconv * 0.9144;
    }
  else
    snprintf (txt, sizeof (txt), "%.1f%s", m / 1000.0,
	      (settings->milesflag) ? "mi" : ((settings->metricflag) ? "km" :
						"nmi"));

  //gdk_gc_set_foreground (mgc->gtk_gc, &settings->textback);
  map_bkg_rectangle(mgc, settings,
		    (width - 20) - pixels - 5, height - 35,
		    pixels + 10, 30);

  //gdk_gc_set_foreground (mgc->gtk_gc, &settings->black);

  map_text(mgc, settings, width - 25, height - 33, txt, 0x100);

  //gdk_gc_set_line_attributes (mgc->gtk_gc, 2, 0, 0, 0);
  map_line(mgc, settings,
	   (width - 20) - pixels, height - 20 + 5,
	   (width - 20), height - 20 + 5, 0);
  map_line(mgc, settings,
	   (width - 20) - pixels, height - 20,
	   (width - 20) - pixels, height - 20 + 10, 0);
  map_line(mgc, settings,
	   (width - 20), height - 20,
	   (width - 20), height - 20 + 10, 0);

#ifdef USETELEATLAS
  /* display the streetname */
  ta_displaystreetname (actualstreetname);
#endif

  /*  draw zoom factor */
  snprintf (txt, sizeof (txt), "%lfx", zoom);

  //gdk_gc_set_foreground (mgc->gtk_gc, &settings->mygray);
  //map_bkg_rectangle(&mgc, settings, (width - 30), 0, 30, 30);

  map_text(mgc, settings, width - 2, 2, txt, 0xd01);

#ifdef MAP_DEBUG
  /*  draw file name */
  {
    char *filename = strrchr(path, '/');

    if (filename)
      filename++;
    else
      filename = path;

    map_text(mgc, settings, 2, 2, filename, 0xc01);
  }
#endif
}

/* *****************************************************************************
 * draw the position on the map 
 * And many more 
 * TODO: sort out
 */
void
drawposition(MapGC *mgc,
	     MapSettings *settings,
	     int posx, int posy,
	     double angle_to_destination, double direction)
{
#define PFSIZE 55
#define PFSIZE2 45

  if (settings->havepos || settings->blink)
    {
      if (settings->posmode)
	{
	  map_pos_rectangle(mgc, settings,
			    posx - 10, posy - 10, 20, 20);
	}
      else
	{
#ifdef HAVE_GTK
	  GdkPoint poly[16];
	  double w;

	  if (settings->shadow)
	    {
	      /*  draw shadow of  position marker */
	      gdk_gc_set_foreground (mgc->gtk_gc, &settings->darkgrey);
	      gdk_gc_set_line_attributes (mgc->gtk_gc, 3, 0, 0,
					  0);
	      gdk_gc_set_function (mgc->gtk_gc, GDK_AND);
	      gdk_draw_arc (mgc->gtk_drawable, mgc->gtk_gc, 0,
			    posx - 7 + SHADOWOFFSET,
			    posy - 7 + SHADOWOFFSET, 14, 14,
			    0, 360 * 64);
	      /*  draw pointer to destination */
	      gdk_gc_set_line_attributes (mgc->gtk_gc, 4, 0, 0, 0);
	      /* gdk_draw_line (drawable, mgc->gtk_gc, */
	      /*       posx + 4 * sin (angle_to_destination) + */
	      /*       SHADOWOFFSET, */
	      /*       posy - 4 * cos (angle_to_destination) + */
	      /*       SHADOWOFFSET, */
	      /*       posx + 20 * sin (angle_to_destination) + */
	      /*       SHADOWOFFSET, */
	      /*       posy - 20 * cos (angle_to_destination) + */
	      /*       SHADOWOFFSET); */
	      w = angle_to_destination + M_PI;
	      poly[0].x =
		posx + SHADOWOFFSET +
		(PFSIZE) / 2.3 * (cos (w + M_PI_2));
	      poly[0].y =
		posy + SHADOWOFFSET +
		(PFSIZE) / 2.3 * (sin (w + M_PI_2));
	      poly[1].x =
		posx + SHADOWOFFSET +
		(PFSIZE) / 9 * (cos (w + M_PI));
	      poly[1].y =
		posy + SHADOWOFFSET +
		(PFSIZE) / 9 * (sin (w + M_PI));
	      poly[2].x =
		posx + SHADOWOFFSET +
		PFSIZE / 10 * (cos (w + M_PI_2));
	      poly[2].y =
		posy + SHADOWOFFSET +
		PFSIZE / 10 * (sin (w + M_PI_2));
	      poly[3].x =
		posx + SHADOWOFFSET -
		(PFSIZE) / 9 * (cos (w + M_PI));
	      poly[3].y =
		posy + SHADOWOFFSET -
		(PFSIZE) / 9 * (sin (w + M_PI));
	      poly[4].x = poly[0].x;
	      poly[4].y = poly[0].y;
	      gdk_draw_polygon (mgc->gtk_drawable, mgc->gtk_gc, 1,
				(GdkPoint *) poly, 5);

	      /*  draw pointer to direction */
	      /*  gdk_draw_line (drawable, mgc->gtk_gc, */
	      /*       posx + 4 * sin (direction) + SHADOWOFFSET, */
	      /*       posy - 4 * cos (direction) + SHADOWOFFSET, */
	      /*       posx + 20 * sin (direction) + SHADOWOFFSET, */
	      /*       posy - 20 * cos (direction) + SHADOWOFFSET); */
	      gdk_gc_set_line_attributes (mgc->gtk_gc, 2, 0, 0,
					  0);
	      gdk_draw_arc (mgc->gtk_drawable, mgc->gtk_gc, 0,
			    posx + 2 - 7 + SHADOWOFFSET,
			    posy + 2 - 7 + SHADOWOFFSET, 10,
			    10, 0, 360 * 64);

	      w = direction + M_PI;
	      poly[0].x =
		posx + SHADOWOFFSET +
		(PFSIZE2) / 2.3 * (cos (w + M_PI_2));
	      poly[0].y =
		posy + SHADOWOFFSET +
		(PFSIZE2) / 2.3 * (sin (w + M_PI_2));
	      poly[1].x =
		posx + SHADOWOFFSET +
		(PFSIZE2) / 9 * (cos (w + M_PI));
	      poly[1].y =
		posy + SHADOWOFFSET +
		(PFSIZE2) / 9 * (sin (w + M_PI));
	      poly[2].x =
		posx + SHADOWOFFSET +
		PFSIZE2 / 10 * (cos (w + M_PI_2));
	      poly[2].y =
		posy + SHADOWOFFSET +
		PFSIZE2 / 10 * (sin (w + M_PI_2));
	      poly[3].x =
		posx + SHADOWOFFSET -
		(PFSIZE2) / 9 * (cos (w + M_PI));
	      poly[3].y =
		posy + SHADOWOFFSET -
		(PFSIZE2) / 9 * (sin (w + M_PI));
	      poly[4].x = poly[0].x;
	      poly[4].y = poly[0].y;
	      gdk_draw_polygon (mgc->gtk_drawable, mgc->gtk_gc, 0,
				(GdkPoint *) poly, 5);
	      gdk_gc_set_function (mgc->gtk_gc, GDK_COPY);
	    }
	  /*  draw real position marker */

	  gdk_gc_set_foreground (mgc->gtk_gc, &settings->black);
	  gdk_gc_set_line_attributes (mgc->gtk_gc, 3, 0, 0, 0);
	  gdk_draw_arc (mgc->gtk_drawable, mgc->gtk_gc, 0, posx - 7,
			posy - 7, 14, 14, 0, 360 * 64);
	  /*  draw pointer to destination */

	  w = angle_to_destination + M_PI;

	  poly[0].x =
	    posx + (PFSIZE) / 2.3 * (cos (w + M_PI_2));
	  poly[0].y =
	    posy + (PFSIZE) / 2.3 * (sin (w + M_PI_2));
	  poly[1].x = posx + (PFSIZE) / 9 * (cos (w + M_PI));
	  poly[1].y = posy + (PFSIZE) / 9 * (sin (w + M_PI));
	  poly[2].x = posx + PFSIZE / 10 * (cos (w + M_PI_2));
	  poly[2].y = posy + PFSIZE / 10 * (sin (w + M_PI_2));
	  poly[3].x = posx - (PFSIZE) / 9 * (cos (w + M_PI));
	  poly[3].y = posy - (PFSIZE) / 9 * (sin (w + M_PI));
	  poly[4].x = poly[0].x;
	  poly[4].y = poly[0].y;
	  gdk_draw_polygon (mgc->gtk_drawable, mgc->gtk_gc, 1,
			    (GdkPoint *) poly, 5);

	  /*  draw pointer to direction */
	  gdk_gc_set_foreground (mgc->gtk_gc, &settings->red);
	  gdk_draw_arc (mgc->gtk_drawable, mgc->gtk_gc, 0, posx + 2 - 7,
			posy + 2 - 7, 10, 10, 0, 360 * 64);
	  w = direction + M_PI;
	  poly[0].x =
	    posx + (PFSIZE2) / 2.3 * (cos (w + M_PI_2));
	  poly[0].y =
	    posy + (PFSIZE2) / 2.3 * (sin (w + M_PI_2));
	  poly[1].x = posx + (PFSIZE2) / 9 * (cos (w + M_PI));
	  poly[1].y = posy + (PFSIZE2) / 9 * (sin (w + M_PI));
	  poly[2].x = posx + PFSIZE2 / 10 * (cos (w + M_PI_2));
	  poly[2].y = posy + PFSIZE2 / 10 * (sin (w + M_PI_2));
	  poly[3].x = posx - (PFSIZE2) / 9 * (cos (w + M_PI));
	  poly[3].y = posy - (PFSIZE2) / 9 * (sin (w + M_PI));
	  poly[4].x = poly[0].x;
	  poly[4].y = poly[0].y;
	  gdk_draw_polygon (mgc->gtk_drawable, mgc->gtk_gc, 0,
			    (GdkPoint *) poly, 5);
#endif
	}
      if (settings->markwaypoint)
	{
	  /*calcxy (&posxmarker, &posymarker, wplon, wplat, zoom);

	  gdk_gc_set_foreground (mgc->gtk_gc, &green);
	  gdk_gc_set_line_attributes (mgc->gtk_gc, 5, 0, 0, 0);
	  gdk_draw_arc (drawable, mgc->gtk_gc, 0, posxmarker - 10,
	  posymarker - 10, 20, 20, 0, 360 * 64);*/
	}
      /*  If we are in position mode we set direction to zero to see where is the  */
      /*  target  */
      if (settings->posmode)
	direction = 0;

      /*bearing = angle_to_destination - direction;
      if (bearing < 0)
	bearing += 2 * M_PI;
      if (bearing > (2 * M_PI))
	bearing -= 2 * M_PI;
	display_status2 ();*/
    }
}

/* *****************************************************************************
 */
/*
 * Draw a grid over the map
 */
void
draw_grid2 (MapGC *mgc, int width, int height,
	    MapSettings *settings,
	    MapState *state, double pixelsize)
{
	int count;
	double step;
	double lat, lon;
	double lat_ul, lon_ul;
	double lat_ll, lon_ll;
	double lat_ur, lon_ur;
	double lat_lr, lon_lr;

	double lat_min, lon_min;
	double lat_max, lon_max;

	double mapscale = 10000;

	// calculate the start and stop for lat/lon according to the displayed section
	screen2wgs(state, 0, 0, &lat_ul, &lon_ul);
	screen2wgs(state, 0, height, &lat_ll, &lon_ll);
	screen2wgs(state, width, 0, &lat_ur, &lon_ur);
	screen2wgs(state, width, height, &lat_lr, &lon_lr);
	//calcxytopos2 (0, 0, &lat_ul, &lon_ul, zoom);
	//calcxytopos2 (0, height, &lat_ll, &lon_ll, zoom);
	//calcxytopos2 (width, 0, &lat_ur, &lon_ur, zoom);
	//calcxytopos2 (width, height, &lat_lr, &lon_lr, zoom);

	lat_min = MIN (lat_ll, lat_ul);
	lat_max = MAX (lat_lr, lat_ur);
	lon_min = MIN (lon_ll, lon_ul);
	lon_max = MAX (lon_lr, lon_ur);

	// Calculate distance between grid lines
	{
	  double log10Step = floor(log10(lon_max - lon_min));
	  step = pow(10., log10Step);
	  double lines = (lon_max - lon_min) / step;

	  fprintf(stderr, "lines: %lf step: %lf\n", lines, step);

	  if (lines <= 1.5)
	    {
	      step *= 0.2;
	    }
	  else if (lines <= 3.5)
	    {
	      step *= 0.5;
	    }

	  fprintf(stderr, "step2: %lf\n", step);
	}

	lat_min = floor(lat_min / step) * step;
	lon_min = floor(lon_min / step) * step;

#ifdef MAP_DEBUG
		printf ("Draw Grid: (%.2f,%.2f) - (%.2f,%.2f)\n", lat_min,
			lon_min, lat_max, lon_max);
#endif

	// Set Drawing Colors
	//gdk_gc_set_function (mgc->gtk_gc, GDK_AND);


	// Loop over desired lat/lon  
	count = 0;
	for (lon = lon_min; lon <= lon_max; lon = lon + step)
	    {
		for (lat = lat_min; lat <= lat_max; lat = lat + step)
		    {
		      //gdouble posxdest11, posydest11;
		      //gdouble posxdest12, posydest12;
		      //gdouble posxdest21, posydest21;
		      //gdouble posxdest22, posydest22;
		      //gdouble posxdist, posydist;
			GInt32 posxdest11, posydest11;
			GInt32 posxdest12, posydest12;
			GInt32 posxdest21, posydest21;
			GInt32 posxdest22, posydest22;
			GInt32 posxdist, posydist;
			char str[200];

			count++;

			wgs2screen(state, lat, lon, &posxdest11, &posydest11);
			wgs2screen(state, lat + step, lon, &posxdest12, &posydest12);
			wgs2screen(state, lat, lon + step, &posxdest21, &posydest21);
			wgs2screen(state, lat + step, lon + step, &posxdest22, &posydest22);
			//calcxy2 (&posxdest11, &posydest11, lon, lat, zoom);
			//calcxy2 (&posxdest12, &posydest12, lon, lat + step, zoom);
			//calcxy2 (&posxdest21, &posydest21, lon + step, lat, zoom);
			//calcxy2 (&posxdest22, &posydest22, lon + step, lat + step, zoom);

			if (((posxdest11 >= 0) && (posxdest11 < width) &&
			     (posydest11 >= 0) && (posydest11 < height))
			    ||
			    ((posxdest22 >= 0) && (posxdest22 < width) &&
			     (posydest22 >= 0) && (posydest22 < height))
			    ||
			    ((posxdest21 >= 0) && (posxdest21 < width) &&
			     (posydest21 >= 0) && (posydest21 < height))
			    ||
			    ((posxdest12 >= 0) && (posxdest12 < width) &&
			     (posydest12 >= 0) && (posydest12 < height)))
			    {
				// TODO: add linethikness 2 for Mayor Lines
				map_line(mgc, settings,
					 posxdest11, posydest11,
					 posxdest21, posydest21, 1);
				map_line(mgc, settings,
					 posxdest11, posydest11,
					 posxdest12, posydest12, 1);
 
				// Text lon
				if (step >= 1)
				    snprintf (str, sizeof (str), "%.0f",
					      lon);
				else if (step >= .1)
				    snprintf (str, sizeof (str), "%.1f",
					      lon);
				else if (step >= .01)
				    snprintf (str, sizeof (str), "%.2f",
					      lon);
				else if (step >= .001)
				    snprintf (str, sizeof (str), "%.3f",
					      lon);
				else
				    snprintf (str, sizeof (str), "%.4f",
					      lon);
				posxdist = (posxdest12 - posxdest11) / 4;
				posydist = (posydest12 - posydest11) / 4;
				map_text (mgc, settings,
					  posxdest11 + posxdist,
					  posydest11 + posydist,
					  str, 0x300);

				// Text lat
				if (step >= 1)
				    snprintf (str, sizeof (str), "%.0f",
					      lat);
				else if (step >= .1)
				    snprintf (str, sizeof (str), "%.1f",
					      lat);
				else if (step >= .01)
				    snprintf (str, sizeof (str), "%.2f",
					      lat);
				else if (step >= .001)
				    snprintf (str, sizeof (str), "%.3f",
					      lat);
				else
				    snprintf (str, sizeof (str), "%.4f",
					      lat);
				posxdist = (posxdest21 - posxdest11) / 4;
				posydist = (posydest21 - posydest11) / 4;
				map_text (mgc, settings,
					  posxdest11 + posxdist,
					  posydest11 + posydist,
					  str, 0x300);
			    }
		    }
	    }
#ifdef MAP_DEBUG
	    printf ("draw_grid loops: %d\n", count);
#endif
}

#ifdef yetToBeRefactored

/* *****************************************************************************
 * draw wlan Waypoints
 */
void
drawwlan (gint posxdest, gint posydest, gint wlan)
{
	/*  wlan=0: no wlan, 1:open wlan, 2:WEP crypted wlan */

	if (wlan == 0)
		return;

	if ((posxdest >= 0) && (posxdest < SCREEN_X))
	{
		if ((posydest >= 0) && (posydest < SCREEN_Y))
		{
			if (wlan == 1)
				gdk_draw_pixbuf (drawable, mgc->gtk_gc,
						 openwlanpixbuf, 0, 0,
						 posxdest - 14, posydest - 12,
						 24, 24, GDK_RGB_DITHER_NONE,
						 0, 0);
			else
				gdk_draw_pixbuf (drawable, mgc->gtk_gc,
						 closedwlanpixbuf, 0, 0,
						 posxdest - 14, posydest - 12,
						 24, 24, GDK_RGB_DITHER_NONE,
						 0, 0);
		}
	}
}

/* *****************************************************************************
 */
void
drawfriends (void)
{
	gint i;
	gdouble posxdest, posydest, clong, clat, direction;
	gint width, height;
	gdouble w;
	GdkPoint poly[16];
	struct tm *t;
	time_t ti, tif;
#define PFSIZE 55

	actualfriends = 0;
	/*   g_print("Maxfriends: %d\n",maxfriends); */
	for (i = 0; i < maxfriends; i++)
	{

		/* return if too old  */
		ti = time (NULL);
		tif = atol ((friends + i)->timesec);
		if (!(tif > 1000000000))
			fprintf (stderr,
				 "Format error! timesec: %s, Name: %s, i: %d\n",
				 (friends + i)->timesec, (friends + i)->name,
				 i);
		if ((ti - maxfriendssecs) > tif)
			continue;
		actualfriends++;
		clong = g_strtod ((friends + i)->longi, NULL);
		clat = g_strtod ((friends + i)->lat, NULL);


		calcxy (&posxdest, &posydest, clong, clat, zoom);

		/* If Friend is visible inside SCREEN display him/her */
		if ((posxdest >= 0) && (posxdest < SCREEN_X))
		{

			if ((posydest >= 0) && (posydest < SCREEN_Y))
			{

				gdk_draw_pixbuf (drawable, mgc->gtk_gc,
						 friendspixbuf, 0, 0,
						 posxdest - 18, posydest - 12,
						 39, 24, GDK_RGB_DITHER_NONE,
						 0, 0);
				gdk_gc_set_line_attributes (mgc->gtk_gc, 4, 0, 0,
							    0);

				/*  draw pointer to direction */
				direction =
					strtod ((friends + i)->heading,
						NULL) * M_PI / 180.0;
				w = direction + M_PI;
				gdk_gc_set_line_attributes (mgc->gtk_gc, 2, 0, 0,
							    0);
				poly[0].x =
					posxdest +
					(PFSIZE) / 2.3 * (cos (w + M_PI_2));
				poly[0].y =
					posydest +
					(PFSIZE) / 2.3 * (sin (w + M_PI_2));
				poly[1].x =
					posxdest +
					(PFSIZE) / 9 * (cos (w + M_PI));
				poly[1].y =
					posydest +
					(PFSIZE) / 9 * (sin (w + M_PI));
				poly[2].x =
					posxdest +
					PFSIZE / 10 * (cos (w + M_PI_2));
				poly[2].y =
					posydest +
					PFSIZE / 10 * (sin (w + M_PI_2));
				poly[3].x =
					posxdest -
					(PFSIZE) / 9 * (cos (w + M_PI));
				poly[3].y =
					posydest -
					(PFSIZE) / 9 * (sin (w + M_PI));
				poly[4].x = poly[0].x;
				poly[4].y = poly[0].y;
				gdk_gc_set_foreground (mgc->gtk_gc, &blue);
				gdk_draw_polygon (drawable, mgc->gtk_gc, 0,
						  (GdkPoint *) poly, 5);
				gdk_draw_arc (drawable, mgc->gtk_gc, 0,
					      posxdest + 2 - 7,
					      posydest + 2 - 7, 10, 10, 0,
					      360 * 64);

				/*   draw + sign at destination   */
				gdk_gc_set_foreground (mgc->gtk_gc, &red);
				gdk_draw_line (drawable, mgc->gtk_gc,
					       posxdest + 1, posydest + 1 - 5,
					       posxdest + 1,
					       posydest + 1 + 5);
				gdk_draw_line (drawable, mgc->gtk_gc,
					       posxdest + 1 + 5, posydest + 1,
					       posxdest + 1 - 5,
					       posydest + 1);

				{	/* print friends name / speed on map */
					PangoFontDescription *pfd;
					PangoLayout *wplabellayout;
					gchar txt[200], txt2[100], s1[10];
					time_t sec;
					char *as, day[20], dispname[40];
					int speed, ii;

					sec = atol ((friends + i)->timesec);
					sec += 3600 * zone;
					t = gmtime (&sec);

					as = asctime (t);
					sscanf (as, "%s", day);
					sscanf ((friends + i)->speed, "%d",
						&speed);

					/* replace _ with  spaces in name */
					g_strlcpy (dispname,
						   (friends + i)->name,
						   sizeof (dispname));
					for (ii = 0;
					     (size_t) ii < strlen (dispname);
					     ii++)
						if (dispname[ii] == '_')
							dispname[ii] = ' ';

					g_snprintf (txt, sizeof (txt),
						    "%s,%d", dispname,
						    (int) (speed *
							   milesconv));
					if (milesflag)
						g_snprintf (s1, sizeof (s1),
							    "%s", _("mi/h"));
					else if (nauticflag)
						g_snprintf (s1, sizeof (s1),
							    "%s", _("knots"));
					else
						g_snprintf (s1, sizeof (s1),
							    "%s", _("km/h"));
					g_strlcat (txt, s1, sizeof (txt));
					g_snprintf (txt2, sizeof (txt2),
						    "%s, %2d:%02d\n", day,
						    t->tm_hour, t->tm_min);
					g_strlcat (txt, txt2, sizeof (txt));
					wplabellayout =
						gtk_widget_create_pango_layout
						(drawing_area, txt);
					if (pdamode)
						pfd = pango_font_description_from_string ("Sans 8");
					else
						pfd = pango_font_description_from_string ("Sans bold 11");
					pango_layout_set_font_description
						(wplabellayout, pfd);
					pango_layout_get_pixel_size
						(wplabellayout, &width,
						 &height);
					gdk_gc_set_foreground (mgc->gtk_gc,
							       &textbacknew);
					/*              gdk_draw_rectangle (drawable, mgc->gtk_gc, 1, posxdest + 18,
					 *                                  posydest - height/2 , width + 2,
					 *                                  height + 2);
					 */

					gdk_draw_layout_with_colors (drawable,
								     mgc->gtk_gc,
								     posxdest
								     + 21,
								     posydest
								     -
								     height /
								     2 + 1,
								     wplabellayout,
								     &black,
								     NULL);
					gdk_draw_layout_with_colors (drawable,
								     mgc->gtk_gc,
								     posxdest
								     + 20,
								     posydest
								     -
								     height /
								     2,
								     wplabellayout,
								     &orange,
								     NULL);

					if (wplabellayout != NULL)
						g_object_unref (G_OBJECT
								(wplabellayout));
					/* freeing PangoFontDescription, cause it has been copied by prev. call */
					pango_font_description_free (pfd);

				}


			}
		}
	}
}

/* *****************************************************************************
 * Draw waypoints on map
 */
// TODO: Put this in its own file
void
draw_waypoints ()
{
	gdouble posxdest, posydest;
	gint k, k2, i, shownwp = 0;
	gchar txt[200];

	if (debug)
		printf ("draw_waypoints()\n");

	/*  draw waypoints */
	for (i = 0; i < maxwp; i++)
	{
		calcxy (&posxdest, &posydest,
			(wayp + i)->lon, (wayp + i)->lat, zoom);

		if ((posxdest >= 0) && (posxdest < SCREEN_X)
		    && (shownwp < MAXSHOWNWP)
		    && (posydest >= 0) && (posydest < SCREEN_Y))
		{
			gdk_gc_set_line_attributes (mgc->gtk_gc, 2, 0, 0, 0);
			shownwp++;
			g_strlcpy (txt, (wayp + i)->name, sizeof (txt));

			// Draw Icon(typ) or + Sign
			if ((wayp + i)->wlan > 0)
				drawwlan (posxdest, posydest,
					  (wayp + i)->wlan);
			else
				drawicon (posxdest, posydest,
					  (wayp + i)->typ);

			// Draw Proximity Circle
			if ((wayp + i)->proximity > 0.0)
			{
				gint proximity_pixels;
				if (mapscale)
					proximity_pixels =
						((wayp + i)->proximity)
						* zoom * PIXELFACT / mapscale;
				else
					proximity_pixels = 2;

				gdk_gc_set_foreground (mgc->gtk_gc, &blue);

				gdk_draw_arc (drawable, mgc->gtk_gc, FALSE,
					      posxdest - proximity_pixels,
					      posydest - proximity_pixels,
					      proximity_pixels * 2,
					      proximity_pixels * 2, 0,
					      64 * 360);
			}


			{	/* draw shadow of text */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;
				gint width, height;
				gchar *tn;

				gdk_gc_set_foreground (mgc->gtk_gc, &darkgrey);
				gdk_gc_set_function (mgc->gtk_gc, GDK_AND);
				tn = g_strdelimit (txt, "_", ' ');

				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_area, tn);
				pfd = pango_font_description_from_string
					(wplabelfont);
				pango_layout_set_font_description
					(wplabellayout, pfd);
				pango_layout_get_pixel_size (wplabellayout,
							     &width, &height);
				/* printf("j: %d\n",height);    */
				k = width + 4;
				k2 = height;
				if (shadow)
				{
					gdk_draw_layout_with_colors (drawable,
								     mgc->gtk_gc,
								     posxdest
								     + 15 +
								     SHADOWOFFSET,
								     posydest
								     -
								     k2 / 2 +
								     SHADOWOFFSET,
								     wplabellayout,
								     &darkgrey,
								     NULL);
				}
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}
			gdk_gc_set_function (mgc->gtk_gc, GDK_COPY);


			gdk_gc_set_function (mgc->gtk_gc, GDK_AND);

			gdk_gc_set_foreground (mgc->gtk_gc, &textbacknew);
			gdk_draw_rectangle (drawable, mgc->gtk_gc, 1,
					    posxdest + 13, posydest - k2 / 2,
					    k + 1, k2);
			gdk_gc_set_function (mgc->gtk_gc, GDK_COPY);
			gdk_gc_set_foreground (mgc->gtk_gc, &black);
			gdk_gc_set_line_attributes (mgc->gtk_gc, 1, 0, 0, 0);
			gdk_draw_rectangle (drawable, mgc->gtk_gc, 0,
					    posxdest + 12,
					    posydest - k2 / 2 - 1, k + 2, k2);

			/*            gdk_gc_set_foreground (mgc->gtk_gc, &yellow);  */
			{
				/* prints in pango */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;

				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_area, txt);
				pfd = pango_font_description_from_string
					(wplabelfont);
				pango_layout_set_font_description
					(wplabellayout, pfd);

				gdk_draw_layout_with_colors (drawable,
							     mgc->gtk_gc,
							     posxdest + 15,
							     posydest -
							     k2 / 2,
							     wplabellayout,
							     &white, NULL);
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}

			/*              gdk_draw_text (drawable, smalltextfont, mgc->gtk_gc,
			 *                           posxdest + 13, posydest + 6, txt,
			 *                           strlen (txt));
			 */
		}
	}
}

#endif

/* *****************************************************************************
 * draw the markers on the map 
 * And many more 
 * TODO: sort out
 */
int
drawmarkers (MapGC *mgc, int width, int height,
	     MapSettings *settings, MapState *state, double pixelsize,
	     double angle_to_destination, double direction)
// guint * datum)
{
  double posxdest, posydest, posxmarker, posymarker;
  char s2[100], s3[200], s2a[20];
  int k;
  int posx;
  int posy;

  wgs2screen(state, state->req_lat, state->req_lon, &posx, &posy);

  settings->gblink = !settings->gblink;
  /*    g_print("simmode: %d, nmea %d garmin %d\n",simmode,haveNMEA,haveGARMIN); */

  //drawtracks ();

  if (settings->drawgrid)
    draw_grid2 (mgc, width, height,
		settings, state, pixelsize);

#ifdef yetToBeRefactored

  if (usesql)
    {
      poi_draw_list ();
      streets_draw_list ();
      tracks_draw_list ();
    }

  if (wpflag)
    draw_waypoints ();

  if (havefriends)
    drawfriends ();

  if (havekismet)
    readkismet ();
#endif

  /*  draw scale */
  if (settings->zoomscale)
    draw_zoom_scale2 (mgc, width, height, settings,
		      state->dataset[0], state->path[0],
		      state->act_yZoom[0], pixelsize);

#ifdef yetToBeRefactored
  if (havekismet)
    gdk_draw_pixbuf (drawable, mgc->gtk_gc, kismetpixbuf, 0, 0,
		     10, SCREEN_Y - 42,
		     36, 20, GDK_RGB_DITHER_NONE, 0, 0);

  if (savetrack)
    {
      /*  k = gdk_text_width (smalltextfont, savetrackfn, strlen (savetrackfn));
       */
      k = 100;
      gdk_gc_set_foreground (mgc->gtk_gc, &white);
      gdk_draw_rectangle (drawable, mgc->gtk_gc, 1, 10,
			  SCREEN_Y - 21, k + 3, 14);
      gdk_gc_set_foreground (mgc->gtk_gc, &red);
      {
	/* prints in pango */
	PangoFontDescription *pfd;
	PangoLayout *wplabellayout;

	wplabellayout =
	  gtk_widget_create_pango_layout (drawing_area,
					  savetrackfn);
	//KCFX  
	if (pdamode)
	  pfd = pango_font_description_from_string
	    ("Sans 7");
	else
	  pfd = pango_font_description_from_string
	    ("Sans 10");
	pango_layout_set_font_description (wplabellayout,
					   pfd);

	gdk_draw_layout_with_colors (drawable, mgc->gtk_gc,
				     14, SCREEN_Y - 22,
				     wplabellayout, &red,
				     NULL);
	if (wplabellayout != NULL)
	  g_object_unref (G_OBJECT (wplabellayout));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);
      }


      /*    gdk_draw_text (drawable, smalltextfont, mgc->gtk_gc,
       *                   11, SCREEN_Y - 10, savetrackfn,
       *                   strlen (savetrackfn));
       */

      /*      gdk_draw_text (drawable, textfont, mgc->gtk_gc, 10, */
      /*                     SCREEN_Y - 10, savetrackfn, strlen (savetrackfn)); */
    }

  if (settings->posmode)
    {
      blink = TRUE;
    }
#endif

  drawposition(mgc, settings, posx, posy, angle_to_destination, direction);
// guint * datum)

#ifdef yetToBeRefactored
	/*  now draw marker for destination point */

	calcxy (&posxdest, &posydest, target_long, target_lat, zoom);

	gdk_gc_set_line_attributes (mgc->gtk_gc, 4, 0, 0, 0);
	if (shadow)
	{
		/*  draw + sign at destination */
		gdk_gc_set_foreground (mgc->gtk_gc, &darkgrey);
		gdk_gc_set_function (mgc->gtk_gc, GDK_AND);
		gdk_draw_line (drawable, mgc->gtk_gc, posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 - 10 + SHADOWOFFSET,
			       posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 - 2 + SHADOWOFFSET);
		gdk_draw_line (drawable, mgc->gtk_gc, posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 + 2 + SHADOWOFFSET,
			       posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 + 10 + SHADOWOFFSET);
		gdk_draw_line (drawable, mgc->gtk_gc,
			       posxdest + 1 + 10 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET,
			       posxdest + 1 + 2 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET);
		gdk_draw_line (drawable, mgc->gtk_gc,
			       posxdest + 1 - 2 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET,
			       posxdest + 1 - 10 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET);
		gdk_gc_set_function (mgc->gtk_gc, GDK_COPY);
	}

	if (crosstoogle)
		gdk_gc_set_foreground (mgc->gtk_gc, &blue);
	else
		gdk_gc_set_foreground (mgc->gtk_gc, &red);
	crosstoogle = !crosstoogle;
	/*  draw + sign at destination */
	gdk_draw_line (drawable, mgc->gtk_gc, posxdest + 1,
		       posydest + 1 - 10, posxdest + 1, posydest + 1 - 2);
	gdk_draw_line (drawable, mgc->gtk_gc, posxdest + 1,
		       posydest + 1 + 2, posxdest + 1, posydest + 1 + 10);
	gdk_draw_line (drawable, mgc->gtk_gc, posxdest + 1 + 10,
		       posydest + 1, posxdest + 1 + 2, posydest + 1);
	gdk_draw_line (drawable, mgc->gtk_gc, posxdest + 1 - 2,
		       posydest + 1, posxdest + 1 - 10, posydest + 1);


	/* display messages on map */
	display_dsc ();
	/*  if distance is less then 1 km show meters */
	if (milesflag)
	{
		if (dist <= 1.0)
		{
			g_snprintf (s2, sizeof (s2), "%.0f", dist * 1760.0);
			g_strlcpy (s2a, "yrds", sizeof (s2a));
		}
		else
		{
			if (dist <= 10.0)
			{
				g_snprintf (s2, sizeof (s2), "%.2f", dist);
				g_strlcpy (s2a, "mi", sizeof (s2a));
			}
			else
			{
				g_snprintf (s2, sizeof (s2), "%.1f", dist);
				g_strlcpy (s2a, "mi", sizeof (s2a));
			}
		}
	}
	if (metricflag)
	{
		if (dist <= 1.0)
		{
			g_snprintf (s2, sizeof (s2), "%.0f", dist * 1000.0);
			g_strlcpy (s2a, "m", sizeof (s2a));
		}
		else
		{
			if (dist <= 10.0)
			{
				g_snprintf (s2, sizeof (s2), "%.2f", dist);
				g_strlcpy (s2a, "km", sizeof (s2a));
			}
			else
			{
				g_snprintf (s2, sizeof (s2), "%.1f", dist);
				g_strlcpy (s2a, "km", sizeof (s2a));
			}
		}
	}
	if (nauticflag)
	{
		if (dist <= 1.0)
		{
			g_snprintf (s2, sizeof (s2), "%.3f", dist);
			g_strlcpy (s2a, "nmi", sizeof (s2a));
		}
		else
		{
			if (dist <= 10.0)
			{
				g_snprintf (s2, sizeof (s2), "%.2f", dist);
				g_strlcpy (s2a, "nmi", sizeof (s2a));
			}
			else
			{
				g_snprintf (s2, sizeof (s2), "%.1f", dist);
				g_strlcpy (s2a, "nmi", sizeof (s2a));
			}
		}
	}
	/*    display distance, speed and zoom */
	/*   g_snprintf (s3, */
	/*     "<span color=\"%s\" font_desc=\"%s\">%s</span><span color=\"%s\" font_desc=\"%s\">%s</span>", */
	/*     bluecolor, bigfont, s2, bluecolor, "sans bold 18", s2a); */
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"%s\" font_desc=\"%s\">%s<span size=\"16000\">%s</span></span>",
		    bluecolor, bigfont, s2, s2a);
	gtk_label_set_markup (GTK_LABEL (distlabel), s3);
	/* gtk_label_set_text (GTK_LABEL (distlabel), s2);  */
	if (milesflag)
		g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
	if (metricflag)
		g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
	if (nauticflag)
		g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"%s\" font_desc=\"%s\">%s</span>",
		    bluecolor, bigfont, s2);
	gtk_label_set_markup (GTK_LABEL (speedlabel), s3);

	/* gtk_label_set_text (GTK_LABEL (speedlabel), s2); */

	if (havealtitude)
	{
		if (milesflag || nauticflag)
		{
			g_snprintf (s2, sizeof (s2), "%.0f",
				    altitude * 3.2808399 + normalnull);
			g_strlcpy (s2a, "ft", sizeof (s2a));
		}
		else
		{
			g_snprintf (s2, sizeof (s2), "%.0f",
				    altitude + normalnull);
			g_strlcpy (s2a, "m", sizeof (s2a));
		}
		gtk_label_set_text (GTK_LABEL (altilabel), s2);

		if (pdamode)
		{
			if (normalnull == 0.0)

				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>",
					    bluecolor, s2, bluecolor, s2a);
			else
				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>"
					    "<span color=\"red\" font_family=\"Arial\" size=\"5000\">\nNN %+.1f</span>",
					    bluecolor, s2, bluecolor, s2a,
					    normalnull);
		}
		else
		{
			if (normalnull == 0.0)

				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
					    bluecolor, s2, bluecolor, s2a);
			else
				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>"
					    "<span color=\"red\" font_family=\"Arial\" weight=\"bold\" size=\"8000\">\nNN %+.1f</span>",
					    bluecolor, s2, bluecolor, s2a,
					    normalnull);
		}
		gtk_label_set_markup (GTK_LABEL (altilabel), s3);
	}
	if (simmode)
		blink = TRUE;
	else
	{
		if (!havepos)
			blink = !blink;
	}

	if (newsatslevel)
		expose_sats_cb (NULL, 0);

	if (downloadwindowactive)
	{
		drawdownloadrectangle (1);
		expose_mini_cb (NULL, 0);
	}

	/* force to say new direction */
	if (!strcmp (oldangle, "XXX"))
		speech_out_cb (NULL, 0);
#endif
	return (TRUE);
}
