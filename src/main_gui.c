/*
**********************************************************************

Copyright (c) 2001-2007 Fritz Ganter <ganter@ganter.at>
Copyright (c) 2007 Guenther Meyer <d.s.e (at) sordidmusic.com>

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
#include "config.h"
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
#include <gdk/gdkkeysyms.h>
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
#include "map_download.h"
#include "main_gui.h"
#include "poi.h"
#include "routes.h"
#include "track.h"
#include "gpx.h"
#include "gps_handler.h"
#include "waypoint.h"


#ifdef MAEMO

#include <hildon/hildon-program.h>
HildonProgram *program;

#endif


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
extern tripdata_struct trip;
extern int actualfriends, maxfriends;
extern gint mydebug;
extern gint debug;
extern gint iszoomed, xoff, yoff;

extern gdouble wp_saved_target_lat;
extern gdouble wp_saved_target_lon;
extern gdouble wp_saved_expmode_lat;
extern gdouble wp_saved_expmode_lon;
extern gdouble pixelfact;
extern gps_satellite_struct *gps_sats;

extern gint slistsize, nlist[];
extern gint PSIZE;
extern GtkWidget *bestmap_bt;
extern gint borderlimit;
extern gint max_display_map;
extern map_dir_struct *display_map;

extern GtkWidget *main_window;
extern GtkToolItem *explore_bt;

/* Globally accessible widgets: */
GtkWidget *map_drawingarea;
GtkWidget *frame_statusbar, *frame_statusfriends;
GtkWidget *main_table;
GtkWidget *menuitem_sendmsg;
GtkWidget *wp_draw_bt;
GtkToolItem *mute_bt, *routing_bt;
GtkToolItem *find_poi_bt;
GtkWidget *menuitem_saveroute;
GtkTooltips *main_tooltips;
GtkWidget *routeinfo_evbox, *routeinfo_icon, *routeinfo_label;
GtkWidget *main_toolbar;

// TODO: maybe these should be moved to local ones...
GtkWidget *drawing_compass, *drawing_minimap, *drawing_sats;
GdkDrawable *drawable_compass, *drawable_minimap;
GdkGC *kontext_compass, *kontext_gps, *kontext_minimap, *kontext_map;


/* local widgets */
static GtkWidget *mainbox_status, *mainframe_map;
static GtkWidget *drawing_gpsfix;
static GtkWidget *frame_maptype;
static GtkWidget *mapscaler_scaler, *mapscaler_hbox;
static GtkToolItem *zoomin_bt, *zoomout_bt;
static GtkToolItem *scaler_in_bt, *scaler_out_bt;
static GtkToolItem *options_bt, *mapcontrol_bt;
static GtkToolItem *trk_clear_bt, *trk_restart_bt;
static GtkToolItem *addwaypoint_bt;
static GtkWidget *statusprefscale_lb, *statusmapscale_lb;
static GtkObject *mapscaler_adj;
static GtkWidget *frame_dash_1, *frame_dash_2, *frame_dash_3, *frame_dash_4;
static GtkWidget *statusfriends_lb, *statustime_lb;
static GtkWidget *statuslat_lb, *statuslon_lb;
static GtkWidget *statuscourse_lb, *statusbearing_lb;
static GtkWidget *hbox_zoom, *dash_menu, *dash_menu_window;
static GtkWidget *controlbox_window;
static GtkWidget *satellite_window;
static GtkWidget *minimap_window;
static GtkWidget *statusbar_box, *statussmall_box;
static GtkWidget *statusdashboard_box, *compass_box;
static gboolean menu_expand = FALSE;

	GtkAdjustment *adj_h, *adj_v;


/* Definitions for main menu */
enum
{
	MENU_MAPIMPORT,
	MENU_MAPDOWNLOAD,
	MENU_LOADTRKOLD,
	MENU_LOADTRACK,
	MENU_LOADROUTE,
	MENU_LOADWPT,
	MENU_SAVETRACK,
	MENU_SAVEROUTE,
	MENU_SENDMSG,
	MENU_SETTINGS,
	MENU_HELPABOUT,
	MENU_HELPCONTENT,
	MENU_TRIPRESET,
	MENU_N_ITEMS
};

/* *****************************************************************************
 * CALLBACKS
 * *****************************************************************************/


/* *****************************************************************************
 * popup menu for dashoard mode selection 
 */
gint
dash_menu_cb (GtkWidget *widget, GdkEventButton *event, gint dash_nr)
{
	if (mydebug > 20)
		fprintf (stderr, "dash_menu_cb: Dashboard %d clicked\n",
		dash_nr);

	local_config.dashboard[0] = dash_nr;

	gtk_menu_popup (GTK_MENU (dash_menu), NULL, NULL, NULL, NULL,
		event->button, event->time);


	return TRUE;
}


/* *****************************************************************************
 * enable options menu button again, when closing menu
 */
gint
close_options_menu_cb (GtkWidget *widget, gpointer data)
{
	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (options_bt), FALSE);
}


/* *****************************************************************************
 * track state of main window
 */
gboolean
window_state_cb  (GtkWidget *window, GdkEventWindowState *event, gpointer data)
{
	current.window_state = 	event->new_window_state;
	return TRUE;
}


/* *****************************************************************************
 * popup menu for dashoard mode selection (Car Mode)
 */
gint
dash_carmenu_cb (GtkWidget *widget, GdkEventButton *event, gint dash_nr)
{
	if (mydebug > 20)
		fprintf (stderr, "dash_carmenu_cb: Dashboard %d clicked\n", dash_nr);

	local_config.dashboard[0] = dash_nr;
	//gtk_window_set_decorated (GTK_WINDOW (dash_menu_window), FALSE);
	gtk_widget_show_all (dash_menu_window);

	return TRUE;
}


/* *****************************************************************************
 * select display mode for the selected dashboard 
 */
gint
dash_select_cb (GtkWidget *item, gint selection)
{
	if (local_config.dashboard[0])
	{
		local_config.dashboard[local_config.dashboard[0]] = selection;
		current.needtosave = TRUE;
		local_config.dashboard[0] = 0;
#ifndef MAEMO
		if (local_config.guimode == GUI_CAR)
#endif
			gtk_widget_hide_all (dash_menu_window);

		if (mydebug > 20)
		{
			fprintf (stderr,
				"dash_select_cb: Dashboard: %d : Mode %d\n",
				local_config.dashboard[0], selection);
		}
	}

	return TRUE;
}


/* *****************************************************************************
 * select map to display
 */
gint
display_maps_cb (GtkWidget *widget, guint datum)
{
	gint i;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (display_map[datum].checkbox)))
		display_map[datum].to_be_displayed = TRUE;
	else
		display_map[datum].to_be_displayed = FALSE;

	for (i = 0; i < max_display_map; i++)
    {
		gchar tbd = display_map[i].to_be_displayed ? 'D' : '_';
		g_print ("Found %s,%c\n", display_map[i].name, tbd);
    }

	current.needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 * select which pois should be drawn on the map
 */
gint
poi_button_cb (GtkWidget *button, gint type)
{
	gint t_val;

	t_val = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

	if (type == POIDRAW_USER)
	{
		if (local_config.showpoi == POIDRAW_NONE && t_val)
			local_config.showpoi = POIDRAW_USER;
		if (local_config.showpoi == POIDRAW_OSM && t_val)
			local_config.showpoi = POIDRAW_ALL;
		if (local_config.showpoi == POIDRAW_USER && !t_val)
			local_config.showpoi = POIDRAW_NONE;
		if (local_config.showpoi == POIDRAW_ALL && !t_val)
			local_config.showpoi = POIDRAW_OSM;
	}

	if (type == POIDRAW_OSM)
	{
		if (local_config.showpoi == POIDRAW_NONE && t_val)
			local_config.showpoi = POIDRAW_OSM;
		if (local_config.showpoi == POIDRAW_USER && t_val)
			local_config.showpoi = POIDRAW_ALL;
		if (local_config.showpoi == POIDRAW_OSM && !t_val)
			local_config.showpoi = POIDRAW_NONE;
		if (local_config.showpoi == POIDRAW_ALL && !t_val)
			local_config.showpoi = POIDRAW_USER;
	}

	if (mydebug > 20)
		g_print ("poi_button_cb (): Setting showpoi to %d\n", local_config.showpoi);

	poi_draw_list (TRUE);
	current.needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 * quit the program 
 */
gint
quit_program_cb (GtkWidget *widget, gpointer datum)
{
    gtk_main_quit ();
    return TRUE;
}


/* *****************************************************************************
 * rebuild the main toolbar
 */
void rebuild_toolbar_mainbox_cb()
{
	gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), options_bt, 0);
	if (local_config.showbutton_map)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), mapcontrol_bt, -1);
	if (local_config.showbutton_zoom)
	{
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), zoomout_bt, -1);
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), zoomin_bt, -1);
	}
	if (local_config.showbutton_scaler)
	{
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), scaler_out_bt, -1);
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), scaler_in_bt, -1);
	}
	if (local_config.showbutton_explore)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), explore_bt, -1);
	if (local_config.showbutton_find)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), find_poi_bt, -1);
	if (local_config.showbutton_addwpt)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), addwaypoint_bt, -1);
	if (local_config.showbutton_trackclear)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), trk_clear_bt, -1);
	if (local_config.showbutton_trackrestart)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), trk_restart_bt, -1);
	if (local_config.showbutton_route)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), routing_bt, -1);
	if (local_config.showbutton_mute)
		gtk_toolbar_insert (GTK_TOOLBAR (main_toolbar), mute_bt, -1);
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
		if (local_config.guimode == GUI_CAR)
			gtk_widget_set_sensitive (frame_maptype, FALSE);
		else
			gtk_widget_hide_all (frame_maptype);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmap_bt), FALSE);
		gtk_toggle_button_set_active  (GTK_TOGGLE_BUTTON (widget), TRUE);
		autobestmap_cb(bestmap_bt,0);
	}
	else 
	{
		local_config.MapnikStatusInt = 0;
		if (local_config.guimode == GUI_CAR)
			gtk_widget_set_sensitive (frame_maptype, TRUE);
		else
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
 *  toggle map control box
 */
gint
toggle_controlbox_cb (GtkWidget *widget, gint show)
{
	if (show == 1)
	{
		gtk_widget_show_all (controlbox_window);
		gtk_widget_set_sensitive (GTK_WIDGET (mapcontrol_bt), FALSE);
	}	
	else
	{
		gtk_widget_hide_all (controlbox_window);
		gtk_widget_set_sensitive (GTK_WIDGET (mapcontrol_bt), TRUE);
	}
	return TRUE;
}


/* *****************************************************************************
 *  toggle satellite info window
 */
gint
toggle_satelliteinfo_cb (GtkWidget *widget, GdkEventButton *event, gboolean datum)
{
	if (datum)
		gtk_widget_show_all (satellite_window);
	else
		gtk_widget_hide_all (satellite_window);
	return TRUE;
}


/* *****************************************************************************
 *  show/hide main menu
 */
gint
toggle_main_menu_cb (GtkToggleToolButton *button, GtkMenu *menu)
{
	if (gtk_toggle_tool_button_get_active (button))
		gtk_menu_popup (menu, NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time ());
	else
	{
		gtk_menu_popdown (menu);
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (options_bt), FALSE);
	}
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
//		case MENU_MAPDOWNLOAD:	download_cb (NULL, 0); break;
		case MENU_MAPDOWNLOAD:	map_download_cb (NULL, 0); break;
		case MENU_LOADTRKOLD:	loadtrack_cb (NULL, 0); break;
		case MENU_LOADTRACK:	loadgpx_cb (GPX_TRK); break;
		case MENU_LOADROUTE:	loadgpx_cb (GPX_RTE); break;
		case MENU_LOADWPT:	loadgpx_cb (GPX_WPT); break;
		case MENU_SAVETRACK:	savegpx_cb (GPX_TRK); break;
		case MENU_SAVEROUTE:	savegpx_cb (GPX_RTE); break;
		case MENU_SENDMSG:	sel_message_cb (NULL, 0); break;
		case MENU_SETTINGS:	settings_main_cb (NULL, 0); break;
		case MENU_HELPABOUT:	about_cb (NULL, 0); break;
		case MENU_HELPCONTENT:	help_cb (NULL, 0); break;
		case MENU_TRIPRESET:	trip_reset_cb (); break;
	}

	gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (options_bt), FALSE);

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
		gtk_widget_set_sensitive (GTK_WIDGET (scaler_out_bt), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (scaler_in_bt), FALSE);
		gtk_label_set_text (GTK_LABEL (statusprefscale_lb), _("Auto"));
		if (mapscaler_scaler)
			gtk_widget_set_sensitive (mapscaler_scaler, FALSE);
		local_config.autobestmap = TRUE;
	}
	else
	{
		gtk_widget_set_sensitive (GTK_WIDGET (scaler_out_bt), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (scaler_in_bt), TRUE);
		g_snprintf (sc, sizeof (sc), "1:%d", local_config.scale_wanted);
		gtk_label_set_text (GTK_LABEL (statusprefscale_lb), sc);
		if (mapscaler_scaler)
			gtk_widget_set_sensitive (mapscaler_scaler, TRUE);
		local_config.autobestmap = FALSE;
	}

	current.needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 */
