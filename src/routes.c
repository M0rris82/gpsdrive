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

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "poi.h"
#include "config.h"
#include "gettext.h"
#include "icons.h"
#include "routes.h"
#include "gui.h"
#include <speech_out.h>
#include <speech_strings.h>

#include "gettext.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern gint debug, mydebug;
extern gdouble target_lon, target_lat;
extern GtkWidget *destframe;
extern gchar targetname[40];
extern gint havespeechout, speechcount;
extern gchar oldangle[100];
extern gint saytarget;
extern wpstruct *wayp;
extern gint maxwp, maxfriends;
extern friendsstruct *friends, *fserver;
extern int sortcolumn, sortflag;
extern gint selected_wp_list_line;
extern GtkWidget *mylist;
extern gint onemousebutton;
extern GtkWidget *mainwindow;
extern gint dontsetwp;
extern gint usesql;
extern poi_struct *poi_list;
extern glong poi_list_count;	
extern color_struct colors;

GtkWidget *routewindow;
wpstruct *routelist;
GtkListStore *route_list_tree;
GtkWidget *myroutelist;
gint thisrouteline = 0;
gdouble routenearest = 9999999999.0;
GtkWidget *create_route_button, *create_route2_button, *select_route_button, *gotobt;
gint forcenextroutepoint = FALSE;
status_struct route;
extern GtkWidget *route_window;

/* ******************************************************************
 */
void
init_route_list ()
{
    routelist = g_new0 (wpstruct, 100);
}

/* ******************************************************************
 */
void
free_route_list ()
{
    g_free (routelist);
}

/* *****************************************************************************
 * set the target in routemode 
 */

void
setroutetarget (GtkWidget * widget, gint datum)
{
	gchar buf[1000], buf2[1000], str[200], *tn;


	if ( mydebug >50 ) fprintf(stderr , "setroutetarget()\n");

	if (datum != -1)
		route.pointer = datum;

	routenearest = 9999999;
	g_strlcpy (targetname, (routelist + route.pointer)->name,
		   sizeof (targetname));
	target_lat = (routelist + route.pointer)->lat;
	target_lon = (routelist + route.pointer)->lon;
	g_snprintf (str, sizeof (str), "%s: %s[%d/%d]", _("To"), targetname,
		    route.pointer + 1, route.items);
	gtk_frame_set_label (GTK_FRAME (destframe), str);
	tn = g_strdelimit (targetname, "_", ' ');
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

	speechcount = 0;
	g_strlcpy (oldangle, "XXX", sizeof (oldangle));
	saytarget = TRUE;
}

/* *****************************************************************************
 * cancel sel_route window 
 */
gint
sel_routecancel_cb (GtkWidget * widget, guint datum)
{
	gchar str[200];

	gtk_widget_destroy (GTK_WIDGET (routewindow));

	g_snprintf (str, sizeof (str), "%s: %s", _("To"), targetname);
	gtk_frame_set_label (GTK_FRAME (destframe), str);
	route.edit = FALSE;
	route.active = FALSE;
	route.pointer = route.items = 0;
	gtk_widget_set_sensitive (create_route_button, TRUE);
	/* enable jump button */
	gtk_widget_set_sensitive (gotobt, TRUE);
	return FALSE;
}

/* *****************************************************************************
 * destroy sel_route window but continue routing
 */
gint
sel_routeclose_cb (GtkWidget * widget, guint datum)
{
	route.edit = FALSE;
	gtk_widget_destroy (GTK_WIDGET (routewindow));
	gtk_widget_set_sensitive (create_route_button, TRUE);
	return FALSE;
}

/* *****************************************************************************
 */
gint
do_route_cb (GtkWidget * widget, guint datum)
{
	gtk_widget_destroy (GTK_WIDGET (routewindow));
	gtk_widget_set_sensitive (create_route_button, TRUE);
	/* disable waypoint jump button */
	gtk_widget_set_sensitive (gotobt, FALSE);
	route.edit = FALSE;
	route.active = TRUE;
	setroutetarget (NULL, -1);
	return FALSE;
}


/* *****************************************************************************
 */
