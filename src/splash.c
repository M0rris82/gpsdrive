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

/*
  $Log$
  Revision 1.10  2005/04/13 19:58:31  tweety
  renew indentation to 4 spaces + tabstop=8

  Revision 1.9  2005/04/10 21:50:50  tweety
  reformatting c-sources

  Revision 1.8  2005/04/01 21:17:08  tweety
  Fix grid config bug. The grid was alway on after start
  reformatted reading of parameters

  Revision 1.7  2005/02/06 17:52:44  tweety
  extract icon handling to icons.c

  Revision 1.6  2005/02/02 17:42:54  tweety
  Add some comments
  extract some code into funktions
  added POI mySQL Support

  Revision 1.5  2005/01/22 11:38:06  tweety
  added more Help for keys

  Revision 1.4  2005/01/20 00:54:07  tweety
  added Help for more Keys

  Revision 1.3  2005/01/20 00:11:38  tweety
  add debug output if config is written

  Revision 1.2  2005/01/15 23:46:46  tweety
  Added config option to disable/enable drawing of grid

  Revision 1.1.1.1  2004/12/23 16:03:24  commiter
  Initial import, straight from 2.10pre2 tar.gz archive

  Revision 1.110  2004/03/02 03:07:17  ganter
  added speech output for received messages

  Revision 1.109  2004/03/02 00:53:35  ganter
  v2.09pre1
  added new gpsfetchmap.pl (works again with Expedia)
  added sound settings in settings menu
  max serial device string is now 40 char

  Revision 1.108  2004/02/19 18:00:27  ganter
  added Tele Atlas logo in about window

  Revision 1.107  2004/02/18 13:24:19  ganter
  navigation

  Revision 1.106  2004/02/11 11:25:46  ganter
  added patch from Johnny Cache <johnycsh@hick.org>, dbname is now configurable
  in gpsdriverc
  additional search path for libmysql for cygwin

  Revision 1.105  2004/02/08 17:39:46  ganter
  v2.08pre12

  Revision 1.104  2004/02/08 17:16:25  ganter
  replacing all strcat with g_strlcat to avoid buffer overflows

  Revision 1.103  2004/02/08 12:44:11  ganter
  replacing all sprintf with g_snprintf to avoid buffer overflows

  Revision 1.102  2004/02/07 00:02:16  ganter
  added "store timezone" button in settings menu

  Revision 1.101  2004/02/05 19:47:31  ganter
  replacing strcpy with g_strlcpy to avoid bufferoverflows
  USB receiver does not send sentences in direct serial mode,
  so I first send a "\n" to it

  Revision 1.100  2004/02/02 03:38:32  ganter
  code cleanup

  Revision 1.99  2004/02/01 04:51:22  ganter
  added "no_ssid" button in the SQL settings

  Revision 1.98  2004/02/01 02:25:56  ganter
  use dbhostname (the hostname of the SQL server) now works
  dbhostname may be edited in gpsdriverc

  Revision 1.97  2004/01/31 13:43:57  ganter
  nasamaps are working better, but still bugs

  Revision 1.96  2004/01/28 15:31:43  ganter
  initialize FDs to -1

  Revision 1.95  2004/01/28 09:32:57  ganter
  tested for memory leaks with valgrind, looks good :-)

  Revision 1.94  2004/01/27 22:51:59  ganter
  added "direct serial connection" button in settings menu

  Revision 1.93  2004/01/27 06:59:14  ganter
  The baudrate is now selectable in settings menu
  GpsDrive now connects to the GPS receiver in following order:
  Try to connect to gpsd
  Try to find Garble-mode Garmin
  Try to read data directly from serial port

  If this all fails, it falls back into simulation mode

  Revision 1.92  2004/01/26 11:55:19  ganter
  just indented some files

  Revision 1.91  2004/01/24 03:27:51  ganter
  friends label color is now changeable in settings menu

  Revision 1.90  2004/01/22 07:13:27  ganter
  ...

  Revision 1.89  2004/01/18 19:37:24  ganter
  this is the nice 2.07 release

  Revision 1.88  2004/01/18 06:34:54  ganter
  button for reminder window

  Revision 1.87  2004/01/18 05:53:35  ganter
  try to find the problem that x-server eats cpu after 5 hours

  Revision 1.86  2004/01/18 05:28:02  ganter
  changed all popups to gtk_dialog instead of a toplevel window
  cosmetic changes in settings menu

  Revision 1.85  2004/01/17 06:11:04  ganter
  added color setting for track color

  Revision 1.84  2004/01/17 00:58:09  ganter
  randomize the startposition and set it the hamburg cementry :-)

  Revision 1.83  2004/01/16 19:49:30  ganter
  added new icon

  Revision 1.82  2004/01/15 16:00:21  ganter
  added gpssmswatch

  Revision 1.81  2004/01/14 00:48:49  ganter
  fixed bug if no crypt is avail.

  Revision 1.80  2004/01/13 23:38:30  ganter
  added new field in waypoints display for number of friends received

  Revision 1.79  2004/01/13 19:57:48  ganter
  added GNU license to about-popup

  Revision 1.78  2004/01/12 23:52:15  ganter
  grrr

  Revision 1.77  2004/01/12 22:38:36  ganter
  v2.07pre8

  Revision 1.76  2004/01/12 22:09:59  ganter
  some text changes for messages

  Revision 1.75  2004/01/12 21:52:02  ganter
  added friends message service

  Revision 1.74  2004/01/11 13:48:28  ganter
  added about screen
  added menubar

  Revision 1.73  2004/01/03 03:39:52  ganter
  added settings switch for etched frames

  Revision 1.72  2004/01/01 09:07:33  ganter
  v2.06
  trip info is now live updated
  added cpu temperature display for acpi
  added tooltips for battery and temperature

  Revision 1.71  2003/12/22 19:33:37  ganter
  better test if image has alpha

  Revision 1.70  2003/12/21 17:31:32  ganter
  error handling for not installed program
  real v2.05 :-)

  Revision 1.69  2003/12/21 17:08:07  ganter
  release v2.05

  Revision 1.68  2003/12/21 16:59:05  ganter
  fixed bug in timezone setting
  timezone will be stored now

  Revision 1.67  2003/12/17 02:17:56  ganter
  added donation window
  waypoint describtion (.dsc files) works again
  added dist_alarm ...

  Revision 1.66  2003/08/12 12:21:20  ganter
  fixed bugs of PDA patch

  Revision 1.65  2003/06/08 13:31:50  ganter
  release 2.0pre9
  Added setting of timeperiod in friends mode (see settings menu)

  Revision 1.64  2003/06/01 17:27:34  ganter
  v2.0pre8
  friendsmode works fine and can be set in settings menu

  Revision 1.63  2003/05/31 20:32:01  ganter
  friendsd2 works fine with sven's server

  Revision 1.62  2003/05/31 20:12:35  ganter
  new UDP friendsserver build in, needs some work

  Revision 1.61  2003/05/28 22:20:12  ganter
  added load balancer

  Revision 1.60  2003/05/11 21:15:46  ganter
  v2.0pre7
  added script convgiftopng
  This script converts .gif into .png files, which reduces CPU load
  run this script in your maps directory, you need "convert" from ImageMagick

  Friends mode runs fine now
  Added parameter -H to correct the alitude

  Revision 1.59  2003/05/08 21:18:05  ganter
  added settings menu entry for fonts setting
  made a new cool splash screen
  updated da and it translations
  v2.0-pre6

  Revision 1.58  2003/05/07 10:52:23  ganter
  ...

  Revision 1.57  2003/05/06 17:49:11  ganter
  wp label text is now pango

  Revision 1.56  2003/05/03 18:59:47  ganter
  shortcuts are now working

  Revision 1.55  2003/05/03 03:03:30  ganter
  added help window

  Revision 1.54  2003/05/02 19:40:43  ganter
  changed location of datadir files

  Revision 1.53  2003/05/02 18:27:18  ganter
  porting to GTK+-2.2
  GpsDrive Version 2.0pre3

  Revision 1.52  2003/01/15 15:30:28  ganter
  before dynamically loading mysql

  Revision 1.51  2002/12/24 01:46:00  ganter
  FAQ

  Revision 1.50  2002/11/25 01:15:17  ganter
  ...

  Revision 1.49  2002/11/24 23:36:18  ganter
  added icon

  Revision 1.48  2002/11/02 12:38:55  ganter
  changed website to www.gpsdrive.de

  Revision 1.47  2002/10/27 10:51:30  ganter
  1.28pre8

  Revision 1.46  2002/10/24 08:44:09  ganter
  ...

  Revision 1.45  2002/10/16 14:16:13  ganter
  working on SQL gui

  Revision 1.44  2002/10/15 07:44:11  ganter
  ...

  Revision 1.43  2002/10/14 08:38:59  ganter
  v1.29pre3
  added SQL support

  Revision 1.42  2002/09/24 08:51:55  ganter
  updated translations
  changed gpsdrive.spec

  Revision 1.41  2002/09/23 07:51:35  ganter
  splited help text in more strings
  v1.28pre2

  Revision 1.40  2002/07/30 21:33:53  ganter
  added "J" key to switch to next waypoint

  Revision 1.39  2002/06/29 00:23:18  ganter
  added ACPI support for battery meter

  Revision 1.38  2002/06/23 17:09:35  ganter
  v1.23pre9
  now PDA mode looks good.

  Revision 1.37  2002/06/02 20:54:10  ganter
  added navigation.c and copyrights

  Revision 1.36  2002/05/23 09:07:37  ganter
  v1.23pre1
  added new BSD battery stuff

  Revision 1.35  2002/05/20 20:40:23  ganter
  v1.22

  Revision 1.34  2002/05/20 10:02:48  ganter
  v1.22pre7

  Revision 1.33  2002/05/12 23:14:53  ganter
  new 1.21
  changed B to N key.

  Revision 1.32  2002/05/12 20:54:08  ganter
  v1.21

  Revision 1.31  2002/05/11 15:45:31  ganter
  v1.21pre1
  degree,minutes,seconds should work now

  Revision 1.30  2002/05/02 01:34:11  ganter
  added speech output of waypoint description

  Revision 1.29  2002/04/29 02:52:18  ganter
  v1.20pre1
  added display of sat position

  Revision 1.28  2002/04/28 11:53:43  ganter
  v1.19pre2
  button to delete waypoint
  fixed miles distance on startup in miles mode
  new sat level display colors

  Revision 1.27  2002/04/14 15:25:56  ganter
  v1.17pre3
  added simulaton follow switch in setup menu

  Revision 1.26  2002/04/13 17:30:35  ganter
  v1.17pre2

  Revision 1.25  2002/04/06 17:08:56  ganter
  v1.16pre8
  cleanup of gpsd files

  Revision 1.24  2002/04/02 19:46:15  ganter
  v1.16pre4
  you can type in coordinates in the "Add waypoint" window (x-key)
  Autosave of configuration
  update spanish translation (translater had holiday)

  Revision 1.23  2002/04/01 22:32:04  ganter
  added garmin and serialdevice in setup

  Revision 1.22  2002/03/31 18:11:07  ganter
  v1.15-pre5
  you can select your "way*.txt" file in setup menu
  The DEFAUL entry in way.txt is now obsolet
  The "setdefaultpos" entry in gpsdricerc in now obsolet

  Revision 1.21  2002/03/30 20:07:33  ganter
  2nd pre 1.15

  Revision 1.20  2002/03/30 10:37:43  ganter
  pre 1.15

  Revision 1.19  2002/03/24 17:45:26  ganter
  v1.14 some cosmetic changes

  Revision 1.18  2002/03/22 00:36:40  ganter
  bugfix for late gpsd start
  remembering last position if setdefaultpos = 0 in gpsdriverc

  Revision 1.17  2002/03/21 21:49:08  ganter
  added question if gpsd should be started.
  Thanks to daZwerg(gEb-Dude) for suggestion.

  Revision 1.16  2002/03/16 20:21:07  ganter
  fixed segfault if no gpsdriverc exists. New v1.11

  Revision 1.15  2002/03/16 19:17:59  ganter
  v1.11

  Revision 1.14  2002/02/26 15:34:17  ganter
  added x key for set waypoint on actual position

  Revision 1.13  2002/02/24 22:26:41  ganter
  new v1.9, added shortcuts

  Revision 1.12  2002/02/24 16:40:37  ganter
  v1.9

  Revision 1.11  2002/02/18 01:50:39  ganter
  changed help text

  Revision 1.10  2001/10/14 10:41:59  ganter
  v0.32 correct path for prefix other than /usr/local

  Revision 1.9  2001/10/13 23:04:56  ganter
  corrected paths for locale and pixmap, ./configure --prefix= works now as
  expected

  Revision 1.8  2001/09/30 18:45:27  ganter
  v0.29
  added choice of map type

  Revision 1.7  2001/09/30 15:54:32  ganter
  added minimap, removed gpsdrivelogo

  Revision 1.6  2001/09/28 15:43:56  ganter
  v0.28 changed layout, some bugfixes

  Revision 1.5  2001/08/29 19:05:20  ganter
  trying splashfile also in current dir

  Revision 1.4  2001/08/29 13:59:56  ganter
  removed large xpm file, load png instead

  Revision 1.3  2001/08/26 15:49:09  ganter
  indent

  Revision 1.2  2001/08/26 15:47:19  ganter
  works fine

  Revision 1.1  2001/08/26 15:03:15  ganter
  added splash.c

*/
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <gpsdrive.h>
#include <time.h>
#include <xpm_talogo.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern GtkWidget *mainwindow;
GtkWidget *splash_window;
extern gchar homedir[500], mapdir[500];
extern gint wpflag, trackflag, muteflag, displaymap_top, displaymap_map;
extern gint scaleprefered, milesflag, nauticflag, metricflag, sqlflag;
extern gint debug, scalewanted, savetrack, defaultserver;
extern gchar serialdev[80];
extern gdouble current_long, current_lat, old_long, old_lat, groundspeed;
extern gint setdefaultpos, shadow, etch, drawgrid, poi_draw, testgarmin, needtosave, usedgps,
    simfollow;
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
extern char dbtypelist[100][40];
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
extern enum
{ english, german, spanish }
voicelang;

