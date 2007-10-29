/***********************************************************************

Copyright (c) 2001,2002 Fritz Ganter <ganter@ganter.at>

Website: www.gpsdrive.de/

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


#ifndef GPSDRIVE_WLAN_H
#define GPSDRIVE_WLAN_H

/*
 * See wlan.c for details.
 */

#include <gtk/gtk.h>

typedef struct
{
  gint    wp_id;
  gdouble lon;
  gdouble lat;
  gdouble alt;
  gchar   name[80];
  gint    poi_type_id; 
  gdouble proximity;
  gchar   comment[255]; 
  gchar   url[160]; 
  gint    source_id; 
  gint    nettype;
  gdouble    x;    // x position on screen
  gdouble    y;    // y position on screen
}
wlan_struct;

void wlan_init (void);
void wlan_rebuild_list (void);
void wlan_draw_list (void);
void wlan_query_area ( gdouble lat1, gdouble lon1 ,gdouble lat2, gdouble lon2 );
void wlan_rebuild_list (void);
int wlan_check_if_moved (void);
GdkPixbuf * read_wlan_icon (gchar * icon_name);
void get_poi_type_id_for_wlan();

#define wlan_type_list_string_length 80
typedef struct
{
  gint poi_type_id;
  gchar name[wlan_type_list_string_length];
  gchar icon_name[wlan_type_list_string_length];
  GdkPixbuf *icon;
  gint scale_min;
  gint scale_max;
} wlan_type_struct;
#define wlan_type_list_max 4000


#endif /* GPSDRIVE_WLAN_H */