void
insertroutepoints ()
{
	gint i, j;
	gchar *text[5], text0[20], text1[20], text2[20], text3[20];

	i = thisrouteline;
	(wayp + i)->dist = calcdist ((wayp + i)->lon, (wayp + i)->lat);
	text[1] = (wayp + i)->name;
	g_snprintf (text0, sizeof (text0), "%d", i + 1);

	coordinate2gchar(text1, sizeof(text1), (wayp+i)->lat, TRUE,
		local_config.coordmode);
	coordinate2gchar(text2, sizeof(text2), (wayp+i)->lon, FALSE,
		local_config.coordmode);
	g_snprintf (text3, sizeof (text3), "%9.3f", (wayp + i)->dist);
	text[0] = text0;
	text[2] = text1;
	text[3] = text2;
	text[4] = text3;
	j = gtk_clist_append (GTK_CLIST (myroutelist), (gchar **) text);
	gtk_clist_set_foreground (GTK_CLIST (myroutelist), j, &colors.black);
	g_strlcpy ((routelist + route.items)->name, (wayp + i)->name, 40);
	(routelist + route.items)->lat = (wayp + i)->lat;
	(routelist + route.items)->lon = (wayp + i)->lon;
	route.items++;
	gtk_widget_set_sensitive (select_route_button, TRUE);

}

/* *****************************************************************************
 */
void
insertallroutepoints ()
{
	gint i, j;
	gchar *text[5], text0[20], text1[20], text2[20], text3[20];

	for (i = 0; i < maxwp; i++)
	{
		(wayp + i)->dist =
			calcdist ((wayp + i)->lon, (wayp + i)->lat);
		text[1] = (wayp + i)->name;
		g_snprintf (text0, sizeof (text0), "%d", i + 1);

		coordinate2gchar(text1, sizeof(text1), (wayp+i)->lat, TRUE,
			local_config.coordmode);
		coordinate2gchar(text2, sizeof(text2), (wayp+i)->lon, FALSE,
			local_config.coordmode);
		g_snprintf (text3, sizeof (text3), "%9.3f", (wayp + i)->dist);
		text[0] = text0;
		text[2] = text1;
		text[3] = text2;
		text[4] = text3;
		j = gtk_clist_append (GTK_CLIST (myroutelist),
				      (gchar **) text);
		gtk_clist_set_foreground (GTK_CLIST (myroutelist), j, &colors.black);
		g_strlcpy ((routelist + route.items)->name, (wayp + i)->name,
			   40);
		(routelist + route.items)->lat = (wayp + i)->lat;
		(routelist + route.items)->lon = (wayp + i)->lon;
		route.items++;
	}
	gtk_widget_set_sensitive (select_route_button, TRUE);

}

/* *****************************************************************************
 */
void
insertwaypoints (gint mobile)
{
	gint i, j;
	gchar *text[6], text0[20], text1[20], text2[20], text3[20], name[40];
	gdouble la, lo, dist;
	time_t ti, tif;

	/*  insert waypoint into the clist */

	if (!mobile)
	{
		if (usesql)
		{
			return;
		}
		else
		{
			for (i = 0; i < maxwp; i++)
			{
				(wayp + i)->dist =
				  calcdist ((wayp + i)->lon, (wayp + i)->lat);
				
				text[1] = (wayp + i)->name;
				text[2] = (wayp + i)->typ;
				
				g_snprintf (text0, sizeof (text0), "%02d", i + 1);
				coordinate2gchar(text1, sizeof(text1),
					(wayp+i)->lat, TRUE,
					local_config.coordmode);
				coordinate2gchar(text2, sizeof(text2),
					(wayp+i)->lon, FALSE,
					local_config.coordmode);
				g_snprintf (text3, sizeof (text3), "%9.3f",
				          (wayp + i)->dist);
				text[0] = text0;
				text[3] = text1;
				text[4] = text2;
				text[5] = text3;
				j = gtk_clist_append (GTK_CLIST (mylist),
					      (gchar **) text);
				gtk_clist_set_foreground (GTK_CLIST (mylist), j,
						  &colors.black);
			}
		}
	}
	
	for (i = 0; i < maxfriends; i++)
	{
		ti = time (NULL);
		tif = atol ((friends + i)->timesec);
		if ((ti - local_config.friends_maxsecs) > tif)
			continue;

		if (mobile)
			g_strlcpy (name, "", sizeof (name));
		else
			g_strlcpy (name, "*", sizeof (name));
		g_strlcat (name, (friends + i)->name, sizeof (name));
		g_snprintf (text0, sizeof (text0), "%d", i + maxwp + 1);
		coordinate_string2gdouble((friends + i)->lat, &la);
		coordinate_string2gdouble((friends + i)->lon, &lo);
		coordinate2gchar(text1, sizeof(text1), la, TRUE,
			local_config.coordmode);
		coordinate2gchar(text2, sizeof(text2), lo, FALSE,
			local_config.coordmode);

		if (!mobile)
		{
			text[0] = text0;
			text[1] = name;
			text[2] = (friends + i)->type;
			text[3] = text1;
			text[4] = text2;
			dist = calcdist (lo, la);
			g_snprintf (text3, sizeof (text3), "%9.3f", dist);
			text[5] = text3;
		}
		else
		{
			text[0] = name;
			text[1] = text1;
			text[2] = text2;
			dist = calcdist (lo, la);
			g_snprintf (text3, sizeof (text3), "%9.3f", dist);
			text[3] = text3;
		}

		j = gtk_clist_append (GTK_CLIST (mylist), (gchar **) text);
		if (mobile)
			gtk_clist_set_foreground (GTK_CLIST (mylist), j,
						  &colors.black);
		else
			gtk_clist_set_foreground (GTK_CLIST (mylist), j,
						  &colors.red);
	}

	/*  we want te columns sorted by distance from current position */
	gtk_clist_set_sort_column (GTK_CLIST (mylist), (gint) sortcolumn);
	gtk_clist_sort (GTK_CLIST (mylist));
}

