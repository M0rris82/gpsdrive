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

void poi_lookup_cb (GtkWidget *calling_button);
void poi_info_cb (void);
void route_window_cb (GtkWidget *calling_button);

gint popup_yes_no (GtkWindow *parent, gchar *message);
gint popup_warning (GtkWindow *parent, gchar *message);
gint popup_error (GtkWindow *parent, gchar *message);

gint switch_nightmode (gboolean value);

gchar *get_colorstring (GdkColor *tcolor);

int get_window_sizing (gchar *geom, gint usegeom, gint screen_height, gint screen_width);


/* this will be changed to following once -s command line option is dropped */
//int create_main_window (gchar *geom, gint *usegeom);

int resize_all (void);

int gui_init (void);


typedef struct
{
	GdkColor track;
	GdkColor route;
	GdkColor friends;
	GdkColor wplabel;
	GdkColor bigdisplay;

	// TODO: Check, which of these colors are really needed.
	// The reason is that defined colors should only be used where it is
	// really important (e.g. inside the map window), the rest should be
	// left to the selected gtk theme.
	GdkColor nightmode;
	GdkColor defaultcolor;
	GdkColor trackcolorv;
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
	GdkColor darkgrey;
	GdkColor shadow;
} color_struct;

typedef struct
{
	guint width;
	guint height;
	gboolean nightmode;
	gboolean mapnik;
} guistatus_struct;

extern guistatus_struct gui_status;

#endif /* GPSDRIVE_GUI_H */
