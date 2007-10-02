/*
**********************************************************************

Copyright (c) 2001-2007 Fritz Ganter <ganter@ganter.at>

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
     */

    /*
      4 Feb 2007  Version 0.1 
      Move sizing of window to this file from gpsdrive.c
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
#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "gui.h"
#include "poi.h"
#include "poi_gui.h"
#include "main_gui.h"
#include "routes.h"
#include "track.h"

/*  Defines for gettext I18n */
#include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#define PFSIZE 15

extern gint do_unit_test;

    /* External Values */

extern gint mydebug;
extern gint debug;

extern GtkTreeStore *poi_types_tree;
extern GtkListStore *poi_result_tree;
extern GtkListStore *route_list_tree;

extern gdouble wp_saved_target_lat;
extern gdouble wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat;
extern gdouble wp_saved_posmode_lon;
extern gint usesql;
extern color_struct colors;
extern currentstatus_struct current;

extern GtkWidget *poi_types_window;

// Some of these shouldn't be necessary, when all the gui stuff is finally moved
extern GtkWidget *find_poi_bt;
extern GtkWidget *map_drawingarea;
extern GdkGC *kontext_map;

extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu;
extern gint SCREEN_X_2, SCREEN_Y_2;

GdkColormap *colmap;
color_struct colors;
guistatus_struct gui_status;

GdkPixbuf *posmarker_img, *targetmarker_img;
GdkCursor *cursor_cross;
GdkCursor *cursor_watch;

gint PSIZE;

extern gint borderlimit;

extern gdouble posx, posy;

GtkWidget *main_window;


/* included from freedesktop.org source, copyright as below do not change anything in between here and function get_window_sizing */

/*

Copyright 1985, 1986, 1987,1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/X11/ParseGeom.c,v 1.2 2001/10/28 03:32:30 tsi Exp $ */

/*
  #include "Xlibint.h"
  #include "Xutil.h"
*/

#include <X11/Xlibint.h>
#include <X11/Xutil.h>

#ifdef notdef
/* 
 *Returns pointer to first char ins search which is also in what, else NULL.
 */
static char *strscan (search, what)
     char *search, *what;
{
    int i, len = strlen (what);
    char c;

    while ((c = *(search++)) != NULL)
	for (i = 0; i < len; i++)
	    if (c == what [i])
		return (--search);
    return (NULL);
}
#endif

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */

static int
ReadInteger(char *string, char **NextString)
{
    register int Result = 0;
    int Sign = 1;
    
    if (*string == '+')
	string++;
    else if (*string == '-')
	{
	    string++;
	    Sign = -1;
	}
    for (; (*string >= '0') && (*string <= '9'); string++)
	{
	    Result = (Result * 10) + (*string - '0');
	}
    *NextString = string;
    if (Sign >= 0)
	return (Result);
    else
	return (-Result);
}

int XParseGeometry (
		    _Xconst char *string,
		    int *x,
		    int *y,
		    unsigned int *width,    /* RETURN */
		    unsigned int *height)    /* RETURN */
{
    int mask = NoValue;
    register char *strind;
    unsigned int tempWidth = 0, tempHeight = 0;
    int tempX = 0, tempY = 0;
    char *nextCharacter;

    if ( (string == NULL) || (*string == '\0')) return(mask);
    if (*string == '=')
	string++;  /* ignore possible '=' at beg of geometry spec */

    strind = (char *)string;
    if (*strind != '+' && *strind != '-' && *strind != 'x') {
	tempWidth = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter) 
	    return (0);
	strind = nextCharacter;
	mask |= WidthValue;
    }

    if (*strind == 'x' || *strind == 'X') {	
	strind++;
	tempHeight = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	    return (0);
	strind = nextCharacter;
	mask |= HeightValue;
    }

    if ((*strind == '+') || (*strind == '-')) {
	if (*strind == '-') {
	    strind++;
	    tempX = -ReadInteger(strind, &nextCharacter);
	    if (strind == nextCharacter)
		return (0);
	    strind = nextCharacter;
	    mask |= XNegative;

	}
	else
	    {	strind++;
	    tempX = ReadInteger(strind, &nextCharacter);
	    if (strind == nextCharacter)
		return(0);
	    strind = nextCharacter;
	    }
	mask |= XValue;
	if ((*strind == '+') || (*strind == '-')) {
	    if (*strind == '-') {
		strind++;
		tempY = -ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return(0);
		strind = nextCharacter;
		mask |= YNegative;

	    }
	    else
		{
		    strind++;
		    tempY = ReadInteger(strind, &nextCharacter);
		    if (strind == nextCharacter)
			return(0);
		    strind = nextCharacter;
		}
	    mask |= YValue;
	}
    }
	
    /* If strind isn't at the end of the string the it's an invalid
       geometry specification. */

    if (*strind != '\0') return (0);

    if (mask & XValue)
	*x = tempX;
    if (mask & YValue)
	*y = tempY;
    if (mask & WidthValue)
	*width = tempWidth;
    if (mask & HeightValue)
	*height = tempHeight;
    return (mask);
}


