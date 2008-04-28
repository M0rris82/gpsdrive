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


#ifndef GPSDRIVE_POI_H
#define GPSDRIVE_POI_H

/*
 * See poi.c for details.
 */

#include <gtk/gtk.h>

typedef struct
{
  gint    poi_id;
  gdouble lon;
  gdouble lat;
  gdouble alt;
  gchar   name[80];
  gchar   poi_type[160];
  gint    poi_type_id;
  gchar   comment[255]; 
  gchar   last_modified[11];
  gboolean private_flag;
  gint    source_id;
  gdouble    x;    // x position on screen
  gdouble    y;    // y position on screen
}
poi_struct;

#define POI_TYPE_LIST_STRING_LENGTH 80

/* poi related functions */
void draw_label (char *txt, gdouble posx, gdouble posy);
void draw_label_friend (char *txt, gdouble posx, gdouble posy);
void poi_init (void);
void poi_rebuild_list (void);
void poi_draw_list (gboolean draw_now);
gint poi_draw_cb (GtkWidget * widget, guint datum);
void poi_query_area ( gdouble lat1, gdouble lon1 ,gdouble lat2, gdouble lon2 );
GdkPixbuf * read_poi_icon (gchar * icon_name);
void get_poitype_tree (void);
guint poi_get_results (const gchar *text, const gchar *dist, const gint posflag, const gchar *type);
void update_poi_type_filter (void);
void init_poi_type_filter(void);

typedef struct
{
  guint id;
  gchar name[POI_TYPE_LIST_STRING_LENGTH];
  gchar icon_name[POI_TYPE_LIST_STRING_LENGTH];
  GdkPixbuf *icon;
  gint scale_min;
  gint scale_max;
  gint level;
  gchar description[POI_TYPE_LIST_STRING_LENGTH];
  gchar title[POI_TYPE_LIST_STRING_LENGTH];
} poi_type_struct;
#define poi_type_list_max 1000

enum {
	RESULT_ID,
	RESULT_NAME,
	RESULT_COMMENT,
	RESULT_TYPE_TITLE,
	RESULT_TYPE_NAME,
	RESULT_TYPE_ICON,
	RESULT_DISTANCE,
	RESULT_DIST_NUM,
	RESULT_LAT,
	RESULT_LON,
	RESULT_SOURCE,
	RES_COLUMS
};

enum {
	POITYPE_ID,
	POITYPE_NAME,
	POITYPE_ICON,
	POITYPE_SCALE_MIN,
	POITYPE_SCALE_MAX,
	POITYPE_DESCRIPTION,
	POITYPE_TITLE,
	POITYPE_SELECT,
	POITYPE_LABEL,
	POITYPE_COLUMS
};

#endif /* GPSDRIVE_POI_H */
