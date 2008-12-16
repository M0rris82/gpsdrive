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
 * waypoint module: 
 */

#include "config.h"
#include "gettext.h"
#include "gpsdrive.h"
#include "icons.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "routes.h"
#include "import_map.h"
#include "map_download.h"
#include "icons.h"
#include "poi.h"
#include "gui.h"
#include "gpsdrive_config.h"
#include "main_gui.h"

#include "gettext.h"


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern GtkWidget *mylist;
extern gint maploaded;
extern routestatus_struct route;
extern color_struct colors;
extern gint mydebug;
extern GtkWidget *map_drawingarea;
extern GdkGC *kontext_map;
extern glong mapscale;

gint dontsetwp = FALSE;
extern gint selected_wp_mode;
static GtkWidget *add_wp_lon_text, *add_wp_lat_text;
extern gdouble earthr;
extern gchar *displaytext;
extern gdouble olddist;
extern GtkWidget *messagewindow;
extern GdkDrawable *drawable;
extern GtkTreeModel *poi_types_tree_filtered;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern GdkGC *kontext;

gint saytarget = FALSE;
gint markwaypoint = FALSE;
GtkWidget *addwaypointwindow;
wpstruct *wayp;
gint wpsize = 1000;
gint maxwp;
time_t waytxtstamp = 0;
gint deleteline = 0;
gint selected_wp_list_line = 0;
GtkWidget *gotowindow;
gint setwpactive = FALSE;
GtkWidget *add_wp_name_text, *wptext2;
GtkWidget *add_wp_comment_text;
long sortcolumn = 4, sortflag = 0;
gdouble wp_saved_target_lat = 0;
gdouble wp_saved_target_lon = 0;
gdouble wp_saved_expmode_lat = 0;
gdouble wp_saved_expmode_lon = 0;


/* ******************************************************************
 */
void check_and_reload_way_txt()
{
    gchar mappath[2048];

	if (g_ascii_strncasecmp (local_config.wp_file, "/", 1) != 0)
	{
		g_strlcpy (mappath, local_config.dir_home, sizeof (mappath));
		g_strlcat (mappath, local_config.wp_file, sizeof (mappath));
	}
 
    loadwaypoints ();
}


/* *****************************************************************************
 * Draw waypoints on map
 */
