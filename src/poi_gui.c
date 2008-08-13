/*
**********************************************************************

Copyright (c) 2001-2007 Fritz Ganter <ganter@ganter.at>

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

**********************************************************************
*/

/*
 * poi_gui.c
 *
 * This module holds all the gui stuff for the poi-related windows
 */


#define _GNU_SOURCE
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <gpsdrive.h>
#include <icons.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#include "gettext.h"
#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "gui.h"
#include "import_map.h"
#include "poi.h"
#include "poi_gui.h"
#include "routes.h"
#include "main_gui.h"
#include "gpx.h"

/*  Defines for gettext I18n */
#include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


struct pattern
{
	GtkEntry *text, *distance;
	gchar *poitype_name;
	gint posflag;	/* 0: search at current position, 1: search at destination */
	guint result_count;
} criteria;

static GtkWidget *statusbar_poilist;
static gint statusbar_id;


extern routestatus_struct route;
extern coordinate_struct coords;

extern gint mydebug;
extern gint debug;

extern GtkTreeStore *poi_types_tree;
extern GtkListStore *poi_result_tree;
extern GtkListStore *route_list_tree;

extern gdouble wp_saved_target_lat;
extern gdouble wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat;
extern gdouble wp_saved_posmode_lon;

extern color_struct colors;
extern currentstatus_struct current;

extern GtkWidget *find_poi_bt;
extern GtkWidget *routing_bt;
extern GtkWidget *posbt;
extern GtkWidget *settings_window;
extern GtkWidget *menuitem_saveroute;
extern GtkWidget *routeinfo_evbox;

extern GdkPixbuf *targetmarker_img;

extern poi_struct poi_buf;

GtkWidget *poi_types_window;

/* poi lookup window */
GtkWidget *poi_lookup_window;
GtkWidget *button_addtoroute;
static GtkWidget *button_delete;
static GtkWidget *button_target;
static GtkWidget *button_refpoint;
static GtkWidget *button_jumpto;

/* route window */
GtkWidget *route_window;
GtkWidget *button_startroute;
GtkWidget *button_remove, *button_routesave;



/* *****************************************************************************
 * CALLBACKS
 */
gint
show_poi_lookup_cb (GtkWidget *button, gint mode)
{
	/* mode: 0 = standard lookup, 1 = set reference point for map import */
	if (mode == 1)
	{
		gtk_window_set_modal (GTK_WINDOW (poi_lookup_window), TRUE);
		gtk_window_set_title (GTK_WINDOW (poi_lookup_window), _("Select Reference Point"));
	}
	else
	{
		gtk_window_set_modal (GTK_WINDOW (poi_lookup_window), FALSE);
		gtk_window_set_title (GTK_WINDOW (poi_lookup_window), _("Lookup Point of Interest"));
		gtk_widget_set_sensitive (find_poi_bt, FALSE);
		gtk_widget_set_sensitive (posbt, FALSE);
	}

	/* save old target/posmode for cancel event */
	wp_saved_target_lat = coords.target_lat;
	wp_saved_target_lon = coords.target_lon;
	if (gui_status.posmode)
	{
		wp_saved_posmode_lat = coords.posmode_lat;
		wp_saved_posmode_lon = coords.posmode_lon;
	}

	gtk_widget_show_all (poi_lookup_window);

	if (mode == 1)
	{
		gtk_widget_hide_all (button_addtoroute);
		gtk_widget_hide_all (button_target);
		gtk_widget_hide_all (button_delete);
		gtk_widget_hide_all (button_jumpto);
	}
	else
	{
		gtk_widget_hide_all (button_refpoint);
	}

	gtk_window_present (GTK_WINDOW (poi_lookup_window));

	return TRUE;
}

static void
evaluate_poi_search_cb (GtkWidget *button, struct pattern *entries)
{
	gchar search_status[80];
	
	if (mydebug>5)
	{
		fprintf (stdout, "\npoi search criteria:\n  text: %s\n  distance: %s\t  posflag: %d\n",
			gtk_entry_get_text (entries->text) ,
			gtk_entry_get_text (entries->distance),
			entries->posflag);
	}
		
	entries->result_count = poi_get_results
		(gtk_entry_get_text (entries->text),
		gtk_entry_get_text (entries->distance),
		entries->posflag, entries->poitype_name);

	gtk_statusbar_pop (GTK_STATUSBAR (statusbar_poilist), statusbar_id);
	if (entries->result_count == local_config.poi_results_max)
	{
		g_snprintf (search_status, sizeof (search_status),
		_(" Found %d matching entries (Limit reached)."),
		entries->result_count);
	}
	else
	{
		g_snprintf (search_status, sizeof (search_status),
		_(" Found %d matching entries."),
		entries->result_count);
	}
	gtk_statusbar_push (GTK_STATUSBAR (statusbar_poilist), statusbar_id, search_status);
}

void
toggle_window_poi_info_cb (GtkToggleButton *togglebutton, gpointer user_data)
{

}


static void
searchdistancemode_cb (GtkToggleButton *button, gpointer user_data)
{
	if (gtk_toggle_button_get_active(button))
	{
		criteria.posflag = 0; /* search near current position */
	}
	else
	{
		criteria.posflag = 1; /* search near destination */
	}
}


static void
close_poi_lookup_window_cb (GtkWidget *window)
{
	/* restore saved values */
	if (!route.active)
	{
		coords.target_lat = wp_saved_target_lat;
		coords.target_lon = wp_saved_target_lon;
	}
	if (gui_status.posmode)
	{
		coords.posmode_lat = wp_saved_posmode_lat;
		coords.posmode_lon = wp_saved_posmode_lon;
	}

	gtk_widget_hide_all (poi_lookup_window);
	gtk_widget_set_sensitive (find_poi_bt, TRUE);
	gtk_widget_set_sensitive (posbt, TRUE);
}