#define KM2MILES 0.62137119
#define KM2NAUTIC  0.54
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
				"justification", GTK_JUSTIFY_CENTER, NULL);

    gtk_text_buffer_create_tag (buffer, "underline",
				"underline", PANGO_UNDERLINE_SINGLE, NULL);

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
		  "+    : Zoom in \n"
		  "-    : Zoom out\n"
		  );
    gchar *t3 =
	_("Press the underlined key together with the ALT-key.\n\n"
	  "You can move on the map by selecting the Position-Mode"
	  " in the menu. A blue rectangle shows this mode, you can set this cursor by clicking on the map."
	  " If you click on the border of the map (the outer 20%) then the map switches to the next area.\n\n"
	  "Suggestions welcome.\n\n");

    gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);


    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, _("GpsDrive v"),
					      -1, "heading", NULL);
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
					      -1, "blue_foreground", NULL);

    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_insert (buffer, &iter, t1, -1);

    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _("Short cuts:\n"), -1,
					      "blue_foreground", NULL);
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    gtk_text_buffer_insert (buffer, &iter, t2, -1);
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);

    gtk_text_buffer_insert (buffer, &iter,
			    _("The other key shortcuts are marked as "), -1);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, _("underlined"),
					      -1, "underline", NULL);
    gtk_text_buffer_insert (buffer, &iter, _(" letters in the button text.\n"),
			    -1);
    gtk_text_buffer_insert (buffer, &iter, t3, -1);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _("Have a lot of fun!"), -1,
					      "big", NULL);
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);


    /* Apply word_wrap tag to whole buffer */
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_apply_tag_by_name (buffer, "not_editable", &start, &end);
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
    gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

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
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_add1 (GTK_PANED (vpaned), sw);

    gtk_container_add (GTK_CONTAINER (sw), view1);

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
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
					      _("Please donate to GpsDrive"),
					      -1, "heading", NULL);


    gtk_text_buffer_insert (buffer, &iter,
			    _
			    ("\n\nGpsDrive is a project with no comercial background. \n\n"
			     "It would be nice if you can give a donation to help me pay the costs for hardware and the webserver.\n\nTo do so, just go to"),
			    -1);
    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
					      _(" http://www.gpsdrive.cc "), -1,
					      "bold", NULL);
    gtk_text_buffer_insert (buffer, &iter,
			    _("and click on the PayPal button.\n\n"
			      "Thank you very much for your donation!\n\n"
			      "This message is only displayed once when you start an new version of GpsDrive.\n\n"),
			    -1);

    /* Apply word_wrap tag to whole buffer */
    gtk_text_buffer_get_bounds (buffer, &start, &end);
    gtk_text_buffer_apply_tag_by_name (buffer, "not_editable", &start, &end);
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
    gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

    g_signal_connect (window, "destroy",
		      G_CALLBACK (gtk_widget_destroyed), &window);

    gtk_window_set_title (GTK_WINDOW (window), _("About GpsDrive donation"));
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);

    vpaned = gtk_vpaned_new ();
    gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);

    gtk_box_pack_start (GTK_BOX
			(GTK_DIALOG (window)->vbox), vpaned, TRUE, TRUE, 2);

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
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_add1 (GTK_PANED (vpaned), sw);

    gtk_container_add (GTK_CONTAINER (sw), view1);

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
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
    gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

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
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_paned_add1 (GTK_PANED (vpaned), sw);

    gtk_container_add (GTK_CONTAINER (sw), view1);

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

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
				PANGO_WEIGHT_BOLD, "size", 10 * PANGO_SCALE,
				NULL);
    gtk_text_buffer_create_tag (buffer, "red_foreground",
				"foreground", "red", "weight",
				PANGO_WEIGHT_BOLD, "size", 10 * PANGO_SCALE,
				NULL);
    gtk_text_buffer_create_tag (buffer, "center",
				"justification", GTK_JUSTIFY_CENTER, NULL);
    if (fs)
	{
	    g_snprintf (buf, sizeof (buf),
			_("You received a message from\nthe friends server (%s)\n"),
			name);
	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, buf, -1,
						      "heading", "center",
						      "red_foreground", NULL);
	}
    else
	{
	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter,
						      _
						      ("You received a message through the friends server from:\n"),
						      -1, "heading", "center",
						      NULL);
	    gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, name, -1,
						      "blue_foreground", "center",
						      NULL);
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

    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (sw), knopf2);


    gtk_widget_show_all (vpaned);

    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

    gtk_widget_show_all (window);
    g_snprintf (buf, sizeof (buf), "ACK: %s", msgid);
    if (debug)
	fprintf (stderr, "\nsending to %s:\n%s\n", friendsserverip, buf);
    sockfd = -1;
    friends_sendmsg (friendsserverip, buf);
    gdk_beep ();
    switch (voicelang)
	{
	case english:
	    g_snprintf (buf, sizeof (buf),
			"You received a message from %s\n", name);
	    break;
	case spanish:
	    g_snprintf (buf, sizeof (buf),
			"You received a message from %s\n", name);
	    break;
	case german:
	    g_snprintf (buf, sizeof (buf),
			"Sie haben eine Nachricht von %s erhalten", name);
	}
    speech_out_speek (buf);

    return TRUE;

}


