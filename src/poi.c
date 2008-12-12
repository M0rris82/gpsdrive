/***********************************************************************

Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>

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

*********************************************************************/


/*
 * poi_ support module: display
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <gmodule.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"

#include "gpsdrive.h"
#include "poi.h"
#include "gettext.h"
#include "icons.h"
#include <gpsdrive_config.h>
#include "gui.h"
#include "database.h"

#include "gettext.h"
#include <libxml/xmlreader.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gchar language[];
extern gint do_unit_test;
extern gint maploaded;
extern color_struct colors;
extern gdouble wp_saved_target_lat, wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat, wp_saved_posmode_lon;
extern gint debug, mydebug;
extern GtkWidget *map_drawingarea;
extern glong mapscale;
extern gdouble dbdistance;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern GdkGC *kontext_map;

extern GdkPixbuf *posmarker_img;
extern GdkGC *kontext;

PangoLayout *poi_label_layout;

// keep actual visible POIs in Memory
poi_struct *poi_list;

// keep POI info from last search result in Memory
poi_struct *poi_result;
GtkListStore *poi_result_tree;

// keep data for poi_info window
poi_info_struct poi_info;

// buffer for storing temporary POI data
poi_struct *poi_buf;

glong poi_nr;			// current number of poi to count
glong poi_list_count;		// max index of POIs actually in memory
glong poi_limit = -1;		// max allowed index (if you need more you have to alloc memory)

gchar poi_label_font[100];
GdkColor poi_colorv;
PangoFontDescription *pfd;
PangoLayout *poi_label_layout;

poi_type_struct poi_type_list[poi_type_list_max];
int poi_type_list_count = 0;
GtkTreeStore *poi_types_tree;
GtkTreeModel *poi_types_tree_filtered;
GHashTable *poi_types_hash;

gdouble poi_lat_lr = 0, poi_lon_lr = 0;
gdouble poi_lat_ul = 0, poi_lon_ul = 0;


/* ******************************************************************   */

void poi_rebuild_list (void);


/* ******************************************************************
 * get data to display inj poi info window for selected poi
 */
get_poi_info (GtkTreeModel *model, GtkTreeIter *iter)
{
	gboolean t_priv;
	gdouble t_lat, t_lon, t_alt;
	gchar *t_name, *t_cmt, *t_type, *t_path;
	gchar t_coord[20];
	GtkTreeIter t_iter, t_fiter;

	gtk_tree_model_get (model, iter,
		RESULT_ID, &poi_info.id,
		RESULT_TYPE_NAME, &t_type,
		RESULT_NAME, &t_name,
		RESULT_COMMENT, &t_cmt,
		RESULT_LAT, &t_lat,
		RESULT_LON, &t_lon,
		-1);

	gtk_entry_set_text (GTK_ENTRY (poi_info.entry_name), t_name);
	gtk_entry_set_text (GTK_ENTRY (poi_info.entry_comment), t_cmt);
	coordinate2gchar(t_coord, sizeof(t_coord), t_lat, TRUE, local_config.coordmode);
	gtk_entry_set_text (GTK_ENTRY (poi_info.entry_lat), t_coord);
	coordinate2gchar(t_coord, sizeof(t_coord), t_lon, FALSE, local_config.coordmode);
	gtk_entry_set_text (GTK_ENTRY (poi_info.entry_lon), t_coord);

	t_path = g_hash_table_lookup (poi_types_hash, t_type);
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (poi_types_tree),
		&t_iter, t_path);
	gtk_tree_model_filter_convert_child_iter_to_iter (GTK_TREE_MODEL_FILTER (
		poi_types_tree_filtered), &t_fiter, &t_iter);
	gtk_combo_box_set_active_iter (GTK_COMBO_BOX (poi_info.combobox_type), &t_fiter);

	gtk_list_store_clear (poi_info.extra_list);
	db_poi_extra_get_all (&poi_info.id, poi_info.extra_list);

	g_free (t_name);
	g_free (t_cmt);
	g_free (t_type);
/*
	gint id;
	GtkListStore *extra_list;
	GtkWidget *entry_name;
	GtkWidget *entry_comment;
	GtkWidget *entry_lat;
	GtkWidget *entry_lon;
	GtkWidget *combobox_type;
	GtkWidget *statusbar_poiinfo;
*/

}


/* ******************************************************************
 * filter function for filtering poi_type views
 */
static gboolean
treefilterfunc_cb  (GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
	gint t_id;
	gtk_tree_model_get (model, iter, POITYPE_ID, &t_id, -1);
	if (t_id == -1)
		return FALSE;
	return TRUE;
}


/* ******************************************************************
 * initialize POI filter from config settings
 */
gboolean
init_poi_filter (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	gchar *t_name, *t_delimit;
	gboolean t_sl = TRUE;
	gboolean t_lb = FALSE;

	gtk_tree_model_get (model, iter, POITYPE_NAME, &t_name, -1);

	t_delimit = g_strstr_len (t_name, 30, ".");
	if (t_delimit)
		*t_delimit = '\0';

	/* set poi display filter */
	if (g_strstr_len (local_config.poi_filter, sizeof (local_config.poi_filter), t_name))
		t_sl = FALSE;

	/* set poi label filter */
	if (g_strstr_len (local_config.poi_label, sizeof (local_config.poi_label), t_name))
		t_lb = TRUE;

	gtk_tree_store_set (GTK_TREE_STORE (model), iter,
		POITYPE_SELECT, t_sl,
		POITYPE_LABEL, t_lb, -1);

	g_free (t_name);
	return FALSE;
}


/* ******************************************************************
 * db callback for reading searched points of interest in poi lookup window
 */
