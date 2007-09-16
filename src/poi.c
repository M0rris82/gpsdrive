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
#include "config.h"
#include "gettext.h"
#include "icons.h"
#include <gpsdrive_config.h>
#include "gui.h"

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
extern gint isnight, disableisnight;
extern color_struct colors;
extern gdouble wp_saved_target_lat, wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat, wp_saved_posmode_lon;
extern gint debug, mydebug;
extern GtkWidget *map_drawingarea;
extern gint usesql;
extern glong mapscale;
extern gdouble dbdistance;
extern gint friends_poi_id[TRAVEL_N_MODES];
extern coordinate_struct coords;
extern currentstatus_struct current;
extern GdkGC *kontext_map;

extern GdkPixbuf *posmarker_img;
extern GdkGC *kontext;

char txt[5000];
PangoLayout *poi_label_layout;
#include "mysql/mysql.h"

extern MYSQL mysql;
extern MYSQL_RES *res;
extern MYSQL_ROW row;
#define MAXDBNAME 30
extern char poitypetable[MAXDBNAME];

// keep actual visible POIs in Memory
poi_struct *poi_list;

// keep POI info from last search result in Memory
poi_struct *poi_result;
GtkListStore *poi_result_tree;

glong poi_nr;			// current number of poi to count
glong poi_list_count;		// max index of POIs actually in memory
guint poi_result_count;		// max index of POIs found in POI search
glong poi_limit = -1;		// max allowed index (if you need more you have to alloc memory)

gchar poi_label_font[100];
GdkColor poi_colorv;
PangoFontDescription *pfd;
PangoLayout *poi_label_layout;

poi_type_struct poi_type_list[poi_type_list_max];
int poi_type_list_count = 0;
GtkTreeStore *poi_types_tree;

gdouble poi_lat_lr = 0, poi_lon_lr = 0;
gdouble poi_lat_ul = 0, poi_lon_ul = 0;


/* ******************************************************************   */

void poi_rebuild_list (void);
void get_poitype_tree (void);


/* *******************************************************
 * check, which poi_types should be shown in the map.
 * filtering is only done on the base category level.
 */
void
update_poi_type_filter ()
{
	GtkTreeIter t_iter;
	gboolean t_selected;
	gchar *t_name;
	gchar t_string[200];
	gchar t_config[2000];

	if (mydebug > 21)
		fprintf (stderr, "update_poi_type_filter:\n");

	g_strlcpy (current.poifilter, "AND (", sizeof (current.poifilter));
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
				"poi_type.name NOT LIKE \"%s%%\" AND ",
				t_name);
			g_strlcat (current.poifilter,
				t_string, sizeof (current.poifilter));
				
			/* build settings string for config file */
			g_snprintf (t_config, sizeof (t_config),
				"%s|", t_name);
			g_strlcat (local_config.poi_filter,
				t_config, sizeof (local_config.poi_filter));
		}
	}
	while (gtk_tree_model_iter_next
		(GTK_TREE_MODEL (poi_types_tree), &t_iter));

	g_strlcat (current.poifilter, "TRUE)", sizeof (current.poifilter));

	g_free (t_name);

	current.needtosave = TRUE;
	poi_draw_list (TRUE);
}


/* *******************************************************
 * search database for POIs filtered by data entered
 * into the POI-Lookup window
 */
