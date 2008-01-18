/***********************************************************************

Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>
Website: www.gpsdrive.de

Copyright (c) 2007 Guenther Meyer <d.s.e@sordidmusic.com>

Copyright (c) 2007 Ross Scanlon <info@4x4falcon.co>
Website: www.4x4falcon.com/gpsdrive/


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

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "poi.h"
#include "gettext.h"
#include "icons.h"
#include "routes.h"
#include "gui.h"
#include "main_gui.h"
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
extern GtkWidget *destframe;
extern gint speechcount;
extern gchar oldangle[100];
extern gint saytarget;
extern wpstruct *wayp;
extern gint maxwp, maxfriends;
extern friendsstruct *friends, *fserver;
extern int sortcolumn, sortflag;
extern gint selected_wp_list_line;
extern GtkWidget *mylist;
extern gint onemousebutton;
extern gint dontsetwp;
extern gint usesql;
extern poi_struct *poi_list;
extern glong poi_list_count;
extern gdouble milesconv;	
extern color_struct colors;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern GdkGC *kontext_map;
extern GtkWidget *map_drawingarea;
extern GtkWidget *frame_statusbar;
extern poi_type_struct poi_type_list[poi_type_list_max];
extern GtkWidget *main_window;
extern GtkWidget *menuitem_saveroute;

GtkWidget *routewindow;
wpstruct *routelist;
GtkListStore *route_list_tree;
GtkWidget *myroutelist;
gint thisrouteline = 0;
GtkWidget *create_route_button, *create_route2_button, *select_route_button, *gotobt;
routestatus_struct route;
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
	//gchar str[200];
	gchar buf[1000], buf2[1000], *tn;


	if ( mydebug >50 ) fprintf(stderr , "setroutetarget()\n");

	if (datum != -1)
		route.pointer = datum;

	g_strlcpy (current.target, (routelist + route.pointer)->name,
		   sizeof (current.target));
	coords.target_lat = (routelist + route.pointer)->lat;
	coords.target_lon = (routelist + route.pointer)->lon;
//	g_snprintf (str, sizeof (str), "%s: %s[%d/%d]", _("To"), current.target,
//		    route.pointer + 1, route.items);
//	gtk_frame_set_label (GTK_FRAME (destframe), str);
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
	//gchar str[200];

	gtk_widget_destroy (GTK_WIDGET (routewindow));

//	g_snprintf (str, sizeof (str), "%s: %s", _("To"), current.target);
//	gtk_frame_set_label (GTK_FRAME (destframe), str);
	route.edit = FALSE;
	route.active = FALSE;
	route.pointer = route.items = 0;
	gtk_widget_set_sensitive (create_route_button, TRUE);
	gtk_widget_set_sensitive (menuitem_saveroute, FALSE);
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
	gtk_widget_set_sensitive (menuitem_saveroute, TRUE);

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
	gtk_widget_set_sensitive (menuitem_saveroute, TRUE);

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


/* *****************************************************************************
 */
void
route_next_target ()
{
  //gchar str[100];
  gchar buf[200];
  gdouble d;
  /*  test for new route point */
  if (strcmp (current.target, "     "))
    {
      if (route.active)
	d = calcdist ((routelist + route.pointer)->lon,
		      (routelist + route.pointer)->lat);
      else
	d = calcdist (coords.target_lon, coords.target_lat);

      if (d <= ROUTEREACH || route.forcenext)
	{
	  route.forcenext = FALSE;
	  if ((route.pointer != (route.items - 1)) && (route.active))
	    {
	      route.pointer++;

	      /* let's say the waypoint description */
	      saytargettext (local_config.wp_file, current.target);

	      setroutetarget (NULL, -1);
	    }
	  else
	    {
	      /*  route endpoint reached */
	      if (saytarget)
		{
		  g_snprintf (buf, sizeof (buf),
			      speech_target_reached[voicelang], current.target);
		  speech_out_speek (buf);

		  /* let's say the waypoint description */
		  saytargettext (local_config.wp_file, current.target);
		}
//		  g_snprintf (str, sizeof (str),
//		      "%s: %s", _("To"), current.target);
//		  gtk_frame_set_label (GTK_FRAME (destframe), str);
		  route.edit	= FALSE;
		  route.active = FALSE;
		  saytarget = FALSE;
		  route.pointer = route.items = 0;
	    }
	}
    }
}


