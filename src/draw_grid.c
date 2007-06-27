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
 * draw_grid module: 
 */

#include "config.h"
#include "gettext.h"
#include "gpsdrive.h"
#include "icons.h"
#include "streets.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "routes.h"
#include "import_map.h"
#include "download_map.h"
#include "gui.h"

#include "gettext.h"
#include <speech_strings.h>
#include <speech_out.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern GtkWidget *mylist;
extern gint maploaded;
extern gint importactive;
extern gint zoom;
extern status_struct route;
extern gint isnight, disableisnight;
extern color_struct colors;
extern gdouble current_long, current_lat;
extern gint mydebug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats,
  *drawing_miniimage;
extern glong mapscale;

extern gint muteflag;
extern gdouble alarm_lat, alarm_lon, alarm_dist;
extern gdouble target_lon, target_lat;
extern gdouble current_lon, current_lat, old_lon, old_lat, groundspeed;
extern gint posmode;
extern GtkWidget *posbt;
extern gdouble earthr;
extern gchar *displaytext;
extern gint do_display_dsc, textcount;
extern GtkWidget *mainwindow;
extern gchar targetname[40];
extern GtkWidget *destframe;
extern GTimer *timer, *disttimer;
extern gdouble gbreit, glang, olddist;
extern gdouble dist;
extern GtkWidget *messagewindow;
extern gint onemousebutton;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu;
extern gint int_padding;
extern GdkDrawable *drawable, *drawable_bearing, *drawable_sats;
extern gchar oldfilename[2048];

/* *****************************************************************************
 * Draw Text (lat/lon) into Grid
 */
void
draw_grid_text (GtkWidget * widget, gdouble posx, gdouble posy, gchar * txt)
{
	/* prints in pango */
	PangoFontDescription *pfd;
	PangoLayout *grid_label_layout;
	gint width, height;

	grid_label_layout = gtk_widget_create_pango_layout (drawing_area, txt);

	pfd = pango_font_description_from_string ("Sans 6");

	pango_layout_set_font_description (grid_label_layout, pfd);
	pango_layout_get_pixel_size (grid_label_layout, &width, &height);
	gdk_gc_set_function (kontext, GDK_XOR);
	gdk_gc_set_background (kontext, &colors.white);
	gdk_gc_set_foreground (kontext, &colors.mygray);
	
	gdk_draw_layout_with_colors (drawable, kontext, posx - width / 2,
				     posy - height / 2, grid_label_layout, &colors.black,
				     NULL);

	if (grid_label_layout != NULL)
		g_object_unref (G_OBJECT (grid_label_layout));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);

}

/* *****************************************************************************
 */
/*
 * Draw a grid over the map
 */
