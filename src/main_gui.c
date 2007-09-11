/*
**********************************************************************

Copyright (c) 2001-2007 Fritz Ganter <ganter@ganter.at>

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

**********************************************************************
*/

/*
 * main_gui.c
 *
 * This module holds all the gui stuff for the main window
 *
 */


#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <gpsdrive.h>
#include <icons.h>
#include <config.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#include "gettext.h"
#include <ctype.h>
#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "gui.h"
#include "poi_gui.h"
#include "battery.h"
#include "map_handler.h"
#include "import_map.h"
#include "download_map.h"
#include "main_gui.h"
#include "poi.h"
#include "wlan.h"
#include "routes.h"

/*  Defines for gettext I18n */
#include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern currentstatus_struct current;
extern coordinate_struct coords;
extern color_struct colors;
extern routestatus_struct route;
extern int actualfriends, maxfriends;
extern gchar loctime[20];
extern gint mydebug;
extern gint debug;
extern gint iszoomed, xoff, yoff;
extern gint sats_used, sats_in_view;

extern gdouble wp_saved_target_lat;
extern gdouble wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat;
extern gdouble wp_saved_posmode_lon;
extern gdouble pixelfact;

extern gint usesql, havespeechout;
extern gint slistsize, nlist[];
extern gint PSIZE;
extern GtkWidget *posbt;
extern GtkWidget *bestmap_bt;
extern GtkWidget *mapnik_bt;

extern GtkWidget *main_window;


/* Globally accessible widgets: */
GtkWidget *map_drawingarea;
GtkWidget *scaler_left_bt, *scaler_right_bt;
GtkWidget *frame_statusbar, *frame_statusfriends;
GtkWidget *main_table;

// TODO: maybe these should be moved to local ones...
GtkWidget *drawing_compass, *drawing_minimap, *drawing_gpsfix;
GdkDrawable *drawable_compass, *drawable_minimap;
GdkGC *kontext_compass, *kontext_gps, *kontext_minimap, *kontext_map;


/* local widgets */
static GtkWidget *mainbox_controls, *mainbox_status, *mainframe_map;
static GtkTooltips *main_tooltips;
static GtkWidget *frame_maptype;
static GtkWidget *mapscaler_scaler;
static GtkWidget *zoomin_bt, *zoomout_bt;
static GtkWidget *statusprefscale_lb, *statusmapscale_lb;
static GtkObject *mapscaler_adj;
static GtkWidget *frame_dash_1, *frame_dash_2, *frame_dash_3, *frame_dash_4;
static GtkWidget *statusfriends_lb, *statustime_lb;
static GtkWidget *statuslat_lb, *statuslon_lb;
static GtkWidget *statusheading_lb, *statusbearing_lb;
static GtkWidget *hbox_zoom;

/* Definitions for main menu */
enum
{
	MENU_MAPIMPORT,
	MENU_MAPDOWNLOAD,
	MENU_LOADTRACK,
	MENU_LOADROUTE,
	MENU_SENDMSG,
	MENU_SETTINGS,
	MENU_HELPABOUT,
	MENU_HELPCONTENT,
};

/* *****************************************************************************
 * CALLBACKS
 * *****************************************************************************/


/* *****************************************************************************
 * quit the program 
 */
gint
quit_program_cb (GtkWidget *widget, gpointer datum)
{
    gtk_main_quit ();
    return TRUE;
}


/* ****************************************************************************
 * toggle checkbox for mapnik mode
 * datum == 2 also means switch off mapnik button
 */
gint
toggle_mapnik_cb (GtkWidget *widget, guint datum)
{
	if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) 
	     || (datum == 2))
	{
		local_config.MapnikStatusInt = 1;
		gtk_widget_hide_all (frame_maptype);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmap_bt), FALSE);
		gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (widget), TRUE);
		autobestmap_cb(bestmap_bt,0);
	}
	else 
	{
		local_config.MapnikStatusInt = 0;
		gtk_widget_show_all (GTK_WIDGET (frame_maptype));
	}
	
	// TODO: reload/update map display.....

    return TRUE;
}


/* *****************************************************************************
 *  toggle coordinate format mode 
 */
gint
toggle_coords_cb (GtkWidget *widget)
{
	local_config.coordmode++;
	if (local_config.coordmode >= LATLON_N_FORMATS)
		local_config.coordmode = 0;
	return TRUE;
}


/* *****************************************************************************
 *  evaluate choice from main menu
 */
gint
main_menu_cb (GtkWidget *widget, gint choice)
{
	switch (choice)
	{
		case MENU_MAPIMPORT:	import1_cb (NULL, 1); break;
		case MENU_MAPDOWNLOAD:	download_cb (NULL, 0); break;
		case MENU_LOADTRACK:	loadtrack_cb (NULL, 0); break;
		case MENU_LOADROUTE:	return TRUE; break;
		case MENU_SENDMSG:	sel_message_cb (NULL, 0); break;
		case MENU_SETTINGS:	settings_main_cb (NULL, 0); break;
		case MENU_HELPABOUT:	about_cb (NULL, 0); break;
		case MENU_HELPCONTENT:	help_cb (NULL, 0); break;
	}
	return TRUE;
}


/* *****************************************************************************
 */
gint
autobestmap_cb (GtkWidget *widget, guint datum)
{
	gchar sc[15];

	if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) )
	{
		gtk_widget_set_sensitive (scaler_right_bt, FALSE);
		gtk_widget_set_sensitive (scaler_left_bt, FALSE);
		gtk_label_set_text (GTK_LABEL (statusprefscale_lb), _("Auto"));
		if (mapscaler_scaler)
			gtk_widget_set_sensitive (mapscaler_scaler, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive (scaler_right_bt, TRUE);
		gtk_widget_set_sensitive (scaler_left_bt, TRUE);
		g_snprintf (sc, sizeof (sc), "1:%d", local_config.scale_wanted);
		gtk_label_set_text (GTK_LABEL (statusprefscale_lb), sc);
		if (mapscaler_scaler)
			gtk_widget_set_sensitive (mapscaler_scaler, TRUE);
	}

	current.needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 */
gint
minimapclick_cb (GtkWidget *widget, GdkEventMotion *event)
{
	gint x, y, px, py;
	gdouble dif, lon, lat;
	GdkModifierType state;

	if (event->is_hint)
		gdk_window_get_pointer (event->window, &x, &y, &state);
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
	if (state == 0)
		return 0;
#define MINISCREEN_X_2 64
#define MINISCREEN_Y_2 51
	px = (MINISCREEN_X_2 - x) * 10 * pixelfact;
	py = (-MINISCREEN_Y_2 + y) * 10 * pixelfact;

	minimap_xy2latlon(px, py, &lon, &lat, &dif);

	/*        g_print("\nstate: %x x:%d y:%d\n", state, x, y); */

	/*  Left mouse button */
	if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
	{
	    if (gui_status.posmode)
		{
		    coords.posmode_lon = lon;
		    coords.posmode_lat = lat;
		    rebuildtracklist ();
		}
	}
	/*  Middle mouse button */
	if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt),
					      FALSE);
		rebuildtracklist ();
	}

	/*    g_print("\nx: %d, y: %d\n", x, y); */
	return TRUE;
}


/* *****************************************************************************
 */