// ***********************************************************************
// ***********************************************************************
// ***********************************************************************
// Here follows the new, POI-related route stuff...



/* *****************************************************************************
 * core function to add a new point to the current routes
 * used by several other functions
 */
void add_routepoint
	(glong t_id, gchar *t_name, gchar *t_cmt,
	 gchar *t_type, gdouble t_lon, gdouble t_lat)
{

	gchar t_dist[15], t_trip[15];
	gdouble last_lon, last_lat, t_dist_num;
	gint t_ptid;
	GdkPixbuf *t_icon;	
	GtkTreeIter iter_route;
	GtkTreePath *path_route;

	t_ptid = poi_type_id_from_name (t_type);
	t_icon = poi_type_list[t_ptid].icon;

	gtk_list_store_append (route_list_tree, &iter_route);

	route.items +=1;

	/* calculate route trip distance */
	if (route.items > 1)
	{
		path_route = gtk_tree_model_get_path (GTK_TREE_MODEL
			(route_list_tree), &iter_route);
		gtk_tree_path_prev (path_route);
		gtk_tree_model_get_iter (GTK_TREE_MODEL (route_list_tree),
			&iter_route, path_route);
		gtk_tree_model_get (GTK_TREE_MODEL (route_list_tree),
			&iter_route,
			ROUTE_LON, &last_lon, ROUTE_LAT, &last_lat, -1);
		route.distance += calc_wpdist (last_lon, last_lat,
			t_lon, t_lat, FALSE);
		gtk_tree_path_next (path_route);
		gtk_tree_model_get_iter (GTK_TREE_MODEL (route_list_tree),
			&iter_route, path_route);

		if (mydebug>25)
		{
			fprintf (stderr, "add_routepoint: Path: %s\n",
				gtk_tree_path_to_string (path_route));
		}
	}
	else if (route.items == 1)
	{
		route.distance =+ calcdist (t_lon, t_lat);
	}

	g_snprintf (t_trip, sizeof (t_trip), "%9.3f", route.distance);
	t_dist_num = calcdist (t_lon, t_lat);
	g_snprintf (t_dist, sizeof (t_dist), "%9.3f", t_dist_num);

	if (mydebug>25)
	{
		fprintf (stderr, "add_point_to_route: (%d)  ID: %ld  |"
			"  NAME: %s  |  LON: %f  |  LAT: %f  |  ICON: %p\n",
			route.items, t_id, t_name, t_lon, t_lat, t_icon);
	}

	gtk_list_store_set (route_list_tree, &iter_route,
		ROUTE_ID, t_id,
		ROUTE_NUMBER, route.items,
		ROUTE_ICON, t_icon,
		ROUTE_NAME, t_name,
		ROUTE_DISTANCE, t_dist,
		ROUTE_TRIP, t_trip,
		ROUTE_LON, t_lon,
		ROUTE_LAT, t_lat,
		ROUTE_CMT, t_cmt,
		ROUTE_TYPE, t_type,
		-1);

	current.needtosave = TRUE;
}


/* *****************************************************************************
 * create a new POI of type "waypoint.routepoint"
 * and append it to the current route
 */
void add_quickpoint_to_route ()
{
	gchar t_name[100], t_cmt[100];
	gdouble t_lat, t_lon;
	glong t_id;
	gint t_x, t_y;
	GdkModifierType state;

	g_snprintf (t_name, sizeof (t_name), "%s %d", _("Routepoint"), route.items+1);
	g_snprintf (t_cmt, sizeof (t_cmt), _("Quicksaved Routepoint"));
	gdk_window_get_pointer (GTK_LAYOUT (map_drawingarea)->bin_window, &t_x, &t_y, &state);
	calcxytopos (t_x, t_y, &t_lat, &t_lon, current.zoom);
	if ( mydebug > 0 )
		printf ("Add Routepoint: %s lat:%f,lon:%f (x:%d,y:%d)\n",
			t_name, t_lat, t_lon, t_x, t_y);

	t_id = addwaypoint (t_name,
		"waypoint.routepoint", t_cmt, t_lat, t_lon, TRUE);

	add_routepoint
		(t_id, t_name, t_cmt, "waypoint.routepoint", t_lon, t_lat);

	gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
		current.statusbar_id,
		_("Routepoint added."));

	gtk_widget_set_sensitive (menuitem_saveroute, TRUE);
}


