/*******************************************************************

Copyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>

Website: www.gpsdrive.de

Disclaimer: Please do not use as a primary means of navigation. 

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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gpsdrive_config.h"
#include <gpsdrive.h>
#include <poi.h>
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


extern map_dir_struct *display_map;

GtkWidget *splash_window;
extern gint mydebug;
extern int sockfd;

extern coordinate_struct coords;


gint
remove_splash_cb (GtkWidget * widget, gpointer datum)
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
	const gchar *t1 =
		_("Left mouse button\t\t: Set position (Position Mode only)\n"
		 "Right mouse button\t\t: Set destination target on the map\n"
		 "Middle mouse button\t\t: Leave Position Mode\n"
		 "Shift-left mouse button\t: Prefer maps showing greater detail\n"
		 "Shift-right mouse button\t: Prefer maps showing a larger area\n"
		 "Control-left mouse button\t: Create a waypoint at the mouse cursor position\n"
		 "Control-right mouse button\t: Create a waypoint at the current GPS position\n\n");

	const gchar *t2 = _(" j\t: Jump to the next waypoint in route mode\n"
		      " x\t: Add a waypoint at the current GPS location\n"
		      " y\t: Add a waypoint at the mouse cursor position\n"
		      " w\t: Quickly add a waypoint at the current GPS location without confirmation\n"
		      " p\t: Quickly add a waypoint at the mouse cursor position without confirmation\n"
		      " r\t: Add the mouse position to the end of the current route (or begin a new one)\n"
		      " n\t: Switch on light for 60 seconds in nightmode\n"
		      " g\t: Toggle grid overlay\n"
		      " f\t: Toggle friends display\n"
		      " +\t: Zoom in (map scale)\n"
		      " -\t: Zoom out (map scale)\n");
	const gchar *t3 =
		_("You can move freely around the map by selecting Position Mode"
		  " in the Map Control menu. In this mode the GPS position"
		  " indicator is replaced by a blue rectangle. Reposition the"
		  " cursor with a left-click on the map. If you click on the"
		  " border of the map (the outer 20%) then the map switches"
		  " to the next area. Right-click to exit Position Mode.\n\n"
		  "Suggestions welcome at http://www.gpsdrive.de.\n\n");

	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("GpsDrive v"), -1,
						  "heading", NULL);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, VERSION, -1,
						  "heading", NULL);
	gtk_text_buffer_insert (buffer, &iter,
		_("\n\nYou can find new versions at http://www.gpsdrive.de\n\n"), -1);

	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
		_("Disclaimer: Do not use as a primary means of navigation!\n"),
		-1, "red_foreground", NULL);

	gtk_text_buffer_insert (buffer, &iter,
		_("This program is distributed in the hope that it will be useful, "
		  "but WITHOUT ANY WARRANTY; without even the implied warranty of "
		  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
		  "GNU General Public License (GPL) for more details: "
		  "http://www.gnu.org/licenses/gpl-2.0.html\n\n"), -1);

	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
		_("Please have a look in the manpage (man gpsdrive) for detailed help."),
		-1, "italic", NULL);
	gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
		_("Mouse control (clicking on the map):\n"),
		-1, "blue_foreground", NULL);

	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
	gtk_text_buffer_insert (buffer, &iter, t1, -1);

	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("Shortcuts:\n"), -1,
						  "blue_foreground", NULL);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);
	gtk_text_buffer_insert (buffer, &iter, t2, -1);
	gtk_text_buffer_insert (buffer, &iter, "\n", -1);

	gtk_text_buffer_insert (buffer, &iter,
				_("More key shortcuts are marked as "),
				-1);
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						  _("underlined"), -1,
						  "underline", NULL);
	gtk_text_buffer_insert (buffer, &iter,
		_(" letters in the control button text.\n"
		  "Press the underlined key together with the Alt-key.\n\n"), -1);

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
		fprintf (stderr, "\nsending to %s:\n%s\n", local_config.friends_serverip,
			 buf);
	sockfd = -1;
	friends_sendmsg (local_config.friends_serverip, buf);
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
show_splash (void)
{
	gchar xpmfile[400];
	GtkWidget *pixmap = NULL;

	gtk_window_set_auto_startup_notification (FALSE);
	
	g_snprintf (xpmfile, sizeof (xpmfile), "%s/gpsdrive/%s", DATADIR,
		"pixmaps/gpsdrivesplash.png");

	splash_window = gtk_window_new (GTK_WINDOW_POPUP);

	gtk_window_set_type_hint (GTK_WINDOW (splash_window),
		GDK_WINDOW_TYPE_HINT_SPLASHSCREEN);
	gtk_window_set_title (GTK_WINDOW (splash_window),
		_("Starting GPS Drive"));
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
			("\nWarning: unable to open splash picture\nPlease "
			"install the program as root with:\nmake install\n\n"));
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

	gtk_timeout_add (3000, (GtkFunction) remove_splash_cb, NULL);
}


gint
about_cb (GtkWidget * widget, guint datum)
{
	GtkAboutDialog *about_window;
	gchar xpmfile[400];
	GdkPixbuf *pixmap = NULL;
	
	const gchar *authors[] = 
		{
			"Aart Koelewijn <aart(at)mtack.xs4all nl>",
			"Belgabor <belgabor(at)gmx de>",
			"Blake Swadling <blake(at)swadling com>",
			"Christoph Metz <loom(at)mopper de>",
			"Chuck Gantz <chuck.gantz(at)globalstar com>",
			"Dan Egnor <egnor(at)ofb net>",
			"Daniel Hiepler <rigid(at)akatash de>",
			"Darazs Attila <zumi(at)freestart hu>",
			"Fritz Ganter <ganter(at)ganter at>",
			"Guenther Meyer <d.s.e(at)sordidmusic com>",
			"Hamish Bowman <hamish_b(at)yahoo com>",
 			"J.D. Schmidt <jdsmobile(at)gmail com>",
			"Joerg Ostertag <gpsdrive(at)ostertag name>"	,
			"Jan-Benedict Glaw <jbglaw(at)lug-owl de>",
			"John Hay <jhay(at)icomtek.csir.co za>",
			"Johnny Cache <johnycsh(at)hick org>",
			"Miguel Angelo Rozsas <miguel(at)rozsas.xx.nom br>",
			"Mike Auty",
			"Oddgeir Kvien <oddgeir(at)oddgeirkvien com>",
			"Oliver Kuehlert <Oliver.Kuehlert(at)mpi-hd.mpg de>",
			"Olli Salonen <olli(at)cabbala net>",
			"Philippe De Swert",
			"Richard Scheffenegger <rscheff(at)chello at>",
			"Rob Stewart <rob(at)groupboard.com>",
			"Russell Harding <hardingr(at)billingside com>",
			"Russell Mirov <russell.mirov(at)sun com>",
			"Wilfried Hemp <Wilfried.Hemp(at)t-online de>",
			"<molter(at)gufi org>",
			"<pdana(at)mail.utexas edu>",
			"<timecop(at)japan.co jp>",
			"<wulf(at)netbsd org>",
			NULL
		};
	
	about_window = GTK_ABOUT_DIALOG (gtk_about_dialog_new ());
	gtk_window_set_position (GTK_WINDOW (about_window), GTK_WIN_POS_CENTER);
	
	gtk_about_dialog_set_version (about_window, VERSION);
	gtk_about_dialog_set_copyright (about_window,
		"Copyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>\n"
		"Copyright (c) 2005-2008 by the members of the GpsDrive Development Team\n");
	gtk_about_dialog_set_website (about_window, "http://www.gpsdrive.de\n");
	gtk_about_dialog_set_authors (about_window, authors);
	gtk_about_dialog_set_translator_credits (about_window, _("translator-credits"));
	gtk_about_dialog_set_comments (about_window,
		_("GpsDrive is a car (bike, ship, plane, foot) navigation "
		  "system that displays your position provided from a GPS "
		  "receiver on a zoomable map and much more..."));
	
	gtk_about_dialog_set_license (about_window,
		_("This program is free software; you can redistribute it and/or "
		  "modify it under the terms of the GNU General Public License "
		  "as published by the Free Software Foundation; either "
		  "version 2 of the License, or (at your option) any later "
		  "version.\nSee http://www.gnu.org/licenses/gpl-2.0.html\n\n"
		  "GpsDrive uses data from the OpenStreetMap Project "
		  "(http://www.openstreetmap.org), which is freely "
		  "available under the terms of the Creative Commons "
		  "Attribution-ShareAlike 2.0 license."));
	gtk_about_dialog_set_wrap_license (about_window, TRUE);
	
	g_snprintf (xpmfile, sizeof (xpmfile), "%s/gpsdrive/%s", DATADIR, "pixmaps/gpsdrivelogo.png");		
	pixmap = gdk_pixbuf_new_from_file (xpmfile, NULL);
	if (pixmap == NULL)
	{
		fprintf (stderr, _("\nWarning: unable to open logo picture\nPlease install the program as root with:\nmake install\n\n"));
		return TRUE;
	}
	gtk_about_dialog_set_logo (about_window, pixmap);
	
	gtk_widget_show_all (GTK_WIDGET (about_window));
	g_signal_connect (GTK_WIDGET (about_window), "response", G_CALLBACK
			  (gtk_widget_destroy), NULL);
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
	fprintf (f, "POS %f %f\n", coords.current_lat, coords.current_lon);
	fclose (f);
}
