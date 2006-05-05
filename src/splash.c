/*******************************************************************

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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <gpsdrive.h>
#include <poi.h>
#include <streets.h>
#include <time.h>
#include <speech_out.h>
#include <speech_strings.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern gint max_display_map;
extern map_dir_struct *display_map;

extern GtkWidget *mainwindow;
GtkWidget *splash_window;
extern gchar homedir[500], mapdir[500];
extern gint wpflag, trackflag, muteflag, displaymap_top, displaymap_map;
extern gint scaleprefered, milesflag, nauticflag, metricflag, sqlflag;
extern gint mydebug, scalewanted, savetrack, defaultserver;
extern gchar serialdev[80];
extern gdouble current_lon, current_lat, old_lon, old_lat, groundspeed;
extern gint setdefaultpos, shadow, etch;
extern gint do_draw_grid, streets_draw, tracks_draw, poi_draw, testgarmin;
extern gint needtosave, usedgps, simfollow;
extern gchar activewpfile[200];
extern gdouble milesconv;
extern gint satposmode, printoutsats, minsecmode, nightmode, cpuload;
/* extern gint flymode, vfr,disdevwarn; */
gint flymode = FALSE, vfr = 1, disdevwarn = TRUE;
extern gint pdamode;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu,
	int_padding, lastnotebook;
#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char dbtable[MAXDBNAME], dbname[MAXDBNAME];
extern char dbwherestring[5000];
extern char wp_typelist[100][40];
extern double dbdistance;
extern int dbusedist, needreminder;
extern gint earthmate, havefriends, zone;
extern gchar wplabelfont[100], bigfont[100];
extern char friendsserverip[20], friendsname[40], friendsidstring[40],
	friendsserverfqn[255];
extern long int maxfriendssecs;
extern gchar bluecolor[40], trackcolor[40], friendscolor[40];
extern int messagenumber;
extern int sockfd, serialspeed, disableserial, showsid, storetz;
extern int sound_direction, sound_distance, sound_speed, sound_gps;

#define KM2MILES 0.62137119
#define PADDING int_padding


gint
splashaway_cb (GtkWidget * widget, gpointer datum)
{


	gtk_widget_destroy (splash_window);

	return (FALSE);
}


static void
create_tags (GtkTextBuffer * buffer)
{
	gtk_text_buffer_create_tag (buffer, "heading",
				    "weight", PANGO_WEIGHT_BOLD,
				    "size", 12 * PANGO_SCALE, NULL);

	gtk_text_buffer_create_tag (buffer, "italic",
				    "style", PANGO_STYLE_ITALIC, NULL);

	gtk_text_buffer_create_tag (buffer, "bold",
				    "weight", PANGO_WEIGHT_BOLD, NULL);

	gtk_text_buffer_create_tag (buffer, "big",
				    /* points times the PANGO_SCALE factor */
				    "size", 20 * PANGO_SCALE, NULL);


	gtk_text_buffer_create_tag (buffer, "blue_foreground",
				    "foreground", "blue", NULL);

	gtk_text_buffer_create_tag (buffer, "red_foreground",
				    "foreground", "red", NULL);

	gtk_text_buffer_create_tag (buffer, "not_editable",
				    "editable", FALSE, NULL);

	gtk_text_buffer_create_tag (buffer, "word_wrap",
				    "wrap_mode", GTK_WRAP_WORD, NULL);

	gtk_text_buffer_create_tag (buffer, "center",
				    "justification", GTK_JUSTIFY_CENTER,
				    NULL);

	gtk_text_buffer_create_tag (buffer, "underline",
				    "underline", PANGO_UNDERLINE_SINGLE,
				    NULL);

}