gint
handle_poi_search_cb (gchar *result, gint columns, gchar **values, gchar **names)
{
	gchar t_buf[100];
	gint poi_posx, poi_posy;
	gdouble t_lat, t_lon;
	gdouble t_dist_num;
	gchar t_dist[15];
	gchar *t_path, *t_title;
	GdkPixbuf *t_icon;
	GtkTreeIter iter, type_iter;

	if (columns == 0)
		return 0;

	if (mydebug > 20)
	{
		g_print ("Query Result: %s\t%s\t%s\t%s\t%s\t%s\t%s\n",
			values[0], values[1], values[2], values[3], values[4], values[5], values[6]);
	}

	// get next free mem for point
	poi_nr++;
	if (poi_nr > poi_limit)
	{
		poi_limit = poi_nr + 10000;
		if (mydebug > 20)
			g_print ("Try to allocate Memory for %ld poi\n", poi_limit);
		poi_result = g_renew (poi_struct, poi_result, poi_limit);
		if (NULL == poi_result)
		{
			g_print ("Error: Cannot allocate Memory for %ld poi\n", poi_limit);
			poi_limit = -1;
			return -1;
		}
	}

	// Save retrieved poi information into structure
	(poi_result + poi_nr)->poi_id = (gint) g_strtod (values[0], NULL);

	g_strlcpy ((poi_result + poi_nr)->name, values[1],
		sizeof ((poi_result + poi_nr)->name));

	if (values[2] == NULL)
		g_strlcpy ((poi_result + poi_nr)->comment, "n/a",
			sizeof ((poi_result + poi_nr)->comment));		
	else
		g_strlcpy ((poi_result + poi_nr)->comment, values[2],
			sizeof ((poi_result + poi_nr)->comment));

	g_strlcpy ((poi_result + poi_nr)->poi_type, values[3],
			sizeof ((poi_result + poi_nr)->poi_type));

	t_path = g_hash_table_lookup (poi_types_hash, (poi_result + poi_nr)->poi_type);
	if (t_path == NULL)
		t_path = g_hash_table_lookup (poi_types_hash, "unknown");

	(poi_result + poi_nr)->lon = g_strtod (values[4], NULL);
	t_lon = g_strtod (values[4], NULL);

	(poi_result + poi_nr)->lat = g_strtod (values[5], NULL);
	t_lat = g_strtod (values[5], NULL);

	(poi_result + poi_nr)->source_id = g_strtod (values[6], NULL);

	t_dist_num = calcdist (t_lon, t_lat);
	g_snprintf (t_dist, sizeof (t_dist), "%9.3f", t_dist_num);

	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (poi_types_tree), &type_iter, t_path);
	gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &type_iter,
		POITYPE_TITLE, &t_title,
		POITYPE_ICON, &t_icon,
		-1);

	gtk_list_store_append (poi_result_tree, &iter);
	gtk_list_store_set (poi_result_tree, &iter,
		RESULT_ID, (poi_result + poi_nr)->poi_id,
		RESULT_NAME, (poi_result + poi_nr)->name,
		RESULT_COMMENT,  (poi_result + poi_nr)->comment,
		RESULT_TYPE_TITLE, t_title,
		RESULT_TYPE_NAME, (poi_result + poi_nr)->poi_type,
		RESULT_TYPE_ICON, t_icon,
		RESULT_DISTANCE, t_dist,
		RESULT_DIST_NUM, t_dist_num,
		RESULT_LAT, t_lat,
		RESULT_LON, t_lon,
		RESULT_SOURCE, (poi_result + poi_nr)->source_id,
		-1);

	return 0;
}


/* ******************************************************************
 * db callback for reading searched points of interest in
 * poi lookup window from the mapnik/postgis database
 */
gint
handle_osm_poi_search_cb (const gchar *name, const gchar *type, const gchar *geometry, const gchar *id)
{
	gchar **t_coords;
	gdouble x, y, t_lat, t_lon;
	glong t_osmid = 0;
	gdouble t_dist_num;
	gchar t_dist[15];
	gchar *t_path, *t_title;
	GtkTreeIter iter, type_iter;
	GdkPixbuf *t_icon;

	if (g_str_has_prefix (geometry, "POINT(") == FALSE)
		return -1;

	t_coords = g_strsplit ((geometry + 6), " ", 2);
	x = g_strtod (t_coords[0], NULL);
	y = g_strtod (t_coords[1], NULL);
#ifdef MAPNIK
	convert_mapnik_coords(&t_lon, &t_lat, x, y, 1);
#endif
	if (id)
		t_osmid = atol (id);

	if (mydebug > 40)
	{
		g_printf ("handle_osm_poi_search_cb: %d - %s [%s] - %.6f / %.6f\n",
			t_osmid, name, type, t_lat, t_lon);
	}

	// get next free mem for point
	poi_nr++;
	if (poi_nr > poi_limit)
	{
		poi_limit = poi_nr + 10000;
		if (mydebug > 20)
			g_print ("Try to allocate Memory for %ld poi\n", poi_limit);
		poi_result = g_renew (poi_struct, poi_result, poi_limit);
		if (NULL == poi_result)
		{
			g_print ("Error: Cannot allocate Memory for %ld poi\n", poi_limit);
			poi_limit = -1;
			return -1;
		}
	}

	// Save retrieved poi information into structure
	(poi_result + poi_nr)->poi_id = t_osmid;

	g_strlcpy ((poi_result + poi_nr)->name, name,
		sizeof ((poi_result + poi_nr)->name));

	g_strlcpy ((poi_result + poi_nr)->comment, "",
		sizeof ((poi_result + poi_nr)->comment));		

	g_strlcpy ((poi_result + poi_nr)->poi_type, type,
			sizeof ((poi_result + poi_nr)->poi_type));

	t_path = g_hash_table_lookup (poi_types_hash, (poi_result + poi_nr)->poi_type);
	if (t_path == NULL)
		t_path = g_hash_table_lookup (poi_types_hash, "unknown");

	(poi_result + poi_nr)->lon = t_lon;

	(poi_result + poi_nr)->lat = t_lat;

	(poi_result + poi_nr)->source_id = 10;

	t_dist_num = calcdist (t_lon, t_lat);
	g_snprintf (t_dist, sizeof (t_dist), "%9.3f", t_dist_num);

	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (poi_types_tree), &type_iter, t_path);
	gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &type_iter,
		POITYPE_TITLE, &t_title,
		POITYPE_ICON, &t_icon,
		-1);

	gtk_list_store_append (poi_result_tree, &iter);
	gtk_list_store_set (poi_result_tree, &iter,
		RESULT_ID, (poi_result + poi_nr)->poi_id,
		RESULT_NAME, (poi_result + poi_nr)->name,
		RESULT_COMMENT,  (poi_result + poi_nr)->comment,
		RESULT_TYPE_TITLE, t_title,
		RESULT_TYPE_NAME, (poi_result + poi_nr)->poi_type,
		RESULT_TYPE_ICON, t_icon,
		RESULT_DISTANCE, t_dist,
		RESULT_DIST_NUM, t_dist_num,
		RESULT_LAT, t_lat,
		RESULT_LON, t_lon,
		RESULT_SOURCE, (poi_result + poi_nr)->source_id,
		-1);

	return 0;
}


