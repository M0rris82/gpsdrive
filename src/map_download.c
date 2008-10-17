/***********************************************************************

Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>

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


/*  Include Dateien */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>

#include "gettext.h"
#include "os_specific.h"

#include <curl/curl.h>


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


#include <gpsdrive.h>
#include <map_handler.h>
#include "gpsdrive_config.h"
#include "gui.h"
#include "map_download.h"


extern gint mydebug;
extern mapsstruct *maps;
extern gint nrmaps;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern guistatus_struct gui_status;

enum
{
	MAPSOURCE_LANDSAT,
	MAPSOURCE_OSM_TAH,
	MAPSOURCE_N_ITEMS
};

gboolean mapdl_active = FALSE;
guint mapdl_scale;
gdouble mapdl_lat, mapdl_lon;

static gchar mapdl_url[2000];
static gboolean mapdl_abort = FALSE;
static gint mapdl_zoom;
static gchar mapdl_file_w_path[1024];
static GtkWidget *scale_combobox, *lat_entry, *lon_entry;
static GtkWidget *mapdl_progress;
static GtkTreeModel *types_list, *scales_list;
static CURL *curl_handle;

static struct mapsource_struct
{
	gint type;
	gchar scale[50];
	gint zoom;
	gint scale_int;
} mapsource[] =
{
/* LANDSAT data is 30m resolution per pixel, * scale factor of PIXELFACT = 1:85000,
 *  so anything finer than that is just downloading interpolated noise and you
 *  might as well just use the magnifying glass tool.
 * At the other end, top_gpsworld covering entire planet is about 1:88 million
 */
	MAPSOURCE_LANDSAT, "NASA's OnEarth Landsat Global Mosaic", -1, -1,
	MAPSOURCE_LANDSAT, "1 : 50 000 000", 0, 50000000,
	MAPSOURCE_LANDSAT, "1 : 10 000 000", 0, 10000000,
	MAPSOURCE_LANDSAT, "1 : 5 000 000", 0, 5000000,
	MAPSOURCE_LANDSAT, "1 : 1 000 000", 0, 1000000,
	MAPSOURCE_LANDSAT, "1 : 500 000", 0, 500000,
	MAPSOURCE_LANDSAT, "1 : 250 000", 0, 250000,
	MAPSOURCE_LANDSAT, "1 : 100 000", 0, 100000,
	MAPSOURCE_LANDSAT, "1 : 75 000", 0, 75000,
	MAPSOURCE_LANDSAT, "1 : 50 000", 0, 50000,
	MAPSOURCE_OSM_TAH, "OpenStreetMap Tiles@Home", -1, -1,
	MAPSOURCE_OSM_TAH, "1 : 147 456 000", 1, 256*576000,
	MAPSOURCE_OSM_TAH, "1 : 73 728 000", 2, 128*576000,
	MAPSOURCE_OSM_TAH, "1 : 36 864 000", 3, 64*576000,
	MAPSOURCE_OSM_TAH, "1 : 18 432 000", 4, 32*576000,
	MAPSOURCE_OSM_TAH, "1 : 9 216 000", 5, 16*576000,
	MAPSOURCE_OSM_TAH, "1 : 4 608 000", 6, 8*576000,
	MAPSOURCE_OSM_TAH, "1 : 2 304 000", 7, 4*576000,
	MAPSOURCE_OSM_TAH, "1 : 1 152 000", 8, 2*576000,
	MAPSOURCE_OSM_TAH, "1 : 576 000", 9, 576000,
	MAPSOURCE_OSM_TAH, "1 : 288 000", 10, 288000,
	MAPSOURCE_OSM_TAH, "1 : 144 000", 11, 144000,
	MAPSOURCE_OSM_TAH, "1 : 72 000", 12, 72000,
	MAPSOURCE_OSM_TAH, "1 : 36 000", 13, 36000,
	MAPSOURCE_OSM_TAH, "1 : 18 000", 14, 18000,
	MAPSOURCE_OSM_TAH, "1 : 9 000", 15, 9000,
	MAPSOURCE_OSM_TAH, "1 : 4 500", 16, 4500,
	MAPSOURCE_OSM_TAH, "1 : 2 250", 17, 2250,
	MAPSOURCE_N_ITEMS, "", 0, 0
};


/* *****************************************************************************
 * set coordinates in text entries (called when map is clicked)
 */
void
mapdl_set_coords (gchar *lat, gchar *lon)
{
	gtk_entry_set_text (GTK_ENTRY (lat_entry), lat);
	gtk_entry_set_text (GTK_ENTRY (lon_entry), lon);
}