gint
showminimap_cb (GtkWidget *widget, guint datum)
{
	if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) )
	{
		gtk_widget_show_all (minimap_window);
		local_config.showminimap = TRUE;
	}
	else
	{
		gtk_widget_hide_all (minimap_window);
		local_config.showminimap = FALSE;
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
	    if (gui_status.expmode)
		{
		    coords.expmode_lon = lon;
		    coords.expmode_lat = lat;
		    rebuildtracklist ();
		}
	}
	/*  Middle mouse button */
	if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
	{
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (explore_bt), FALSE);
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
		gtk_widget_set_sensitive (GTK_WIDGET (zoomout_bt), FALSE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (zoomout_bt), TRUE);
	if (current.zoom == ZOOM_MAX)
		gtk_widget_set_sensitive (GTK_WIDGET (zoomin_bt), FALSE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (zoomin_bt), TRUE);

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
 * TODO: Improve finding of next appropriate map
 * datum:
 *    1 Zoom out
 *    2 Zoom in 
 */
gint
scalerbt_cb (GtkWidget *widget, guint datum)
{
	if (GTK_IS_OBJECT (mapscaler_adj))
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
	}

	// TODO: check, if this is really necessary, and maybe remove...
	expose_cb (NULL, 0);
	expose_mini_cb (NULL, 0);

	current.needtosave = TRUE;

	return TRUE;
}


/*******************************************************************************
 * Draw the type of the current map on the screen.
 *  When mapnik live rendering is used, nothing will be drawn.
 */
void
draw_maptype (void)
{
	PangoFontDescription *pfd;
	PangoLayout *layout_filename;
	gint cx, cy;

	layout_filename = gtk_widget_create_pango_layout
		(map_drawingarea, current.maptype);
	pfd = pango_font_description_from_string ("Sans 9");
	pango_layout_set_font_description (layout_filename, pfd);
	pango_layout_get_pixel_size (layout_filename, &cx, &cy);

	gdk_gc_set_function (kontext_map, GDK_OR);
	gdk_gc_set_foreground (kontext_map, &colors.mygray);
	gdk_draw_rectangle (drawable, kontext_map, 1,
		0, 0, cx, cy);
	gdk_gc_set_function (kontext_map, GDK_COPY);
	gdk_gc_set_foreground (kontext_map, &colors.blue);

	gdk_draw_layout_with_colors (drawable, kontext_map,
		0, 0, layout_filename, &colors.blue, NULL);
	if (layout_filename != NULL)
		g_object_unref (G_OBJECT (layout_filename));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);
}


/* *****************************************************************************
 * Update Contents of Dashboard fields
 *  the first parameter chooses one of the dashboard fields to update
 *  the second parameter chooses, which value should be dispayed there
 * Possible Values:
 *	DSH_DIST, DASH_TIMEREMAIN, DASH_BEARING, DASH_TURN, DASH_SPEED,
 *	DASH_COURSE, DASH_ALT, DASH_TRIP, DASH_GPSPRECISION, DASH_TIME,
 *	DASH_SPEED_AVG, DASH_SPEED_MAX, DASH_POSITION, DASH_MAPSCALE,
 *	DASH_DESTINATION, DASH_XTE
 */
gint
update_dashboard (GtkWidget *frame, gint source)
{
	gchar head[100], content[200], ctmp[10], unit[10], dirs = ' ';
	gchar turn_color[10];
	gint fontsize_unit;

	gdouble dir = 0.0; /* for DASH_TURN */

	fontsize_unit = atoi (g_strrstr_len (local_config.font_dashboard,
		100, " ")) * 512;
	if (fontsize_unit < 6144)
		fontsize_unit = 6144;

	/* cleanly future-proof if gpsdriverc holds a new item */
	if (source > DASH_N_ITEMS)
		source = DASH_POSITION;


	switch (source)
	{
		case DASH_DIST:
		{
			g_strlcpy (head, _("Distance"), sizeof (head));
			distance2gchar (current.dist, ctmp, sizeof (ctmp), unit, sizeof (unit));
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s"
				"<span size=\"%d\"> %s</span></span>",
				local_config.color_dashboard,
				local_config.font_dashboard, ctmp,
				fontsize_unit, unit);
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
			"% 3.1f<span size=\"%d\"> %s</span></span>",
			local_config.color_dashboard,
			local_config.font_dashboard, current.groundspeed,
			fontsize_unit, unit);
			break;
		}
		case DASH_SPEED_AVG:
		{
			gdouble t_avgspeed;

			g_strlcpy (head, _("Avg. Speed"), sizeof (head));
			t_avgspeed = trip.speed_avg * local_config.distfactor / trip.countavgspeed;

			if (t_avgspeed >= 0.0)
			{
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
				"% 3.1f<span size=\"%d\"> %s</span></span>",
				local_config.color_dashboard,
				local_config.font_dashboard, t_avgspeed,
				fontsize_unit, unit);
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
		case DASH_SPEED_MAX:
		{
			g_strlcpy (head, _("Max. Speed"), sizeof (head));
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
			"% 3.1f<span size=\"%d\"> %s</span></span>",
			local_config.color_dashboard,
			local_config.font_dashboard, trip.speed_max,
			fontsize_unit, unit);
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
		case DASH_COURSE:
		{
			g_strlcpy (head, _("Course"), sizeof (head));
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%03.f"
				"%s</span>",
			local_config.color_dashboard,
			local_config.font_dashboard,
			RAD2DEG (current.course), DEGREE);
			break;
		}
		case DASH_TURN:
		{
			g_strlcpy (head, _("Turn"), sizeof (head));
			dir = RAD2DEG (current.bearing)
				- RAD2DEG (current.course);
			if (dir > 180)
				dir = dir - 360;
			else if (dir < -180)
				dir = 360 + dir;
			/* color the text:
				  right: green  (starboard)
				  left: red     (port) */
			if (dir < 0.0) {
				dirs = 'L';
				g_strlcpy(turn_color, "#800000", sizeof(turn_color));
			}
			else if (dir > 0.0) {
				dirs = 'R';
				g_strlcpy(turn_color, "#008000", sizeof(turn_color));
			}
			else { // dir==0.0 [or 180.0 ??]). dir is a double so rather unlikely
				dirs = ' ';
				g_snprintf (turn_color, sizeof(turn_color), "%s",
					local_config.color_dashboard);
			}
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%c %03d"
				"%s</span>",
				turn_color,
				local_config.font_dashboard,
				dirs, abs (dir), DEGREE);   // use fabs(dir) + %.0f ???
			break;
		}
		case DASH_TIMEREMAIN:
		{
			gdouble t_avgspeed, t_remain;
			gint t_hours;
			g_strlcpy (head, _("Time remaining"), sizeof (head));
			t_avgspeed = trip.speed_avg * local_config.distfactor / trip.countavgspeed;

			if (t_avgspeed > 0.0)
			{
				t_remain = current.dist / t_avgspeed * 60;
				if (t_remain < 1.0)
				{
					g_snprintf (content, sizeof (content),
						"<span color=\"%s\" font_desc=\"%s\">"
						" &lt; 1<span size=\"%d\"> min</span></span>",
						local_config.color_dashboard,
						local_config.font_dashboard, fontsize_unit);
				}
				else if (t_remain >= 60.0)
				{
					t_hours = t_remain / 60;
					t_remain -= t_hours * 60;
					g_snprintf (content, sizeof (content),
						"<span color=\"%s\" font_desc=\"%s\">"
						"% d<span size=\"%d\"> h </span>"
						"%02.0f<span size=\"%d\"> min</span></span>",
						local_config.color_dashboard,
						local_config.font_dashboard, t_hours,
						fontsize_unit, t_remain,
						fontsize_unit);
				}
				else
				{
					g_snprintf (content, sizeof (content),
						"<span color=\"%s\" font_desc=\"%s\">"
						"%2.0f<span size=\"%d\"> min</span></span>",
						local_config.color_dashboard,
						local_config.font_dashboard, t_remain,
						fontsize_unit);
				}
			}
			else
			{
				g_snprintf (content, sizeof (content),
					"<span color=\"%s\" font_desc=\"%s\">"
					" 0<span size=\"%d\"> min</span></span>",
					local_config.color_dashboard,
					local_config.font_dashboard, fontsize_unit);
			}
			break;
		}
		case DASH_ALT:
		{
			/* to avoid jumping of the altitude display due to bad
			 * nmea data, delay displaying after loss of 3D Fix */
			GTimeVal t_now;
			const gint ALTDELAY = 5;

			g_strlcpy (head, _("Altitude"), sizeof (head));

			if (current.gps_mode == GPSMODE_3D)
				g_get_current_time (&current.last3dfixtime);
			g_get_current_time (&t_now);

			if (t_now.tv_sec - current.last3dfixtime.tv_sec < ALTDELAY)
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
				"%s<span size=\"%d\"> %s</span></span>",
				local_config.color_dashboard,
				local_config.font_dashboard,
				ctmp, fontsize_unit, unit);
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
		case DASH_TRIP:
		{
			g_strlcpy (head, _("Odometer"), sizeof (head));

			switch (local_config.distmode)
			{
				case DIST_MILES:
				{
					g_strlcpy (unit, "mi", sizeof (unit));
					if (trip.odometer <= 1.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.0f",
							trip.odometer * 1760.0);
						g_strlcpy (unit, "yrds",
							sizeof (unit));
					}
					else if (current.dist <= 10.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.2f",
							trip.odometer);
					}
					else
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.1f", 
							trip.odometer);
					}
					break;
				}
				case DIST_NAUTIC:
				{
					g_strlcpy (unit, _("nmi"),
						sizeof (unit));
					if (trip.odometer <= 1.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.3f", 
							trip.odometer);
					}
					else if (current.dist <= 10.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.2f", 
							trip.odometer);
					}
					else
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.1f", 
							trip.odometer);
					}
					break;
				}
				default:
				{
					g_strlcpy (unit, _("km"),
						sizeof (unit));
					if (trip.odometer <= 1.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.0f",
							trip.odometer * 1000.0);
						g_strlcpy (unit, "m",
							sizeof (unit));
					}
					else if (current.dist <= 10.0)
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.2f", 
							trip.odometer);
					}
					else
					{
						g_snprintf (ctmp, sizeof
							(ctmp), "%.1f", 
							trip.odometer);
					}
				}
			}
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s"
				"<span size=\"%d\"> %s</span></span>",
				local_config.color_dashboard,
				local_config.font_dashboard, ctmp,
				fontsize_unit, unit);
			break;
		}
		case DASH_GPSPRECISION:
		{
			gint pfd_size;
			gchar *font_prec;
			PangoFontDescription *pfd;
			
			pfd = pango_font_description_from_string
				(local_config.font_dashboard);
			pfd_size = pango_font_description_get_size (pfd);
			pango_font_description_set_size (pfd, pfd_size*0.5);
			font_prec = pango_font_description_to_string (pfd);

			g_strlcpy (head, _("GPS Precision"), sizeof (head));

			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">EPH: %.1f m\nEPV: %.1f m</span>",
				local_config.color_dashboard, font_prec,
				current.gps_eph, current.gps_epv);

			pango_font_description_free (pfd);
			g_free (font_prec);
			break;
		}
		case DASH_TIME:
		{
			g_strlcpy (head, _("Current Time"), sizeof (head));
			if (current.gps_status > GPS_NO_FIX)
			{
				g_snprintf (content, sizeof (content),
					"<span color=\"%s\" font_desc=\"%s\">"
					"%s</span>",
				local_config.color_dashboard,
				local_config.font_dashboard, current.loc_time);
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
		case DASH_POSITION:
		{
			gchar slat[20];
			gchar slon[20];
			gint pfd_size;
			gchar *font_pos;
			PangoFontDescription *pfd;
			
			pfd = pango_font_description_from_string
				(local_config.font_dashboard);
			pfd_size = pango_font_description_get_size (pfd);
			pango_font_description_set_size (pfd, pfd_size*0.5);
			font_pos = pango_font_description_to_string (pfd);

			g_strlcpy (head, _("Position"), sizeof (head));
			coordinate2gchar(slat, sizeof(slat), coords.current_lat,
				TRUE, local_config.coordmode);
			coordinate2gchar(slon, sizeof(slon), coords.current_lon,
				FALSE, local_config.coordmode);
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s\n%s</span>",
				local_config.color_dashboard, font_pos,
				slat, slon);

			pango_font_description_free (pfd);
			g_free (font_pos);
			break;
		}
		case DASH_DESTINATION:
		{
			gchar slat[20];
			gchar slon[20];
			gint pfd_size;
			gchar *font_pos;
			PangoFontDescription *pfd;
			
			pfd = pango_font_description_from_string
				(local_config.font_dashboard);
			pfd_size = pango_font_description_get_size (pfd);
			pango_font_description_set_size (pfd, pfd_size*0.5);
			font_pos = pango_font_description_to_string (pfd);

			g_strlcpy (head, _("Destination"), sizeof (head));
			if (route.items > 0)
			{
				coordinate2gchar(slat, sizeof(slat), coords.dest_lat,
					TRUE, local_config.coordmode);
				coordinate2gchar(slon, sizeof(slon), coords.dest_lon,
					FALSE, local_config.coordmode);
			}
			else
			{
				coordinate2gchar(slat, sizeof(slat), coords.target_lat,
					TRUE, local_config.coordmode);
				coordinate2gchar(slon, sizeof(slon), coords.target_lon,
					FALSE, local_config.coordmode);
			}
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">%s\n%s</span>",
				local_config.color_dashboard, font_pos,
				slat, slon);

			pango_font_description_free (pfd);
			g_free (font_pos);
			break;
		}
		case DASH_MAPSCALE:
		{
			g_strlcpy (head, _("Map Scale"), sizeof (head));
			g_snprintf (content, sizeof (content),
				"<span color=\"%s\" font_desc=\"%s\">1:%ld</span>",
				local_config.color_dashboard,
				local_config.font_dashboard,
				current.mapscale);
			break;
		}