/*  write the configurationfile ~/.gpsdrive/gpsdriverc */
void
writeconfig ()
{
    FILE *fp;
    gchar fname[220], str[40];

    g_strlcpy (fname, homedir, sizeof (fname));
    g_strlcat (fname, "gpsdriverc", sizeof (fname));

    if (debug) 
	printf("Write config %s\n",fname);



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

    g_snprintf (str, sizeof (str), "%.6f", current_long);
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

    fprintf (fp, "minsecmode = ");
    if (minsecmode == 0)
	fprintf (fp, "0\n");
    else
	fprintf (fp, "1\n");

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
    fprintf (fp, "drawgrid = %d\n", drawgrid);
    fprintf (fp, "drawpoi = %d\n", poi_draw);
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

    if (debug)
	g_print ("reading config file...\n");

    while ((fgets (buf, 1000, fp)) > 0)
	{
	    g_strlcpy (par1, "", sizeof (par1));
	    g_strlcpy (par2, "", sizeof (par2));
	    e = sscanf (buf, "%s = %[^\n]", par1, par2);

	    if (debug)
		g_print ("%d [%s] = [%s]\n", e, par1, par2);

	    if (e == 2)
		{
		    if ((strcmp (par1, "showwaypoints")) == 0)	    wpflag = atoi (par2);
		    if ((strcmp (par1, "showtrack")) == 0)	    trackflag = atoi (par2);
		    if ((strcmp (par1, "mutespeechoutput")) == 0)	    muteflag = atoi (par2);
		    if ((strcmp (par1, "showtopomaps")) == 0)	    displaymap_top = atoi (par2);
		    if ((strcmp (par1, "showstreetmaps")) == 0)	    displaymap_map = atoi (par2);

		    /*  To set the right sensitive flags bestmap_cb is called later*/
		    if ((strcmp (par1, "autobestmap")) == 0)	    scaleprefered = !(atoi (par2));


		    if ((strcmp (par1, "units")) == 0)
			{
			    milesflag = metricflag = nauticflag = FALSE;
			    if ((strcmp (par2, "miles")) == 0)
				{
				    milesflag = TRUE;
				    milesconv = KM2MILES;
				}
			    else
				{
				    if ((strcmp (par2, "metric")) == 0)
					{
					    metricflag = TRUE;
					    milesconv = 1.0;
					}
				    else if ((strcmp (par2, "nautic")) == 0)
					{
					    nauticflag = TRUE;
					    milesconv = KM2NAUTIC;
					}
				}
			}

		    if ((strcmp (par1, "savetrack")) == 0)	    savetrack = atoi (par2);
		    if ((strcmp (par1, "scalewanted")) == 0)	    scalewanted = atoi (par2);
		    if ((strcmp (par1, "serialdevice")) == 0)	    g_strlcpy (serialdev, par2, sizeof (serialdev));
		    if ((strcmp (par1, "lastlong")) == 0)	            current_long = g_strtod (par2, 0);
		    if ((strcmp (par1, "lastlat")) == 0)	            current_lat = g_strtod (par2, 0);
		    /*        if ((strcmp (par1, "setdefaultpos")) == 0)   	    setdefaultpos = atoi (par2); */
		    if ((strcmp (par1, "shadow")) == 0)	            shadow = atoi (par2);
		    if ((strcmp (par1, "defaultserver")) == 0)	    defaultserver = atoi (par2);
		    if ((strcmp (par1, "waypointfile")) == 0)	    g_strlcpy (activewpfile, par2, sizeof (activewpfile));
		    if ((strcmp (par1, "testgarminmode")) == 0)	    testgarmin = atoi (par2);
		    if ((strcmp (par1, "usedgps")) == 0)     	    usedgps = atoi (par2);
		    if ((strcmp (par1, "mapdir")) == 0)      	    g_strlcpy (mapdir, par2, sizeof (mapdir));
		    if ((strcmp (par1, "simfollow")) == 0)	    simfollow = atoi (par2);
		    if ((strcmp (par1, "satposmode")) == 0)	    satposmode = atoi (par2);
		    if ((strcmp (par1, "printoutsats")) == 0)	    printoutsats = atoi (par2);
		    if ((strcmp (par1, "minsecmode")) == 0)	    minsecmode = atoi (par2);
		    if ((strcmp (par1, "nightmode")) == 0)	    nightmode = atoi (par2);
		    if ((strcmp (par1, "cpuload")) == 0)	            cpuload = atoi (par2);
		    if ((strcmp (par1, "flymode")) == 0)    	    flymode = atoi (par2);
		    if ((strcmp (par1, "vfr")) == 0)        	    vfr = atoi (par2);
		    if ((strcmp (par1, "disdevwarn")) == 0)	    disdevwarn = atoi (par2);
		    if ((strcmp (par1, "sqlflag")) == 0)	            sqlflag = atoi (par2);
		    if ((strcmp (par1, "lastnotebook")) == 0)	    lastnotebook = atoi (par2);
		    if ((strcmp (par1, "dbhostname")) == 0)
			{
			    g_strlcpy (dbhost, par2, sizeof (dbhost));
			    g_strstrip (dbhost);
			}
		    if ((strcmp (par1, "dbname")) == 0)	    	    g_strlcpy (dbname, par2, sizeof (dbname));
		    if ((strcmp (par1, "dbuser")) == 0)	    	    g_strlcpy (dbuser, par2, sizeof (dbuser));
		    if ((strcmp (par1, "dbpass")) == 0)	    	    g_strlcpy (dbpass, par2, sizeof (dbpass));
		    if ((strcmp (par1, "dbtable")) == 0)	            g_strlcpy (dbtable, par2, sizeof (dbtable));
		    if ((strcmp (par1, "dbname")) == 0)	            g_strlcpy (dbname, par2, sizeof (dbname));
		    if ((strcmp (par1, "dbdistance")) == 0)	    dbdistance = g_strtod (par2, 0);
		    if ((strcmp (par1, "dbusedist")) == 0)	    dbusedist = atoi (par2);
		    if ((strcmp (par1, "dbwherestring")) == 0)        g_strlcpy (dbwherestring, par2, sizeof (dbwherestring));
		    if ((strcmp (par1, "earthmate")) == 0)	    earthmate = atoi (par2);
		    if ((strcmp (par1, "wplabelfont")) == 0)	    g_strlcpy (wplabelfont, par2, sizeof (wplabelfont));
		    if ((strcmp (par1, "bigfont")) == 0)	            g_strlcpy (bigfont, par2, sizeof (bigfont));
		    if ((strcmp (par1, "friendsserverip")) == 0)	    g_strlcpy (friendsserverip, par2, sizeof (friendsserverip));
		    if ((strcmp (par1, "friendsserverfqn")) == 0)	    g_strlcpy (friendsserverfqn, par2, sizeof (friendsserverfqn));
		    if ((strcmp (par1, "friendsname")) == 0)	    g_strlcpy (friendsname, par2, sizeof (friendsname));
		    if ((strcmp (par1, "friendsidstring")) == 0)	    g_strlcpy (friendsidstring, par2, sizeof (friendsidstring));
		    if ((strcmp (par1, "usefriendsserver")) == 0)	    havefriends = atoi (par2);
		    if ((strcmp (par1, "maxfriendssecs")) == 0)	    maxfriendssecs = atoi (par2);
		    if ((strcmp (par1, "reminder")) == 0)
			if ((strcmp (par2, VERSION)) == 0)
			    {
				needreminder = FALSE;
				/* 		  fprintf(stderr,"needreminder false\n"); */
			    }
		    if ((strcmp (par1, "storetz")) == 0)	            storetz = atoi (par2);
		    if (storetz)
			if ((strcmp (par1, "timezone")) == 0)             zone = atoi (par2);
		    if ((strcmp (par1, "etch")) == 0)                 etch = atoi (par2);
		    if ((strcmp (par1, "drawgrid")) == 0)	            { drawgrid = atoi (par2); printf("DrawGrid set to %d\n",drawgrid);};
		    if ((strcmp (par1, "drawpoi")) == 0)	            poi_draw = atoi (par2);
		    if ((strcmp (par1, "bigcolor")) == 0)	            g_strlcpy (bluecolor, par2, sizeof (bluecolor));
		    if ((strcmp (par1, "trackcolor")) == 0)	    g_strlcpy (trackcolor, par2, sizeof (trackcolor));
		    if ((strcmp (par1, "friendscolor")) == 0)	    g_strlcpy (friendscolor, par2, sizeof (friendscolor));
		    if ((strcmp (par1, "messagenumber")) == 0)	    messagenumber = atoi (par2);
		    if ((strcmp (par1, "serialspeed")) == 0)	    serialspeed = atoi (par2);
		    if ((strcmp (par1, "disableserial")) == 0)	    disableserial = atoi (par2);
		    if ((strcmp (par1, "showssid")) == 0)	    	    showsid = atoi (par2);
		    if ((strcmp (par1, "sound_direction")) == 0)	    sound_direction = atoi (par2);
		    if ((strcmp (par1, "sound_distance")) == 0)	    sound_distance = atoi (par2);
		    if ((strcmp (par1, "sound_speed")) == 0)	    sound_speed = atoi (par2);
		    if ((strcmp (par1, "sound_gps")) == 0)	    sound_gps = atoi (par2);
		}			/* if e==2 */
	}

    if (debug)
	g_print ("\nreading config file finished\n");
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
    gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap, &mask,
						     NULL, filename);
    if (gdkpixmap == NULL)
	{
	    printf ("**** couldn't create pixmap from file: '%s'\n", filename);
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
    gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (NULL, colormap, &mask,
						       NULL, xpmname);
    if (gdkpixmap == NULL)
	{
	    printf ("**** couldn't create pixmap from xpm: '%s'\n", *xpmname);
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
		"gpsdrivesplash.png");

    splash_window = gtk_window_new (GTK_WINDOW_POPUP);


    gtk_window_set_title (GTK_WINDOW (splash_window), "gpsdrive");
    gtk_window_set_position (GTK_WINDOW (splash_window), GTK_WIN_POS_CENTER);

    gtk_widget_realize (splash_window);
    gdk_window_set_decorations (GTK_WIDGET (splash_window)->window, 0);

    /*   gtk_widget_realize (splash_window);  */

    /* get image */
    pixmap = getPixmapFromFile (splash_window, xpmfile);

    if (pixmap != NULL)
	{
	    gtk_pixmap_set (GTK_PIXMAP (pixmap),
			    GTK_PIXMAP (pixmap)->pixmap, GTK_PIXMAP (pixmap)->mask);
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
    gtk_widget_shape_combine_mask (splash_window, GTK_PIXMAP (pixmap)->mask, 0,
				   0);


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

    gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));

    g_signal_connect (window, "destroy",
		      G_CALLBACK (gtk_widget_destroyed), &window);

    gtk_window_set_title (GTK_WINDOW (window), _("About GpsDrive"));
    gtk_container_set_border_width (GTK_CONTAINER (window), 5);
    /*   gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER); */


    pixmap = getPixmapFromFile (window, xpmfile);

    if (pixmap != NULL)
	{
	    gtk_pixmap_set (GTK_PIXMAP (pixmap),
			    GTK_PIXMAP (pixmap)->pixmap, GTK_PIXMAP (pixmap)->mask);
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
    fprintf (f, "POS %f %f\n", current_lat, current_long);
    fclose (f);
}