static void
insert_text (GtkTextBuffer * buffer)
{
	GtkTextIter iter;
	GtkTextIter start, end;
	gchar *t1 =
		_
		("Left mouse button         : Set position (usefull in simulation mode)\n"
		 "Right mouse button        : Set target directly on the map\n"
		 "Middle mouse button       : Display position again\n"
		 "Shift left mouse button   : smaller map\n"
		 "Shift right mouse button  : larger map\n"
		 "Control left mouse button : Set a waypoint (mouse position) on the map\n"
		 "Control right mouse button: Set a waypoint at current position on the map\n\n");

	gchar *t2 = _("j    : switch to next waypoint in route mode\n"
		      "x    : add waypoint at current position\n"
		      "y    : add waypoint at mouse cursor position\n"
		      "n    : switch on light for 60sec in nightmode\n"
		      "g    : Toggle grid\n"
		      "f    : Toggle friends display\n"
		      "w    : Set Waypoint at current location without asking\n"
		      "p    : Set Waypoint at current cursor position without asking\n"
		      "+    : Zoom in \n" "-    : Zoom out\n");
	gchar *t3 =
		_("Press the underlined key together with the ALT-key.\n\n"
		  "You can move on the map by selecting the Position-Mode"
		  " in the menu. A blue rectangle shows this mode, you can set this cursor by clicking on the map."
		  " If you click on the border of the map (the outer 20%) then the map switches to the next area.\n\n"
		  "Suggestions welcome.\n\n");

	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);


	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("GpsDrive v"), -1,
						  "heading", NULL);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, VERSION, -1,
						  "heading", NULL);
	gtk_text_buffer_insert (buffer, &iter,
				_
				("\n\nYou find new versions on http://www.gpsdrive.cc\n"),
				-1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _
						  ("Disclaimer: Please do not use for navigation. \n\n"),
						  -1, "red_foreground", NULL);

	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _
						  ("Please have a look into the manpage (man gpsdrive) for program details!"),
						  -1, "italic", NULL);
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _
						  ("Mouse control (clicking on the map):\n"),
						  -1, "blue_foreground",
						  NULL);

	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
	gtk_text_buffer_insert (buffer, &iter, t1, -1);

	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("Short cuts:\n"), -1,
						  "blue_foreground", NULL);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
	gtk_text_buffer_insert (buffer, &iter, t2, -1);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);

	gtk_text_buffer_insert (buffer, &iter,
				_("The other key shortcuts are marked as "),
				-1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("underlined"), -1,
						  "underline", NULL);
	gtk_text_buffer_insert (buffer, &iter,
				_(" letters in the button text.\n"), -1);
	gtk_text_buffer_insert (buffer, &iter, t3, -1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("Have a lot of fun!"), -1,
						  "big", NULL);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);


	/* Apply word_wrap tag to whole buffer */
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_apply_tag_by_name (buffer, "not_editable", &start,
					   &end);
	gtk_text_buffer_apply_tag_by_name (buffer, "word_wrap", &start, &end);


}


gint
help_cb (GtkWidget * widget, guint datum)
{
	static GtkWidget *window = NULL;
	GtkWidget *vpaned, *knopf;
	GtkWidget *view1;
	GtkWidget *sw, *vbox;
	/*   GtkTextBuffer *buffer;  */
	GtkTextBuffer *buffer;



	window = gtk_dialog_new ();
	gtk_window_set_default_size (GTK_WINDOW (window), 580, 570);
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	g_signal_connect (window, "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &window);

	gtk_window_set_title (GTK_WINDOW (window), "GpsDrive Help");
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);

	vpaned = gtk_vpaned_new ();
	gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);

	vbox = gtk_vbox_new (FALSE, 3);
	/*   gtk_container_add (GTK_CONTAINER (window), vbox); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX (vbox), vpaned, TRUE, TRUE, 3);

	/* For convenience, we just use the autocreated buffer from
	 * the first text view; you could also create the buffer
	 * by itself with gtk_text_buffer_new(), then later create
	 * a view widget.
	 */
	view1 = gtk_text_view_new ();
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view1));
	/*       view2 = gtk_text_view_new_with_buffer (buffer); */


	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_paned_add1 (GTK_PANED (vpaned), sw);

	gtk_container_add (GTK_CONTAINER (sw), view1);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	/*       gtk_paned_add2 (GTK_PANED (vpaned), sw); */

	/*       gtk_container_add (GTK_CONTAINER (sw), view2); */


	create_tags (buffer);
	insert_text (buffer);



	knopf = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
				   GTK_SIGNAL_FUNC
				   (gtk_widget_destroy), GTK_OBJECT (window));
	/*   gtk_box_pack_start (GTK_BOX (vbox), knopf, FALSE, FALSE, 5); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    knopf, TRUE, TRUE, 2);


	gtk_widget_show_all (vpaned);

	gtk_widget_show_all (window);

	return TRUE;

}


static void
reminder_create_tags (GtkTextBuffer * buffer)
{

	gtk_text_buffer_create_tag (buffer, "heading",
				    "weight", PANGO_WEIGHT_BOLD,
				    "size", 12 * PANGO_SCALE, NULL);

	gtk_text_buffer_create_tag (buffer, "bold",
				    "weight", PANGO_WEIGHT_BOLD, NULL);

	gtk_text_buffer_create_tag (buffer, "not_editable",
				    "editable", FALSE, NULL);

	gtk_text_buffer_create_tag (buffer, "word_wrap",
				    "wrap_mode", GTK_WRAP_WORD, NULL);

}

static void
reminder_insert_text (GtkTextBuffer * buffer)
{
	GtkTextIter iter;
	GtkTextIter start, end;

	/* demo_find_file() looks in the the current directory first,
	 * so you can run gtk-demo without installing GTK, then looks
	 * in the location where the file is installed.
	 */
	/* get start of buffer; each insertion will revalidate the
	 * iterator to point to just after the inserted text.
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);


	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _
						  ("Please donate to GpsDrive"),
						  -1, "heading", NULL);


	gtk_text_buffer_insert (buffer, &iter,
				_
				("\n\nGpsDrive is a project with no comercial background. \n\n"
				 "It would be nice if you can give a donation to help me pay the costs for hardware and the webserver.\n\nTo do so, just go to"),
				-1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _
						  (" http://www.gpsdrive.cc "),
						  -1, "bold", NULL);
	gtk_text_buffer_insert (buffer, &iter,
				_("and click on the PayPal button.\n\n"
				  "Thank you very much for your donation!\n\n"
				  "This message is only displayed once when you start an new version of GpsDrive.\n\n"),
				-1);

	/* Apply word_wrap tag to whole buffer */
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_apply_tag_by_name (buffer, "not_editable", &start,
					   &end);
	gtk_text_buffer_apply_tag_by_name (buffer, "word_wrap", &start, &end);


}