/* *****************************************************************************
 */
gint
reinsertwp_cb (GtkWidget * widget, guint datum)
{
	gint i, j, k, val;
	gchar *p;
	GtkAdjustment *ad;

	/*  update routine for select target window */
	k = 0;
	ad = gtk_clist_get_vadjustment (GTK_CLIST (mylist));
	val = (GTK_ADJUSTMENT (ad)->value);

	gtk_clist_freeze (GTK_CLIST (mylist));
	gtk_clist_clear (GTK_CLIST (mylist));
	insertwaypoints (FALSE);
	for (i = 0; i < maxwp; i++)
	{
		gtk_clist_get_text (GTK_CLIST (mylist), i, 0, &p);
		j = atol (p);
		if (selected_wp_list_line == j)
		{
			k = i;
			break;
		}
	}
	gtk_adjustment_set_value (GTK_ADJUSTMENT (ad), val);
	dontsetwp = TRUE;
	gtk_clist_select_row (GTK_CLIST (mylist), k, 0);
	dontsetwp = FALSE;
	gtk_clist_thaw (GTK_CLIST (mylist));
	return TRUE;
}

/* ******************************************************************
 *
 */
gint
create_route_cb (GtkWidget * widget, guint datum)
{
	GtkWidget *window;
	gchar *tabeltitel1[] = { "#",
		_("Waypoint"), _("Latitude"), _("Longitude"), _("Distance"),
		NULL
	};
	GtkWidget *scrwindow, *vbox, *button, *button3, *hbox, *hbox_displays, *l1;
	gint i, j;
	gchar *text[5], text0[20], text1[20], text2[20], text3[20];
	GtkTooltips *tooltips;

	route.edit = TRUE;
	window = gtk_dialog_new ();
	routewindow = window;
	/*    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE); */
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	gtk_window_set_title (GTK_WINDOW (window), _("Define route"));
	gtk_window_set_default_size (GTK_WINDOW (window), 320, 320);
	myroutelist = gtk_clist_new_with_titles (5, tabeltitel1);
	gtk_signal_connect (GTK_OBJECT (GTK_CLIST (myroutelist)),
			    "select-row",
			    GTK_SIGNAL_FUNC (setroutetarget),
			    GTK_OBJECT (myroutelist));

	select_route_button = gtk_button_new_with_label (_("Start route"));
	gtk_widget_set_sensitive (select_route_button, FALSE);

	GTK_WIDGET_SET_FLAGS (select_route_button, GTK_CAN_DEFAULT);
	gtk_signal_connect (GTK_OBJECT (select_route_button), "clicked",
			    GTK_SIGNAL_FUNC (do_route_cb), 0);
	gtk_window_set_default (GTK_WINDOW (window), select_route_button);

	create_route2_button =
		gtk_button_new_with_label (_("Take all WP as route"));
	GTK_WIDGET_SET_FLAGS (create_route2_button, GTK_CAN_DEFAULT);
	gtk_signal_connect (GTK_OBJECT (create_route2_button), "clicked",
			    GTK_SIGNAL_FUNC (insertallroutepoints), 0);

	button = gtk_button_new_with_label (_("Abort route"));
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				   GTK_SIGNAL_FUNC
				   (sel_routecancel_cb), GTK_OBJECT (window));
	gtk_signal_connect_object (GTK_OBJECT (window),
				   "delete_event",
				   GTK_SIGNAL_FUNC
				   (sel_routeclose_cb), GTK_OBJECT (window));

	/*   button3 = gtk_button_new_with_label (_("Close")); */
	button3 = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	GTK_WIDGET_SET_FLAGS (button3, GTK_CAN_DEFAULT);
	gtk_signal_connect_object (GTK_OBJECT (button3), "clicked",
				   GTK_SIGNAL_FUNC
				   (sel_routeclose_cb), GTK_OBJECT (window));

	/* Font �ndern falls PDA-Mode und Touchscreen */
	if (local_config.guimode == GUI_PDA)
	{
		if (onemousebutton)
		{

			/* Change default font throughout the widget */
			PangoFontDescription *font_desc;
			font_desc =
				pango_font_description_from_string
				("Sans 10");
			gtk_widget_modify_font (myroutelist, font_desc);
			pango_font_description_free (font_desc);
		}
	}

	gtk_clist_set_column_justification (GTK_CLIST (myroutelist), 4,
					    GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification (GTK_CLIST (myroutelist), 0,
					    GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 1, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 2, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 3, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (myroutelist), 4, TRUE);

	scrwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scrwindow), myroutelist);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
					(scrwindow),
					(GtkPolicyType)
					GTK_POLICY_AUTOMATIC,
					(GtkPolicyType) GTK_POLICY_AUTOMATIC);
	vbox = gtk_vbox_new (FALSE, 2);
	/*   gtk_container_add (GTK_CONTAINER (window), vbox); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX (vbox), scrwindow, TRUE, TRUE, 2);
	hbox = gtk_hbox_new (TRUE, 2);
	hbox_displays = gtk_hbox_new (TRUE, 2);
	if (!route.active)
		l1 = gtk_label_new (_
				    ("Click on waypoints list\nto add waypoints"));
	else
		l1 = gtk_label_new (_
				    ("Click on list item\nto select next waypoint"));
	gtk_box_pack_start (GTK_BOX (vbox), l1, FALSE, FALSE, 2);
	/*   gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 2); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    hbox, TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX (hbox), select_route_button, TRUE, TRUE,
			    2);
	gtk_box_pack_start (GTK_BOX (hbox), create_route2_button, TRUE, TRUE,
			    2);
	gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), hbox_displays, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox), button3, TRUE, TRUE, 2);
	gtk_widget_set_sensitive (create_route_button, FALSE);

	if (route.active)
	{
		gtk_widget_set_sensitive (select_route_button, FALSE);
		gtk_clist_clear (GTK_CLIST (myroutelist));
		for (i = 0; i < route.items; i++)
		{
			(routelist + i)->dist =
				calcdist ((routelist + i)->lon,
					  (routelist + i)->lat);
			text[1] = (routelist + i)->name;
			g_snprintf (text0, sizeof (text0), "%d", i + 1);
			g_snprintf (text1, sizeof (text1), "%8.5f",
				    (routelist + i)->lat);
			g_snprintf (text2, sizeof (text2), "%8.5f",
				    (routelist + i)->lon);
			g_snprintf (text3, sizeof (text3), "%9.3f",
				    (routelist + i)->dist);
			text[0] = text0;
			text[2] = text1;
			text[3] = text2;
			text[4] = text3;
			j = gtk_clist_append (GTK_CLIST (myroutelist),
					      (gchar **) text);
			gtk_clist_set_foreground (GTK_CLIST (myroutelist), j,
						  &colors.black);
		}
	}
	else
		route.items = 0;
	tooltips = gtk_tooltips_new ();
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), create_route2_button,
			      _
			      ("Create a route from all waypoints. Sorted with order in file, not distance."),
			      NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), select_route_button,
			      _
			      ("Click here to start your journey. GpsDrive guides you through the waypoints in this list."),
			      NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), button,
			      _("Abort your journey"), NULL);

	gtk_widget_show_all (window);

	return TRUE;
}


/* *******************************************************
 * append selected poi to the end of the route
 */