guint
poi_get_results (const gchar *text, const gchar *pdist, const gint posflag, const gint typeflag, const gchar *type)
{
	gdouble lat, lon, dist;
	gdouble lat_min, lon_min;
	gdouble lat_max, lon_max;
	gdouble dist_lat, dist_lon;
	gdouble temp_lon, temp_lat;
	gdouble temp_dist_num;
	
	char sql_query[5000];
	char type_filter[3000];
	gchar *temp_text;
	char temp_dist[15];
	int r, rges;
	int temp_id;
	
	GtkTreeIter iter;
	
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
	
	/* choose poi_type_ids to search */
	if (typeflag)
	{
		g_snprintf (type_filter, sizeof (type_filter),
			"AND (poi_type.name LIKE \'%s%%\')",type);
	}
	else
	{
		g_snprintf (type_filter, sizeof (type_filter), " ");
	}

	/* prepare search text for database query */
	temp_text = escape_sql_string (text);
	g_strdelimit (temp_text, "*", '%');
		
	g_snprintf (sql_query, sizeof (sql_query),
		"SELECT poi.poi_id,poi.name,poi.comment,poi.poi_type_id,"
		"poi.lon,poi.lat FROM poi INNER JOIN poi_type ON"
		" poi.poi_type_id=poi_type.poi_type_id "
		" WHERE ( lat BETWEEN %.6f AND %.6f ) AND ( lon BETWEEN %.6f"
		" AND %.6f ) AND (poi.name LIKE '%%%s%%' OR comment LIKE"
		" '%%%s%%') %s ORDER BY"
		" (pow((poi.lat-%.6f),2)+pow((poi.lon-%.6f),2)) LIMIT %d;",
		lat_min, lat_max, lon_min, lon_max, temp_text, temp_text,
		type_filter, lat, lon, local_config.poi_results_max);

	if (mydebug > 20)
		printf ("poi_get_results: POI mysql query: %s\n", sql_query);

	if (dl_mysql_query (&mysql, sql_query))
	{
		printf ("poi_get_results: Error in query: \n");
		fprintf (stderr, "poi_get_results: Error in query: %s\n",
				dl_mysql_error (&mysql));
		return 0;
	}

	if (!(res = dl_mysql_store_result (&mysql)))
	{
		fprintf (stderr, "Error in store results: %s\n",
				dl_mysql_error (&mysql));
		dl_mysql_free_result (res);
		res = NULL;
		return 0;
	}
	
	g_free (temp_text);
	
	rges = r = 0;
	poi_nr = 0;
	while ((row = dl_mysql_fetch_row (res)))
	{
		rges++;
		
		if (mydebug > 20)
			fprintf (stderr, "Query Result: %s\t%s\t%s\t%s\n",
					row[0], row[1], row[2], row[3]);
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
				return 0;
			}
	    }

		// Save retrieved poi information into structure
		(poi_result + poi_nr)->poi_id = (gint) g_strtod (row[0], NULL);
				g_strlcpy ((poi_result + poi_nr)->name, row[1], sizeof ((poi_result + poi_nr)->name));
		if (row[2] == NULL)
			g_strlcpy ((poi_result + poi_nr)->comment, "n/a", sizeof ((poi_result + poi_nr)->comment));		
		else
			g_strlcpy ((poi_result + poi_nr)->comment, row[2], sizeof ((poi_result + poi_nr)->comment));		
		(poi_result + poi_nr)->poi_type_id = (gint) g_strtod (row[3], NULL);
		temp_id = (gint) g_strtod (row[3], NULL);
		(poi_result + poi_nr)->lon = g_strtod (row[4], NULL);
		temp_lon = g_strtod (row[4], NULL);
		(poi_result + poi_nr)->lat = g_strtod (row[5], NULL);
		temp_lat = g_strtod (row[5], NULL);
		temp_dist_num = calcdist (temp_lon, temp_lat);
		g_snprintf (temp_dist, sizeof (temp_dist), "%9.3f", temp_dist_num);
		
		gtk_list_store_append (poi_result_tree, &iter);
		gtk_list_store_set (poi_result_tree, &iter,
			RESULT_ID, (poi_result + poi_nr)->poi_id,
			RESULT_NAME, (poi_result + poi_nr)->name,
			RESULT_COMMENT,  (poi_result + poi_nr)->comment,
			RESULT_TYPE_TITLE, poi_type_list[temp_id].title,
			RESULT_TYPE_NAME, poi_type_list[temp_id].name,
			RESULT_TYPE_ICON, poi_type_list [temp_id].icon,
			RESULT_DISTANCE, temp_dist,
			RESULT_DIST_NUM, temp_dist_num,
			RESULT_LAT, temp_lat,
			RESULT_LON, temp_lon,
			-1);

		/* check for friendsd entry */
		if (g_str_has_prefix(poi_type_list[temp_id].name, "people.friendsd"))
			g_print ("\nfriend\n");

	}

	poi_result_count = poi_nr;

	if (mydebug > 20)
		printf (_("%ld(%d) rows read\n"), poi_list_count, rges);

	if (!dl_mysql_eof (res))
	{
		fprintf (stderr, "poi_get_results: Error in dl_mysql_eof: %s\n",
				dl_mysql_error (&mysql));
		dl_mysql_free_result (res);
		res = NULL;
		return 0;
	}

	dl_mysql_free_result (res);
	res = NULL;
	
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

  if (!local_config.showpoilabel)
    {
      if (mydebug > 20)
	printf ("draw_label: drawing of label is disabled\n");
      return;
    }

  if (mydebug > 30)
    fprintf (stderr, "draw_label(%s,%g,%g)\n", txt, posx, posy);

  gdk_gc_set_foreground (kontext_map, &colors.textback);

  poi_label_layout = gtk_widget_create_pango_layout (map_drawingarea, txt);
  pfd = pango_font_description_from_string ("Sans 8");
  if (poi_list_count > 200)
    pfd = pango_font_description_from_string ("Sans 6");

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

  calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, current.zoom);
  calcxytopos (0, 0, &lat_ul, &lon_ul, current.zoom);

  if (poi_lat_lr == lat_lr && poi_lon_lr == lon_lr &&
      poi_lat_ul == lat_ul && poi_lon_ul == lon_ul)
    return 0;
  return 1;
}