/* *******************************************************
 * check, which poi_types should be shown in the map.
 * filtering is only done on the base category level
 */
void
update_poi_type_filter ()
{
	GtkTreeIter t_iter;
	gboolean t_selected;
	gchar *t_name;
	gchar t_string[200];
	gchar t_config[2000];
	gchar t_filter[20000];

	if (mydebug > 21)
		fprintf (stderr, "update_poi_type_filter:\n");

	g_strlcpy (t_filter, "AND (", sizeof (t_filter));
	g_strlcpy (local_config.poi_filter, "", sizeof (local_config.poi_filter));

	gtk_tree_model_get_iter_first
		(GTK_TREE_MODEL (poi_types_tree), &t_iter);

	do
	{
		gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &t_iter,
			POITYPE_NAME, &t_name,
			POITYPE_SELECT, &t_selected, -1);
		if (!t_selected)
		{
			/* build SQL string for filter */
			g_snprintf (t_string, sizeof (t_string),
				"poi_type NOT LIKE \"%s%%\" AND ",
				t_name);
			g_strlcat (t_filter, t_string, sizeof (t_filter));
				
			/* build settings string for config file */
			g_snprintf (t_config, sizeof (t_config),
				"%s|", t_name);
			g_strlcat (local_config.poi_filter,
				t_config, sizeof (local_config.poi_filter));
		}
		g_free (t_name);
	}
	while (gtk_tree_model_iter_next
		(GTK_TREE_MODEL (poi_types_tree), &t_iter));

	g_strlcat (t_filter, "1)", sizeof (t_filter));

	current.needtosave = TRUE;

	db_get_visible_poi_types (t_filter);
}


/* *******************************************************
 * search database for POIs filtered by data entered
 * into the POI-Lookup window
 */
guint
poi_get_results (const gchar *text, const gchar *pdist, const gint posflag, const gchar *type)
{
	gdouble lat, lon, dist;
	gdouble lat_min, lon_min;
	gdouble lat_max, lon_max;
	gdouble dist_lat, dist_lon;
	guint poi_result_count;

	char sql_query[20000];
	char type_filter[3000];
	gchar *temp_text;

	// clear results from last search	
	gtk_list_store_clear (poi_result_tree);
	
	dist = g_strtod (pdist, NULL);
	if (dist <= 0)
		dist = dbdistance;
	
	if (posflag)
	{
		lat = wp_saved_target_lat;
		lon = wp_saved_target_lon;
	}
	else
	{
		if (gui_status.posmode)
		{
			lat = wp_saved_posmode_lat;
			lon = wp_saved_posmode_lon;
		}
		else
		{
			lat = coords.current_lat;
			lon = coords.current_lon;
		}
	}
	
 	// calculate bbox around starting point derived from specified distance
	//   latitude: 1 degree = 111,13 km
	//   longitude: 1 degree = 111,13 km * cos(latitude)
	// (this is not very accurate, but should be suitable for filtering pois)
	dist_lat = fabs (dist/111.13);
	if (lat==90.0 || lat ==-90.0)
		dist_lon = 180;
	else
		dist_lon = fabs (dist/(111.13*cos(lat)));
	
	lat_min = lat-dist_lat;
	lon_min = lon-dist_lon;
	lat_max = lat+dist_lat;
	lon_max = lon+dist_lon;
	
	if (mydebug > 25)
	{
		fprintf (stderr, "  --- lat: %f  lon: %f  dist: %f\n", lat, lon, dist);
		fprintf (stderr, "  --- lat_min: %f  lat_max: %f  dist_lat: %f\n", lat_min, lat_max, dist_lat);
		fprintf (stderr, "  --- lon_min: %f  lon_max: %f  dist_lon: %f\n", lon_min, lon_max, dist_lon);
	}
	
	if (lon_min < -180.0) lon_min = -180.0;
	if (lon_max > 180.0) lon_max = 180.0;
	if (lat_min < -90.0) lat_min = -90.0;
	if (lat_max > 90.0) lat_max = 90.0;
	
	/* choose poi_types to search */
	if (g_ascii_strncasecmp (type, "__NO_FILTER__", 13))
	{
		g_snprintf (type_filter, sizeof (type_filter),
			" LIKE \'%s%%\'",type);
	}
	else
	{
		g_snprintf (type_filter, sizeof (type_filter),
			"!=''");
	}

	/* prepare search text for database query */
	temp_text = escape_sql_string (text);
	g_strdelimit (temp_text, "*", '%');

	poi_nr = 0;

	/* query sqlite db */
	if (current.poi_osm)
	{
		if (mydebug > 30)
			g_print ("poi_get_results (): Searching in OSM and user database\n");
		g_snprintf (sql_query, sizeof (sql_query),
			"SELECT poi_id,name,comment,poi_type,lon,lat,source_id FROM"
			" (SELECT * FROM main.poi UNION SELECT * FROM osm.poi)"
			" WHERE ( lat BETWEEN %.8f AND %.8f ) AND ( lon BETWEEN %.8f"
			" AND %.8f ) AND (name LIKE '%%%s%%' OR comment LIKE"
			" '%%%s%%') AND (poi_type%s) ORDER BY"
			" (lat-(%.8f))*(lat-(%.8f))+(lon-(%.8f))*(lon-(%.8f)) LIMIT %d;",
			lat_min, lat_max, lon_min, lon_max, temp_text, temp_text,
			type_filter, lat, lat, lon, lon, local_config.poi_results_max);
	}
	else
	{
		if (mydebug > 30)
			g_print ("poi_get_results (): Searching in user database\n");
		g_snprintf (sql_query, sizeof (sql_query),
			"SELECT poi_id,name,comment,poi_type,lon,lat,source_id FROM poi"
			" WHERE ( lat BETWEEN %.8f AND %.8f ) AND ( lon BETWEEN %.8f"
			" AND %.8f ) AND (name LIKE '%%%s%%' OR comment LIKE"
			" '%%%s%%') AND (poi_type%s) ORDER BY"
			" (poi.lat-(%.8f))*(poi.lat-(%.8f))+(poi.lon-(%.8f))*(poi.lon-(%.8f)) LIMIT %d;",
			lat_min, lat_max, lon_min, lon_max, temp_text, temp_text,
			type_filter, lat, lat, lon, lon, local_config.poi_results_max);
	}

	db_poi_get (sql_query, handle_poi_search_cb, DB_WP_USER);

	/* query postgis db */
#ifdef MAPNIK
//	gdouble x, y;
//	convert_mapnik_coords(&x, &y, lon, lat, 0);
//	g_snprintf (sql_query, sizeof (sql_query),
//		"SELECT name,poi,ASTEXT(way),osm_id FROM planet_osm_point"
//		" WHERE ST_DWithin(SetSRID(way,-1), 'POINT(%.8f %.8f)', %g)"
//		" AND name ILIKE '%%%s%%' AND poi%s LIMIT %d;",
//		x, y, dist*1000, temp_text,
//		type_filter, local_config.poi_results_max);
//	db_poi_get (sql_query, handle_osm_poi_search_cb, DB_WP_OSM);
#endif

	poi_result_count = poi_nr;

	if (mydebug > 20)
		printf (_("%ldrows read\n"), poi_list_count);

	g_free (temp_text);
	return poi_result_count;
}