void
add_poi_to_route (GtkTreeModel *model, GtkTreeIter iter)
{
	GtkTreeIter iter_route;
	GtkTreePath *path_route;
	
	GdkPixbuf *t_icon;
	gchar *t_name, *t_dist;
	gchar t_trip[15];
	gint t_id;
	gdouble t_lon, t_lat, t_distnum, last_lon, last_lat;

	route.items +=1;
	
	gtk_tree_model_get (model, &iter,
				RESULT_ID, &t_id,
				RESULT_TYPE_ICON, &t_icon,
				RESULT_NAME, &t_name,
				RESULT_LON, &t_lon,
				RESULT_LAT, &t_lat,
				RESULT_DISTANCE, &t_dist,
				RESULT_DIST_NUM, &t_distnum,
				-1);
	
	gtk_list_store_append (route_list_tree, &iter_route);

	/* calculate trip distance */
	if (route.items > 1)
	{
		path_route = gtk_tree_model_get_path (GTK_TREE_MODEL (route_list_tree), &iter_route);
		gtk_tree_path_prev (path_route);
		gtk_tree_model_get_iter (GTK_TREE_MODEL (route_list_tree), &iter_route, path_route);
		gtk_tree_model_get (GTK_TREE_MODEL (route_list_tree), &iter_route,
					ROUTE_LON, &last_lon,
					ROUTE_LAT, &last_lat,
					-1);
		route.distance += calc_wpdist (last_lon, last_lat, t_lon, t_lat, FALSE);
		gtk_tree_path_next (path_route);
		gtk_tree_model_get_iter (GTK_TREE_MODEL (route_list_tree), &iter_route, path_route);
	}
	else if (route.items == 1)
	{
		route.distance =+ calcdist (t_lon, t_lat);
	}
	g_snprintf (t_trip, sizeof (t_trip), "%9.3f", route.distance);	
		
	if (mydebug>25)
		fprintf (stderr, "add_poi_to_route: (%d)  ID: %d  |  NAME: %s |  LON: %f  |  LAT: %f  |  ICON: %p\n", route.items, t_id, t_name, t_lon, t_lat, t_icon);
	
	gtk_list_store_set (route_list_tree, &iter_route,
				ROUTE_ID, t_id,
				ROUTE_NUMBER, route.items,
				ROUTE_ICON, t_icon,
				ROUTE_NAME, t_name,
				ROUTE_DISTANCE, t_dist,
				ROUTE_TRIP, t_trip,
				ROUTE_LON, t_lon,
				ROUTE_LAT, t_lat,
				-1);
	
	g_object_unref (t_icon);
	g_free (t_name);
	g_free (t_dist);
	
}