gint
reminder_cb (GtkWidget * widget, guint datum)
{
	static GtkWidget *window = NULL;
	GtkWidget *vpaned, *knopf2;
	GtkWidget *view1;
	GtkWidget *sw;
	/*   GtkTextBuffer *buffer;  */
	GtkTextBuffer *buffer;



	window = gtk_dialog_new ();
	gtk_window_set_default_size (GTK_WINDOW (window), 380, 380);
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	g_signal_connect (window, "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &window);

	gtk_window_set_title (GTK_WINDOW (window),
			      _("About GpsDrive donation"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);

	vpaned = gtk_vpaned_new ();
	gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);

	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vpaned, TRUE, TRUE,
			    2);

	/* For convenience, we just use the autocreated buffer from
	 * the first text view; you could also create the buffer
	 * by itself with gtk_text_buffer_new(), then later create
	 * a view widget.
	 */
	view1 = gtk_text_view_new ();
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view1));
	/*       view2 = gtk_text_view_new_with_buffer (buffer); */


	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_paned_add1 (GTK_PANED (vpaned), sw);

	gtk_container_add (GTK_CONTAINER (sw), view1);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	/*       gtk_paned_add2 (GTK_PANED (vpaned), sw); */

	/*       gtk_container_add (GTK_CONTAINER (sw), view2); */


	reminder_create_tags (buffer);
	reminder_insert_text (buffer);


	/*       attach_widgets (GTK_TEXT_VIEW (view1)); */
	knopf2 = gtk_button_new_from_stock (GTK_STOCK_OK);
	gtk_signal_connect_object (GTK_OBJECT (knopf2), "clicked",
				   GTK_SIGNAL_FUNC
				   (gtk_widget_destroy), GTK_OBJECT (window));

	/*   gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), knopf2); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    knopf2, TRUE, TRUE, 2);


	gtk_widget_show_all (vpaned);

	gtk_widget_show_all (window);

	return TRUE;

}

gint
message_cb (char *msgid, char *name, char *text, int fs)
{
	static GtkWidget *window = NULL;
	GtkWidget *vpaned, *knopf2;
	GtkWidget *view1;
	GtkWidget *sw, *knopf, *vbox;
	/*   GtkTextBuffer *buffer;  */
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	GtkTextIter start, end;
	gchar titlestr[60];
	gchar buf[MAXMESG];

	window = gtk_dialog_new ();
	gtk_window_set_default_size (GTK_WINDOW (window), 320, 240);
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	g_signal_connect (window, "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &window);

	g_snprintf (titlestr, sizeof (titlestr), "%s %s", _("From:"), name);
	gtk_window_set_title (GTK_WINDOW (window), titlestr);
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);

	vpaned = gtk_vpaned_new ();
	gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);

	vbox = gtk_vbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);


	gtk_box_pack_start (GTK_BOX (vbox), vpaned, TRUE, TRUE, 3);


	knopf = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
				   GTK_SIGNAL_FUNC
				   (gtk_widget_destroy), GTK_OBJECT (window));
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    knopf, TRUE, TRUE, 2);


	/* For convenience, we just use the autocreated buffer from
	 * the first text view; you could also create the buffer
	 * by itself with gtk_text_buffer_new(), then later create
	 * a view widget.
	 */
	view1 = gtk_text_view_new ();
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view1));
	/*       view2 = gtk_text_view_new_with_buffer (buffer); */


	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_paned_add1 (GTK_PANED (vpaned), sw);

	gtk_container_add (GTK_CONTAINER (sw), view1);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	/*   reminder_create_tags (buffer); */
	/*   reminder_insert_text (buffer); */
	/*       attach_widgets (GTK_TEXT_VIEW (view1)); */

	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
	gtk_text_buffer_create_tag (buffer, "word_wrap",
				    "wrap_mode", GTK_WRAP_WORD, NULL);
	gtk_text_buffer_create_tag (buffer, "heading",
				    "weight", PANGO_WEIGHT_BOLD,
				    "size", 10 * PANGO_SCALE, NULL);

	gtk_text_buffer_create_tag (buffer, "blue_foreground",
				    "foreground", "blue", "weight",
				    PANGO_WEIGHT_BOLD, "size",
				    10 * PANGO_SCALE, NULL);
	gtk_text_buffer_create_tag (buffer, "red_foreground", "foreground",
				    "red", "weight", PANGO_WEIGHT_BOLD,
				    "size", 10 * PANGO_SCALE, NULL);
	gtk_text_buffer_create_tag (buffer, "center", "justification",
				    GTK_JUSTIFY_CENTER, NULL);
	if (fs)
	{
		g_snprintf (buf, sizeof (buf),
			    _
			    ("You received a message from\nthe friends server (%s)\n"),
			    name);
		gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, buf,
							  -1, "heading",
							  "center",
							  "red_foreground",
							  NULL);
	}
	else
	{
		gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
							  _
							  ("You received a message through the friends server from:\n"),
							  -1, "heading",
							  "center", NULL);
		gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, name,
							  -1,
							  "blue_foreground",
							  "center", NULL);
	}
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _
						  ("Message text:\n"),
						  -1, "heading", NULL);
	gtk_text_buffer_insert (buffer, &iter, text, -1);
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_apply_tag_by_name (buffer, "word_wrap", &start, &end);
	/*   gtk_text_iter_can_insert (&iter, FALSE); */

	knopf2 = gtk_button_new_from_stock (GTK_STOCK_OK);

	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw),
					       knopf2);


	gtk_widget_show_all (vpaned);

	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	gtk_widget_show_all (window);
	g_snprintf (buf, sizeof (buf), "ACK: %s", msgid);
	if ( mydebug > 10 )
		fprintf (stderr, "\nsending to %s:\n%s\n", friendsserverip,
			 buf);
	sockfd = -1;
	friends_sendmsg (friendsserverip, buf);
	gdk_beep ();

  g_snprintf( buf, sizeof(buf), speech_message_received[voicelang], name );
	speech_out_speek (buf);

	return TRUE;
}


