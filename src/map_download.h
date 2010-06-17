/***********************************************************************

Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>
Website: www.gpsdrive.de/

Disclaimer: Please do not use as a primary means of navigation.

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



#ifndef GPSDRIVE_MAP_DOWNLOAD_H
#define GPSDRIVE_MAP_DOWNLOAD_H

#include <gtk/gtk.h>

#define WMS_SERVER 1
#define TMS_SERVER 2

/*
 * See map_download.c for details.
 */
gint map_download_cb (GtkWidget *widget, gpointer data);
void mapdl_set_coords (gchar *lat, gchar *lon);
gint mapdl_init (void);
gint mapdl_cleanup (void);
double calc_webtile_scale (double, int);

#endif /* GPSDRIVE_MAP_DOWNLOAD_H */