void
draw_waypoints ()
{
	gint posxdest, posydest;
	gint k, k2, i, shownwp = 0;
	gchar txt[200];

	if (mydebug > 10)
		printf ("draw_waypoints()\n");

	/*  draw waypoints */
	for (i = 0; i < maxwp; i++)
	{
		calcxy (&posxdest, &posydest,
			(wayp + i)->lon, (wayp + i)->lat, current.zoom);

		if ((posxdest >= 0) && (posxdest < gui_status.mapview_x)
		    && (shownwp < MAXSHOWNWP)
		    && (posydest >= 0) && (posydest < gui_status.mapview_y))
		{
			gdk_gc_set_line_attributes (kontext_map, 2, 0, 0, 0);
			shownwp++;
			g_strlcpy (txt, (wayp + i)->name, sizeof (txt));

			// Draw Icon(typ)
			drawicon (posxdest, posydest, (wayp + i)->typ);

			// Draw Proximity Circle
			if ((wayp + i)->proximity > 0.0)
			{
				gint proximity_pixels;
				if (current.mapscale)
					proximity_pixels =
						((wayp + i)->proximity)
						* current.zoom * PIXELFACT / current.mapscale;
				else
					proximity_pixels = 2;

				gdk_gc_set_foreground (kontext_map, &colors.blue);

				gdk_draw_arc (drawable, kontext_map, FALSE,
					posxdest - proximity_pixels,
					posydest - proximity_pixels,
					proximity_pixels * 2,
					proximity_pixels * 2, 0,
					64 * 360);
			}

			/* draw name label (if there is one) */
			if( (wayp + i)->name[0] != '\0' ) {
			    {	/* draw shadow of text */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;
				gint width, height;
				gchar *tn;

				gdk_gc_set_foreground (kontext_map, &colors.shadow);
				gdk_gc_set_function (kontext_map, GDK_AND);
				tn = g_strdelimit (txt, "_", ' ');

				wplabellayout =
					gtk_widget_create_pango_layout
					(map_drawingarea, tn);
				pfd = pango_font_description_from_string
					(local_config.font_wplabel);
				pango_layout_set_font_description
					(wplabellayout, pfd);
				pango_layout_get_pixel_size (wplabellayout,
					&width, &height);
				/* printf("j: %d\n",height);    */
				k = width + 4;
				k2 = height;
				if (local_config.showshadow)
				{
					gdk_draw_layout_with_colors (drawable,
						kontext_map,
						posxdest + 15 + SHADOWOFFSET,
						posydest - k2 / 2 + SHADOWOFFSET,
						wplabellayout,
						&colors.shadow,
						NULL);
				}
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
						(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			    }
			    gdk_gc_set_function (kontext_map, GDK_COPY);


			    gdk_gc_set_function (kontext_map, GDK_AND);

			    gdk_gc_set_foreground (kontext_map, &colors.textbacknew);
			    gdk_draw_rectangle (drawable, kontext_map, 1,
					    posxdest + 13, posydest - k2 / 2,
					    k + 1, k2);
			    gdk_gc_set_function (kontext_map, GDK_COPY);
			    gdk_gc_set_foreground (kontext_map, &colors.black);
			    gdk_gc_set_line_attributes (kontext_map, 1, 0, 0, 0);
			    gdk_draw_rectangle (drawable, kontext_map, 0,
					    posxdest + 12,
					    posydest - k2 / 2 - 1, k + 2, k2);

			    /*            gdk_gc_set_foreground (kontext, &yellow);  */
			    {
				/* prints in pango */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;

				wplabellayout =
					gtk_widget_create_pango_layout
					(map_drawingarea, txt);
				pfd = pango_font_description_from_string
					(local_config.font_wplabel);
				pango_layout_set_font_description
					(wplabellayout, pfd);

				gdk_draw_layout_with_colors (drawable,
							     kontext_map,
							     posxdest + 15,
							     posydest -
							     k2 / 2,
							     wplabellayout,
							     &colors.wplabel, NULL);
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			    }
			}
		}
	}
}


/* *****************************************************************************
 */
gint
addwaypointchange_cb (GtkWidget * widget, guint datum)
{
	gchar *s;
	gdouble lo, la;

	s = g_strstrip ((char *)
			gtk_entry_get_text (GTK_ENTRY (add_wp_lon_text)));
	coordinate_string2gdouble(s, &lo);
	if ((lo > -181) && (lo < 181))
		coords.wp_lon = lo;
	s = g_strstrip ((char *)
			gtk_entry_get_text (GTK_ENTRY (add_wp_lat_text)));
	coordinate_string2gdouble(s, &la);
	if ((la > -181) && (la < 181))
		coords.wp_lat = la;

	return TRUE;
}


/* *****************************************************************************
 * Add waypoint at coords.wp_lat, coords.wp_lon
 * with Strings for Name and Type
 */
glong
addwaypoint (gchar * wp_name, gchar * wp_type, gchar * wp_comment,
	     gdouble wp_lat, gdouble wp_lon, gboolean save_in_db)
{
	gint i;

	if (local_config.use_database && save_in_db)
	{
		return db_poi_edit (0, wp_lat, wp_lon, (char *) wp_name,
			(char *) wp_type, (char *) wp_comment, 3, FALSE);
	}
	else
	{
		i = maxwp;
		(wayp + i)->lat = wp_lat;
		(wayp + i)->lon = wp_lon;
		(wayp + i)->proximity = 0;
		g_strdelimit ((char *) wp_name, " ", '_');
		g_strdelimit ((char *) wp_comment, " ", '_');

		g_strlcpy ((wayp + i)->name, wp_name, sizeof ((wayp + i)->name));
		//(wayp + i)->name[20] = 0;

		g_strlcpy ((wayp + i)->typ, wp_type, sizeof ((wayp + i)->typ));
		//(wayp + i)->typ[40-1] = 0;
		
		g_strlcpy ((wayp + i)->comment, wp_comment, sizeof ((wayp + i)->comment));
		//(wayp + i)->comment[80-1] = 0;


		maxwp++;
		if (maxwp >= wpsize)
		{
			wpsize += 1000;
			wayp = g_renew (wpstruct, wayp, wpsize);
		}
		savewaypoints ();
		return 0;
	}
}

/* *****************************************************************************
 * callback from gtk to add waypoint
 */
gint
addwaypoint_gtk_cb (GtkWidget * widget, guint datum)
{
	G_CONST_RETURN gchar *s1, *s2;
	gchar wp_name[80];
	gchar *wp_type;
	gchar wp_comment[255];
    GtkTreeIter t_iter;

	s1 = gtk_entry_get_text (GTK_ENTRY (add_wp_name_text));
	s2 = gtk_entry_get_text (GTK_ENTRY (add_wp_comment_text));
	
	g_strlcpy(wp_name,s1,sizeof(wp_name));
	g_strlcpy(wp_comment,s2,sizeof(wp_comment));

    if (!current.poitype_path) return FALSE;
    if (!gtk_tree_model_get_iter_from_string
        (GTK_TREE_MODEL (poi_types_tree_filtered), &t_iter, current.poitype_path)
        ) return 0;
      

	gtk_tree_model_get (poi_types_tree_filtered, &t_iter, POITYPE_NAME, &wp_type, -1);

	addwaypoint (wp_name, wp_type, wp_comment,
		coords.wp_lat, coords.wp_lon, current.save_in_db);

	gtk_widget_destroy (GTK_WIDGET (widget));
	markwaypoint = FALSE;

	g_free (wp_type);
	
	return TRUE;
}

/* *****************************************************************************/
void
add_wp_change_save_in_cb (GtkWidget *widget, gint user_data)
{
	current.save_in_db = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
}

/* *****************************************************************************
 * destroy sel_target window 
 */
gint
addwaypointdestroy_cb (GtkWidget * widget, guint datum)
{

	gtk_widget_destroy (GTK_WIDGET (addwaypointwindow));
	markwaypoint = FALSE;

	return FALSE;
}


/* *****************************************************************************
 */
static gint
select_wptype_cb (GtkComboBox *combo_box, gpointer data)
{
  GtkTreeIter t_iter;
  gtk_combo_box_get_active_iter (combo_box, &t_iter);
  if (current.poitype_path) g_free(current.poitype_path);
  current.poitype_path = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL (poi_types_tree_filtered), &t_iter);
  return FALSE;
}