/* *******************************************************
 * draw standard poi label
 */
void
draw_label (char *txt, gdouble posx, gdouble posy)
{
  gint width, height;
  gint k, k2;

  if (strlen (txt) == 0)
      return;

  if (mydebug > 30)
    fprintf (stderr, "draw_label(%s,%g,%g)\n", txt, posx, posy);

  gdk_gc_set_foreground (kontext_map, &colors.textback);

  poi_label_layout = gtk_widget_create_pango_layout (map_drawingarea, txt);

  if (poi_list_count > 200)
    pfd = pango_font_description_from_string ("Sans 6");
  else
    pfd = pango_font_description_from_string ("Sans 8");

  pango_layout_set_font_description (poi_label_layout, pfd);
  pango_layout_get_pixel_size (poi_label_layout, &width, &height);
  k = width + 4;
  k2 = height;

  gdk_gc_set_function (kontext_map, GDK_COPY);

  gdk_gc_set_function (kontext_map, GDK_AND);

  {                             // Draw rectangle arround Text
    // gdk_gc_set_foreground (kontext, &textbacknew);
    gdk_gc_set_foreground (kontext_map, &colors.darkgrey);
    gdk_draw_rectangle (drawable, kontext_map, 1,
                        posx + 13, posy - k2 / 2, k + 1, k2);

  }

  poi_label_layout = gtk_widget_create_pango_layout (map_drawingarea, txt);
  pango_layout_set_font_description (poi_label_layout, pfd);

  gdk_draw_layout_with_colors (drawable, kontext_map,
                               posx + 15, posy - k2 / 2,
                               poi_label_layout, &colors.lightgrey, NULL);
  if (poi_label_layout != NULL)
    g_object_unref (G_OBJECT (poi_label_layout));
  /* freeing PangoFontDescription, cause it has been copied by prev. call */
  pango_font_description_free (pfd);
}


/* *******************************************************
 * draw friends label
 */
void
draw_label_friend (char *txt, gdouble posx, gdouble posy)
{
	gint width, height;
	gint k, k2;

	if (mydebug > 30)
		fprintf (stderr, "draw_label(%s,%g,%g)\n", txt, posx, posy);

	poi_label_layout = gtk_widget_create_pango_layout (map_drawingarea, txt);

	pfd = pango_font_description_from_string (local_config.font_friends);
	gdk_gc_set_foreground (kontext_map, &colors.textbacknew);

	pango_layout_set_font_description (poi_label_layout, pfd);
	pango_layout_get_pixel_size (poi_label_layout, &width, &height);
	k = width + 4;
	k2 = height;

	gdk_gc_set_function (kontext_map, GDK_COPY);
	gdk_gc_set_function (kontext_map, GDK_AND);

	gdk_draw_layout_with_colors (drawable, kontext_map,
		posx + 16, posy - k2 / 2 + 1,
		poi_label_layout, &colors.black, NULL);

	gdk_draw_layout_with_colors (drawable, kontext_map,
		posx + 15, posy - k2 / 2,
		poi_label_layout, &colors.friends, NULL);
	if (poi_label_layout != NULL)
		g_object_unref (G_OBJECT (poi_label_layout));
	/* freeing PangoFontDescription, cause it
	 * has been copied by prev. call */
	pango_font_description_free (pfd);
}


int
poi_check_if_moved (void)
{
  gdouble lat_lr, lon_lr;
  gdouble lat_ul, lon_ul;

  if (poi_lat_lr == 0 && poi_lon_lr == 0 &&
      poi_lat_ul == 0 && poi_lon_ul == 0)
    return 1;

  calcxytopos (gui_status.mapview_x, gui_status.mapview_y, &lat_lr, &lon_lr, current.zoom);
  calcxytopos (0, 0, &lat_ul, &lon_ul, current.zoom);

  if (poi_lat_lr == lat_lr && poi_lon_lr == lon_lr &&
      poi_lat_ul == lat_ul && poi_lon_ul == lon_ul)
    return 0;
  return 1;
}