/* ******************************************************************
 * get poi_type_id from given poi_type name
*/
gint
poi_type_id_from_name (gchar name[POI_TYPE_LIST_STRING_LENGTH])
{
    int i;
    for (i = 0; i < poi_type_list_max; i++)
      {
		if (strcmp (poi_type_list[i].name,name) == 0)
	    return poi_type_list[i].poi_type_id;
      }
	return 1;  // return poi_type 1 = 'unknown' if not in table
}


/* ******************************************************************
 * check if poi is friend
*/
gboolean
poi_is_friend (gint type)
{
	gint i;
	
	for (i = 0; i < TRAVEL_N_MODES; i++)
	{
		if (friends_poi_id[i] == type)
			return TRUE;
	}
	return FALSE;
}

/* ******************************************************************
 * add new row to poitype tree
*/
static gboolean 
poitypetree_addrow (guint i, GtkTreeIter *parent)
{
	GtkTreeIter iter;
	
	gtk_tree_store_append (poi_types_tree, &iter, parent);
 
	gtk_tree_store_set (poi_types_tree, &iter,
		POITYPE_ID, poi_type_list[i].poi_type_id,
		POITYPE_NAME, poi_type_list[i].name,
		POITYPE_ICON, poi_type_list[i].icon,
		POITYPE_SCALE_MIN, poi_type_list[i].scale_min,
		POITYPE_SCALE_MAX, poi_type_list[i].scale_max,
		POITYPE_DESCRIPTION, poi_type_list[i].description,
		POITYPE_TITLE, poi_type_list[i].title,
		POITYPE_SELECT, FALSE,
		-1);

	if (mydebug > 30)
	{
		fprintf (stderr,
			"poitypetree_addrow:added %d - %s\n",
			poi_type_list[i].poi_type_id,
			poi_type_list[i].name);
	}

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

	if (mydebug > 20)
		fprintf (stderr, "create_poitype_tree:\n");

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

	return gtk_tree_model_get_iter_first
		(GTK_TREE_MODEL (poi_types_tree), &current.poitype_iter);
}