/*  write the configurationfile ~/.gpsdrive/gpsdriverc */
void
writeconfig ()
{
	FILE *fp;
	gchar fname[220], str[40];
	gint i;

	g_strlcpy (fname, homedir, sizeof (fname));
	g_strlcat (fname, "gpsdriverc", sizeof (fname));

	if ( mydebug > 0 )
		printf ("Write config %s\n", fname);



	fp = fopen (fname, "w");
	if (fp == NULL)
	{
		perror (_("Error saving config file ~/.gpsdrive/gpsdriverc"));
		return;
	}

	fprintf (fp, "showwaypoints = ");
	if (wpflag)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "showtrack = ");
	if (trackflag)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "mutespeechoutput = ");
	if (muteflag)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "showtopomaps = ");
	if (displaymap_top)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "showstreetmaps = ");
	if (displaymap_map)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "autobestmap = ");
	if (!scaleprefered)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "units = ");
	if (milesflag)
		fprintf (fp, "miles\n");
	else
	{
		if (metricflag)
			fprintf (fp, "metric\n");
		else
			fprintf (fp, "nautic\n");
	}

	fprintf (fp, "savetrack = ");
	if (savetrack)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "scalewanted = %d\n", scalewanted);

	fprintf (fp, "serialdevice = %s\n", serialdev);

	g_snprintf (str, sizeof (str), "%.6f", current_lon);
	g_strdelimit (str, ",", '.');
	fprintf (fp, "lastlong = %s\n", str);

	g_snprintf (str, sizeof (str), "%.6f", current_lat);
	g_strdelimit (str, ",", '.');
	fprintf (fp, "lastlat = %s\n", str);

	fprintf (fp, "shadow = ");
	if (shadow)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "defaultserver = ");
	if (defaultserver == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "testgarminmode = ");
	if (testgarmin == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "waypointfile = ");
	fprintf (fp, "%s\n", activewpfile);

	fprintf (fp, "usedgps = ");
	if (usedgps == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "mapdir = ");
	fprintf (fp, "%s\n", mapdir);

	fprintf (fp, "simfollow = ");
	if (simfollow == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "satposmode = ");
	if (satposmode == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "printoutsats = ");
	if (printoutsats == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "minsecmode = %d\n",minsecmode);

	fprintf (fp, "nightmode = ");
	fprintf (fp, "%d\n", nightmode);

	fprintf (fp, "cpuload = %d\n", cpuload);

	fprintf (fp, "flymode = ");
	if (flymode == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "vfr = ");
	fprintf (fp, "%d\n", vfr);

	fprintf (fp, "disdevwarn = ");
	if (disdevwarn == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "sqlflag = ");
	if (sqlflag == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");
	fprintf (fp, "lastnotebook = ");
	fprintf (fp, "%d\n", lastnotebook);

	fprintf (fp, "dbhostname = %s\n", dbhost);
	fprintf (fp, "dbname = %s\n", dbname);
	fprintf (fp, "dbuser = %s\n", dbuser);
	fprintf (fp, "dbpass = %s\n", dbpass);
	fprintf (fp, "dbtable = %s\n", dbtable);
	fprintf (fp, "dbdistance = %.1f\n", dbdistance);
	fprintf (fp, "dbusedist = %d\n", dbusedist);
	fprintf (fp, "dbwherestring = %s\n", dbwherestring);
	fprintf (fp, "earthmate = %d\n", earthmate);
	fprintf (fp, "wplabelfont = %s\n", wplabelfont);
	fprintf (fp, "bigfont = %s\n", bigfont);
	fprintf (fp, "friendsserverip = %s\n", friendsserverip);
	fprintf (fp, "friendsserverfqn = %s\n", friendsserverfqn);
	fprintf (fp, "friendsname = %s\n", friendsname);
	fprintf (fp, "friendsidstring = %s\n", friendsidstring);
	fprintf (fp, "usefriendsserver = %d\n", havefriends);
	fprintf (fp, "maxfriendssecs = %ld\n", maxfriendssecs);
	fprintf (fp, "reminder = %s\n", VERSION);
	fprintf (fp, "storetz = %d\n", storetz);
	if (storetz)
		fprintf (fp, "timezone = %d\n", zone);
	fprintf (fp, "etch = %d\n", etch);
	fprintf (fp, "bigcolor = %s\n", bluecolor);
	fprintf (fp, "trackcolor = %s\n", trackcolor);
	fprintf (fp, "friendscolor = %s\n", friendscolor);
	fprintf (fp, "messagenumber = %d\n", messagenumber);
	fprintf (fp, "serialspeed = %d\n", serialspeed);
	fprintf (fp, "disableserial = %d\n", disableserial);
	fprintf (fp, "showssid = %d\n", showsid);
	fprintf (fp, "sound_direction = %d\n", sound_direction);
	fprintf (fp, "sound_distance = %d\n", sound_distance);
	fprintf (fp, "sound_speed = %d\n", sound_speed);
	fprintf (fp, "sound_gps = %d\n", sound_gps);

	fprintf (fp, "draw_grid = %d\n", do_draw_grid);
	fprintf (fp, "draw_streets = %d\n", streets_draw);
	fprintf (fp, "draw_poi = %d\n", poi_draw);
	fprintf (fp, "draw_tracks = %d\n", tracks_draw);
	for ( i = 0; i < max_display_map; i++)
	    {
		fprintf (fp, "display_map_%s = %d\n",
			 display_map[i].name,
			 display_map[i].to_be_displayed);
	    }
	


	fclose (fp);
	needtosave = FALSE;
}