/* *****************************************************************************
 * create a new POI of given type and append it to the current route
 * (used by gpx import)
 */
void add_arbitrary_point_to_route
	(gchar *name, gchar *cmt, gchar *type, gdouble lat, gdouble lon)
{
	gchar t_name[255], t_cmt[255], t_type[255];
	glong t_id;

	if (name)
		g_strlcpy (t_name, name, sizeof (t_name));
	else
		g_snprintf (t_name, sizeof (t_name), "%s %d", _("Routepoint"), route.items+1);

	if (cmt)
		g_strlcpy (t_cmt, cmt, sizeof (t_cmt));
	else
		g_snprintf (t_cmt, sizeof (t_cmt), _("Unnamed Routepoint"));

	if (type)
	{
		if (g_str_has_prefix (type, "waypoint.routepoint"))
			g_strlcpy (t_type, type, sizeof (t_type));
		else
			g_snprintf (t_type, sizeof (t_type), "waypoint.routepoint.%s", type);
	}
	else
		g_snprintf (t_type, sizeof (t_type), _("waypoint.routepoint"));

	if ( mydebug > 0 )
		printf ("Add Routepoint (%d): %s [%s], lat:%f, lon:%f\n",
			route.items+1, t_name, t_type, lat, lon);

	t_id = addwaypoint (t_name, t_type, t_cmt, lat, lon, TRUE);

	add_routepoint
		(t_id, t_name, t_cmt, t_type, lon, lat);
}



/* *****************************************************************************
 * save current route to gpx file
 */
gboolean route_export_cb (GtkWidget *widget, gboolean usedefault)
{
	const gchar foo[] =
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n"
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
		"xmlns=\"http://www.topografix.com/GPX/1/0\"\n"
		"xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0"
		" http://www.topografix.com/GPX/1/0/gpx.xsd\"\n";

	return TRUE;
}


/* ****************************************************************************
 * set target to the given route item
 */
void
route_settarget (gint rt_ptr)
{
	gchar t_rt_ptr[5];
	gchar *t_name;
	GtkTreeIter iter_route;

	if ( mydebug >50 )
		fprintf (stderr , "route_settarget: ");

	if (rt_ptr == -1)
	{
		g_snprintf (t_rt_ptr, sizeof (t_rt_ptr),
			"%d", (route.pointer));
	}
	else
	{
		g_snprintf (t_rt_ptr, sizeof (t_rt_ptr),
			"%d", (rt_ptr));
	}
	gtk_tree_model_get_iter_from_string
		(GTK_TREE_MODEL (route_list_tree), &iter_route, t_rt_ptr);
	gtk_tree_model_get
		(GTK_TREE_MODEL (route_list_tree), &iter_route,
		ROUTE_NAME, &t_name,
		ROUTE_LON, &(coords.target_lon),
		ROUTE_LAT, &(coords.target_lat),
		-1);
	g_snprintf (current.target, sizeof (current.target), t_name);

	if ( mydebug >50 )
	fprintf (stderr , "(%d/%d) %.6f / %.6f  -  %s\n",
		route.pointer, route.items, coords.target_lat,
		coords.target_lon, current.target);
	
	//TODO: do speech output, if enabled
/*	
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
*/


	g_free (t_name);
}


/* ****************************************************************************
 * display info for next target in main window while route is active
 */