#ifdef NOTEYET
		case DASH_XTE:
		{
			gdouble xte = 0.0/0.0; /* init as nan */
			gdouble dist_to_next, azimuth_to_next, azimuth_of_leg;
			gdouble leg_length, back_azimuth; /* unused but needed for inv_geodesic() */

			/* get these from ??? */
			gdouble last_wpt_lon, last_wpt_lat, next_wpt_lon, next_wpt_lat;

			g_strlcpy (head, _("Cross-track error"), sizeof (head));

			if (route.active)
			{

			/*** TODO ***/

			/* need:
				current position (coords.current_lon, coords.current_lat)
				next rte point (wpt) coord (?? coords.target_lon, coords.target_lat ??) ["target" is cross-hair posn?]
				prev rte point (new variables), or nan,nan if not past first point in route
				--(coords.dest_lon, coords.dest_lat is posn of final rte pt. not useful here)
			   plan: (hopefully bug free)
				calc geodesic dist from curr_pos to rte_ next [inv_geodesic() or calc_wpdist() in gpsmisc.c]
				calc heading angle for geodesic line from rte_last to rte_next. [inv_geodesic() in gpsmisc.c]
				calc heading angle for geodesic line from curr_pos to rte_next.[inv_geodesic() in gpsmisc.c]
					this is "faz" or "baz" from inv_geodesic() in gpsmisc.c

				angle2 - angle1 = relative course to steer to get to rte_next.
				xte (meters) = geo_dist (meters) * sin(relative course to rte_next)

			    Probably there is a simpler way?
			     It is important to use exact calcuations: meters count here.
			*/

				/* get dist + angle from current pos to next waypoint */
				inv_geodesic(next_wpt_lon, next_wpt_lat, 0, 0, TRUE,
					     &dist_to_next, &azimuth_to_next, &back_azimuth);

				/* get angle from last waypoint to next waypoint */
				inv_geodesic(next_wpt_lon, next_wpt_lat,
					     last_wpt_lon, last_wpt_lat, FALSE,
					     &leg_length, &azimuth_of_leg, &back_azimuth);

				xte = sin(azimuth_of_leg - azimuth_to_next) * dist_to_next; /* meters */

				/* color the text:
					  right: green  (starboard)
					  left: red     (port) */
				if (xte < 0.0) {
				    dirs = 'L';
				    g_strlcpy(turn_color, "#800000", sizeof(turn_color));
				}
				else if (xte > 0.0) {
				    dirs = 'R';
				    g_strlcpy(turn_color, "#008000", sizeof(turn_color));
				}
				else { // xte==0.0 [or 180.0 ??]). xte is a double so rather unlikely
				    dirs = ' ';
				    g_snprintf (turn_color, sizeof(turn_color), "%s",
					local_config.color_dashboard);
				}

				g_snprintf (content, sizeof (content),
				    "<span color=\"%s\" font_desc=\"%s\">%c %.0f m "
				    "</span>",
				    turn_color,
				    local_config.font_dashboard,
				    dirs, abs(xte) );  // use fabs(xte) + %.0f ???
			}
			else
			{
				g_snprintf (content, sizeof (content),
					"<span color=\"%s\" font_desc=\"%s\" "
					"size=\"16000\">%s</span>",
					local_config.color_dashboard,
					local_config.font_dashboard,
					 _("n/a"));
			}
			break;
		}
#endif

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
	gtk_label_set_text (GTK_LABEL (statustime_lb), current.loc_time);

	/* update friends */
	if (local_config.showfriends)
	{
		g_snprintf (stmp, sizeof (stmp), "%d/%d", actualfriends,
			maxfriends);
		gtk_label_set_text (GTK_LABEL (statusfriends_lb), stmp);
	}

	/* update lat/lon display */
	if (gui_status.expmode)
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

	/* update course */
	g_snprintf (stmp, sizeof (stmp), "%3.0f%s",
		RAD2DEG (current.course), DEGREE);
	gtk_label_set_text (GTK_LABEL (statuscourse_lb), stmp);

	/* update bearing */
	g_snprintf (stmp, sizeof (stmp), "%3.0f%s",
		RAD2DEG (current.bearing), DEGREE);
	gtk_label_set_text (GTK_LABEL (statusbearing_lb), stmp);

	/* update dashboard */
	update_dashboard (frame_dash_1, local_config.dashboard[1]);
	update_dashboard (frame_dash_2, local_config.dashboard[2]);
	update_dashboard (frame_dash_3, local_config.dashboard[3]);
	update_dashboard (frame_dash_4, local_config.dashboard[4]);

	/* update gps status displays */
	expose_gpsfix (NULL, local_config.gpsfix_style);
	expose_sats_cb (NULL, 0);

	return TRUE;
}


/* *****************************************************************************
 * Update Display of GPS Status
 */
gint
expose_gpsfix (GtkWidget *widget, gboolean vertical)
{
	GdkDrawable *drawable_gpsfix;
	GdkGC *kontext_gpsfix;
	PangoFontDescription *pfd_gpsfix;
	PangoLayout *layout_gpsfix;
	gint t_x, t_y, t_tx, t_ty, t_w, i;

	if (mydebug >50)
		g_print ("expose_gpsfix()\n");

	drawable_gpsfix = drawing_gpsfix->window;
	if (!drawable_gpsfix)
		return 0;

	kontext_gpsfix = gdk_gc_new (drawable_gpsfix);
	pfd_gpsfix = pango_font_description_from_string ("Sans 8");
	gdk_drawable_get_size (drawable_gpsfix, &t_x, &t_y);

	layout_gpsfix = gtk_widget_create_pango_layout (drawing_gpsfix, _("No GPS"));
	pango_layout_set_font_description (layout_gpsfix, pfd_gpsfix);
	pango_layout_get_pixel_size (layout_gpsfix, &t_tx, &t_ty);

	if (current.simmode)
	{
		gdk_gc_set_foreground (kontext_gpsfix, &colors.blue);
		pango_layout_set_text (layout_gpsfix, _("Simulation Mode"), -1);
		gdk_draw_rectangle (drawable_gpsfix, kontext_gpsfix, TRUE, 0,0, t_x, t_y);
		gdk_draw_layout_with_colors (drawable_gpsfix, kontext_gpsfix, 5, (t_y-t_ty)/2, layout_gpsfix, &colors.white, NULL);	
	}
	else
	{
		switch (current.gps_mode)
		{
		case GPSMODE_NOT_SEEN:
		{
			gdk_gc_set_foreground (kontext_gpsfix, &colors.red);
			break;
		}
		case GPSMODE_NO_FIX:
		{
			gdk_gc_set_foreground (kontext_gpsfix, &colors.red);
			pango_layout_set_text (layout_gpsfix, "No Fix", -1);
			break;
		}
		case GPSMODE_2D:
		{
			gdk_gc_set_foreground (kontext_gpsfix, &colors.green);
			pango_layout_set_text (layout_gpsfix, "2D Fix", -1);
			break;
		}
		case GPSMODE_3D:
		{
			if (current.gps_status != GPS_DGPS_FIX)
			{
				gdk_gc_set_foreground (kontext_gpsfix, &colors.green);
				pango_layout_set_text (layout_gpsfix, "3D Fix", -1);
			}
			else
			{
				gdk_gc_set_foreground (kontext_gpsfix, &colors.green_light);
				pango_layout_set_text (layout_gpsfix, "3D Fix+", -1);
			}
			break;
		}
		}

		gdk_draw_rectangle (drawable_gpsfix, kontext_gpsfix, TRUE, 0,0, t_x, t_y);
		if (vertical)
			gdk_draw_layout_with_colors (drawable_gpsfix, kontext_gpsfix,
				2, t_y-t_ty, layout_gpsfix, &colors.black, NULL);
		else
			gdk_draw_layout_with_colors (drawable_gpsfix, kontext_gpsfix,
				t_x-t_tx-5, (t_y-t_ty)/2, layout_gpsfix, &colors.black, NULL);

		gdk_gc_set_foreground (kontext_gpsfix, &colors.black);
		t_w = (vertical ? (t_y-t_ty-26) : (t_x-t_tx-26))/12;
		if (t_w < 1)
			t_w =1;
		gdk_gc_set_line_attributes (kontext_gpsfix, t_w, 0, 0, 0);

		for (i=1; i<13; i++)
		{
			if (i > current.gps_sats_in_view)
				gdk_gc_set_foreground (kontext_gpsfix, &colors.grey);
			else if (i > current.gps_sats_used)
				gdk_gc_set_foreground (kontext_gpsfix, &colors.darkgrey);
			if (vertical)
				gdk_draw_line (drawable_gpsfix, kontext_gpsfix, 5, t_y-t_ty-(i*(t_w+1))-5, t_x-5, t_y-t_ty-(i*(t_w+1))-5);
			else
				gdk_draw_line (drawable_gpsfix, kontext_gpsfix, i*(t_w+1)+5, 5, i*(t_w+1)+5, t_y-5);
		}
	}

	pango_font_description_free (pfd_gpsfix);

	return TRUE;
}


/* *****************************************************************************
 * show satellite information
 * TODO:
 *	- show satellite numbers
 */