/* ******************************************************************
 * add new row to poitype tree
*/
static gboolean 
poitypetree_addrow (guint i, GtkTreeIter *parent)
{
	GtkTreeIter iter;
	gchar *t_buf;

	poi_type_list[i].id = i;

	gtk_tree_store_append (poi_types_tree, &iter, parent);
 
	gtk_tree_store_set (poi_types_tree, &iter,
		POITYPE_ID, poi_type_list[i].id,
		POITYPE_NAME, poi_type_list[i].name,
		POITYPE_ICON, poi_type_list[i].icon,
		POITYPE_SCALE_MIN, poi_type_list[i].scale_min,
		POITYPE_SCALE_MAX, poi_type_list[i].scale_max,
		POITYPE_DESCRIPTION, poi_type_list[i].description,
		POITYPE_TITLE, poi_type_list[i].title,
		POITYPE_SELECT, FALSE,
		POITYPE_LABEL, FALSE,
		-1);

	t_buf = gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (poi_types_tree), &iter);
	g_strlcpy (poi_type_list[i].path, t_buf, sizeof (poi_type_list[i].path));

	g_hash_table_insert (poi_types_hash, poi_type_list[i].name, poi_type_list[i].path);

	if (mydebug > 30)
	{
		fprintf (stderr,
			"poitypetree_addrow: added %s - %s\n",
			t_buf, poi_type_list[i].name);
	}

	g_free (t_buf);

	return TRUE;
}


/* ******************************************************************
 * find parent for given poitype and add new entry if found
 */
static gboolean poitypetree_find_parent (
	GtkTreeModel *model, GtkTreePath *path,
	GtkTreeIter *iter, guint i)
{
	gchar *value;
	
	gchar *parent_name;
	gint k;
	guint l = 0;

	for (k = poi_type_list[i].level; k>0; k--)
	{
		l += strcspn (
			(poi_type_list[i].name + poi_type_list[i].level
			- k + l), ".");
	}
	
	parent_name = g_strndup (poi_type_list[i].name,
		poi_type_list[i].level + l -1);

	gtk_tree_model_get (model, iter, POITYPE_NAME, &value, -1);

	if (g_ascii_strcasecmp (parent_name, (gchar *) value) == 0)
	{	
		
		if (mydebug > 40)
		{
			fprintf (stderr,
				"poitypetree_find_parent: %d+%d : %s => %s\n",
				l, poi_type_list[i].level - 1,
				poi_type_list[i].name, parent_name);
		}
		poitypetree_addrow (i, iter);
		g_free (value);
		g_free (parent_name);
		return TRUE;
	}
	else
	{
		g_free (value);
		g_free (parent_name);
		return FALSE;
	}
}


/* ******************************************************************
 * sort poitype data from flat struct into a gtk-tree
 */
gboolean
create_poitype_tree (guint max_level)
{
	guint i = 0;
	guint j = 0;
	GtkTreeIter iter;

	if (mydebug > 20)
		fprintf (stderr, "create_poitype_tree:\n");

	/* create hash table, to quickly find poi types by name */
	if (poi_types_hash)
		g_hash_table_destroy (poi_types_hash);
	poi_types_hash = g_hash_table_new (g_str_hash, g_str_equal);

	/* insert "no filter" entry into tree (used for poi lookup) */
	gtk_tree_store_append (poi_types_tree, &iter, NULL);
	gtk_tree_store_set (poi_types_tree, &iter,
		POITYPE_ID, -1,
		POITYPE_NAME, "__NO_FILTER__",
		POITYPE_ICON, NULL,
		POITYPE_SCALE_MIN, -1,
		POITYPE_SCALE_MAX, -1,
		POITYPE_DESCRIPTION, NULL,
		POITYPE_TITLE, _("all types"),
		POITYPE_SELECT, FALSE,
		POITYPE_LABEL, FALSE,
		-1);

	/* insert base categories into tree */
	for (i = 0; i < poi_type_list_max; i++)
	{
		if (strlen (poi_type_list[i].name) > 0)
		{
			if (poi_type_list[i].level == 0)
			{
				poitypetree_addrow (i, (GtkTreeIter *) NULL);
				if (mydebug > 30)
				{
					fprintf (stderr,
						"create_poitype_tree:"
						" adding base type %s\n",
						poi_type_list[i].name);
				}
			}
		}
		else
		{
			poi_type_list[i].level = 99;
		}
	}

	/* insert subcategories into tree */
	for (j = 1; j <= max_level; j++)
	{
		for (i = 0; i < poi_type_list_max; i++)
		{
			if (poi_type_list[i].level == j)
			{
				gtk_tree_model_foreach
					(GTK_TREE_MODEL (poi_types_tree),
					*(GtkTreeModelForeachFunc)
					poitypetree_find_parent, (gpointer) i);
			}
		}
	}

	return TRUE;
}


/* ******************************************************************
 * db callback for reading poitype data from geoinfo.db
 */
gint
handle_poitype_tree_cb (guint count[], gint columns, gchar **values, gchar **names)
{
	guint i,j;

	if (mydebug > 30)
	{
		g_print ("Query Result: %s\t%s\t%s\t%s\t%s\n",
			values[0], values[1], values[2], values[3], values[4]);
			/* poi_type,scale_min,scale_max,title,description */
	}

	poi_type_list[count[0]].scale_min = (gint) g_strtod (values[1], NULL);
	poi_type_list[count[0]].scale_max = (gint) g_strtod (values[2], NULL);
	g_strlcpy (poi_type_list[count[0]].title, values[3],
		sizeof (poi_type_list[count[0]].title));
	g_strlcpy (poi_type_list[count[0]].description, values[4],
		sizeof (poi_type_list[count[0]].description));
	g_strlcpy (poi_type_list[count[0]].name, values[0],
		sizeof(poi_type_list[count[0]].name));
	g_strlcpy (poi_type_list[count[0]].icon_name, values[0],
		sizeof(poi_type_list[count[0]].icon_name));
	poi_type_list[count[0]].icon = read_themed_icon (poi_type_list[count[0]].icon_name);


	j = 0;			
	for (i=0; i < strlen (poi_type_list[count[0]].name); i++)
	{
		if (g_str_has_prefix ((poi_type_list[count[0]].name + i) , "."))
		{
			j++;
		}
	}
	poi_type_list[count[0]].level = j;
	if (count[1] < j)
	{
		count[1] = j;
	}

	if (poi_type_list[count[0]].icon == NULL)
	{
		if (mydebug > 40)
			g_printf ("get_poitype_tree: %3d:Icon '%s' for '%s'\tnot found\n",
				index, poi_type_list[count[0]].icon_name, poi_type_list[count[0]].name);
		if (do_unit_test)
			exit (-1);
	}

	count[0]++;

	return 0;
};