static void
select_jump_poi_cb (GtkWidget *window)
{
	gtk_widget_hide_all (poi_lookup_window);

	coords.current_lat = coords.target_lat;
	coords.current_lon = coords.target_lon;
	
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON (posbt),  TRUE);

	gtk_widget_set_sensitive (find_poi_bt, TRUE);
	gtk_widget_set_sensitive (posbt, TRUE);
}


static void
select_target_poi_cb (GtkWidget *window)
{
	gtk_widget_hide_all (poi_lookup_window);

	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON (posbt), FALSE);

	gtk_widget_set_sensitive (find_poi_bt, TRUE);
	gtk_widget_set_sensitive (posbt, TRUE);
}


static void
delete_poi_cb (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gint selected_poi_id = 0;
	gint selected_poi_source = 0;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter,
			RESULT_ID, &selected_poi_id,
			RESULT_SOURCE, &selected_poi_source,
			-1);
		
		/* entries fetched from the mapnik database can't be deleted */
		if (selected_poi_source == 10)
			return;

		if (popup_yes_no(GTK_WINDOW (poi_lookup_window), NULL) == GTK_RESPONSE_YES)
		{
			if (mydebug>20)
				fprintf (stderr, "deleting poi with id: %d\n", selected_poi_id);
			db_poi_delete (selected_poi_id);
			gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
			gtk_widget_set_sensitive (button_delete, FALSE);
		}
	}
}


static void
select_poi_cb (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gint t_src = 0;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		if (current.importactive)
		{
			gchar *t_buf;
			gtk_tree_model_get (model, &iter,
						RESULT_LAT, &poi_buf.lat,
						RESULT_LON, &poi_buf.lon,
						RESULT_NAME, &t_buf,
						-1);
			g_strlcpy (poi_buf.name, t_buf, sizeof (poi_buf.name));
			g_free (t_buf);
			gtk_widget_set_sensitive (button_refpoint, TRUE);
			return;
		}
		if (route.edit)
		{
			add_poi_to_route (model, iter);
			if (route.items)
			{
				gtk_widget_set_sensitive
					(button_startroute, TRUE);
				gtk_widget_set_sensitive
					(button_routesave, TRUE);
			}
		}
		else
		{
			if (!route.active)
			{
				gtk_tree_model_get (model, &iter,
					RESULT_LAT, &coords.target_lat,
					RESULT_LON, &coords.target_lon,
					RESULT_SOURCE, &t_src,
					-1);
				if (mydebug>50)
					fprintf (stdout,
						" new target -> %f / %f\n",
						coords.target_lat,
						coords.target_lon);
			}
		}
		
		if (t_src == 10)
			gtk_widget_set_sensitive (button_delete, FALSE);
		else
			gtk_widget_set_sensitive (button_delete, TRUE);

		/* if posmode enabled set posmode_lat/lon */
		if (gui_status.posmode)
		{
			gtk_tree_model_get (model, &iter,
						RESULT_LAT, &coords.posmode_lat,
						RESULT_LON, &coords.posmode_lon,
						-1);
		}
	}
}


static gint
select_poitype_cb (GtkComboBox *combo_box, gpointer data)
{
	GtkTreeIter iter;

	if (gtk_combo_box_get_active_iter (combo_box, &iter))
	{
		gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &iter,
			POITYPE_NAME, &criteria.poitype_name, -1);
	}


	if (mydebug>50)
	{
		fprintf (stderr, " selected poi-type -> %s\n", criteria.poitype_name);
	}
	return FALSE;
}


static void
close_route_window_cb ()
{
	gtk_widget_destroy (route_window);
	route.edit = FALSE;
	
	if (GTK_IS_WIDGET (button_addtoroute))
		gtk_widget_set_sensitive (button_addtoroute, TRUE);
	gtk_widget_set_sensitive (routing_bt, TRUE);
}


static void
route_startstop_cb ()
{
	route.active = !route.active;
	
	if (route.active)
	{
		route.edit = FALSE;
		route_settarget (-1);
		gtk_widget_show_all (routeinfo_evbox);
	}
	else
	{
		// TODO: stop routing
		gtk_widget_hide_all (routeinfo_evbox);
	}
	close_route_window_cb ();	
}


static void
route_cancel_cb ()
{
	gtk_list_store_clear (route_list_tree);
	db_cleanup_route ();
	close_route_window_cb ();
	route.items = 0;
	route.distance = 0.0;
	gtk_widget_set_sensitive (menuitem_saveroute, FALSE);
}


static void
select_routepoint_cb ()
{
	gtk_widget_set_sensitive (button_remove, TRUE);
}


static void
remove_routepoint_cb (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
	}
	
	/* cancel route, if no elements are left */
	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL
		(route_list_tree), &iter) == FALSE)
	{
		route_cancel_cb ();
	}
}


static void
poilist_highlight_cb
	(GtkTreeViewColumn *tree_column, GtkCellRenderer *cell,
	GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
	gint t_src;

	gtk_tree_model_get (tree_model, iter, RESULT_SOURCE, &t_src, -1);

	/* show data coming from friendsd in red/yellow */
	if (t_src == 7)
	{	
		g_object_set (G_OBJECT (cell),
			"foreground-gdk", &colors.red,
			"background-gdk", &colors.yellow,
			"style", PANGO_STYLE_NORMAL,
			"weight", PANGO_WEIGHT_NORMAL,
			NULL);
	}
	/* show data coming from kismet in lightgrey/darkgrey */
	else if (t_src == 9)
	{	
		g_object_set (G_OBJECT (cell),
			"foreground-gdk", &colors.lightgrey,
			"background-gdk", &colors.darkgrey,
			"style", PANGO_STYLE_NORMAL,
			"weight", PANGO_WEIGHT_NORMAL,
			NULL);
	}
	/* show data coming from mapnik/postgis in normal colors */
	else if (t_src == 10)
	{	
		g_object_set (G_OBJECT (cell),
			"foreground-gdk", NULL,
			"background-gdk", NULL,
			"style", PANGO_STYLE_NORMAL,
			"weight", PANGO_WEIGHT_NORMAL,
			NULL);
	}
	else
	{
	/* show user data in normal colors, but in bold/italic font */
		g_object_set (G_OBJECT (cell),
			"foreground-gdk", NULL,
			"background-gdk", NULL,
			"style", PANGO_STYLE_ITALIC,
			"weight", PANGO_WEIGHT_BOLD,
			NULL);
	}
}