gint
expose_sats_cb (GtkWidget *widget, guint *datum)
{
	GdkDrawable *drawable_sats;
	GdkGC *kontext_sats;
	PangoFontDescription *pfd_sats;
	PangoLayout *layout_sats;
	gint x, y, w, px, py, bx, h, i, j;
	gdouble el, az;
	gchar slat[20], slon[20], t_buf[200];

	if ( mydebug > 50 )
		g_print ("expose_sats_cb ()\n");

	if (current.gps_status == GPS_NO_FIX)
		return TRUE;

	drawable_sats = drawing_sats->window;
	if (!drawable_sats)
		return TRUE;

	kontext_sats = gdk_gc_new (drawable_sats);
	gdk_drawable_get_size (drawable_sats, &x, &y);
	w = y <= x/2 ? y : x/2;

	/* draw background for satellite position view */
	gdk_gc_set_line_attributes (kontext_sats, 1, GDK_LINE_SOLID, 0, 0);
	gdk_gc_set_foreground (kontext_sats, &colors.grey);
	gdk_draw_arc (drawable_sats, kontext_sats, FALSE, 6, 6, w-12, w-12, 100 * 64, 340 * 64);
	gdk_draw_arc (drawable_sats, kontext_sats, FALSE, 3+w/4, 3+w/4, w/2-6, w/2-6, 0, 360 * 64);
	gdk_draw_arc (drawable_sats, kontext_sats, FALSE, w/2-4, w/2-4, 8, 8, 0, 360 * 64);
	layout_sats = gtk_widget_create_pango_layout (drawing_sats, "N");
	pfd_sats = pango_font_description_from_string ("Sans Bold 12");
	pango_layout_set_font_description (layout_sats, pfd_sats);
	pango_layout_get_pixel_size (layout_sats, &px, NULL);
	gdk_draw_layout_with_colors (drawable_sats, kontext_sats, (w-px)/2, 0,
		layout_sats, &colors.grey, &colors.darkgrey);
	if (layout_sats != NULL)
		g_object_unref (G_OBJECT (layout_sats));
	pango_font_description_free (pfd_sats);

	/* draw background for satellite level view */
	gdk_gc_set_foreground (kontext_sats, &colors.darkgrey);
	gdk_draw_rectangle (drawable_sats, kontext_sats, TRUE, x/2+6, w/2+6, w-12, w/2-12);
	gdk_gc_set_line_attributes (kontext_sats, 1, 0, 0, 0);
	gdk_gc_set_foreground (kontext_sats, &colors.grey);
	gdk_draw_rectangle (drawable_sats, kontext_sats, FALSE, x/2+6, w/2+6, w-12, w/2-12);
	bx = (w-60)/12;
	if (bx < 1)
		bx = 1;
	gdk_gc_set_line_attributes (kontext_sats, bx, 0, 0, 0);
	for (i=0; i<12; i++)
	{
		if (i == current.gps_sats_in_view)
			gdk_gc_set_foreground (kontext_sats, &colors.black);
		gdk_draw_line (drawable_sats, kontext_sats,
			x/2+12 + i*(bx+4)+bx/2 , w/2+12 , x/2+12 + i*(bx+4) + bx/2 , w-12);
	}
	j=0;
	/* draw graphical satellite info */
	gdk_gc_set_line_attributes (kontext_sats, 1, 0, 0, 0);
	for (i = 0; i < current.gps_sats_in_view; i++)
	{
		if (mydebug > 60)
		{
			g_print ("Satinfo: PRN %03d, Signal %02d dB, Position: %d/%d\n",
				gps_sats[i].prn, gps_sats[i].snr,
				gps_sats[i].elevation, gps_sats[i].azimuth);
		}
		/* draw satellite levels */
		h = gps_sats[i].snr - 20;
		if (h < 0)
			gdk_gc_set_foreground (kontext_sats, &colors.red_dark);
		else if (h < 10)
			gdk_gc_set_foreground (kontext_sats, &colors.red);
		else if (h < 15)
			gdk_gc_set_foreground (kontext_sats, &colors.yellow);
		else if (h < 20)
			gdk_gc_set_foreground (kontext_sats, &colors.green);
		else
			gdk_gc_set_foreground (kontext_sats, &colors.green_light);
		if (h < 1)
			h = 1;
		if (h > 30)
			h = 30;
		h = (w-48)*h/60;
		gdk_draw_rectangle (drawable_sats, kontext_sats, TRUE,
			x/2+12 + j*(bx+4), w-12-h, bx, h);

		/* draw satellite positions */
		el = 1 - (gps_sats[i].elevation / 90.0);
		az = DEG2RAD(gps_sats[i].azimuth);
		//x = (w / 2) + sin (az) * (el / 90.0) * (w / 2);
		//y = (w / 2) - cos (az) * (el / 90.0) * (w / 2);
		gdk_draw_arc (drawable_sats, kontext_sats, gps_sats[i].used,
			w/2 * (1 + sin(az) * el) - 3,
			w/2 * (1 - cos(az) * el) - 3,
			12, 12, 0, 360 * 64);
			j++;
	}

	/* draw text info */
	coordinate2gchar(slat, sizeof(slat), coords.current_lat, TRUE, local_config.coordmode);
	coordinate2gchar(slon, sizeof(slon), coords.current_lon, FALSE, local_config.coordmode);
	g_snprintf (t_buf, sizeof (t_buf),
		"Latitude: %s\nLongitude: %s\nAltitude: %.1f m\nUTC Time:"
		" %s\nEPH: %.1f m / EPV: %.1f m",
		slat, slon, current.altitude, current.utc_time,
		current.gps_eph, current.gps_epv);
	layout_sats = gtk_widget_create_pango_layout (drawing_sats, t_buf);
	pfd_sats = pango_font_description_from_string ("Sans Bold 10");
	pango_layout_set_font_description (layout_sats, pfd_sats);
	pango_layout_get_pixel_size (layout_sats, &px, &py);
	gdk_draw_layout_with_colors (drawable_sats, kontext_sats, w*1.5-px/2, (w-px)/2,
		layout_sats, &colors.grey, &colors.darkgrey);
	if (layout_sats != NULL)
		g_object_unref (G_OBJECT (layout_sats));
	pango_font_description_free (pfd_sats);

	return TRUE;
}


/* *****************************************************************************
 * callback to update mapview size when application window is resized
 */
gint
set_mapviewsize_cb (void)
{
	gui_status.mapview_x = map_drawingarea->allocation.width;
	gui_status.mapview_y = map_drawingarea->allocation.height;

	if (gui_status.mapview_x > MAPWIDTH)
		gui_status.mapview_x = MAPWIDTH;
	if (gui_status.mapview_y > MAPHEIGHT)
		gui_status.mapview_y = MAPHEIGHT;

	if (gui_status.mapview_x < 150)
		gui_status.mapview_x = 150;
	if (gui_status.mapview_y < 150)
		gui_status.mapview_y = 150;


	if (gui_status.mapview_x < gui_status.mapview_y)
		borderlimit = gui_status.mapview_x / 5;
	else
		borderlimit = gui_status.mapview_y / 5;

	if (mydebug > 20)
	{
		fprintf (stderr, "  setting borderlimit to %d\n", borderlimit);
		fprintf (stderr, "  setting size of mapview to %dx%d px\n",
			gui_status.mapview_x, gui_status.mapview_y);
	}

	return FALSE;
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
	guint modifiers;

	if ( mydebug > 0 )
		g_print ("event:%x key:%c\n", event->keyval, event->keyval);

	// Ctrl-Q exits the program
 	modifiers = gtk_accelerator_get_default_mod_mask ();
	if (((event->state & modifiers) == GDK_CONTROL_MASK) && 
	    ((event->keyval == GDK_Q) || (event->keyval == GDK_q)))
	{
		gtk_main_quit();
		return TRUE;
	}
  

#ifdef MAEMO
	/* make use of hardware keys */

	// toggle fullscreen
	if (event->keyval == GDK_F6)
	{
		if (current.window_state << GDK_WINDOW_STATE_FULLSCREEN)
			gtk_window_unfullscreen (GTK_WINDOW (main_window));
		else
			gtk_window_fullscreen (GTK_WINDOW (main_window));
	}

	// zoom in
	if (event->keyval == GDK_F7)
	{
		 hildon_banner_show_information (main_window, GTK_STOCK_ADD, _("Zoom in"));
		 scalerbt_cb (NULL, 2);
	}

	// zoom out
	if (event->keyval == GDK_F8)
	{
		 hildon_banner_show_information (main_window, GTK_STOCK_REMOVE, _("Zoom out"));
		 scalerbt_cb (NULL, 1);
	}	
#endif

	// Show Help Window
	if (event->keyval == GDK_F1)
		help_cb (NULL, 0);

	// Toggle Display of Control Buttons
	if ((toupper (event->keyval)) == 'M' && local_config.guimode != GUI_PDA)
	{
		if (local_config.show_controls)
		{
			gtk_widget_hide_all (main_toolbar);
			local_config.show_controls = FALSE;
		}
		else
		{
			gtk_widget_show_all (main_toolbar);
			local_config.show_controls = TRUE;
		}
	}

	// Toggle Display of Compass
	if ((toupper (event->keyval)) == 'C' && local_config.guimode != GUI_PDA && local_config.show_dashboard == TRUE)
	{
		if (local_config.show_compass)
		{
			
			gtk_widget_hide_all (compass_box);
			local_config.show_compass = FALSE;
		}
		else
		{
			gtk_widget_show_all (compass_box);
			local_config.show_compass = TRUE;
		}
	}

	// Toggle Display of Dashboard
	if ((toupper (event->keyval)) == 'D' && local_config.guimode != GUI_PDA)
	{
		if (local_config.show_dashboard)
		{
			if (local_config.guimode == GUI_DESKTOP)
			{
				gtk_widget_hide_all (statusdashboard_box);
				gtk_widget_hide_all (compass_box);
			}
			else
				gtk_widget_hide_all (mainbox_status);
			local_config.show_dashboard = FALSE;
		}
		else
		{
			if (local_config.guimode == GUI_DESKTOP)
			{
				gtk_widget_show_all (statusdashboard_box);
				if (local_config.show_compass)
					gtk_widget_show_all (compass_box);
			}
			else
				gtk_widget_show_all (mainbox_status);
			local_config.show_dashboard = TRUE;
		}
	}

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
		quickaddwaypoint ();
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

	// Add instant waypoint at current mouse location
	if ((toupper (event->keyval)) == 'P')
	{
		gchar wp_name[100], wp_type[100], wp_comment[100];
		GTimeVal current_time;
		g_get_current_time (&current_time);
		g_snprintf (wp_name, sizeof (wp_name), "%s",
			(g_time_val_to_iso8601 (&current_time))+5);
		g_snprintf (wp_type, sizeof (wp_type),
			"waypoint.wpttemp.wpttemp-yellow");
		g_snprintf (wp_comment, sizeof (wp_comment),
			_("Temporary Waypoint"));
		gdk_window_get_pointer (map_drawingarea->window, &x, &y, &state);
		calcxytopos (x, y, &lat, &lon, current.zoom);
		if ( mydebug > 0 )
			printf ("Add Waypoint: %s lat:%f,lon:%f (x:%d,y:%d)\n",
				wp_name, lat, lon, x, y);
		if (local_config.use_database)
			addwaypoint (wp_name, wp_type, wp_comment, lat, lon, TRUE);
		else
			addwaypoint (wp_name, wp_type, wp_comment, lat, lon, FALSE);
	}

	// Add instant routepoint at current mouse location
	if ((toupper (event->keyval)) == 'R')
	{
		add_quickpoint_to_route ();
	}

	// In Route mode Force next Route Point
	if (((toupper (event->keyval)) == 'J') && route.active)
	{
		route.forcenext = TRUE;
	}


	// Zoom in/out
	if (event->keyval == '-' || event->keyval == 0xFFad)
	{
		/* Zoom out */
		scalerbt_cb (NULL, 1);
	}
	if (event->keyval == '+' || event->keyval == 0xFFab || event->keyval == '=')
	{
		/* Zoom in */
		scalerbt_cb (NULL, 2);
	}

	// Switch Night Mode
	if ((toupper (event->keyval)) == 'N')
	{
		; // TODO: do something
	}

	// Switch Explore Mode ("e" for Explore)
	if ((toupper (event->keyval)) == 'E')
	{
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (explore_bt), !gui_status.expmode);
	}

	// Query Info for near points
	if ( ( (toupper (event->keyval)) == '?' )
	     || ( (toupper (event->keyval)) == 'Q') )
	{
	    gdouble lat1,lon1,lat2,lon2;
	    gint delta = 10;

	    gdk_window_get_pointer (map_drawingarea->window, &x, &y, &state);
	    calcxytopos (x-delta, y-delta, &lat1, &lon1, current.zoom);
	    calcxytopos (x+delta, y+delta, &lat2, &lon2, current.zoom);
	    printf ("---------------------------------------------\n");
	    if ( local_config.showpoi )
		poi_query_area (min(lat1,lat2), min(lon1,lon2),
			max(lat1,lat2), max(lon1,lon2) );

		{
			gdouble lat,lon;
			gdouble dist=lat2-lat1;
			calcxytopos (x, y, &lat, &lon, current.zoom);
			dist = dist>0?dist:-dist;
		}
	}


	if ( mydebug>10 ) 
	    fprintf(stderr,"Key pressed: %0x\n",event->keyval);


	if (gui_status.expmode)
	    {
		gint old_x, old_y, new_x, new_y;
		gint have_new = FALSE;

		calcxy (&old_x, &old_y, coords.current_lon,
			coords.current_lat, current.zoom);

		/* if in the outer third of the map canvas jump to the
			 map edge, otherwise jump 1/4 of the map */
		if ( event->keyval == 0xff52 || event->keyval == 0xff97 ) // Up
		    {
			new_x = old_x;
			new_y = old_y < MAPHEIGHT/3 ? 0 : old_y - MAPHEIGHT/4;
			have_new = TRUE;
		    }
		if ( event->keyval == 0xff54 || event->keyval == 0xff99 ) // Down
		    {
			new_x = old_x;
			new_y = old_y > gui_status.mapview_y - MAPHEIGHT/3
					? gui_status.mapview_y
					: old_y + MAPHEIGHT/4;
			have_new = TRUE;
		    }
		if ( event->keyval == 0xff51 || event->keyval == 0xff96 ) // Left
		    {
			new_x = old_x < MAPWIDTH/3 ? 0 : old_x - MAPWIDTH/4;
			new_y = old_y;
			have_new = TRUE;
		    }
		if ( event->keyval == 0xff53 || event->keyval == 0xff98 ) // Right 
		    {
			new_x = old_x > gui_status.mapview_x - MAPWIDTH/3
					? gui_status.mapview_x
					: old_x + MAPWIDTH/4;
			new_y = old_y;
			have_new = TRUE;
		    }

		if ( event->keyval == 0xff95 ) // Keypad Home
		    {
			new_x = old_x < MAPWIDTH/3 ? 0 : old_x - MAPWIDTH/4;
			new_y = old_y < MAPHEIGHT/3 ? 0 : old_y - MAPHEIGHT/4;
			have_new = TRUE;
		    }
		if ( event->keyval == 0xff9a ) // Keypad PgUp
		    {
			new_x = old_x > gui_status.mapview_x - MAPWIDTH/3
					? gui_status.mapview_x
					: old_x + MAPWIDTH/4;
			new_y = old_y < MAPHEIGHT/3 ? 0 : old_y - MAPHEIGHT/4;
			have_new = TRUE;
		    }
		if ( event->keyval == 0xff9b ) // Keypad PgDn
		    {
			new_x = old_x > gui_status.mapview_x - MAPWIDTH/3
					? gui_status.mapview_x
					: old_x + MAPWIDTH/4;
			new_y = old_y > gui_status.mapview_y - MAPHEIGHT/3
					? gui_status.mapview_y
					: old_y + MAPHEIGHT/4;
			have_new = TRUE;
		    }
		if ( event->keyval == 0xff9c ) // Keypad End
		    {
			new_x = old_x < MAPWIDTH/3 ? 0 : old_x - MAPWIDTH/4;
			new_y = old_y > gui_status.mapview_y - MAPHEIGHT/3
					? gui_status.mapview_y
					: old_y + MAPHEIGHT/4;
			have_new = TRUE;
		    }


		if(have_new)
		    calcxytopos (new_x, new_y, &coords.current_lat,
				 &coords.current_lon, current.zoom );

		coords.expmode_lon = coords.current_lon;
		coords.expmode_lat = coords.current_lat;
	    }
	return 0;
}