/* ******************************************************************
 * get a list of all possible poi_types and load their icons
*/
void
get_poitype_tree (void)
{
	/* count[0] = number of poi-types read
	 * count[1] = maximum depth of tree */
	guint count[] = {0, 0};

	if (mydebug > 10)
		printf ("get_poitype_tree ()\n");

	// Clear poi_type_tree
	gtk_tree_store_clear (poi_types_tree);

	db_get_all_poi_types (handle_poitype_tree_cb, count);

	g_printf ("Read %d POI-Types from geoinfo.db\n", count[0]);

	create_poitype_tree (count[1]);

	/* set poi filter according to config file */
	gtk_tree_model_foreach (GTK_TREE_MODEL (poi_types_tree),
		*(GtkTreeModelForeachFunc) init_poi_filter, NULL);
}


/* ******************************************************************
 * db callback for reading currently visible points of interest
 */
gint
handle_poi_view_cb (gchar *result, gint columns, gchar **values, gchar **names)
{
	gchar t_buf[100];
	gint poi_posx, poi_posy;
	gdouble lat, lon;

	if (columns == 0)
		return 0;

	if (mydebug > 20)
	{
		g_print ("Query Result: %s\t%s\t%s\t%s\n",
			values[0], values[1], values[2], values[3]);
	}

	lat = g_strtod (values[0], NULL);
	lon = g_strtod (values[1], NULL);

	calcxy (&poi_posx, &poi_posy, lon, lat, current.zoom);

	if ((poi_posx > -50) && (poi_posx < (gui_status.mapview_x + 50)) &&
	   (poi_posy > -50) && (poi_posy < (gui_status.mapview_y + 50)))
	{
		// get next free mem for point
		poi_nr++;
		if (poi_nr > poi_limit)
		{
			poi_limit = poi_nr + 10000;
			if (mydebug > 20)
				g_print ("Try to allocate Memory for %ld poi\n", poi_limit);
			poi_list = g_renew (poi_struct, poi_list, poi_limit);
			if (NULL == poi_list)
			{
				g_print ("Error: Cannot allocate Memory for %ld poi\n",
				poi_limit);
				poi_limit = -1;
				return;
			}
		}

		// Save retrieved poi information into structure
		(poi_list + poi_nr)->lat = lat;
		(poi_list + poi_nr)->lon = lon;
		(poi_list + poi_nr)->x = poi_posx;
		(poi_list + poi_nr)->y = poi_posy;
		g_strlcpy ((poi_list + poi_nr)->name, values[2],
			sizeof ((poi_list + poi_nr)->name));
		g_strlcpy ((poi_list + poi_nr)->poi_type, values[3],
			sizeof ((poi_list + poi_nr)->poi_type));
		if (values[4] != NULL)
		{
			g_strlcpy ((poi_list + poi_nr)->comment, values[4],
				sizeof ((poi_list + poi_nr)->comment));
		}
		/*
		 * poi_id        INTEGER      PRIMARY KEY
		 * name          VARCHAR(80)  NOT NULL default 'not specified'
		 * poi_type      VARCHAR(160) NOT NULL default 'unknown'`
		 * lat           DOUBLE       NOT NULL default '0'
		 * lon           DOUBLE       NOT NULL default '0'
		 * alt           DOUBLE                default '0'
		 * comment       VARCHAR(255)          default NULL
		 * last_modified DATETIME     NOT NULL default '0000-00-00'
		 * source_id     INTEGER      NOT NULL default '1'
		 * private       CHAR(1)               default NULL)
		 */
	}
	return 0;
}


/* ******************************************************************
 * db callback for reading currently visible points of interest
 * from openstreetmap data
 */
gint
handle_osm_poi_view_cb (const gchar *name, const gchar *type, const gchar *geometry)
{
	gchar **t_coords;
	gdouble x, y, t_lat, t_lon;
	gchar t_buf[100];
	gint poi_posx, poi_posy;

	if (g_str_has_prefix (geometry, "POINT(") == FALSE)
		return -1;

	t_coords = g_strsplit ((geometry + 6), " ", 2);
	x = g_strtod (t_coords[0], NULL);
	y = g_strtod (t_coords[1], NULL);
#ifdef MAPNIK
	convert_mapnik_coords (&t_lon, &t_lat, x, y, 1);
#endif

	if (mydebug > 40)
	{
		g_printf ("handle_osm_poi_view_cb: %s [%s] - %.6f / %.6f\n",
			name, type, t_lat, t_lon);
	}

	calcxy (&poi_posx, &poi_posy, t_lon, t_lat, current.zoom);

	if ((poi_posx > -50) && (poi_posx < (gui_status.mapview_x + 50)) &&
	   (poi_posy > -50) && (poi_posy < (gui_status.mapview_y + 50)))
	{
		// get next free mem for point
		poi_nr++;
		if (poi_nr > poi_limit)
		{
			poi_limit = poi_nr + 10000;
			if (mydebug > 20)
				g_print ("Try to allocate Memory for %ld poi\n", poi_limit);
			poi_list = g_renew (poi_struct, poi_list, poi_limit);
			if (NULL == poi_list)
			{
				g_print ("Error: Cannot allocate Memory for %ld poi\n",
				poi_limit);
				poi_limit = -1;
				return;
			}
		}

		// Save retrieved poi information into structure
		(poi_list + poi_nr)->lat = t_lat;
		(poi_list + poi_nr)->lon = t_lon;
		(poi_list + poi_nr)->x = poi_posx;
		(poi_list + poi_nr)->y = poi_posy;
		if (g_ascii_strcasecmp (name, "NULL") == 0)
			g_strlcpy ((poi_list + poi_nr)->name, "", sizeof ((poi_list + poi_nr)->name));
		else
			g_strlcpy ((poi_list + poi_nr)->name, name, sizeof ((poi_list + poi_nr)->name));
		g_strlcpy ((poi_list + poi_nr)->poi_type, type, sizeof ((poi_list + poi_nr)->poi_type));
	}

	g_strfreev (t_coords);
	return 0;
}


