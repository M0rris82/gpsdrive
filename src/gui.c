/*
**********************************************************************

Copyright (c) 2001-2007 Fritz Ganter <ganter@ganter.at>
Copyright (c) 2007 Guenther Meyer <d.s.e (at) sordidmusic.com>

Website: www.gpsdrive.de


Copyright (c) 2007 Ross Scanlon <ross@theinternethost.com.au>

Website: www.4x4falcon.com/gpsdrive/

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
     * gui.c  
     * The purpose of this module is to do the setup
     * and resizing of user interface for gspdrive
     *
     * It also contains some generic windows and popups
     * that can be used in various functions
     */

    /*
      4 Feb 2007  Version 0.1 
      Move sizing of window to this file from gpsdrive.c
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
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#include "gettext.h"
#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "gui.h"
#include "poi.h"
#include "poi_gui.h"
#include "main_gui.h"
#include "routes.h"
#include "track.h"
#include "mapnik.h"

/*  Defines for gettext I18n */
#include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#define PFSIZE 20

extern gint do_unit_test;

    /* External Values */

extern gint mydebug;

extern GtkListStore *poi_result_tree;
extern GtkListStore *route_list_tree;

extern gdouble wp_saved_target_lat;
extern gdouble wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat;
extern gdouble wp_saved_posmode_lon;
extern color_struct colors;
extern currentstatus_struct current;

extern GtkWidget *poi_types_window;
extern GtkWidget *frame_statusfriends;
extern GtkWidget *mute_bt;
extern GtkWidget *frame_battery;
extern GtkWidget *frame_temperature;

// Some of these shouldn't be necessary, when all the gui stuff is finally moved
extern GtkWidget *map_drawingarea;
extern GdkGC *kontext_map;
extern GtkWidget *routeinfo_evbox;
extern gint havefestival;

extern gint real_psize, real_smallmenu;

GdkColormap *colmap;
color_struct colors;
guistatus_struct gui_status;

GdkPixbuf *posmarker_img, *targetmarker_img;
GdkCursor *cursor_cross;
GdkCursor *cursor_watch;

gint PSIZE;

GtkWidget *main_window;
GtkWidget *mapnik_bt;


	extern GtkAdjustment *adj_h, *adj_v;


/****
     function get_window_sizing
     This sets the main window dimensions relative to the screen size
     if no valid --geometry is given
****/
static int get_window_sizing (void)
{

	guint screen_width, screen_height;
	gui_status.app_width = 160;
	gui_status.app_height = 160;

	PSIZE = 50;
	SMALLMENU = 0;

	screen_width = gdk_screen_width ();
	screen_height = gdk_screen_height ();

	if (screen_height >= 1024)		 /* > 1280x1024 */
	    {
		gui_status.app_width = min(1280,screen_width-300);
		gui_status.app_height = min(1024,screen_height-200);
		if ( mydebug > 0 )
			g_print ("Set application window size to %d,%d\n",
				gui_status.app_width, gui_status.app_height);
	    }
	else if (screen_height >= 768)	/* 1024x768 */
	    {
		gui_status.app_width = 800;
		gui_status.app_height = 540;
	    }
	else if (screen_height >= 750)	/* 1280x800 */
	    {
		gui_status.app_width = 1140;
		gui_status.app_height = 600;
	    }
	else if (screen_height >= 480)	/* 640x480 */
	    {
		if (screen_width == 0)
		    gui_status.app_width = 630;
		else
		    gui_status.app_width = screen_width - XMINUS;
		gui_status.app_width = screen_height - YMINUS;
	    }
	else if (screen_height < 480)
	    {
		if (screen_width == 0)
		    gui_status.app_width = 220;
		else
		    gui_status.app_width = screen_width - XMINUS;
		gui_status.app_height = screen_height - YMINUS;
		PSIZE = 25;
		SMALLMENU = 1;
	    }


    /** from gpsdrive.c line 4824 to 4857 */

    if (   ((screen_width  == 240) && (screen_height == 320)) 
	   || ((screen_height == 240) && (screen_width  == 320)) )
	{
	    local_config.guimode = GUI_PDA;
	    // SMALLMENU = 1;
	    gui_status.app_width = screen_width - 8;
	    gui_status.app_height = screen_height - 70;
	}
    if (local_config.guimode == GUI_PDA)
	{
	    g_print ("\nPDA mode\n");
	}

    PSIZE = 50;

	gtk_window_set_default_size (GTK_WINDOW (main_window),
		gui_status.app_width, gui_status.app_height);

	if ( mydebug > 10 )
	{
		g_print ("Screen size is %d,%d\n", screen_width, screen_height);
		g_print ("Setting map screen size to %d,%d\n", gui_status.app_width, gui_status.app_height);
	}

    return 0;
}


