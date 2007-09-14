/***********************************************************************

Copyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>

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

    *********************************************************************

***********************************************************************/

#ifndef GPSDRIVE_POI_GUI_H
#define GPSDRIVE_POI_GUI_H

void create_window_poi_lookup (void);
void poi_info_cb (void);
void route_window_cb (GtkWidget *calling_button);
gint show_poi_lookup_cb (GtkWidget *button, gpointer data);
GtkWidget * create_poi_types_window (void);
gint toggle_window_poitypes_cb (GtkWidget *trigger, gboolean multi);

#endif /* GPSDRIVE_POI_GUI_H */