/* *******************************************************
 * update route, if the current position has changed:
 */
void
update_route (void)
{
	/* check if current target is reached, and select next */
	if (route.active)
	{
		// TODO: add functionality...
	}
	
	/* redraw route display if enabled */
	if (route.items && route.show)
	{
		// TODO: add functionality...
	}
	
	/* recalculate trip /distance data if values are displayed */
	if (route.items && GTK_IS_WIDGET (route_window))
	{
		// TODO: add functionality...
	}
}


/* *******************************************************
 */
void
route_init (void)
{

	/* init gtk-list for storage of route data */
	route_list_tree = gtk_list_store_new (ROUTE_COLUMS,
				G_TYPE_INT,				/* ROUTE_ID */
				G_TYPE_INT,				/* ROUTE_NUMBER */
				GDK_TYPE_PIXBUF,	/* ROUTE_ICON */
				G_TYPE_STRING,		/* ROUTE_NAME */
				G_TYPE_STRING,		/* ROUTE_DISTANCE */
				G_TYPE_STRING,		/* ROUTE_TRIP */
				G_TYPE_DOUBLE,		/* ROUTE_LON */
				G_TYPE_DOUBLE		/* ROUTE_LAT */
				);
	
 	/* init route status data */
	route.active = FALSE;		/* routemode off */
	route.edit = FALSE;			/* route editmode off */
	route.items = 0;				/* route is empty/not available */
	route.distance = 0.0;		/* route length is 0 */
	route.pointer = 0;			/* reset next route target */
	route.show = TRUE;		/* default route display is on */
}
