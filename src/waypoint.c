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
#include "streets.h"
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
#include "download_map.h"
#include "icons.h"
#include "poi.h"
#include "gui.h"
#include "gpsdrive_config.h"
#include "main_gui.h"

#include "gettext.h"
#include <speech_strings.h>
#include <speech_out.h>

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
extern gint isnight, disableisnight;
extern color_struct colors;
extern gint mydebug;
extern GtkWidget *map_drawingarea;
extern GdkGC *kontext_map;
extern gint usesql;
extern glong mapscale;

#include "mysql/mysql.h"

extern MYSQL mysql;
extern MYSQL_RES *res;
extern MYSQL_ROW row;
extern gdouble alarm_dist;
extern GtkWidget *posbt;
gint dontsetwp = FALSE;
extern gint selected_wp_mode;
extern GtkWidget *add_wp_lon_text, *add_wp_lat_text;
extern gint wptotal, wpselected;
extern GtkWidget *wp4eventbox;
extern GtkWidget *wp5eventbox, *satsvbox, *satshbox, *satslabel1eventbox;
extern gdouble posx, posy;
extern gdouble earthr;
extern gchar *displaytext;
extern gint do_display_dsc, textcount;
extern GtkWidget *destframe;
extern GTimer *timer, *disttimer;
extern gdouble gbreit, glang, olddist;
extern GtkWidget *messagewindow;
extern gint onemousebutton;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu;
extern GdkDrawable *drawable;
extern gchar oldfilename[2048];
extern poi_type_struct poi_type_list[poi_type_list_max];
extern int poi_type_list_count;
extern GList *poi_types_formatted;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern GdkGC *kontext;

gint saytarget = FALSE;
gint markwaypoint = FALSE;
gint foundradar;
GtkWidget *addwaypointwindow;
wpstruct *wayp;
gint wpsize = 1000;
gint maxwp;
time_t waytxtstamp = 0;
gint deleteline = 0;
gint selected_wp_list_line = 0;
GtkWidget *gotowindow;
gint setwpactive = FALSE;
gchar lastradar[40], lastradar2[40];
gdouble radarbearing;
/* action=1: radar (speedtrap) */
GtkWidget *add_wp_name_text, *wptext2;
GtkWidget *add_wp_comment_text;
long sortcolumn = 4, sortflag = 0;
gdouble wp_saved_target_lat = 0;
gdouble wp_saved_target_lon = 0;
gdouble wp_saved_posmode_lat = 0;
gdouble wp_saved_posmode_lon = 0;

gint save_in_db = TRUE;

/* *****************************************************************************
 * check for Radar WP near me and warn me
 */