void
draw_grid (GtkWidget * widget)
{
	int count;
	gdouble step;
	gdouble lat, lon;
	gdouble lat_ul, lon_ul;
	gdouble lat_ll, lon_ll;
	gdouble lat_ur, lon_ur;
	gdouble lat_lr, lon_lr;

	gdouble lat_min, lon_min;
	gdouble lat_max, lon_max;

	if ( mydebug >50 ) 
	    fprintf(stderr , "draw_grid()\n");


	// calculate the start and stop for lat/lon according to the displayed section
	calcxytopos (0, 0, &lat_ul, &lon_ul, zoom);
	calcxytopos (0, SCREEN_Y, &lat_ll, &lon_ll, zoom);
	calcxytopos (SCREEN_X, 0, &lat_ur, &lon_ur, zoom);
	calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, zoom);

	// add more lines as the scale increases

	// Calculate distance between grid lines
	step = (gdouble) mapscale / 2000000.0 / zoom;
	gchar precission[10];
	gint iteration_count =0;
	do {
	    if (step >= 1)	            g_snprintf (precission, sizeof (precission), "%%.0f");
	    else if (step >= .1)	    g_snprintf (precission, sizeof (precission), "%%.1f");
	    else if (step >= .01)	    g_snprintf (precission, sizeof (precission), "%%.2f");
	    else if (step >= .001)	    g_snprintf (precission, sizeof (precission), "%%.3f");
	    else if (step >= .0001)	    g_snprintf (precission, sizeof (precission), "%%.4f");
	    else           		    g_snprintf (precission, sizeof (precission), "%%.5f");

	    if (mapscale < 5000000)
		{
		    lat_min = min (lat_ll, lat_ul) - step;
		    lat_max = max (lat_lr, lat_ur) + step;
		    lon_min = min (lon_ll, lon_ul) - step;
		    lon_max = max (lon_lr, lon_ur) + step;
		}
	    else
		{
		    lat_min = -90;
		    lat_max = 90;
		    lon_min = -180;
		    lon_max = 180;
		}
	    lat_min = floor (lat_min / step) * step;
	    lon_min = floor (lon_min / step) * step;

	    if ( mydebug > 20 )
		printf ("Draw Grid: (%.2f,%.2f) - (%.2f,%.2f) Step %f for Scale %ld Zoom %d\n", lat_min,
			lon_min, lat_max, lon_max,step,mapscale,zoom);
	    if ( mydebug > 40 )
		{
		    printf ("Draw Grid: (%.2f) Iterations for lat\n", (lat_max-lat_min)/step);
		    printf ("Draw Grid: (%.2f) Iterations for lon\n", (lon_max-lon_min)/step);
		}

	    // limit number of grid iterations to 100 iterations
	    iteration_count = ((lat_max-lat_min)/step) * ((lon_max-lon_min)/step);
	    if ( iteration_count > 100 ) {
		step = step * 2;
	    }
	} while ( iteration_count > 100 );

	// Loop over desired lat/lon  
	count = 0;
	for (lon = lon_min; lon <= lon_max; lon = lon + step)
	    {
		for (lat = lat_min; lat <= lat_max; lat = lat + step)
		    {
			gdouble posxdest11, posydest11;
			gdouble posxdest12, posydest12;
			gdouble posxdest21, posydest21;
			gdouble posxdest22, posydest22;
			gdouble posxdist, posydist;
			gchar str[200];

			count++;
			calcxy (&posxdest11, &posydest11, lon,        max(-90,lat)       , zoom);
			calcxy (&posxdest12, &posydest12, lon,        min( 90,lat + step), zoom);
			calcxy (&posxdest21, &posydest21, lon + step, max(-90,lat),        zoom);
			calcxy (&posxdest22, &posydest22, lon + step, min( 90,lat + step), zoom);

			if (((posxdest11 >= 0) && (posxdest11 < SCREEN_X) &&
			     (posydest11 >= 0) && (posydest11 < SCREEN_Y))
			    ||
			    ((posxdest22 >= 0) && (posxdest22 < SCREEN_X) &&
			     (posydest22 >= 0) && (posydest22 < SCREEN_Y))
			    ||
			    ((posxdest21 >= 0) && (posxdest21 < SCREEN_X) &&
			     (posydest21 >= 0) && (posydest21 < SCREEN_Y))
			    ||
			    ((posxdest12 >= 0) && (posxdest12 < SCREEN_X) &&
			     (posydest12 >= 0) && (posydest12 < SCREEN_Y)))
			    {
				// TODO: add linethikness 2 for Mayor Lines
				// Set Drawing Mode
				gdk_gc_set_function (kontext, GDK_XOR);
				gdk_gc_set_foreground (kontext, &colors.darkgrey);
				gdk_gc_set_line_attributes (kontext, 1, GDK_LINE_SOLID, 0, 0);

				gdk_draw_line (drawable, kontext, posxdest11,
					       posydest11, posxdest21,
					       posydest21);
				gdk_draw_line (drawable, kontext, posxdest11,
					       posydest11, posxdest12,
					       posydest12);
			    
				// Text lon
				g_snprintf (str, sizeof (str), precission,lon);

				posxdist = (posxdest12 - posxdest11) / 4;
				posydist = (posydest12 - posydest11) / 4;
				draw_grid_text (widget, 
						posxdest11 + posxdist,
						posydest11 + posydist, str);
				
				// Text lat
				g_snprintf (str, sizeof (str), precission,lat);
					    
				posxdist = (posxdest21 - posxdest11) / 4;
				posydist = (posydest21 - posydest11) / 4;
				draw_grid_text (widget, 
						posxdest11 + posxdist,
						posydest11 + posydist-5, str);
			    }
		    }
	    }
	if ( mydebug > 30 )
	    printf ("draw_grid loops: %d\n", count);
}
