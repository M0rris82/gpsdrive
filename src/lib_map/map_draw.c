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
  double xmpp, ympp;
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

	// calculate the start and stop for lat/lon according to the displayed section
	screen2wgs(state, 0, 0, &lat_ul, &lon_ul);
	screen2wgs(state, 0, height, &lat_ll, &lon_ll);
	screen2wgs(state, width, 0, &lat_ur, &lon_ur);
	screen2wgs(state, width, height, &lat_lr, &lon_lr);
	//calcxytopos2 (0, 0, &lat_ul, &lon_ul, current.zoom);
	//calcxytopos2 (0, height, &lat_ll, &lon_ll, current.zoom);
	//calcxytopos2 (width, 0, &lat_ur, &lon_ur, current.zoom);
	//calcxytopos2 (width, height, &lat_lr, &lon_lr, current.zoom);

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
			//calcxy2 (&posxdest11, &posydest11, lon, lat, current.zoom);
			//calcxy2 (&posxdest12, &posydest12, lon, lat + step, current.zoom);
			//calcxy2 (&posxdest21, &posydest21, lon + step, lat, current.zoom);
			//calcxy2 (&posxdest22, &posydest22, lon + step, lat + step, current.zoom);

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

	if ((posxdest >= 0) && (posxdest < gui_status.mapview_x))
	{
		if ((posydest >= 0) && (posydest < gui_status.mapview_y))
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
			(wayp + i)->lon, (wayp + i)->lat, current.zoom);

		if ((posxdest >= 0) && (posxdest < gui_status.mapview_x)
		    && (shownwp < MAXSHOWNWP)
		    && (posydest >= 0) && (posydest < gui_status.mapview_y))
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
						* current.zoom * PIXELFACT
						/ mapscale;
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
					(map_drawingarea, tn);
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
					(map_drawingarea, txt);
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