/* *****************************************************************************
 * Popup Menu: Dashboard Mode
 */
void create_dashboard_menu (void)
{
	const gchar *dash_array[] =
	{
		_("Distance to Target"),	/* DASH_DIST */
		_("Time Remaining"),		/* DASH_TIMEREMAIN */
		_("Bearing"),			/* DASH_BEARING */
		_("Turn"),			/* DASH_TURN */
		_("Speed"),			/* DASH_SPEED */
		_("Avg. Speed"),		/* DASH_SPEED_AVG */
		_("Max. Speed"),		/* DASH_SPEED_MAX */
		_("Course"),			/* DASH_COURSE */
		_("Altitude"),			/* DASH_ALT */
		_("Trip Odometer"),		/* DASH_TRIP */
		_("GPS Precision"),		/* DASH_GPSPRECISION */
		_("Current Time"),		/* DASH_TIME */
		_("Position"),			/* DASH_POSITION */
		_("Map Scale"),			/* DASH_MAPSCALE */
		_("Destination"),		/* DASH_DESTINATION */
#ifdef NOTYET
		_("Cross-track error"),		/* DASH_XTE */
#endif
	};

	gint i;
	GtkWidget *dash_menuitem;

	dash_menu = gtk_menu_new ();
	
	for (i=0; i< DASH_N_ITEMS; i++)
	{
		dash_menuitem = gtk_menu_item_new_with_label (dash_array[i]);
		gtk_menu_shell_append (GTK_MENU_SHELL (dash_menu),
			dash_menuitem);
		g_signal_connect (dash_menuitem, "activate",
			GTK_SIGNAL_FUNC (dash_select_cb), GINT_TO_POINTER (i));
	}
	gtk_widget_show_all (dash_menu);
}


/* *****************************************************************************
 * Popup Menu: Dashboard Mode (used for Car Mode)
 */
void create_dashboard_carmenu (void)
{
	GtkWidget *dash_button[DASH_N_ITEMS+1];
	GtkTable *dash_table;
	gint i;
	gint x = 0;
	gint y = 0;

	const gchar *dash_array[] =
	{
		_("Distance to Target"),	/* DASH_DIST */
		_("Time Remaining"),		/* DASH_TIMEREMAIN */
		_("Bearing"),			/* DASH_BEARING */
		_("Turn"),			/* DASH_TURN */
		_("Speed"),			/* DASH_SPEED */
		_("Avg. Speed"),		/* DASH_SPEED_AVG */
		_("Max. Speed"),		/* DASH_SPEED_MAX */
		_("Heading"),			/* DASH_HEADING */
		_("Altitude"),			/* DASH_ALT */
		_("Trip Odometer"),		/* DASH_TRIP */
		_("GPS Precision"),		/* DASH_GPSPRECISION */
		_("Current Time"),		/* DASH_TIME */
		_("Position"),			/* DASH_POSITION */
		_("Map Scale"),			/* DASH_MAPSCALE */
		_("Destination"),		/* DASH_DESTINATION */
#ifdef NOTYET
		_("Cross-track error"),		/* DASH_XTE */
#endif
	};

	dash_menu_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_transient_for (GTK_WINDOW (dash_menu_window), GTK_WINDOW (main_window));
	gtk_window_set_modal (GTK_WINDOW (dash_menu_window), TRUE);
	gtk_window_maximize (GTK_WINDOW (dash_menu_window));
	gtk_window_set_decorated (GTK_WINDOW (dash_menu_window), FALSE);

	dash_table = GTK_TABLE (gtk_table_new (5, 3, TRUE));
	gtk_table_set_row_spacings (dash_table, 10);
	gtk_table_set_col_spacings (dash_table, 10);
	gtk_container_add (GTK_CONTAINER (dash_menu_window), GTK_WIDGET (dash_table));

	for (i=0; i< DASH_N_ITEMS; i++)
	{
		if (x == 3)
		{
			x = 0; y++;
		}
		dash_button[i] = gtk_button_new_with_label (dash_array[i]);
		gtk_table_attach_defaults (dash_table, dash_button[i], x, x+1, y, y+1);
		x++;
		g_signal_connect (dash_button[i], "clicked",
			GTK_SIGNAL_FUNC (dash_select_cb), GINT_TO_POINTER (i));
	}
	dash_button[i] = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_table_attach_defaults (dash_table, dash_button[i], x, x+1, y, y+1);
	g_signal_connect_swapped (dash_button[i], "clicked",
			GTK_SIGNAL_FUNC (gtk_widget_hide_all), dash_menu_window);

}


/* *****************************************************************************
 * Window: Satellite Info
 */
void create_satellite_window (void)
{
	satellite_window = gtk_dialog_new_with_buttons (_("Satellite Info"),
		GTK_WINDOW (main_window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	g_signal_connect (satellite_window, "delete_event",
		G_CALLBACK (toggle_satelliteinfo_cb), GINT_TO_POINTER (FALSE));
	g_signal_connect (satellite_window, "response",
		G_CALLBACK (toggle_satelliteinfo_cb), GINT_TO_POINTER (FALSE));
	gtk_window_set_icon_name (GTK_WINDOW (satellite_window), "gtk-info");

#ifdef MAEMO
	gtk_window_maximize (GTK_WINDOW (satellite_window));
	gtk_window_set_modal (GTK_WINDOW (satellite_window), TRUE);
#else
	if (local_config.guimode == GUI_CAR)
	{
		gtk_window_maximize (GTK_WINDOW (satellite_window));
		gtk_window_set_decorated (GTK_WINDOW (satellite_window), FALSE);
		gtk_window_set_modal (GTK_WINDOW (satellite_window), TRUE);
	}
	else
		gtk_window_set_resizable (GTK_WINDOW (satellite_window), FALSE);
#endif

	drawing_sats = gtk_drawing_area_new ();
	gtk_widget_set_size_request (drawing_sats, 400, 200);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(satellite_window)->vbox), drawing_sats);
	g_signal_connect (drawing_sats, "expose_event", G_CALLBACK (expose_sats_cb), NULL);
}


/* *****************************************************************************
 * Window: Map Control Box (not used in PDA Mode)
 */
create_mapcontrol_box (void)
{
	gint i;

	controlbox_window = gtk_dialog_new_with_buttons (_("Map Control"),
		GTK_WINDOW (main_window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	g_signal_connect (controlbox_window, "delete_event", G_CALLBACK (toggle_controlbox_cb), GINT_TO_POINTER (0));
	g_signal_connect (controlbox_window, "response", G_CALLBACK (toggle_controlbox_cb), GINT_TO_POINTER (0));
	gtk_window_set_icon_name (GTK_WINDOW (controlbox_window), "gtk-properties");
	if (local_config.guimode == GUI_CAR)
	{
		gtk_window_set_decorated (GTK_WINDOW (controlbox_window), FALSE);
		gtk_window_set_modal (GTK_WINDOW (controlbox_window), TRUE);
	}

	GtkWidget *buttons_hbox = gtk_hbox_new (FALSE, 1 * PADDING);
	GtkWidget *buttons_vbox = gtk_vbox_new (FALSE, 1 * PADDING);
	
	GtkWidget *frame_poi = gtk_frame_new (_("Points"));
	GtkWidget *vbox_poi = gtk_vbox_new (TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_poi), vbox_poi);

	if (local_config.use_database)
	{
		/* Checkbox: POI Draw User */
		GtkWidget *poidraw_user_bt = gtk_check_button_new_with_mnemonic (_("_User DB"));
		if (local_config.showpoi == POIDRAW_USER || local_config.showpoi == POIDRAW_ALL)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poidraw_user_bt), TRUE);
		gtk_box_pack_start (GTK_BOX (vbox_poi), poidraw_user_bt, FALSE, FALSE, 0 * PADDING);
		g_signal_connect (poidraw_user_bt, "clicked", G_CALLBACK (poi_button_cb), GINT_TO_POINTER (POIDRAW_USER));
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), poidraw_user_bt, _("Show Points Of Interest found in user database"), NULL);

		/* Checkbox: POI Draw OSM */
		GtkWidget *poidraw_osm_bt = gtk_check_button_new_with_mnemonic (_("_OSM DB"));
		if (local_config.showpoi > POIDRAW_USER)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poidraw_osm_bt), TRUE);
		if (current.poi_osm)
			gtk_box_pack_start (GTK_BOX (vbox_poi), poidraw_osm_bt, FALSE, FALSE, 0 * PADDING);
		g_signal_connect (poidraw_osm_bt, "clicked", G_CALLBACK (poi_button_cb), GINT_TO_POINTER (POIDRAW_OSM));
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), poidraw_osm_bt, _("Show Points Of Interest found in OSM database"), NULL);
	}

	/* Checkbox: Draw Waypoints from file */
	wp_draw_bt = gtk_check_button_new_with_mnemonic (_("_WP File"));
	if (local_config.showwaypoints)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wp_draw_bt), TRUE);
	g_signal_connect (wp_draw_bt, "clicked", G_CALLBACK (toggle_button_cb), &local_config.showwaypoints);
	gtk_box_pack_start (GTK_BOX (vbox_poi), wp_draw_bt, FALSE, FALSE, 0 * PADDING);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), wp_draw_bt, _("Show Waypoints found in file way.txt"), NULL);

	GtkWidget *frame_track = gtk_frame_new (_("Track"));
	GtkWidget *vbox_track = gtk_vbox_new (TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_track), vbox_track);

	/* Checkbox: Show Track */
	GtkWidget *showtrack_bt = gtk_check_button_new_with_mnemonic (_("_Show"));
	if (local_config.showtrack)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (showtrack_bt), TRUE);
	g_signal_connect (showtrack_bt, "clicked", G_CALLBACK (toggle_button_cb), &local_config.showtrack);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), showtrack_bt, _("Show tracking on the map"), NULL);
	gtk_box_pack_start (GTK_BOX (vbox_track), showtrack_bt, FALSE, FALSE, 0 * PADDING);

	/* Checkbox: Save Track */
	GtkWidget *savetrack_bt = gtk_check_button_new_with_mnemonic (_("Sa_ve"));
	if (local_config.savetrack)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrack_bt), TRUE);
	g_signal_connect (savetrack_bt, "clicked", G_CALLBACK (toggle_track_button_cb), &local_config.savetrack);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), savetrack_bt, _("Save the track to given filename at program exit"), NULL);
	gtk_box_pack_start (GTK_BOX (vbox_track), savetrack_bt, FALSE, FALSE,0 * PADDING);

	GtkWidget *frame_mapcontrol = gtk_frame_new (_("Map Controls"));
	GtkWidget *vbox_map_controls = gtk_vbox_new (TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_mapcontrol), vbox_map_controls);

	/* Checkbox: Auto best Map */
	bestmap_bt = gtk_check_button_new_with_label (_("Auto _best map"));
	gtk_button_set_use_underline (GTK_BUTTON (bestmap_bt), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox_map_controls), bestmap_bt, FALSE, FALSE,0 * PADDING);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), bestmap_bt, _("Always select the most detailed map available"), NULL);
	if (local_config.autobestmap)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmap_bt),  TRUE);
	g_signal_connect (bestmap_bt, "clicked", G_CALLBACK (autobestmap_cb), GINT_TO_POINTER (1));
