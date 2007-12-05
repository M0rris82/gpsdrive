/***********************************************************************

Copyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>

Website: www.gpsdrive.de

Copyright (c) 2007 Ross Scanlon <ross@theinternethost.com.au>
Website: www.4x4falcon.com

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

***********************************************************************/

#ifndef GPSDRIVE_GUI_H
#define GPSDRIVE_GUI_H
#define MAPWIDTH 1280
#define MAPHEIGHT 1024
#define MAP_X_2 (gui_status.mapview_x / 2)
#define MAP_Y_2 (gui_status.mapview_y / 2)

/*** Mod by Arms */
#define XMINUS 60
/*** Mod by Arms (move) */
#define YMINUS 67

gint popup_yes_no (GtkWindow *parent, gchar *message);
gint popup_warning (GtkWindow *parent, gchar *message);
gint popup_error (GtkWindow *parent, gchar *message);
gint popup_info (GtkWindow *parent, gchar *message);

int toggle_button_cb (GtkWidget *button, gboolean *value);

gint switch_nightmode (gboolean value);

gchar *get_colorstring (GdkColor *tcolor);

gboolean draw_posmarker
	(gdouble posx, gdouble posy, gdouble direction,
	 GdkColor *color, gint style, gboolean shadow, gboolean outline);


int gui_init (gchar *geometry, gint usegeometry);

gint set_cursor_style(int cursor);

typedef struct
{
	GdkColor track;
	GdkColor route;
	GdkColor friends;
	GdkColor wplabel;
	GdkColor dashboard;

	// TODO: Check, which of these colors are really needed.
	// The reason is that defined colors should only be used where it is
	// really important (e.g. inside the map window), the rest should be
	// left to the selected gtk theme.
	GdkColor nightmode;
	GdkColor defaultcolor;
	GdkColor red;
	GdkColor black;
	GdkColor white;
	GdkColor blue;
	GdkColor nightcolor;
	GdkColor lcd;
	GdkColor lcd2;
	GdkColor yellow;
	GdkColor green;
	GdkColor green2;
	GdkColor mygray;
	GdkColor textback;
	GdkColor textbacknew;
	GdkColor grey;
	GdkColor orange;
	GdkColor orange2;
	GdkColor lightorange;
	GdkColor darkgrey;
	GdkColor lightgrey;
	GdkColor shadow;
} color_struct;

typedef struct
{
	guint app_width;	/* width of application window in px */
	guint app_height;	/* height of application window in px */
	gint mapview_x;		/* width of shown map part in px */
	gint mapview_y;		/* height of shown map part in px */
	gboolean nightmode;
	gboolean posmode;
} guistatus_struct;

/* gpsdrive cursors */
enum {
	CURSOR_DEFAULT,
	CURSOR_WATCH
};

extern guistatus_struct gui_status;


#endif /* GPSDRIVE_GUI_H */