/*  read the configurationfile ~/.gpsdrive/gpsdriverc */
void
readconfig ()
{
    FILE *fp;
    gchar fname[220], par1[40], par2[1000], buf[1000];
    gint e;

    g_strlcpy (fname, homedir, sizeof (fname));
    g_strlcat (fname, "gpsdriverc", sizeof (fname));
    fp = fopen (fname, "r");
    if (fp == NULL)
	return;

    // mydebug is not set, because getopt was not run yet
    // So you won't see this Debug Output
    if ( mydebug > 0 )
	fprintf (stderr,"reading config file %s ...\n",fname);

    while ((fgets (buf, 1000, fp)) > 0)
	{
	    g_strlcpy (par1, "", sizeof (par1));
	    g_strlcpy (par2, "", sizeof (par2));
	    e = sscanf (buf, "%s = %[^\n]", par1, par2);

	    if ( mydebug > 1 )
		fprintf ( stderr,"%d [%s] = [%s]\n", e, par1, par2);

	    if (e == 2)
		{
		    if ( (strcmp(par1, "showwaypoints")) == 0)
			wpflag = atoi (par2);
		    else if ( (strcmp(par1, "showtrack")) == 0)
			trackflag = atoi (par2);
		    else if ( (strcmp(par1, "mutespeechoutput")) == 0)
			muteflag = atoi (par2);
		    else if ( (strcmp(par1, "showtopomaps")) == 0)
			displaymap_top = atoi (par2);
		    else if ( (strcmp(par1, "showstreetmaps")) == 0)
			displaymap_map = atoi (par2);
		    /*  To set the right sensitive flags bestmap_cb is called later */
		    else if ( (strcmp(par1, "autobestmap")) == 0)
			scaleprefered = !(atoi (par2));
		    else if ( (strcmp(par1, "units")) == 0)
			{
			    milesflag = metricflag = nauticflag = FALSE;
			    if ( (strcmp(par2, "miles")) == 0)
				{
				    milesflag = TRUE;
				    milesconv = KM2MILES;
				}
			    else
				{
				    if ( (strcmp(par2, "metric")) == 0)
					{
					    metricflag = TRUE;
					    milesconv = 1.0;
					}
				    else if ( (strcmp(par2, "nautic")) ==
					     0)
					{
					    nauticflag = TRUE;
					    milesconv = KM2NAUTIC;
					}
				}
			}
		    else if ( (strcmp(par1, "savetrack")) == 0)
			savetrack = atoi (par2);
		    else if ( (strcmp(par1, "scalewanted")) == 0)
			scalewanted = atoi (par2);
		    else if ( (strcmp(par1, "serialdevice")) == 0)
			g_strlcpy (serialdev, par2,
				   sizeof (serialdev));
		    else if ( (strcmp(par1, "lastlong")) == 0)
			coordinate_string2gdouble(par2, &current_lon);
		    else if ( (strcmp(par1, "lastlat")) == 0)
			coordinate_string2gdouble(par2, &current_lat);
		    /* 
		       else if ( (strcmp(par1, "setdefaultpos")) == 0)            
		       setdefaultpos = atoi (par2); 
		    */
		    else if ( (strcmp(par1, "shadow")) == 0)
			shadow = atoi (par2);
		    else if ( (strcmp(par1, "defaultserver")) == 0)
			defaultserver = atoi (par2);
		    else if ( (strcmp(par1, "waypointfile")) == 0)
			g_strlcpy (activewpfile, par2,
				   sizeof (activewpfile));
		    else if ( (strcmp(par1, "testgarminmode")) == 0)
			testgarmin = atoi (par2);
		    else if ( (strcmp(par1, "usedgps")) == 0)
			usedgps = atoi (par2);
		    else if ( (strcmp(par1, "mapdir")) == 0)
			g_strlcpy (mapdir, par2, sizeof (mapdir));
		    else if ( (strcmp(par1, "simfollow")) == 0)
			simfollow = atoi (par2);
		    else if ( (strcmp(par1, "satposmode")) == 0)
			satposmode = atoi (par2);
		    else if ( (strcmp(par1, "printoutsats")) == 0)
			printoutsats = atoi (par2);
		    else if ( (strcmp(par1, "minsecmode")) == 0)
			minsecmode = atoi (par2);
		    else if ( (strcmp(par1, "nightmode")) == 0)
			nightmode = atoi (par2);
		    else if ( (strcmp(par1, "cpuload")) == 0)
			cpuload = atoi (par2);
		    else if ( (strcmp(par1, "flymode")) == 0)
			flymode = atoi (par2);
		    else if ( (strcmp(par1, "vfr")) == 0)
			vfr = atoi (par2);
		    else if ( (strcmp(par1, "disdevwarn")) == 0)
			disdevwarn = atoi (par2);
		    else if ( (strcmp(par1, "sqlflag")) == 0)
			sqlflag = atoi (par2);
		    else if ( (strcmp(par1, "lastnotebook")) == 0)
			lastnotebook = atoi (par2);
		    else if ( (strcmp(par1, "dbhostname")) == 0)
			{
			    g_strlcpy (dbhost, par2, sizeof (dbhost));
			    g_strstrip (dbhost);
			}
		    else if ( (strcmp(par1, "dbname")) == 0)
			g_strlcpy (dbname, par2, sizeof (dbname));
		    else if ( (strcmp(par1, "dbuser")) == 0)
			g_strlcpy (dbuser, par2, sizeof (dbuser));
		    else if ( (strcmp(par1, "dbpass")) == 0)
			g_strlcpy (dbpass, par2, sizeof (dbpass));
		    else if ( (strcmp(par1, "dbtable")) == 0)
			g_strlcpy (dbtable, par2, sizeof (dbtable));
		    else if ( (strcmp(par1, "dbname")) == 0)
			g_strlcpy (dbname, par2, sizeof (dbname));
		    else if ( (strcmp(par1, "dbdistance")) == 0)
			dbdistance = g_strtod (par2, 0);
		    else if ( (strcmp(par1, "dbusedist")) == 0)
			dbusedist = atoi (par2);
		    else if ( (strcmp(par1, "dbwherestring")) == 0)
			g_strlcpy (dbwherestring, par2,
				   sizeof (dbwherestring));
		    else if ( (strcmp(par1, "earthmate")) == 0)
			earthmate = atoi (par2);
		    else if ( (strcmp(par1, "wplabelfont")) == 0)
			g_strlcpy (wplabelfont, par2,
				   sizeof (wplabelfont));
		    else if ( (strcmp(par1, "bigfont")) == 0)
			g_strlcpy (bigfont, par2, sizeof (bigfont));
		    else if ( (strcmp(par1, "friendsserverip")) == 0)
			g_strlcpy (friendsserverip, par2,
				   sizeof (friendsserverip));
		    else if ( (strcmp(par1, "friendsserverfqn")) == 0)
			g_strlcpy (friendsserverfqn, par2,
				   sizeof (friendsserverfqn));
		    else if ( (strcmp(par1, "friendsname")) == 0)
			g_strlcpy (friendsname, par2,
				   sizeof (friendsname));
		    else if ( (strcmp(par1, "friendsidstring")) == 0)
			g_strlcpy (friendsidstring, par2,
				   sizeof (friendsidstring));
		    else if ( (strcmp(par1, "usefriendsserver")) == 0)
			havefriends = atoi (par2);
		    else if ( (strcmp(par1, "maxfriendssecs")) == 0)
			maxfriendssecs = atoi (par2);
		    else if (
			     ( (strcmp(par1, "reminder")) == 0 ) &&
			     ( (strcmp(par2, VERSION)) == 0) )
			{
			    needreminder = FALSE;
			    /*                fprintf(stderr,"needreminder false\n"); */
			}
		    else if ( (strcmp(par1, "storetz")) == 0)
			storetz = atoi (par2);
		    else if ( storetz &&
			      (strcmp(par1, "timezone")) == 0)
			zone = atoi (par2);
		    else if ( (strcmp(par1, "etch")) == 0)
			etch = atoi (par2);
		    else if ( (strcmp(par1, "bigcolor")) == 0)
			g_strlcpy (bluecolor, par2,
				   sizeof (bluecolor));
		    else if ( (strcmp(par1, "trackcolor")) == 0)
			g_strlcpy (trackcolor, par2,
				   sizeof (trackcolor));
		    else if ( (strcmp(par1, "friendscolor")) == 0)
			g_strlcpy (friendscolor, par2, sizeof (friendscolor));
		    else if ( (strcmp(par1, "messagenumber")) == 0)
			messagenumber = atoi (par2);
		    else if ( (strcmp(par1, "serialspeed")) == 0)
			serialspeed = atoi (par2);
		    else if ( (strcmp(par1, "disableserial")) == 0)
			disableserial = atoi (par2);
		    else if ( (strcmp(par1, "showssid")) == 0)
			showsid = atoi (par2);
		    else if ( (strcmp(par1, "sound_direction")) == 0)
			sound_direction = atoi (par2);
		    else if ( (strcmp(par1, "sound_distance")) == 0)
			sound_distance = atoi (par2);
		    else if ( (strcmp(par1, "sound_speed")) == 0)
			sound_speed = atoi (par2);
		    else if ( (strcmp(par1, "sound_gps")) == 0)
			sound_gps = atoi (par2);
		    else if ( (strcmp(par1, "draw_grid")) == 0)
			do_draw_grid = atoi (par2);
		    else if ( (strcmp(par1, "draw_streets")) == 0)
			streets_draw = atoi (par2);
		    else if ( (strcmp(par1, "draw_poi")) == 0)
			poi_draw = atoi (par2);
		    else if ( (strcmp(par1, "draw_tracks")) == 0)
			tracks_draw = atoi (par2);
		    else if ( ! strncmp(par1, "display_map_",12) )
			{
			    // printf ("display_map: %s %s\n",par1,par2);
			    max_display_map += 1;
			    display_map = g_renew(map_dir_struct, display_map, max_display_map);
			    g_strlcpy (display_map[max_display_map-1].name,
				       (par1+12),
				       sizeof (display_map[max_display_map-1].name));
			    display_map[max_display_map-1].to_be_displayed = atoi (par2);
			}
		    else
			fprintf (stderr, "ERROR: Unknown Config Parameter '%s=%s'\n",par1,par2);
		}		/* if e==2 */
	}

	if ( mydebug > 1 )
		fprintf ( stderr,"\nreading config file finished\n");
	fclose (fp);
}