#ifdef MAPNIK
	if (active_mapnik_ysn())
	{
		/* Checkbox: Mapnik Mode */
		GtkWidget *mapnik_bt = gtk_check_button_new_with_label (_("Mapnik Mode"));
		gtk_button_set_use_underline (GTK_BUTTON (mapnik_bt), TRUE);
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), mapnik_bt,
			_("Turn mapnik mode on. In this mode vector maps rendered by mapnik"
			" (e.g. OpenStreetMap Data) are used instead of the other maps."), NULL);
		g_signal_connect (mapnik_bt, "clicked", G_CALLBACK (toggle_mapnik_cb), GINT_TO_POINTER (1));
		gtk_box_pack_start(GTK_BOX (vbox_map_controls), mapnik_bt, FALSE, FALSE,0 * PADDING);
	}
#endif
if (local_config.guimode==GUI_DESKTOP)
{
	GtkWidget *minimap_bt = gtk_check_button_new_with_label (_("Show Minimap"));
	gtk_button_set_use_underline (GTK_BUTTON (minimap_bt), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox_map_controls), minimap_bt, FALSE, FALSE,0 * PADDING);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), bestmap_bt, _("Always select the most detailed map available"), NULL);
	if (local_config.showminimap)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (minimap_bt),  TRUE);
	g_signal_connect (minimap_bt, "clicked", G_CALLBACK (showminimap_cb), GINT_TO_POINTER (1));
}

	frame_maptype = gtk_frame_new (_("Included mapsets"));
	GtkWidget *vbox_map_types = gtk_vbox_new (FALSE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_maptype), vbox_map_types);

	for (i = 0; i < max_display_map; i++)
    {
		/* Checkbox: Show Map: name xy */
		gchar display_name[100];
		if (mydebug > 1)
			g_snprintf (display_name, sizeof (display_name), "%s (%d)", _(display_map[i].name), display_map[i].count);
		else
			g_snprintf (display_name, sizeof (display_name), "%s", _(display_map[i].name));
		display_map[i].count++;
		display_map[i].checkbox = gtk_check_button_new_with_label (display_name);
		if (display_map[i].to_be_displayed)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (display_map[i].checkbox), TRUE);
		g_signal_connect (display_map[i].checkbox, "clicked", G_CALLBACK (display_maps_cb), GINT_TO_POINTER (i));
		gtk_box_pack_start (GTK_BOX (vbox_map_types), display_map[i].checkbox, FALSE, FALSE, 0 * PADDING);
	}

	gtk_box_pack_start (GTK_BOX (buttons_vbox),frame_poi, TRUE, TRUE, 1 * PADDING);
	gtk_box_pack_start (GTK_BOX (buttons_vbox),frame_track, TRUE, TRUE, 1 * PADDING);
	gtk_box_pack_start (GTK_BOX (buttons_vbox),frame_mapcontrol, TRUE, TRUE, 1 * PADDING);

	gtk_box_pack_start (GTK_BOX (buttons_hbox),buttons_vbox, TRUE, TRUE, 1 * PADDING);
	gtk_box_pack_start (GTK_BOX (buttons_hbox),frame_maptype, TRUE, TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(controlbox_window)->vbox), buttons_hbox);
}


/* *****************************************************************************
 * Window: Main -> Controls
 */
void create_toolbar_mainbox (void)
{
	GtkWidget *options_img = gtk_image_new_from_pixbuf (read_icon ("bt_options.png",0));
	GtkWidget *zoom_in_img = gtk_image_new_from_pixbuf (read_icon ("bt_zoom-in.png",0));
	GtkWidget *zoom_out_img = gtk_image_new_from_pixbuf (read_icon ("bt_zoom-out.png",0));
	GtkWidget *map_img = gtk_image_new_from_pixbuf (read_icon ("bt_map.png",0));
	GtkWidget *scaler_in_img = gtk_image_new_from_pixbuf (read_icon ("bt_scale-in.png",0));
	GtkWidget *scaler_out_img = gtk_image_new_from_pixbuf (read_icon ("bt_scale-out.png",0));
	GtkWidget *mute_speech_img = gtk_image_new_from_pixbuf (read_icon ("bt_mute-speech.png",0));
	GtkWidget *poi_find_img = gtk_image_new_from_pixbuf (read_icon ("bt_poi-find.png",0));
	GtkWidget *addwaypoint_img = gtk_image_new_from_pixbuf (read_icon ("bt_add-waypoint.png",0));
	GtkWidget *explore_img = gtk_image_new_from_pixbuf (read_icon ("bt_explore.png",0));
	GtkWidget *trk_restart_img = gtk_image_new_from_pixbuf (read_icon ("bt_track_restart.png",0));
	GtkWidget *trk_clear_img = gtk_image_new_from_pixbuf (read_icon ("bt_track_clear.png",0));

	main_toolbar = gtk_toolbar_new ();
	gtk_toolbar_set_orientation (GTK_TOOLBAR (main_toolbar), GTK_ORIENTATION_VERTICAL);
	gtk_toolbar_set_style (GTK_TOOLBAR (main_toolbar), local_config.buttonsmode);
	gtk_toolbar_set_icon_size (GTK_TOOLBAR (main_toolbar), GTK_ICON_SIZE_LARGE_TOOLBAR);
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (main_toolbar), TRUE);

	if (local_config.guimode == GUI_CAR || local_config.guimode == GUI_PDA)
		menu_expand = TRUE;

	/* Button: Zoom in */
	zoomin_bt = gtk_tool_button_new (zoom_in_img, _("Zoom in"));
	gtk_tool_item_set_tooltip_text (zoomin_bt, _("Zooms in on the current map"));
	gtk_tool_item_set_expand (zoomin_bt, menu_expand);
	g_signal_connect (zoomin_bt, "clicked", G_CALLBACK (zoom_cb), GINT_TO_POINTER (1));
	if (current.zoom >= ZOOM_MAX) gtk_widget_set_sensitive (GTK_WIDGET (zoomin_bt), FALSE);

	/* Button: Zoom out */
	zoomout_bt = gtk_tool_button_new (zoom_out_img, _("Zoom out"));
	gtk_tool_item_set_tooltip_text (zoomout_bt, _("Zooms out on the current map"));
	gtk_tool_item_set_expand (zoomout_bt, menu_expand);
	g_signal_connect (zoomout_bt, "clicked", G_CALLBACK (zoom_cb), GINT_TO_POINTER (2));
	if (current.zoom <= ZOOM_MIN) gtk_widget_set_sensitive (GTK_WIDGET (zoomout_bt), FALSE);

	/* Button: Scaler out */
	scaler_out_bt = gtk_tool_button_new (scaler_out_img, _("Scale out"));
	gtk_tool_item_set_tooltip_text (scaler_out_bt, _("Select the next less detailed map"));
	gtk_tool_item_set_expand (scaler_out_bt, menu_expand);
	g_signal_connect (scaler_out_bt, "clicked", G_CALLBACK (scalerbt_cb), GINT_TO_POINTER (1));

	/* Button: Scaler in */
	scaler_in_bt = gtk_tool_button_new (scaler_in_img, _("Scale in"));
	gtk_tool_item_set_tooltip_text (scaler_in_bt, _("Select the next more detailed map"));
	gtk_tool_item_set_expand (scaler_in_bt, menu_expand);
	g_signal_connect (scaler_in_bt, "clicked", G_CALLBACK (scalerbt_cb), GINT_TO_POINTER (2));

	/* Button: Mute Speech */
	mute_bt = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (mute_bt), _("Mute Speech"));
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (mute_bt), mute_speech_img);
	gtk_tool_item_set_tooltip_text (mute_bt, _("Disable output of speech"));
	if (local_config.mute)
	{
		gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (mute_bt), TRUE);
	}
	gtk_tool_item_set_expand (mute_bt, menu_expand);
	g_signal_connect (mute_bt, "clicked", G_CALLBACK (toggle_toolbutton_cb), &local_config.mute);

	/* Button: Search POIs */
	find_poi_bt = gtk_tool_button_new (poi_find_img, _("_Find POI"));
	gtk_tool_item_set_tooltip_text (find_poi_bt, _("Find Points of Interest"));
	gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (find_poi_bt), TRUE);
	gtk_tool_item_set_expand (find_poi_bt, menu_expand);
	g_signal_connect (find_poi_bt, "clicked", G_CALLBACK (show_poi_lookup_cb), GINT_TO_POINTER (0));

	/* Button: Routing */
	routing_bt = gtk_tool_button_new (NULL, _("_Route List"));
	gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (routing_bt), GTK_STOCK_INDEX);
	gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (routing_bt), TRUE);
	gtk_tool_item_set_expand (routing_bt, menu_expand);
	g_signal_connect (routing_bt, "clicked", G_CALLBACK (route_window_cb), NULL);  

	/* Button: Map Control */
	mapcontrol_bt = gtk_tool_button_new (NULL, _("_Map Control"));
	gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (mapcontrol_bt), GTK_STOCK_PROPERTIES);
	gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (mapcontrol_bt), TRUE);
	gtk_tool_item_set_expand (mapcontrol_bt, menu_expand);
	g_signal_connect (mapcontrol_bt, "clicked", G_CALLBACK (toggle_controlbox_cb), GINT_TO_POINTER (1));

	/* Button: Add Waypoint */
	addwaypoint_bt = gtk_tool_button_new (addwaypoint_img, _("Add _Waypoint"));
	gtk_tool_button_set_use_underline (GTK_TOOL_BUTTON (addwaypoint_bt), TRUE);
	gtk_tool_item_set_expand (addwaypoint_bt, menu_expand);
	g_signal_connect (addwaypoint_bt, "clicked", G_CALLBACK (quickaddwaypoint), NULL);

	/* Button: Explore Mode */
	explore_bt = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (explore_bt), _("Explore Mode"));
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (explore_bt), explore_img);
	gtk_tool_item_set_tooltip_text (mute_bt,
		_("Turn explore mode on. You can move on the map with the "
		"left mouse button click. Clicking near the border switches "
		"to the proximate map."));
	gtk_tool_item_set_expand (explore_bt, menu_expand);
	g_signal_connect (explore_bt, "clicked", G_CALLBACK (explore_cb), GINT_TO_POINTER (1));

	/* Button: Track Clear */
	trk_clear_bt = gtk_tool_button_new (NULL, _("Clear Track"));
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (trk_clear_bt), trk_clear_img);
	gtk_tool_item_set_expand (trk_clear_bt, menu_expand);
	g_signal_connect (trk_clear_bt, "clicked", G_CALLBACK (track_clear_cb), NULL);  

	/* Button: Track Restart */
	trk_restart_bt = gtk_tool_button_new (NULL, _("Restart Track"));
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (trk_restart_bt), trk_restart_img);
	gtk_tool_item_set_expand (trk_restart_bt, menu_expand);
	g_signal_connect (trk_restart_bt, "clicked", G_CALLBACK (track_restart_cb), NULL);  


	/* Main Menu */
	GtkWidget *options_menu = gtk_menu_new ();

	options_bt = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_label (GTK_TOOL_BUTTON (options_bt), _("Options"));
	gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (options_bt), options_img);
	gtk_tool_item_set_expand (options_bt, menu_expand);
	g_signal_connect (options_bt, "clicked", G_CALLBACK (toggle_main_menu_cb), options_menu);

	GtkWidget *menu_maps = gtk_menu_new ();
	GtkWidget *menuitem_maps = gtk_image_menu_item_new_with_mnemonic (_("_Maps"));
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem_maps), map_img);
	GtkWidget *menuitem_mapimport = gtk_menu_item_new_with_mnemonic (_("_Import"));
	GtkWidget *menuitem_mapdownload = gtk_menu_item_new_with_mnemonic (_("_Download"));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_maps);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_maps), menu_maps);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_maps), menuitem_mapimport);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu_maps), menuitem_mapdownload);
	g_signal_connect (menuitem_mapimport, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_MAPIMPORT));
	g_signal_connect (menuitem_mapdownload, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_MAPDOWNLOAD));

	GtkWidget *load_menu = gtk_menu_new ();
	GtkWidget *menuitem_load = gtk_image_menu_item_new_with_mnemonic (_("_Import"));
	GtkWidget *load_img = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem_load), load_img);
	GtkWidget *menuitem_loadtrack = gtk_menu_item_new_with_mnemonic (_("GPX _Track"));
	GtkWidget *menuitem_loadroute = gtk_menu_item_new_with_mnemonic (_("GPX _Route"));
	GtkWidget *menuitem_loadwpt = gtk_menu_item_new_with_mnemonic (_("GPX _Waypoints"));