gint
zoom_cb (GtkWidget *widget, guint datum)
{

	if (iszoomed == FALSE)	/* needed to be sure the old zoom is made */
		return TRUE;
	iszoomed = FALSE;
	
	if (datum == 1)
	{	/* zoom in */
		if (current.zoom >= ZOOM_MAX)
		{
			current.zoom = ZOOM_MAX;
			iszoomed = TRUE;
		}
		else
		{
			current.zoom *= 2;
			xoff *= 2;
			yoff *= 2;
		}
	}
	else if (datum == 2)
	{	/* zoom out */
		if (current.zoom <= ZOOM_MIN)
		{
			current.zoom = ZOOM_MIN;
			iszoomed = TRUE;
		}
		else
		{
			current.zoom /= 2;
			xoff /= 2;
			yoff /= 2;
		}
	}

	if (current.zoom == ZOOM_MIN)
		gtk_widget_set_sensitive (zoomout_bt, FALSE);
	else
		gtk_widget_set_sensitive (zoomout_bt, TRUE);
	if (current.zoom == ZOOM_MAX)
		gtk_widget_set_sensitive (zoomin_bt, FALSE);
	else
		gtk_widget_set_sensitive (zoomin_bt, TRUE);

	if (current.importactive)
	{
		expose_cb (NULL, 0);
	}
	return TRUE;
}


/* *****************************************************************************
 */
gint
scaler_cb (GtkAdjustment *adj, gdouble *datum)
{
	gchar sc[15];
	local_config.scale_wanted = nlist[(gint) rint (adj->value)];
	g_snprintf (sc, sizeof (sc), "1:%d", local_config.scale_wanted);
	gtk_label_set_text (GTK_LABEL (statusprefscale_lb), sc);
	if ( mydebug > 12 )
		g_print ("Scaler: %d\n", local_config.scale_wanted);
	current.needtosave = TRUE;

	return TRUE;
}


/* *****************************************************************************
 * Increase/decrease displayed map scale
 * TODO: Improve finding of next apropriate map
 * datum:
 *    1 Zoom out
 *    2 Zoom in 
 */
gint
scalerbt_cb (GtkWidget *widget, guint datum)
{
	if (datum == 1)
	{
		gtk_adjustment_set_value (GTK_ADJUSTMENT (mapscaler_adj),
			GTK_ADJUSTMENT (mapscaler_adj)->value +
			GTK_ADJUSTMENT (mapscaler_adj)->step_increment);
	}
	else if (datum == 2)
	{
		gtk_adjustment_set_value (GTK_ADJUSTMENT (mapscaler_adj),
			GTK_ADJUSTMENT (mapscaler_adj)->value -
			GTK_ADJUSTMENT (mapscaler_adj)->step_increment);
	}

	// TODO: check, if this is really necessary, and maybe remove...
	expose_cb (NULL, 0);
	expose_mini_cb (NULL, 0);

	current.needtosave = TRUE;

	return TRUE;
}


/* *****************************************************************************
 * Update Contents of Dashboard fields
 *  the first parameter chooses one of the dashboard fields to update
 *  the second parameter chooses, which value should be dispayed there
 */
gint
update_dashboard (GtkWidget *frame, gint source)
{
	// TODO: add "remaining time", "trip", "gps precision"
	//gchar s2[100], s3[200], s2a[20];
	gchar head[100], content[100], ctmp[10], unit[10], dirs = ' ';
	//gchar font_unit[100];
	gint font_size;

	gdouble dir = 0.0;

	font_size = 24;
	//font_size = pango_font_description_get_size
	//	(local_config.color_dashboard);

	switch (source)
	{
		case DASH_DIST:
		{
			g_strlcpy (head, _("Distance"), sizeof (head));
			switch (local_config.distmode)
			{
				case DIST_MILES:
				{
					g_strlcpy (unit, "mi", sizeof (unit));
					if (current.dist <= 1.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.0f",
							current.dist * 1760.0);
						g_strlcpy (unit, "yrds",
							sizeof (unit));
					}
					else if (current.dist <= 10.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.2f",
							current.dist);
					}
					else
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.1f", 
							current.dist);
					}
					break;
				}
				case DIST_NAUTIC:
				{
					g_strlcpy (unit, _("nmi"),
						sizeof (unit));
					if (current.dist <= 1.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.3f", 
							current.dist);
					}
					else if (current.dist <= 10.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.2f", 
							current.dist);
					}
					else
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.1f", 
							current.dist);
					}
					break;
				}
				default:
				{
					g_strlcpy (unit, _("km"),
						sizeof (unit));
					if (current.dist <= 1.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.0f",
							current.dist * 1000.0);
						g_strlcpy (unit, "m",
							sizeof (unit));
					}
					else if (current.dist <= 10.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.2f", 
							current.dist);
					}
					else
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.1f", 
							current.dist);
					}
				}
			}
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s"
				"<span font_desc=\"%.f\"> %s</span></span>",
			local_config.color_dashboard,
			local_config.font_dashboard, ctmp,
			font_size*0.66, unit);
			break;
		}
		case DASH_SPEED:
		{
			g_strlcpy (head, _("Speed"), sizeof (head));
			switch (local_config.distmode)
			{
				case DIST_MILES:
					g_strlcpy (unit, _("mi/h"),
						sizeof (unit));
					break;
				case DIST_NAUTIC:
					g_strlcpy (unit, _("knots"),
						sizeof (unit));
					break;
				default:
					g_strlcpy (unit, _("km/h"),
						sizeof (unit));
			}
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">"
				"% 3.1f<span font_desc=\"%.f\"> %s</span></span>",
				local_config.color_dashboard,
				local_config.font_dashboard,
				current.groundspeed,
				font_size*0.66, unit);
			break;
		}
		case DASH_BEARING:
		{
			g_strlcpy (head, _("Bearing"), sizeof (head));
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%03.f"
				"%s</span>",
			local_config.color_dashboard,
			local_config.font_dashboard,
			RAD2DEG (current.bearing), DEGREE);
			break;
		}
		case DASH_HEADING:
		{
			g_strlcpy (head, _("Heading"), sizeof (head));
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%03.f"
				"%s</span>",
			local_config.color_dashboard,
			local_config.font_dashboard,
			RAD2DEG (current.heading), DEGREE);
			break;
		}
		case DASH_TURN:
		{
			g_strlcpy (head, _("Turn"), sizeof (head));
			dir = RAD2DEG (current.bearing)
				- RAD2DEG (current.heading);
			if (dir > 180)
				dir = dir - 360;
			else if (dir < -180)
				dir = 360 + dir;
			if (dir < 0.0)
				dirs = 'L';
			else if (dir > 0.0)
				dirs = 'R';
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%c %03d"
				"%s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard,
				dirs, abs (dir), DEGREE);
			break;
		}
		case DASH_TIMEREMAIN:
		{
			g_strlcpy (head, _("Time remaining"), sizeof (head));
				
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s"
				"%s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard, "---", "min");
			break;
		}
		case DASH_ALT:
		{
			g_strlcpy (head, _("Altitude"), sizeof (head));
			if (current.gpsfix == 3)
			{
			switch (local_config.altmode)
			{
				case ALT_FEET:
				{
					g_snprintf (ctmp, sizeof (ctmp),
						"%.0f", current.altitude
						* 3.2808399 +
						local_config.normalnull);
					g_strlcpy (unit, "ft", sizeof (unit));
					break;
				}
				case ALT_YARDS:
				{
					g_snprintf (ctmp, sizeof (ctmp),
						"%.0f", current.altitude
						* 1.093613 +
						local_config.normalnull);
					g_strlcpy (unit, "yd", sizeof (unit));
					break;
				}
				default:
				{
					g_snprintf (ctmp, sizeof (ctmp),
						"%.0f", current.altitude
						+ local_config.normalnull);
					g_strlcpy (unit, "m", sizeof (unit));
				}
			}
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">"
				"% 3.1f<span font_desc=\"%.f\"> %s</span></span>",
				local_config.color_dashboard,
				local_config.font_dashboard,
				current.groundspeed,
				font_size*0.66, unit);
			}
			else
			{
				g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\" "
				"size=\"small\"> %s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard, _("n/a"));
			}
			break;
		}
		case DASH_TRIP:
		{
			g_strlcpy (head, _("Trip"), sizeof (head));
				
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s"
				"%s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard, "---", "km");
			break;
		}
		case DASH_GPSPRECISION:
		{
			g_strlcpy (head, _("GPS Precision"), sizeof (head));
				
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s"
				"%s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard, "---", "m");
			break;
		}
		case DASH_TIME:
		{
			g_strlcpy (head, _("Current Time"), sizeof (head));
			if (current.gpsfix > 1)
			{
				g_snprintf (content, sizeof (content),
					"<span color=\"%s\" font_desc=\"%s\">"
					"%s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard, loctime);
			}
			else
			{
				g_snprintf (content, sizeof (content),
					"<span color=\"%s\" font_desc=\"%s\" "
					"size=\"16000\">%s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard, _("n/a"));
			}
			break;
		}
	}

	g_object_set (frame, "label", head, NULL);
	gtk_label_set_markup (GTK_LABEL (GTK_BIN (frame)->child), content);

	return TRUE;
}