/* ******************************************************************
 * get a list of all possible poi_types and load their icons
*/
void
get_poitype_tree (void)
{
	if (mydebug > 25)
		printf ("get_poitype_tree ()\n");

	// Clear poi_type_tree
	gtk_tree_store_clear (poi_types_tree);

	guint counter = 0;
	guint max_level = 0;

  // get poi_type info from icons.xml
  {
	xmlTextReaderPtr xml_reader;
	gchar iconsxml_file[200];
	gint valid_poi_type = 0;
	gint xml_status = 0; // 0 = empty, 1 = node, -1 = error
	gint tag_type = 0;  // 1 = start, 3 = #text, 15 = end
	xmlChar *tag_name;
	gchar t_scale_max[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_scale_min[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_title[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_title_en[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_title_lang[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_description[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_desc_en[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_desc_lang[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_name[POI_TYPE_LIST_STRING_LENGTH];
	gchar t_poi_type_id[POI_TYPE_LIST_STRING_LENGTH];
	guint i,j;

	g_snprintf (iconsxml_file, sizeof (iconsxml_file),
		"./data/map-icons/icons.xml" );
	xml_reader = xmlNewTextReaderFilename(iconsxml_file);

	if (xml_reader == NULL)
	{
		g_snprintf (iconsxml_file, sizeof (iconsxml_file),
			"%s/icons.xml", local_config.dir_home);
		xml_reader = xmlNewTextReaderFilename(iconsxml_file);
	}
	if (xml_reader == NULL)
	{
		g_snprintf (iconsxml_file, sizeof (iconsxml_file),
			"%s/map-icons/icons.xml", DATADIR);
		xml_reader = xmlNewTextReaderFilename(iconsxml_file);
	}
	if (xml_reader == NULL)
	{
		fprintf (stderr, "get_poitype_tree: File %s not found!\n",
			iconsxml_file);
			return;
	}
	if (mydebug > 10)
	{
		fprintf (stderr, "get_poitype_tree: Trying to parse file:"
		"%s\n", iconsxml_file);
	}
	xml_status = xmlTextReaderRead(xml_reader);
	while (xml_status == 1) /* parse complete file */
	{
		g_strlcpy (t_title, "n/a", sizeof(t_title));
		g_strlcpy (t_title_en, "n/a", sizeof(t_title));
		g_strlcpy (t_description, "n/a", sizeof(t_description));
		g_strlcpy (t_desc_en, "n/a", sizeof(t_description));
		g_strlcpy (t_name, "n/a", sizeof(t_name));
		g_strlcpy (t_scale_min, "1", sizeof(t_scale_min));
		g_strlcpy (t_scale_max, "50000", sizeof(t_scale_max));
		valid_poi_type = 0;
		tag_type = xmlTextReaderNodeType(xml_reader);
		tag_name = xmlTextReaderName(xml_reader);
		if (tag_type == 1 && xmlStrEqual(tag_name, BAD_CAST "rule"))
		/* START element rule */
		{
			if (mydebug > 50) fprintf (stderr, "----------\n");
			while ( tag_type != 15 || !xmlStrEqual(tag_name,
				BAD_CAST "rule")) /* inside element rule */
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 1)
		{
			if (xmlStrEqual(tag_name, BAD_CAST 	"condition") && xmlStrEqual (xmlTextReaderGetAttribute(xml_reader, BAD_CAST "k"), BAD_CAST "poi"))
		{
			if (mydebug > 4)
			fprintf(stderr, "*** poi = %s\n", xmlTextReaderGetAttribute(xml_reader, BAD_CAST "v"));
			valid_poi_type = 1;
			counter++;
			continue;
		}
			if (xmlStrEqual(tag_name, BAD_CAST "scale_min")) /* scale_min */
		{
			do
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 3)
		{
			g_snprintf(t_scale_min, sizeof(t_scale_min), "%s", xmlTextReaderConstValue(xml_reader));
			continue;
		}
		} while (tag_type != 15);
			continue;
		}
			if (xmlStrEqual(tag_name, BAD_CAST "scale_max"))  /* scale_max */
		{
			do
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 3)
		{
			g_snprintf(t_scale_max, sizeof(t_scale_max), "%s", xmlTextReaderConstValue(xml_reader));
			continue;
		}
		} while (tag_type != 15);
			continue;
		}
			if (xmlStrEqual(tag_name, BAD_CAST "title")) /* title */
		{
			g_snprintf(t_title_lang, sizeof(t_title_lang), "%s", xmlTextReaderGetAttribute(xml_reader, BAD_CAST "lang"));
			do
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 3)
		{
			if (strcmp(t_title_lang, "en")==0)
		{
			g_snprintf(t_title_en, sizeof(t_title_en), "%s", xmlTextReaderConstValue (xml_reader));
			continue;
		}
			if (strcmp(t_title_lang, language)==0)
		{
			g_snprintf(t_title, sizeof(t_title), "%s", xmlTextReaderConstValue(xml_reader));
			continue;
		}
		}
		} while (tag_type != 15);
			continue;
		}
			if (xmlStrEqual(tag_name, BAD_CAST "description"))  /* description */
		{
			g_snprintf(t_desc_lang, sizeof(t_desc_lang), "%s", xmlTextReaderGetAttribute(xml_reader, BAD_CAST "lang"));
			do
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 3)
		{
			if (strcmp(t_desc_lang, "en")==0)
		{
			g_snprintf(t_desc_en, sizeof(t_desc_en), "%s", xmlTextReaderConstValue(xml_reader));
			continue;
		}
			if (strcmp(t_desc_lang, language)==0)
		{
			g_snprintf(t_description, sizeof(t_description), "%s", xmlTextReaderConstValue(xml_reader));
			continue;
		}
		}
		} while (tag_type != 15);
			continue;
		}
			if (xmlStrEqual(tag_name, BAD_CAST "geoinfo"))  /* geoinfo */
		{
			while ( tag_type != 15 || !xmlStrEqual(tag_name, BAD_CAST "geoinfo")) /* inside element geoinfo */
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 1)
		{
			if (xmlStrEqual(tag_name, BAD_CAST "name")) /* name */
		{
			do
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 3)
		{
			g_snprintf(t_name, sizeof(t_name), "%s", xmlTextReaderConstValue(xml_reader));
			continue;
		}
		} while (tag_type != 15);
			continue;
		}
			if (xmlStrEqual(tag_name, BAD_CAST "poi_type_id")) /* poi_type_id */
		{
			do
		{
			xml_status = xmlTextReaderRead(xml_reader);
			tag_type = xmlTextReaderNodeType(xml_reader);
			tag_name = xmlTextReaderName(xml_reader);
			if (tag_type == 3)
		{
			g_snprintf(t_poi_type_id, sizeof(t_poi_type_id), "%s", xmlTextReaderConstValue(xml_reader));
			continue;
		}
		} while (tag_type != 15);
		continue;
		}
		}
		}
			continue;
		} /* END element geoinfo */
		}
		}
		if (valid_poi_type)
		{
			int index = (gint) g_strtod (t_poi_type_id, NULL);
			if (index >= poi_type_list_max)
			{
				fprintf (stderr, "get_poitype_tree: "
				"index(%d) > poi_type_list_max(%d)\n",
				index, poi_type_list_max);
				continue;
			}
			if (poi_type_list_count < index)
			{
				poi_type_list_count = index;
			}
			poi_type_list[index].poi_type_id = index;
			poi_type_list[index].scale_min = (gint) g_strtod (t_scale_min, NULL);
			poi_type_list[index].scale_max = (gint) g_strtod (t_scale_max, NULL);

			/* use english title/description,
			 * if no localized info is available */
			if (strcmp(t_description, "n/a")==0)
				g_strlcpy(t_description, t_desc_en, sizeof(t_description));
			if (strcmp(t_title, "n/a")==0)
				g_strlcpy(t_title, t_title_en, sizeof(t_title));


			g_strlcpy (poi_type_list[index].description,
				t_description, sizeof
				(poi_type_list[index].description));
			g_strlcpy (poi_type_list[index].title, t_title,
				sizeof (poi_type_list[index].title));
			g_strlcpy (poi_type_list[index].name, t_name,
				sizeof(poi_type_list[index].name));
			g_strlcpy (poi_type_list[index].icon_name, t_name,
				sizeof(poi_type_list[index].icon_name));
			poi_type_list[index].icon = read_themed_icon
				(poi_type_list[index].icon_name);
			
			j = 0;			
			for (i=0; i < strlen (poi_type_list[index].name); i++)
			{
				if (g_str_has_prefix
					((poi_type_list[index].name + i) , "."))
				{
					j++;
				}
			}
			poi_type_list[index].level = j;
			if (max_level < j)
			{
				max_level = j;
			}
			
			if (poi_type_list[index].icon == NULL)
			{
				if (mydebug > 40)
					printf ("get_poitype_tree: %3d:Icon '%s' for '%s'\tnot found\n",
					index, poi_type_list[index].icon_name, poi_type_list[index].name);
				if (do_unit_test)
					exit (-1);
			}
		}
		}  /* END element rule */
		xml_status = xmlTextReaderRead(xml_reader);
	}
	xmlFreeTextReader(xml_reader);
	if (xml_status != 0)
		fprintf(stderr,
		"get_poitype_tree: Failed to parse file: %s\n",
		iconsxml_file);
	else
		fprintf (stdout, "Read %d POI-Types from %s\n",
		counter, iconsxml_file);

  }

	if (mydebug > 20)
		fprintf (stderr,
		"get_poitype_tree: Loaded %d Icons for poi_types 1 - %d\n",
		counter, poi_type_list_count);

	create_poitype_tree (max_level);
}