/* TODO: menu item greyed out because the GPX_WPT code is not yet written (see gpx.c) */
	gtk_widget_set_sensitive(menuitem_loadwpt, FALSE);

	GtkWidget *menuitem_loadtrkold = gtk_menu_item_new_with_mnemonic (_("_GpsDrive Track"));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_load);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_load), load_menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (load_menu), menuitem_loadtrack);
	gtk_menu_shell_append (GTK_MENU_SHELL (load_menu), menuitem_loadroute);
	gtk_menu_shell_append (GTK_MENU_SHELL (load_menu), menuitem_loadwpt);
	gtk_menu_shell_append (GTK_MENU_SHELL (load_menu), menuitem_loadtrkold);
	g_signal_connect (menuitem_loadtrack, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_LOADTRACK));
	g_signal_connect (menuitem_loadroute, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_LOADROUTE));
	g_signal_connect (menuitem_loadwpt, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_LOADWPT));
	g_signal_connect (menuitem_loadtrkold, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_LOADTRKOLD));

	GtkWidget *save_menu = gtk_menu_new ();
	GtkWidget *menuitem_save = gtk_image_menu_item_new_with_mnemonic (_("_Export"));
	GtkWidget *save_img = gtk_image_new_from_stock ("gtk-save", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem_save), save_img);
	GtkWidget *menuitem_savetrack = gtk_menu_item_new_with_mnemonic (_("_Track"));
	GtkWidget *menuitem_saveroute = gtk_menu_item_new_with_mnemonic (_("_Route"));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_save);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_save), save_menu);
	gtk_menu_shell_append (GTK_MENU_SHELL (save_menu), menuitem_savetrack);
	gtk_menu_shell_append (GTK_MENU_SHELL (save_menu), menuitem_saveroute);
	g_signal_connect (menuitem_savetrack, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_SAVETRACK));
	g_signal_connect (menuitem_saveroute, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_SAVEROUTE));

	GtkWidget *menuitem_sendmsg = gtk_image_menu_item_new_with_mnemonic (_("_Send Message"));
	GtkWidget *sendmsg_img = gtk_image_new_from_stock ("gtk-network", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem_sendmsg), sendmsg_img);
	g_signal_connect (menuitem_sendmsg, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_SENDMSG));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_sendmsg);
	if (!local_config.showfriends)
		gtk_widget_set_sensitive (menuitem_sendmsg, FALSE);

	GtkWidget *menuitem_tripreset = gtk_image_menu_item_new_with_mnemonic (_("_Reset Trip"));
	GtkWidget *tripreset_img = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem_tripreset), tripreset_img);
	g_signal_connect (menuitem_tripreset, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_TRIPRESET));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_tripreset);

	GtkWidget *menuitem_settings = gtk_image_menu_item_new_from_stock ("gtk-preferences", NULL);
	g_signal_connect (menuitem_settings, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_SETTINGS));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_settings);

	GtkWidget *menuitem_sep = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_sep);

	GtkWidget *helpabout_menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_ABOUT, NULL);
	g_signal_connect (helpabout_menuitem, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_HELPABOUT));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), helpabout_menuitem);
	
	GtkWidget *helpcontent_menuitem = gtk_image_menu_item_new_from_stock (GTK_STOCK_HELP, NULL);
	g_signal_connect (helpcontent_menuitem, "activate", G_CALLBACK (main_menu_cb), GINT_TO_POINTER (MENU_HELPCONTENT));
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), helpcontent_menuitem);

	menuitem_sep = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_sep);

	GtkWidget *menuitem_quit = gtk_image_menu_item_new_from_stock ("gtk-quit", NULL);
	g_signal_connect (menuitem_quit, "activate", G_CALLBACK (quit_program_cb), NULL);
	gtk_menu_shell_append (GTK_MENU_SHELL (options_menu), menuitem_quit);

	g_signal_connect (options_menu, "selection-done", G_CALLBACK (close_options_menu_cb), NULL);

	gtk_widget_show_all (options_menu);

	rebuild_toolbar_mainbox_cb();

}


/* *****************************************************************************
 * Window: Main -> Status
 */
void create_status_mainbox (void)
{
	GtkWidget *statusdashsub1_box, *statusdashsub2_box;
	GtkWidget *frame_statustime;
	GtkWidget *frame_statusbearing, *frame_statuscourse;
	GtkWidget *frame_statuslat, *frame_statuslon;
	GtkWidget *eventbox_statuslat, *eventbox_statuslon;
	GtkWidget *frame_statusmapscale, *frame_statusprefscale;
	GtkWidget *frame_statusgpsfix;
	GtkWidget *dashboard_1_lb, *dashboard_2_lb;
	GtkWidget *dashboard_3_lb, *dashboard_4_lb;
	GtkWidget *eventbox_dash_1, *eventbox_dash_2;
	GtkWidget *eventbox_dash_3, *eventbox_dash_4;

	gchar sc[15];
	gint t_scalerpos = 0;
	gint t_scale = 0;
	

	if ( mydebug > 11 )
	    fprintf(stderr,"create_status_mainbox\n");

	mainbox_status = gtk_vbox_new (FALSE, 0 * PADDING);


	/* Mini Map Window */
	minimap_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);	
	drawing_minimap = gtk_drawing_area_new ();
	gtk_widget_set_size_request (drawing_minimap, 128, 103);
	g_signal_connect (drawing_minimap, "expose_event", G_CALLBACK (expose_mini_cb), NULL);
	gtk_widget_add_events (GTK_WIDGET (drawing_minimap), GDK_BUTTON_PRESS_MASK);
	g_signal_connect_swapped (drawing_minimap, "button-press-event", G_CALLBACK (minimapclick_cb), drawing_minimap);
	gtk_container_add (GTK_CONTAINER (minimap_window), drawing_minimap);
	drawable_minimap = drawing_minimap->window;


	/* DASHBOARD */
	{
		/* Frame Dashboard 1 */
		frame_dash_1 = gtk_frame_new (" = 1 = ");	
		dashboard_1_lb = gtk_label_new ("---");
		eventbox_dash_1 = gtk_event_box_new ();
		gtk_container_add (GTK_CONTAINER (frame_dash_1),
			dashboard_1_lb);
		gtk_container_add (GTK_CONTAINER (eventbox_dash_1),
			frame_dash_1);
		gtk_widget_add_events (eventbox_dash_1,
			GDK_BUTTON_PRESS_MASK);

		/* Frame Dashboard 2 */
		frame_dash_2 = gtk_frame_new (" = 2 = ");	
		dashboard_2_lb = gtk_label_new ("---");
		eventbox_dash_2 = gtk_event_box_new ();
		gtk_container_add (GTK_CONTAINER (frame_dash_2),
			dashboard_2_lb);
		gtk_container_add (GTK_CONTAINER (eventbox_dash_2),
			frame_dash_2);
		gtk_widget_add_events (eventbox_dash_2,
			GDK_BUTTON_PRESS_MASK);

		/* Frame_Dashboard 3 */
		frame_dash_3 = gtk_frame_new (" = 3 = ");	
		dashboard_3_lb = gtk_label_new ("---");
		eventbox_dash_3 = gtk_event_box_new ();
		gtk_container_add (GTK_CONTAINER (frame_dash_3),
			dashboard_3_lb);
		gtk_container_add (GTK_CONTAINER (eventbox_dash_3),
			frame_dash_3);
		gtk_widget_add_events (eventbox_dash_3,
			GDK_BUTTON_PRESS_MASK);
	
		/* Frame_Dashboard 4 */
		frame_dash_4 = gtk_frame_new (" = 4 = ");	
		dashboard_4_lb = gtk_label_new ("---");
		eventbox_dash_4 = gtk_event_box_new ();
		gtk_container_add (GTK_CONTAINER (frame_dash_4),
			dashboard_4_lb);
		gtk_container_add (GTK_CONTAINER (eventbox_dash_4),
			frame_dash_4);
		gtk_widget_add_events (eventbox_dash_4,
			GDK_BUTTON_PRESS_MASK);
		
#ifndef MAEMO
		if (local_config.guimode == GUI_CAR)
#endif
		{
			g_signal_connect (eventbox_dash_1, "button_press_event",
				GTK_SIGNAL_FUNC (dash_carmenu_cb), (gpointer) 1);
			g_signal_connect (eventbox_dash_2, "button_press_event",
				GTK_SIGNAL_FUNC (dash_carmenu_cb), (gpointer) 2);
			g_signal_connect (eventbox_dash_3, "button_press_event",
				GTK_SIGNAL_FUNC (dash_carmenu_cb), (gpointer) 3);
			g_signal_connect (eventbox_dash_4, "button_press_event",
				GTK_SIGNAL_FUNC (dash_carmenu_cb), (gpointer) 4);
		}
#ifndef MAEMO
		else
		{
			g_signal_connect (eventbox_dash_1, "button_press_event",
				GTK_SIGNAL_FUNC (dash_menu_cb), (gpointer) 1);
			g_signal_connect (eventbox_dash_2, "button_press_event",
				GTK_SIGNAL_FUNC (dash_menu_cb), (gpointer) 2);
			g_signal_connect (eventbox_dash_3, "button_press_event",
				GTK_SIGNAL_FUNC (dash_menu_cb), (gpointer) 3);
			g_signal_connect (eventbox_dash_4, "button_press_event",
				GTK_SIGNAL_FUNC (dash_menu_cb), (gpointer) 4);
		}
#endif
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

		/* Course */
		frame_statuscourse = gtk_frame_new (_("Course"));
		statuscourse_lb = gtk_label_new ("0000");
		gtk_container_add (GTK_CONTAINER (frame_statuscourse),
			statuscourse_lb);

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
		frame_statusgpsfix = gtk_frame_new (NULL);
		drawing_gpsfix = gtk_drawing_area_new ();
		if (local_config.guimode == GUI_CAR)
		{
			local_config.gpsfix_style = TRUE;
			gtk_widget_set_size_request (drawing_gpsfix, 40, 100);
		}
		else
		{
			gtk_widget_set_size_request (drawing_gpsfix, 100, 25);
			local_config.gpsfix_style = FALSE;
		}
		gtk_container_add (GTK_CONTAINER (frame_statusgpsfix), drawing_gpsfix);
		g_signal_connect (drawing_gpsfix, "expose_event", G_CALLBACK (expose_gpsfix), GINT_TO_POINTER (local_config.gpsfix_style));
		gtk_widget_add_events (GTK_WIDGET (drawing_gpsfix), GDK_BUTTON_PRESS_MASK);
		g_signal_connect (drawing_gpsfix, "button-press-event", G_CALLBACK (toggle_satelliteinfo_cb), GINT_TO_POINTER (TRUE));
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips), frame_statusgpsfix,
			_("This shows the GPS Status and Number of satellites in use."), NULL);
	}	/* SMALL STATUS TEXT */


	/* STATUS BAR AND SCALE SLIDER */
	{
		/* Status Bar */
		frame_statusbar = gtk_statusbar_new ();
		gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (frame_statusbar), FALSE);
		current.statusbar_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (frame_statusbar), "main");
		gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, _("GpsDrive (c)2001-2007 F. Ganter"));

	/* Map Scale Slider */
	/* search which scaler_pos is fitting scale_wanted */
	while ( (local_config.scale_wanted > t_scale ) && (t_scalerpos <= slistsize ) )
	{
		t_scalerpos++;
		t_scale = nlist[(gint) rint (t_scalerpos)];
	}
	mapscaler_adj = gtk_adjustment_new (t_scalerpos, 0, slistsize - 1, 1, slistsize / 4, 1 / slistsize );
	mapscaler_scaler = gtk_hscale_new (GTK_ADJUSTMENT (mapscaler_adj));
	gtk_range_set_inverted (GTK_RANGE (mapscaler_scaler), TRUE);
	g_signal_connect (mapscaler_adj, "value_changed", G_CALLBACK (scaler_cb), NULL);
	gtk_scale_set_draw_value (GTK_SCALE (mapscaler_scaler), FALSE);

	mapscaler_hbox = gtk_hbox_new (FALSE, 2 * PADDING);
	GtkWidget *mapscaler_in_lb = gtk_label_new (_("+"));
	GtkWidget *mapscaler_out_lb = gtk_label_new (_("-"));
	gtk_box_pack_start (GTK_BOX (mapscaler_hbox), mapscaler_out_lb, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (mapscaler_hbox), mapscaler_scaler, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (mapscaler_hbox), mapscaler_in_lb, FALSE, FALSE, 3);
	}	/* END STATUS BAR */


	/* Pack all the widgets together according to GUI-Mode */