/* *****************************************************************************
 * Update Status Displays
 TODO: move functionality from gpsdrive.c to here
 */
gint
update_statusdisplay ()
{
	gchar stmp[20];
	gdouble t_lat, t_lon;

	/* update gps time */
	gtk_label_set_text (GTK_LABEL (statustime_lb), loctime);

	/* update friends */
	if (local_config.showfriends)
	{
		g_snprintf (stmp, sizeof (stmp), "%d/%d", actualfriends,
			maxfriends);
		gtk_label_set_text (GTK_LABEL (statusfriends_lb), stmp);
	}

	/* update lat/lon display */
	if (gui_status.posmode)
	{
		GdkModifierType state;
		gint x, y;
		
		gdk_window_get_pointer (map_drawingarea->window, &x, &y,&state);
		calcxytopos (x, y, &t_lat, &t_lon, current.zoom);
		if ( mydebug > 20 )
			printf ("Actual mouse position: lat:%f,lon:%f"
				"(x:%d,y:%d)\n", t_lat, t_lon, x, y);
		/* display position of Mouse in lat/lon Fields */
	}
	else
	{
		t_lat = coords.current_lat;
		t_lon = coords.current_lon;
	}
	coordinate2gchar(stmp, sizeof(stmp), t_lat, TRUE,
		local_config.coordmode);
	gtk_label_set_text (GTK_LABEL (statuslat_lb), stmp);
	coordinate2gchar(stmp, sizeof(stmp), t_lon, FALSE,
		local_config.coordmode);
	gtk_label_set_text (GTK_LABEL (statuslon_lb), stmp);

	/*update mapscale */
	g_snprintf (stmp, sizeof (stmp), "1:%ld", current.mapscale);
	gtk_label_set_text (GTK_LABEL (statusmapscale_lb), stmp);

	/* update heading */
	g_snprintf (stmp, sizeof (stmp), "%3.0f%s",
		RAD2DEG (current.heading), DEGREE);
	gtk_label_set_text (GTK_LABEL (statusheading_lb), stmp);

	/* update bearing */
	g_snprintf (stmp, sizeof (stmp), "%3.0f%s",
		RAD2DEG (current.bearing), DEGREE);
	gtk_label_set_text (GTK_LABEL (statusbearing_lb), stmp);

	/* update dashboard */
	update_dashboard (frame_dash_1, local_config.dashboard_1);
	update_dashboard (frame_dash_2, local_config.dashboard_2);
	update_dashboard (frame_dash_3, local_config.dashboard_3);
	update_dashboard (frame_dash_4, local_config.dashboard_4);

	return TRUE;
}


/* *****************************************************************************
 * Update Display of GPS Status
 */
gint
expose_gpsfix (GtkWidget *widget, guint *datum)
{
	GdkDrawable *drawable_gpsfix;
	GdkGC *kontext_gpsfix;
	PangoFontDescription *pfd_gpsfix;
	PangoLayout *layout_gpsfix;
	gint t_x, t_y, t_tx, t_ty, t_wx, i;

	if (mydebug >50)
		fprintf (stderr, "expose_gpsfix()\n");

	drawable_gpsfix = drawing_gpsfix->window;
	kontext_gpsfix = gdk_gc_new (drawable_gpsfix);
	pfd_gpsfix = pango_font_description_from_string ("Sans 8");
	gdk_drawable_get_size (drawable_gpsfix, &t_x, &t_y);

	layout_gpsfix = gtk_widget_create_pango_layout
		(drawing_gpsfix, _("No GPS"));
	pango_layout_set_font_description (layout_gpsfix, pfd_gpsfix);
	pango_layout_get_pixel_size (layout_gpsfix, &t_tx, &t_ty);

	if (current.simmode)
	{
		gdk_gc_set_foreground (kontext_gpsfix, &colors.blue);
		pango_layout_set_text (layout_gpsfix, _("Simulation Mode"), -1);
		gdk_draw_rectangle (drawable_gpsfix, kontext_gpsfix,
			TRUE, 0,0, t_x, t_y);
		gdk_draw_layout_with_colors (drawable_gpsfix, kontext_gpsfix,
			5, (t_y-t_ty)/2, layout_gpsfix,
			&colors.white, NULL);	
	}
	else
	{
		switch (current.gpsfix)
		{
		case 0:
		{
			gdk_gc_set_foreground (kontext_gpsfix, &colors.red);
			break;
		}
		case 1:
		{
			gdk_gc_set_foreground (kontext_gpsfix, &colors.red);
			pango_layout_set_text (layout_gpsfix, "No Fix", -1);
			break;
		}
		case 2:
		{
			// TODO: have a look at the nmea parsing
			// to avoid "jumping" between 2D and 3D
			gdk_gc_set_foreground (kontext_gpsfix, &colors.green);
			pango_layout_set_text (layout_gpsfix, "2D Fix", -1);
			break;
		}
		case 3:
		{
			gdk_gc_set_foreground (kontext_gpsfix, &colors.green);
			pango_layout_set_text (layout_gpsfix, "3D Fix", -1);
			break;
		}
		}
	
		gdk_draw_rectangle (drawable_gpsfix, kontext_gpsfix,
			TRUE, 0,0, t_x, t_y);
		gdk_draw_layout_with_colors (drawable_gpsfix, kontext_gpsfix,
			t_x-t_tx-5, (t_y-t_ty)/2, layout_gpsfix,
			&colors.black, NULL);

		gdk_gc_set_foreground (kontext_gpsfix, &colors.black);
		t_wx = (t_x-t_tx-26)/12;
		if (t_wx < 1)
			t_wx =1;
		gdk_gc_set_line_attributes (kontext_gpsfix, t_wx, 0, 0, 0);
		for (i=1; i<13; i++)
		{
			if (i > sats_in_view)
				gdk_gc_set_foreground
					(kontext_gpsfix, &colors.grey);
			else if (i > sats_used)
				gdk_gc_set_foreground
					(kontext_gpsfix, &colors.darkgrey);
			gdk_draw_line (drawable_gpsfix, kontext_gpsfix,
				i*(t_wx+1)+5, 5, i*(t_wx+1)+5, t_y-5);
		}
	}

	return TRUE;
}


/* *****************************************************************************
 * Reactions to pressed key
 TODO: check if everything is fine and not obsolete
 */