void
route_display_targetinfo (void)
{
	GdkPixbuf *t_icon;
	gchar *t_name, *t_cmt;
	gchar t_text[500], t_dist[10], t_unit[10];
	gchar t_ptr[5];
	GtkTreeIter t_iter;

	if (!route.active)
		return;

	g_snprintf (t_ptr, sizeof (t_ptr), "%d", (route.pointer));
	gtk_tree_model_get_iter_from_string
		(GTK_TREE_MODEL (route_list_tree), &t_iter, t_ptr);
	gtk_tree_model_get
		(GTK_TREE_MODEL (route_list_tree), &t_iter,
		ROUTE_ICON, &t_icon,
		ROUTE_NAME, &t_name,
		ROUTE_CMT, &t_cmt,
		-1);

	distance2gchar (current.dist, t_dist, sizeof (t_dist), t_unit, sizeof (t_unit));
	if (t_cmt)
		g_snprintf (t_text, sizeof (t_text),
			"<span color=\"#ffff00\" font_desc=\"Sans 16\">"
			"<span background=\"#000000\">%s%s</span> %s</span>",
			t_dist, t_unit, t_cmt);
	else
		g_snprintf (t_text, sizeof (t_text),
			"<span color=\"#ffff00\" font_desc=\"Sans 16\">"
			"<span background=\"#000000\">%s%s</span> %s</span>",
			t_dist, t_unit, t_name);

	gtk_label_set_markup (GTK_LABEL (route.label), t_text);
	gtk_image_set_from_pixbuf (GTK_IMAGE (route.icon), t_icon);

	gtk_widget_set_size_request (route.label,
		gui_status.mapview_x - gdk_pixbuf_get_width (t_icon) - 20, -1);

	g_free (t_name);
	g_free (t_cmt);
	g_object_unref (t_icon);
}


/* ****************************************************************************
 * draw lines showing the route
 */
void
draw_route (void)
{
	GdkSegment *route_seg;

	gint destpos_x, destpos_y, curpos_x, curpos_y;
	gint i, j;
	gint t = 0;
	gchar t_routept[5];
	GtkTreeIter iter_route;
	gdouble t_lon, t_lat;
	
	if (route.items < 1)
		return;

	i = (route.items + 5);
	route_seg = g_new0 (GdkSegment, i);

	if (usesql)
	{
	/* poi mode */	
		g_snprintf (t_routept, sizeof (t_routept), "%d",
			(route.pointer));
		gtk_tree_model_get_iter_from_string
			(GTK_TREE_MODEL (route_list_tree),
			&iter_route, t_routept);

		calcxy (&curpos_x, &curpos_y,
			coords.current_lon, coords.current_lat, current.zoom);
		(route_seg)->x1 = curpos_x;
		(route_seg)->y1 = curpos_y;

		do
		{
			gtk_tree_model_get
				(GTK_TREE_MODEL (route_list_tree), &iter_route,
				ROUTE_LON, &t_lon, ROUTE_LAT, &t_lat, -1);
			if (t != 0)
			{
				(route_seg + t)->x1 = (route_seg + t - 1)->x2;
				(route_seg + t)->y1 = (route_seg + t - 1)->y2;
			}
			calcxy (&destpos_x, &destpos_y, t_lon, t_lat,
				current.zoom);
			(route_seg + t)->x2 = destpos_x;
			(route_seg + t)->y2 = destpos_y;
			t++;
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL
			(route_list_tree), &iter_route));
	}
	else
	{
	/* waypoints mode */
		/* start beginning with actual route.pointer */
		for (j = route.pointer; j < route.items; j++)
		{
			/* start drawing with current_pos */
			if (j == route.pointer)
			{
				calcxy (&curpos_x, &curpos_y,
					coords.current_lon,
					coords.current_lat, current.zoom);
				(route_seg + t)->x1 = curpos_x;
				(route_seg + t)->y1 = curpos_y;
			}
			else
			{
				(route_seg + t)->x1 = (route_seg + t - 1)->x2;
				(route_seg + t)->y1 = (route_seg + t - 1)->y2;
			}
			calcxy (&destpos_x, &destpos_y, (routelist +
				j)->lon, (routelist + j)->lat, current.zoom);
			(route_seg + t)->x2 = destpos_x;
			(route_seg + t)->y2 = destpos_y;
			t++;
		}
	}

	gdk_gc_set_foreground (kontext_map, &colors.route);
	gdk_gc_set_background (kontext_map, &colors.white);
	gdk_gc_set_line_attributes (kontext_map, 4, GDK_LINE_DOUBLE_DASH, 0, 0);
	gdk_gc_set_function (kontext_map, GDK_COPY);
	gdk_draw_segments (drawable, kontext_map, (GdkSegment *) route_seg, t);
	g_free (route_seg);
}