#ifndef MAEMO
	if (local_config.guimode == GUI_PDA)
	{  /* PDA Mode */
		statusdashboard_box = gtk_table_new (2, 2, TRUE);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box),
			eventbox_dash_1, 0, 1, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box),
			eventbox_dash_2, 1, 2, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box),
			eventbox_dash_3, 0, 1, 1, 2);
		gtk_table_attach_defaults (GTK_TABLE (statusdashboard_box),
			eventbox_dash_4, 1, 2, 1, 2);

		statussmall_box = gtk_vbox_new (FALSE, PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslat, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslon, FALSE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusgpsfix, TRUE, TRUE, 1 * PADDING);

		statusbar_box = gtk_hbox_new (TRUE, PADDING);
//		gtk_box_pack_start (GTK_BOX (statusbar_box), frame_compass, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (statusbar_box), statussmall_box, TRUE, TRUE, 1 * PADDING);

		gtk_box_pack_start (GTK_BOX (mainbox_status), statusdashboard_box, TRUE, FALSE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status), statusbar_box, TRUE, TRUE, 1 * PADDING);
		gtk_box_pack_start (GTK_BOX (mainbox_status), frame_statusbar, TRUE, FALSE, 1* PADDING);
	}
	else
	{	/* Classic Mode (Standard) and Car Mode */
		statusdashboard_box = gtk_hbox_new (FALSE, PADDING);
		statussmall_box = gtk_hbox_new (FALSE, PADDING);
		if (local_config.guimode == GUI_CAR)
		{
			gtk_box_pack_start (GTK_BOX (statusdashboard_box), eventbox_dash_1, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusdashboard_box), eventbox_dash_2, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusdashboard_box), eventbox_dash_3, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusdashboard_box), frame_statusgpsfix, FALSE, FALSE, 1 * PADDING);
		}
		else
		{
			statusdashsub1_box = gtk_hbox_new (FALSE, PADDING);
			statusdashsub2_box = gtk_hbox_new (TRUE, PADDING);
			statusbar_box = gtk_hbox_new (FALSE, PADDING);

			gtk_box_pack_start (GTK_BOX (statusdashsub2_box), eventbox_dash_1, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusdashsub2_box), eventbox_dash_2, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusdashsub2_box), eventbox_dash_3, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusdashboard_box),statusdashsub1_box, FALSE, FALSE, 0);
			gtk_box_pack_start (GTK_BOX (statusdashboard_box),statusdashsub2_box, TRUE, TRUE, 0);

			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statustime, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusfriends, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusbearing, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuscourse, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslat, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statuslon, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusmapscale, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusprefscale, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statussmall_box), frame_statusgpsfix, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusbar_box), frame_statusbar, TRUE, TRUE, 1 * PADDING);
			gtk_box_pack_start (GTK_BOX (statusbar_box), mapscaler_hbox, TRUE, TRUE, 1 * PADDING);
		}

		//  --- ACPI / Temperature / Battery
		create_temperature_widget(statusdashboard_box);
		create_battery_widget(statusdashboard_box);

	}
#endif

#ifdef MAEMO
	hildon_window_add_toolbar (main_window, main_toolbar);
#endif

}


/* *****************************************************************************
 * Window: Main -> Compass
 */
void create_compass_box (void)
{
	GtkWidget *frame_compass;

	frame_compass = gtk_frame_new (NULL);
	drawing_compass = gtk_drawing_area_new ();
	gtk_widget_set_size_request (drawing_compass, 100, 100);
	gtk_container_add (GTK_CONTAINER (frame_compass), drawing_compass);
	g_signal_connect (drawing_compass, "expose_event", G_CALLBACK (expose_compass), NULL);
	gtk_widget_add_events (GTK_WIDGET (drawing_compass), GDK_BUTTON_PRESS_MASK);
	g_signal_connect (drawing_compass, "button-press-event", G_CALLBACK (toggle_satelliteinfo_cb), GINT_TO_POINTER (TRUE));

	compass_box = gtk_hbox_new (FALSE, 1* PADDING);
	gtk_box_pack_start (GTK_BOX (compass_box), frame_compass, TRUE, FALSE, 1 * PADDING);

}


/* *****************************************************************************
 * Create Map Drawable
 */
void create_map_drawable (void)
{
	drawable = gdk_pixmap_new (main_window->window, MAPWIDTH, MAPHEIGHT, -1);
	kontext_map = gdk_gc_new (drawable);
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

	//map_drawingarea = gtk_drawing_area_new ();
	map_drawingarea = gtk_layout_new (NULL, NULL);
	gtk_layout_set_size (GTK_LAYOUT (map_drawingarea), 1280, 1024);

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
 * Window: Main -> Route Info
 */
void create_routeinfo_box (void)
{
	GtkWidget *routeinfo_box;

	routeinfo_evbox = gtk_event_box_new ();
	routeinfo_box = gtk_hbox_new (FALSE, 5);

	route.icon = gtk_image_new ();
	route.label = gtk_label_new ("no routing info");
	gtk_label_set_line_wrap (GTK_LABEL (route.label), TRUE);
	gtk_widget_modify_bg (routeinfo_evbox, GTK_STATE_NORMAL, &colors.darkgrey);

	gtk_box_pack_start (GTK_BOX (routeinfo_box), route.icon, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (routeinfo_box), route.label, FALSE, FALSE, 2);
	gtk_container_add (GTK_CONTAINER (routeinfo_evbox), routeinfo_box);
}


/* *****************************************************************************
 * Window: Main
 */
gint create_main_window (void)
{
	gchar main_title[100];
	GdkPixbuf *mainwindow_icon_pixbuf;

	if ( mydebug > 11 )
	    fprintf(stderr,"create_main_window\n");

	if (!local_config.embeddable_gui)
	{
		/* default application mode */
#ifdef MAEMO
			main_window = hildon_window_new ();
			program = hildon_program_get_instance ();
			hildon_program_add_window (program, HILDON_WINDOW (main_window));
#else
	 		main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
			g_snprintf (main_title, sizeof (main_title),
				"%s v%s", "GpsDrive", VERSION);
			gtk_window_set_title (GTK_WINDOW (main_window), main_title);
#endif
	}
	else
	{
		/* embeddable mode means no GUI appears until it is
		 * hooked into a GTK Socket widget in a separate application.
		 * Thus, we print out the window ID here so that the other
		 * app can pass that to the GTK Socket's add_id() method. */
		main_window = gtk_plug_new(0);
		/* gtk_plug_get_id() returns a GdkNativeWindow, which is 
		 * really a guint32 on all platforms according to:
		 * * https://www.linux-foundation.org/dbadmin/browse/type_single.php?cmd=list-by-id&Tid=12788
		 */
		fprintf(stdout, "Main Window ID is 0x%08x\n",
			gtk_plug_get_id(GTK_PLUG(main_window)));
	}

	gtk_container_set_border_width (GTK_CONTAINER (main_window), 0);
	gtk_window_set_position (GTK_WINDOW (main_window), GTK_WIN_POS_CENTER);
	g_signal_connect (GTK_OBJECT (main_window), "delete_event", GTK_SIGNAL_FUNC (quit_program_cb), NULL);
	g_signal_connect (GTK_OBJECT (main_window), "key_press_event", GTK_SIGNAL_FUNC (key_pressed_cb), NULL);
	g_signal_connect (main_window, "window-state-event", G_CALLBACK (window_state_cb), NULL);


	mainwindow_icon_pixbuf = read_icon ("gpsdrive16.png",1);
	if (mainwindow_icon_pixbuf)
	{
		gtk_window_set_icon (GTK_WINDOW (main_window), mainwindow_icon_pixbuf);
		gdk_pixbuf_unref (mainwindow_icon_pixbuf);
	}

	main_tooltips = gtk_tooltips_new();

	/* Create the parts of the main window */
	create_toolbar_mainbox ();
	create_status_mainbox ();
	create_map_mainbox ();
	create_mapcontrol_box ();
	create_routeinfo_box ();
	create_compass_box ();

#ifdef MAEMO
	create_dashboard_carmenu ();
#else
	if (local_config.guimode == GUI_CAR)
		create_dashboard_carmenu ();
	else
		create_dashboard_menu ();
#endif

	create_satellite_window ();

#ifndef MAEMO
	if (local_config.guimode == GUI_PDA)
	{  /* PDA Mode */
		GtkWidget *map_lb, *controls_lb, *status_lb;
		
		main_table = gtk_notebook_new ();
		
		map_lb = gtk_label_new (_("Map"));
		gtk_notebook_append_page (GTK_NOTEBOOK (main_table),
			mainframe_map, map_lb);
		controls_lb = gtk_label_new (_("Controls"));
		gtk_notebook_append_page (GTK_NOTEBOOK (main_table),
			main_toolbar, controls_lb);
		status_lb = gtk_label_new (_("Status"));
		gtk_notebook_append_page (GTK_NOTEBOOK (main_table),
			mainbox_status, status_lb);
		gtk_container_add (GTK_CONTAINER (main_window), main_table);
	}
	else
	{  /* Classic Mode (Standard) and Car Mode */
		GtkWidget *main_box = gtk_vbox_new (FALSE, 0);
		GtkWidget *main_upper_table = gtk_table_new (2, 2, FALSE);
		GtkWidget *main_lower_table = gtk_table_new (3, 2, FALSE);

		gtk_table_attach (GTK_TABLE (main_upper_table), main_toolbar, 0, 1, 0, 2, GTK_SHRINK, GTK_EXPAND | GTK_FILL, 0, 0);
		gtk_table_attach (GTK_TABLE (main_upper_table), routeinfo_evbox, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
		gtk_table_attach_defaults (GTK_TABLE (main_upper_table), mainframe_map, 1, 2, 1, 2);
		gtk_table_attach (GTK_TABLE (main_lower_table), compass_box, 0, 1, 0, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
		gtk_table_attach (GTK_TABLE (main_lower_table), statusdashboard_box, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
		if (local_config.guimode != GUI_CAR)
		{
			gtk_table_attach (GTK_TABLE (main_lower_table), statussmall_box, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
			gtk_table_attach (GTK_TABLE (main_lower_table), statusbar_box, 0, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
		}
		gtk_box_pack_start (GTK_BOX (main_box), main_upper_table, TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (main_box), main_lower_table, FALSE, FALSE, 0);
		gtk_container_add (GTK_CONTAINER (main_window), main_box);
	}

	if (local_config.guimode == GUI_CAR)
	{
		gtk_window_maximize (GTK_WINDOW (main_window));
		gtk_window_set_decorated (GTK_WINDOW (main_window), FALSE);
	}
#endif

    // let the window manager show stuff before we continue
    while (gtk_events_pending()) gtk_main_iteration();

	return 0;
}