gint
key_pressed_cb (GtkWidget * widget, GdkEventKey * event)
{
	gdouble lat, lon;
	gint x, y;
	GdkModifierType state;

	if ( mydebug > 0 )
		g_print ("event:%x key:%c\n", event->keyval, event->keyval);

	// Toggle Grid Display
	if ((toupper (event->keyval)) == 'G')
	{
		local_config.showgrid = !local_config.showgrid;
		current.needtosave = TRUE;
	}

	// Toggle Friends Server activities
	if ((toupper (event->keyval)) == 'F')
	{
		local_config.showfriends = !local_config.showfriends;
		current.needtosave = TRUE;
	}

	// Add Waypoint at current gps location
	if ((toupper (event->keyval)) == 'X')
	{
		coords.wp_lat = coords.current_lat;
		coords.wp_lon = coords.current_lon;
		addwaypoint_cb (NULL, NULL);
	}

	// Add Waypoint at current gps location without asking
	if ((toupper (event->keyval)) == 'W')
	{
		gchar wp_name[100], wp_type[100], wp_comment[100];
		time_t t;
		struct tm *ts;
		time (&t);
		ts = localtime (&t);
		g_snprintf (wp_name, sizeof (wp_name), "%s", asctime (ts));
		g_snprintf (wp_type, sizeof (wp_type),
			"waypoint.wpttemp.wpttemp-green");
		g_snprintf (wp_comment, sizeof (wp_comment),
			_("Quicksaved Waypoint"));
		if (usesql)
			addwaypoint (wp_name, wp_type, wp_comment,
				coords.current_lat, coords.current_lon, TRUE);
		else
			addwaypoint (wp_name, wp_type, wp_comment,
				coords.current_lat, coords.current_lon, FALSE);
	}

	// Add waypoint at current mouse location
	if ((toupper (event->keyval)) == 'Y')
	{
		gdk_window_get_pointer (map_drawingarea->window, &x, &y, &state);
		calcxytopos (x, y, &lat, &lon, current.zoom);
		if ( mydebug > 0 )
			printf ("Actual Position: lat:%f,lon:%f (x:%d,y:%d)\n",
				lat, lon, x, y);
		/*  Add mouse position as waypoint */
		coords.wp_lat = lat;
		coords.wp_lon = lon;
		addwaypoint_cb (NULL, 0);
	}

	// Add instant waypoint a current mouse location
	if ((toupper (event->keyval)) == 'P')
	{
		gchar wp_name[100], wp_type[100], wp_comment[100];
		time_t t;
		struct tm *ts;
		time (&t);
		ts = localtime (&t);
		g_snprintf (wp_name, sizeof (wp_name), "%s", asctime (ts));
		g_snprintf (wp_type, sizeof (wp_type),
			"waypoint.wpttemp.wpttemp-yellow");
		g_snprintf (wp_comment, sizeof (wp_comment),
			_("Temporary Waypoint"));
		gdk_window_get_pointer (map_drawingarea->window, &x, &y, &state);
		calcxytopos (x, y, &lat, &lon, current.zoom);
		if ( mydebug > 0 )
			printf ("Add Waypoint: %s lat:%f,lon:%f (x:%d,y:%d)\n",
				wp_name, lat, lon, x, y);
		if (usesql)
			addwaypoint (wp_name, wp_type, wp_comment, lat, lon, TRUE);
		else
			addwaypoint (wp_name, wp_type, wp_comment, lat, lon, FALSE);
	}

	// Add instant routepoint at current mouse location
	if ((toupper (event->keyval)) == 'R')
	{
		quickadd_routepoint ();
	}

	// In Route mode Force next Route Point
	if (((toupper (event->keyval)) == 'J') && route.active)
	{
		route.forcenext = TRUE;
	}


	// Zoom in/out
	
	//    if (local_config.guimode == GUI_PDA)
	//	{
	//	    if (event->keyval == 0xFF52)
	//		scalerbt_cb (NULL, 1);	/* RNM */
	//	    if (event->keyval == 0xFF54)
	//		scalerbt_cb (NULL, 2);	/* RNM */
	//	}
	if ((toupper (event->keyval)) == '-' || (event->keyval == 0xFFad))
	{
		/* Zoom out */
		scalerbt_cb (NULL, 1);
	}
	if ((toupper (event->keyval)) == '+' || (event->keyval == 0xFFab))
	{
		/* Zoom in */
		scalerbt_cb (NULL, 2);
	}

	// Switch Night Mode
	if ((toupper (event->keyval)) == 'N')
	{
		// TODO: enable functionality once nightmode
		//	is available again
	}

	// Toggle POI Display
	/*
	 * if ((toupper (event->keyval)) == 'I' )
	 * {
	 * poi_draw = !poi_draw;
	 * poi_draw_list ();
	 * gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poi_draw_bt), poi_draw);
	 * current.needtosave = TRUE;
	 * }
	 */

	// Query Info for next points and streets
	if ( ( (toupper (event->keyval)) == '?' )
	     || ( (toupper (event->keyval)) == 'Q') )
	{
	    gdk_window_get_pointer (map_drawingarea->window, &x, &y, &state);
	    gdouble lat1,lon1,lat2,lon2;
	    gint delta = 10;
	    calcxytopos (x-delta, y-delta, &lat1, &lon1, current.zoom);
	    calcxytopos (x+delta, y+delta, &lat2, &lon2, current.zoom);
	    printf ("---------------------------------------------\n");
	    if ( local_config.showpoi )
		poi_query_area (min(lat1,lat2), min(lon1,lon2),
			max(lat1,lat2), max(lon1,lon2) );

	    if ( local_config.showwlan )
		wlan_query_area (min(lat1,lat2),
			min(lon1,lon2), max(lat1,lat2), max(lon1,lon2) );

	    gdouble lat,lon;
	    calcxytopos (x, y, &lat, &lon, current.zoom);
	    gdouble dist=lat2-lat1;
	    dist = dist>0?dist:-dist;
//	    if ( streets_draw )
//		streets_query_point ( lat,lon, dist );
	}


	if ( mydebug>10 ) 
	    fprintf(stderr,"Key pressed: %0x\n",event->keyval);


	if (gui_status.posmode)
	    {
		gdouble x, y;

		if ( (event->keyval == 0xff52))	// Up
		    {
			calcxy (&x, &y, coords.current_lon,
				coords.current_lat, current.zoom);
			calcxytopos (x, 0, &coords.current_lat,
				&coords.current_lon, current.zoom);
		    }
		
		if ( (event->keyval == 0xff54 )) // Down
		    {
			calcxy (&x, &y, coords.current_lon,
				coords.current_lat, current.zoom);
			calcxytopos (x, SCREEN_Y, &coords.current_lat,
				&coords.current_lon, current.zoom);
		    }
		if ( (event->keyval == 0xff51 )) // Left
		    {
			calcxy (&x, &y, coords.current_lon,
				coords.current_lat, current.zoom);
			calcxytopos (0, y, &coords.current_lat,
				&coords.current_lon, current.zoom);
		    }
		if ( (event->keyval == 0xff53 )) //  Right
			    
		    {
			calcxy (&x, &y, coords.current_lon, coords.current_lat, current.zoom);
			calcxytopos (SCREEN_X, y, &coords.current_lat, &coords.current_lon, current.zoom);
		    }
		coords.posmode_lon = coords.current_lon;
		coords.posmode_lat = coords.current_lat;
	    }
	return 0;
}


/* *****************************************************************************
 * Window: Main -> Controls
 */