/* *****************************************************************************
 * callback to set paramaters for map to download
 */
gint
mapdl_setparm_cb (GtkWidget *widget, gint data)
{
	G_CONST_RETURN gchar *s;
	GtkTreeIter t_iter;

	if (!gui_status.dl_window)
		return TRUE;

	s = gtk_entry_get_text (GTK_ENTRY (lat_entry));
	coordinate_string2gdouble(s, &mapdl_lat);
	if (mydebug > 3)
	    g_print ("new map lat: %s\n",s);

	s = gtk_entry_get_text (GTK_ENTRY (lon_entry));
	coordinate_string2gdouble(s, &mapdl_lon);
	if (mydebug > 3)
	    g_print ("new map lon: %s\n", s);

	if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (scale_combobox), &t_iter))
	{
		gtk_tree_model_get (GTK_TREE_MODEL (scales_list), &t_iter,
			2, &mapdl_zoom,
			3, &mapdl_scale, -1);
		if (mydebug > 3)
			g_print ("new map scale/zoom level: %d / %d\n",
				mapdl_scale, mapdl_zoom);
		local_config.mapsource_scale = gtk_combo_box_get_active (GTK_COMBO_BOX (scale_combobox));
		current.needtosave = TRUE;
	}

	return TRUE;
}


/* *****************************************************************************
 * callback to set possible scales for chosen map source
 */
static gint
mapdl_setsource_cb (GtkComboBox *combo_box, gpointer data)
{
	GtkTreeIter t_iter;

	if (gtk_combo_box_get_active_iter (combo_box, &t_iter))
	{
		gtk_tree_model_get (GTK_TREE_MODEL (types_list), &t_iter,
			0, &local_config.mapsource_type, -1);
		gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (scales_list));
		gtk_combo_box_set_active (GTK_COMBO_BOX (scale_combobox), local_config.mapsource_scale);
		current.needtosave = TRUE;
		if (mydebug > 3)
			g_print ("new map source: %d\n", local_config.mapsource_type);
	}

	return TRUE;
}


/* *****************************************************************************
 * build the url for landsat
 */
void
mapdl_geturl_landsat (void)
{
	gdouble t_lat1, t_lat2, t_lon1, t_lon2;
	gdouble meters_per_pixel, dist_to_edge_m, dist_to_edge_deg;
	gchar wms_url[512];

	/* output is a planimetric "map_" projection (UTM-like, x_scale=y_scale) */
	meters_per_pixel = mapdl_scale / PIXELFACT;

	/* lat */
	dist_to_edge_m = meters_per_pixel * MAPHEIGHT/2;
	dist_to_edge_deg = dist_to_edge_m / (1852.0*60);
	  /* 1852m/naut mile (arc-minute of LAT) */
	t_lat1 = mapdl_lat - dist_to_edge_deg;
	t_lat2 = mapdl_lat + dist_to_edge_deg;

	/* lon */
	dist_to_edge_m = meters_per_pixel * MAPWIDTH/2;
	dist_to_edge_deg = dist_to_edge_m / (1852.0*60*cos(DEG2RAD(mapdl_lat)));
	  /* 1852m/naut mile (arc-minute of LAT), lon:lat ratio = cos(lat) */
	t_lon1 = mapdl_lon - dist_to_edge_deg;
	t_lon2 = mapdl_lon + dist_to_edge_deg;

	if ( t_lat1 < -90 )  t_lat1 = -90;
	if ( t_lat2 > 90 )   t_lat2 = 90;
	if ( t_lon1 < -180 ) t_lon1 += 360;
	if ( t_lon2 > 180 )  t_lon2 -= 360;

	/* DEBUG
	printf("-> mapdl_scale=%d  mapdl_zoom=%d\n", mapdl_scale, mapdl_zoom);
	printf("-> mapdl_lat=%f  mapdl_lon=%f\n", mapdl_lat, mapdl_lon);
	printf("-> t_lat1=%f  t_lon1=%f\n", t_lat1, t_lon1);
	printf("-> t_lat2=%f  t_lon2=%f\n", t_lat2, t_lon2);
	*/

	strcpy(wms_url, "http://onearth.jpl.nasa.gov/wms.cgi");

	g_snprintf (mapdl_url, sizeof (mapdl_url),
		"%s?request=GetMap"
		"&width=%d&height=%d"
		"&layers=global_mosaic&styles="
		"&srs=EPSG:4326"
		"&format=image/jpeg"
		"&bbox=%.5f,%.5f,%.5f,%.5f",
		wms_url, MAPWIDTH, MAPHEIGHT, t_lon1, t_lat1, t_lon2, t_lat2);
}