/* *******************************************************
 * if zoom, xoff, yoff or map are changed 
 * TODO: call this only if the above mentioned events occur!
 */
void
poi_rebuild_list (void)
{
  gchar sql_query[20000];
  GTimeVal t;
  int r, rges;
  time_t ti;

  gdouble lat_ul, lon_ul;
  gdouble lat_ll, lon_ll;
  gdouble lat_ur, lon_ur;
  gdouble lat_lr, lon_lr;
  gdouble lat_min, lon_min;
  gdouble lat_max, lon_max;
  gdouble lat_mid, lon_mid;

  if (!local_config.use_database)
    return;

  if (local_config.showpoi == POIDRAW_NONE)
    {
      if (mydebug > 20)
	g_print ("poi_rebuild_list: POI_draw is off\n");
      return;
    }

  if (mydebug > 20)
    {
      g_print ("\n\npoi_rebuild_list: Start (%d)\n", local_config.showpoi);
    }

  if (!maploaded)
    return;

  if (current.importactive)
    return;

	//TODO: could be optimized:
	//	call this function only on the first run and
	//	when the map scale changes,
	update_poi_type_filter ();

	// calculate the start and stop for lat/lon according to the displayed section
	calcxytopos (0, 0, &lat_ul, &lon_ul, current.zoom);
	calcxytopos (0, gui_status.mapview_y, &lat_ll, &lon_ll, current.zoom);
	calcxytopos (gui_status.mapview_x, 0, &lat_ur, &lon_ur, current.zoom);
	calcxytopos (gui_status.mapview_x, gui_status.mapview_y, &lat_lr, &lon_lr, current.zoom);

	lat_min = min (lat_ll, lat_ul);
	lat_max = max (lat_lr, lat_ur);
	lon_min = min (lon_ll, lon_ul);
	lon_max = max (lon_lr, lon_ur);

	lat_mid = (lat_min + lat_max) / 2;
	lon_mid = (lon_min + lon_max) / 2;

	g_get_current_time (&t);
	ti = t.tv_sec + t.tv_usec / 1000000.0;

	rges = r = 0;
	poi_nr = 0;

	/* query sqlite db */
	if (local_config.showpoi == POIDRAW_ALL && current.poi_osm)
	{
		if (mydebug > 30)
			g_print ("poi_rebuild_list (): Getting all POI Data\n");

		g_snprintf (sql_query, sizeof (sql_query),
		"SELECT lat,lon,name,poi_type,comment FROM"
		" (SELECT * FROM main.poi UNION SELECT * FROM osm.poi)"
		" WHERE (lat BETWEEN %.8f AND %.8f ) AND ( lon BETWEEN %.8f AND %.8f )"
		" AND poi_type IN (%s) LIMIT 20000;",
		lat_min, lat_max, lon_min, lon_max, current.poifilter);
	}
	else if (local_config.showpoi == POIDRAW_OSM && current.poi_osm)
	{
		if (mydebug > 30)
			g_print ("poi_rebuild_list (): Getting only OSM POI Data\n");

		g_snprintf (sql_query, sizeof (sql_query),
		"SELECT lat,lon,name,poi_type,comment FROM osm.poi"
		" WHERE (lat BETWEEN %.8f AND %.8f ) AND ( lon BETWEEN %.8f AND %.8f )"
		" AND poi_type IN (%s) LIMIT 20000;",
		lat_min, lat_max, lon_min, lon_max, current.poifilter);
	}
	else if (local_config.showpoi == POIDRAW_USER || local_config.showpoi == POIDRAW_ALL)
	{
		if (mydebug > 30)
			g_print ("poi_rebuild_list (): Getting only User POI Data\n");

		g_snprintf (sql_query, sizeof (sql_query),
		"SELECT lat,lon,name,poi_type,comment FROM main.poi"
		" WHERE (lat BETWEEN %.8f AND %.8f ) AND ( lon BETWEEN %.8f AND %.8f )"
		" AND poi_type IN (%s) LIMIT 20000;",
		lat_min, lat_max, lon_min, lon_max, current.poifilter);
	}
	else
		return;

	db_poi_get (sql_query, handle_poi_view_cb, DB_WP_USER);

	/* query postgis db */
#ifdef MAPNIK
//	gdouble x1, y1, x2, y2;
//	convert_mapnik_coords(&x1, &y1, lon_min, lat_min, 0);
//	convert_mapnik_coords(&x2, &y2, lon_max, lat_max, 0);
//	g_snprintf (sql_query, sizeof (sql_query),
//		"SELECT name,poi,ASTEXT(way) AS geometry FROM planet_osm_point"
//		" WHERE SetSRID(way,-1) && SetSRID('BOX3D(%.8f %.8f , %.8f %.8f)'::box3d,-1)"
//		" AND poi IN (%s) LIMIT 20000;", x1, y1, x2, y2, current.poifilter);
//	db_poi_get (sql_query, handle_osm_poi_view_cb, DB_WP_OSM);
#endif

	poi_list_count = poi_nr;

	// print time for getting Data
	g_get_current_time (&t);
	ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
	if (mydebug > 20)
		g_printf (_("%ld(%d) rows read in %.2f seconds\n"),
			poi_list_count, rges, (gdouble) ti);


	/* remember where the data belongs to */
	poi_lat_lr = lat_lr;
	poi_lon_lr = lon_lr;
	poi_lat_ul = lat_ul;
	poi_lon_ul = lon_ul;

	if (mydebug > 20)
		g_print ("poi_rebuild_list: End\n\n");
}