void create_controls_mainbox (void)
{
	GtkWidget *vbox_buttons, *frame_poi, *frame_track;
	GtkWidget *frame_mapcontrol;
	GtkWidget *zoomin_img, *zoomout_img;
	GtkWidget *hbox_scaler, *mute_bt;
	GtkWidget *pda_box_left, *pda_box_right;

	GtkWidget *menuitem_maps, *menuitem_mapimport, *menuitem_mapdownload;
	GtkWidget *menuitem_load, *menuitem_sendmsg, *menuitem_settings;
	GtkWidget *menuitem_help, *menuitem_helpabout, *menuitem_helpcontent;
	GtkWidget *menuitem_loadtrack, *menuitem_loadroute;
	GtkWidget *menuitem_quit, *main_menu, *menuitem_menu;
	GtkWidget *menu_menu, *menu_help, *menu_maps, *menu_load;
	GtkWidget *menuitem_sep, *sendmsg_img;

	GtkWidget *vbox_poi, *poi_draw_bt, *wlan_draw_bt, *wp_draw_bt;
	GtkWidget *vbox_track, *showtrack_bt, *savetrack_bt;

	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox\n");

	/* MENU AND BUTTONS */
	{
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(MENU AND BUTTONS)\n");
	vbox_buttons = gtk_vbox_new (TRUE, 3 * PADDING);

	/* Main Menu */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Main Menu)\n");
	main_menu = gtk_menu_bar_new ();
	menu_menu = gtk_menu_new ();
	menuitem_menu = gtk_menu_item_new_with_label (_("Options"));
	gtk_menu_shell_append (GTK_MENU_SHELL (main_menu), menuitem_menu);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_menu), menu_menu);

	menu_maps = gtk_menu_new ();
	menuitem_maps = gtk_menu_item_new_with_label (_("Maps"));
	menuitem_mapimport = gtk_menu_item_new_with_label (_("Import"));
	menuitem_mapdownload = gtk_menu_item_new_with_label (_("Download"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_menu), menuitem_maps);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_maps), menu_maps);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_maps), menuitem_mapimport);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_maps),
		menuitem_mapdownload);
	g_signal_connect (menuitem_mapimport, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_MAPIMPORT);
	g_signal_connect (menuitem_mapdownload, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_MAPDOWNLOAD);

	menu_load = gtk_menu_new ();
	menuitem_load = gtk_image_menu_item_new_from_stock ("gtk-open", NULL);
	menuitem_loadtrack = gtk_menu_item_new_with_label (_("Track File"));
	menuitem_loadroute = gtk_menu_item_new_with_label (_("Route File"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_menu), menuitem_load);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_load), menu_load);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_load), menuitem_loadtrack);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_load), menuitem_loadroute);
	g_signal_connect (menuitem_loadtrack, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_LOADTRACK);
	g_signal_connect (menuitem_loadroute, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_LOADROUTE);

	// TODO: add load route functionality, until then the item is disabled
	gtk_widget_set_sensitive (menuitem_loadroute, FALSE);

	menuitem_sendmsg =
		gtk_image_menu_item_new_with_label (_("Send Message"));
	sendmsg_img =
		gtk_image_new_from_stock ("gtk-network", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image
		(GTK_IMAGE_MENU_ITEM (menuitem_sendmsg), sendmsg_img);
	menuitem_settings =
		gtk_image_menu_item_new_from_stock ("gtk-preferences", NULL);
	menuitem_sep = gtk_separator_menu_item_new ();
	menuitem_quit =
		gtk_image_menu_item_new_from_stock ("gtk-quit", NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_menu), menuitem_sendmsg);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_menu), menuitem_settings);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_menu), menuitem_sep);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_menu), menuitem_quit);
	g_signal_connect (menuitem_sendmsg, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_SENDMSG);
	g_signal_connect (menuitem_settings, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_SETTINGS);
	g_signal_connect (menuitem_quit, "activate",
		GTK_SIGNAL_FUNC (quit_program_cb), NULL);

	/* Help Menu */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Help Menu)\n");
	menu_help = gtk_menu_new ();
	menuitem_help = gtk_menu_item_new_with_label (_("Help"));
	menuitem_helpabout =
		gtk_image_menu_item_new_from_stock ("gtk-about", NULL);
	menuitem_helpcontent =
		gtk_image_menu_item_new_from_stock ("gtk-help", NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (main_menu), menuitem_help);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_help), menu_help);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_help),
		menuitem_helpabout);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_help),
		menuitem_helpcontent);
	g_signal_connect (menuitem_helpabout, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_HELPABOUT);
	g_signal_connect (menuitem_helpcontent, "activate",
		GTK_SIGNAL_FUNC (main_menu_cb), (gpointer) MENU_HELPCONTENT);

	gtk_box_pack_start (GTK_BOX (vbox_buttons),
		main_menu, FALSE, FALSE, 1 * PADDING);

	/* Buttons: Zoom */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: Zoom)\n");
	hbox_zoom = gtk_hbox_new (FALSE, 1 * PADDING);
	zoomin_bt = gtk_button_new ();
	zoomin_img = gtk_image_new_from_stock
		("gtk-zoom-in", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image (GTK_BUTTON (zoomin_bt), zoomin_img);
	g_signal_connect (GTK_OBJECT (zoomin_bt), "clicked",
		GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 1);
	zoomout_bt = gtk_button_new ();
	zoomout_img = gtk_image_new_from_stock
		("gtk-zoom-out", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image (GTK_BUTTON (zoomout_bt), zoomout_img);
	g_signal_connect (GTK_OBJECT (zoomout_bt), "clicked",
		GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 2);
	if (current.zoom <= ZOOM_MIN)
		gtk_widget_set_sensitive (zoomout_bt, FALSE);
	if (current.zoom >= ZOOM_MAX)
		gtk_widget_set_sensitive (zoomin_bt, FALSE);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), zoomin_bt,
		_("Zoom into the current map"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), zoomout_bt,
			      _("Zooms out off the current map"), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_zoom),
			    zoomout_bt, TRUE, TRUE, 1 * PADDING);
	gtk_box_pack_start (GTK_BOX (hbox_zoom),
			    zoomin_bt, TRUE, TRUE,  1 * PADDING);
	if (local_config.guimode != GUI_CAR)	{
	    gtk_box_pack_start (GTK_BOX (vbox_buttons),
				hbox_zoom, FALSE, FALSE, 1 * PADDING);
	}

	/* Buttons: Scaler */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: Scaler)\n");
	hbox_scaler  = gtk_hbox_new (FALSE, 1 * PADDING);
	scaler_right_bt = gtk_button_new_with_label (">>");
	g_signal_connect (GTK_OBJECT (scaler_right_bt), "clicked",
		GTK_SIGNAL_FUNC (scalerbt_cb), (gpointer) 1);
	scaler_left_bt = gtk_button_new_with_label ("<<");
	g_signal_connect (GTK_OBJECT (scaler_left_bt), "clicked",
		GTK_SIGNAL_FUNC (scalerbt_cb), (gpointer) 2);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), scaler_left_bt,
		_("Select the next more detailed map"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), scaler_right_bt,
		_("Select the next less detailed map"), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_scaler),
		scaler_left_bt, TRUE, TRUE, 1 * PADDING);
	gtk_box_pack_start (GTK_BOX (hbox_scaler),
		scaler_right_bt, TRUE, TRUE, 1 * PADDING);
	gtk_box_pack_start (GTK_BOX (vbox_buttons),
		hbox_scaler, FALSE, FALSE, 1 * PADDING);


	/* Button: Mute Speech */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: Mute Speech)\n");
	if (havespeechout)
	{
		mute_bt = gtk_check_button_new_with_label (_("Mute Speech"));
		gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (mute_bt), FALSE);
		if (local_config.mute)
		{
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (mute_bt), TRUE);
		}
		g_signal_connect (GTK_OBJECT (mute_bt), "clicked",
		GTK_SIGNAL_FUNC (toggle_button_cb),
			&local_config.mute);
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), mute_bt,
			_("Disable output of speech"), NULL);
		gtk_box_pack_start (GTK_BOX (vbox_buttons),
			mute_bt, FALSE, FALSE, 1 * PADDING);
	}

	/* Button: Search POIs */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: Search POI)\n");
	find_poi_bt = gtk_button_new_from_stock (GTK_STOCK_FIND);
	if (!usesql)
	{
		g_signal_connect (GTK_OBJECT (find_poi_bt), "clicked",
			GTK_SIGNAL_FUNC (sel_target_cb), (gpointer) 2);
	}
	else
	{
		g_signal_connect (GTK_OBJECT (find_poi_bt), "clicked",
			GTK_SIGNAL_FUNC (poi_lookup_cb), (gpointer) 2);
	}
	gtk_box_pack_start (GTK_BOX (vbox_buttons),
		find_poi_bt, FALSE, FALSE, 1 * PADDING);

	}	/* END MENU AND BUTTONS */

	/* WAYPOINTS AND POIs */
	{
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: WAYPOINTS AND POIs)\n");

	frame_poi = gtk_frame_new (_("Points"));
	vbox_poi = gtk_vbox_new (TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_poi), vbox_poi);

	/* Checkbox: POI Draw */
	if (usesql)
	{
		poi_draw_bt = gtk_check_button_new_with_label (_("POI"));
		if (local_config.showpoi)
		{
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (poi_draw_bt), TRUE);
		}
		gtk_box_pack_start (GTK_BOX (vbox_poi),
			poi_draw_bt, FALSE, FALSE, 0 * PADDING);
		g_signal_connect (GTK_OBJECT (poi_draw_bt), "clicked",
			GTK_SIGNAL_FUNC (toggle_button_cb),
			&local_config.showpoi);
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), poi_draw_bt,
			_("Show Points Of Interest found in Database"), NULL);
	}
	
	/* Checkbox: WLAN Draw */
	wlan_draw_bt = gtk_check_button_new_with_label (_("WLAN"));
	if (local_config.showwlan)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (wlan_draw_bt), TRUE);
	}
	if (usesql)
	{
		gtk_box_pack_start (GTK_BOX (vbox_poi),
			wlan_draw_bt, FALSE, FALSE, 0 * PADDING);
	}
	g_signal_connect (GTK_OBJECT (wlan_draw_bt), "clicked",
		GTK_SIGNAL_FUNC (toggle_button_cb),
		&local_config.showwlan);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), wlan_draw_bt,
		_("Show Data found in Kismet Database"), NULL);

	/* Checkbox: Draw Waypoints from file */
	wp_draw_bt = gtk_check_button_new_with_label (_("WP"));
	if (local_config.showwaypoints)
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (wp_draw_bt), TRUE);
	g_signal_connect (GTK_OBJECT (wp_draw_bt), "clicked",
		GTK_SIGNAL_FUNC (toggle_button_cb),
		&local_config.showwaypoints);
	gtk_box_pack_start (GTK_BOX (vbox_poi), wp_draw_bt,
		FALSE, FALSE, 0 * PADDING);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), wlan_draw_bt,
		_("Show Waypoints found in way.txt file"), NULL);

	}	/* END WAYPOINTS AND POIs */

	/* TRACKS */
	{
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: TRACKS)\n");
	frame_track = gtk_frame_new (_("Track"));
	vbox_track = gtk_vbox_new (TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_track), vbox_track);
	
	/* Checkbox: Show Track */
	showtrack_bt = gtk_check_button_new_with_label (_("Show"));
	if (local_config.showtrack)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (showtrack_bt), TRUE);
	}
	g_signal_connect (GTK_OBJECT (showtrack_bt), "clicked",
		GTK_SIGNAL_FUNC (toggle_button_cb), &local_config.showtrack);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), showtrack_bt,
		_("Show tracking on the map"), NULL);
	gtk_box_pack_start (GTK_BOX (vbox_track), showtrack_bt,
		FALSE, FALSE, 0 * PADDING);

	/* Checkbox: Save Track */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: Save Track)\n");
	savetrack_bt = gtk_check_button_new_with_label (_("Save"));
	if (local_config.savetrack)
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (savetrack_bt), TRUE);
	g_signal_connect (GTK_OBJECT (savetrack_bt), "clicked",
		GTK_SIGNAL_FUNC (toggle_button_cb), &local_config.savetrack);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), savetrack_bt,
		_("Save the track to given filename at program exit"),
		NULL);
	gtk_box_pack_start (GTK_BOX (vbox_track), savetrack_bt,
		FALSE, FALSE,0 * PADDING);
	}	/* END TRACKS */

	/* MAP CONTROL */
	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox(Bottons: MAP CONTROL)\n");
	{
		frame_mapcontrol = make_display_map_controls ();
	}	/* END MAP CONTROL */
	
	/* MAP TYPE */
	{
	        if ( mydebug > 11 )
			fprintf(stderr,"create_controls_mainbox(Bottons: MAP TYPE)\n");
		frame_maptype = make_display_map_checkboxes();
	}	/* END MAP TYPE */


	if (local_config.guimode == GUI_PDA)
	{
		mainbox_controls = gtk_hbox_new (TRUE, 0 * PADDING);
		
		pda_box_left = gtk_vbox_new (FALSE, 1 * PADDING);
		pda_box_right = gtk_vbox_new (FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),	pda_box_left, TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),	pda_box_right, TRUE, TRUE, 0);

		gtk_box_pack_start (GTK_BOX (pda_box_left), vbox_buttons, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (pda_box_left), frame_poi, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (pda_box_left), frame_track, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (pda_box_right),frame_mapcontrol, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (pda_box_right),frame_maptype, TRUE, TRUE, 1 * PADDING);
	}
	else
	if (local_config.guimode == GUI_CAR)
	{
		mainbox_controls = gtk_vbox_new (FALSE, 0 * PADDING);

		gtk_box_pack_start (GTK_BOX (mainbox_controls),vbox_buttons,	FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),frame_poi,	FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),frame_track, 	FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),frame_mapcontrol, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),frame_maptype, 	TRUE, TRUE, 1 * PADDING);
	}
	else
	{
		mainbox_controls = gtk_vbox_new (FALSE, 0 * PADDING);

		gtk_box_pack_start (GTK_BOX (mainbox_controls),	vbox_buttons, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),	frame_poi, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),	frame_track, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),	frame_mapcontrol, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls),	frame_maptype, TRUE, TRUE, 1 * PADDING);
	}

	if ( mydebug > 11 )
	    fprintf(stderr,"create_controls_mainbox: END\n");
}