/* do not change the above */

/****
     function get_window_sizing
     This creates the main window sizing from either --geometry or based on screen size if no --geometry
     as at 4 February 2007 will also use -s -r from command line but will be removed eventually
****/

int get_window_sizing (gchar *geom, gint usegeom, gint screen_height, gint screen_width) {

    int ret;
    uint width, height;
    int x, y;



    if ( mydebug >= 0 ) {
	fprintf (stderr, "Creating main window\n");
    }

    /*** do we have geometry from the command line ***/

    if (usegeom) {

	ret = XParseGeometry (geom, &x, &y, &width, &height);


	/* if geometry returns a width then use that otherwise use default for 640x480 */

	if (ret & WidthValue) {
         if (width > TILEWIDTH) {
          real_screen_x = TILEWIDTH;
          }
         else {
          real_screen_x = width;
          }
	}
	else {
	    real_screen_x = 630;
	}

	/* if geometry returns a height then use that otherwise use default for 640x480 */

	if (ret & HeightValue) {
         if (height > TILEHEIGHT) {
          real_screen_y = TILEHEIGHT;
          }
         else {
          real_screen_y = height;
          }
	}
	else {
	    real_screen_y = screen_height - YMINUS;
	}
    }
    else {

	/** from gpsdrive.c line 4773 to 4814 */

	PSIZE = 50;
	SMALLMENU = 0;
	if (screen_height >= 1024)		 /* > 1280x1024 */
	    {
		real_screen_x = min(1280,screen_width-300);
		real_screen_y = min(1024,screen_height-200);
		if ( mydebug > 0 )
		    g_print ("Set real Screen size to %d,%d\n", 
			     real_screen_x,real_screen_y);
		
	    }
	else if (screen_height >= 768)	/* 1024x768 */
	    {
		real_screen_x = 800;
		real_screen_y = 540;
	    }
	else if (screen_height >= 750)	/* 1280x800 */
	    {
		real_screen_x = 1140;
		real_screen_y = 600;
	    }
	else if (screen_height >= 480)	/* 640x480 */
	    {
		if (screen_width == 0)
		    real_screen_x = 630;
		else
		    real_screen_x = screen_width - XMINUS;
		real_screen_y = screen_height - YMINUS;
	    }
	else if (screen_height < 480)
	    {
		if (screen_width == 0)
		    real_screen_x = 220;
		else
		    real_screen_x = screen_width - XMINUS;
		real_screen_y = screen_height - YMINUS;
		PSIZE = 25;
		SMALLMENU = 1;
	    }
    }  /*** if usegeom  */

    /** from gpsdrive.c line 4824 to 4857 */

    if ((local_config.guimode == GUI_XWIN) && (screen_width != 0))
	{
	    real_screen_x += XMINUS - 10;
	    real_screen_y += YMINUS - 30;
	}

    if (   ((screen_width  == 240) && (screen_height == 320)) 
	   || ((screen_height == 240) && (screen_width  == 320)) )
	{
	    local_config.guimode = GUI_PDA;
	    // SMALLMENU = 1;
	    real_screen_x = screen_width - 8;
	    real_screen_y = screen_height - 70;
	}
    if (local_config.guimode == GUI_PDA)
	{
	    g_print ("\nPDA mode\n");
	}

    if (real_screen_x < real_screen_y)
	borderlimit = real_screen_x / 5;
    else
	borderlimit = real_screen_y / 5;

    if (borderlimit < 30)
	borderlimit = 30;

    SCREEN_X_2 = SCREEN_X / 2;
    SCREEN_Y_2 = SCREEN_Y / 2;
    PSIZE = 50;
    posx = SCREEN_X_2;
    posy = SCREEN_Y_2;

    if ( mydebug > 10 ) {
	g_print ("Set real Screen size to %d,%d\n", real_screen_x,real_screen_y);
    }

    if ( mydebug > 10 ) {
	g_print ("Screen size is %d,%d\n", screen_width,screen_height);
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
	gchar *question = "Are you sure?";

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
 *	Currently the Nightmode is represented by changing only the map
 *	colors, but the rest of the GUI stays the same.
 *	Maybe we should switch the gtk-theme instead, so that the complete
 *	GUI changes the colors. But this has time until after the 2007 release.
 */
gint switch_nightmode (gboolean value)
{
	if (value && !gui_status.nightmode)
	{
		gtk_widget_modify_bg (main_window, GTK_STATE_NORMAL,
			&colors.nightmode);
		gui_status.nightmode = TRUE;
		//if (mydebug > 4)
			fprintf (stderr, "setting to night view\n");
		return TRUE;
	}
	else if (!value && gui_status.nightmode)
	{
		gtk_widget_modify_bg (main_window, GTK_STATE_NORMAL,
				      &colors.defaultcolor);
		gui_status.nightmode = FALSE;
		//if (mydebug > 4)
			fprintf (stderr, "setting to daylight view\n");
		return TRUE;
	}
	
	return FALSE;
}


/* *****************************************************************************
 * Draw position marker, also showing direction
 * style sets the pinter style to be drawn:
 *  0 = full circle position pointer
 *  1 = only arrow pointer
 *  2 = only light crosshair pointer
 *  3 = plus sign
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
			0, 0, posx - 15, posy - 15,
			-1, -1, GDK_RGB_DITHER_NONE, 0, 0);
	}

	if (style == 0 && shadow == TRUE)
	{
		/* draw shadow of position icon */
		gdk_draw_arc (drawable, kontext_map, TRUE, posx-15, posy-15,
			30, 30, 0, 360 * 64);
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
		poly[2].x = posx;
		poly[2].y = posy;
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
			gdk_gc_set_foreground (kontext_map, &colors.lightorange);
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
	
	return TRUE;
}


/* *****************************************************************************
 * GUI Init Main
 * This will call all the necessary functions to init the graphical interface
 */
int gui_init (void)
{
	GdkRectangle rectangle = {0, 0, SCREEN_X, SCREEN_Y};

	/* init colors */
	colmap = gdk_colormap_get_system ();
	init_color (local_config.color_track, &colors.track);
	init_color (local_config.color_route, &colors.route);
	init_color (local_config.color_friends, &colors.friends);
	init_color (local_config.color_wplabel, &colors.wplabel);
	init_color (local_config.color_dashboard, &colors.dashboard);
	
	init_color ("#a0a0a0", &colors.shadow); 
	// TODO: see gui.h
	init_color ("#a00000", &colors.nightmode);
	init_color ("#000000", &colors.black);
	init_color ("#ff0000", &colors.red);
	init_color ("#ffffff", &colors.white);
	init_color ("#0000ff", &colors.blue);
	init_color ("#8b958b", &colors.lcd);
	init_color ("#737d6a", &colors.lcd2);
	init_color ("#ffff00", &colors.yellow);
	init_color ("#00b000", &colors.green);
	init_color ("#00ff00", &colors.green2);
	init_color ("#d5d5d5", &colors.mygray);
	init_color ("#a5a5a5", &colors.textback);
	init_color ("#4076cf", &colors.textbacknew);
	init_color ("#c0c0c0", &colors.grey);
	init_color ("#f06000", &colors.orange);
	init_color ("#f0995f", &colors.lightorange);
	init_color ("#ff8000", &colors.orange2);
	init_color ("#a0a0a0", &colors.darkgrey); 
	init_color ("#d0d0d0", &colors.lightgrey);

	gtk_window_set_auto_startup_notification (TRUE);
	
	/* init poi search dialog (cached) */
	create_window_poi_lookup();
	
	// TODO: use real values for geometry
	create_main_window(NULL, 0);

	// TODO: create_button_add_wp();

	posmarker_img = read_icon ("posmarker.png", 0);
	targetmarker_img = read_icon ("targetmarker.png", 0); 


// the following lines habe been moved from gpsdrive.c to here.
// maybe something has to be sorted out:
{	
	drawable =
	gdk_pixmap_new (map_drawingarea->window, SCREEN_X, SCREEN_Y, -1);
	kontext_map = gdk_gc_new (main_window->window);

	gdk_gc_set_clip_origin (kontext_map, 0, 0);
	rectangle.width = SCREEN_X;
	rectangle.height = SCREEN_Y;

	gdk_gc_set_clip_rectangle (kontext_map, &rectangle);

	/* fill window with color */
	gdk_gc_set_function (kontext_map, GDK_COPY);
	gdk_gc_set_foreground (kontext_map, &colors.lcd2);
	gdk_draw_rectangle (map_drawingarea->window, kontext_map, 1, 0, 0,
		SCREEN_X, SCREEN_Y);
    {
	GtkStyle *style;
	style = gtk_rc_get_style (main_window);
	colors.defaultcolor = style->bg[GTK_STATE_NORMAL];
    }
}

	/* set cross cursor for map posmode */
	cursor_cross = gdk_cursor_new(GDK_TCROSS);
	/* set watch cursor used e.g. when rendering a mapnik map*/
	cursor_watch = gdk_cursor_new(GDK_WATCH);
	return 0;
}

/*
 * function to set cursors styles
 */
gint
set_cursor_style(int cursor) {
	switch(cursor) {
		case CURSOR_DEFAULT:
			/* different cursors in posmode */
			if (gui_status.posmode == TRUE)
				gdk_window_set_cursor (map_drawingarea->window, cursor_cross);
			else
				gdk_window_set_cursor (map_drawingarea->window, NULL);
			break;
		case CURSOR_WATCH:
			gdk_window_set_cursor(map_drawingarea->window, cursor_watch);
	}
	/* update all events to fastly switch cursor */
	while (gtk_events_pending())
		gtk_main_iteration();
	return 0;
}