/* *****************************************************************************
 * Window: POI-Info
 */
void create_poi_info_window (void)
{
  GtkWidget *poi_info_window;
  GtkWidget *vbox_poidata;
  GtkWidget *frame_poi_basicdata;
  GtkWidget *alignment_basic;
  GtkWidget *table_basic;
  GtkWidget *label_name;
  GtkWidget *label_comment;
  GtkWidget *checkbutton_private;
  GtkWidget *entry_name;
  GtkWidget *entry_comment;
  GtkWidget *entry_lat;
  GtkWidget *entry_lon;
  GtkWidget *entry_alt;
  GtkWidget *label_alt;
  GtkWidget *label_lon;
  GtkWidget *label_lat;
  GtkWidget *hseparator_basic;
  GtkWidget *label_type;
  GtkWidget *combobox_type;
  GtkWidget *entry_poitypeid;
  GtkWidget *label_basic;
  GtkWidget *expander_extra;
  GtkWidget *frame_extra;
  GtkWidget *alignment_extra;
  GtkWidget *scrolledwindow_extra;
  GtkWidget *treeview_extra;
  GtkWidget *label_extradata;
  GtkWidget *hbox_status;
  GtkWidget *statusbar_poiinfo;
  GtkWidget *button_save;
  GtkWidget *button_close;

  poi_info_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (poi_info_window), _("POI-Info"));
  gtk_window_set_icon_name (GTK_WINDOW (poi_info_window), "gtk-info");
  //gtk_window_set_decorated (GTK_WINDOW (poi_info_window), FALSE);
  gtk_window_set_focus_on_map (GTK_WINDOW (poi_info_window), FALSE);

  vbox_poidata = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_poidata);
  gtk_container_add (GTK_CONTAINER (poi_info_window), vbox_poidata);

  frame_poi_basicdata = gtk_frame_new (NULL);
  gtk_widget_show (frame_poi_basicdata);
  gtk_box_pack_start (GTK_BOX (vbox_poidata), frame_poi_basicdata, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_poi_basicdata), 5);

  alignment_basic = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment_basic);
  gtk_container_add (GTK_CONTAINER (frame_poi_basicdata), alignment_basic);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_basic), 5, 5, 5, 5);

  table_basic = gtk_table_new (6, 6, FALSE);
  gtk_widget_show (table_basic);
  gtk_container_add (GTK_CONTAINER (alignment_basic), table_basic);

  label_name = gtk_label_new (_("Name"));
  gtk_widget_show (label_name);
  gtk_table_attach (GTK_TABLE (table_basic), label_name, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_name), 0, 0.5);

  label_comment = gtk_label_new (_("Comment"));
  gtk_widget_show (label_comment);
  gtk_table_attach (GTK_TABLE (table_basic), label_comment, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_comment), 0, 0.5);

  checkbutton_private = gtk_check_button_new_with_mnemonic (_("private"));
  gtk_widget_show (checkbutton_private);
  gtk_table_attach (GTK_TABLE (table_basic), checkbutton_private, 5, 6, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  entry_name = gtk_entry_new ();
  gtk_widget_show (entry_name);
  gtk_table_attach (GTK_TABLE (table_basic), entry_name, 1, 5, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_max_length (GTK_ENTRY (entry_name), 80);

  entry_comment = gtk_entry_new ();
  gtk_widget_show (entry_comment);
  gtk_table_attach (GTK_TABLE (table_basic), entry_comment, 1, 6, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_max_length (GTK_ENTRY (entry_comment), 255);

  entry_lat = gtk_entry_new ();
  gtk_widget_show (entry_lat);
  gtk_table_attach (GTK_TABLE (table_basic), entry_lat, 0, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_lat), 12);

  entry_lon = gtk_entry_new ();
  gtk_widget_show (entry_lon);
  gtk_table_attach (GTK_TABLE (table_basic), entry_lon, 2, 4, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_lon), 12);

  entry_alt = gtk_entry_new ();
  gtk_widget_show (entry_alt);
  gtk_table_attach (GTK_TABLE (table_basic), entry_alt, 4, 6, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_alt), 10);

  label_alt = gtk_label_new (_("Altitude"));
  gtk_widget_show (label_alt);
  gtk_table_attach (GTK_TABLE (table_basic), label_alt, 4, 5, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_alt), 0, 0.5);

  label_lon = gtk_label_new (_("Longitude"));
  gtk_widget_show (label_lon);
  gtk_table_attach (GTK_TABLE (table_basic), label_lon, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_lon), 0, 0.5);

  label_lat = gtk_label_new (_("Latitude"));
  gtk_widget_show (label_lat);
  gtk_table_attach (GTK_TABLE (table_basic), label_lat, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_lat), 0, 0.5);

  hseparator_basic = gtk_hseparator_new ();
  gtk_widget_show (hseparator_basic);
  gtk_table_attach (GTK_TABLE (table_basic), hseparator_basic, 0, 6, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 5);

  label_type = gtk_label_new (_("Type"));
  gtk_widget_show (label_type);
  gtk_table_attach (GTK_TABLE (table_basic), label_type, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_type), 0, 0.5);

  combobox_type = gtk_combo_box_new_text ();
  gtk_widget_show (combobox_type);
  gtk_table_attach (GTK_TABLE (table_basic), combobox_type, 1, 5, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  entry_poitypeid = gtk_entry_new ();
  gtk_widget_show (entry_poitypeid);
  gtk_table_attach (GTK_TABLE (table_basic), entry_poitypeid, 5, 6, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_max_length (GTK_ENTRY (entry_poitypeid), 5);
  gtk_editable_set_editable (GTK_EDITABLE (entry_poitypeid), FALSE);
  gtk_entry_set_has_frame (GTK_ENTRY (entry_poitypeid), FALSE);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_poitypeid), 5);

  label_basic = gtk_label_new (_("Basic Data"));
  gtk_widget_show (label_basic);
  gtk_frame_set_label_widget (GTK_FRAME (frame_poi_basicdata), label_basic);

  expander_extra = gtk_expander_new (NULL);
  gtk_widget_show (expander_extra);
  gtk_box_pack_start (GTK_BOX (vbox_poidata), expander_extra, TRUE, TRUE, 0);
  gtk_expander_set_expanded (GTK_EXPANDER (expander_extra), TRUE);

  frame_extra = gtk_frame_new (NULL);
  gtk_widget_show (frame_extra);
  gtk_container_add (GTK_CONTAINER (expander_extra), frame_extra);
  gtk_container_set_border_width (GTK_CONTAINER (frame_extra), 5);
  gtk_frame_set_label_align (GTK_FRAME (frame_extra), 0, 0);

  alignment_extra = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment_extra);
  gtk_container_add (GTK_CONTAINER (frame_extra), alignment_extra);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_extra), 0, 0, 12, 0);

  scrolledwindow_extra = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_extra);
  gtk_container_add (GTK_CONTAINER (alignment_extra), scrolledwindow_extra);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_extra), GTK_SHADOW_IN);

  treeview_extra = gtk_tree_view_new ();
  gtk_widget_show (treeview_extra);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_extra), treeview_extra);

  label_extradata = gtk_label_new (_("Extra Data"));
  gtk_widget_show (label_extradata);
  gtk_expander_set_label_widget (GTK_EXPANDER (expander_extra), label_extradata);

  hbox_status = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_status);
  gtk_box_pack_start (GTK_BOX (vbox_poidata), hbox_status, FALSE, TRUE, 0);

  statusbar_poiinfo = gtk_statusbar_new ();
  gtk_widget_show (statusbar_poiinfo);
  gtk_box_pack_start (GTK_BOX (hbox_status), statusbar_poiinfo, TRUE, TRUE, 0);
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar_poiinfo), FALSE);

  button_save = gtk_button_new_from_stock ("gtk-save");
  gtk_widget_show (button_save);
  gtk_box_pack_start (GTK_BOX (hbox_status), button_save, FALSE, FALSE, 0);

  button_close = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (button_close);
  gtk_box_pack_start (GTK_BOX (hbox_status), button_close, FALSE, FALSE, 0);

  gtk_widget_show_all (poi_info_window);
}