/* *****************************************************************************
 * Window: Main -> Status
 */
void create_status_mainbox (void)
{

	GtkWidget *statusdashboard_box;
	GtkWidget *statusdashsub1_box, *statusdashsub2_box;
	GtkWidget *statussmall_box, *frame_statustime;
	GtkWidget *frame_statusbearing, *frame_statusheading;
	GtkWidget *frame_statuslat, *frame_statuslon;
	GtkWidget *eventbox_statuslat, *eventbox_statuslon;
	GtkWidget *frame_statusmapscale, *frame_statusprefscale;
	GtkWidget *statusbar_box, *frame_statusgpsfix;
	GtkWidget *dashboard_1_lb, *dashboard_2_lb;
	GtkWidget *dashboard_3_lb, *dashboard_4_lb;
	GtkWidget *frame_compass, *frame_minimap;

	gchar sc[15];
	gint scaler_pos= 0;
	gint scale = 0;

	if ( mydebug > 11 )
	    fprintf(stderr,"create_status_mainbox\n");

	mainbox_status = gtk_vbox_new (FALSE, 0 * PADDING);


	/* DASHBOARD */
	{
		/* Frame Mini Map */
		frame_minimap = gtk_frame_new (NULL);	
		gtk_frame_set_shadow_type
			(GTK_FRAME (frame_minimap), GTK_SHADOW_NONE);
		drawing_minimap = gtk_drawing_area_new ();
		gtk_widget_set_size_request (drawing_minimap, 128, 103);
		g_signal_connect (drawing_minimap, "expose_event",
			GTK_SIGNAL_FUNC (expose_mini_cb), NULL);
		gtk_widget_add_events (GTK_WIDGET (drawing_minimap),
			GDK_BUTTON_PRESS_MASK);
		g_signal_connect_swapped (drawing_minimap,
			"button-press-event",
			GTK_SIGNAL_FUNC (minimapclick_cb),
			GTK_OBJECT (drawing_minimap));
		gtk_container_add (GTK_CONTAINER (frame_minimap),
			drawing_minimap);
		drawable_minimap = drawing_minimap->window;

		/* Frame Compass */
		frame_compass = gtk_frame_new (NULL);	
		drawing_compass = gtk_drawing_area_new ();
		gtk_widget_set_size_request (drawing_compass, 100, 100);
		gtk_container_add (GTK_CONTAINER (frame_compass),
			drawing_compass);
		g_signal_connect (GTK_OBJECT (drawing_compass),
			"expose_event", GTK_SIGNAL_FUNC (expose_compass),
			NULL);

		/* Frame Dashboard 1 */
		frame_dash_1 = gtk_frame_new (" = 1 = ");	
		dashboard_1_lb = gtk_label_new ("---");
		gtk_container_add (GTK_CONTAINER (frame_dash_1),
			dashboard_1_lb);

		/* Frame Dashboard 2 */
		frame_dash_2 = gtk_frame_new (" = 2 = ");	
		dashboard_2_lb = gtk_label_new ("---");
		gtk_container_add (GTK_CONTAINER (frame_dash_2),
			dashboard_2_lb);
	
		/* Frame_Dashboard 3 */
		frame_dash_3 = gtk_frame_new (" = 3 = ");	
		dashboard_3_lb = gtk_label_new ("---");
		gtk_container_add (GTK_CONTAINER (frame_dash_3),
			dashboard_3_lb);
	
		/* Frame_Dashboard 4 */
		frame_dash_4 = gtk_frame_new (" = 4 = ");	
		dashboard_4_lb = gtk_label_new ("---");
		gtk_container_add (GTK_CONTAINER (frame_dash_4),
			dashboard_4_lb);

	}	/* END DASHBOARD */


	/* SMALL STATUS TEXT */
	{
		/* Time */
		frame_statustime = gtk_frame_new (_("GPS-Time"));
		statustime_lb = gtk_label_new ("00:00");
		gtk_container_add (GTK_CONTAINER (frame_statustime),
			statustime_lb);
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips),
			frame_statustime,
			_("This shows the time from your GPS receiver"), NULL);

		/* Friends */
		frame_statusfriends = gtk_frame_new (_("Friends"));
		statusfriends_lb = gtk_label_new ("0/0");
		gtk_container_add (GTK_CONTAINER (frame_statusfriends),
			statusfriends_lb);
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips),
			frame_statusfriends,
			_("Number of mobile targets within timeframe/total"
			" received from friendsserver"), NULL);

		/* Bearing */
		frame_statusbearing = gtk_frame_new (_("Bearing"));
		statusbearing_lb = gtk_label_new ("0000");
		gtk_container_add (GTK_CONTAINER (frame_statusbearing),
			statusbearing_lb);

		/* Heading */
		frame_statusheading = gtk_frame_new (_("Heading"));
		statusheading_lb = gtk_label_new ("0000");
		gtk_container_add (GTK_CONTAINER (frame_statusheading),
			statusheading_lb);

		/* Latitude */
		frame_statuslat = gtk_frame_new (_("Latitude"));
		statuslat_lb = gtk_label_new ("00,00000N");
		eventbox_statuslat = gtk_event_box_new ();
		gtk_widget_add_events (eventbox_statuslat,
			GDK_BUTTON_PRESS_MASK);
		g_signal_connect (eventbox_statuslat, "button_press_event",
			GTK_SIGNAL_FUNC (toggle_coords_cb), NULL);
		gtk_container_add (GTK_CONTAINER (eventbox_statuslat),
			statuslat_lb);
		gtk_container_add (GTK_CONTAINER (frame_statuslat),
			eventbox_statuslat);

		/* Longitude */
		frame_statuslon = gtk_frame_new (_("Longitude"));
		statuslon_lb = gtk_label_new ("000,00000E");
		eventbox_statuslon = gtk_event_box_new ();
		gtk_widget_add_events (eventbox_statuslon,
			GDK_BUTTON_PRESS_MASK);
		g_signal_connect (eventbox_statuslon, "button_press_event",
			GTK_SIGNAL_FUNC (toggle_coords_cb), NULL);
		gtk_container_add (GTK_CONTAINER (eventbox_statuslon),
			statuslon_lb);
		gtk_container_add (GTK_CONTAINER (frame_statuslon),
			eventbox_statuslon);

		/* Map Scale */
		frame_statusmapscale = gtk_frame_new (_("Map scale"));
		statusmapscale_lb = gtk_label_new ("---");
		gtk_container_add (GTK_CONTAINER (frame_statusmapscale),
			statusmapscale_lb);

		/* Preferred Scale */
		frame_statusprefscale = gtk_frame_new (_("Pref. scale"));
		g_snprintf (sc, sizeof (sc), "1:%d", local_config.scale_wanted);
		statusprefscale_lb = gtk_label_new (sc);
		gtk_container_add (GTK_CONTAINER (frame_statusprefscale),
			statusprefscale_lb);

		/* GPS Fix Status */
		frame_statusgpsfix = gtk_frame_new (_("GPS Status"));
		drawing_gpsfix = gtk_drawing_area_new ();
		gtk_widget_set_size_request (drawing_gpsfix,
			100, -1);
		gtk_container_add (GTK_CONTAINER (frame_statusgpsfix),
			drawing_gpsfix);
		g_signal_connect (GTK_OBJECT (drawing_gpsfix),
			"expose_event", GTK_SIGNAL_FUNC (expose_gpsfix),
			NULL);
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips),
			frame_statusgpsfix,
			_("This shows the GPS Status and Number of satellites"
			" in use."), NULL);
	}	/* SMALL STATUS TEXT */


	/* STATUS BAR AND SCALE SLIDER */
	{
		/* Status Bar */
		frame_statusbar = gtk_statusbar_new ();
		gtk_statusbar_set_has_resize_grip
			(GTK_STATUSBAR (frame_statusbar), FALSE);
		current.statusbar_id = gtk_statusbar_get_context_id
			(GTK_STATUSBAR (frame_statusbar), "main");
		gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
			current.statusbar_id,
			_("GpsDrive (c)2001-2007 F. Ganter"));

		/* Map Scale Slider */
		/* search which scaler_pos is fitting scale_wanted */
		while ( (local_config.scale_wanted > scale )
			&& (scaler_pos <= slistsize ) )
		{
			scaler_pos++;
			scale = nlist[(gint) rint (scaler_pos)];
		}
		mapscaler_adj = gtk_adjustment_new
			(scaler_pos, 0, slistsize - 1, 1,
			slistsize / 4, 1 / slistsize );
		mapscaler_scaler = gtk_hscale_new
			(GTK_ADJUSTMENT (mapscaler_adj));
		g_signal_connect (GTK_OBJECT (mapscaler_adj), "value_changed",
			GTK_SIGNAL_FUNC (scaler_cb), NULL);
		gtk_scale_set_draw_value (GTK_SCALE (mapscaler_scaler), FALSE);
	}	/* END STATUS BAR AND SCALE SLIDER */
	
	/* Pack all the widgets together according to GUI-Mode */
	if (local_config.guimode == GUI_PDA)
	{
		statusdashboard_box = gtk_table_new (2, 2, TRUE);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box), frame_dash_1, 0, 1, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box), frame_dash_2, 1, 2, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box), frame_dash_3, 0, 1, 1, 2);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box), frame_dash_4, 1, 2, 1, 2);

		statussmall_box = gtk_vbox_new (FALSE, PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslat, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslon, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusgpsfix, TRUE, TRUE, 1 * PADDING);

		statusbar_box = gtk_hbox_new (TRUE, PADDING);
		gtk_box_pack_start (GTK_BOX (statusbar_box), frame_compass, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusbar_box), statussmall_box, TRUE, TRUE, 1 * PADDING);

		gtk_box_pack_start (GTK_BOX (mainbox_status), statusdashboard_box, TRUE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status), statusbar_box, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status), frame_statusbar, TRUE, FALSE, 1* PADDING);
	}
	else
	if (local_config.guimode == GUI_CAR)
	{
		statusdashboard_box = gtk_hbox_new (FALSE, PADDING);
		statusdashsub1_box = gtk_hbox_new (FALSE, PADDING);
		statusdashsub2_box = gtk_hbox_new (TRUE, PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_controls), frame_compass, FALSE, FALSE, 1 * PADDING);
		// gtk_box_pack_start (GTK_BOX (mainbox_controls), frame_minimap, FALSE, FALSE, 1 * PADDING);

		gtk_box_pack_start (GTK_BOX (statusdashsub2_box), frame_dash_1, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashsub2_box), frame_dash_2, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashsub2_box), frame_dash_3, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashboard_box), statusdashsub1_box, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (statusdashboard_box), statusdashsub2_box, TRUE, TRUE, 0);

		// --- ACPI / Temperature / Battery
		create_temperature_widget(statusdashboard_box);
		create_battery_widget(statusdashboard_box);

		statussmall_box = gtk_hbox_new (FALSE, PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statustime, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusfriends, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusbearing, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusheading, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslat, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslon, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusmapscale, TRUE, TRUE, 1 * PADDING);
		//gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusprefscale, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusgpsfix, TRUE, TRUE, 1 * PADDING);

		statusbar_box = gtk_hbox_new (FALSE, PADDING);
		gtk_box_pack_start (GTK_BOX (statusbar_box), frame_statusbar, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusbar_box), mapscaler_scaler, TRUE, TRUE, 1 * PADDING);
	
		gtk_box_pack_start (GTK_BOX (mainbox_status), statusdashboard_box, TRUE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status), statussmall_box, TRUE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status), statusbar_box, TRUE, FALSE, 1 * PADDING);
	} else
	{
		statusdashboard_box = gtk_hbox_new (FALSE, PADDING);
		statusdashsub1_box = gtk_hbox_new (FALSE, PADDING);
		statusdashsub2_box = gtk_hbox_new (TRUE, PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashsub1_box),frame_minimap, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashsub1_box),frame_compass, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashsub2_box),frame_dash_1, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashsub2_box),frame_dash_2, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashsub2_box),frame_dash_3, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusdashboard_box),statusdashsub1_box, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (statusdashboard_box),statusdashsub2_box, TRUE, TRUE, 0);


		//  --- ACPI / Temperature / Battery
		create_temperature_widget(statusdashboard_box);
		create_battery_widget(statusdashboard_box);

		statussmall_box = gtk_hbox_new (FALSE, PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statustime, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusfriends, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusbearing, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusheading, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslat, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslon, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusmapscale, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusprefscale, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusgpsfix, TRUE, TRUE, 1 * PADDING);

		statusbar_box = gtk_hbox_new (FALSE, PADDING);
		gtk_box_pack_start (GTK_BOX (statusbar_box), frame_statusbar, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusbar_box), mapscaler_scaler, TRUE, TRUE, 1 * PADDING);
	
		gtk_box_pack_start (GTK_BOX (mainbox_status),statusdashboard_box, TRUE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status),statussmall_box, TRUE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status),statusbar_box, TRUE, FALSE, 1 * PADDING);
	}
}