/* *****************************************************************************
 * build the url for openstreetmap_tah
 */
void
mapdl_geturl_osm_tah (void)
{
	g_snprintf (mapdl_url, sizeof (mapdl_url),
		"http://server.tah.openstreetmap.org/MapOf/"
		"?lat=%.5f&long=%.5f"
		"&z=%d&w=1280&h=1024&format=png",
		mapdl_lat, mapdl_lon, mapdl_zoom);
}


/* *****************************************************************************
 * do the actual download work
 */
void
mapdl_download (void)
{
	FILE *map_file;
	struct stat file_stat;

	mapdl_active = TRUE;

	if (g_file_test (mapdl_file_w_path, G_FILE_TEST_IS_REGULAR))
		if (popup_yes_no (NULL,
		   _("The map file already exists, overwrite?"))
		   == GTK_RESPONSE_NO)
			return;

	map_file = fopen (mapdl_file_w_path, "w");
	curl_easy_setopt (curl_handle, CURLOPT_URL, mapdl_url);
	curl_easy_setopt (curl_handle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt (curl_handle, CURLOPT_WRITEDATA, map_file);

	set_cursor_style (CURSOR_WATCH);
	
	if (curl_easy_perform (curl_handle))
	{
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (mapdl_progress), 0.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (mapdl_progress),
			_("Aborted."));
	}
	else
	{
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (mapdl_progress), 1.0);
		gtk_progress_bar_set_text (GTK_PROGRESS_BAR (mapdl_progress),
			_("Download complete."));
	}
	fclose (map_file);

	/* add new map to map_koords.txt */
	g_stat (mapdl_file_w_path, &file_stat);
	if (file_stat.st_size > 0)
	{
		loadmapconfig ();
		maps = g_renew (mapsstruct, maps, (nrmaps + 2));
		g_strlcpy ((maps + nrmaps)->filename,
			&mapdl_file_w_path[strlen (local_config.dir_maps)], 200);
		(maps + nrmaps)->map_dir = add_map_dir (&mapdl_file_w_path[strlen (local_config.dir_maps)]);
		(maps + nrmaps)->hasbbox = FALSE;
		(maps + nrmaps)->lat = mapdl_lat;
		(maps + nrmaps)->lon = mapdl_lon;
		(maps + nrmaps)->scale = mapdl_scale;
		nrmaps++;
		savemapconfig ();
	}
	else
		g_remove (mapdl_file_w_path);

	set_cursor_style (CURSOR_DEFAULT);
	mapdl_active = FALSE;
}


/* *****************************************************************************
 * update the progress bar while downloading
 */
gint mapdl_progress_cb (gpointer clientp, gdouble dltotal, gdouble dlnow, gdouble ultotal, gdouble ulnow)
{
	gchar t_buf[24];
	if (mapdl_abort)
		return 1;

	g_snprintf (t_buf, sizeof (t_buf), "Downloaded %d kBytes", (gint) dlnow/1024);
	/* as the current servers don't send a file size in the header, we
	 * pulse the progress bar instead of filling it */
	//gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (mapdl_progress), dlnow/dltotal);
	gtk_progress_bar_pulse (GTK_PROGRESS_BAR (mapdl_progress));
	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (mapdl_progress), t_buf);
	while (gtk_events_pending ())
		gtk_main_iteration ();

	return 0;
}


/* *****************************************************************************
 * start the map download
 */