/* *****************************************************************************
 * Window: POI-Lookup
 */
void create_window_poi_lookup (void)
{
	GtkWidget *dialog_vbox_poisearch;
	GtkWidget *vbox_searchbox, *expander_poisearch;
	GtkWidget *frame_search_criteria, *alignment_criteria;
	GtkWidget *vbox_criteria, *hbox_text;
	GtkWidget *label_text, *entry_text;
	GtkWidget *button_search, *hbox_distance;
	GtkWidget *label_distance, *entry_distance;
	GtkWidget *label_distfrom, *radiobutton_distcurrent;
	GSList *radiobutton_distance_group = NULL;
	GtkWidget *radiobutton_distcursor;
	GtkWidget *hbox_type, *label_type;
	GtkWidget *label_criteria, *frame_poiresults;
	GtkWidget *alignment_poiresults, *vbox_poiresults;
	GtkWidget *scrolledwindow_poilist, *treeview_poilist;
	GtkCellRenderer *renderer_poilist;
	GtkTreeViewColumn *column_poilist;
	GtkTreeSelection *poilist_select;
	GtkWidget *hbox_poistatus;
	GtkWidget *togglebutton_poiinfo, *alignment_poiinfo, *hbox11;
	GtkWidget *image_poiinfo, *label_poiinfo, *label_results;
	GtkWidget *dialog_action_area_poisearch, *alignment_addtoroute;
	GtkWidget *hbox_addtoroute, *image_addtoroute, *label_addtoroute;
	GtkWidget *alignment_target, *hbox_target, *image_target;
	GtkWidget *alignment_refpoint, *hbox_refpoint, *image_refpoint;
	GtkWidget *label_target, *label_refpoint, *button_close;
	GtkWidget *alignment_jumpto, *hbox_jumpto, *image_jumpto;
	GtkWidget *label_jumpto, *combobox_typetree;
	GtkCellRenderer *renderer_type_name;
	GtkCellRenderer *renderer_type_icon;
	GtkTooltips *tooltips_poilookup;
	gchar text[50];
	
	criteria.posflag = 0;
	criteria.result_count = 0;

	tooltips_poilookup = gtk_tooltips_new();
	gtk_tooltips_set_delay (tooltips_poilookup, TOOLTIP_DELAY);
	if (local_config.showtooltips)
		gtk_tooltips_enable (tooltips_poilookup);
	else
		gtk_tooltips_disable (tooltips_poilookup);
	  
	poi_lookup_window = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (poi_lookup_window), 2);
	gtk_window_set_title (GTK_WINDOW (poi_lookup_window),
		_("Lookup Point of Interest"));
	gtk_window_set_position (GTK_WINDOW (poi_lookup_window),
		GTK_WIN_POS_CENTER);
	gtk_window_set_type_hint (GTK_WINDOW (poi_lookup_window),
		GDK_WINDOW_TYPE_HINT_DIALOG);
	if (local_config.guimode == GUI_PDA)
		gtk_window_set_default_size (GTK_WINDOW (poi_lookup_window),
			gui_status.mapview_x, gui_status.mapview_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (poi_lookup_window),
			-1, 400);
	
	dialog_vbox_poisearch = GTK_DIALOG (poi_lookup_window)->vbox;
	
	vbox_searchbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (dialog_vbox_poisearch),
		vbox_searchbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_searchbox), 2);
	
	expander_poisearch = gtk_expander_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_searchbox),
		expander_poisearch, FALSE, TRUE, 0);
	gtk_expander_set_expanded (GTK_EXPANDER (expander_poisearch), TRUE);

	/* Frame: POI Search Criteria */
  {
	frame_search_criteria = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (expander_poisearch),
		frame_search_criteria);
	gtk_frame_set_label_align (GTK_FRAME (frame_search_criteria), 0, 0);
	
	alignment_criteria = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame_search_criteria),
		alignment_criteria);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_criteria),
		5, 5, 12, 5);
	
	vbox_criteria = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (alignment_criteria), vbox_criteria);
	
	hbox_text = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_criteria), hbox_text, TRUE, TRUE, 0);
	
	label_text = gtk_label_new (_("<i>Search Text:</i>"));
	gtk_box_pack_start (GTK_BOX (hbox_text), label_text, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label_text), TRUE);
	
	entry_text = gtk_entry_new ();
	criteria.text = GTK_ENTRY (entry_text);
	gtk_box_pack_start (GTK_BOX (hbox_text), entry_text, TRUE, TRUE, 5);
	gtk_entry_set_max_length (GTK_ENTRY (entry_text), 255);
	g_signal_connect (entry_text, "activate",
		GTK_SIGNAL_FUNC (evaluate_poi_search_cb), &criteria);
	
	/* Button to activate POI search */
	button_search = gtk_button_new_from_stock ("gtk-find");
	gtk_box_pack_start (GTK_BOX (hbox_text), button_search,
		FALSE, FALSE, 5);
	g_signal_connect (button_search, "clicked",
		GTK_SIGNAL_FUNC (evaluate_poi_search_cb), &criteria);
	
	/* Distance selection */
	hbox_distance = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_criteria),
		hbox_distance, TRUE, TRUE, 0);
	
	label_distance = gtk_label_new (_("<i>max. Distance:</i>"));
	gtk_box_pack_start (GTK_BOX (hbox_distance),
		label_distance, FALSE, FALSE, 10);
	gtk_label_set_use_markup (GTK_LABEL (label_distance), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (label_distance), TRUE);
	
	entry_distance = gtk_entry_new ();
	criteria.distance = GTK_ENTRY (entry_distance);
	gtk_box_pack_start (GTK_BOX (hbox_distance),
		entry_distance, FALSE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (entry_distance), 5);
	g_snprintf (text, sizeof (text), "%0.1f",
		local_config.poi_searchradius);
	gtk_entry_set_text (GTK_ENTRY (entry_distance), text);
	gtk_entry_set_width_chars (GTK_ENTRY (entry_distance), 5);
	g_signal_connect (entry_distance, "activate",
		GTK_SIGNAL_FUNC (evaluate_poi_search_cb), &criteria);
	
	label_distfrom = gtk_label_new (_("km from"));
	gtk_box_pack_start (GTK_BOX (hbox_distance),
		label_distfrom, FALSE, FALSE, 0);
	
	radiobutton_distcurrent = gtk_radio_button_new_with_mnemonic
		(NULL, _("current position"));
	gtk_box_pack_start (GTK_BOX (hbox_distance),
		radiobutton_distcurrent, FALSE, FALSE, 15);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON
		(radiobutton_distcurrent), radiobutton_distance_group);
	radiobutton_distance_group = gtk_radio_button_get_group
		(GTK_RADIO_BUTTON (radiobutton_distcurrent));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
		(radiobutton_distcurrent), TRUE);
	gtk_tooltips_set_tip ( tooltips_poilookup, radiobutton_distcurrent, 
		_("Search near current Position"), NULL);
	g_signal_connect (radiobutton_distcurrent, "toggled",
		GTK_SIGNAL_FUNC (searchdistancemode_cb), NULL);
	
	radiobutton_distcursor = gtk_radio_button_new_with_mnemonic
		(NULL, _("Destination/Cursor"));
	gtk_box_pack_start (GTK_BOX (hbox_distance),
		radiobutton_distcursor, FALSE, FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER
		(radiobutton_distcursor), 5);
	gtk_button_set_focus_on_click (GTK_BUTTON
		(radiobutton_distcursor), FALSE);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON
		(radiobutton_distcursor), radiobutton_distance_group);
	radiobutton_distance_group = gtk_radio_button_get_group
		(GTK_RADIO_BUTTON (radiobutton_distcursor));
	gtk_tooltips_set_tip ( tooltips_poilookup, radiobutton_distcursor, 
		_("Search near selected Destination"), NULL);

	/* POI-Type Selection */
	hbox_type = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_criteria), hbox_type, TRUE, TRUE, 0);

	label_type = gtk_label_new (_("<i>POI-Types:</i>"));
	gtk_box_pack_start (GTK_BOX (hbox_type), label_type, FALSE, FALSE, 10);
	gtk_label_set_use_markup (GTK_LABEL (label_type), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (label_type), TRUE);

	combobox_typetree = gtk_combo_box_new_with_model
		(GTK_TREE_MODEL (poi_types_tree));
	renderer_type_icon = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox_typetree),
		renderer_type_icon, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox_typetree),
		renderer_type_icon, "pixbuf", POITYPE_ICON, NULL);
	renderer_type_name = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox_typetree),
		renderer_type_name, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox_typetree),
		renderer_type_name, "text", POITYPE_TITLE, NULL);

    
    /* TODO: Set active poitype
       
       GtkTreeIter t_iter;
       gtk_combo_box_set_active_iter
       (GTK_COMBO_BOX(combobox_typetree),
       &t_iter);
    */
    
	select_poitype_cb (GTK_COMBO_BOX (combobox_typetree), NULL);
	g_signal_connect (G_OBJECT (combobox_typetree), "changed",
		G_CALLBACK (select_poitype_cb), NULL);
	gtk_box_pack_end (GTK_BOX (hbox_type),
		combobox_typetree, TRUE, TRUE, 5);

	label_criteria = gtk_label_new (_("Search Criteria"));
	gtk_widget_show (label_criteria);
	gtk_expander_set_label_widget (GTK_EXPANDER (expander_poisearch),
		label_criteria);
  }
	
	/* Frame: POI-Results */
  {
	frame_poiresults = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_searchbox),
		frame_poiresults, TRUE, TRUE, 5);
	gtk_frame_set_label_align (GTK_FRAME (frame_poiresults), 0.02, 0.5);

	alignment_poiresults = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame_poiresults),
		alignment_poiresults);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_poiresults),
		2, 2, 2, 2);

	vbox_poiresults = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (alignment_poiresults),
		vbox_poiresults);

	scrolledwindow_poilist = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox_poiresults),
		scrolledwindow_poilist, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
		(scrolledwindow_poilist),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW
		(scrolledwindow_poilist), GTK_SHADOW_IN);

	treeview_poilist = gtk_tree_view_new_with_model
		(GTK_TREE_MODEL (poi_result_tree));

	renderer_poilist = gtk_cell_renderer_pixbuf_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes ("_",
		renderer_poilist, "pixbuf", RESULT_TYPE_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist),
		column_poilist);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
		_("Name"), renderer_poilist,
		"text", RESULT_NAME, NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist, RESULT_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist),
		column_poilist);
	gtk_tree_view_column_set_cell_data_func
		(column_poilist, renderer_poilist, poilist_highlight_cb,
		NULL, NULL);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
		_("Distance"), renderer_poilist,
		"text", RESULT_DISTANCE, NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist,
		RESULT_DIST_NUM);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist),
		column_poilist);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
		_("Type"), renderer_poilist,
		"text", RESULT_TYPE_TITLE, NULL);
	g_object_set (G_OBJECT (renderer_poilist),
		"foreground-gdk", &colors.textback, NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist,
		RESULT_TYPE_TITLE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist),
		column_poilist);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
		_("Comment"), renderer_poilist,
		"text", RESULT_COMMENT, NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist,
		RESULT_COMMENT);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist),
		column_poilist);

	gtk_container_add (GTK_CONTAINER (scrolledwindow_poilist),
		treeview_poilist);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview_poilist), TRUE);

	poilist_select = gtk_tree_view_get_selection (GTK_TREE_VIEW
		(treeview_poilist));
	gtk_tree_selection_set_mode (poilist_select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (poilist_select), "changed",
		G_CALLBACK (select_poi_cb), NULL);

	hbox_poistatus = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_poiresults),
		hbox_poistatus, FALSE, TRUE, 0);

	statusbar_poilist = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (hbox_poistatus),
		statusbar_poilist, TRUE, TRUE, 0);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR
		(statusbar_poilist), FALSE);
	statusbar_id = gtk_statusbar_get_context_id (GTK_STATUSBAR
		(statusbar_poilist), "poilist");
	gtk_statusbar_push (GTK_STATUSBAR (statusbar_poilist), statusbar_id,
		_(" Please enter your search criteria!"));
	
	/* button "POI-Info" */
	togglebutton_poiinfo = gtk_toggle_button_new ();
	gtk_box_pack_start (GTK_BOX (hbox_poistatus),
		togglebutton_poiinfo, FALSE, FALSE, 0);
	alignment_poiinfo = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (togglebutton_poiinfo),
		alignment_poiinfo);
	hbox11 = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_poiinfo), hbox11);
	image_poiinfo = gtk_image_new_from_stock ("gtk-info",
		GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox11), image_poiinfo, FALSE, FALSE, 0);
	label_poiinfo = gtk_label_new_with_mnemonic (_("POI-Info"));
	gtk_box_pack_start (GTK_BOX (hbox11), label_poiinfo, FALSE, FALSE, 0);
	gtk_tooltips_set_tip ( tooltips_poilookup, togglebutton_poiinfo, 
		_("Show detailed Information for selected Point of Interest"),
		NULL);

	// TODO: complete POI-Info functionality, until then button is disabled:
	gtk_widget_set_sensitive (togglebutton_poiinfo, FALSE);

	label_results = gtk_label_new (_("Results"));
	gtk_frame_set_label_widget (GTK_FRAME (frame_poiresults),
		label_results);
	gtk_label_set_use_markup (GTK_LABEL (label_results), TRUE);
  }

	dialog_action_area_poisearch = GTK_DIALOG
		(poi_lookup_window)->action_area;

	/* button "edit route" */
	button_addtoroute = gtk_button_new ();
	GTK_WIDGET_SET_FLAGS (button_addtoroute, GTK_CAN_DEFAULT);
	alignment_addtoroute = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_addtoroute),
		alignment_addtoroute);
	hbox_addtoroute = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_addtoroute),
		hbox_addtoroute);
	image_addtoroute = gtk_image_new_from_stock ("gtk-add",
		GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox_addtoroute),
		image_addtoroute, FALSE, FALSE, 0);
	label_addtoroute = gtk_label_new_with_mnemonic (_("Edit _Route"));
	gtk_box_pack_start (GTK_BOX (hbox_addtoroute),
		label_addtoroute, FALSE, FALSE, 0);
	gtk_tooltips_set_tip ( tooltips_poilookup, button_addtoroute, 
	_("Switch to Add selected entry to Route"), NULL);
	g_signal_connect (button_addtoroute, "clicked",
		GTK_SIGNAL_FUNC (route_window_cb), NULL);  
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (poi_lookup_window)->action_area),
		button_addtoroute, FALSE, FALSE, 0);

	/* button "delete POI" */
	button_delete = gtk_button_new_from_stock ("gtk-delete");
	GTK_WIDGET_SET_FLAGS (button_delete, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip ( tooltips_poilookup, button_delete, 
		_("Delete selected entry"), NULL);
	g_signal_connect_swapped (button_delete, "clicked",
		GTK_SIGNAL_FUNC (delete_poi_cb), poilist_select);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (poi_lookup_window)->action_area),
		button_delete, FALSE, FALSE, 0);

	/* button "Jump to POI" */
	button_jumpto = gtk_button_new ();
	GTK_WIDGET_SET_FLAGS (button_jumpto, GTK_CAN_DEFAULT);
	alignment_jumpto = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_jumpto), alignment_jumpto);
	hbox_jumpto = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_jumpto), hbox_jumpto);
	image_jumpto = gtk_image_new_from_stock ("gtk-jump-to",
		GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox_jumpto),
		image_jumpto, FALSE, FALSE, 0);
	label_jumpto = gtk_label_new (_("Jump to POI"));		
	gtk_tooltips_set_tip ( tooltips_poilookup, button_jumpto, 
		_("Jump to selected entry (and switch to Pos. Mode if not already active)"), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_jumpto),
		label_jumpto, FALSE, FALSE, 0);
	g_signal_connect_swapped (button_jumpto, "clicked",
		GTK_SIGNAL_FUNC (select_jump_poi_cb), NULL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (poi_lookup_window)->action_area),
		button_jumpto, FALSE, FALSE, 0);

	/* button "Select as Destination" */
	button_target = gtk_button_new ();
	GTK_WIDGET_SET_FLAGS (button_target, GTK_CAN_DEFAULT);
	alignment_target = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_target), alignment_target);
	hbox_target = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_target), hbox_target);
	image_target = gtk_image_new_from_pixbuf (targetmarker_img);
	gtk_box_pack_start (GTK_BOX (hbox_target),
		image_target, FALSE, FALSE, 0);
	label_target = gtk_label_new (_("Select Target"));
	gtk_tooltips_set_tip ( tooltips_poilookup, button_target, 
		_("Use selected entry as target destination (and leave Pos. Mode if active)"), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_target),
		label_target, FALSE, FALSE, 0);
	g_signal_connect_swapped (button_target, "clicked",
		GTK_SIGNAL_FUNC (select_target_poi_cb), NULL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (poi_lookup_window)->action_area),
		button_target, FALSE, FALSE, 0);

	/* button "Select as Reference Point" */
	button_refpoint = gtk_button_new ();
	alignment_refpoint = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_refpoint), alignment_refpoint);
	hbox_refpoint = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_refpoint), hbox_refpoint);
	image_refpoint = gtk_image_new_from_stock ("gtk-apply", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox_refpoint),
		image_refpoint, FALSE, FALSE, 0);
	label_refpoint = gtk_label_new (_("Select Point"));
	gtk_tooltips_set_tip ( tooltips_poilookup, button_refpoint, 
		_("Use selected entry as reference point"), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_refpoint),
		label_refpoint, FALSE, FALSE, 0);
	g_signal_connect_swapped (button_refpoint, "clicked",
		G_CALLBACK (select_refpoint_poi_cb), poi_lookup_window);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (poi_lookup_window)->action_area),
		button_refpoint, FALSE, FALSE, 0);
	gtk_widget_set_sensitive (button_refpoint, FALSE);

	/* button "close" */
	button_close = gtk_button_new_from_stock ("gtk-close");
	GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip ( tooltips_poilookup, button_close, 
		_("Close this window"), NULL);
	g_signal_connect_swapped (button_close, "clicked",
		GTK_SIGNAL_FUNC (close_poi_lookup_window_cb),
		poi_lookup_window);
	g_signal_connect (GTK_OBJECT (poi_lookup_window), "delete_event",
		GTK_SIGNAL_FUNC (close_poi_lookup_window_cb), NULL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (poi_lookup_window)->action_area),
		button_close, FALSE, FALSE, 0);

	/* disable delete button until POI is selected from list */
	gtk_widget_set_sensitive (button_delete, FALSE);
	
	/* disable target selection in active routemode */
	if (route.active)
		gtk_widget_set_sensitive (button_target, FALSE);

	gtk_widget_grab_focus (entry_text);
}