/* *****************************************************************************
 * Window: Main -> Map
 */
void create_map_mainbox (void)
{
	if ( mydebug > 11 )
	    fprintf(stderr,"create_map_mainbox\n");

	mainframe_map =  gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (mainframe_map), GTK_SHADOW_IN);
	map_drawingarea = gtk_drawing_area_new ();
	gtk_widget_set_size_request (map_drawingarea, SCREEN_X, SCREEN_Y);
	gtk_container_add (GTK_CONTAINER (mainframe_map), map_drawingarea);

	gtk_widget_add_events (GTK_WIDGET (map_drawingarea),
		GDK_BUTTON_PRESS_MASK);
	g_signal_connect_swapped (GTK_OBJECT (map_drawingarea), 
		"button-press-event", GTK_SIGNAL_FUNC (mapclick_cb),
		GTK_OBJECT (map_drawingarea));
	g_signal_connect_swapped (GTK_OBJECT (map_drawingarea),
		"scroll_event", GTK_SIGNAL_FUNC (mapscroll_cb),
		GTK_OBJECT (map_drawingarea));
}


/* *****************************************************************************
 * Window: Main
 */
gint create_main_window (gchar *geom, gint *usegeom)
{
	gchar main_title[100];
	GdkPixbuf *mainwindow_icon_pixbuf;

	if ( mydebug > 11 )
	    fprintf(stderr,"create_main_window\n");

	main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	g_snprintf (main_title, sizeof (main_title),
		"%s v%s", "GpsDrive", VERSION);

	gtk_window_set_title (GTK_WINDOW (main_window), main_title);
	gtk_container_set_border_width (GTK_CONTAINER (main_window), 0);
	gtk_window_set_position (GTK_WINDOW (main_window),
		GTK_WIN_POS_CENTER);
	g_signal_connect (GTK_OBJECT (main_window), "delete_event",
		GTK_SIGNAL_FUNC (quit_program_cb), NULL);
	g_signal_connect (GTK_OBJECT (main_window), "key_press_event",
		GTK_SIGNAL_FUNC (key_pressed_cb), NULL);

	mainwindow_icon_pixbuf = read_icon ("gpsicon.png",1);
	if (mainwindow_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (main_window),
			mainwindow_icon_pixbuf);
		gdk_pixbuf_unref (mainwindow_icon_pixbuf);
	}

	main_tooltips = gtk_tooltips_new();

	/* Create the three parts of the main window */
	create_controls_mainbox ();
	create_status_mainbox ();
	create_map_mainbox ();

	if (local_config.guimode == GUI_PDA)
	{  /* PDA Mode */
		GtkWidget *map_lb, *controls_lb, *status_lb;
		
		main_table = gtk_notebook_new ();
		
		map_lb = gtk_label_new (_("Map"));
		gtk_notebook_append_page (GTK_NOTEBOOK (main_table),
			mainframe_map, map_lb);
		controls_lb = gtk_label_new (_("Controls"));
		gtk_notebook_append_page (GTK_NOTEBOOK (main_table),
			mainbox_controls, controls_lb);
		status_lb = gtk_label_new (_("Status"));
		gtk_notebook_append_page (GTK_NOTEBOOK (main_table),
			mainbox_status, status_lb);
		gtk_container_add (GTK_CONTAINER (main_window), main_table);
	}
	else if (local_config.guimode == GUI_XWIN)
	{  /* X-Win Mode (separate windows) */
		// TODO: ...
	}
	else
	{  /* Classic Mode (Standard) */
		main_table = gtk_table_new (4, 2, FALSE);
		gtk_table_attach_defaults (GTK_TABLE (main_table),
			mainbox_controls, 0, 1, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (main_table),
			mainframe_map, 1, 2, 0, 1);		
		gtk_table_attach_defaults (GTK_TABLE (main_table),
			mainbox_status, 0, 2, 2, 3);		
		gtk_container_add (GTK_CONTAINER (main_window), main_table);

	}
	
	gtk_widget_show_all (main_window);

	if ( ( local_config.guimode == GUI_CLASSIC )
	     && (local_config.MapnikStatusInt ) ){
	    toggle_mapnik_cb( mapnik_bt, 2 );
	}
	
	if ( !local_config.showfriends)
	    gtk_widget_hide_all (frame_statusfriends);

	return 0;
}