gint mapdl_start_cb (GtkWidget *widget, gpointer data)
{
	gchar file_path[512];
	gchar path[40];
	gchar type[4];

	if (mapdl_active)
		return TRUE;

	/* preset filename and build download url */
	switch (local_config.mapsource_type)
	{
		case MAPSOURCE_LANDSAT:
			g_strlcpy (path, "landsat", sizeof (path));
			g_strlcpy (type, "jpg", sizeof (type));
			mapdl_geturl_landsat ();
			break;
		case MAPSOURCE_OSM_TAH:
			g_strlcpy (path, "openstreetmap_tah", sizeof (path));
			g_strlcpy (type, "png", sizeof (type));
			mapdl_geturl_osm_tah ();
			break;
		default:
			return TRUE;
	}

	if (mydebug > 20)
		g_print ("  download url:\n%s\n", mapdl_url);

	/* set file path and create directory if necessary */
	
	
	g_snprintf (file_path, sizeof (file_path), "%s%s/%d/%.0f/%.0f/",
		local_config.dir_maps, path, mapdl_scale, mapdl_lat, mapdl_lon);
	if(!g_file_test (file_path, G_FILE_TEST_IS_DIR))
	{
		if (g_mkdir_with_parents (file_path, 0700))
			g_print ("Error creating %s\n",file_path);
		else if (mydebug > 10)
			printf("created directory %s\n",file_path);
	}

	/* complete filename */
	g_snprintf (mapdl_file_w_path, sizeof (mapdl_file_w_path), "%smap_%d_%5.3f_%5.3f.%s",
		file_path, mapdl_scale, mapdl_lat, mapdl_lon, type);

	if (mydebug > 10)
		g_print ("  filename: %s\n", mapdl_file_w_path);

	gtk_progress_bar_set_text (GTK_PROGRESS_BAR (mapdl_progress),
		_("Loading Map..."));
	mapdl_abort = FALSE;
	mapdl_download ();

	return TRUE;
}


/* *****************************************************************************
 * set the text, that is shown in the comboboxes
 */
static gboolean
mapdl_set_combo_filter_cb (GtkTreeModel *model, GtkTreeIter *iter, gpointer scales)
{
	gint t_type, t_flag;

	if (scales)
	{
		gtk_tree_model_get (model, iter,
			0, &t_type,
			2, &t_flag, -1);
		if (t_flag != -1  && t_type == local_config.mapsource_type)
			return TRUE;
	}
	else
	{
		gtk_tree_model_get (model, iter,
			2, &t_flag, -1);
		if (t_flag == -1)
			return TRUE;
	}

	return FALSE;
}


/* *****************************************************************************
 * close map download window
 */
gint
mapdl_close_cb (GtkWidget *widget, gpointer data)
{
	if (mapdl_active)
	{
		mapdl_abort = TRUE;
		return TRUE;
	}

	gui_status.dl_window = mapdl_active = FALSE;
	gtk_widget_destroy (widget);
	expose_mini_cb (NULL, 0);
	set_cursor_style (CURSOR_DEFAULT);

	return TRUE;
}


/* *****************************************************************************
 * create map download dialog
 */
