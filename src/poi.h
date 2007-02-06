/***********************************************************************

Copyright (c) 2001,2002 Fritz Ganter <ganter@ganter.at>

Website: www.kraftvoll.at/software

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


#ifndef GPSDRIVE_POI_H
#define GPSDRIVE_POI_H

/*
 * See poi.c for details.
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
  gint    scale_min;  
  gint    scale_max;  
  //date  last_modified  
  gchar   url[160]; 
  gint    address_id;
  gint    source_id; 
  gdouble    x;    // x position on screen
  gdouble    y;    // y position on screen
}
poi_struct;

#define POI_TYPE_LIST_STRING_LENGTH 80

void draw_text (char *txt, gdouble posx, gdouble posy);
void poi_init (void);
void poi_rebuild_list (void);
gint poi_type_id_from_name (gchar name[POI_TYPE_LIST_STRING_LENGTH]);
void poi_draw_list (void);
gint poi_draw_cb (GtkWidget * widget, guint datum);
void poi_query_area ( gdouble lat1, gdouble lon1 ,gdouble lat2, gdouble lon2 );
GdkPixbuf * read_poi_icon (gchar * icon_name);
void
get_poi_type_list (void);

typedef struct
{
  gint poi_type_id;
  gchar name[POI_TYPE_LIST_STRING_LENGTH];
  gchar icon_name[POI_TYPE_LIST_STRING_LENGTH];
  GdkPixbuf *icon;
  gint scale_min;
  gint scale_max;
  gint parent;
  gchar description[POI_TYPE_LIST_STRING_LENGTH];
  gchar title[POI_TYPE_LIST_STRING_LENGTH];
} poi_type_struct;
#define poi_type_list_max 1000


#endif /* GPSDRIVE_POI_H */