/* *******************************************************
 * draw poi_ on image 
 TODO: find free space on drawing area. So the Text doesn't overlap
*/
void
poi_draw_list (gboolean draw_now)
{
  gint i;
  GdkPixbuf *icon;
  gchar *t_path;
  GtkTreeIter t_iter;
  gboolean t_lb;

  if (!local_config.use_database)
    return;

  if (current.importactive)
    return;

  if (!maploaded)
    return;

  if (!local_config.showpoi)
    {
      if (mydebug > 20)
	printf ("poi_draw_list: POI_draw is off\n");
      return;
    }


	if (mydebug > 20)
		g_print ("poi_draw_list: Start\t\t\tvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");

	if (poi_check_if_moved () || draw_now)
		poi_rebuild_list ();

  /* ------------------------------------------------------------------ */
  /*  draw poi_list points */
	if (mydebug > 20)
		g_printf ("poi_draw_list: drawing %ld points\n", poi_list_count);

  for (i = 1; i <= poi_list_count; i++)
  {
      gdouble posx, posy;

      posx = (poi_list + i)->x;
      posy = (poi_list + i)->y;

	if ((posx >= 0) && (posx < gui_status.mapview_x) &&
	   (posy >= 0) && (posy < gui_status.mapview_y))
	{
		gdk_gc_set_line_attributes (kontext_map, 2, 0, 0, 0);

		t_path = g_hash_table_lookup (poi_types_hash, (poi_list + i)->poi_type);
		if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (poi_types_tree),
		   &t_iter, t_path))
		{
			gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &t_iter,
				POITYPE_ICON, &icon,
				POITYPE_LABEL, &t_lb, -1);
		}
		else
			icon = NULL;

		if (icon != NULL)
		{
			if (poi_list_count < 2000)
			{
				gint wx = gdk_pixbuf_get_width (icon);
				gint wy = gdk_pixbuf_get_height (icon);

				if (local_config.showfriends
				   && g_str_has_prefix ((poi_list + i)->poi_type, "people.friendsd"))
				{
					draw_posmarker (posx, posy,
						(atoi ((poi_list + i)->comment)) * M_PI / 180.0,
						&colors.lightorange, 4, FALSE, FALSE);
				}

				gdk_draw_pixbuf (drawable, kontext_map, icon,
					0, 0, posx - wx / 2, posy - wy / 2,
					wx, wy, GDK_RGB_DITHER_NONE, 0, 0);
			}
		}
		else
		{
			gdk_gc_set_foreground (kontext_map, &colors.red);
			/* Only draw small + if more than ... Points */
			if (poi_list_count < 20000)
				draw_plus_sign (posx, posy);
			else
				draw_small_plus_sign (posx, posy);
		}

		/* draw friends label in configured color */
		if (local_config.showfriends
		    && g_str_has_prefix ((poi_list + i)->poi_type, "people.friendsd"))
		{
			draw_label_friend ((poi_list + i)->name, posx, posy);
		}
		/* draw label only if we display less than 1000 POIs */
		else if (poi_list_count < 1000)
		{
			if (t_lb)
				draw_label ((poi_list + i)->name, posx, posy);
		}
	}
  }

	if (mydebug > 20)
		g_print ("poi_draw_list: End\t\t\t^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
}


/* *******************************************************
 * query all Info for Points in area around lat/lon
*/
void
poi_query_area (gdouble lat1, gdouble lon1, gdouble lat2, gdouble lon2)
{
  gint i;
  printf ("Query: %f ... %f , %f ... %f\n", lat1, lat2, lon1, lon2);

  for (i = 0; i < poi_list_count; i++)
    {
      //printf ("check POI: %ld: %f %f :%s\n",i,(poi_list + i)->lat, (poi_list + i)->lon,(poi_list + i)->name);
      if ((lat1 <= (poi_list + i)->lat) &&
	  ((poi_list + i)->lat <= lat2) &&
	  (lon1 <= (poi_list + i)->lon) && ((poi_list + i)->lon <= lon2))
	{
	  printf ("Query POI: %d - %f %f : %s [%s]\n",
		  i, (poi_list + i)->lat, (poi_list + i)->lon,
		  (poi_list + i)->name, (poi_list + i)->poi_type);
	}
    }
}


/* *******************************************************
 */
void
poi_init (void)
{
	poi_limit = 40000;
	poi_list = g_new (poi_struct, poi_limit);
	poi_result = g_new (poi_struct, poi_limit);
	if (poi_list == NULL || poi_result == NULL)
	{
		g_print ("Error: Cannot allocate Memory for %ld poi\n",
			poi_limit);
		poi_limit = -1;
		return;
	}

	/* init buffer for poi buffer */
	poi_buf = g_new0 (poi_struct, 1);

	/* init gtk-list for storage of results of poi-search */
	poi_result_tree = gtk_list_store_new (RES_COLUMS,
		G_TYPE_INT,		/* poi.poi_id */
		G_TYPE_STRING,		/* poi.name */
		G_TYPE_STRING,		/* poi.comment */
		G_TYPE_STRING,		/* poi_type.title */
		G_TYPE_STRING,		/* poi_type.name */
		GDK_TYPE_PIXBUF,	/* poi_type.icon */
		G_TYPE_STRING,		/* formatted distance */
		G_TYPE_DOUBLE,		/* numerical distance */
		G_TYPE_DOUBLE,		/* numerical latitude */
		G_TYPE_DOUBLE,		/* numerical longitude */
		G_TYPE_INT		/* poi.source_id */
		);

	/* init gtk-list for storage of poi extra info */
	poi_info.extra_list = gtk_list_store_new (2,
		G_TYPE_STRING,		/* key */
		G_TYPE_STRING		/* value */
		);

	/* init gtk-tree for storage of poi-type data */
	poi_types_tree = gtk_tree_store_new (POITYPE_COLUMS,
		G_TYPE_INT,		/* id */
		G_TYPE_STRING,		/* name */
		GDK_TYPE_PIXBUF,	/* icon */
		G_TYPE_INT,		/* scale_min */
		G_TYPE_INT,		/* scale_max */
		G_TYPE_STRING,		/* description */
		G_TYPE_STRING,		/* title */
		G_TYPE_BOOLEAN,		/* select */
		G_TYPE_BOOLEAN		/* label */
		);

	/* init filtered gtk-tree for use in comboboxes */
	poi_types_tree_filtered = gtk_tree_model_filter_new
		(GTK_TREE_MODEL (poi_types_tree), NULL);
	gtk_tree_model_filter_set_visible_func (
		GTK_TREE_MODEL_FILTER (poi_types_tree_filtered),
		treefilterfunc_cb, NULL, NULL);

	/* read poi-type data and icons from geoinfo.db and set filters */
	get_poitype_tree ();
}


void
cleanup_poi (void)
{
	g_free (poi_buf);
	g_free (poi_list);
	g_free (poi_result);
}