gint
watchwp_cb (GtkWidget * widget, guint * datum)
{
	gint angle, i, radarnear;
	gdouble d;
	gchar buf[400], lname[200], l2name[200];
	gdouble ldist = 9999.0, l2dist = 9999.0;
	gdouble tx, ty, lastbearing;

	if ( mydebug >50 ) fprintf(stderr , "watchwp_cb()\n");

	/*  calculate new earth radius */
	earthr = calcR (coords.current_lat);

	if (current.importactive)
		return TRUE;

	foundradar = FALSE;
	radarnear = FALSE;


	for (i = 0; i < maxwp; i++)
	{
		/*  test for radar */
		if (((wayp + i)->action) == 1)
		{
			d = calcdist2 ((wayp + i)->lon, (wayp + i)->lat);
			if (d < 0.6)
			{
				lastbearing = radarbearing;
				tx = -coords.current_lon + (wayp + i)->lon;
				ty = -coords.current_lat + (wayp + i)->lat;
				radarbearing = atan (tx / ty);
				if (!finite (radarbearing))
					radarbearing = lastbearing;

				if (ty < 0)
					radarbearing = M_PI + radarbearing;
				radarbearing -= current.heading;
				if (radarbearing >= (2 * M_PI))
					radarbearing -= 2 * M_PI;
				if (radarbearing < 0)
					radarbearing += 2 * M_PI;
				if (radarbearing < 0)
					radarbearing += 2 * M_PI;
				angle = radarbearing * 180.0 / M_PI;

				if ((angle < 40) || (angle > 320))
				{
					foundradar = TRUE;
					if (d < ldist)
					{
						ldist = d;
						g_strlcpy (lname,
							   (wayp + i)->name,
							   sizeof (lname));
					}
					if (d < 0.2)
					{
						foundradar = TRUE;
						radarnear = TRUE;
						if (d < l2dist)
						{
							l2dist = d;
							g_strlcpy (l2name,
								   (wayp +
								    i)->name,
								   sizeof
								   (l2name));
						}
					}
				}

			}

		}
	}
	if (!foundradar)
	{
		g_strlcpy (lastradar, "----", sizeof (lastradar));
		g_strlcpy (lastradar2, "----", sizeof (lastradar2));
	}
	else
	{
		if ((strcmp (lname, lastradar)) != 0)
		{
			g_strlcpy (lastradar, lname, sizeof (lastradar));

      g_snprintf(
          buf, sizeof(buf), speech_danger_radar[voicelang],
          (int) (ldist * 1000.0), (int) current.groundspeed );
			speech_out_speek (buf);

			if (displaytext != NULL)
				free (displaytext);
			displaytext = strdup (buf + 10);
			displaytext = g_strdelimit (displaytext, "\n", ' ');
			displaytext = g_strdelimit (displaytext, "\")", ' ');

			do_display_dsc = TRUE;
			textcount = 0;
		}

		if (radarnear)
			if ((strcmp (l2name, lastradar2)) != 0)
			{
				g_strlcpy (lastradar2, l2name, sizeof (lastradar2));

        g_snprintf(
            buf, sizeof(buf), speech_info_radar[voicelang],
            (int) (ldist * 1000.0) );
				speech_out_speek (buf);
			}

	}

	return TRUE;
}

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
	gdouble posxdest, posydest;
	gint k, k2, i, shownwp = 0;
	gchar txt[200];

	if (mydebug > 10)
		printf ("draw_waypoints()\n");

	/*  draw waypoints */
	for (i = 0; i < maxwp; i++)
	{
		calcxy (&posxdest, &posydest,
			(wayp + i)->lon, (wayp + i)->lat, current.zoom);

		if ((posxdest >= 0) && (posxdest < SCREEN_X)
		    && (shownwp < MAXSHOWNWP)
		    && (posydest >= 0) && (posydest < SCREEN_Y))
		{
			gdk_gc_set_line_attributes (kontext_map, 2, 0, 0, 0);
			shownwp++;
			g_strlcpy (txt, (wayp + i)->name, sizeof (txt));

			// Draw Icon(typ) or + Sign
			if ((wayp + i)->wlan > 0)
				drawwlan (posxdest, posydest,
					(wayp + i)->wlan);
			else
				drawicon (posxdest, posydest,
					(wayp + i)->typ);

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


/* *****************************************************************************
 */
gint
importaway_cb (GtkWidget * widget, guint datum)
{
	current.importactive = FALSE;
	gtk_widget_destroy (widget);
	g_strlcpy (oldfilename, "XXXXXXXXXXXXXXXXXX", sizeof (oldfilename));
	return FALSE;
}


/* *****************************************************************************
 */
gint
delwp_cb (GtkWidget * widget, guint datum)
{
	gint i, j;
	gchar *p;

	i = deleteline;
	if ( mydebug > 0 )
		g_print ("delwp: remove line %d\n", i);
	gtk_clist_get_text (GTK_CLIST (mylist), i, 0, &p);
	j = atol (p) - 1;
	gtk_clist_remove (GTK_CLIST (mylist), i);
	if ( mydebug > 0 )
		g_print ("delwp: remove entry %d\n", j);

	deletesqldata ((wayp + j)->sqlnr);
	for (i = j; i < (maxwp - 1); i++)
		*(wayp + i) = *(wayp + i + 1);
	maxwp--;
	savewaypoints ();
	gtk_clist_get_text (GTK_CLIST (mylist), deleteline, 0, &p);
	selected_wp_list_line = atol (p);

	return TRUE;
}

/* *****************************************************************************
 * destroy sel_target window 
 */
gint
sel_targetweg_cb (GtkWidget * widget, guint datum)
{
	/*   gtk_timeout_remove (selwptimeout); */
	gtk_widget_destroy (GTK_WIDGET (gotowindow));
	/* restore old target */
	if (widget != NULL && !route.active) {
		coords.target_lat = wp_saved_target_lat;
		coords.target_lon = wp_saved_target_lon;
	}
	/* restore old posmode */
	if (widget != NULL && gui_status.posmode) {
		coords.posmode_lat = wp_saved_posmode_lat;
		coords.posmode_lon = wp_saved_posmode_lon;
	}
	setwpactive = FALSE;

	return FALSE;
}

/* *****************************************************************************
 */
 
/*
 * destroy sel_target window event:
 */
 gint
 sel_target_destroy_cb (GtkWidget *widget, guint datum) {
 	selected_wp_mode = FALSE;
 	return TRUE;
 }
 
gint
jumpwp_cb (GtkWidget * widget, guint datum)
{
	gint i;
	gchar *p;

	i = deleteline;
	if (gui_status.posmode) {
		gtk_clist_get_text (GTK_CLIST (mylist), i, 3, &p);
		coordinate_string2gdouble(p, &coords.posmode_lat);
		gtk_clist_get_text (GTK_CLIST (mylist), i, 4, &p);
		coordinate_string2gdouble(p, &coords.posmode_lon);
	}

	if ((!gui_status.posmode) && (!current.simmode))
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt),
					      TRUE);
	getsqldata ();
	reinsertwp_cb (NULL, 0);
	sel_targetweg_cb (NULL, 0);
	return TRUE;
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
addwaypoint (gchar * wp_name, gchar * wp_type, gchar * wp_comment, gdouble wp_lat, gdouble wp_lon, gint save_flag)
{
	gint i;

	if (usesql && save_in_db)
	{
		return insertsqldata (wp_lat, wp_lon, (char *) wp_name,
			(char *) wp_type, (char *) wp_comment, 3);
	}
	else
	{
		i = maxwp;
		(wayp + i)->lat = wp_lat;
		(wayp + i)->lon = wp_lon;
		g_strdelimit ((char *) wp_name, " ", '_');
		g_strdelimit ((char *) wp_comment, " ", '_');

		/*  limit waypoint name to 20 chars */
		g_strlcpy ((wayp + i)->name, wp_name, 40);
		(wayp + i)->name[20] = 0;

		/* limit waypoint type to 40 chars */
		g_strlcpy ((wayp + i)->typ, wp_type, 40);
		(wayp + i)->typ[40] = 0;
		
		/*  limit waypoint comment to 80 chars */
		g_strlcpy ((wayp + i)->comment, wp_comment, 80);
		(wayp + i)->comment[80] = 0;
		

		(wayp + i)->wlan = 0;

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
	G_CONST_RETURN gchar *s1, *s2, *s3;
	gchar wp_name[80];
	gchar wp_type[80];
	gchar wp_comment[255];

	s1 = gtk_entry_get_text (GTK_ENTRY (add_wp_name_text));
	s2 = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (wptext2)->entry));
	s3 = gtk_entry_get_text (GTK_ENTRY (add_wp_comment_text));
	
	if (!usesql)
		save_in_db = FALSE;
		
	g_strlcpy(wp_name,s1,sizeof(wp_name));
	g_strlcpy(wp_type,s2,sizeof(wp_type));
	g_strlcpy(wp_comment,s3,sizeof(wp_comment));

	addwaypoint (wp_name, wp_type, wp_comment,
		coords.wp_lat, coords.wp_lon, save_in_db);

	gtk_widget_destroy (GTK_WIDGET (widget));
	markwaypoint = FALSE;

	return TRUE;
}