GtkWidget *
getPixmapFromFile (GtkWidget * widget, const gchar * filename)
{
	GtkWidget *pixmap;
	GdkColormap *colormap;
	GdkPixmap *gdkpixmap;
	GdkBitmap *mask;

	if (!filename || !filename[0])
		return NULL;

	colormap = gtk_widget_get_colormap (widget);
	gdkpixmap =
		gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask,
						     NULL, filename);
	if (gdkpixmap == NULL)
	{
		printf ("**** couldn't create pixmap from file: '%s'\n",
			filename);
		return NULL;
	}

	pixmap = gtk_pixmap_new (gdkpixmap, mask);
	gdk_pixmap_unref (gdkpixmap);
	gdk_bitmap_unref (mask);
	return pixmap;
}


GtkWidget *
getPixmapFromXpm (GtkWidget * widget, gchar ** xpmname)
{
	GtkWidget *pixmap;
	GdkColormap *colormap;
	GdkPixmap *gdkpixmap;
	GdkBitmap *mask;

	if (!xpmname || !xpmname[0])
		return NULL;

	colormap = gtk_widget_get_colormap (widget);
	gdkpixmap =
		gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask,
						       NULL, xpmname);
	if (gdkpixmap == NULL)
	{
		printf ("**** couldn't create pixmap from xpm: '%s'\n",
			*xpmname);
		return NULL;
	}

	pixmap = gtk_pixmap_new (gdkpixmap, mask);
	gdk_pixmap_unref (gdkpixmap);
	gdk_bitmap_unref (mask);
	return pixmap;
}

