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
#include "poi.h"
#include "config.h"
#include "gettext.h"
#include "icons.h"
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
extern gint minsecmode;
extern gint maxwp, maxfriends;
extern friendsstruct *friends, *fserver;
extern long int maxfriendssecs;
extern int sortcolumn, sortflag;
extern gint selected_wp_list_line;
extern GtkWidget *mylist;
extern gint onemousebutton;
extern gint pdamode;
extern GtkWidget *mainwindow;
extern gint dontsetwp;
extern gint usesql;
extern poi_struct *poi_list;
extern glong poi_list_count;	

GtkWidget *routewindow;
wpstruct *routelist;
GtkWidget *myroutelist;
gint thisrouteline = 0, routeitems = 0, routepointer = 0;
gint createroute = FALSE;
gint routemode = FALSE;
gdouble routenearest = 9999999999.0;
GtkWidget *create_route_button, *create_route2_button, *select_route_button, *gotobt;
gint forcenextroutepoint = FALSE;
gint showroute = TRUE;


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
		routepointer = datum;

	routenearest = 9999999;
	g_strlcpy (targetname, (routelist + routepointer)->name,
		   sizeof (targetname));
	target_lat = (routelist + routepointer)->lat;
	target_lon = (routelist + routepointer)->lon;
	g_snprintf (str, sizeof (str), "%s: %s[%d/%d]", _("To"), targetname,
		    routepointer + 1, routeitems);
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
	createroute = FALSE;
	routemode = FALSE;
	routepointer = routeitems = 0;
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
	createroute = FALSE;
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
	createroute = FALSE;
	routemode = TRUE;
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

	coordinate2gchar(text1, sizeof(text1), (wayp+i)->lat, TRUE, minsecmode);
	coordinate2gchar(text2, sizeof(text2), (wayp+i)->lon, FALSE, minsecmode);
	g_snprintf (text3, sizeof (text3), "%9.3f", (wayp + i)->dist);
	text[0] = text0;
	text[2] = text1;
	text[3] = text2;
	text[4] = text3;
	j = gtk_clist_append (GTK_CLIST (myroutelist), (gchar **) text);
	gtk_clist_set_foreground (GTK_CLIST (myroutelist), j, &black);
	g_strlcpy ((routelist + routeitems)->name, (wayp + i)->name, 40);
	(routelist + routeitems)->lat = (wayp + i)->lat;
	(routelist + routeitems)->lon = (wayp + i)->lon;
	routeitems++;
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

		coordinate2gchar(text1, sizeof(text1), (wayp+i)->lat, TRUE, minsecmode);
		coordinate2gchar(text2, sizeof(text2), (wayp+i)->lon, FALSE, minsecmode);
		g_snprintf (text3, sizeof (text3), "%9.3f", (wayp + i)->dist);
		text[0] = text0;
		text[2] = text1;
		text[3] = text2;
		text[4] = text3;
		j = gtk_clist_append (GTK_CLIST (myroutelist),
				      (gchar **) text);
		gtk_clist_set_foreground (GTK_CLIST (myroutelist), j, &black);
		g_strlcpy ((routelist + routeitems)->name, (wayp + i)->name,
			   40);
		(routelist + routeitems)->lat = (wayp + i)->lat;
		(routelist + routeitems)->lon = (wayp + i)->lon;
		routeitems++;
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
				coordinate2gchar(text1, sizeof(text1), (wayp+i)->lat, TRUE, minsecmode);
				coordinate2gchar(text2, sizeof(text2), (wayp+i)->lon, FALSE, minsecmode);
				g_snprintf (text3, sizeof (text3), "%9.3f",
				          (wayp + i)->dist);
				text[0] = text0;
				text[3] = text1;
				text[4] = text2;
				text[5] = text3;
				j = gtk_clist_append (GTK_CLIST (mylist),
					      (gchar **) text);
				gtk_clist_set_foreground (GTK_CLIST (mylist), j,
						  &black);
			}
		}
	}
	
	for (i = 0; i < maxfriends; i++)
	{
		ti = time (NULL);
		tif = atol ((friends + i)->timesec);
		if ((ti - maxfriendssecs) > tif)
			continue;

		if (mobile)
			g_strlcpy (name, "", sizeof (name));
		else
			g_strlcpy (name, "*", sizeof (name));
		g_strlcat (name, (friends + i)->name, sizeof (name));
		g_snprintf (text0, sizeof (text0), "%d", i + maxwp + 1);
		coordinate_string2gdouble((friends + i)->lat, &la);
		coordinate_string2gdouble((friends + i)->lon, &lo);
		coordinate2gchar(text1, sizeof(text1), la, TRUE, minsecmode);
		coordinate2gchar(text2, sizeof(text2), lo, FALSE, minsecmode);

		if (!mobile)
		{
			text[0] = text0;
			text[1] = name;
			text[2] = (wayp + i)->typ;
			text[3] = text1;
			text[4] = text2;
			dist = calcdist (lo, la);
			g_snprintf (text3, sizeof (text3), "%9.3f", dist);
			text[4] = text3;
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
						  &black);
		else
			gtk_clist_set_foreground (GTK_CLIST (mylist), j,
						  &red);
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
	GtkWidget *scrwindow, *vbox, *button, *button3, *hbox, *hbox2, *l1;
	gint i, j;
	gchar *text[5], text0[20], text1[20], text2[20], text3[20];
	GtkTooltips *tooltips;

	createroute = TRUE;
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
	if (pdamode)
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
	hbox2 = gtk_hbox_new (TRUE, 2);
	if (!routemode)
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
	gtk_box_pack_start (GTK_BOX (vbox), hbox2, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox), button3, TRUE, TRUE, 2);
	gtk_widget_set_sensitive (create_route_button, FALSE);

	if (routemode)
	{
		gtk_widget_set_sensitive (select_route_button, FALSE);
		gtk_clist_clear (GTK_CLIST (myroutelist));
		for (i = 0; i < routeitems; i++)
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
						  &black);
		}
	}
	else
		routeitems = 0;
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