/* *****************************************************************************
 */
gint
addwaypoint_cb (GtkWidget * widget, gpointer datum)
{
	GtkWidget *window;
	GtkWidget *vbox;
	gchar buff[40];

	GtkWidget *add_wp_name_label;
	GtkWidget *add_wp_type_label;
	GtkWidget *add_wp_comment_label;
	GtkWidget *add_wp_lon_label;
	GtkWidget *add_wp_lat_label;
	GtkWidget *add_wp_button_hbox;
	GtkWidget *button, *button2;
	GtkWidget *table_add_wp;
	GSList *radiobutton_savein_group = NULL;
	GtkWidget *radiobutton_db;
	GtkWidget *radiobutton_wp;
	GtkWidget *add_wp_savein_label;
	GtkWidget *add_wp_type_combo;
	GtkCellRenderer *renderer_type_name;
	GtkCellRenderer *renderer_type_icon;
	GtkTreeIter t_iter;

	addwaypointwindow = window = gtk_dialog_new ();

	gtk_window_set_default_size (GTK_WINDOW (addwaypointwindow), 320, -1);
	gotowindow = window;
	markwaypoint = TRUE;

	gtk_window_set_modal (GTK_WINDOW (window), TRUE);
	gtk_window_set_title (GTK_WINDOW (window), _("Add Point of Interest"));

	vbox = gtk_vbox_new (TRUE, 2);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), vbox, TRUE,
			    TRUE, 2);

	table_add_wp = gtk_table_new (6, 3, FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table_add_wp, TRUE, TRUE, 2);
	
	{			/* Name */
		add_wp_name_text = gtk_entry_new ();
		add_wp_name_label = gtk_label_new (_("Name:"));
		gtk_entry_set_max_length (GTK_ENTRY (add_wp_name_text), 80);
		gtk_window_set_focus (GTK_WINDOW (window), add_wp_name_text);

		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_name_label, 0, 1, 0, 1);
		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_name_text, 1, 3, 0, 1);
	}

	{			/* Types */
	add_wp_type_label = gtk_label_new (_(" Type: "));
	add_wp_type_combo = gtk_combo_box_new_with_model
		(poi_types_tree_filtered);
	renderer_type_icon = gtk_cell_renderer_pixbuf_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (add_wp_type_combo),
		renderer_type_icon, FALSE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (add_wp_type_combo),
		renderer_type_icon, "pixbuf", POITYPE_ICON, NULL);
	renderer_type_name = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (add_wp_type_combo),
		renderer_type_name, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (add_wp_type_combo),
		renderer_type_name, "text", POITYPE_TITLE, NULL);
	
    
    gboolean PoiInitPath = TRUE;
    if (current.poitype_path) {
        if (gtk_tree_model_get_iter_from_string (
	    GTK_TREE_MODEL (poi_types_tree_filtered), &t_iter, current.poitype_path))
        {
            PoiInitPath = FALSE;
        }
    }
    
    if (PoiInitPath) {
        /* new init set first entry */
        gtk_tree_model_get_iter_first (poi_types_tree_filtered, &t_iter);
        current.poitype_path = gtk_tree_model_get_string_from_iter(
				    GTK_TREE_MODEL (poi_types_tree_filtered), &t_iter);
    }
    
	gtk_combo_box_set_active_iter (GTK_COMBO_BOX(add_wp_type_combo), &t_iter);

	g_signal_connect (G_OBJECT (add_wp_type_combo), "changed",
		G_CALLBACK (select_wptype_cb), NULL);

	gtk_table_attach_defaults (GTK_TABLE (table_add_wp),
		add_wp_type_label, 0, 1, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (table_add_wp),
		add_wp_type_combo, 1, 3, 1, 2);
	}

	{		/* Comment */
		add_wp_comment_text = gtk_entry_new ();
		add_wp_comment_label = gtk_label_new (_(" Comment: "));
		gtk_entry_set_max_length (GTK_ENTRY (add_wp_comment_text), 255);

		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_comment_label, 0, 1, 2, 3);
		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_comment_text, 1, 3, 2, 3);
	}
	
	{		/* Lat */
		add_wp_lat_text = gtk_entry_new_with_max_length (20);
		coordinate2gchar(buff, sizeof(buff), coords.wp_lat, TRUE,
			local_config.coordmode);
		gtk_entry_set_text (GTK_ENTRY (add_wp_lat_text), buff);
		add_wp_lat_label = gtk_label_new (_("Latitude"));
		gtk_signal_connect (GTK_OBJECT (add_wp_lat_text), "changed",
					GTK_SIGNAL_FUNC (addwaypointchange_cb),
					(gpointer) 2);

		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_lat_label, 0, 1, 3, 4);
		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_lat_text, 1, 3, 3, 4);
	}

	{			/* Lon */
		add_wp_lon_text = gtk_entry_new_with_max_length (20);
		coordinate2gchar(buff, sizeof(buff), coords.wp_lon, FALSE,
			local_config.coordmode);
		gtk_entry_set_text (GTK_ENTRY (add_wp_lon_text), buff);
		add_wp_lon_label = gtk_label_new (_("Longitude"));
		gtk_signal_connect (GTK_OBJECT (add_wp_lon_text), "changed",
				    GTK_SIGNAL_FUNC (addwaypointchange_cb),
				    (gpointer) 1);

		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_lon_label, 0, 1, 4, 5);
		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_lon_text, 1, 3, 4, 5);
	}

	{		/* Save WP in... */
		if (local_config.use_database)
		{
			add_wp_savein_label = gtk_label_new (_(" Save waypoint in: "));
			gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_savein_label, 0, 1, 5, 6);

			radiobutton_db = gtk_radio_button_new_with_mnemonic (NULL, _("Database"));
			gtk_table_attach_defaults (GTK_TABLE (table_add_wp), radiobutton_db, 1, 2, 5, 6);
			gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_db), radiobutton_savein_group);
		
			radiobutton_savein_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton_db));
			radiobutton_wp = gtk_radio_button_new_with_mnemonic (NULL, _("way.txt File"));
			gtk_table_attach_defaults (GTK_TABLE (table_add_wp), radiobutton_wp, 2, 3, 5, 6);
			gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_wp), radiobutton_savein_group);
			if (current.save_in_db)
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					(radiobutton_db), TRUE);
			}
			else
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					(radiobutton_wp), TRUE);
			}
			g_signal_connect (radiobutton_db, "toggled",
						GTK_SIGNAL_FUNC (add_wp_change_save_in_cb), 0);
		}
	}
	
	{			/* Buttons */
		button = gtk_button_new_from_stock (GTK_STOCK_OK);
		button2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);

		GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
		GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
		gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				    GTK_SIGNAL_FUNC (addwaypoint_gtk_cb),
				    GTK_OBJECT (window));
		gtk_signal_connect (GTK_OBJECT (button2), "clicked",
				    GTK_SIGNAL_FUNC (addwaypointdestroy_cb),
				    0);
		gtk_signal_connect (GTK_OBJECT (window), "delete_event",
				    GTK_SIGNAL_FUNC (addwaypointdestroy_cb),
				    0);

		add_wp_button_hbox = gtk_hbox_new (TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_button_hbox), button2,
				    TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_button_hbox), button,
				    TRUE, TRUE, 2);
		gtk_window_set_default (GTK_WINDOW (window), button);
		gtk_box_pack_start (GTK_BOX
				    (GTK_DIALOG (window)->action_area),
				    add_wp_button_hbox, TRUE, TRUE, 2);
	}

	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_widget_show_all (window);
	return TRUE;
}