void
splash (void)
{
	gchar xpmfile[400];
	GtkWidget *pixmap = NULL;


	g_snprintf (xpmfile, sizeof (xpmfile), "%s/gpsdrive/%s", DATADIR,
		    "pixmaps/gpsdrivesplash.png");

	splash_window = gtk_window_new (GTK_WINDOW_POPUP);


	gtk_window_set_title (GTK_WINDOW (splash_window), "gpsdrive");
	gtk_window_set_position (GTK_WINDOW (splash_window),
				 GTK_WIN_POS_CENTER);

	gtk_widget_realize (splash_window);
	gdk_window_set_decorations (GTK_WIDGET (splash_window)->window, 0);

	/* get image */
	pixmap = getPixmapFromFile (splash_window, xpmfile);

	if (pixmap != NULL)
	{
		gtk_pixmap_set (GTK_PIXMAP (pixmap),
				GTK_PIXMAP (pixmap)->pixmap,
				GTK_PIXMAP (pixmap)->mask);
	}
	else
	{
		fprintf (stderr,
			 _
			 ("\nWarning: unable to open splash picture\nPlease install the program as root with:\nmake install\n\n"));
		return;
	}
	/*   gtk_widget_show (splash_window);  */


	gtk_container_add (GTK_CONTAINER (splash_window), pixmap);
	gtk_widget_shape_combine_mask (splash_window,
				       GTK_PIXMAP (pixmap)->mask, 0, 0);


	gtk_widget_show (pixmap);
	gtk_widget_show (splash_window);
	while (gtk_events_pending ())
		gtk_main_iteration ();

	gtk_timeout_add (3000, (GtkFunction) splashaway_cb, NULL);
}