/* *******************************************************
 * if zoom, xoff, yoff or map are changed 
 * TODO: use the real datatype for reading from database
 * (don't convert string to double)
 * TODO: call this only if the above mentioned events occur!
 */
void
poi_rebuild_list (void)
{
  char sql_query[5000];
  struct timeval t;
  int r, rges;
  time_t ti;

  gdouble lat_ul, lon_ul;
  gdouble lat_ll, lon_ll;
  gdouble lat_ur, lon_ur;
  gdouble lat_lr, lon_lr;
  gdouble lat_min, lon_min;
  gdouble lat_max, lon_max;
  gdouble lat_mid, lon_mid;


  if (!usesql)
    return;

  if (!local_config.showpoi)
    {
      if (mydebug > 20)
	printf ("poi_rebuild_list: POI_draw is off\n");
      return;
    }

  if (mydebug > 20)
    {
      printf
	("poi_rebuild_list: Start\t\t\t\t\t\tvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
    }


  if (!maploaded)
    return;

  if (current.importactive)
    return;


  // calculate the start and stop for lat/lon according to the displayed section
  calcxytopos (0, 0, &lat_ul, &lon_ul, current.zoom);
  calcxytopos (0, SCREEN_Y, &lat_ll, &lon_ll, current.zoom);
  calcxytopos (SCREEN_X, 0, &lat_ur, &lon_ur, current.zoom);
  calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, current.zoom);

  lat_min = min (lat_ll, lat_ul);
  lat_max = max (lat_lr, lat_ur);
  lon_min = min (lon_ll, lon_ul);
  lon_max = max (lon_lr, lon_ur);

  lat_mid = (lat_min + lat_max) / 2;
  lon_mid = (lon_min + lon_max) / 2;

  gdouble poi_posx, poi_posy;
  gettimeofday (&t, NULL);
  ti = t.tv_sec + t.tv_usec / 1000000.0;

  g_snprintf (sql_query, sizeof (sql_query),
     "SELECT poi.lat,poi.lon,poi.name,poi.poi_type_id,poi.source_id FROM poi "
     "INNER JOIN poi_type ON poi.poi_type_id=poi_type.poi_type_id "
     "WHERE ( lat BETWEEN %.6f AND %.6f ) AND ( lon BETWEEN %.6f AND %.6f ) "
     "AND ( %ld BETWEEN scale_min AND scale_max ) %s LIMIT 20000;",
     lat_min, lat_max, lon_min, lon_max, current.mapscale, current.poifilter);

  if (mydebug > 20)
  {
    printf ("poi_rebuild_list: POI mysql query: %s\n", sql_query);
  }

  if (dl_mysql_query (&mysql, sql_query))
    {
      printf ("poi_rebuild_list: Error in query: \n");
      fprintf (stderr, "poi_rebuild_list: Error in query: %s\n",
	       dl_mysql_error (&mysql));
      return;
    }

  if (!(res = dl_mysql_store_result (&mysql)))
    {
      fprintf (stderr, "Error in store results: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
      return;
    }
	
  rges = r = 0;
  poi_nr = 0;
  while ((row = dl_mysql_fetch_row (res)))
    {
      rges++;
      gdouble lat, lon;

      if (mydebug > 20)
	fprintf (stderr, "Query Result: %s\t%s\t%s\t%s\n",
		 row[0], row[1], row[2], row[3]);

      lat = g_strtod (row[0], NULL);
      lon = g_strtod (row[1], NULL);
      calcxy (&poi_posx, &poi_posy, lon, lat, current.zoom);

      if ((poi_posx > -50) && (poi_posx < (SCREEN_X + 50)) &&
	  (poi_posy > -50) && (poi_posy < (SCREEN_Y + 50)))
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
	  g_strlcpy ((poi_list + poi_nr)->name, row[2],
		     sizeof ((poi_list + poi_nr)->name));
	  (poi_list + poi_nr)->poi_type_id = (gint) g_strtod (row[3], NULL);
	  if (mydebug > 20)
	    {
	      g_snprintf ((poi_list + poi_nr)->name,
			  sizeof ((poi_list + poi_nr)->name), "%s %s"
			  //"\n(%.4f ,%.4f)",
			  //                  (poi_list + poi_nr)->poi_type_id,
			  , row[2], row[4]
			  // , lat, lon
		);
	      /*
	       * `type_id` int(11) NOT NULL default \'0\',
	       * `alt` double default \'0\',
	       * `proximity` float default \'0\',
	       * `comment` varchar(255) default NULL,
	       * `scale_min` smallint(6) NOT NULL default \'0\',
	       * `scale_max` smallint(6) NOT NULL default \'0\',
	       * `last_modified` date NOT NULL default \'0000-00-00\',
	       * `url` varchar(160) NULL ,
	       * `address_id` int(11) default \'0\',
	       * `source_id` int(11) NOT NULL default \'0\',
	       */
	    }

	  /*
	   * if ( mydebug > 20 ) 
	   * printf ("DB: %f %f \t( x:%f, y:%f )\t%s\n",
	   * (poi_list + poi_nr)->lat, (poi_list + poi_nr)->lon, 
	   * (poi_list + poi_nr)->x, (poi_list + poi_nr)->y, 
	   * (poi_list + poi_nr)->name
	   * );
	   */
	}
    }


  poi_list_count = poi_nr;

  // print time for getting Data
  gettimeofday (&t, NULL);
  ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
  if (mydebug > 20)
    printf (_("%ld(%d) rows read in %.2f seconds\n"), poi_list_count,
	    rges, (gdouble) ti);


  /* remember where the data belongs to */
  poi_lat_lr = lat_lr;
  poi_lon_lr = lon_lr;
  poi_lat_ul = lat_ul;
  poi_lon_ul = lon_ul;

  if (!dl_mysql_eof (res))
    {
      fprintf (stderr, "poi_rebuild_list: Error in dl_mysql_eof: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
      return;
    }

  dl_mysql_free_result (res);
  res = NULL;

  if (mydebug > 20)
    {
      printf ("poi_rebuild_list: End \t\t\t\t\t\t");
      printf ("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    }
}


/* *******************************************************
 * draw poi_ on image 
 TODO: find free space on drawing area. So the Text doesn't overlap
*/
void
poi_draw_list (gboolean draw_now)
{
  gint i;

  if (!usesql)
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
    printf
      ("poi_draw_list: Start\t\t\tvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");

  if (poi_check_if_moved () || draw_now)
    poi_rebuild_list ();

  /* ------------------------------------------------------------------ */
  /*  draw poi_list points */
  if (mydebug > 20)
    printf ("poi_draw_list: drawing %ld points\n", poi_list_count);

  for (i = 1; i <= poi_list_count; i++)
    {
      gdouble posx, posy;

      posx = (poi_list + i)->x;
      posy = (poi_list + i)->y;

      if ((posx >= 0) && (posx < SCREEN_X) &&
	  (posy >= 0) && (posy < SCREEN_Y))
	{


	  gdk_gc_set_line_attributes (kontext_map, 2, 0, 0, 0);

	  g_strlcpy (txt, (poi_list + i)->name, sizeof (txt));

	    GdkPixbuf *icon;
	    int icon_index = (poi_list + i)->poi_type_id;
	    icon = poi_type_list[icon_index].icon;

	    if (icon != NULL && icon_index > 0)
	      {
		if (poi_list_count < 2000)
		  {
		    int wx = gdk_pixbuf_get_width (icon);
		    int wy = gdk_pixbuf_get_height (icon);

		    gdk_draw_pixbuf (drawable, kontext_map, icon,
				     0, 0,
				     posx - wx / 2,
				     posy - wy / 2,
				     wx, wy, GDK_RGB_DITHER_NONE, 0, 0);
		  }
	      }
	    else
	      {
		gdk_gc_set_foreground (kontext_map, &colors.red);
		if (poi_list_count < 20000)
		  {		// Only draw small + if more than ... Points 
		    draw_plus_sign (posx, posy);
		  }
		else
		  {
		    draw_small_plus_sign (posx, posy);
		  }
	      }
	
		/* draw friends label in configured color */
		if (local_config.showfriends && poi_is_friend (icon_index))
		{
			draw_label_friend (txt, posx, posy);
			//draw_posmarker (posx, posy, 45,
			//	&colors.blue, 1, FALSE, FALSE);
		}
		/* draw label only if we display less than 1000 POIs */
		else if (poi_list_count < 1000)
		{
			draw_label (txt, posx, posy);
		}
	}
    }
  if (mydebug > 20)
    printf
      ("poi_draw_list: End\t\t\t^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
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
	  printf ("Query POI: %d: %f %f :%s\n",
		  i, (poi_list + i)->lat, (poi_list + i)->lon,
		  (poi_list + i)->name);
	}
    }
}

/* *******************************************************
 * initialize POI filter from config settings
*/
void init_poi_type_filter ()
{
	GtkTreeIter t_iter;
	gchar *t_name;

	gtk_tree_model_get_iter_first
		(GTK_TREE_MODEL (poi_types_tree), &t_iter);
	do
	{
		gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &t_iter,
			POITYPE_NAME, &t_name, -1);
		
		if (g_strstr_len (local_config.poi_filter,
			sizeof (local_config.poi_filter), t_name))
		{
			gtk_tree_store_set
				(poi_types_tree, &t_iter,
				POITYPE_SELECT, 0, -1);
		}
		else
		{
			gtk_tree_store_set
				(poi_types_tree, &t_iter,
				POITYPE_SELECT, 1, -1);
		}
	}
	while (gtk_tree_model_iter_next
		(GTK_TREE_MODEL (poi_types_tree), &t_iter));

	g_free (t_name);
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
		G_TYPE_DOUBLE		/* numerical longitude */
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
		G_TYPE_BOOLEAN		/* select */
		);

	/* read poi-type data and icons from icons.xml */
	get_poitype_tree ();
	
	/* set poi filter according to config file */
	init_poi_type_filter ();
	update_poi_type_filter ();
}