/* *****************************************************************************
 * save waypoints to way.txt 
 */
void
savewaypoints ()
{
	gchar la[20], lo[20];
	FILE *st;
	gint i, e;

	st = fopen (local_config.wp_file, "w+");
	if (st == NULL)
	{
		perror (local_config.wp_file);
	}
	else
	{
		for (i = 0; i < maxwp; i++)
		{
			g_snprintf (la, sizeof (la), "%10.6f",
				    (wayp + i)->lat);
			g_snprintf (lo, sizeof (lo), "%10.6f",
				    (wayp + i)->lon);
			g_strdelimit (la, ",", '.');
			g_strdelimit (lo, ",", '.');

			/* still need to write something if no label */
			if( (wayp + i)->name[0] == '\0' )
			    g_strlcpy ( (wayp + i)->name, "_",
			    		sizeof ((wayp + i)->name));

			if ( (wayp + i)->typ[0] == '\0' ) {
			    g_strlcpy ( (wayp + i)->typ, "unknown", 
					sizeof ((wayp + i)->typ));
			}

			if( (wayp + i)->proximity > 0 || 
				strlen((wayp + i)->comment) > 0) {
			    e = fprintf (st, "%s %10s %11s "
				     "%s"
				     " 0 0 0 %d %s"
				     "\n",
				     (wayp + i)->name, la, lo,
				     (wayp + i)->typ,
				     (wayp + i)->proximity,
				     (wayp + i)->comment);
			}
			else {
			    e = fprintf (st, "%s %10s %11s %s\n",
				     (wayp + i)->name, la, lo,
				     (wayp + i)->typ);
			}
		}
		fclose (st);
	}

}