/* *****************************************************************************/
void
add_wp_change_save_in_cb (GtkWidget *widget, gint user_data)
{
	save_in_db = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
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
gint
addwaypoint_cb (GtkWidget * widget, gpointer datum)
{
	GtkWidget *window;
	GtkWidget *vbox;
	gchar buff[40];
	GList *wp_types = NULL;
	gint i;

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
		wp_types = g_list_append (wp_types, "unknown");
		for (i = 2; i <= poi_type_list_count; i++)
		{
			if (g_ascii_strcasecmp(poi_type_list[i].name,"\0") != 0)
				wp_types = g_list_append (wp_types, poi_type_list[i].name);
		}		

		wptext2 = gtk_combo_new ();
		gtk_combo_set_popdown_strings (GTK_COMBO (wptext2),
					       (GList *) wp_types);
		add_wp_type_label = gtk_label_new (_(" Type: "));

		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_type_label, 0, 1, 1, 2);
		gtk_table_attach_defaults (GTK_TABLE (table_add_wp), wptext2, 1, 3, 1, 2);
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
		if (usesql)
		{
			add_wp_savein_label = gtk_label_new (_(" Save waypoint in: "));
			gtk_table_attach_defaults (GTK_TABLE (table_add_wp), add_wp_savein_label, 0, 1, 5, 6);

			radiobutton_db = gtk_radio_button_new_with_mnemonic (NULL, _("Database"));
			gtk_table_attach_defaults (GTK_TABLE (table_add_wp), radiobutton_db, 1, 2, 5, 6);
			gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_db), radiobutton_savein_group);
		
			radiobutton_savein_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton_db));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_db), TRUE);
	
			radiobutton_wp = gtk_radio_button_new_with_mnemonic (NULL, _("way.txt File"));
			gtk_table_attach_defaults (GTK_TABLE (table_add_wp), radiobutton_wp, 2, 3, 5, 6);
			gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_wp), radiobutton_savein_group);
			
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
 */