/* *******************************************************
 * append selected poi to the end of the route
 */
void
add_poi_to_route (GtkTreeModel *model, GtkTreeIter iter)
{
	gchar *t_name, *t_cmt, *t_type;
	gint t_id;
	gdouble t_lon, t_lat;

	
	/* get data from selected POI */
	gtk_tree_model_get (model, &iter,
		RESULT_ID, &t_id,
		RESULT_NAME, &t_name,
		RESULT_LON, &t_lon,
		RESULT_LAT, &t_lat,
		RESULT_COMMENT, &t_cmt,
		RESULT_TYPE_NAME, &t_type,
		-1);

	add_routepoint
		(t_id, t_name, t_cmt, t_type, t_lon, t_lat);

	g_free (t_name);
	g_free (t_cmt);
	g_free (t_type);

	gtk_widget_set_sensitive (menuitem_saveroute, TRUE);
}


/* *******************************************************
 * update route, if the current position has changed:
 */
void
update_route (void)
{
	gdouble d;
	
	/* in waypoint mode call the old function */
	if (!usesql)
	{
		route_next_target ();
		return;
	}
	
	/* check if current target is reached, and select next */
	if (route.active)
	{
		d = calcdist (coords.target_lon, coords.target_lat);

		if (d <= ROUTEREACH || route.forcenext)
		{
			route.forcenext = FALSE;
			if (route.pointer != (route.items - 1))
			{
				/* set target to next route item */
				route.pointer++;
				route_settarget (-1);
			}
			else
			{
				/* endpoint reached,  stop routing */
				route.active = FALSE;
				route.pointer = route.items = 0;
			}
		}
	}
	
	/* recalculate trip /distance data if values are displayed */
	if (route.items && GTK_IS_WIDGET (route_window))
	{
		// TODO: add functionality...
	}
}


/* *******************************************************
 * basic init for routing support
 */
void
route_init (gchar *name, gchar *desc, gchar *src)
{
	/* check, if route already exists, and free it */
	if (route_list_tree)
		gtk_list_store_clear (route_list_tree);

	/* remove old route data from database */
	cleanupsql_routedata ();

	/* init gtk-list for storage of route data */
	route_list_tree = gtk_list_store_new (ROUTE_COLUMS,
		G_TYPE_INT,		/* ROUTE_ID */
		G_TYPE_INT,		/* ROUTE_NUMBER */
		GDK_TYPE_PIXBUF,	/* ROUTE_ICON */
		G_TYPE_STRING,		/* ROUTE_NAME */
		G_TYPE_STRING,		/* ROUTE_DISTANCE */
		G_TYPE_STRING,		/* ROUTE_TRIP */
		G_TYPE_DOUBLE,		/* ROUTE_LON */
		G_TYPE_DOUBLE,		/* ROUTE_LAT */
		G_TYPE_STRING,		/* ROUTE_CMT */
		G_TYPE_STRING		/* ROUTE_TYPE */
		);

	/* init route status data */
	if (name)
		g_strlcpy (route.name, name, sizeof (route.name));
	else
		g_strlcpy (route.name, "Empty Route", sizeof (route.name));
	if (desc)
		g_strlcpy (route.desc, desc, sizeof (route.desc));
	else
		g_strlcpy (route.desc, "empty route", sizeof (route.desc));
	if (src)
		g_strlcpy (route.src, src, sizeof (route.src));
	else	
		g_snprintf (route.src, sizeof (route.src),
			"generated with GpsDrive %s", PACKAGE_VERSION);
	route.active = FALSE;		/* routemode off */
	route.edit = FALSE;		/* route editmode off */
	route.items = 0;		/* route is empty/not available */
	route.distance = 0.0;		/* route length is 0 */
	route.pointer = 0;		/* reset next route target */
	route.show = TRUE;		/* default route display is on */
	route.forcenext = FALSE;
}
