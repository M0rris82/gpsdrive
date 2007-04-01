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

gint popup_yes_no (GtkWindow *parent, gchar *message);

int get_window_sizing (gchar *geom, gint usegeom, gint screen_height, gint screen_width);


/* this will be changed to following once -s command line option is dropped */
//int create_main_window (gchar *geom, gint *usegeom);

int resize_all (void);

int gui_init (void);

#endif /* GPSDRIVE_GUI_H */