gint
setsortcolumn (GtkWidget * w, gpointer datum)
{
	sortflag = !sortflag;
	sortcolumn = (long) datum;

	if (sortflag)
		gtk_clist_set_sort_type (GTK_CLIST (mylist),
					 GTK_SORT_ASCENDING);
	else
		gtk_clist_set_sort_type (GTK_CLIST (mylist),
					 GTK_SORT_DESCENDING);

	if (w != messagewindow)
		reinsertwp_cb (NULL, 0);
	else
	{
		gtk_clist_set_sort_column (GTK_CLIST (mylist),
					   sortcolumn);
		gtk_clist_sort (GTK_CLIST (mylist));
	}
	return TRUE;
}


/* *****************************************************************************
 */
gint
click_clist (GtkWidget * widget, GdkEventButton * event, gpointer data)
{

	g_print ("\nclist: %d, data: %d\n", event->button, selected_wp_list_line);
	if ((event->button == 3))
	{

		return TRUE;
	}

	return FALSE;

}

/* *****************************************************************************
 * if a waypoint is selected set the target_* variables 
 */
gint
setwp_cb (GtkWidget * widget, guint datum)
{
	//gchar str[200];
	gchar b[100], buf[1000], buf2[1000];
	gchar *p, *tn;
	p = b;

	deleteline = datum;
	if (dontsetwp)
		return TRUE;
		
	/* enter selected_wp_mode -> 
	 * enables map jumping to target_lat/lon */
	selected_wp_mode = TRUE;

	gtk_clist_get_text (GTK_CLIST (mylist), datum, 0, &p);
	if (route.edit)
	{
		/*        g_print("route: %s\n", p); */
		thisrouteline = atol (p) - 1;
		insertroutepoints ();
		return TRUE;
	}
	if (route.active) {
		/* in routingmode do nothing further */
		return TRUE;
	}
	selected_wp_list_line = atol (p);
	/*    g_print("%d\n", selected_wp_list_line); */
	gtk_clist_get_text (GTK_CLIST (mylist), datum, 1, &p);
	g_strlcpy (current.target, p, sizeof (current.target));


//	g_snprintf (str, sizeof (str), "%s: %s", _("To"), current.target);
//	tn = g_strdelimit (str, "_", ' ');
//	gtk_frame_set_label (GTK_FRAME (destframe), tn);
	gtk_clist_get_text (GTK_CLIST (mylist), datum, 3, &p);
	coordinate_string2gdouble(p, &coords.target_lat);
	gtk_clist_get_text (GTK_CLIST (mylist), datum, 4, &p);
	coordinate_string2gdouble(p, &coords.target_lon);
	/* if posmode enabled set posmode_lat/lon */
	if (gui_status.posmode) {
		coords.posmode_lat = coords.target_lat;
		coords.posmode_lon = coords.target_lon;
	}
	/*    gtk_timeout_add (5000, (GtkFunction) sel_targetweg_cb, widget); */
	g_timer_stop (disttimer);
	g_timer_start (disttimer);
	olddist = current.dist;

	tn = g_strdelimit (current.target, "_", ' ');
	g_strlcpy (buf2, "", sizeof (buf2));
	if (tn[0] == '*')
	{
		g_strlcpy (buf2, "das mobile Ziel ", sizeof (buf2));
		g_strlcat (buf2, (tn + 1), sizeof (buf2));
	}
	else
		g_strlcat (buf2, tn, sizeof (buf2));

  g_snprintf( buf, sizeof(buf), speech_new_target[voicelang], buf2 );
	speech_out_speek (buf);

	saytarget = TRUE;

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

			if ( (wayp + i)->typ[0] == '\0' ) {
			    g_strlcpy ( (wayp + i)->typ, "unknown", 
					sizeof ((wayp + i)->typ));
			}

			e = fprintf (st, "%-22s %10s %11s "
				     "%s"
				     " %d %d %d %d %s"
				     "\n",
				     (wayp + i)->name, la, lo,
				     (wayp + i)->typ,
				     (wayp + i)->wlan, (wayp + i)->action,
				     (wayp + i)->sqlnr,
				     (wayp + i)->proximity,
					 (wayp + i)->comment);
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
    gint i, e, p, wlan, action, sqlnr, proximity;
    gchar buf[512], slat[80], slong[80], typ[40];
    struct stat stat_buf;
    
    if ( 0 == waytxtstamp )
	wayp = g_new (wpstruct, wpsize);
    
//    g_strlcpy (fn_way_txt, local_config.dir_home, sizeof (fn_way_txt));
//	g_strlcat (fn_way_txt, local_config.wp_file, sizeof (fn_way_txt));
    g_strlcpy (fn_way_txt, local_config.wp_file, sizeof (fn_way_txt));


    /* Check m_time of way.txt file */
    stat(fn_way_txt, &stat_buf);
    if ( stat_buf.st_mtime == waytxtstamp )
	return;
    waytxtstamp = stat_buf.st_mtime;


    
    maxwp = 0;
    sqlnr = -1;
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
	    e = sscanf (buf, "%s %s %s %s %d %d %d %d\n",
			(wayp + i)->name, slat, slong, typ,
			&wlan, &action, &sqlnr, &proximity);
	    coordinate_string2gdouble(slat, &((wayp + i)->lat));
	    coordinate_string2gdouble(slong,&((wayp + i)->lon));
	    /*  limit waypoint name to 20 chars */
	    (wayp + i)->name[20] = 0;
	    g_strlcpy ((wayp + i)->typ, "unknown", 40);
	    (wayp + i)->wlan = 0;
	    (wayp + i)->action = 0;
	    (wayp + i)->sqlnr = -1;
	    (wayp + i)->proximity = 0;
		    
	    if (e >= 3)
		{
		    (wayp + i)->dist = 0;
				
		    if (e >= 4)
			g_strlcpy ((wayp + i)->typ, typ, 40);
				
		    if (e >= 5)
			(wayp + i)->wlan = wlan;
		    if (e >= 6)
			(wayp + i)->action = action;
		    if (e >= 7)
			(wayp + i)->sqlnr = sqlnr;
		    if (e >= 8)
			(wayp + i)->proximity = proximity;

		    if ((strncmp((wayp + i)->name, "R-",2)) == 0)
	  		(wayp + i)->action = 1;

		    i++;
		    maxwp = i;

		    if (maxwp >= wpsize)
			{
			    wpsize += 1000;
			    wayp = g_renew (wpstruct, wayp,
					    wpsize);
			}

		}
	}

    fclose (st);

	g_print ("%s reloaded\n", local_config.wp_file);

}