gint
map_download_cb (GtkWidget *widget, gpointer data)
{
	GtkWidget *mapdl_dialog, *mapdl_box, *mapdl_table;
	GtkWidget *dl_bt, *close_bt;
	GtkWidget *lat_lb, *lon_lb, *source_lb;
	GtkWidget *help_lb, *scale_lb;
	GtkWidget *cover_lb, *cover_entry;
	GtkWidget *source_combobox;
	GtkListStore *source_list;
	GtkCellRenderer *renderer_source, *renderer_scale;
	GtkTreeIter t_iter;

	gchar t_buf[300];
	gint i;

	curl_handle = curl_easy_init();
	curl_easy_setopt (curl_handle, CURLOPT_PROGRESSFUNCTION, mapdl_progress_cb);

	set_cursor_style (CURSOR_CROSS);

	source_list = gtk_list_store_new (4,
		G_TYPE_INT,		/* type */
		G_TYPE_STRING,		/* scale */
		G_TYPE_INT,		/* zoom */
		G_TYPE_INT		/* scale_int */
		);

	for (i = 0; mapsource[i].type != MAPSOURCE_N_ITEMS; i++)
	{
		gtk_list_store_append (source_list, &t_iter);
		gtk_list_store_set (source_list, &t_iter,
			0, mapsource[i].type,
			1, mapsource[i].scale,
			2, mapsource[i].zoom,
			3, mapsource[i].scale_int,
			-1);
	}
	types_list = gtk_tree_model_filter_new (GTK_TREE_MODEL (source_list), NULL);
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (types_list),
		mapdl_set_combo_filter_cb, (gpointer) 0, NULL);
	scales_list = gtk_tree_model_filter_new (GTK_TREE_MODEL (source_list), NULL);
	gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (scales_list),
		mapdl_set_combo_filter_cb, (gpointer) 1, NULL);

	mapdl_dialog = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (mapdl_dialog), _("Select coordinates and scale"));
	gtk_container_set_border_width (GTK_CONTAINER (mapdl_dialog), 5);
	gtk_window_set_position (GTK_WINDOW (mapdl_dialog), GTK_WIN_POS_CENTER);
	mapdl_box = gtk_vbox_new (TRUE, 2);

	dl_bt = gtk_button_new_with_label (_("Download map"));
	GTK_WIDGET_SET_FLAGS (dl_bt, GTK_CAN_DEFAULT);
	gtk_window_set_default (GTK_WINDOW (mapdl_dialog), dl_bt);
	g_signal_connect (dl_bt, "clicked", G_CALLBACK (mapdl_start_cb), 0);

	close_bt = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	g_signal_connect_swapped (close_bt, "clicked", G_CALLBACK (mapdl_close_cb), mapdl_dialog);
	g_signal_connect_swapped (mapdl_dialog, "delete_event", G_CALLBACK (mapdl_close_cb), mapdl_dialog);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mapdl_dialog)->action_area), dl_bt, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mapdl_dialog)->action_area), close_bt, TRUE, TRUE, 2);

	source_lb = gtk_label_new (_("Map Source"));
	source_combobox = gtk_combo_box_new_with_model (GTK_TREE_MODEL (types_list));
	gtk_combo_box_set_active (GTK_COMBO_BOX (source_combobox), local_config.mapsource_type);
	g_signal_connect (source_combobox, "changed", G_CALLBACK (mapdl_setsource_cb), NULL);
	renderer_source = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (source_combobox), renderer_source, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (source_combobox),
		renderer_source, "text", 1, NULL);

	cover_lb = gtk_label_new (_("Map covers"));
	cover_entry = gtk_entry_new ();
	gtk_editable_set_editable (GTK_EDITABLE (cover_entry), FALSE);
	g_signal_connect (cover_entry, "changed", G_CALLBACK (mapdl_setparm_cb), NULL);

	lat_lb = gtk_label_new (_("Latitude"));
	lat_entry = gtk_entry_new ();
	coordinate2gchar(t_buf, sizeof(t_buf), coords.current_lat, TRUE, local_config.coordmode);
	gtk_entry_set_text (GTK_ENTRY (lat_entry), t_buf);
	g_signal_connect (lat_entry, "changed", G_CALLBACK (mapdl_setparm_cb), NULL);

	lon_lb = gtk_label_new (_("Longitude"));
	lon_entry = gtk_entry_new ();
	coordinate2gchar(t_buf, sizeof(t_buf), coords.current_lon, FALSE, local_config.coordmode);
	gtk_entry_set_text (GTK_ENTRY (lon_entry), t_buf);
	g_signal_connect (lon_entry, "changed", G_CALLBACK (mapdl_setparm_cb), NULL);

	scale_combobox = gtk_combo_box_new_with_model (GTK_TREE_MODEL (scales_list));
	g_signal_connect (scale_combobox, "changed", G_CALLBACK (mapdl_setparm_cb), NULL);
	renderer_scale = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (scale_combobox), renderer_scale, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (scale_combobox),
		renderer_scale, "text", 1, NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (scale_combobox), 0);

	g_snprintf (t_buf, sizeof (t_buf), "%s",
		_("You can also select the position\nwith a mouse click on the map."));
	help_lb = gtk_label_new (t_buf);
	scale_lb = gtk_label_new (_("Scale"));

	mapdl_progress = gtk_progress_bar_new ();
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (mapdl_progress), 0.0);
	gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR (mapdl_progress), 0.01);

	mapdl_table = gtk_table_new (7, 2, FALSE);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), source_lb, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), source_combobox, 1, 2, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), lat_lb, 0, 1, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), lat_entry, 1, 2, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), lon_lb, 0, 1, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), lon_entry, 1, 2, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), cover_lb, 0, 1, 3, 4);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), cover_entry, 1, 2, 3, 4);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), scale_lb, 0, 1, 4, 5);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), scale_combobox, 1, 2, 4, 5);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), help_lb, 0, 2, 5, 6);
	gtk_table_attach_defaults (GTK_TABLE (mapdl_table), mapdl_progress, 0, 2, 6, 7);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mapdl_dialog)->vbox), mapdl_table, TRUE, TRUE, 2);

	gtk_widget_show_all (mapdl_dialog);
	gui_status.dl_window = TRUE;
	mapdl_setparm_cb (NULL, 0);

	return TRUE;
}


/* *****************************************************************************
 * init curl for map download
 */
gint
mapdl_init (void)
{
	if (curl_global_init (CURL_GLOBAL_ALL))
	{
		g_print (_("Initialization of CURL for map download failed!\n"));
		return FALSE;
	}
	return TRUE;
}


/* *****************************************************************************
 * cleanup curl for map download
 */
gint
mapdl_cleanup (void)
{
	curl_global_cleanup ();
	return TRUE;
}