gint
about_cb (GtkWidget * widget, guint datum)
{
	static GtkWidget *window = NULL;
	GtkWidget *knopf, *vbox;
	gchar xpmfile[400], s3[800];
	GtkWidget *pixmap = NULL, *label1, *label2;
#ifdef USETELEATLAS
	GtkWidget *pixmap2 = NULL;
#endif


	g_snprintf (xpmfile, sizeof (xpmfile), "%s/gpsdrive/%s", DATADIR,
		    "gpsdrivemini.png");


	window = gtk_dialog_new ();

	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	g_signal_connect (window, "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &window);

	gtk_window_set_title (GTK_WINDOW (window), _("About GpsDrive"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	/*   gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER); */


	pixmap = getPixmapFromFile (window, xpmfile);

	if (pixmap != NULL)
	{
		gtk_pixmap_set (GTK_PIXMAP (pixmap),
				GTK_PIXMAP (pixmap)->pixmap,
				GTK_PIXMAP (pixmap)->mask);
	}
	else
	{
		fprintf (stderr,
			 _
			 ("\nWarning: unable to open splash picture\nPlease install the program as root with:\nmake install\n\n"));
		return TRUE;
	}


	vbox = gtk_vbox_new (FALSE, 3);
	/*   gtk_container_add (GTK_CONTAINER (window), vbox); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX (vbox), pixmap, TRUE, TRUE, 3);
	label1 = gtk_label_new ("");
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"NavyBlue\" font_desc=\"%s\">GpsDrive %s\n\n</span><span color=\"black\" font_desc=\"%s\">Copyright \xc2\xa9 2001-2004\nby Fritz Ganter (ganter@ganter.at)\n\nURL: http://www.gpsdrive.cc\n</span>",
		    "sans bold 16", VERSION, "sans italic 10");
	gtk_label_set_markup (GTK_LABEL (label1), s3);
	gtk_box_pack_start (GTK_BOX (vbox), label1, TRUE, TRUE, 3);

#ifdef USETELEATLAS
	pixmap2 = getPixmapFromXpm (window, logo_teleatlas_xpm);

	gtk_box_pack_start (GTK_BOX (vbox), pixmap2, TRUE, TRUE, 3);
#endif


	label2 = gtk_label_new ("");
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"black\" font_desc=\"%s\">This program is free software; you can redistribute\nit and/or modify it under the terms of the\nGNU General Public License as published by\nthe Free Software Foundation; either version 2\nof the License, or (at your option) any later version.\n</span>",
		    "sans normal 8");

	gtk_label_set_markup (GTK_LABEL (label2), s3);
	gtk_box_pack_start (GTK_BOX (vbox), label2, TRUE, TRUE, 3);


	knopf = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	GTK_WIDGET_SET_FLAGS (knopf, GTK_CAN_DEFAULT);
	gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
				   GTK_SIGNAL_FUNC
				   (gtk_widget_destroy), GTK_OBJECT (window));
	/*   gtk_box_pack_start (GTK_BOX (vbox), knopf, FALSE, FALSE, 3); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    knopf, TRUE, TRUE, 2);

	gtk_widget_show_all (window);

	return TRUE;

}

/* writes time and position to /tmp/gpsdrivepos */

void
signalposreq ()
{
	FILE *f;
	time_t t;
	struct tm *ts;

	f = fopen ("/tmp/gpsdrivepos", "w");
	if (f == NULL)
	{
		perror ("/tmp/gpsdrivepos");
		return;
	}
	time (&t);
	ts = localtime (&t);
	fprintf (f, asctime (ts));
	fprintf (f, "POS %f %f\n", current_lat, current_lon);
	fclose (f);
}
