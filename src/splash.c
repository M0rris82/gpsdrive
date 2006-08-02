/*******************************************************************

Copyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>

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
#include <icons.h>
#include <splash.h>
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
extern gint wpflag, trackflag, muteflag, displaymap_top, displaymap_map;
extern gint scaleprefered, milesflag, nauticflag, metricflag, wp_from_sql;
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
		    "pixmaps/gpsdrivemini.png");


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