/* *****************************************************************************
 * Window: Edit Route
 */
void route_window_cb (GtkWidget *calling_button)
{
	GtkWidget *dialog_vbox_route;
	GtkWidget *vbox_routedetails;
	GtkWidget *frame_routedetails;
	GtkWidget *scrolledwindow_routedetails;
	GtkWidget *alignment_startroute;
	GtkWidget *hbox_startroute;
	GtkWidget *image_startroute;
	GtkWidget *label_startroute;
	GtkWidget *label_routedetails;
	GtkWidget *treeview_routelist;
	GtkCellRenderer *renderer_routelist;
	GtkTreeViewColumn *column_routelist;
	GtkTreeSelection *routelist_select;
	GtkWidget *dialog_action_area_routedetails;
	GtkWidget *button_close;
	GtkWidget *button_cancel;
	GtkWidget *alignment_cancel;
	GtkWidget *hbox_cancel;
	GtkWidget *image_cancel;
	GtkWidget *label_cancel;
	GtkTooltips *tooltips_routewindow;

	gtk_widget_set_sensitive (button_addtoroute, FALSE);
	gtk_widget_set_sensitive (routing_bt, FALSE);

	route.edit = TRUE;

	if (!route.items)
	{
		route.distance = 0.0;
		route.pointer = 0;
	}

	tooltips_routewindow = gtk_tooltips_new();
	gtk_tooltips_set_delay (tooltips_routewindow, TOOLTIP_DELAY);
	if (local_config.showtooltips)
		gtk_tooltips_enable (tooltips_routewindow);
	else
		gtk_tooltips_disable (tooltips_routewindow);

	route_window = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (route_window), 2);
	gtk_window_set_title (GTK_WINDOW (route_window), _("Edit Route"));
	gtk_window_set_position (GTK_WINDOW (route_window),
		GTK_WIN_POS_NONE);
	gtk_window_set_type_hint (GTK_WINDOW (route_window),
		GDK_WINDOW_TYPE_HINT_DIALOG);
	if (local_config.guimode == GUI_PDA)
	{
		gtk_window_set_default_size (GTK_WINDOW (route_window),
			gui_status.mapview_x, gui_status.mapview_y);
	}
	else
		gtk_window_set_default_size (GTK_WINDOW (route_window), -1, 250);
	
	dialog_vbox_route = GTK_DIALOG (route_window)->vbox;
	
	vbox_routedetails = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (dialog_vbox_route),
		vbox_routedetails, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_routedetails), 2);
	
	/* Frame: Route-Details */
  {
	frame_routedetails = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_routedetails),
		frame_routedetails, TRUE, TRUE, 5);
	gtk_frame_set_label_align (GTK_FRAME (frame_routedetails), 0.02, 0.5);

	scrolledwindow_routedetails = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
		(scrolledwindow_routedetails), GTK_POLICY_AUTOMATIC,
		GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW
		(scrolledwindow_routedetails), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame_routedetails),
		scrolledwindow_routedetails);

	treeview_routelist = gtk_tree_view_new_with_model
		(GTK_TREE_MODEL (route_list_tree));

	  	// ### show number of point in route list...
	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
		"#", renderer_routelist,
		"text", ROUTE_NUMBER, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist),
		column_routelist);

	renderer_routelist = gtk_cell_renderer_pixbuf_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes ("_",
		renderer_routelist, "pixbuf", ROUTE_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist),
		column_routelist);

	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
		_("Name"), renderer_routelist,
		"text", ROUTE_NAME, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist),
		column_routelist);

	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
		_("Distance"), renderer_routelist,
		"text", ROUTE_DISTANCE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist),
		column_routelist);
	
	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
		_("Trip"), renderer_routelist,
		"text", ROUTE_TRIP, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist),
		column_routelist);

	gtk_container_add (GTK_CONTAINER (scrolledwindow_routedetails),
		treeview_routelist);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview_routelist), TRUE);

	routelist_select = gtk_tree_view_get_selection
		(GTK_TREE_VIEW (treeview_routelist));
	gtk_tree_selection_set_mode (routelist_select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (routelist_select), "changed",
					G_CALLBACK (select_routepoint_cb), NULL);

	label_routedetails = gtk_label_new (_("Route List"));
	gtk_frame_set_label_widget (GTK_FRAME (frame_routedetails),
		label_routedetails);
	gtk_label_set_use_markup (GTK_LABEL (label_routedetails), TRUE);
  }
  
	dialog_action_area_routedetails = GTK_DIALOG (route_window)->action_area;

	/* button "start route" */
	button_startroute = gtk_button_new ();
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_startroute, 0);
	GTK_WIDGET_SET_FLAGS (button_startroute, GTK_CAN_DEFAULT);
	alignment_startroute = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_startroute), alignment_startroute);
	hbox_startroute = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_startroute), hbox_startroute);
	if (route.active)
	{
		image_startroute = gtk_image_new_from_stock
			("gtk-media-stop", GTK_ICON_SIZE_BUTTON);
		label_startroute = gtk_label_new_with_mnemonic (_("Stop Route"));
		gtk_tooltips_set_tip ( tooltips_routewindow, button_startroute,
					_("Stop the Route Mode"), NULL);
	}
	else
	{
		image_startroute = gtk_image_new_from_stock
			("gtk-media-play", GTK_ICON_SIZE_BUTTON);
		label_startroute = gtk_label_new_with_mnemonic (_("Start Route"));
		gtk_tooltips_set_tip ( tooltips_routewindow,
			button_startroute, _("Start the Route Mode"), NULL);
	}
	if (!route.items)
		gtk_widget_set_sensitive (button_startroute, FALSE);
	
	gtk_box_pack_start (GTK_BOX (hbox_startroute),
		image_startroute, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox_startroute),
		label_startroute, FALSE, FALSE, 0);
	g_signal_connect (button_startroute, "clicked",
		GTK_SIGNAL_FUNC (route_startstop_cb), NULL);

	/* button "remove point from route" */
	button_remove = gtk_button_new_from_stock ("gtk-remove");
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_remove, 0);
	GTK_WIDGET_SET_FLAGS (button_remove, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip ( tooltips_routewindow, button_remove, 
				_("Remove selected Entry from Route"), NULL);
	gtk_widget_set_sensitive (button_remove, FALSE);
	g_signal_connect_swapped (button_remove, "clicked",
		GTK_SIGNAL_FUNC (remove_routepoint_cb), routelist_select);

	/* button "cancel route" */
	button_cancel = gtk_button_new ();
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_cancel, 0);
	GTK_WIDGET_SET_FLAGS (button_cancel, GTK_CAN_DEFAULT);
	alignment_cancel = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_cancel), alignment_cancel);
	hbox_cancel = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_cancel), hbox_cancel);
	image_cancel = gtk_image_new_from_stock ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox_cancel), image_cancel, FALSE, FALSE, 0);
	label_cancel = gtk_label_new (_("Cancel Route"));		
	gtk_tooltips_set_tip ( tooltips_routewindow, button_cancel,
				_("Discard Route"), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_cancel), label_cancel, FALSE, FALSE, 0);
	g_signal_connect (button_cancel, "clicked",
				GTK_SIGNAL_FUNC (route_cancel_cb), NULL);

	/* button "save route" */
	button_routesave = gtk_button_new_from_stock ("gtk-save");
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_routesave, 0);
	GTK_WIDGET_SET_FLAGS (button_routesave, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip ( tooltips_routewindow, button_routesave, 
		_("Export current route to a GPX File"), NULL);
	gtk_widget_set_sensitive (button_routesave, FALSE);
	g_signal_connect_swapped (button_routesave, "clicked",
		GTK_SIGNAL_FUNC (savegpx_cb), (gpointer) GPX_RTE);

	/* button "close" */
	button_close = gtk_button_new_from_stock ("gtk-close");
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window),
		button_close, GTK_RESPONSE_CLOSE);
	GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip (tooltips_routewindow, button_close, 
				_("Close this window"), NULL);
	g_signal_connect_swapped (button_close, "clicked",
		GTK_SIGNAL_FUNC (close_route_window_cb), route_window);
	g_signal_connect (GTK_OBJECT (route_window), "delete_event",
		GTK_SIGNAL_FUNC (close_route_window_cb), NULL);

	gtk_widget_show_all (route_window);
}