/* *****************************************************************************
 * Generic Callback to handle toggle- and checkbuttons
 */
int
toggle_button_cb (GtkWidget *button, gboolean *value)
{
	*value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

	current.needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 * Popup: Are you sure, y/n
 */
gint popup_yes_no (GtkWindow *parent, gchar *message)
{
	GtkDialog *dialog_yesno;
	gint response_id;
	gchar question[200];

	if (message != NULL)
		g_strlcpy (question, message, sizeof (question));
	else
		g_strlcpy (question, "Are you sure?", sizeof (question));

	if (mydebug >10)
		fprintf (stderr, "POPUP: Question\n");

	dialog_yesno = GTK_DIALOG (gtk_message_dialog_new (parent,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		"%s", question));
	response_id = gtk_dialog_run (dialog_yesno);
	gtk_widget_destroy (GTK_WIDGET (dialog_yesno));
	return response_id;
}

/* *****************************************************************************
 * Popup: Warning, ok
 */
gint popup_warning (GtkWindow *parent, gchar *message)
{
	GtkDialog *dialog_warning;
	gchar warning[80];

	if (mydebug >10)
		fprintf (stderr, "POPUP: Warning\n");

	if (!parent)
		parent = GTK_WINDOW (main_window);
	if (message)
		g_strlcpy (warning, message, sizeof (warning));
	else
		g_strlcpy (warning, _("Press OK to continue!"),
			sizeof (warning));

	dialog_warning = GTK_DIALOG (gtk_message_dialog_new (parent,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_WARNING,
		GTK_BUTTONS_OK,
		"%s", warning));

	gdk_beep ();
	gtk_widget_show_all (GTK_WIDGET (dialog_warning));

	g_signal_connect_swapped (dialog_warning, "response",
		G_CALLBACK (gtk_widget_destroy), dialog_warning);
	return TRUE;
}

/* *****************************************************************************
 * Popup: Error, ok
 */
gint popup_error (GtkWindow *parent, gchar *message)
{
	GtkDialog *dialog_error;
	gint response_id;
	gchar error[80];

	if (mydebug >10)
		fprintf (stderr, "POPUP: Error\n");

	if (!parent)
		parent = GTK_WINDOW (main_window);
	if (message)
		g_strlcpy (error, message, sizeof (error));
	else
		g_strlcpy (error,
			_("An error has occured.\nPress OK to continue!"),
			sizeof (error));

	dialog_error = GTK_DIALOG (gtk_message_dialog_new (parent,
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_OK,
		"%s", error));

	gdk_beep ();
	
	response_id = gtk_dialog_run (dialog_error);
	gtk_widget_destroy (GTK_WIDGET (dialog_error));
	return response_id;
}

/* *****************************************************************************
 * Popup: Info, ok
 */
gint popup_info (GtkWindow *parent, gchar *message)
{
	GtkDialog *dialog_info;
	gchar info[80];

	if (mydebug >10)
		fprintf (stderr, "POPUP: Info\n");

	if (!parent)
		parent = GTK_WINDOW (main_window);
	if (message)
		g_strlcpy (info, message, sizeof (info));
	else
		g_strlcpy (info, _("Press OK to continue!"),
			sizeof (info));

	dialog_info = GTK_DIALOG (gtk_message_dialog_new (parent,
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK,
		"%s", info));

	gdk_beep ();
	gtk_widget_show_all (GTK_WIDGET (dialog_info));

	g_signal_connect_swapped (dialog_info, "response",
		G_CALLBACK (gtk_widget_destroy), dialog_info);
	return TRUE;
}


/* *****************************************************************************
 * Button: Add new Waypoint
 */
void create_button_add_wp (void)
{
	GtkWidget *add_wp_bt;
	gchar imagefile[400];
	GdkPixbuf *pixmap = NULL;
	
	local_config.showaddwpbutton = TRUE; // TODO: add this to settings
	
	if (local_config.showaddwpbutton)
	{
		g_snprintf (imagefile, sizeof (imagefile), "%s/gpsdrive/%s", DATADIR, "pixmaps/add_waypoint.png");
		pixmap = gdk_pixbuf_new_from_file (imagefile, NULL);
		if (imagefile == NULL)
		{
			fprintf (stderr, _("\nWarning: unable to open 'add waypoint' picture\nPlease install the program as root with:\nmake install\n\n"));
			return;
		}
		
		add_wp_bt = gtk_button_new ();
		gtk_button_set_relief (GTK_BUTTON (add_wp_bt), GTK_RELIEF_NONE);
		gtk_button_set_image (GTK_BUTTON (add_wp_bt), GTK_WIDGET (pixmap));
		gtk_container_add (GTK_CONTAINER (map_drawingarea), add_wp_bt);
		gtk_widget_show_all (add_wp_bt);
		return;
	}
	else
	{
		return;
	}
}


/* *****************************************************************************
 * parse the color string allocate the color
 */
static void
init_color (gchar *tcolname, GdkColor *tcolor)
{
	gdk_color_parse (tcolname, tcolor);
	gdk_colormap_alloc_color (colmap, tcolor, FALSE, TRUE);
}

/* *****************************************************************************
 * get color string from GdkColor struct
 * (needed to save settings in config file)
 * returned string has to be freed after usage!
 */
gchar
*get_colorstring (GdkColor *tcolor)
{
	guint r, g, b;
	gchar *cstring;
	
	r = tcolor->red / 256;
	g = tcolor->green / 256;
	b = tcolor->blue / 256;

	cstring = g_malloc (sizeof (gchar)*7+1);
	g_snprintf (cstring, 8, "#%02x%02x%02x", r, g, b);
	
	if (mydebug > 20)
		fprintf (stderr, "get_colorstring result: %s\n", cstring);

	return cstring;
}

/* *****************************************************************************
 * switch nightmode on or off
 *
 * TODO:
 *	Currently the Nightmode is represented by changing only the
 *      the bg_color of the main window and the map colors for mapnik,
 *      but the rest of the GUI stays the same.
 *	Maybe we should switch the gtk-theme instead, so that the complete
 *	GUI changes the colors. But this has time until after the 2.10 release.
 */
gint switch_nightmode (gboolean value)
{
	gint t_res = FALSE;

	if (value && !gui_status.nightmode)
	{
		gtk_widget_modify_bg (main_window, GTK_STATE_NORMAL, &colors.night);
		gui_status.nightmode = TRUE;
		if (mydebug > 1)
			g_print ("switching to night view\n");
		t_res = TRUE;
	}
	else if (!value && gui_status.nightmode)
	{
		gtk_widget_modify_bg (main_window, GTK_STATE_NORMAL, NULL);
		gui_status.nightmode = FALSE;
		if (mydebug > 1)
			g_print ("switching to daylight view\n");
		t_res = TRUE;
	}

#ifdef MAPNIK
	/* reinit mapnik to reflect bg_color changes */
	if ( t_res && local_config.MapnikStatusInt )
	{
		if ( gen_mapnik_config_xml_ysn(local_config.mapnik_xml_file,
		   (char*) g_get_user_name(), value ))
		{
			init_mapnik(local_config.mapnik_xml_file);
		}
		else
		{
			fprintf(stderr,"Could not init Mapnik, disabling Mapnik Support!\n");
			local_config.MapnikStatusInt = 0; // <-- disable mapnik
		}
	}
#endif

	return t_res;
}


/* *****************************************************************************
 * Draw position marker, also showing direction
 * style sets the pinter style to be drawn:
 *  0 = full circle position pointer
 *  1 = only arrow pointer
 *  2 = only light crosshair pointer
 *  3 = plus sign
 *  4 = slim arrow for friendsd
 */
gboolean
draw_posmarker (
	gdouble posx, gdouble posy,
	gdouble direction, GdkColor *color, gint style,
	gboolean shadow, gboolean outline)
{
	gdouble w;
	GdkPoint poly[16];

	if (shadow)
		gdk_gc_set_function (kontext_map, GDK_AND);
	else
		gdk_gc_set_function (kontext_map, GDK_COPY);
	
	gdk_gc_set_foreground (kontext_map, color);
	
	w = direction + M_PI;

	if (style == 0 && shadow == FALSE)
	{
		/* draw position icon */
		gdk_draw_pixbuf (drawable, kontext_map, posmarker_img,
			0, 0, posx - 20, posy - 20,
			-1, -1, GDK_RGB_DITHER_NONE, 0, 0);
	}

	if (style == 0 && shadow == TRUE)
	{
		/* draw shadow of position icon */
		gdk_draw_arc (drawable, kontext_map, TRUE, posx-15, posy-15,
			40, 40, 0, 360 * 64);
	}

	if (style == 0 || style == 1)
	{
		/* draw arrow pointer */
		gdk_gc_set_line_attributes (kontext_map, 3, 0, 0, 0);
		poly[0].x =
			posx + PFSIZE * (cos (w + M_PI_2));
		poly[0].y =
			posy + PFSIZE * (sin (w + M_PI_2));
		poly[1].x = posx
			+ PFSIZE / 2 * (cos (w + M_PI))
			- PFSIZE / 1.5 * (cos (w + M_PI_2));
		poly[1].y = posy
			+ PFSIZE / 2 * (sin (w + M_PI))
			- PFSIZE / 1.5 * (sin (w + M_PI_2));
		poly[2].x =
			posx - PFSIZE / 3 * (cos (w + M_PI_2));
		poly[2].y =
			posy - PFSIZE / 3 * (sin (w + M_PI_2));
		poly[3].x = posx
			- PFSIZE / 2 * (cos (w + M_PI))
			- PFSIZE / 1.5 * (cos (w + M_PI_2));
		poly[3].y = posy
			- PFSIZE / 2 * (sin (w + M_PI))
			- PFSIZE / 1.5 * (sin (w + M_PI_2));
		poly[4].x = poly[0].x;
		poly[4].y = poly[0].y;
		gdk_draw_polygon (drawable, kontext_map, TRUE,
			(GdkPoint *) poly, 5);
		/* draw outline */
		if (outline)
		{
			gdk_gc_set_foreground (kontext_map, &colors.white);
			gdk_gc_set_line_attributes (kontext_map, 1, 0, 0, 0);		
			gdk_draw_polygon (drawable, kontext_map, FALSE,
				(GdkPoint *) poly, 5);
		}
	}

	if (style == 2)
	{
		/* draw crosshair pointer */
		gdk_gc_set_line_attributes (kontext_map, 3, 0, 0, 0);
		gdk_draw_line (drawable, kontext_map,
			posx + PFSIZE * 0.5 * (cos (w + M_PI)),
			posy + PFSIZE * 0.5 * (sin (w + M_PI)),
			posx - PFSIZE * 0.5 * (cos (w + M_PI)),
			posy - PFSIZE * 0.5 * (sin (w + M_PI)));
		gdk_draw_line (drawable, kontext_map, posx, posy,
			posx + PFSIZE * (cos (w + M_PI_2)),
			posy + PFSIZE * (sin (w + M_PI_2)));
	}

	if (style == 3)
	{
		/*  draw + sign at position */
		gdk_gc_set_line_attributes (kontext_map, 4, 0, 0, 0);
		gdk_draw_line (drawable, kontext_map, posx + 1,
			posy + 1 - 10, posx + 1, posy + 1 - 2);
		gdk_draw_line (drawable, kontext_map, posx + 1,
			posy + 1 + 2, posx + 1, posy + 1 + 10);
		gdk_draw_line (drawable, kontext_map, posx + 1 + 10,
			posy + 1, posx + 1 + 2, posy + 1);
		gdk_draw_line (drawable, kontext_map, posx + 1 - 2,
			posy + 1, posx + 1 - 10, posy + 1);
	}

	if (style == 4)
	{
		/* draw slim arrow */
		gdk_gc_set_line_attributes (kontext_map, 3, 0, 0, 0);
		poly[0].x = posx + PFSIZE * 2 * (cos (w + M_PI_2));
		poly[0].y = posy + PFSIZE * 2 * (sin (w + M_PI_2));

		poly[1].x = posx + PFSIZE * 0.2 * (cos (w + M_PI));
		poly[1].y = posy + PFSIZE * 0.2 * (sin (w + M_PI));
		poly[2].x = posx - PFSIZE * 0.2 * (cos (w + M_PI));
		poly[2].y = posy - PFSIZE * 0.2 * (sin (w + M_PI));
		poly[3].x = poly[0].x;
		poly[3].y = poly[0].y;
		gdk_draw_polygon (drawable, kontext_map, TRUE,
			(GdkPoint *) poly, 4);
	}

	return TRUE;
}


/*
 * function to set cursors styles
 */
gint
set_cursor_style(int cursor) {
	switch(cursor) {
		case CURSOR_DEFAULT:
			/* different cursors in posmode */
			if (gui_status.posmode == TRUE )
				gdk_window_set_cursor (GTK_LAYOUT (map_drawingarea)->bin_window, cursor_cross);
			else
				gdk_window_set_cursor (GTK_LAYOUT (map_drawingarea)->bin_window, NULL);
			break;
		case CURSOR_WATCH:
			gdk_window_set_cursor(GTK_LAYOUT (map_drawingarea)->bin_window, cursor_watch);
		case CURSOR_CROSS:
			gdk_window_set_cursor(GTK_LAYOUT (map_drawingarea)->bin_window, cursor_cross);
	}
	/* update all events to fastly switch cursor */
	while (gtk_events_pending())
		gtk_main_iteration();
	return 0;
}


/* *****************************************************************************
 * GUI Init Main
 * This will call all the necessary functions to init the graphical interface
 */
int gui_init (gchar *geometry, gint usegeometry)
{
	gint app_x, app_y;

	if ( mydebug > 0 )
		fprintf (stderr, "Initializing GUI\n");

#ifdef MAPNIK
	/* init mapnik before gui */
	if ( gen_mapnik_config_xml_ysn(local_config.mapnik_xml_file, (char*) g_get_user_name(), 
	     gui_status.nightmode ))
	{
		init_mapnik(local_config.mapnik_xml_file);
	}
	else
	{
		fprintf(stderr,"Could not init Mapnik, disabling Mapnik Support!\n");
		local_config.MapnikStatusInt = 0; // <-- disable mapnik
	}
#endif

	/* init colors */
	colmap = gdk_colormap_get_system ();
	init_color (local_config.color_track, &colors.track);
	init_color (local_config.color_route, &colors.route);
	init_color (local_config.color_friends, &colors.friends);
	init_color (local_config.color_wplabel, &colors.wplabel);
	init_color (local_config.color_dashboard, &colors.dashboard);
	
	init_color ("#a0a0a0", &colors.shadow); 
	// TODO: see gui.h
	init_color ("#a00000", &colors.night);
	init_color ("#000000", &colors.black);
	init_color ("#ff0000", &colors.red);
	init_color ("#800000", &colors.red_dark);
	init_color ("#ffffff", &colors.white);
	init_color ("#0000ff", &colors.blue);
	init_color ("#8b958b", &colors.lcd);
	init_color ("#737d6a", &colors.lcd2);
	init_color ("#ffff00", &colors.yellow);
	init_color ("#00b000", &colors.green);
	init_color ("#00ff00", &colors.green_light);
	init_color ("#d5d5d5", &colors.mygray);
	init_color ("#a5a5a5", &colors.textback);
	init_color ("#4076cf", &colors.textbacknew);
	init_color ("#a0a0a0", &colors.grey);
	init_color ("#f06000", &colors.orange);
	init_color ("#f0995f", &colors.lightorange);
	init_color ("#ff8000", &colors.orange2);
	init_color ("#404040", &colors.darkgrey); 
	init_color ("#d0d0d0", &colors.lightgrey);

	posmarker_img = read_icon ("posmarker.png", 0);
	targetmarker_img = read_icon ("targetmarker.png", 0); 

	/* init poi search dialog (cached) */
	create_window_poi_lookup();
	
	/* init size and contents of the main window */
	create_main_window();
	if (usegeometry && local_config.guimode != GUI_CAR)
	{
		usegeometry = gtk_window_parse_geometry (GTK_WINDOW (main_window), geometry);
		if (!usegeometry)
			fprintf(stderr, "Failed to parse %s\n", geometry);
	}
	if (!usegeometry)
		get_window_sizing ();
	gtk_widget_show_all (main_window);
	gtk_widget_hide_all (routeinfo_evbox);

	//if ( ( local_config.guimode == GUI_DESKTOP )
#ifdef MAPNIK
	if( (local_config.MapnikStatusInt ) )
	{
		toggle_mapnik_cb( mapnik_bt, 2 );
	}
#endif
	
	if (!local_config.showfriends)
      if (G_IS_OBJECT(frame_statusfriends))
		gtk_widget_hide_all (frame_statusfriends);
	if (!local_config.speech && !havefestival)
      if (G_IS_OBJECT(mute_bt))
		gtk_widget_hide_all (mute_bt);
	if (!local_config.showbatt)
      if (G_IS_OBJECT(frame_battery))
		gtk_widget_hide_all (frame_battery);
	if (!local_config.showtemp)
      if (G_IS_OBJECT(frame_temperature))
		gtk_widget_hide_all (frame_temperature);

	/* init map view:
	 * this has to be done after the main window has been realized
	 * because now we now the necessary map size */
	create_map_drawable ();

	// TODO: create_button_add_wp();


// the following lines habe been moved from gpsdrive.c to here.
// maybe something has to be sorted out:
{	
	GtkStyle *style;
	style = gtk_rc_get_style (main_window);
	colors.normal = style->bg[GTK_STATE_NORMAL];
}

	/* set cross cursor for map posmode */
	cursor_cross = gdk_cursor_new(GDK_TCROSS);
	/* set watch cursor used e.g. when rendering a mapnik map*/
	cursor_watch = gdk_cursor_new(GDK_WATCH);
	
	gtk_window_set_auto_startup_notification (TRUE);

	if ( mydebug > 5 )
	{
		gtk_window_get_size (GTK_WINDOW (main_window), &app_x, &app_y);
		fprintf(stderr , "size of application : %dx%d px\n", app_x, app_y);
	}

	gui_status.dl_window = FALSE;

	return 0;
}