/* *****************************************************************************
 * load the waypoint from way.txt
 */
void
loadwaypoints ()
{
    gchar fn_way_txt[2048];
    FILE *st;
    gint i, e, p, wlan, action, proximity, tmp;
    gchar buf[512], slat[80], slong[80], typ[40];
    struct stat stat_buf;
    
    if ( 0 == waytxtstamp )
	wayp = g_new (wpstruct, wpsize);
    
//    g_strlcpy (fn_way_txt, local_config.dir_home, sizeof (fn_way_txt));
//    g_strlcat (fn_way_txt, local_config.wp_file, sizeof (fn_way_txt));
    g_strlcpy (fn_way_txt, local_config.wp_file, sizeof (fn_way_txt));


    /* Check m_time of way.txt file */
    stat(fn_way_txt, &stat_buf);
    if ( stat_buf.st_mtime == waytxtstamp )
	return;
    waytxtstamp = stat_buf.st_mtime;


    
    maxwp = 0;
    st = fopen (fn_way_txt, "r");
    if (st == NULL)
	{
	    perror (fn_way_txt);
	    return;
	}

    if ( mydebug > 0 )
	{
	    g_print ("load waypoint file %s\n",fn_way_txt);
	}

    i = 0;
    while ((p = fgets (buf, 512, st) != 0))
	{
	    e = sscanf (buf, "%s %s %s %s %d %d %d %d %79s",
			(wayp + i)->name, slat, slong, typ,
			&tmp, &tmp, &tmp, &proximity, (wayp + i)->comment);

	    /* convert DDD:MM:SSh or DDDdMM'SS"h to decimal degrees */
	    coordinate_string2gdouble(slat, &((wayp + i)->lat));
	    coordinate_string2gdouble(slong,&((wayp + i)->lon));

	    /* ignore blank placeholder labels */
	    if( strcmp( (wayp + i)->name, "_") == 0 )
		(wayp + i)->name[0] = 0;

	    g_strlcpy ((wayp + i)->typ, "unknown", sizeof (wayp + i)->typ);
	    (wayp + i)->proximity = 0;

	    if (e >= 3)
		{
		    (wayp + i)->dist = 0;
		
		    if (e >= 4)
			g_strlcpy ((wayp + i)->typ, typ, sizeof (wayp + i)->typ);
		    if (e >= 8)
			(wayp + i)->proximity = proximity;
		    if (e >= 9)
			(wayp + i)->comment[254] = 0;


		    i++;
		    maxwp = i;

		    if (maxwp >= wpsize)
			{
			    wpsize += 1000;
			    wayp = g_renew (wpstruct, wayp, wpsize);
			}
		}
	}

    fclose (st);

    g_print ("%s reloaded, read %d waypoints\n", local_config.wp_file, maxwp);

}
