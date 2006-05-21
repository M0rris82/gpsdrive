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

    *********************************************************************

***********************************************************************/

/* *****************************************************************************
   Contributors:
     <molter@gufi.org>.
     <wulf@netbsd.org>
     Aart Koelewijn <aart@mtack.xs4all.nl>
     Belgabor <belgabor@gmx.de>
     Blake Swadling <blake@swadling.com>
     Chuck Gantz- chuck.gantz@globalstar.com
     Dan Egnor <egnor@ofb.net>
     Daniel Hiepler <rigid@akatash.de>
     Darazs Attila <zumi@freestart.hu>
     Fritz Ganter <ganter@ganter.at>
     J.D. Schmidt <jdsmobile@gmail.com>
     Jan-Benedict Glaw <jbglaw@lug-owl.de>
     Joerg Ostertag <gpsdrive@ostertag.name>
     John Hay <jhay@icomtek.csir.co.za>
     Johnny Cache <johnycsh@hick.org>
     Miguel Angelo Rozsas <miguel@rozsas.xx.nom.br>
     Mike Auty
     Oddgeir Kvien <oddgeir@oddgeirkvien.com>
     Oliver Kuehlert <Oliver.Kuehlert@mpi-hd.mpg.de>!
     Olli Salonen <olli@cabbala.net>
     Philippe De Swert
     Richard Scheffenegger <rscheff@chello.at>
     Rob Stewart <rob@groupboard.com>
     Russell Harding <hardingr@billingside.com>
     Russell Mirov <russell.mirov@sun.com>
     Wilfried Hemp <Wilfried.Hemp@t-online.de>
     pdana@mail.utexas.edu
     timecop@japan.co.jp
*/

/*  Include Dateien */
#include "../config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <errno.h>
#include <gpsdrive_config.h>

#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

#if HAVE_LOCALE_H
#include <locale.h>
#else
# define setlocale(Category, Locale)
#endif

#include "gettext.h"

#include <dirent.h>


#include "LatLong-UTMconversion.h"
#include "gpsdrive.h"
#include "battery.h"
#include "track.h"
#include "poi.h"
#include "streets.h"
#include "draw_tracks.h"
#include "routes.h"
#include "gps_handler.h"
#include "nmea_handler.h"
#include <speech_strings.h>
#include <speech_out.h>

#include "import_map.h"
#include "download_map.h"
#include "icons.h"

#ifndef NOPLUGINS
#include "gmodule.h"
#endif
#ifdef USETELEATLAS
#include "gpsnavlib.h"
#endif

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


#if GTK_MINOR_VERSION < 2
#define gdk_draw_pixbuf _gdk_draw_pixbuf
#endif

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/*  global variables */
gint statusid, messagestatusbarid, timeoutcount;
GtkWidget *mainwindow;
GtkWidget *messagestatusbar;
GtkWidget *pixmapwidget, *gotowindow;
GtkWidget *messagewindow;
extern GtkWidget *splash_window;
gint debug = 0;
gint do_unit_test = FALSE;
gchar *buffer = NULL, *big = NULL;
struct timeval timeout;
gchar targetname[40];
gdouble current_lon, current_lat, old_lon, old_lat, groundspeed;
gdouble zero_lon, zero_lat;
gdouble target_lon, target_lat;
gdouble dist;
gdouble long_diff = 0, lat_diff = 0;
GdkGC *kontext;
GdkColor red = { 0, 0xFF00, 0x0000, 0x0000 };
GdkColor black = { 0, 0x0000, 0x0000, 0x0000 };
GdkColor white = { 0, 0xFFFF, 0xFFFF, 0xFFFF };
GdkColor blue = { 0, 0x0000, 0x0000, 0xff00 };
GdkColor nightcolor = { 0, 0xa000, 0x0000, 0x0000 };
GdkColor lcd = { 0, 0x8b00, 0x9500, 0x8b00 };
GdkColor lcd2 = { 0, 0x7300, 0x7d00, 0x6a00 };
GdkColor yellow = { 0, 0xff00, 0xff00, 0x0000 };
GdkColor green = { 0, 0x0000, 0xb000, 0x0000 };
GdkColor green2 = { 0, 0x0000, 0xff00, 0x0000 };
GdkColor mygray = { 0, 0xd500, 0xd600, 0xd500 };
GdkColor textback = { 0, 0xa500, 0xa600, 0xa500 };
GdkColor textbacknew = { 0, 0x4000, 0x7600, 0xcf00 };
GdkColor grey = { 0, 0xC000, 0xC000, 0xC000 };
GdkColor orange = { 0, 0xf000, 0x6000, 0x0000 };
GdkColor orange2 = { 0, 0xff00, 0x8000, 0x0000 };
GdkColor darkgrey = { 0, SHADOWGREY, SHADOWGREY, SHADOWGREY };
GdkColor defaultcolor;


GtkWidget *drawing_area, *drawing_bearing;
GtkWidget *drawing_sats, *drawing_miniimage;

GtkWidget *distlabel, *speedlabel, *altilabel, *miles, *startgpsbt;
GdkDrawable *drawable, *drawable_bearing, *drawable_sats;
gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle = 0;
gdouble pixelfact, posx, posy, angle_to_destination, direction, bearing;
GdkPixbuf *image = NULL, *tempimage = NULL, *miniimage = NULL;

extern GdkPixbuf *friendsimage, *friendspixbuf;

extern mapsstruct *maps;

extern GtkWidget *drawing_battery, *drawing_temp;


/* action=1: radar (speedtrap) */
wpstruct *wayp;

friendsstruct *friends, *fserver;
char friendsserverip[20], friendsname[40];
char friendsidstring[40], friendsserverfqn[255];
/* socket for friends  */
extern int sockfd;
extern long int maxfriendssecs;


gchar oldfilename[2048];
GString *tempmapfile;
gdouble earthr;
/*Earth radius calibrated with GARMIN III */
/*  gdouble R = 6383254.445; */


#define R earthr


gint maploaded = FALSE;
gint simmode, zoom;
gint iszoomed;
gchar homedir[500];
gchar mapdir[500];
static gchar const rcsid[] =
	"$Id$";
gint thisline;
gint maxwp, maxfriends = 0;
GtkStyle *style = NULL;
GtkRcStyle *mainstyle;
gint satlist[MAXSATS][4], satlistdisp[MAXSATS][4], satbit = 0;
GtkWidget *mylist;
GtkWidget *destframe;

extern gchar mapfilename[2048];

gint milesflag, metricflag, nauticflag;
gdouble milesconv;
gdouble gbreit, glang, olddist = 99999.0;
GTimer *timer, *disttimer;
gint gcount;
gchar localedecimal;

glong mapscale = 1000;
gint scaleprefered = 1;
gint scalewanted = 100000;

gint importactive;
extern gint downloadwindowactive;
extern gint downloadactive;

GtkWidget *add_wp_name_text, *wptext2;
gdouble wplat, wplon;
GtkWidget *scaler;
gchar oldangle[100];
GdkCursor *cursor;

// Uncomment this (or add a make flag?) to only have scales for expedia maps
//#define EXPEDIA_SCALES_ONLY

#ifdef EXPEDIA_SCALES_ONLY
gint slistsize = 13;
gchar *slist[] = { "5000", "15000", "20000", "50000", 
		   "100000", "200000", "750000", "3000000", "7500000", "75000000",
		   "88067900","90000000"
};
gint nlist[] = { 5000, 15000, 20000, 50000, 
		 100000, 200000, 750000, 3000000, 7500000, 75000000,
		 88067900,90000000,
};
#else
gint slistsize = 34;
gchar *slist[] = { "300", "500", 
		   "1000", "1500", "2000", "3000", "5000", "7500", 
		   "10000", "15000", "20000", "30000", "50000", "75000",
		   "100000", "150000", "200000", "300000", "500000", "750000", 
		   "1000000", "1500000", "2000000", "3000000", "5000000", "7500000", 
		   "10000000", "15000000", "20000000", "30000000", "50000000", "75000000",
		   "88067900","90000000"
};
gint nlist[] = { 300, 500,
		 1000, 1500, 2000, 3000, 5000, 7500,
		 10000, 15000, 20000, 30000, 50000, 75000,
		 100000, 150000, 200000, 300000, 500000, 750000,
		 1000000, 1500000, 2000000, 3000000, 5000000, 7500000,
		 10000000, 15000000, 20000000, 30000000, 50000000, 75000000,
		 88067900,90000000
};
#endif

GtkWidget *label_lat, *label_lon;
GtkWidget *label_map_filename, *label_map_scale;
GtkWidget *label_heading, *label_baering, *label_timedest;
GtkWidget *label_prefscale, *mutebt, *sqlbt;
GtkWidget *trackbt, *wpbt;
GtkWidget *bestmapbt, *poi_draw_bt, *streets_draw_bt, *tracks_draw_bt;


GtkWidget *savetrackbt;
GtkWidget *loadtrackbt;
GtkWidget *scalerlbt, *scalerrbt;
GtkWidget *setupbt;
gint savetrack = 0, havespeechout, hours, minutes, speechcount = 0;
gint muteflag = 0, wp_from_sql = 0, trackflag = 1, wpflag = TRUE;
gint posmode = 0;
gdouble posmode_x, posmode_y;
GtkObject *scaler_adj;
gchar lastradar[40], lastradar2[40]; 
gint foundradar;
gdouble radarbearing;
gint errortextmode = TRUE;
gchar serialdev[80];
GdkSegment *track, *trackshadow;
glong tracknr;
trackcoordstruct *trackcoord;
extern glong trackcoordnr, tracklimit, trackcoordlimit,old_trackcoordnr;
gint extrawinmenu = FALSE;
gdouble lat2RadiusArray[201];
gint haveproxy, proxyport;
gchar proxy[256], hostname[256], savetrackfn[256];

/*** Mod by Arms */
gint real_screen_x, real_screen_y, real_psize, real_smallmenu, int_padding;
gint SCREEN_X_2, SCREEN_Y_2;
gint havefriends = 0;
time_t waytxtstamp = 0;
gint showallmaps = TRUE;
/* guint selwptimeout; */
gint dontsetwp = FALSE;
gint setwpactive = FALSE;
gint onemousebutton = FALSE;
gint shadow = TRUE;


GtkWidget *askwindow;
extern time_t maptxtstamp;
gint simpos_timeout = 0;
gint setdefaultpos = TRUE;
gint markwaypoint = FALSE;
GtkWidget *addwaypointwindow, *setupfn[30];
gint oldbat = 125, oldloading = FALSE;
gint bat, loading;
gint disableapm = FALSE;
typedef struct
{
	gchar n[200];
}
namesstruct;
namesstruct *names;
gchar activewpfile[200], gpsdservername[200], setpositionname[80];
gint newsatslevel = TRUE, testgarmin = TRUE;
gint needtosave = FALSE;
GtkWidget *serialbt, *mapdirbt;
GtkWidget *slowcpubt;
GtkWidget *add_wp_lon_text, *add_wp_lat_text;

gdouble altitude = 0.0, precision = (-1.0), gsaprecision = (-1.0);
gint oldsatfix = 0, oldsatsanz = -1, havealtitude = FALSE;
gint wpsize = 1000, satfix = 0, usedgps = FALSE;
gchar dgpsserver[80], dgpsport[10];
GtkWidget *posbt;
GtkWidget *cover;
extern gint needreloadmapconfig;
gint simfollow = TRUE;
GdkPixbuf *batimage = NULL;
GdkPixbuf *temimage = NULL;
GdkPixbuf *satsimage = NULL;
gint numsats = 0, satsavail = 0;
gint numgrids = 4, scroll = TRUE, deleteline = 0;
gint satposmode = FALSE;
gint saytarget = FALSE, printoutsats = FALSE;
extern gchar *displaytext;
extern gint do_display_dsc, textcount;
gint minsecmode = LATLON_DEGDEC;
gint nightmode = FALSE, isnight = FALSE, disableisnight;
gint nighttimer;
GtkWidget *setupentry[50], *setupentrylabel[50];
void (*setupfunction[50]) ();
gchar utctime[20], loctime[20];
gint mod_setupcounter = 4;
gint cpuload = 40, redrawtimeout;
gint borderlimit;
gint zoomscale = TRUE;
gint pdamode = FALSE;
gint exposecounter = 0, exposed = FALSE;
gint useplugins = FALSE;
gdouble tripodometer, tripavspeed, triptime, tripmaxspeed, triptmp;
gint tripavspeedcount;
gdouble trip_lon, trip_lat;
gint lastnotebook = 0;
GtkWidget *settingsnotebook;
gint useflite = FALSE;
extern gint zone;
gint ignorechecksum = FALSE;
GdkColor trackcolorv;

/* Give more debug informations */
gint mydebug = 0;
#define MAXDBNAME 30
char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
char dbtable[MAXDBNAME], dbname[MAXDBNAME];
char wp_typelist[100][40];
int wp_typelistcount;
char dbwherestring[5000];
double dbdistance;
gint usesql = FALSE, dbusedist = FALSE;
extern gint sqlselects[100], sqlplace, friendsplace, kismetsock, havekismet;

extern GdkPixbuf *kismetpixbuf,	*iconpixbuf[50];
gint earthmate = FALSE;

extern gint wptotal, wpselected;
gchar wplabelfont[100], bigfont[100];

extern gint zone;
static gchar gradsym[] = "\xc2\xb0";
gdouble normalnull = 0;
gint etch = 1;
gint do_draw_grid = FALSE;
extern gint poi_draw;
extern gint streets_draw;
extern gint tracks_draw;
gint drawmarkercounter = 0, loadpercent = 10, globruntime = 30;
extern int pleasepollme;


gint forcehavepos = FALSE, needreminder = TRUE;
gdouble alarm_lat = 53.583033, alarm_lon = 9.969533, alarm_dist = 9999999.0;
extern gchar cputempstring[20], batstring[20];
extern GtkWidget *tempeventbox, *batteventbox;
GtkWidget *sateventbox = NULL, *compasseventbox = NULL;
GtkWidget *wplabel1, *wplabel2, *wplabel3, *wplabel4, *wplabel5;
GtkWidget *wp1eventbox, *wp2eventbox, *wp3eventbox, *wp4eventbox;
GtkWidget *wp5eventbox, *satsvbox, *satshbox, *satslabel1eventbox;
GtkWidget *satslabel2eventbox, *satslabel3eventbox;
GtkWidget *satslabel1, *satslabel2, *satslabel3;
GtkWidget *frame_maptype;
GtkWidget *frame_status;
GtkWidget *frame_speed;
GtkWidget *frame_sats;
GtkWidget *frame_lat, *frame_lon, *frame_mapfile, *frame_mapscale;
GtkWidget *frame_heading, *frame_bearing, *frame_timedest, *frame_prefscale;
GtkWidget *frame_map_area, *frame_bearing, *frame_target, *frame_altitude;
GtkWidget *frame_wp;
GtkWidget *frame_toogles, *lab1;
GtkWidget *menubar;
gchar bluecolor[40], trackcolor[40], friendscolor[40];
gchar messagename[40], messagesendtext[1024], messageack[100];
GtkItemFactory *item_factory;
gint statuslock = 0, gpson = FALSE;
int messagenumber = 0, actualfriends = 0, didrootcheck = 0, haveserial = 0;
int gotneverserial = TRUE, timerto = 0, serialspeed = 1;
int disableserial = 1, disableserialcl = 0;
GtkTextBuffer *getmessagebuffer;
GdkColormap *cmap;
extern int newdata;
extern pthread_mutex_t mutex;
//int mapistopo = FALSE;
extern int havenasa;
int nosplash = FALSE;
int sortcolumn = 4, sortflag = 0;
int havedefaultmap = TRUE, alreadydaymode = FALSE, alreadynightmode = FALSE;

int storetz = FALSE;
int egnoson = 0, egnosoff = 0;
extern char actualstreetname[200];
int sound_direction = 1, sound_distance = 1, sound_speed = 1, sound_gps = 1;

#define DEG2RAD M_PI/180.0

// ---------------------- for nmea_handler.c
extern gint haveRMCsentence;
extern gchar nmeamodeandport[50];
extern gdouble NMEAsecs;
extern gint NMEAoldsecs;
extern FILE *nmeaout;
/*  if we get data from gpsd in NMEA format haveNMEA is TRUE */
/*  haveGARMIN is TRUE if we get data from program garble in GARMIN we get only long and lat */
extern gint haveNMEA, haveGARMIN;
#ifdef DBUS_ENABLE
extern gint useDBUS;
#endif
extern int nmeaverbose;
extern gint bigp , bigpGGA , bigpRME , bigpGSA, bigpGSV;
extern gint lastp, lastpGGA, lastpRME, lastpGSA, lastpGSV;

static GtkItemFactoryEntry main_menu[] = {
    {N_("/_Misc. Menu"),                    NULL, NULL,                     0, "<Branch>"},
    {N_("/_Misc. Menu/Maps"),               NULL, NULL,                     0, "<Branch>"},
    {N_("/_Misc. Menu/Maps/_Import map"),   NULL, (gpointer) import1_cb,    1, NULL},
    {N_("/_Misc. Menu/Maps/_Download map"), NULL, (gpointer) download_cb,   0, NULL},
    {N_("/_Misc. Menu/_Waypoint Manager"),  NULL, (gpointer) sel_target_cb, 0, NULL},
    {N_("/_Misc. Menu/_Reinitialize GPS"),  NULL, (gpointer) reinitgps_cb,  0, NULL},
    {N_("/_Misc. Menu/_Start gpsd"),        NULL, (gpointer) startgpsd,     0, NULL},
    {N_("/_Misc. Menu/_Load track file"),   NULL, (gpointer) loadtrack_cb,  0, "<StockItem>", GTK_STOCK_OPEN},
    {N_("/_Misc. Menu/Messages"),           NULL, NULL,                     0, "<Branch>"},
    {N_("/_Misc. Menu/Messages/Send message to mobile target"), 
                                            NULL, (gpointer) sel_message_cb,0,     NULL},
    {N_("/_Misc. Menu/Help"),               NULL, NULL,                     0, "<LastBranch>"},
    {N_("/_Misc. Menu/Help/About"),         NULL, (gpointer) about_cb,      0, "<StockItem>", GTK_STOCK_DIALOG_INFO},
    {N_("/_Misc. Menu/Help/Topics"),        NULL, (gpointer) help_cb,       0, "<StockItem>", GTK_STOCK_HELP}
};

void sql_load_lib();
void unit_test(void);
void drawdownloadrectangle (gint big);
GtkWidget * make_display_map_checkboxes();




/* *****************************************************************************
*****************************************************************************
*****************************************************************************
*/


/*  stolen from bluefish, translates the menu */
gchar *
menu_translate (const gchar * path, gpointer data)
{
	static gchar *menupath = NULL;

	gchar *retval;
	gchar *factory;

	factory = (gchar *) data;

	if (menupath)
		g_free (menupath);

	menupath = g_strdup (path);

	if ((strstr (path, "/tearoff1") != NULL)
	    || (strstr (path, "/---") != NULL)
	    || (strstr (path, "/MRU") != NULL))
		return menupath;

	retval = _(menupath);

	return retval;
}


/* *****************************************************************************
 * quit the program 
 */
gint
quit_program (GtkWidget * widget, gpointer datum)
{
	gtk_main_quit ();
	return (TRUE);
}



/* *****************************************************************************
 */
gint
error_popup (gpointer datum)
{
	GtkWidget *popup;
	GtkWidget *knopf2;
	GtkWidget *knopf3;
	GtkWidget *image;


	if ( mydebug >50 ) fprintf(stderr , "error_popup()\n");

	if (errortextmode)
	{
		g_print ("\nError: %s\n", (char *) datum);
		return (FALSE);
	}
	popup = gtk_dialog_new ();
	gtk_window_set_transient_for (GTK_WINDOW (popup),
				      GTK_WINDOW (mainwindow));

	image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR,
					  GTK_ICON_SIZE_DIALOG);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (popup)->vbox), image,
			    FALSE, FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (popup), 5);

	gtk_window_set_title (GTK_WINDOW (popup), _("  Message  "));
	knopf2 = gtk_button_new_from_stock (GTK_STOCK_OK);
	GTK_WIDGET_SET_FLAGS (knopf2, GTK_HAS_FOCUS);

	gtk_signal_connect_object (GTK_OBJECT (knopf2), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (popup));

	gdk_beep ();
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (popup)->action_area), knopf2,
			    TRUE, TRUE, 20);
	knopf3 = gtk_label_new (datum);
	gtk_label_set_use_markup (GTK_LABEL (knopf3), TRUE);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (popup)->vbox), knopf3, FALSE,
			    FALSE, 10);


	gtk_window_set_position (GTK_WINDOW (popup), GTK_WIN_POS_CENTER);
	gtk_widget_show (image);
	gtk_widget_show_all (popup);

	return (FALSE);

}


/* *****************************************************************************
 */

void
display_status (char *message)
{
    gchar tok[200];
    
    if ( mydebug >50 ) fprintf(stderr , "display_status(%s)\n",message);
    
    if (downloadactive == TRUE)
	return;
    if (importactive == TRUE)
	return;
    if (statuslock)
	return;
    g_snprintf (tok, sizeof (tok), "%s", message);
    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, tok);
}


/* *****************************************************************************
 *
 */
int
checkalarm (void)
{
	gdouble d;
	FILE *f;
	int pid, e;

	if ( mydebug >50 ) fprintf(stderr , "checkalarm()\n");

	d = calcdist (alarm_lon, alarm_lat);
	if (d < alarm_dist)
		return 0;

	fprintf (stderr,
		 _("Distance to HomeBase: %.1fkm, max. allowed: %.1fkm\n"), d,
		 alarm_dist);

	/* send sig USR1 to cammain 
	 * TODO: move PID's --> /var/run
	 * TODO: Explain what cammain is?
	 */
	f = fopen ("/tmp/cammain.pid", "r");
	if (f <= 0)
	{
	    fprintf (stderr, "no cammain running!\n");
	    return -1;
	}
	fscanf (f, "%d", &pid);
	e = kill (pid, SIGUSR1);
	if (e != 0)
	    fprintf (stderr, "sending signal failed!\n");

	return 0;
}


/* *****************************************************************************
 */
gint
lightoff (GtkWidget * widget, guint * datum)
{
	disableisnight = FALSE;
	/* gtk_widget_modify_bg (mainwindow, GTK_STATE_NORMAL, &nightcolor); */
	return FALSE;
}

/* *****************************************************************************
 */
gint
lighton (void)
{
	disableisnight = TRUE;
	/*   nighttimer = gtk_timeout_add (30000, (GtkFunction) lightoff, 0); */

	/*   gtk_widget_restore_default_style(mainwindow); */
	return TRUE;
}

/* *****************************************************************************
 */
gint
tripreset ()
{
	tripodometer = tripavspeed = triptime = tripmaxspeed = triptmp = 0.0;
	tripavspeedcount = 0;
	trip_lat = current_lat;
	trip_lon = current_lon;
	triptime = time (NULL);

	return TRUE;
}

/* ******************************************************************
 * TODO: This is a strange collection of function calls 
 * TODO: put them where they belong
 */
gint
testconfig_cb (GtkWidget * widget, guint * datum)
{
    if ( mydebug >50 ) fprintf(stderr , "testconfig_cb()\n");

#ifdef MAKETHISTEST
    test_loaded_map_names();
#endif
    
    friendsagent_cb (NULL, 0);
    tripreset ();
    gtk_timeout_add (TRIPMETERTIMEOUT * 1000, (GtkFunction) dotripmeter,
		     NULL);
    
    return FALSE;
}

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
	earthr = calcR (current_lat);

	if (importactive)
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
				tx = -current_lon + (wayp + i)->lon;
				ty = -current_lat + (wayp + i)->lat;
				radarbearing = atan (tx / ty);
				if (!finite (radarbearing))
					radarbearing = lastbearing;

				if (ty < 0)
					radarbearing = M_PI + radarbearing;
				radarbearing -= direction;
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
          (int) (ldist * 1000.0), (int) groundspeed );
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



/* *****************************************************************************
 * display upper status line 
 * speak how long it takes till we reach our destination
 */
void
display_status2 ()
{
	gchar s2[100], buf[200], mfmap_filename[60];
	gint h, m;
	gdouble secs, v;

	if ( mydebug >50 ) fprintf(stderr , "display_status2()\n");

	if (downloadactive == TRUE)
		return;
	if (importactive == TRUE)
		return;
	secs = g_timer_elapsed (disttimer, 0);
	h = hours;
	m = minutes;
	if (secs >= 300.0)
	{
		g_timer_stop (disttimer);
		g_timer_start (disttimer);
		v = 3600.0 * (olddist - dist) / secs;
		h = dist / v;
		m = 60 * (dist / v - h);
		if (mydebug)
			g_print ("secs: %.0fs,v:%.0f,h:%d,m:%d\n", secs, v, h,
				 m);
		if ((m < 0) || (h > 99))
		{
			h = 99;
			m = 99;
		}
		olddist = dist;
		hours = h;
		minutes = m;


    if( !muteflag )
    {
      if( hours < 99 )
      {
        if( hours > 0 )
        {
          if( 1 == hours )
          {
            g_snprintf(
              buf, sizeof(buf), speech_arrival_one_hour_mins[voicelang],
              minutes );
          }
          else
          {
            g_snprintf(
              buf, sizeof(buf), speech_arrival_hours_mins[voicelang], hours,
              minutes );
          }
        }
        else
        {
          g_snprintf(
            buf, sizeof(buf), speech_arrival_mins[voicelang], minutes );
        }

        speech_out_speek( buf );
	    }
    }
	}

	/* shows the current position on the bottom of the window */
	if (  posmode )
	    {		// Print out actual position of Mouse
		gdouble lat, lon;
		GdkModifierType state;
		gint x, y;
		
		gdk_window_get_pointer (drawing_area->window, &x, &y,&state);
		calcxytopos (x, y, &lat, &lon, zoom);
		if ( mydebug > 10 )
		    printf ("Actual mouse position: lat:%f,lon:%f (x:%d,y:%d)\n", lat, lon, x, y);
		// display position of Mouse in lat/lon Fields
		coordinate2gchar(s2, sizeof(s2), lat, TRUE, minsecmode);
		gtk_label_set_text (GTK_LABEL (label_lat), s2);
		coordinate2gchar(s2, sizeof(s2), lon, FALSE, minsecmode);
		gtk_label_set_text (GTK_LABEL (label_lon), s2);
	    }
	else 
	    {
		coordinate2gchar(s2, sizeof(s2), current_lat, TRUE, minsecmode);
		gtk_label_set_text (GTK_LABEL (label_lat), s2);
		coordinate2gchar(s2, sizeof(s2), current_lon, FALSE, minsecmode);
		gtk_label_set_text (GTK_LABEL (label_lon), s2);
	    }

	if ( mydebug > 10 )
	    {
		if (havepos)
		    g_print ("***Position: %f %f***\n", current_lat,
			     current_lon);
		else
		    g_print ("***no valid Position:\n");
	    }


	strncpy (mfmap_filename, g_basename (mapfilename), 59);
	mfmap_filename[59] = 0;
	if ( mydebug >10 )
	    gtk_label_set_text (GTK_LABEL (label_map_filename), mfmap_filename);

	g_snprintf (s2, sizeof (s2), "1:%ld", mapscale);
	gtk_label_set_text (GTK_LABEL (label_map_scale), s2);

	g_snprintf (s2, sizeof (s2), "%3.0f%s", direction * 180.0 / M_PI,
		    gradsym);
	gtk_label_set_text (GTK_LABEL (label_heading), s2);

	g_snprintf (s2, sizeof (s2), "%3.0f%s", bearing * 180.0 / M_PI,
		    gradsym);
	gtk_label_set_text (GTK_LABEL (label_baering), s2);

	g_snprintf (s2, sizeof (s2), "%2d:%02dh", h, m);
	gtk_label_set_text (GTK_LABEL (label_timedest), s2);

	if (scaleprefered)
		g_snprintf (s2, sizeof (s2), "1:%d", scalewanted);
	else
		g_snprintf (s2, sizeof (s2), _("Auto"));
	gtk_label_set_text (GTK_LABEL (label_prefscale), s2);
}


/* *****************************************************************************
 * draw the marker on the map 
 * calculate CPU load: loadpercent
 * check night mode switching
 * check for new map
 * TODO: eventually split this callback or rename it
 */
gint
drawmarker_cb (GtkWidget * widget, guint * datum)
{
	static struct timeval tv1, tv2;
	struct timezone tz;
	long runtime, runtime2;

	if ( mydebug >50 ) fprintf(stderr , "drawmacker_cb()\n");

	if (importactive)
		return TRUE;

	if ((!disableisnight) && (!downloadwindowactive))
	{
		if (((nightmode == 1) || ((nightmode == 2) && isnight))
		    && (!alreadynightmode))
		{
			gtk_widget_modify_bg (mainwindow, GTK_STATE_NORMAL,
					      &nightcolor);
			alreadydaymode = FALSE;
			alreadynightmode = TRUE;
			if (mydebug>1)
				fprintf (stderr, "setting to night view\n");
		}
	}

	if (((nightmode == 0) || disableisnight || downloadwindowactive)
	    && (!alreadydaymode))
	{
		gtk_widget_modify_bg (mainwindow, GTK_STATE_NORMAL,
				      &defaultcolor);
		if ( mydebug > 10 )
			fprintf (stderr, "setting to daylight view\n");
		alreadydaymode = TRUE;
		alreadynightmode = FALSE;
	}


	gettimeofday (&tv1, &tz);
	runtime2 = tv1.tv_usec - tv2.tv_usec;
	if (tv1.tv_sec != tv2.tv_sec)
		runtime2 += 1000000l * (tv1.tv_sec - tv2.tv_sec);

	/* we test if we have to load a new map because we are outside 
	 * the currently loaded map 
	 */
	testnewmap ();

	/*   g_print("drawmarker_cb %d\n",drawmarkercounter++); */
	exposed = FALSE;
	/* The position calculation is made in expose_cb() */
	gtk_widget_draw (drawing_area, NULL);	/* this  calls expose handler */
	if (!exposed)
		expose_cb (NULL, 0);
	expose_compass (NULL, 0);

	gettimeofday (&tv2, &tz);
	runtime = tv2.tv_usec - tv1.tv_usec;
	if (tv2.tv_sec != tv1.tv_sec)
		runtime += 1000000l * (tv2.tv_sec - tv1.tv_sec);
	globruntime = runtime / 1000;
	loadpercent = (int) (100.0 * (float) runtime / (runtime + runtime2));
	if ( mydebug>30 )
		g_print ("Rechenzeit: %dms, %d%%\n", (int) (runtime / 1000),
			 loadpercent);

	return FALSE;
}


/* *****************************************************************************
 * Try to produce only given CPU load  
 */
gint
calldrawmarker_cb (GtkWidget * widget, guint * datum)
{
	gint period;

	if ( mydebug >50 ) fprintf(stderr , "calldrawmarker_cb()\n");

	if (cpuload < 1)
		cpuload = 1;
	if (cpuload > 95)
		cpuload = 95;
	if (!haveNMEA)
		expose_sats_cb (NULL, 0);
	if (pleasepollme)
	{
		pleasepollme++;
		if (pleasepollme > 10)
		{
			pleasepollme = 1;
			friends_sendmsg (friendsserverip, NULL);
		}
	}
	period = 10 * globruntime / (10 * cpuload);
	drawmarkercounter++;
	/*   g_print("period: %d, drawmarkercounter %d\n", period, drawmarkercounter);  */

	if (((drawmarkercounter > period) || (drawmarkercounter > 50))
	    && (drawmarkercounter >= 3))
	{
		drawmarkercounter = 0;
		drawmarker_cb (NULL, NULL);
	}
	return TRUE;
}


/* *****************************************************************************
 */
/* Friends agent */
gint
friendsagent_cb (GtkWidget * widget, guint * datum)
{
	time_t tii;
	gchar buf[MAXMESG], buf2[40], la[20], lo[20], num[5];
	gint i;

	if ( mydebug >50 ) fprintf(stderr , "friendsagent_cb()\n");

	/* Don't allow spaces in name */
	for (i = 0; (size_t) i < strlen (friendsname); i++)
		if (friendsname[i] == ' ')
			friendsname[i] = '_';

	/*  send position to friendsserver */

	if (havefriends)
	{
		if (strlen (messagesendtext) > 0)
		{
			/* send message to server */
			if (messagenumber < 99)
				messagenumber++;
			else
				messagenumber = 0;
			needtosave = TRUE;
			g_snprintf (num, sizeof (num), "%02d", messagenumber);
			g_strlcpy (buf2, friendsidstring, sizeof (buf2));
			buf2[0] = 'M';
			buf2[1] = 'S';
			buf2[2] = 'G';
			buf2[3] = num[0];
			buf2[4] = num[1];
			g_snprintf (buf, sizeof (buf), "SND: %s %s %s\n",
				    buf2, messagename, messagesendtext);
			if ( mydebug > 3 )
				fprintf (stderr,
					 "friendsagent: sending to %s:\nfriendsagent: %s\n",
					 friendsserverip, buf);
			if (sockfd != -1)
				close (sockfd);
			sockfd = -1;
			friends_sendmsg (friendsserverip, buf);
			g_snprintf (messageack, sizeof (messageack),
				    "SND: %s", buf2);
		}
		else
		{
			/* send position to server */
			g_snprintf (la, sizeof (la), "%10.6f", current_lat);
			g_snprintf (lo, sizeof (lo), "%10.6f", current_lon);
			g_strdelimit (la, ",", '.');
			g_strdelimit (lo, ",", '.');
			tii = time (NULL);
			g_snprintf (buf, sizeof (buf),
				    "POS: %s %s %s %s %ld %.0f %.0f",
				    friendsidstring, friendsname, la, lo, tii,
				    groundspeed / milesconv,
				    180.0 * direction / M_PI);
			if ( mydebug > 3 )
				fprintf (stderr,
					 "friendsagent: sending to %s:\nfriendsagent: %s\n",
					 friendsserverip, buf);
			if (sockfd != -1)
				close (sockfd);
			sockfd = -1;
			friends_sendmsg (friendsserverip, buf);
		}
	}

	return TRUE;
}


void check_and_reload_way_txt()
{
    gchar mappath[2048];
    g_strlcpy (mappath, homedir, sizeof (mappath));

    if (!wp_from_sql)
	g_strlcat (mappath, activewpfile, sizeof (mappath));
    else
	g_strlcat (mappath, "way-SQLRESULT.txt", sizeof (mappath));

    loadwaypoints ();
}


/* *****************************************************************************
 * Master agent 
 */
gint
masteragent_cb (GtkWidget * widget, guint * datum)
{
	gchar buffer[200];

	if ( mydebug >50 ) fprintf(stderr , "masteragent_cb()\n");

	if (needtosave)
		writeconfig ();


	map_koord_check_and_reload();

	if (needreminder)
	{
		reminder_cb (NULL, 0);
		needreminder = FALSE;
	}
	checkalarm ();

	testifnight ();


	if (!didrootcheck)
		if (getuid () == 0)
		{
			g_snprintf (buffer, sizeof (buffer),
				    "<span  weight=\"bold\">%s</span>\n%s",
				    _("Warning!"),
				    _
				    ("You should not start GpsDrive as user root!!!"));
			error_popup (buffer);
			didrootcheck = TRUE;
		}

	/* Check for changed way.txt and reload if changed */
	check_and_reload_way_txt();

	if (tracknr > (tracklimit - 1000))
	{
		g_print ("tracklimit: %ld", tracklimit);
		track = g_renew (GdkSegment, track, tracklimit + 100000);
		trackshadow =
			g_renew (GdkSegment, trackshadow,
				 tracklimit + 100000);
		tracklimit += 100000;
	}
	if (trackcoordnr > (trackcoordlimit - 1000))
	{
		trackcoord =
			g_renew (trackcoordstruct, trackcoord,
				 trackcoordlimit + 100000);
		trackcoordlimit += 100000;
	}



	return TRUE;
}

/* *****************************************************************************
 * add new trackpoint to  'trackcoordstruct list' to draw track on image 
 */
gint
storetrack_cb (GtkWidget * widget, guint * datum)
{
    if ( mydebug >50 ) 
	fprintf(stderr , "storetrack_cb()\n");

    if (posmode) 
	return TRUE;
	
#ifdef DBUS_ENABLE
	/* If we use DBUS track points are usually stored by the DBUS signal handler */
	/* Only store them by timer if we are in position mode */
	if ( useDBUS && !simmode )
	    return TRUE;
#endif

	storepoint();

	return TRUE;
}

void
storepoint ()
{
	gint so;
	gchar buf3[35];
	time_t t;
	struct tm *ts;
	/*    g_print("Havepos: %d\n", havepos); */
	if ((!simmode && !havepos) || posmode /*  ||((!simmode &&haveposcount<3)) */ )	/* we have no valid position */
	{
		(trackcoord + trackcoordnr)->lon = 1001.0;
		(trackcoord + trackcoordnr)->lat = 1001.0;
		(trackcoord + trackcoordnr)->alt = 1001.0;
	}
	else
	{
		(trackcoord + trackcoordnr)->lon = current_lon;
		(trackcoord + trackcoordnr)->lat = current_lat;
		(trackcoord + trackcoordnr)->alt = altitude;
		if (savetrack != 0) do_incremental_save();
	}


	/*  The double storage seems to be silly, but I have to use  */
	/*  gdk_draw_segments instead of gdk_draw_lines.   */
	/*  gkd_draw_lines is dog slow because of a gdk-bug. */

	if (tracknr == 0)
	{
		if ((trackcoord + trackcoordnr)->lon < 1000.0)
		{
			(track + tracknr)->x1 = posx;
			(track + tracknr)->y1 = posy;
			(trackshadow + tracknr)->x1 = posx + SHADOWOFFSET;
			(trackshadow + tracknr)->y1 = posy + SHADOWOFFSET;
			tracknr++;
		}
	}
	else
	{
		if ((trackcoord + trackcoordnr)->lon < 1000.0)
		{
			if ((posx != (track + tracknr - 1)->x2)
			    || (posy != (track + tracknr - 1)->y2))
			{
				/* so=(int)(((trackcoord + trackcoordnr)->alt))>>5; */
				so = SHADOWOFFSET;
				(track + tracknr)->x1 =
					(track + tracknr - 1)->x2 = posx;
				(track + tracknr)->y1 =
					(track + tracknr - 1)->y2 = posy;
				(trackshadow + tracknr)->x1 =
					(trackshadow + tracknr - 1)->x2 =
					posx + so;
				(trackshadow + tracknr)->y1 =
					(trackshadow + tracknr - 1)->y2 =
					posy + so;
				tracknr += 1;
			}
		}
		else
			tracknr = tracknr & ((glong) - 2);
	}
	time (&t);
	ts = localtime (&t);
	strncpy (buf3, asctime (ts), 32);
	buf3[strlen (buf3) - 1] = '\0';	/* get rid of \n */
	g_strlcpy ((trackcoord + trackcoordnr)->postime, buf3, 30);
	trackcoordnr++;
}

/* *****************************************************************************
 * draw wlan Waypoints
 */
void
drawwlan (gint posxdest, gint posydest, gint wlan)
{
    /*  wlan=0: no wlan, 1:open wlan, 2:WEP crypted wlan */
    
    if (wlan == 0)
	return;
    
    if ((posxdest >= 0) && (posxdest < SCREEN_X))
	{
	    if ((posydest >= 0) && (posydest < SCREEN_Y))
		{
		    if (wlan == 1)
			drawicon(posxdest,posydest,"w-lan.open");
		    else 
			drawicon(posxdest,posydest,"w-lan.wep");
		}
	}
}

/* *****************************************************************************
 */
void
drawfriends (void)
{
	gint i;
	gdouble posxdest, posydest, clong, clat, direction;
	gint width, height;
	gdouble w;
	GdkPoint poly[16];
	struct tm *t;
	time_t ti, tif;
#define PFSIZE 55

	actualfriends = 0;
	/*   g_print("Maxfriends: %d\n",maxfriends); */
	for (i = 0; i < maxfriends; i++)
	{

		/* return if too old  */
		ti = time (NULL);
		tif = atol ((friends + i)->timesec);
		if (!(tif > 1000000000))
			fprintf (stderr,
				 "Format error! timesec: %s, Name: %s, i: %d\n",
				 (friends + i)->timesec, (friends + i)->name,
				 i);
		if ((ti - maxfriendssecs) > tif)
			continue;
		actualfriends++;
		coordinate_string2gdouble( (friends + i)->lon, &clong );
		coordinate_string2gdouble( (friends + i)->lat,   &clat  );

		calcxy (&posxdest, &posydest, clong, clat, zoom);

		/* If Friend is visible inside SCREEN display him/her */
		if ((posxdest >= 0) && (posxdest < SCREEN_X))
		{

			if ((posydest >= 0) && (posydest < SCREEN_Y))
			{

				gdk_draw_pixbuf (drawable, kontext,
						 friendspixbuf, 0, 0,
						 posxdest - 18, posydest - 12,
						 39, 24, GDK_RGB_DITHER_NONE,
						 0, 0);
				gdk_gc_set_line_attributes (kontext, 4, 0, 0,
							    0);

				/*  draw pointer to direction */
				direction =
					strtod ((friends + i)->heading,
						NULL) * M_PI / 180.0;
				w = direction + M_PI;
				gdk_gc_set_line_attributes (kontext, 2, 0, 0,
							    0);
				poly[0].x =
					posxdest +
					(PFSIZE) / 2.3 * (cos (w + M_PI_2));
				poly[0].y =
					posydest +
					(PFSIZE) / 2.3 * (sin (w + M_PI_2));
				poly[1].x =
					posxdest +
					(PFSIZE) / 9 * (cos (w + M_PI));
				poly[1].y =
					posydest +
					(PFSIZE) / 9 * (sin (w + M_PI));
				poly[2].x =
					posxdest +
					PFSIZE / 10 * (cos (w + M_PI_2));
				poly[2].y =
					posydest +
					PFSIZE / 10 * (sin (w + M_PI_2));
				poly[3].x =
					posxdest -
					(PFSIZE) / 9 * (cos (w + M_PI));
				poly[3].y =
					posydest -
					(PFSIZE) / 9 * (sin (w + M_PI));
				poly[4].x = poly[0].x;
				poly[4].y = poly[0].y;
				gdk_gc_set_foreground (kontext, &blue);
				gdk_draw_polygon (drawable, kontext, 0,
						  (GdkPoint *) poly, 5);
				gdk_draw_arc (drawable, kontext, 0,
					      posxdest + 2 - 7,
					      posydest + 2 - 7, 10, 10, 0,
					      360 * 64);

				/*   draw + sign at destination   */
				gdk_gc_set_foreground (kontext, &red);
				gdk_draw_line (drawable, kontext,
					       posxdest + 1, posydest + 1 - 5,
					       posxdest + 1,
					       posydest + 1 + 5);
				gdk_draw_line (drawable, kontext,
					       posxdest + 1 + 5, posydest + 1,
					       posxdest + 1 - 5,
					       posydest + 1);

				{	/* print friends name / speed on map */
					PangoFontDescription *pfd;
					PangoLayout *wplabellayout;
					gchar txt[200], txt2[100], s1[10];
					time_t sec;
					char *as, day[20], dispname[40];
					int speed, ii;

					sec = atol ((friends + i)->timesec);
					sec += 3600 * zone;
					t = gmtime (&sec);

					as = asctime (t);
					sscanf (as, "%s", day);
					sscanf ((friends + i)->speed, "%d",
						&speed);

					/* replace _ with  spaces in name */
					g_strlcpy (dispname,
						   (friends + i)->name,
						   sizeof (dispname));
					for (ii = 0;
					     (size_t) ii < strlen (dispname);
					     ii++)
						if (dispname[ii] == '_')
							dispname[ii] = ' ';

					g_snprintf (txt, sizeof (txt),
						    "%s,%d", dispname,
						    (int) (speed *
							   milesconv));
					if (milesflag)
						g_snprintf (s1, sizeof (s1),
							    "%s", _("mi/h"));
					else if (nauticflag)
						g_snprintf (s1, sizeof (s1),
							    "%s", _("knots"));
					else
						g_snprintf (s1, sizeof (s1),
							    "%s", _("km/h"));
					g_strlcat (txt, s1, sizeof (txt));
					g_snprintf (txt2, sizeof (txt2),
						    "%s, %2d:%02d\n", day,
						    t->tm_hour, t->tm_min);
					g_strlcat (txt, txt2, sizeof (txt));
					wplabellayout =
						gtk_widget_create_pango_layout
						(drawing_area, txt);
					if (pdamode)
						pfd = pango_font_description_from_string ("Sans 8");
					else
						pfd = pango_font_description_from_string ("Sans bold 11");
					pango_layout_set_font_description
						(wplabellayout, pfd);
					pango_layout_get_pixel_size
						(wplabellayout, &width,
						 &height);
					gdk_gc_set_foreground (kontext,
							       &textbacknew);
					/*              gdk_draw_rectangle (drawable, kontext, 1, posxdest + 18,
					 *                                  posydest - height/2 , width + 2,
					 *                                  height + 2);
					 */

					gdk_draw_layout_with_colors (drawable,
								     kontext,
								     posxdest
								     + 21,
								     posydest
								     -
								     height /
								     2 + 1,
								     wplabellayout,
								     &black,
								     NULL);
					gdk_draw_layout_with_colors (drawable,
								     kontext,
								     posxdest
								     + 20,
								     posydest
								     -
								     height /
								     2,
								     wplabellayout,
								     &orange,
								     NULL);

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
 * show satelite information
 * TODO: change color of sat BARs if the reception is bad/acceptable/good to red/yellow/green
 */
gint
expose_sats_cb (GtkWidget * widget, guint * datum)
{
	gint k, i, yabs, h, j, l;
	static GdkGC *mykontext = NULL;
	gchar t[10], t1[20], buf[300], txt[10];
	static GdkPixbufAnimation *anim = NULL;
	static GdkPixbufAnimationIter *animiter = NULL;
	GTimeVal timeresult;
#define SATX 5
	/*  draw satellite level (field strength) only in NMEA modus */

	if ( mydebug >50 ) fprintf(stderr , "expose_stats_cb()\n");


	if (haveNMEA)
	{
		gdk_gc_set_foreground (kontext, &lcd);

		gdk_draw_rectangle (drawable_sats, kontext, 1, 3, 0,
				    PSIZE + 2, PSIZE + 5);
		gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
		gdk_gc_set_foreground (kontext, &black);
		gdk_draw_rectangle (drawable_sats, kontext, 0, 3, 0,
				    PSIZE + 2, PSIZE + 5);

		gdk_gc_set_foreground (kontext, &lcd);

		if (!satposmode)
		{
			if (havepos)
				gdk_gc_set_foreground (kontext, &green);
			else
				gdk_gc_set_foreground (kontext, &red);
			gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
			for (i = 3; i < (PSIZE + 5); i += 3)
				gdk_draw_line (drawable_sats, kontext, 4, i,
					       PSIZE + 4, i);


			gdk_gc_set_foreground (kontext, &lcd2);
			gdk_gc_set_line_attributes (kontext, 5, 0, 0, 0);
			k = 0;
			for (i = 0; i < 16; i++)
			{
				if (i > 5)
					yabs = 2 + PSIZE;
				else
					yabs = 1 + PSIZE / 2;
				h = PSIZE / 2 - 2;
				gdk_draw_line (drawable_sats, kontext,
					       6 + 7 * k + SATX, yabs,
					       6 + 7 * k + SATX, yabs - h);
				k++;
				if (k > 5)
					k = 0;
			}
		}
		if (satfix == 1)	/* normal Fix */
			gdk_gc_set_foreground (kontext, &black);
		else
		{
			if (satfix == 0)	/* no Fix */
				gdk_gc_set_foreground (kontext, &textback);
			else	/* Differntial Fix */
				gdk_gc_set_foreground (kontext, &blue);
		}
		j = k = 0;

		if (satposmode)
		{
			gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
			gdk_gc_set_foreground (kontext, &lcd2);
			gdk_draw_arc (drawable_sats, kontext, 0, 4,
				      4, PSIZE, PSIZE, 105 * 64, 330 * 64);
			gdk_draw_arc (drawable_sats, kontext, 0,
				      5 + PSIZE / 4, 4 + PSIZE / 4, PSIZE / 2,
				      PSIZE / 2, 0, 360 * 64);
			gdk_gc_set_foreground (kontext, &darkgrey);
			{
				/* prints in pango */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;
				gint width;

				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_sats, "N");
				//KCFX  
				if (pdamode)
					pfd = pango_font_description_from_string ("Sans 7");
				else
					pfd = pango_font_description_from_string ("Sans bold 10");
				pango_layout_set_font_description
					(wplabellayout, pfd);

				gdk_draw_layout_with_colors (drawable_sats,
							     kontext,
							     0 + (PSIZE) / 2,
							     -2,
							     wplabellayout,
							     &grey, NULL);
				pango_layout_get_pixel_size (wplabellayout,
							     &width, NULL);
				/*          printf ("w: %d\n", width);  */
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}

			/*    gdk_draw_text (drawable_sats, verysmalltextfont, kontext,
			 *                   2 + (PSIZE) / 2, 9, "N", 1);
			 */
			gdk_gc_set_foreground (kontext, &lcd2);

		}

		for (i = 0; i < MAXSATS; i++)
			if (satlistdisp[i][0] != 0)
			{
				if ((satlistdisp[i][1] > 30)
				    && (printoutsats))
					g_print ("%d %d\n", satlistdisp[i][3],
						 satlistdisp[i][2]);
				if (satposmode)
				{
					gint x, y;
					gdouble el, az;
					el = (90.0 - satlistdisp[i][2]);
					az = satlistdisp[i][3] * DEG2RAD;

					x = (PSIZE / 2) +
						sin (az) * (el / 90.0) *
						(PSIZE / 2);
					y = (PSIZE / 2) -
						cos (az) * (el / 90.0) *
						(PSIZE / 2);
					l = (satlistdisp[i][1] / 6);
					if (l > 7)
						l = 7;
					switch (l & 7)
					{
					case 0:
					case 1:
						gdk_gc_set_foreground
							(kontext, &textback);
						break;
					case 2:
					case 3:
						gdk_gc_set_foreground
							(kontext, &red);
						break;
					case 4:
					case 5:
					case 6:
						gdk_gc_set_foreground
							(kontext, &yellow);
						break;
					case 7:
						gdk_gc_set_foreground
							(kontext, &green2);
						break;
					}
					gdk_draw_arc (drawable_sats, kontext,
						      1, 2 + x, 2 + y, 5, 5,
						      0, 360 * 64);

				}
				else
				{
					if (j > 5)
						yabs = PSIZE + 2;
					else
						yabs = 1 + PSIZE / 2;
					h = satlistdisp[i][1] - 30;
					if (h < 0)
						h = 1;
					if (h > 19)
						h = 19;
					gdk_draw_line (drawable_sats, kontext,
						       6 + 7 * k + SATX, yabs,
						       6 + 7 * k + SATX,
						       yabs -
						       (PSIZE / 2) * h /
						       (PSIZE / 2 - 5));
					k++;
					if (k > 5)
						k = 0;
					j++;
				}
			}
		newsatslevel = FALSE;
		g_snprintf (txt, sizeof (txt), "%d/%d", numsats, satsavail);
		gtk_entry_set_text (GTK_ENTRY (satslabel1), txt);
		if ((precision > 0.0) && (satfix != 0))
		{
			if (milesflag || nauticflag)
				g_snprintf (t1, sizeof (t1), "%.0fft",
					    precision * 3.2808399);
			else
				g_snprintf (t1, sizeof (t1), "%.0fm",
					    precision);
			g_snprintf (t, sizeof (t), "%s", t1);
		}
		else
			g_snprintf (t, sizeof (t), "n/a");

		gtk_entry_set_text (GTK_ENTRY (satslabel2), t);
		if ((gsaprecision > 0.0) && (satfix != 0))
		{
			g_snprintf (t1, sizeof (t1), "%.1f", gsaprecision);
			g_snprintf (t, sizeof (t), "%s", t1);
		}
		else
			g_snprintf (t, sizeof (t), "n/a");
		gtk_entry_set_text (GTK_ENTRY (satslabel3), t);

		g_strlcpy (buf, "", sizeof (buf));
		if ( mydebug > 30 )
			g_print ("gpsd: Satfix: %d\n", satfix);

    if (satfix != oldsatfix)
    {
      if( !muteflag && sound_gps )
      {
        if( 0 == satfix )
        {
          g_snprintf( buf, sizeof(buf), speech_gps_lost[voicelang] );
        }
        else if( 1 == satfix )
        {
          if( 2 == oldsatfix )
          {
            g_snprintf( buf, sizeof(buf), speech_diff_gps_lost[voicelang] );
          }
          else
          {
            g_snprintf( buf, sizeof(buf), speech_gps_good[voicelang] );
          }
        }
        else if( 2 == satfix )
        {
          g_snprintf( buf, sizeof(buf), speech_diff_gps_found[voicelang] );
        }

        speech_out_speek( buf );
      }

      oldsatfix = satfix;
    }
	}
	else
	{
		if (mykontext == NULL)
			mykontext = gdk_gc_new (drawable_sats);
		if (satsimage == NULL)
		{
			gchar mappath[2048];

			g_snprintf (mappath, sizeof (mappath),
				    "%s/gpsdrive/%s", DATADIR,
				    "pixmaps/gpsdriveanim.gif");
			anim = gdk_pixbuf_animation_new_from_file (mappath,
								   NULL);
			if (anim == NULL)
				fprintf (stderr,
					 _
					 ("\nWarning: unable to load gpsdriveanim.gif!\n"
					  "Please install the program as root with:\nmake install\n\n"));
			g_get_current_time (&timeresult);
			if (animiter != NULL)
				g_object_unref (animiter);
			animiter =
				gdk_pixbuf_animation_get_iter (anim,
							       &timeresult);
			satsimage =
				gdk_pixbuf_animation_iter_get_pixbuf
				(animiter);
		}
		if (gdk_pixbuf_animation_iter_advance (animiter, NULL))
			satsimage =
				gdk_pixbuf_animation_iter_get_pixbuf
				(animiter);
		gdk_gc_set_function (mykontext, GDK_AND);
		gdk_draw_pixbuf (drawable_sats, kontext, satsimage, 0, 0,
				 SATX, 0, 50, 50, GDK_RGB_DITHER_NONE, 0, 0);
		gdk_gc_set_function (mykontext, GDK_COPY);
	}
	return TRUE;
}

/* *****************************************************************************
 * Draw Text (lat/lon) into Grid
 */
void
draw_grid_text (GtkWidget * widget, gdouble posx, gdouble posy, gchar * txt)
{
	/* prints in pango */
	PangoFontDescription *pfd;
	PangoLayout *grid_label_layout;
	gint width, height;

	grid_label_layout = gtk_widget_create_pango_layout (drawing_area, txt);

	pfd = pango_font_description_from_string ("Sans 6");

	pango_layout_set_font_description (grid_label_layout, pfd);
	pango_layout_get_pixel_size (grid_label_layout, &width, &height);
	gdk_gc_set_function (kontext, GDK_XOR);
	gdk_gc_set_background (kontext, &white);
	gdk_gc_set_foreground (kontext, &mygray);
	
	gdk_draw_layout_with_colors (drawable, kontext, posx - width / 2,
				     posy - height / 2, grid_label_layout, &black,
				     NULL);

	if (grid_label_layout != NULL)
		g_object_unref (G_OBJECT (grid_label_layout));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);

}

/* *****************************************************************************
 */
/*
 * Draw a grid over the map
 */
void
draw_grid (GtkWidget * widget)
{
	int count;
	gdouble step;
	gdouble lat, lon;
	gdouble lat_ul, lon_ul;
	gdouble lat_ll, lon_ll;
	gdouble lat_ur, lon_ur;
	gdouble lat_lr, lon_lr;

	gdouble lat_min, lon_min;
	gdouble lat_max, lon_max;

	if ( mydebug >50 ) 
	    fprintf(stderr , "draw_grid()\n");


	// calculate the start and stop for lat/lon according to the displayed section
	calcxytopos (0, 0, &lat_ul, &lon_ul, zoom);
	calcxytopos (0, SCREEN_Y, &lat_ll, &lon_ll, zoom);
	calcxytopos (SCREEN_X, 0, &lat_ur, &lon_ur, zoom);
	calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, zoom);

	// add more lines as the scale increases

	// Calculate distance between grid lines
	step = (gdouble) mapscale / 2000000.0 / zoom;
	gchar precission[10];
	if (step >= 1)	            g_snprintf (precission, sizeof (precission), "%%.0f");
	else if (step >= .1)	    g_snprintf (precission, sizeof (precission), "%%.1f");
	else if (step >= .01)	    g_snprintf (precission, sizeof (precission), "%%.2f");
	else if (step >= .001)	    g_snprintf (precission, sizeof (precission), "%%.3f");
	else if (step >= .0001)	    g_snprintf (precission, sizeof (precission), "%%.4f");
	else           		    g_snprintf (precission, sizeof (precission), "%%.5f");

	if (mapscale < 5000000)
	{
		lat_min = min (lat_ll, lat_ul) - step;
		lat_max = max (lat_lr, lat_ur) + step;
		lon_min = min (lon_ll, lon_ul) - step;
		lon_max = max (lon_lr, lon_ur) + step;
	}
	else
	{
		lat_min = -90;
		lat_max = 90;
		lon_min = -180;
		lon_max = 180;
	}
	lat_min = floor (lat_min / step) * step;
	lon_min = floor (lon_min / step) * step;

	if ( mydebug > 20 )
		printf ("Draw Grid: (%.2f,%.2f) - (%.2f,%.2f)\n", lat_min,
			lon_min, lat_max, lon_max);


	// Loop over desired lat/lon  
	count = 0;
	for (lon = lon_min; lon <= lon_max; lon = lon + step)
	    {
		for (lat = lat_min; lat <= lat_max; lat = lat + step)
		    {
			gdouble posxdest11, posydest11;
			gdouble posxdest12, posydest12;
			gdouble posxdest21, posydest21;
			gdouble posxdest22, posydest22;
			gdouble posxdist, posydist;
			gchar str[200];

			count++;
			calcxy (&posxdest11, &posydest11, lon,        max(-90,lat)       , zoom);
			calcxy (&posxdest12, &posydest12, lon,        min( 90,lat + step), zoom);
			calcxy (&posxdest21, &posydest21, lon + step, max(-90,lat),        zoom);
			calcxy (&posxdest22, &posydest22, lon + step, min( 90,lat + step), zoom);

			if (((posxdest11 >= 0) && (posxdest11 < SCREEN_X) &&
			     (posydest11 >= 0) && (posydest11 < SCREEN_Y))
			    ||
			    ((posxdest22 >= 0) && (posxdest22 < SCREEN_X) &&
			     (posydest22 >= 0) && (posydest22 < SCREEN_Y))
			    ||
			    ((posxdest21 >= 0) && (posxdest21 < SCREEN_X) &&
			     (posydest21 >= 0) && (posydest21 < SCREEN_Y))
			    ||
			    ((posxdest12 >= 0) && (posxdest12 < SCREEN_X) &&
			     (posydest12 >= 0) && (posydest12 < SCREEN_Y)))
			    {
				// TODO: add linethikness 2 for Mayor Lines
				// Set Drawing Mode
				gdk_gc_set_function (kontext, GDK_XOR);
				gdk_gc_set_foreground (kontext, &darkgrey);
				gdk_gc_set_line_attributes (kontext, 1, GDK_LINE_SOLID, 0, 0);

				gdk_draw_line (drawable, kontext, posxdest11,
					       posydest11, posxdest21,
					       posydest21);
				gdk_draw_line (drawable, kontext, posxdest11,
					       posydest11, posxdest12,
					       posydest12);
			    
				// Text lon
				g_snprintf (str, sizeof (str), precission,lon);

				posxdist = (posxdest12 - posxdest11) / 4;
				posydist = (posydest12 - posydest11) / 4;
				draw_grid_text (widget, 
						posxdest11 + posxdist,
						posydest11 + posydist, str);
				
				// Text lat
				g_snprintf (str, sizeof (str), precission,lat);
					    
				posxdist = (posxdest21 - posxdest11) / 4;
				posydist = (posydest21 - posydest11) / 4;
				draw_grid_text (widget, 
						posxdest11 + posxdist,
						posydest11 + posydist-5, str);
			    }
		    }
	    }
	if ( mydebug > 30 )
	    printf ("draw_grid loops: %d\n", count);
}

/* *****************************************************************************
 * Draw waypoints on map
 */
// TODO: Put this in its own file
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
			(wayp + i)->lon, (wayp + i)->lat, zoom);

		if ((posxdest >= 0) && (posxdest < SCREEN_X)
		    && (shownwp < MAXSHOWNWP)
		    && (posydest >= 0) && (posydest < SCREEN_Y))
		{
			gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
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
				if (mapscale)
					proximity_pixels =
						((wayp + i)->proximity)
						* zoom * PIXELFACT / mapscale;
				else
					proximity_pixels = 2;

				gdk_gc_set_foreground (kontext, &blue);

				gdk_draw_arc (drawable, kontext, FALSE,
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

				gdk_gc_set_foreground (kontext, &darkgrey);
				gdk_gc_set_function (kontext, GDK_AND);
				tn = g_strdelimit (txt, "_", ' ');

				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_area, tn);
				pfd = pango_font_description_from_string
					(wplabelfont);
				pango_layout_set_font_description
					(wplabellayout, pfd);
				pango_layout_get_pixel_size (wplabellayout,
							     &width, &height);
				/* printf("j: %d\n",height);    */
				k = width + 4;
				k2 = height;
				if (shadow)
				{
					gdk_draw_layout_with_colors (drawable,
								     kontext,
								     posxdest
								     + 15 +
								     SHADOWOFFSET,
								     posydest
								     -
								     k2 / 2 +
								     SHADOWOFFSET,
								     wplabellayout,
								     &darkgrey,
								     NULL);
				}
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}
			gdk_gc_set_function (kontext, GDK_COPY);


			gdk_gc_set_function (kontext, GDK_AND);

			gdk_gc_set_foreground (kontext, &textbacknew);
			gdk_draw_rectangle (drawable, kontext, 1,
					    posxdest + 13, posydest - k2 / 2,
					    k + 1, k2);
			gdk_gc_set_function (kontext, GDK_COPY);
			gdk_gc_set_foreground (kontext, &black);
			gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
			gdk_draw_rectangle (drawable, kontext, 0,
					    posxdest + 12,
					    posydest - k2 / 2 - 1, k + 2, k2);

			/*            gdk_gc_set_foreground (kontext, &yellow);  */
			{
				/* prints in pango */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;

				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_area, txt);
				pfd = pango_font_description_from_string
					(wplabelfont);
				pango_layout_set_font_description
					(wplabellayout, pfd);

				gdk_draw_layout_with_colors (drawable,
							     kontext,
							     posxdest + 15,
							     posydest -
							     k2 / 2,
							     wplabellayout,
							     &white, NULL);
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}

			/*              gdk_draw_text (drawable, smalltextfont, kontext,
			 *                           posxdest + 13, posydest + 6, txt,
			 *                           strlen (txt));
			 */
		}
	}
}

/* *****************************************************************************
 */
void
draw_zoom_scale (void)
{
	gint pixels;
	gint m, l;
	gchar txt[100];

	if (mydebug >50) printf ("draw_zoom_scale()\n");

	pixels = 141 / milesconv;
	m = mapscale / (20 * zoom);
	if (m < 1000)
	{
		if (!nauticflag)
			g_snprintf (txt, sizeof (txt), "%d%s", m,
				    (milesflag) ? "yrds" : "m");
		else
			g_snprintf (txt, sizeof (txt), "%.3f%s", m / 1000.0,
				    (milesflag) ? "mi" : ((metricflag) ? "km"
							  : "nmi"));

		if (!metricflag)
			pixels = pixels * milesconv * 0.9144;
	}
	else
		g_snprintf (txt, sizeof (txt), "%.1f%s", m / 1000.0,
			    (milesflag) ? "mi" : ((metricflag) ? "km" :
						  "nmi"));
	/*       l =
	 *  (SCREEN_X - 20) - pixels + (pixels -
	 *                              gdk_text_width (smalltextfont, txt,
	 *                                              strlen (txt))) / 2;
	 */
	l = (SCREEN_X - 40) - pixels + (pixels - strlen (txt) * 15);

	/*       if ( mydebug > 10 ) */
	/*  g_print("%d\n", m); */

	gdk_gc_set_function (kontext, GDK_OR);
	gdk_gc_set_foreground (kontext, &textback);
	gdk_draw_rectangle (drawable, kontext, 1,
			    (SCREEN_X - 20) - pixels - 5, SCREEN_Y - 35,
			    pixels + 10, 30);
	gdk_gc_set_function (kontext, GDK_COPY);

	gdk_gc_set_foreground (kontext, &black);

	/*    gdk_draw_text (drawable, smalltextfont, kontext, l, SCREEN_Y - 20, txt,
	 *               strlen (txt));
	 */
	{
		/* prints in pango */
		PangoFontDescription *pfd;
		PangoLayout *wplabellayout;

		wplabellayout =
			gtk_widget_create_pango_layout (drawing_area, txt);
		//KCFX  
		if (pdamode)
			pfd = pango_font_description_from_string ("Sans 8");
		else
			pfd = pango_font_description_from_string ("Sans 11");
		pango_layout_set_font_description (wplabellayout, pfd);

		gdk_draw_layout_with_colors (drawable, kontext,
					     l, SCREEN_Y - 33,
					     wplabellayout, &black, NULL);
		if (wplabellayout != NULL)
			g_object_unref (G_OBJECT (wplabellayout));
		/* freeing PangoFontDescription, cause it has been copied by prev. call */
		pango_font_description_free (pfd);

	}

	gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	gdk_draw_line (drawable, kontext,
		       (SCREEN_X - 20) - pixels, SCREEN_Y - 20 + 5,
		       (SCREEN_X - 20), SCREEN_Y - 20 + 5);
	gdk_draw_line (drawable, kontext,
		       (SCREEN_X - 20) - pixels, SCREEN_Y - 20,
		       (SCREEN_X - 20) - pixels, SCREEN_Y - 20 + 10);
	gdk_draw_line (drawable, kontext,
		       (SCREEN_X - 20), SCREEN_Y - 20,
		       (SCREEN_X - 20), SCREEN_Y - 20 + 10);

#ifdef USETELEATLAS
	/* display the streetname */
	ta_displaystreetname (actualstreetname);
#endif


	/*  draw zoom factor */
	g_snprintf (txt, sizeof (txt), "%dx", zoom);
	/*       l = (SCREEN_X - 15) - gdk_text_width (textfont, txt, strlen (txt)) / 2;  */

	l = (SCREEN_X - 15) - 14 - strlen (txt) * 2;

	gdk_gc_set_function (kontext, GDK_OR);

	gdk_gc_set_foreground (kontext, &mygray);
	gdk_draw_rectangle (drawable, kontext, 1, (SCREEN_X - 30), 0, 30, 30);
	gdk_gc_set_function (kontext, GDK_COPY);

	gdk_gc_set_foreground (kontext, &blue);

	/*       gdk_draw_text (drawable, textfont, kontext, l, 22, txt, strlen (txt));  */
	{
		/* prints in pango */
		PangoFontDescription *pfd;
		PangoLayout *wplabellayout;

		wplabellayout =
			gtk_widget_create_pango_layout (drawing_area, txt);
		//KCFX  
		if (pdamode)
			pfd = pango_font_description_from_string ("Sans 9");
		else
			pfd = pango_font_description_from_string ("Sans 14");
		pango_layout_set_font_description (wplabellayout, pfd);

		gdk_draw_layout_with_colors (drawable, kontext,
					     l, 2, wplabellayout, &blue,
					     NULL);
		if (wplabellayout != NULL)
			g_object_unref (G_OBJECT (wplabellayout));
		/* freeing PangoFontDescription, cause it has been copied by prev. call */
		pango_font_description_free (pfd);

	}
}

/* *****************************************************************************
 * draw the markers on the map 
 * And many more 
 * TODO: sort out
 */
gint
drawmarker (GtkWidget * widget, guint * datum)
{
	gdouble posxdest, posydest, posxmarker, posymarker;
	gchar s2[100], s3[200], s2a[20];
	gdouble w;
	GdkPoint poly[16];
	gint k;

	gblink = !gblink;
	/*    g_print("simmode: %d, nmea %d garmin %d\n",simmode,haveNMEA,haveGARMIN); */

	if (importactive)
	    return TRUE;

	drawtracks ();

	if (do_draw_grid)
		draw_grid (widget);

	if (usesql)
	{
		streets_draw_list ();
		poi_draw_list ();
		tracks_draw_list ();
	}

	if (wpflag)
		draw_waypoints ();

	if (havefriends)
		drawfriends ();

	if (havekismet)
		readkismet ();

	/*  draw scale */
	if (zoomscale)
		draw_zoom_scale ();

	if (havekismet && (kismetsock>=0))
	    gdk_draw_pixbuf (drawable, kontext, kismetpixbuf, 0, 0,
			     10, SCREEN_Y - 42,
			     36, 20, GDK_RGB_DITHER_NONE, 0, 0);
	
	if (savetrack)
	{
		/*  k = gdk_text_width (smalltextfont, savetrackfn, strlen (savetrackfn));
		 */
		k = 100;
		gdk_gc_set_foreground (kontext, &white);
		gdk_draw_rectangle (drawable, kontext, 1, 10,
				    SCREEN_Y - 21, k + 3, 14);
		gdk_gc_set_foreground (kontext, &red);
		{
			/* prints in pango */
			PangoFontDescription *pfd;
			PangoLayout *wplabellayout;

			wplabellayout =
				gtk_widget_create_pango_layout (drawing_area,
								savetrackfn);
			//KCFX  
			if (pdamode)
				pfd = pango_font_description_from_string
					("Sans 7");
			else
				pfd = pango_font_description_from_string
					("Sans 10");
			pango_layout_set_font_description (wplabellayout,
							   pfd);

			gdk_draw_layout_with_colors (drawable, kontext,
						     14, SCREEN_Y - 22,
						     wplabellayout, &red,
						     NULL);
			if (wplabellayout != NULL)
				g_object_unref (G_OBJECT (wplabellayout));
			/* freeing PangoFontDescription, cause it has been copied by prev. call */
			pango_font_description_free (pfd);

		}


		/*    gdk_draw_text (drawable, smalltextfont, kontext,
		 *                   11, SCREEN_Y - 10, savetrackfn,
		 *                   strlen (savetrackfn));
		 */

		/*      gdk_draw_text (drawable, textfont, kontext, 10, */
		/*                     SCREEN_Y - 10, savetrackfn, strlen (savetrackfn)); */
	}

	if (posmode)
	{
	    blink = TRUE;
	}

#define PFSIZE 55
#define PFSIZE2 45


	if (havepos || blink)
	{
		if (posmode)
		{
			gdk_gc_set_foreground (kontext, &blue);
			gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
			gdk_draw_rectangle (drawable, kontext, 0, posx - 10,
					    posy - 10, 20, 20);
		}
		else
		{
			if (shadow)
			{
				/*  draw shadow of  position marker */
				gdk_gc_set_foreground (kontext, &darkgrey);
				gdk_gc_set_line_attributes (kontext, 3, 0, 0,
							    0);
				gdk_gc_set_function (kontext, GDK_AND);
				gdk_draw_arc (drawable, kontext, 0,
					      posx - 7 + SHADOWOFFSET,
					      posy - 7 + SHADOWOFFSET, 14, 14,
					      0, 360 * 64);
				/*  draw pointer to destination */
				gdk_gc_set_line_attributes (kontext, 4, 0, 0,
							    0);
				/* gdk_draw_line (drawable, kontext, */
				/*       posx + 4 * sin (angle_to_destination) + */
				/*       SHADOWOFFSET, */
				/*       posy - 4 * cos (angle_to_destination) + */
				/*       SHADOWOFFSET, */
				/*       posx + 20 * sin (angle_to_destination) + */
				/*       SHADOWOFFSET, */
				/*       posy - 20 * cos (angle_to_destination) + */
				/*       SHADOWOFFSET); */
				w = angle_to_destination + M_PI;
				poly[0].x =
					posx + SHADOWOFFSET +
					(PFSIZE) / 2.3 * (cos (w + M_PI_2));
				poly[0].y =
					posy + SHADOWOFFSET +
					(PFSIZE) / 2.3 * (sin (w + M_PI_2));
				poly[1].x =
					posx + SHADOWOFFSET +
					(PFSIZE) / 9 * (cos (w + M_PI));
				poly[1].y =
					posy + SHADOWOFFSET +
					(PFSIZE) / 9 * (sin (w + M_PI));
				poly[2].x =
					posx + SHADOWOFFSET +
					PFSIZE / 10 * (cos (w + M_PI_2));
				poly[2].y =
					posy + SHADOWOFFSET +
					PFSIZE / 10 * (sin (w + M_PI_2));
				poly[3].x =
					posx + SHADOWOFFSET -
					(PFSIZE) / 9 * (cos (w + M_PI));
				poly[3].y =
					posy + SHADOWOFFSET -
					(PFSIZE) / 9 * (sin (w + M_PI));
				poly[4].x = poly[0].x;
				poly[4].y = poly[0].y;
				gdk_draw_polygon (drawable, kontext, 1,
						  (GdkPoint *) poly, 5);

				/*  draw pointer to direction */
				/*  gdk_draw_line (drawable, kontext, */
				/*       posx + 4 * sin (direction) + SHADOWOFFSET, */
				/*       posy - 4 * cos (direction) + SHADOWOFFSET, */
				/*       posx + 20 * sin (direction) + SHADOWOFFSET, */
				/*       posy - 20 * cos (direction) + SHADOWOFFSET); */
				gdk_gc_set_line_attributes (kontext, 2, 0, 0,
							    0);
				gdk_draw_arc (drawable, kontext, 0,
					      posx + 2 - 7 + SHADOWOFFSET,
					      posy + 2 - 7 + SHADOWOFFSET, 10,
					      10, 0, 360 * 64);

				w = direction + M_PI;
				poly[0].x =
					posx + SHADOWOFFSET +
					(PFSIZE2) / 2.3 * (cos (w + M_PI_2));
				poly[0].y =
					posy + SHADOWOFFSET +
					(PFSIZE2) / 2.3 * (sin (w + M_PI_2));
				poly[1].x =
					posx + SHADOWOFFSET +
					(PFSIZE2) / 9 * (cos (w + M_PI));
				poly[1].y =
					posy + SHADOWOFFSET +
					(PFSIZE2) / 9 * (sin (w + M_PI));
				poly[2].x =
					posx + SHADOWOFFSET +
					PFSIZE2 / 10 * (cos (w + M_PI_2));
				poly[2].y =
					posy + SHADOWOFFSET +
					PFSIZE2 / 10 * (sin (w + M_PI_2));
				poly[3].x =
					posx + SHADOWOFFSET -
					(PFSIZE2) / 9 * (cos (w + M_PI));
				poly[3].y =
					posy + SHADOWOFFSET -
					(PFSIZE2) / 9 * (sin (w + M_PI));
				poly[4].x = poly[0].x;
				poly[4].y = poly[0].y;
				gdk_draw_polygon (drawable, kontext, 0,
						  (GdkPoint *) poly, 5);
				gdk_gc_set_function (kontext, GDK_COPY);
			}
			/*  draw real position marker */

			gdk_gc_set_foreground (kontext, &black);
			gdk_gc_set_line_attributes (kontext, 3, 0, 0, 0);
			gdk_draw_arc (drawable, kontext, 0, posx - 7,
				      posy - 7, 14, 14, 0, 360 * 64);
			/*  draw pointer to destination */

			w = angle_to_destination + M_PI;

			poly[0].x =
				posx + (PFSIZE) / 2.3 * (cos (w + M_PI_2));
			poly[0].y =
				posy + (PFSIZE) / 2.3 * (sin (w + M_PI_2));
			poly[1].x = posx + (PFSIZE) / 9 * (cos (w + M_PI));
			poly[1].y = posy + (PFSIZE) / 9 * (sin (w + M_PI));
			poly[2].x = posx + PFSIZE / 10 * (cos (w + M_PI_2));
			poly[2].y = posy + PFSIZE / 10 * (sin (w + M_PI_2));
			poly[3].x = posx - (PFSIZE) / 9 * (cos (w + M_PI));
			poly[3].y = posy - (PFSIZE) / 9 * (sin (w + M_PI));
			poly[4].x = poly[0].x;
			poly[4].y = poly[0].y;
			gdk_draw_polygon (drawable, kontext, 1,
					  (GdkPoint *) poly, 5);

			/*  draw pointer to direction */
			gdk_gc_set_foreground (kontext, &red);
			gdk_draw_arc (drawable, kontext, 0, posx + 2 - 7,
				      posy + 2 - 7, 10, 10, 0, 360 * 64);
			w = direction + M_PI;
			poly[0].x =
				posx + (PFSIZE2) / 2.3 * (cos (w + M_PI_2));
			poly[0].y =
				posy + (PFSIZE2) / 2.3 * (sin (w + M_PI_2));
			poly[1].x = posx + (PFSIZE2) / 9 * (cos (w + M_PI));
			poly[1].y = posy + (PFSIZE2) / 9 * (sin (w + M_PI));
			poly[2].x = posx + PFSIZE2 / 10 * (cos (w + M_PI_2));
			poly[2].y = posy + PFSIZE2 / 10 * (sin (w + M_PI_2));
			poly[3].x = posx - (PFSIZE2) / 9 * (cos (w + M_PI));
			poly[3].y = posy - (PFSIZE2) / 9 * (sin (w + M_PI));
			poly[4].x = poly[0].x;
			poly[4].y = poly[0].y;
			gdk_draw_polygon (drawable, kontext, 0,
					  (GdkPoint *) poly, 5);
		}
		if (markwaypoint)
		{
			calcxy (&posxmarker, &posymarker, wplon, wplat, zoom);

			gdk_gc_set_foreground (kontext, &green);
			gdk_gc_set_line_attributes (kontext, 5, 0, 0, 0);
			gdk_draw_arc (drawable, kontext, 0, posxmarker - 10,
				      posymarker - 10, 20, 20, 0, 360 * 64);
		}
		/*  If we are in position mode we set direction to zero to see where is the  */
		/*  target  */
		if (posmode)
			direction = 0;

		bearing = angle_to_destination - direction;
		if (bearing < 0)
			bearing += 2 * M_PI;
		if (bearing > (2 * M_PI))
			bearing -= 2 * M_PI;
		display_status2 ();

	}



	/*  now draw marker for destination point */

	calcxy (&posxdest, &posydest, target_lon, target_lat, zoom);

	gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
	if (shadow)
	{
		/*  draw + sign at destination */
		gdk_gc_set_foreground (kontext, &darkgrey);
		gdk_gc_set_function (kontext, GDK_AND);
		gdk_draw_line (drawable, kontext, posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 - 10 + SHADOWOFFSET,
			       posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 - 2 + SHADOWOFFSET);
		gdk_draw_line (drawable, kontext, posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 + 2 + SHADOWOFFSET,
			       posxdest + 1 + SHADOWOFFSET,
			       posydest + 1 + 10 + SHADOWOFFSET);
		gdk_draw_line (drawable, kontext,
			       posxdest + 1 + 10 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET,
			       posxdest + 1 + 2 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET);
		gdk_draw_line (drawable, kontext,
			       posxdest + 1 - 2 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET,
			       posxdest + 1 - 10 + SHADOWOFFSET,
			       posydest + 1 + SHADOWOFFSET);
		gdk_gc_set_function (kontext, GDK_COPY);
	}

	if (crosstoogle)
		gdk_gc_set_foreground (kontext, &blue);
	else
		gdk_gc_set_foreground (kontext, &red);
	crosstoogle = !crosstoogle;
	/*  draw + sign at destination */
	gdk_draw_line (drawable, kontext, posxdest + 1,
		       posydest + 1 - 10, posxdest + 1, posydest + 1 - 2);
	gdk_draw_line (drawable, kontext, posxdest + 1,
		       posydest + 1 + 2, posxdest + 1, posydest + 1 + 10);
	gdk_draw_line (drawable, kontext, posxdest + 1 + 10,
		       posydest + 1, posxdest + 1 + 2, posydest + 1);
	gdk_draw_line (drawable, kontext, posxdest + 1 - 2,
		       posydest + 1, posxdest + 1 - 10, posydest + 1);


	/* display messages on map */
	display_dsc ();
	/*  if distance is less then 1 km show meters */
	if (milesflag)
	{
		if (dist <= 1.0)
		{
			g_snprintf (s2, sizeof (s2), "%.0f", dist * 1760.0);
			g_strlcpy (s2a, "yrds", sizeof (s2a));
		}
		else
		{
			if (dist <= 10.0)
			{
				g_snprintf (s2, sizeof (s2), "%.2f", dist);
				g_strlcpy (s2a, "mi", sizeof (s2a));
			}
			else
			{
				g_snprintf (s2, sizeof (s2), "%.1f", dist);
				g_strlcpy (s2a, "mi", sizeof (s2a));
			}
		}
	}
	if (metricflag)
	{
		if (dist <= 1.0)
		{
			g_snprintf (s2, sizeof (s2), "%.0f", dist * 1000.0);
			g_strlcpy (s2a, "m", sizeof (s2a));
		}
		else
		{
			if (dist <= 10.0)
			{
				g_snprintf (s2, sizeof (s2), "%.2f", dist);
				g_strlcpy (s2a, "km", sizeof (s2a));
			}
			else
			{
				g_snprintf (s2, sizeof (s2), "%.1f", dist);
				g_strlcpy (s2a, "km", sizeof (s2a));
			}
		}
	}
	if (nauticflag)
	{
		if (dist <= 1.0)
		{
			g_snprintf (s2, sizeof (s2), "%.3f", dist);
			g_strlcpy (s2a, "nmi", sizeof (s2a));
		}
		else
		{
			if (dist <= 10.0)
			{
				g_snprintf (s2, sizeof (s2), "%.2f", dist);
				g_strlcpy (s2a, "nmi", sizeof (s2a));
			}
			else
			{
				g_snprintf (s2, sizeof (s2), "%.1f", dist);
				g_strlcpy (s2a, "nmi", sizeof (s2a));
			}
		}
	}
	/*    display distance, speed and zoom */
	/*   g_snprintf (s3, */
	/*     "<span color=\"%s\" font_desc=\"%s\">%s</span><span color=\"%s\" font_desc=\"%s\">%s</span>", */
	/*     bluecolor, bigfont, s2, bluecolor, "sans bold 18", s2a); */
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"%s\" font_desc=\"%s\">%s<span size=\"16000\">%s</span></span>",
		    bluecolor, bigfont, s2, s2a);
	gtk_label_set_markup (GTK_LABEL (distlabel), s3);
	/* gtk_label_set_text (GTK_LABEL (distlabel), s2);  */
	if (milesflag)
		g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
	if (metricflag)
		g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
	if (nauticflag)
		g_snprintf (s2, sizeof (s2), "%3.1f", groundspeed);
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"%s\" font_desc=\"%s\">%s</span>",
		    bluecolor, bigfont, s2);
	gtk_label_set_markup (GTK_LABEL (speedlabel), s3);

	/* gtk_label_set_text (GTK_LABEL (speedlabel), s2); */

	if (havealtitude)
	{
		if (milesflag || nauticflag)
		{
			g_snprintf (s2, sizeof (s2), "%.0f",
				    altitude * 3.2808399 + normalnull);
			g_strlcpy (s2a, "ft", sizeof (s2a));
		}
		else
		{
			g_snprintf (s2, sizeof (s2), "%.0f",
				    altitude + normalnull);
			g_strlcpy (s2a, "m", sizeof (s2a));
		}
		gtk_label_set_text (GTK_LABEL (altilabel), s2);

		if (pdamode)
		{
			if (normalnull == 0.0)

				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>",
					    bluecolor, s2, bluecolor, s2a);
			else
				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>"
					    "<span color=\"red\" font_family=\"Arial\" size=\"5000\">\nNN %+.1f</span>",
					    bluecolor, s2, bluecolor, s2a,
					    normalnull);
		}
		else
		{
			if (normalnull == 0.0)

				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
					    bluecolor, s2, bluecolor, s2a);
			else
				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>"
					    "<span color=\"red\" font_family=\"Arial\" weight=\"bold\" size=\"8000\">\nNN %+.1f</span>",
					    bluecolor, s2, bluecolor, s2a,
					    normalnull);
		}
		gtk_label_set_markup (GTK_LABEL (altilabel), s3);
	}
	if (simmode)
		blink = TRUE;
	else
	{
		if (!havepos)
			blink = !blink;
	}

	if (newsatslevel)
		expose_sats_cb (NULL, 0);

	if (downloadwindowactive)
	{
		drawdownloadrectangle (1);
		expose_mini_cb (NULL, 0);
	}

	/* force to say new direction */
	if (!strcmp (oldangle, "XXX"))
		speech_out_cb (NULL, 0);

	return (TRUE);
}




/* *****************************************************************************
 * Copy Image from loaded map 
 */
gint
expose_mini_cb (GtkWidget * widget, guint * datum)
{
	if (mydebug >50) printf ("expose_mini_cb()\n");

	/*  draw the minimap */
	if (!miniimage)
		return TRUE;

	/*   g_print("in expose_mini_cb\n"); */
	if (SMALLMENU == 0)
	{
		gdk_draw_pixbuf (drawing_miniimage->window,
				 kontext, miniimage, 0, 0, 0, 0, 128, 103,
				 GDK_RGB_DITHER_NONE, 0, 0);

		/*       if ((nightmode == 1) || ((nightmode == 2) && (isnight&& !disableisnight)) */
		/*  { */
		/*    gdk_gc_set_function (kontext, GDK_AND); */
		/*    gdk_gc_set_foreground (kontext, &nightcolor); */
		/*    gdk_draw_rectangle (drawing_miniimage->window, kontext, 1, 0, 0, 128, */
		/*                        103); */
		/*    gdk_gc_set_function (kontext, GDK_COPY); */
		/*  } */
		gdk_gc_set_foreground (kontext, &red);
		gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);

		gdk_draw_rectangle (drawing_miniimage->window, kontext, 0,
				    (64 - (SCREEN_X_2 / 10) / zoom) +
				    xoff / (zoom * 10),
				    (50 - (SCREEN_Y_2 / 10) / zoom) +
				    yoff / (zoom * 10),
				    SCREEN_X / (zoom * 10),
				    SCREEN_Y / (zoom * 10));
		drawdownloadrectangle (0);
	}
	return TRUE;
}

/* *****************************************************************************
 */
gint
expose_compass (GtkWidget * widget, guint * datum)
{
	static GdkGC *compasskontext = NULL;
	gint l, l2, j;
	gint line_count;
	gchar txt[10], txt2[10], *txtp;
	gdouble w, kurz;
	GdkPoint poly[16];
	static GdkPixbuf *compassimage = NULL;

	if (mydebug >50) printf ("expose_compass()\n");


	/*   This string means North,East,South,West -- please translate the letters */
	g_strlcpy (txt2, _("NESW"), sizeof (txt2));

	txtp = txt2;

	if (compasskontext == NULL)
		compasskontext = gdk_gc_new (drawable_bearing);
	if (compassimage == NULL)
		compassimage = read_icon("compass.png");
	if (compassimage == NULL && do_unit_test ) {
	    exit (-1);
	}

	gdk_draw_pixbuf (drawable_bearing, compasskontext, compassimage, 0, 0,
			 0, 0, PSIZE, PSIZE, GDK_RGB_DITHER_NONE, 0, 0);


	if (foundradar)
	{
		gdk_gc_set_foreground (kontext, &red);
		gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
		w = radarbearing + M_PI;
		if (w < 0)
			w += 2 * M_PI;
		if (w > (2 * M_PI))
			w -= 2 * M_PI;
		/*            g_print("Radarbearing: %g w: %g\n", radarbearing, w); */
#define KURZW 1.2
		kurz = cos (KURZW);
		poly[0].x = PSIZE / 2 + PSIZE / 2.5 * cos (w);	/* x */
		poly[0].y = PSIZE / 2 + PSIZE / 2.5 * sin (w);	/* y */
		poly[1].x = PSIZE / 2 + PSIZE / 2 * cos (w + M_PI_2);
		poly[1].y = PSIZE / 2 + PSIZE / 2 * sin (w + M_PI_2);
		poly[2].x = PSIZE / 2 + PSIZE / 2.5 * cos (w + M_PI);
		poly[2].y = PSIZE / 2 + PSIZE / 2.5 * sin (w + M_PI);
		poly[3].x = PSIZE / 2 + PSIZE / 2 * kurz * cos (w + M_PI);
		poly[3].y = PSIZE / 2 + PSIZE / 2 * kurz * sin (w + M_PI);
		poly[4].x = PSIZE / 2 + PSIZE / 2 * cos (w + M_PI + KURZW);
		poly[4].y = PSIZE / 2 + PSIZE / 2 * sin (w + M_PI + KURZW);
		poly[5].x =
			PSIZE / 2 + PSIZE / 2 * cos (w + 2 * M_PI - KURZW);
		poly[5].y =
			PSIZE / 2 + PSIZE / 2 * sin (w + 2 * M_PI - KURZW);
		poly[6].x = PSIZE / 2 + PSIZE / 2 * kurz * cos (w);
		poly[6].y = PSIZE / 2 + PSIZE / 2 * kurz * sin (w);
		poly[7].x = poly[0].x;
		poly[7].y = poly[0].y;
		gdk_draw_polygon (drawable_bearing, kontext, 1,
				  (GdkPoint *) poly, 8);
	}

	gdk_gc_set_foreground (kontext, &black);

	/* compass */
	//      /* added by zwerg (Daniel Wernle)
	w = -direction + M_PI;

	j = 0;

	for (line_count = 0; line_count < 12; line_count++)
	{
		gdk_gc_set_foreground (compasskontext, &red);

		if (!(line_count % 3))
		{
			gdk_gc_set_line_attributes (compasskontext, 2, 0, 0,
						    0);


			l = 10;
			l2 = -18;
			{
				/* prints in pango */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;
				g_utf8_strncpy (txt, txtp, 1);
				txtp = g_utf8_next_char (txtp);
				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_bearing, txt);
				if (pdamode)
					pfd = pango_font_description_from_string ("Sans 7");
				else
					pfd = pango_font_description_from_string ("Sans 10");
				pango_layout_set_font_description
					(wplabellayout, pfd);

				gdk_draw_layout_with_colors (drawable_bearing,
							     compasskontext,
							     (PSIZE / 2.0 +
							      (PSIZE) / 3.5 *
							      (cos
							       (w +
								M_PI_2))) -
							     l / 2,
							     (PSIZE / 2.0 +
							      (PSIZE) / 3.5 *
							      (sin
							       (w +
								M_PI_2))) +
							     l2 / 2,
							     wplabellayout,
							     &black, NULL);
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}
			gdk_gc_set_foreground (compasskontext, &red);
		}
		else
		{
			gdk_gc_set_foreground (compasskontext, &black);
			gdk_gc_set_line_attributes (compasskontext, 1, 0, 0,
						    0);
		}

		/*       gdk_draw_line (drawable_bearing, compasskontext, */
		/*               PSIZE / 2.0 + (PSIZE) / 2.75 * (cos (w + M_PI_2)), */
		/*               PSIZE / 2.0 + (PSIZE) / 2.75 * (sin (w + M_PI_2)), */
		/*               PSIZE / 2.0 + (PSIZE) / 2.0 * (cos (w + M_PI_2)), */
		/*               PSIZE / 2.0 + (PSIZE) / 2.0 * (sin (w + M_PI_2))); */

		w = w + M_PI / 6.0;

	}

	w = bearing + M_PI;

#define TRIANGLEFACTOR 0.75
	gdk_gc_set_foreground (compasskontext, &black);

	gdk_gc_set_line_attributes (compasskontext, 1, 0, 0, 0);
	/*   gdk_draw_arc (drawable_bearing, compasskontext, 0, 0, 0, PSIZE, PSIZE, */
	/*          0, 360 * 64); */

	poly[0].x = PSIZE / 2 + (PSIZE) / 2.3 * (cos (w + M_PI_2));
	poly[0].y = PSIZE / 2 + (PSIZE) / 2.3 * (sin (w + M_PI_2));
	poly[1].x = PSIZE / 2 + (PSIZE) / 9 * (cos (w + M_PI));
	poly[1].y = PSIZE / 2 + (PSIZE) / 9 * (sin (w + M_PI));
	poly[2].x = PSIZE / 2 + PSIZE / 10 * (cos (w + M_PI_2));
	poly[2].y = PSIZE / 2 + PSIZE / 10 * (sin (w + M_PI_2));
	poly[3].x = PSIZE / 2 - (PSIZE) / 9 * (cos (w + M_PI));
	poly[3].y = PSIZE / 2 - (PSIZE) / 9 * (sin (w + M_PI));
	poly[4].x = poly[0].x;
	poly[4].y = poly[0].y;

	gdk_gc_set_foreground (compasskontext, &black);
	gdk_gc_set_line_attributes (compasskontext, 2, 0, 0, 0);

	gdk_draw_polygon (drawable_bearing, compasskontext, 1,
			  (GdkPoint *) poly, 5);

	gdk_gc_set_foreground (kontext, &black);

	return TRUE;

}

/* *****************************************************************************
 * Copy Image from loaded map
 */
gint
expose_cb (GtkWidget * widget, guint * datum)
{
	gint x, y, i, oldxoff, oldyoff, xoffmax, yoffmax, ok, okcount;
	gdouble tx, ty, lastangle;
	gchar name[40], s1[40], *tn;

	if (mydebug >50) printf ("expose_cb()\n");

	/*    g_print("\nexpose_cb %d",exposecounter++);   */

	/*   fprintf (stderr, "lat: %f long: %f\n", current_lat, current_lon); */
	if (exposed && pdamode)
		return TRUE;



	errortextmode = FALSE;
	if (!importactive)
	{
		/*  We don't need to draw anything if there is no map yet */
		if (!maploaded)
		{
		    display_status (_("No map available for this position!"));
		    /* return TRUE; */
		}

		if (posmode)
		{
			current_lon = posmode_x;
			current_lat = posmode_y;
		}


		/*  get pos for current position */
		calcxy (&posx, &posy, current_lon, current_lat, zoom);

		/*  do this because calcxy already substracted xoff and yoff */
		posx = posx + xoff;
		posy = posy + yoff;

		/*  Calculate Angle to destination */
		tx = (2 * R * M_PI / 360) * cos (M_PI * current_lat / 180.0) *
			(target_lon - current_lon);
		ty = (2 * R * M_PI / 360) * (target_lat - current_lat);
		lastangle = angle_to_destination;
		angle_to_destination = atan (tx / ty);
		/*        g_print ("\ntx: %f, ty:%f angle_to_dest: %f", tx, ty, */
		/*                 angle_to_destination); */
		if (!finite (angle_to_destination))
			angle_to_destination = lastangle;
		else
		{
			/*  correct the value to be < 2*PI */
			if (ty < 0)
				angle_to_destination =
					M_PI + angle_to_destination;
			if (angle_to_destination >= (2 * M_PI))
				angle_to_destination -= 2 * M_PI;
			if (angle_to_destination < 0)
				angle_to_destination += 2 * M_PI;
		}
		if ( mydebug>30 )
			g_print ("Angle_To_Destination: %.1f �\n",
				 angle_to_destination * 180 / M_PI);

		if (havefriends && targetname[0] == '*')
			for (i = 0; i < maxfriends; i++)
			{
				g_strlcpy (name, "*", sizeof (name));

				g_strlcat (name, (friends + i)->name,
					   sizeof (name));
				tn = g_strdelimit (name, "_", ' ');
				if ((strcmp (targetname, tn)) == 0)
				{
				    coordinate_string2gdouble((friends + i)->lat, &target_lat);
				    coordinate_string2gdouble((friends + i)->lon, &target_lon);
				}
			}

		/*  Calculate distance to destination */
		dist = calcdist (target_lon, target_lat);

		if ( display_background_map() ) 
		    {
			/*  correct the shift of the map */
			oldxoff = xoff;
			oldyoff = yoff;
			/* now we test if the marker fits into the map and set the shift of the 
			 * little SCREEN_XxSCREEN_Y region in relation to the real 1280x1024 map 
			 */
			okcount = 0;
			do
			    {
				ok = TRUE;
				okcount++;
				x = posx - xoff;
				y = posy - yoff;
				
				if (x < borderlimit)
				    xoff -= 2 * borderlimit;
				if (x > (SCREEN_X - borderlimit))
				    xoff += 2 * borderlimit;
				if (y < borderlimit)
				    yoff -= 2 * borderlimit;
				if (y > (SCREEN_Y - borderlimit))
				    yoff += 2 * borderlimit;

				if (x < borderlimit)
				    ok = FALSE;
				if (x > (SCREEN_X - borderlimit))
				    ok = FALSE;
				if (y < borderlimit)
				    ok = FALSE;
				if (y > (SCREEN_Y - borderlimit))
				    ok = FALSE;
				if (okcount > 2000)
				    {
					g_print ("\nloop detected, please report!\n");
					ok = TRUE;
				    }
			    }
			while (!ok);

			xoffmax = (640 * zoom) - SCREEN_X_2;
			yoffmax = (512 * zoom) - SCREEN_Y_2;
			if (xoff > xoffmax)
			    xoff = xoffmax;
			if (xoff < -xoffmax)
			    xoff = -xoffmax;
			if (yoff > yoffmax)
			    yoff = yoffmax;
			if (yoff < -yoffmax)
			    yoff = -yoffmax;

			/*  we only need to create a new region if the shift is not changed */
			if ((oldxoff != xoff) || (oldyoff != yoff))
			    iszoomed = FALSE;
			if ( mydebug>30 )
			    {
				g_print ("x: %d  xoff: %d oldxoff: %d Zoom: %d xoffmax: %d\n", x, xoff, oldxoff, zoom, xoffmax);
				g_print ("y: %d  yoff: %d oldyoff: %d Zoom: %d yoffmax: %d\n", y, yoff, oldyoff, zoom, yoffmax);
			    }
			posx = posx - xoff;
			posy = posy - yoff;
		    }
	}


	/*  zoom from to 1280x1024 map to the SCREEN_XxSCREEN_Y region */
	if (!iszoomed)
	{
		rebuildtracklist ();

		if (tempimage == NULL)
			tempimage =
				gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8,
						1280, 1024);

		if (maploaded)
		{
			/*    g_print ("\nmap loaded, do gdk_pixbuf_scale\n");  */
			gdk_pixbuf_scale (image, tempimage, 0, 0, 1280, 1024,
					  640 - xoff - 640 * zoom,
					  512 - yoff - 512 * zoom, zoom, zoom,
					  GDK_INTERP_BILINEAR);

			/*       image=gdk_pixbuf_scale_simple(tempimage,640 - xoff - 640 * zoom,
			 *                            512 - yoff - 512 * zoom, 
			 *                            GDK_INTERP_BILINEAR);
			 */

		}

		if ( mydebug > 0 )
			g_print ("map zoomed!\n");
		iszoomed = TRUE;
		expose_mini_cb (NULL, 0);

	}

	gdk_draw_pixbuf (drawable, kontext, tempimage,
			 640 - SCREEN_X_2,
			 512 - SCREEN_Y_2, 0, 0,
			 SCREEN_X, SCREEN_Y, GDK_RGB_DITHER_NONE, 0, 0);

	if ((!disableisnight) && (!downloadwindowactive))
	{
		if ((nightmode == 1) || ((nightmode == 2) && isnight))
		{
			gdk_gc_set_function (kontext, GDK_AND);
			gdk_gc_set_foreground (kontext, &nightcolor);
			gdk_draw_rectangle (drawable, kontext, 1, 0, 0,
					    SCREEN_X, SCREEN_Y);
			gdk_gc_set_function (kontext, GDK_COPY);
		}
	}

	if (wp_from_sql)
	{
		g_snprintf (s1, sizeof (s1), "%d", wptotal);
		gtk_entry_set_text (GTK_ENTRY (wplabel1), s1);
		g_snprintf (s1, sizeof (s1), "%d", wpselected);
		gtk_entry_set_text (GTK_ENTRY (wplabel2), s1);
		if (dbusedist)
			g_snprintf (s1, sizeof (s1), "%.1f km", dbdistance);
		else
			g_snprintf (s1, sizeof (s1), _("unused"));
		gtk_entry_set_text (GTK_ENTRY (wplabel3), s1);
		gtk_entry_set_text (GTK_ENTRY (wplabel4), loctime);
	}
	else
	{
		g_strlcpy (s1, _("n/a"), sizeof (s1));
		gtk_entry_set_text (GTK_ENTRY (wplabel1), s1);
		gtk_entry_set_text (GTK_ENTRY (wplabel2), s1);
		gtk_entry_set_text (GTK_ENTRY (wplabel3), s1);
		gtk_entry_set_text (GTK_ENTRY (wplabel4), loctime);
	}
	if (havefriends)
		g_snprintf (s1, sizeof (s1), "%d/%d", actualfriends,
			    maxfriends);
	else
		g_strlcpy (s1, _("n/a"), sizeof (s1));
	gtk_entry_set_text (GTK_ENTRY (wplabel5), s1);

	drawmarker (0, 0);

	gdk_draw_pixmap (drawing_area->window, kontext, drawable, 0,
			 0, 0, 0, SCREEN_X, SCREEN_Y);
	exposed = TRUE;
	return TRUE;
}


/* *****************************************************************************
 *  This is called in simulation mode, it moves the position to the
 *  selected destination 
 */
gint
simulated_pos (GtkWidget * widget, guint * datum)
{
	gdouble ACCELMAX, ACCEL;
	gdouble secs, tx, ty, lastdirection;

	if (mydebug >50) printf ("simulated_pos()\n");

	if (!simfollow)
		return TRUE;

	ACCELMAX = 0.00002 + dist / 30000.0;
	ACCEL = ACCELMAX / 20.0;
	long_diff += ACCEL * sin (angle_to_destination);
	lat_diff += ACCEL * cos (angle_to_destination);
	if (long_diff > ACCELMAX)
		long_diff = ACCELMAX;
	if (long_diff < -ACCELMAX)
		long_diff = -ACCELMAX;
	if (lat_diff > ACCELMAX)
		lat_diff = ACCELMAX;
	if (lat_diff < -ACCELMAX)
		lat_diff = -ACCELMAX;


	current_lat += lat_diff;
	current_lon += long_diff;
	secs = g_timer_elapsed (timer, 0);
	if (secs >= 1.0)
	{
		g_timer_stop (timer);
		g_timer_start (timer);
		tx = (2 * R * M_PI / 360) * cos (M_PI * current_lat / 180.0) *
			(current_lon - old_lon);
		ty = (2 * R * M_PI / 360) * (current_lat - old_lat);
#define MINSPEED 1.0
		if (((fabs (tx)) > MINSPEED) || (((fabs (ty)) > MINSPEED)))
		{
			lastdirection = direction;
			if (ty == 0)
				direction = 0.0;
			else
				direction = atan (tx / ty);
			if (!finite (direction))
				direction = lastdirection;

			if (ty < 0)
				direction = M_PI + direction;
			if (direction >= (2 * M_PI))
				direction -= 2 * M_PI;
			if (direction < 0)
				direction += 2 * M_PI;
			groundspeed =
				milesconv * sqrt (tx * tx +
						  ty * ty) * 3.6 / secs;
		}
		else
			groundspeed = 0.0;
		if (groundspeed > 999)
			groundspeed = 999;
		old_lat = current_lat;
		old_lon = current_lon;
		if (mydebug>30)
			g_print ("Time: %f\n", secs);
	}

	return TRUE;
}



/* *****************************************************************************
 */
gint
zoom_cb (GtkWidget * widget, guint datum)
{

	if (iszoomed == FALSE)	/* needed to be sure the old zoom is made */
		return TRUE;
	iszoomed = FALSE;
	if (datum == 1)
	{
		if (zoom >= 16)
		{
			iszoomed = TRUE;
			return TRUE;
		}
		zoom *= 2;
		xoff *= 2;
		yoff *= 2;
	}
	else
	{
		if (zoom <= 1)
		{
			zoom = 1;
			iszoomed = TRUE;
		}
		else
		{
			zoom /= 2;
			xoff /= 2;
			yoff /= 2;
		}
	}
	if (importactive)
	{
		expose_cb (NULL, 0);
		expose_mini_cb (NULL, 0);
	}
	return TRUE;
}


/* *****************************************************************************
 * Increase/decrease displayed map scale
 * TODO: Improve finding of next apropriate map
 */
gint
scalerbt_cb (GtkWidget * widget, guint datum)
{
	gint val, oldval, old2val;
	gchar oldfilename[2048];

	g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
	val = (GTK_ADJUSTMENT (scaler_adj)->value);
	old2val = val;

	do
	{
	    oldval = val;
	    if (datum == 1)
		{
		    gtk_adjustment_set_value (GTK_ADJUSTMENT (scaler_adj),
					      val + 1);
		}
	    else
		{
		    gtk_adjustment_set_value (GTK_ADJUSTMENT (scaler_adj),
					      val - 1);
		}
	    val = (GTK_ADJUSTMENT (scaler_adj)->value);

	    testnewmap ();
	}
	while ( ( (strcmp (oldfilename, mapfilename)) == 0) 
		&& (val != oldval));

	if ((strcmp (oldfilename, mapfilename)) == 0)
		val = old2val;
	gtk_adjustment_set_value (GTK_ADJUSTMENT (scaler_adj), val);
	expose_cb (NULL, 0);
	expose_mini_cb (NULL, 0);

	needtosave = TRUE;

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
	setwpactive = FALSE;

	return FALSE;
}

/* *****************************************************************************
 */
gint
importaway_cb (GtkWidget * widget, guint datum)
{
	importactive = FALSE;
	gtk_widget_destroy (widget);
	g_strlcpy (oldfilename, "XXXXXXXXXXXXXXXXXX", sizeof (oldfilename));
	return FALSE;
}

/* *****************************************************************************
 */
gint
setup2_cb (GtkWidget * widget, guint datum)
{
	gtk_widget_destroy (GTK_WIDGET (datum));
	return TRUE;
}

/* *****************************************************************************
 */
gint
wpfileselect_cb (GtkWidget * widget, guint datum)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (setupfn[datum])))
		if ((strcmp (activewpfile, (names + datum)->n)))
		{
			g_strlcpy (activewpfile, (names + datum)->n,
				   sizeof (activewpfile));
			if ( mydebug > 3 )
				g_print ("activewpfile: %s\n", activewpfile);
			loadwaypoints ();
			iszoomed = FALSE;
		}
	needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 */
gint
setup_cb (GtkWidget * widget, guint datum)
{
	GtkWidget *notebook, *vbox, *cancel;
	GtkWidget *window = NULL;
	gint i;

	gtk_widget_set_sensitive (setupbt, FALSE);

	mainsetup ();
	infos ();
	trip ();

	window = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (window), _("GpsDrive Control"));
	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	cancel = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	GTK_WIDGET_SET_FLAGS (cancel, GTK_CAN_DEFAULT);
	gtk_window_set_default (GTK_WINDOW (window), cancel);
	/*   GTK_WIDGET_SET_FLAGS (cancel, GTK_HAS_FOCUS); */


	gtk_signal_connect ((GTK_OBJECT (window)), "delete_event",
			    GTK_SIGNAL_FUNC (removesetutc), 0);
	gtk_signal_connect_object ((GTK_OBJECT (window)), "delete_event",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (window));
	gtk_signal_connect ((GTK_OBJECT (window)), "destroy",
			    GTK_SIGNAL_FUNC (removesetutc), 0);
	gtk_signal_connect_object ((GTK_OBJECT (window)), "destroy",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (window));



	gtk_signal_connect ((GTK_OBJECT (cancel)), "clicked",
			    GTK_SIGNAL_FUNC (removesetutc), 0);

	gtk_signal_connect_object ((GTK_OBJECT (cancel)), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (window));

	gtk_container_border_width (GTK_CONTAINER (window), 2 * PADDING);
	vbox = gtk_vbox_new (FALSE, 2 * PADDING);
	/*     table = gtk_table_new(2,1,TRUE); */
	/*   gtk_container_add (GTK_CONTAINER (window), vbox); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);

	/* Create a new notebook, place the position of the tabs */
	settingsnotebook = notebook = gtk_notebook_new ();
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK (notebook));
	/*     gtk_table_attach_defaults(GTK_TABLE(table), notebook, 0,1,0,1); */
	gtk_box_pack_start (GTK_BOX (vbox), notebook, TRUE, TRUE,
			    2 * PADDING);
	/*   gtk_box_pack_start (GTK_BOX (vbox), cancel, FALSE, FALSE, 2 * PADDING); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    cancel, TRUE, TRUE, 2);

	gtk_widget_show (notebook);
	/*   g_print("\nmod_setupcounter: %d",mod_setupcounter); */
	//KCFX
	for (i = 0; i <= mod_setupcounter; i++)
	{
		if (i > 4)
			setupfunction[i] ();

		gtk_widget_show_all (setupentry[i]);
		gtk_notebook_append_page (GTK_NOTEBOOK (notebook),
					  setupentry[i], setupentrylabel[i]);
	}
	gtk_notebook_set_page (GTK_NOTEBOOK (notebook), lastnotebook);
	gtk_widget_show_all (window);

	return TRUE;
}



/* *****************************************************************************
 * switching between kilometers and miles 
 */
gint
miles_cb (GtkWidget * widget, guint datum)
{
	gchar s1[80];
	/*      1=miles, 2=metric, 3=nautic */
	switch (datum)
	{
	case 1:
		milesconv = KM2MILES;
		milesflag = TRUE;
		nauticflag = FALSE;
		metricflag = FALSE;
		break;
	case 2:
		milesconv = 1.0;
		milesflag = FALSE;
		nauticflag = FALSE;
		metricflag = TRUE;
		break;
	case 3:
		milesconv = KM2NAUTIC;
		milesflag = FALSE;
		nauticflag = TRUE;
		metricflag = FALSE;
		break;

	}
	needtosave = TRUE;
	if (pdamode)
	{
		if (milesflag)
			g_snprintf (s1, sizeof (s1), "[%s]", _("mi/h"));
		else if (nauticflag)
			g_snprintf (s1, sizeof (s1), "[%s]", _("knots"));
		else
			g_snprintf (s1, sizeof (s1), "[%s]", _("km/h"));
	}
	else
	{
		if (milesflag)
			g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
				    _("mi/h"));
		else if (nauticflag)
			g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
				    _("knots"));
		else
			g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
				    _("km/h"));
	}

	gtk_frame_set_label (GTK_FRAME (frame_speed), s1);
	return TRUE;
}

/* *****************************************************************************
 * switching nightmode 
 */
gint
night_cb (GtkWidget * widget, guint datum)
{

	switch (datum)
	{
	case 0:
		nightmode = 0;
		break;
	case 1:
		nightmode = 1;
		break;
	case 2:
		nightmode = 2;
		break;

	}

	needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 *  switching shadow on/off 
 */
gint
shadow_cb (GtkWidget * widget, guint datum)
{
	shadow = !shadow;
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 *  switching etch on/off
 */
gint
etch_cb (GtkWidget * widget, guint datum)
{
	int stype;
	extern GtkWidget *frame_battery;
	extern GtkWidget *frame_temperature;


	etch = !etch;
	if (etch)
		stype = GTK_SHADOW_IN;
	else
		stype = GTK_SHADOW_ETCHED_OUT;
	gtk_frame_set_shadow_type (GTK_FRAME (frame_bearing), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_target), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_speed), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_altitude), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_wp), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_sats), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_lat), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_lon), stype);
	if ( mydebug >10 )
	    gtk_frame_set_shadow_type (GTK_FRAME (frame_mapfile), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_mapscale), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_heading), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_bearing), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_timedest), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_prefscale), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_toogles), stype);
	gtk_frame_set_shadow_type (GTK_FRAME (frame_maptype), stype);

	if (frame_battery)
	    gtk_frame_set_shadow_type (GTK_FRAME (frame_battery), stype);
	if (frame_temperature)
	    gtk_frame_set_shadow_type (GTK_FRAME (frame_temperature), stype);

	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 * switching grid on/off 
 */
gint
drawgrid_cb (GtkWidget * widget, guint datum)
{
	if ( datum )
	{
	    do_draw_grid = !do_draw_grid;
	    needtosave = TRUE;
	}
	return TRUE;
}

/* *****************************************************************************
 *  switching slow cpu on/off 
 */
gint
slowcpu_cb (GtkWidget * widget, guint datum)
{
	G_CONST_RETURN gchar *sc;
	gchar dummy[100];
	sc = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (slowcpubt)->entry));
	sscanf (sc, "%d%s", &cpuload, dummy);
	if (cpuload == 0)
		cpuload = 40;


	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 *  switching decimal or degree, minutes, seconds mode 
 */
gint
minsec_cb (GtkWidget * widget, guint datum)
{
	minsecmode=datum;
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 *  switching sat level/sat position display 
 */
gint
satpos_cb (GtkWidget * widget, guint datum)
{
	satposmode = !satposmode;
	needtosave = TRUE;
	expose_sats_cb (NULL, 0);
	return TRUE;
}

/* *****************************************************************************
 *  switching simfollow on/off 
 */
gint
simfollow_cb (GtkWidget * widget, guint datum)
{
	simfollow = !simfollow;
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 *  switching testgarmin on/off 
 */
gint
testgarmin_cb (GtkWidget * widget, guint datum)
{
	testgarmin = !testgarmin;
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 *  should I use DGPS-IP? 
 */
gint
usedgps_cb (GtkWidget * widget, guint datum)
{
	usedgps = !usedgps;
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 */
gint
earthmate_cb (GtkWidget * widget, guint datum)
{
	earthmate = !earthmate;
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 */
gint
serialdev_cb (GtkWidget * widget, guint datum)
{
	G_CONST_RETURN gchar *s;

	s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (serialbt)));
	g_strlcpy (serialdev, s, sizeof (serialdev));
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 */
gint
mapdir_cb (GtkWidget * widget, guint datum)
{
	G_CONST_RETURN gchar *s;

	s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (mapdirbt)));
	g_strlcpy (mapdir, s, sizeof (mapdir));
	needtosave = TRUE;
	needreloadmapconfig = TRUE;
	gtk_timeout_add (2000, (GtkFunction) loadmapconfig, 0);
	return TRUE;
}


/* *****************************************************************************
 */
gint
dotripmeter (GtkWidget * widget, guint datum)
{
	gdouble d;

	d = calcdist (trip_lon, trip_lat);
	trip_lon = current_lon;
	trip_lat = current_lat;
	if (!((d >= 0.0) && (d < (2000.0 * TRIPMETERTIMEOUT / 3600.0))))
	{
		fprintf (stderr,
			 _
			 ("distance jump is more then 2000km/h speed, ignoring\n"));
		return TRUE;
	}
	/* we want always have metric system stored */
	d /= milesconv;
	tripodometer += d;
	if (groundspeed / milesconv > tripmaxspeed)
		tripmaxspeed = groundspeed / milesconv;
	tripavspeedcount++;
	tripavspeed += groundspeed / milesconv;
	return TRUE;
}

/* *****************************************************************************
 */
gint
bestmap_cb (GtkWidget * widget, guint datum)
{
	if (datum == 1)
		scaleprefered = !scaleprefered;
	if (!scaleprefered)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmapbt),
					      TRUE);
		gtk_widget_set_sensitive (scalerrbt, FALSE);
		gtk_widget_set_sensitive (scalerlbt, FALSE);
		if (scaler)
			gtk_widget_set_sensitive (scaler, FALSE);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmapbt),
					      FALSE);
		gtk_widget_set_sensitive (scalerrbt, TRUE);
		gtk_widget_set_sensitive (scalerlbt, TRUE);
		if (scaler)
			gtk_widget_set_sensitive (scaler, TRUE);
	}
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 */
gint
savetrack_cb (GtkWidget * widget, guint datum)
{
	savetrack = !savetrack;
	if (savetrack)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrackbt),
					      TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrackbt),
					      FALSE);
	needtosave = TRUE;
	if (savetrack)
		savetrackfile (1);
	return TRUE;
}

/* *****************************************************************************
 */
gint
mute_cb (GtkWidget * widget, guint datum)
{
	muteflag = !muteflag;
	if (muteflag)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mutebt),
					      TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mutebt),
					      FALSE);
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 *  switching POI on/off 
 */
gint
poi_draw_cb (GtkWidget * widget, guint datum)
{
    if ( NULL == widget ) 
	widget = poi_draw_bt;

    if ( datum)
	poi_draw = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),poi_draw );
    
    

    if (poi_draw)
	poi_draw_list ();
    
    needtosave = TRUE;
    return TRUE;
}

/* *****************************************************************************
 * switching STREETS on/off 
 */
gint
streets_draw_cb (GtkWidget * widget, guint datum)
{

    if ( ! streets_draw ) 
	streets_check_if_moved_reset();

    if ( NULL == widget ) 
	widget = streets_draw_bt;

    if ( datum)
	streets_draw = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),streets_draw );
    

    streets_draw_list ();
    needtosave = TRUE;
    return TRUE;
}

/* *****************************************************************************
 * switching TRACK on/off 
 */
gint
tracks_draw_cb (GtkWidget * widget, guint datum)
{
	if ( ! tracks_draw ) 
	    tracks_check_if_moved_reset();
	tracks_draw = !tracks_draw;
	tracks_draw_list ();
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 * switching SQL on/off 
 */
gint
sql_cb (GtkWidget * widget, guint datum)
{
	wp_from_sql = !wp_from_sql;
	if (wp_from_sql)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sqlbt),
					      TRUE);
		get_sql_type_list ();
		getsqldata ();
	}
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sqlbt),
					      FALSE);
	needtosave = TRUE;
	loadwaypoints ();
	return TRUE;
}

/* *****************************************************************************
 * switching Track display on/off 
 */
gint
track_cb (GtkWidget * widget, guint datum)
{
	trackflag = !trackflag;
	if (trackflag)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trackbt),
					      TRUE);
		rebuildtracklist ();
	}
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trackbt),
					      FALSE);
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 * switching WP display on/off 
 */
gint
wp_cb (GtkWidget * widget, guint datum)
{
	wpflag = !wpflag;
	if (wpflag)
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wpbt), TRUE);
	else
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wpbt), FALSE);
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 * Update the checkbox for Pos-Mode
 */
void
update_posbt()
{
    if ( mydebug > 1 )
	g_print ("posmode=%d\n", posmode);

    if (posmode)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt),   TRUE);
    else
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt),   FALSE);
}

/* *****************************************************************************
 * toggle checkbox for Pos-Mode
 */
gint
pos_cb (GtkWidget * widget, guint datum)
{
    if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (posbt)) )
    	posmode = TRUE;
    else 
    	posmode = FALSE;
    
    posmode_x = current_lon;
    posmode_y = current_lat;
    
    return TRUE;
}


/* *****************************************************************************
 * if a waypoint is selected set the target_* variables 
 */
gint
setwp_cb (GtkWidget * widget, guint datum)
{
	gchar b[100], str[200], buf[1000], buf2[1000];
	gchar *p, *tn;
	p = b;

	deleteline = datum;
	if (dontsetwp)
		return TRUE;

	gtk_clist_get_text (GTK_CLIST (mylist), datum, 0, &p);
	if (createroute)
	{
		/*        g_print("route: %s\n", p); */
		thisrouteline = atol (p) - 1;
		insertroutepoints ();
		return TRUE;
	}
	thisline = atol (p);
	/*    g_print("%d\n", thisline); */
	gtk_clist_get_text (GTK_CLIST (mylist), datum, 1, &p);
	g_strlcpy (targetname, p, sizeof (targetname));


	g_snprintf (str, sizeof (str), "%s: %s", _("To"), targetname);
	tn = g_strdelimit (str, "_", ' ');
	gtk_frame_set_label (GTK_FRAME (destframe), tn);
	gtk_clist_get_text (GTK_CLIST (mylist), datum, 2, &p);
	coordinate_string2gdouble(p, &target_lat);
	gtk_clist_get_text (GTK_CLIST (mylist), datum, 3, &p);
	coordinate_string2gdouble(p, &target_lon);
	/*    gtk_timeout_add (5000, (GtkFunction) sel_targetweg_cb, widget); */
	g_timer_stop (disttimer);
	g_timer_start (disttimer);
	olddist = dist;

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

	saytarget = TRUE;
	routenearest = 9999999;

	return TRUE;
}

/* *****************************************************************************
 */
gint
accepttext (GtkWidget * widget, gpointer data)
{
	GtkTextIter start, end;
	gchar *p;
	GtkWidget *wi;

	gtk_text_buffer_get_bounds (getmessagebuffer, &start, &end);

	gtk_text_buffer_apply_tag_by_name (getmessagebuffer, "word_wrap",
					   &start, &end);

	p = gtk_text_buffer_get_text (getmessagebuffer, &start, &end, FALSE);

	strncpy (messagesendtext, p, 300);
	messagesendtext[301] = 0;
	if ( mydebug > 8 )
		fprintf (stderr, "friends: message:\n%s\n", messagesendtext);
	gtk_widget_destroy (widget);
	wi = gtk_item_factory_get_item (item_factory,
					N_("/Misc. Menu/Messages"));
	statuslock = TRUE;
	gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid,
			    _("Sending message to friends server..."));
	gtk_widget_set_sensitive (wi, FALSE);
	return TRUE;
}

/* *****************************************************************************
 */
gint
textstatus (GtkWidget * widget, gpointer * datum)
{
	gint i;
	GtkTextIter start, end;
	gchar str[20];

	gtk_text_buffer_get_bounds (getmessagebuffer, &start, &end);
	gtk_text_buffer_apply_tag_by_name (getmessagebuffer, "word_wrap",
					   &start, &end);
	i = gtk_text_iter_get_offset (&end);
	if (i >= 300)
	{
		gdk_beep ();
		/* gtk_text_buffer_delete (getmessagebuffer, &end-1, &end); */
	}
	g_snprintf (str, sizeof (str), "%d/300", i);

	gtk_statusbar_pop (GTK_STATUSBAR (messagestatusbar),
			   messagestatusbarid);
	gtk_statusbar_push (GTK_STATUSBAR (messagestatusbar),
			    messagestatusbarid, str);
	return TRUE;
}

/* *****************************************************************************
 */
gint
setmessage_cb (GtkWidget * widget, guint datum)
{
	gchar b[100];
	gchar *p;
	int i;
	gchar titlestr[60];
	static GtkWidget *window = NULL;
	GtkWidget *ok, *cancel;
	GtkWidget *vpaned;
	GtkWidget *view1;
	GtkWidget *sw, *hbox, *vbox;
	GtkTextIter iter;
	gchar pre[180];
	time_t t;
	struct tm *ts;
	GtkTooltips *tooltips;

	p = b;

	gtk_clist_get_text (GTK_CLIST (mylist), datum, 0, &p);
	g_strlcpy (messagename, p, sizeof (messagename));
	for (i = 0; (size_t) i < strlen (messagename); i++)
		if (messagename[i] == ' ')
			messagename[i] = '_';

	gtk_widget_destroy (GTK_WIDGET (messagewindow));


	/* create window to enter text */

	window = gtk_dialog_new ();
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_signal_connect_object ((GTK_OBJECT (window)), "delete_event",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (window));
	gtk_signal_connect_object ((GTK_OBJECT (window)), "destroy",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (window));

	ok = gtk_button_new_from_stock (GTK_STOCK_APPLY);
	gtk_window_set_default_size (GTK_WINDOW (window), 320, 240);
	g_snprintf (titlestr, sizeof (titlestr), "%s %s", _("Message for:"),
		    messagename);
	gtk_window_set_title (GTK_WINDOW (window), titlestr);
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);
	vpaned = gtk_vpaned_new ();
	gtk_container_set_border_width (GTK_CONTAINER (vpaned), 5);

	vbox = gtk_vbox_new (FALSE, 3);
	hbox = gtk_hbutton_box_new ();

	messagestatusbar = gtk_statusbar_new ();
	messagestatusbarid =
		gtk_statusbar_get_context_id (GTK_STATUSBAR
					      (messagestatusbar), "message");

	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), vpaned, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), messagestatusbar, FALSE, FALSE,
			    3);
	/*   gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    hbox, TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX (hbox), ok, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (hbox), cancel, TRUE, TRUE, 3);
	view1 = gtk_text_view_new ();

	getmessagebuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view1));
	g_signal_connect (GTK_TEXT_BUFFER (getmessagebuffer),
			  "changed", G_CALLBACK (textstatus), frame_status);

	gtk_text_buffer_get_iter_at_offset (getmessagebuffer, &iter, 0);

	gtk_text_buffer_create_tag (getmessagebuffer, "word_wrap",
				    "wrap_mode", GTK_WRAP_WORD, NULL);
	gtk_text_buffer_insert_with_tags_by_name (getmessagebuffer, &iter,
						  "", -1, "word_wrap", NULL);

	time (&t);
	ts = localtime (&t);
	g_snprintf (pre, sizeof (pre), _("Date: %s"), asctime (ts));
	gtk_text_buffer_insert (getmessagebuffer, &iter, pre, -1);

	gtk_signal_connect_object ((GTK_OBJECT (ok)), "clicked",
				   GTK_SIGNAL_FUNC (accepttext),
				   GTK_OBJECT (window));

	gtk_signal_connect_object ((GTK_OBJECT (cancel)), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (window));
	tooltips = gtk_tooltips_new ();
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), ok,
			      _
			      ("Sends your text to to selected computer using the friends server"),
			      NULL);

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
	gtk_widget_show_all (window);


	return TRUE;
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
	thisline = atol (p);

	return TRUE;
}

/* *****************************************************************************
 */
gint
jumpwp_cb (GtkWidget * widget, guint datum)
{
	gint i;
	gchar *p;

	i = deleteline;
	gtk_clist_get_text (GTK_CLIST (mylist), i, 2, &p);
	coordinate_string2gdouble(p, &current_lat);
	gtk_clist_get_text (GTK_CLIST (mylist), i, 3, &p);
	coordinate_string2gdouble(p, &current_lon);

	if ((!posmode) && (!simmode))
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
scaler_cb (GtkAdjustment * scaler_adj, gdouble * datum)
{
	gchar s2[100];
	scalewanted = nlist[(gint) rint (scaler_adj->value)];
	g_snprintf (s2, sizeof (s2), "1:%d", scalewanted);
	gtk_label_set_text (GTK_LABEL (label_prefscale), s2);
	if ( mydebug > 0 )
		g_print ("Scaler: %d\n", scalewanted);
	needtosave = TRUE;

	return TRUE;
}

/* *****************************************************************************
 * Reactions to key pressed
 */
gint
key_cb (GtkWidget * widget, GdkEventKey * event)
{
	gdouble lat, lon;
	gint x, y;
	GdkModifierType state;

	if ( mydebug > 0 )
		g_print ("event:%x key:%c\n", event->keyval, event->keyval);


	// Toggle Grid Display
	if ((toupper (event->keyval)) == 'G')
	{
		do_draw_grid = !do_draw_grid;
		needtosave = TRUE;
	}

	// Toggle POI Display
	/*
	 * if ((toupper (event->keyval)) == 'I' )
	 * {
	 * poi_draw = !poi_draw;
	 * poi_draw_list ();
	 * gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poi_draw_bt), poi_draw);
	 * needtosave = TRUE;
	 * }
	 */

	// Toggle Friends Server activities
	if ((toupper (event->keyval)) == 'F')
	{
		havefriends = !havefriends;
		needtosave = TRUE;
	}

	// Add Waypoint at current gps location
	if ((toupper (event->keyval)) == 'X')
	{
		wplat = current_lat;
		wplon = current_lon;
		addwaypoint_cb (NULL, NULL);
	}

	// Add Waypoint at current gps location without asking
	if ((toupper (event->keyval)) == 'W')
	{
		gchar wp_name[100], wp_type[100];
		time_t t;
		struct tm *ts;
		time (&t);
		ts = localtime (&t);
		g_snprintf (wp_name, sizeof (wp_name), "%s", asctime (ts));
		g_snprintf (wp_type, sizeof (wp_type), "Automatic_key");

		addwaypoint (wp_name, wp_type, current_lat, current_lon);
	}

	// Add waypoint a current mouse location
	if ((toupper (event->keyval)) == 'Y')
	{

		gdk_window_get_pointer (drawing_area->window, &x, &y, &state);

		calcxytopos (x, y, &lat, &lon, zoom);

		if ( mydebug > 0 )
			printf ("Actual Position: lat:%f,lon:%f (x:%d,y:%d)\n", lat, lon, x, y);
		/*  Add mouse position as waypoint */
		wplat = lat;
		wplon = lon;
		addwaypoint_cb (NULL, 0);
	}

	// Add instant waypoint a current mouse location
	if ((toupper (event->keyval)) == 'P')
	{
		gchar wp_name[100], wp_type[100];
		time_t t;
		struct tm *ts;
		time (&t);
		ts = localtime (&t);
		g_snprintf (wp_name, sizeof (wp_name), "%s", asctime (ts));
		g_snprintf (wp_type, sizeof (wp_type), "Automatic_key");

		gdk_window_get_pointer (drawing_area->window, &x, &y, &state);
		calcxytopos (x, y, &lat, &lon, zoom);
		//		if ( mydebug > 0 )
		printf ("Add Waypoint: %s lat:%f,lon:%f (x:%d,y:%d)\n", wp_name, lat, lon, x, y);
		addwaypoint (wp_name, wp_type, lat, lon);
	}

	// Query Info for next points and streets
	if ( ( (toupper (event->keyval)) == '?' )
	     || ( (toupper (event->keyval)) == 'Q') )
	{
	    gdk_window_get_pointer (drawing_area->window, &x, &y, &state);
	    gdouble lat1,lon1,lat2,lon2;
	    gint delta = 10;
	    calcxytopos (x-delta, y-delta, &lat1, &lon1, zoom);
	    calcxytopos (x+delta, y+delta, &lat2, &lon2, zoom);
	    printf ("---------------------------------------------\n");
	    if ( poi_draw )
		poi_query_area     ( min(lat1,lat2), min(lon1,lon2), max(lat1,lat2), max(lon1,lon2) );

	    gdouble lat,lon;
	    calcxytopos (x, y, &lat, &lon, zoom);
	    gdouble dist=lat2-lat1;
	    dist = dist>0?dist:-dist;
	    if ( tracks_draw )
		tracks_query_point  ( lat,lon, dist );
	    if ( streets_draw )
		streets_query_point ( lat,lon, dist );
	}

	// In Route mode Force next Route Point
	if (((toupper (event->keyval)) == 'J') && routemode)
	{
		forcenextroutepoint = TRUE;
	}

	// Switch Night Mode
	if ((toupper (event->keyval)) == 'N')
	{
		if (nightmode != 0)
		{
			/*  light on for 30 seconds  */
			if (disableisnight == TRUE)
			{
				/*        gtk_timeout_remove (nighttimer); */
				disableisnight = FALSE;
			}
			else
				lighton ();
		}
	}

	// Zoom in/out
	{
		/*   From Russell Mirov: */
		if (pdamode)
		{
			if (event->keyval == 0xFF52)
				scalerbt_cb (NULL, 1);	/* RNM */
			if (event->keyval == 0xFF54)
				scalerbt_cb (NULL, 2);	/* RNM */
		}

		if ((toupper (event->keyval)) == '+' || (event->keyval == 0xFFab))	// Zoom in
		{
			scalerbt_cb (NULL, 2);
		}

		if ((toupper (event->keyval)) == '-' || (event->keyval == 0xFFad))	// Zoom out
		{
			scalerbt_cb (NULL, 1);
		}
	}


	return 0;
}

/* *****************************************************************************
 */
gint
minimapclick_cb (GtkWidget * widget, GdkEventMotion * event)
{
	gint x, y, px, py;
	gdouble dif, lon, lat;
	GdkModifierType state;

	if (event->is_hint)
		gdk_window_get_pointer (event->window, &x, &y, &state);
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
	if (state == 0)
		return 0;
#define MINISCREEN_X_2 64
#define MINISCREEN_Y_2 51
	px = (MINISCREEN_X_2 - x) * 10 * pixelfact;
	py = (-MINISCREEN_Y_2 + y) * 10 * pixelfact;
	lat = zero_lat - py / (lat2radius (current_lat) * M_PI / 180.0);
	lat = zero_lat - py / (lat2radius (lat) * M_PI / 180.0);
	lon = zero_lon -
		px / ((lat2radius (lat) * M_PI / 180.0) *
		      cos (M_PI * lat / 180.0));

	if ( proj_top == map_proj )
	    //if (mapistopo == FALSE)
	{
		dif = lat * (1 -
			     (cos ((M_PI * fabs (lon - zero_lon)) / 180.0)));
		lat = lat - dif / 1.5;
	}
	else
		dif = 0;
	lon = zero_lon -
		px / ((lat2radius (lat) * M_PI / 180.0) *
		      cos (M_PI * lat / 180.0));


	/*        g_print("\nstate: %x x:%d y:%d\n", state, x, y); */

	/*  Left mouse button */
	if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
	{
	    if (posmode)
		{
		    posmode_x = lon;
		    posmode_y = lat;
		    rebuildtracklist ();
		}
	}
	/*  Middle mouse button */
	if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt),
					      FALSE);
		rebuildtracklist ();
	}

	/*    g_print("\nx: %d, y: %d\n", x, y); */
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
		wplon = lo;
	s = g_strstrip ((char *)
			gtk_entry_get_text (GTK_ENTRY (add_wp_lat_text)));
	coordinate_string2gdouble(s, &la);
	if ((la > -181) && (la < 181))
		wplat = la;

	return TRUE;
}


/* *****************************************************************************
 * Add waypoint at wplat, wplon
 * with Strings for Name and Type
 */
void
addwaypoint (gchar * wp_name, gchar * wp_type, gdouble wp_lat, gdouble wp_lon)
{
	gint i;
	if (wp_from_sql)
	{
		insertsqldata (wp_lat, wp_lon, (char *) wp_name,
			       (char *) wp_type);
		get_sql_type_list ();
		getsqldata ();
	}
	else
	{
		i = maxwp;
		(wayp + i)->lat = wp_lat;
		(wayp + i)->lon = wp_lon;
		g_strdelimit ((char *) wp_name, " ", '_');

		/*  limit waypoint name to 20 chars */
		g_strlcpy ((wayp + i)->name, wp_name, 40);
		(wayp + i)->name[20] = 0;

		g_strlcpy ((wayp + i)->typ, wp_type, 40);
		(wayp + i)->typ[40] = 0;

		(wayp + i)->wlan = 0;

		maxwp++;
		if (maxwp >= wpsize)
		{
			wpsize += 1000;
			wayp = g_renew (wpstruct, wayp, wpsize);
		}
		savewaypoints ();
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
	gchar wp_type[80];

	s1 = gtk_entry_get_text (GTK_ENTRY (add_wp_name_text));
	s2 = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (wptext2)->entry));
	g_strlcpy(wp_name,s1,sizeof(wp_name));
	g_strlcpy(wp_type,s2,sizeof(wp_type));

	addwaypoint (wp_name, wp_type, wplat, wplon);

	gtk_widget_destroy (GTK_WIDGET (datum));
	markwaypoint = FALSE;

	return TRUE;
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
	GtkWidget *hbox4;
	GtkWidget *vbox;
	gchar buff[40];
	GList *wp_types = NULL;
	gint i;


	addwaypointwindow = window = gtk_dialog_new ();
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));
	gotowindow = window;
	markwaypoint = TRUE;

	gtk_window_set_modal (GTK_WINDOW (window), TRUE);
	gtk_window_set_title (GTK_WINDOW (window), _("Add waypoint name"));

	vbox = gtk_vbox_new (TRUE, 2);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), vbox, TRUE,
			    TRUE, 2);

	{			// Name
		GtkWidget *add_wp_name_hbox;
		GtkWidget *add_wp_name_label;
		add_wp_name_text = gtk_entry_new ();
		add_wp_name_label = gtk_label_new (_(" Waypoint name: "));
		gtk_window_set_focus (GTK_WINDOW (window), add_wp_name_text);

		add_wp_name_hbox = gtk_hbox_new (TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_name_hbox),
				    add_wp_name_label, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_name_hbox),
				    add_wp_name_text, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (vbox), add_wp_name_hbox, TRUE,
				    TRUE, 2);
	}

	{			// Types
		GtkWidget *add_wp_type_label;
		if (wp_from_sql)
		{
			get_sql_type_list ();
			for (i = 0; i < wp_typelistcount; i++)
				wp_types =
					g_list_append (wp_types,
						       wp_typelist[i]);
		}
		else
		{
			wp_types = g_list_append (wp_types, "");
			wp_types = g_list_append (wp_types, "Airport");
			wp_types = g_list_append (wp_types, "BurgerKing");
			wp_types = g_list_append (wp_types, "Cafe");
			wp_types = g_list_append (wp_types, "GasStation");
			wp_types = g_list_append (wp_types, "Geocache");
			wp_types = g_list_append (wp_types, "Golf");
			wp_types = g_list_append (wp_types, "Hotel");
			wp_types = g_list_append (wp_types, "McDonalds");
			wp_types = g_list_append (wp_types, "Monu");
			wp_types = g_list_append (wp_types, "Nightclub");
			wp_types = g_list_append (wp_types, "Rest");
			wp_types = g_list_append (wp_types, "Shop");
			wp_types = g_list_append (wp_types, "Speedtrap");
		}

		GtkWidget *add_wp_type_hbox;
		/*   wptext2 = gtk_entry_new (); */
		wptext2 = gtk_combo_new ();
		gtk_combo_set_popdown_strings (GTK_COMBO (wptext2),
					       (GList *) wp_types);

		add_wp_type_label = gtk_label_new (_(" Waypoint type: "));

		add_wp_type_hbox = gtk_hbox_new (TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_type_hbox),
				    add_wp_type_label, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_type_hbox), wptext2, TRUE,
				    TRUE, 2);
		gtk_box_pack_start (GTK_BOX (vbox), add_wp_type_hbox, TRUE,
				    TRUE, 2);

	}

	{			// Lon
		GtkWidget *add_wp_lon_label;
		add_wp_lon_text = gtk_entry_new_with_max_length (20);
		coordinate2gchar(buff, sizeof(buff), wplon, FALSE, minsecmode);
		gtk_entry_set_text (GTK_ENTRY (add_wp_lon_text), buff);
		add_wp_lon_label = gtk_label_new (_("Longitude"));
		gtk_signal_connect (GTK_OBJECT (add_wp_lon_text), "changed",
				    GTK_SIGNAL_FUNC (addwaypointchange_cb),
				    (gpointer) 1);
		hbox4 = gtk_hbox_new (TRUE, 2);
		gtk_box_pack_start (GTK_BOX (hbox4), add_wp_lon_label, TRUE,
				    TRUE, 2);
		gtk_box_pack_start (GTK_BOX (hbox4), add_wp_lon_text, TRUE,
				    TRUE, 2);
		gtk_box_pack_start (GTK_BOX (vbox), hbox4, TRUE, TRUE, 2);
	}

	{			// Lat
		GtkWidget *add_wp_lat_label;
		GtkWidget *add_wp_lat_hbox;
		add_wp_lat_text = gtk_entry_new_with_max_length (20);
		coordinate2gchar(buff, sizeof(buff), wplat, TRUE, minsecmode);
		gtk_entry_set_text (GTK_ENTRY (add_wp_lat_text), buff);
		add_wp_lat_label = gtk_label_new (_("Latitude"));
		gtk_signal_connect (GTK_OBJECT (add_wp_lat_text), "changed",
				    GTK_SIGNAL_FUNC (addwaypointchange_cb),
				    (gpointer) 2);
		add_wp_lat_hbox = gtk_hbox_new (TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_lat_hbox),
				    add_wp_lat_label, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_lat_hbox),
				    add_wp_lat_text, TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (vbox), add_wp_lat_hbox, TRUE,
				    TRUE, 2);
	}


	{			// Buttons
		GtkWidget *add_wp_button_hbox;
		GtkWidget *button, *button2;
		button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
		button2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);

		GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
		GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
		gtk_signal_connect (GTK_OBJECT (button), "clicked",
				    GTK_SIGNAL_FUNC (addwaypoint_gtk_cb),
				    GTK_OBJECT (window));
		gtk_signal_connect (GTK_OBJECT (button2), "clicked",
				    GTK_SIGNAL_FUNC (addwaypointdestroy_cb),
				    0);
		gtk_signal_connect (GTK_OBJECT (window), "delete_event",
				    GTK_SIGNAL_FUNC (addwaypointdestroy_cb),
				    0);


		add_wp_button_hbox = gtk_hbox_new (TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_button_hbox), button,
				    TRUE, TRUE, 2);
		gtk_box_pack_start (GTK_BOX (add_wp_button_hbox), button2,
				    TRUE, TRUE, 2);
		gtk_window_set_default (GTK_WINDOW (window), button);
		/*   gtk_box_pack_start (GTK_BOX (vbox), add_wp_button_hbox, TRUE, TRUE, 2); */
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
	sortcolumn = (gint) datum;

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
					   (gint) sortcolumn);
		gtk_clist_sort (GTK_CLIST (mylist));
	}
	return TRUE;
}


/* *****************************************************************************
 */
gint
click_clist (GtkWidget * widget, GdkEventButton * event, gpointer data)
{

	g_print ("\nclist: %d, data: %d\n", event->button, thisline);
	if ((event->button == 3))
	{

		return TRUE;
	}

	return FALSE;

}

/* *****************************************************************************
 * Select recipient for message to a mobile target over friendsd
 */
gint
sel_message_cb (GtkWidget * widget, guint datum)
{
	GtkWidget *window;
	gchar *tabeltitel1[] = {
		_("Name"), _("Latitude"), _("Longitude"), _("Distance"),
		NULL
	};
	GtkWidget *scrwindow, *vbox, *button;

	window = gtk_dialog_new ();
	/*    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE); */
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));
	messagewindow = window;
	gtk_window_set_title (GTK_WINDOW (window),
			      _("Please select message recipient"));

	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	if (pdamode)
		gtk_window_set_default_size (GTK_WINDOW (window),
					     real_screen_x, real_screen_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (window), 400, 360);

	mylist = gtk_clist_new_with_titles (4, tabeltitel1);

	gtk_signal_connect_object (GTK_OBJECT
				   (GTK_CLIST (mylist)),
				   "click-column",
				   GTK_SIGNAL_FUNC (setsortcolumn),
				   GTK_OBJECT (window));

	gtk_signal_connect (GTK_OBJECT (GTK_CLIST (mylist)),
			    "select-row",
			    GTK_SIGNAL_FUNC (setmessage_cb),
			    GTK_OBJECT (mylist));


	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_window_set_default (GTK_WINDOW (window), button);
	gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				   GTK_SIGNAL_FUNC
				   (gtk_widget_destroy), GTK_OBJECT (window));
	gtk_signal_connect_object (GTK_OBJECT (window),
				   "delete_event",
				   GTK_SIGNAL_FUNC
				   (gtk_widget_destroy), GTK_OBJECT (window));

	insertwaypoints (TRUE);
	gtk_clist_set_column_justification (GTK_CLIST (mylist), 3,
					    GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 1, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 2, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 3, TRUE);


	scrwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scrwindow), mylist);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
					(scrwindow),
					(GtkPolicyType)
					GTK_POLICY_AUTOMATIC,
					(GtkPolicyType) GTK_POLICY_AUTOMATIC);
	vbox = gtk_vbox_new (FALSE, 2 * PADDING);
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), scrwindow, TRUE, TRUE,
			    2 * PADDING);

	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    button, TRUE, TRUE, 2);

	gtk_widget_show_all (window);
	return TRUE;
}


/* *****************************************************************************
 */
gint
sel_target_cb (GtkWidget * widget, guint datum)
{
	GtkWidget *window;
	gchar *tabeltitel1[] = { "#",
		_("Waypoint"), _("Latitude"), _("Longitude"), _("Distance"),
		NULL
	};
	GtkWidget *scrwindow, *vbox, *button, *hbox, *deletebt, *gotobt;
	GtkTooltips *tooltips;

	if (setwpactive)
		return TRUE;


	setwpactive = TRUE;
	window = gtk_dialog_new ();
	/*    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE); */
	gotowindow = window;
	gtk_window_set_transient_for (GTK_WINDOW (window),
				      GTK_WINDOW (mainwindow));

	if (datum == 1)
	{
		gtk_window_set_modal (GTK_WINDOW (window), TRUE);
		gtk_window_set_title (GTK_WINDOW (window),
				      _("Select reference point"));
	}
	else
		gtk_window_set_title (GTK_WINDOW (window),
				      _("Please select your destination"));
	if (pdamode)
		gtk_window_set_default_size (GTK_WINDOW (window),
					     real_screen_x, real_screen_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (window), 400, 360);

	mylist = gtk_clist_new_with_titles (5, tabeltitel1);
	if (datum == 1)
		gtk_signal_connect (GTK_OBJECT (GTK_CLIST (mylist)),
				    "select-row",
				    GTK_SIGNAL_FUNC (setrefpoint_cb),
				    GTK_OBJECT (mylist));
	else
	{
		gtk_signal_connect (GTK_OBJECT (GTK_CLIST (mylist)),
				    "select-row",
				    GTK_SIGNAL_FUNC (setwp_cb),
				    GTK_OBJECT (mylist));
		/*       gtk_signal_connect (GTK_OBJECT (mylist), "button-release-event", */
		/*                    GTK_SIGNAL_FUNC (click_clist), NULL); */
	}

	gtk_signal_connect (GTK_OBJECT
			    (GTK_CLIST (mylist)),
			    "click-column", GTK_SIGNAL_FUNC (setsortcolumn),
			    0);

	if (datum != 1)
	{
		if (routemode)
			create_route_button =
				gtk_button_new_with_label (_("Edit route"));
		else
			create_route_button =
				gtk_button_new_with_label (_("Create route"));
		GTK_WIDGET_SET_FLAGS (create_route_button, GTK_CAN_DEFAULT);
		gtk_signal_connect (GTK_OBJECT (create_route_button),
				    "clicked",
				    GTK_SIGNAL_FUNC (create_route_cb), 0);
	}

	deletebt = gtk_button_new_from_stock (GTK_STOCK_DELETE);
	GTK_WIDGET_SET_FLAGS (deletebt, GTK_CAN_DEFAULT);
	gtk_signal_connect (GTK_OBJECT (deletebt), "clicked",
			    GTK_SIGNAL_FUNC (delwp_cb), 0);

	gotobt = gtk_button_new_from_stock (GTK_STOCK_JUMP_TO);
	GTK_WIDGET_SET_FLAGS (gotobt, GTK_CAN_DEFAULT);
	gtk_signal_connect (GTK_OBJECT (gotobt), "clicked",
			    GTK_SIGNAL_FUNC (jumpwp_cb), 0);

	/*   button = gtk_button_new_with_label (_("Close")); */
	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_window_set_default (GTK_WINDOW (window), button);
	gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
				   GTK_SIGNAL_FUNC
				   (sel_targetweg_cb), GTK_OBJECT (window));
	gtk_signal_connect_object (GTK_OBJECT (window),
				   "delete_event",
				   GTK_SIGNAL_FUNC
				   (sel_targetweg_cb), GTK_OBJECT (window));

	/* Font aendern falls PDA-Mode und Touchscreen */
	if (pdamode)
	{
		if (onemousebutton)
		{

			/* Change default font throughout the widget */
			PangoFontDescription *font_desc;
			font_desc =
				pango_font_description_from_string
				("Sans 20");
			gtk_widget_modify_font (mylist, font_desc);
			pango_font_description_free (font_desc);
		}
	}

	insertwaypoints (FALSE);
	gtk_clist_set_column_justification (GTK_CLIST (mylist), 4,
					    GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification (GTK_CLIST (mylist), 0,
					    GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 1, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 2, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 3, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 4, TRUE);

	scrwindow = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_add (GTK_CONTAINER (scrwindow), mylist);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
					(scrwindow),
					(GtkPolicyType)
					GTK_POLICY_AUTOMATIC,
					(GtkPolicyType) GTK_POLICY_AUTOMATIC);
	vbox = gtk_vbox_new (FALSE, 2 * PADDING);
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), scrwindow, TRUE, TRUE,
			    2 * PADDING);
	hbox = gtk_hbutton_box_new ();
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    hbox, TRUE, TRUE, 2);

	if (datum != 1)
	{
		gtk_box_pack_start (GTK_BOX (hbox), create_route_button, TRUE,
				    TRUE, 2 * PADDING);
		gtk_box_pack_start (GTK_BOX (hbox), deletebt, TRUE, TRUE,
				    2 * PADDING);
		gtk_box_pack_start (GTK_BOX (hbox), gotobt, TRUE, TRUE,
				    2 * PADDING);
	}
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, TRUE, 2 * PADDING);
	/*    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER); */

	/*   I remove this, because you can sort by mouseclick now */
	/*   selwptimeout = gtk_timeout_add (30000, (GtkFunction) reinsertwp_cb, 0); */
	tooltips = gtk_tooltips_new ();
	if (!createroute)
		gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips),
				      create_route_button,
				      _
				      ("Create a route using some waypoints from this list"),
				      NULL);
	if (setwpactive)
		gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), deletebt,
				      _
				      ("Delete the selected waypoint from the waypoint list"),
				      NULL);
	if (setwpactive)
		gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), gotobt,
				      _("Jump to the selected waypoint"),
				      NULL);

	gtk_widget_show_all (window);

	return TRUE;
}



/* *****************************************************************************
 */
void
usage ()
{
    
    g_print ("%s%s%s%s%s%s%s%s%s%s"
#ifdef DBUS_ENABLE
	     "%s"
#endif
	     "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	     "\nCopyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>"
	     "\n         Website: http://www.gpsdrive.de\n\n",
	     _("-v        show version\n"),
	     _("-h        print this help\n"),
	     _("-d        turn on debug info\n"),
	     _("-D X      set debug Level to X\n"),
	     _("-T        do some internal unit Tests(don't start gpsdrive)\n"),
	     _("-e        use Festival-Lite (flite) for speech output\n"),
	     _("-t        set serial device for GPS i.e. /dev/ttyS1\n"),
	     _("-o        serial device, pty master, or file for NMEA *output*\n"),
	     _("-f X      Select friends server, X is i.e. www.gpsdrive.cc\n"),
	     _("-n        Disable use of direct serial connection\n"),
#ifdef DBUS_ENABLE
	     _("-X        Use DBUS for communication with gpsd. This disables serial and socket communication\n"),
#endif
	     _("-l LANG   Select language of the voice,\n"
	       "          LANG may be english, spanish or german\n"),
	     _("-s HEIGHT set height of the screen, if autodetection\n"
	       "          don't satisfy you, X is i.e. 768,600,480,200\n"),
	     _("-r WIDTH  set width of the screen, only with -s\n"),
	     _("-1        have only 1 button mouse, for example using touchscreen\n"),
	     _("-a        don't display battery status (i.e. broken APM)\n"),
	     _("-b Server Servername for NMEA server (if gpsd runs on another host)\n"),
	     _("-c WP     set start position in simulation mode to waypoint name WP\n"),
	     _("-x        create separate window for menu\n"),
	     _("-p        set settings for PDA (iPAQ, Yopy...)\n"),
	     _("-i        ignore NMEA checksum (risky, only for broken GPS receivers\n"),
	     _("-q        disable SQL support\n"),
	     _("-F        force display of position even it is invalid\n"),
	     _("-S        don't show splash screen\n"),
	     _("-P        start in Pos Mode\n"),
	     _("-E        print out data received from direct serial connection\n"),
	     _("-W x      set x to 1 to switch WAAS/EGNOS on, set to 0 to switch off\n"),
	     _("-H ALT    correct altitude, adding this value (ALT) to altitude\n"),
	     _("-z        don't display zoom factor and scale\n\n"));

}

/* *****************************************************************************
 * Load track file and displays it 
 */
gint
loadtrack_cb (GtkWidget * widget, gpointer datum)
{
	GtkWidget *fdialog;
	gchar buf[1000];
	fdialog = gtk_file_selection_new (_("Select a track file"));
	gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
	gtk_window_set_transient_for (GTK_WINDOW (fdialog),
				      GTK_WINDOW (mainwindow));

	gtk_signal_connect (GTK_OBJECT
			    (GTK_FILE_SELECTION (fdialog)->ok_button),
			    "clicked", GTK_SIGNAL_FUNC (gettrackfile),
			    GTK_OBJECT (fdialog));
	gtk_signal_connect_object (GTK_OBJECT
				   (GTK_FILE_SELECTION (fdialog)->
				    cancel_button), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (fdialog));


	g_strlcpy (buf, homedir, sizeof (buf));
	g_strlcat (buf, "track*.sav", sizeof (buf));
	gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), buf);
	gtk_widget_show (fdialog);

	return TRUE;
}



/* *****************************************************************************
 * save waypoints to way.txt 
 */
void
savewaypoints ()
{
	gchar mappath[2048], la[20], lo[20];
	FILE *st;
	gint i, e;


	g_strlcpy (mappath, homedir, sizeof (mappath));
	g_strlcat (mappath, activewpfile, sizeof (mappath));

	st = fopen (mappath, "w+");
	if (st == NULL)
	{
		perror (mappath);
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

			e = fprintf (st, "%-22s %10s %11s %s\n",
				     (wayp + i)->name, la, lo,
				     (wayp + i)->typ);
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
    
    g_strlcpy (fn_way_txt, homedir, sizeof (fn_way_txt));
    if (!wp_from_sql)
	g_strlcat (fn_way_txt, activewpfile, sizeof (fn_way_txt));
    else
	g_strlcat (fn_way_txt, "way-SQLRESULT.txt", sizeof (fn_way_txt));


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
	    g_print ("\nwp_from_sql: %d\n",wp_from_sql);
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
	    g_strlcpy ((wayp + i)->typ, "", 40);
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

		    if (!wp_from_sql)
			{
			    if ((strncmp
				 ((wayp + i)->name, "R-",
				  2)) == 0)
				(wayp + i)->action = 1;
			}

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


    if (!wp_from_sql)
	g_print ("%s reloaded\n", activewpfile);
    else
	g_print ("%s reloaded\n", "way-SQLRESULT.txt");
}



/* *****************************************************************************
 * on a USR2 signal, re-start the GPS connection  
 */
void
usr2handler (int sig)
{
	g_print ("\ngot SIGUSR2\n");
	initgps ();
}


/*******************************************************************************
 *                                                                             *
 *                             Main program                                    *
 *                                                                             *
 *******************************************************************************/
int
main (int argc, char *argv[])
{
    GtkWidget *vbig, *vbig1, *vbox, *vbox2, *vbox4;
    GtkWidget *hbig, *hbox2;
    GtkWidget *hbox2a, *hbox2b, *vmenubig;
    GtkWidget *zoomin, *hbox3, *vboxlow, *hboxlow;
    GtkWidget *menuwin = NULL, *menuwin2 = NULL;
    GtkWidget *zoomout, *sel_target, *vtable, *wplabeltable, *alignment1;
    GtkWidget *alignment2;
    gchar maintitle[100];
    /*   GdkColor farbe;   */
    GdkRectangle rectangle = {
	0, 0, SCREEN_X, SCREEN_Y
    };
    const gchar *hd;
    gchar buf[500];

    /*** Mod by Arms */
    gint i, screen_height, screen_width;
    GtkWidget *table1, *wi;
    GtkTooltips *tooltips;
    gchar s1[100];
    /*** Mod by Arms */
    GtkRequisition requ, *reqptr;
    GtkWidget *mainnotebook;
#ifndef NOPLUGINS
    GModule *mod1;
    void (*modulefunction) ();
    gchar *modpath;
#endif
    char s3[200];
    struct tm *lt;
    time_t local_time, gmt_time;
    /*   GtkAccelGroup *accel_group; */
    gint nmenu_items = sizeof (main_menu) / sizeof (main_menu[0]);
    GdkPixbuf *mainwindow_icon_pixbuf;
    gdouble f;
#ifdef USETELEATLAS
    GtkWidget *navibt;
#endif
    
    tzset ();
    gmt_time = time (NULL);
       
    lt = gmtime (&gmt_time);
    local_time = mktime (lt);
    zone = lt->tm_isdst + (gmt_time - local_time) / 3600;
    /*   fprintf(stderr,"\n zeitzone: %d\n",zone); */
    
    /*   zone = st->tm_gmtoff / 3600; */
    /*  initialize variables */
    /*  Hamburg */
    srand (gmt_time);
    f = 0.02 * (0.5 - rand () / (RAND_MAX + 1.0));
    current_lat = zero_lat = 53.623672 + f;
    f = 0.02 * (0.5 - rand () / (RAND_MAX + 1.0));
    current_lon = zero_lon = 10.055441 + f;
    /*    zero_lat and zero_lon are overwritten by gpsdriverc,  */
    tripreset ();
    
    g_strlcpy (dgpsserver, "dgps.wsrcc.com", sizeof (dgpsserver));
    g_strlcpy (dgpsport, "2104", sizeof (dgpsport));
    g_strlcpy (gpsdservername, "127.0.0.1", sizeof (gpsdservername));
    direction = angle_to_destination = 0;
    g_strlcpy (targetname, "     ", sizeof (targetname));
    g_strlcpy (utctime, "n/a", sizeof (utctime));
    g_strlcpy (oldangle, "none", sizeof (oldangle));
    pixelfact = MAPSCALE / PIXELFACT;
    g_strlcpy (oldfilename, "", sizeof (oldfilename));
    simmode = maploaded = FALSE;
    haveNMEA = FALSE;
    havepos = gblink = blink = FALSE;
    haveposcount = haveGARMIN = debug = 0;
    zoom = 1;
    milesflag = iszoomed = FALSE;
    sel_target = NULL;
#ifdef DBUS_ENABLE
    useDBUS = FALSE;
#endif
    g_strlcpy (wplabelfont, "Sans 11", sizeof (wplabelfont));
    g_strlcpy (bigfont, "Sans bold 26", sizeof (bigfont));
    g_strlcpy (friendsserverip, "127.0.0.1", sizeof (friendsserverip));
    g_strlcpy (friendsserverfqn, "www.gpsdrive.cc",
	       sizeof (friendsserverfqn));
    g_strlcpy (friendsidstring, "XXX", sizeof (friendsidstring));
    
    signal (SIGUSR2, usr2handler);
    timer = g_timer_new ();
    disttimer = g_timer_new ();
    g_timer_start (timer);
    g_timer_start (disttimer);
    memset (satlist, 0, sizeof (satlist));
    memset (satlistdisp, 0, sizeof (satlist));
    buffer = g_new (char, 2010);
    big = g_new (char, MAXBIG + 10);
    
    timeoutcount = lastp = bigp = bigpRME = bigpGSA = bigpGSV = bigpGGA = 0;
    lastp = lastpGGA = lastpGSV = lastpRME = lastpGSA = 0;
    gcount = xoff = yoff = 0;
    hours = minutes = 99;
    milesconv = 1.0;
    /* set default color to navyblue */
    g_strlcpy (bluecolor, "navyblue", sizeof (bluecolor));
    g_strlcpy (trackcolor, "green3", sizeof (trackcolor));
    g_strlcpy (friendscolor, "orange", sizeof (friendscolor));
    g_strlcpy (messagename, "", sizeof (messagename));
    g_strlcpy (messageack, "", sizeof (messageack));
    g_strlcpy (messagesendtext, "", sizeof (messagesendtext));
    
    downloadwindowactive = downloadactive = importactive = FALSE;
    g_strlcpy (lastradar, "", sizeof (lastradar));
    g_strlcpy (lastradar2, "", sizeof (lastradar2));
    g_strlcpy (activewpfile, "way.txt", sizeof (activewpfile));
    g_strlcpy (dbhost, "localhost", sizeof (dbhost));
    g_strlcpy (dbuser, "gast", sizeof (dbuser));
    g_strlcpy (dbpass, "gast", sizeof (dbpass));
    g_strlcpy (dbname, "geoinfo", sizeof (dbname));
    g_strlcpy (dbtable, "waypoints", sizeof (dbtable));
    dbdistance = 2000.0;
    dbusedist = TRUE;
    g_strlcpy (loctime, "n/a", sizeof (loctime));
    voicelang = english;
    track = g_new0 (GdkSegment, 100000);
    trackshadow = g_new0 (GdkSegment, 100000);
    tracknr = 0;
    trackcoord = g_new0 (trackcoordstruct, 100000);
    trackcoordnr = 0;
    tracklimit = trackcoordlimit = 100000;
    init_route_list ();

    earthr = calcR (current_lat);

    /* homedir is the directory where the maps and other  */
    /* files are stored (~/.gpsdrive) */
    hd = g_get_home_dir ();
    g_strlcpy (homedir, hd, sizeof (homedir));
    g_strlcat (homedir, "/.gpsdrive/", sizeof (homedir));
    g_snprintf (mapdir, sizeof (mapdir),"%s%s",homedir,"maps");

    /*  all default values must be set BEFORE readconfig! */
    g_strlcpy (setpositionname, "", sizeof (setpositionname));
    g_strlcpy (serialdev, "/dev/ttyS3", sizeof (serialdev));

    /* setup signal handler */
    signal (SIGUSR1, signalposreq);

    sql_load_lib();
    /*  I18l */

    /*  Detect the language for voice output */
    {
	gchar **lstr, lstr2[200];
	gchar *localestring;
	    
	localestring = setlocale (LC_ALL, "");
	if (localestring == NULL)
	    localestring = setlocale (LC_MESSAGES, "");
	if (localestring != NULL)
	    {
		lstr = g_strsplit (localestring, ";", 50);
		g_strlcpy (lstr2, "", 50);
		for (i = 0; i < 50; i++)
		    if (lstr[i] != NULL)
			{
			    if ((strstr (lstr[i], "LC_MESSAGES")) != NULL)
				{
				    g_strlcpy (lstr2, lstr[i], 50);
				    break;
				}
			}
		    else
			{
			    g_strlcpy (lstr2, lstr[i - 1], 50);
			    break;
			}
		g_strfreev (lstr);
	    }
	    
	/* detect voicelang */
	if ((strstr (lstr2, "de_")) != NULL)
	    voicelang = german;
	else if ((strstr (lstr2, "es_")) != NULL)
	    voicelang = spanish;
	else
	    voicelang = english;

	/*    needed for right decimal delimiter ('.' or ',') */
	setlocale(LC_NUMERIC, "en_US");
    }

    g_strlcpy (friendsname, "", sizeof (friendsname));



    readconfig ();

    real_screen_x = 640;
    real_screen_y = 512;
    target_lon = current_lon + 0.00001;
    target_lat = current_lat + 0.00001;
    gdk_color_parse (trackcolor, &trackcolorv);
    gdk_color_parse (friendscolor, &orange);

    /*  setting defaults if setting is not yet in configuraton file */
    if ((milesflag + metricflag + nauticflag) == 0)
	metricflag = TRUE;



    /*  load waypoints before locale is set! */
    /*  Attention! In this file the decimal point is always a '.' !! */

    /*  load mapfile configurations */
    /*  Attention! In this file the decimal point is that what locale says, 
     * i.e. '.' in english, ',' in german!! 
     */
    loadmapconfig ();

    /* PORTING */
    {
	gchar *p;
	p = bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "utf8");
	p = textdomain (GETTEXT_PACKAGE);
	p = textdomain (NULL);
    }

    /*    Setting locale for correct Umlauts */
    gtk_set_locale ();
    
    /*  initialization for GTK+ */
    gtk_init (&argc, &argv);

    /* Needed 4 hours to find out that this is IMPORTANT!!!! */
    gdk_rgb_init ();
    screen_height = gdk_screen_height ()-50;
    screen_width = gdk_screen_width ();

    /* parse cmd args */
    do
	{
	    i = getopt (argc, argv,
			"W:ESA:ab:c:zXx1qivPdD:TFepH:hnf:l:t:s:o:r:?");
	    switch (i)
		{
		case 'a':
		    disableapm = TRUE;
		    break;
		case 'S':
		    nosplash = TRUE;
		    break;
		case 'E':
		    nmeaverbose = TRUE;
		    break;
		case 'q':
		    usesql = FALSE;
		    break;
		case 'd':
		    debug = TRUE;
		    break;
		case 'D':
		    mydebug = strtol (optarg, NULL, 0);
		    debug = TRUE;
		    break;
		case 'T':
		    do_unit_test = TRUE;
		    break;
		case 'e':
		    useflite = TRUE;
		    break;
		case 'i':
		    ignorechecksum = TRUE;
		    g_print ("\nWARNING: NMEA checksum test switched off!\n\n");
		    break;
		case 'x':
		    extrawinmenu = TRUE;
		    break;
		case 'X':
#ifdef DBUS_ENABLE
		    useDBUS = TRUE;
#else
		    g_print ("\nWARNING: You need to enable DBUS support with './configure --enable-dbus'!\n");
#endif
		    break;
		case 'p':
		    pdamode = TRUE;
		    break;
		case '1':
		    onemousebutton = TRUE;
		    break;
		case 'v':
		    printf ("\ngpsdrive (c) 2001-2004 Fritz Ganter <ganter@ganter.at>\n" "\nVersion %s\n%s\n\n", VERSION, rcsid);
		    exit (0);
		    break;
		case 't':
		    g_strlcpy (serialdev, optarg, sizeof (serialdev));
		    break;
		case 'A':
		    alarm_dist = strtod (optarg, NULL);
		    break;
		case 'b':
		    g_strlcpy (gpsdservername, optarg,
			       sizeof (gpsdservername));
		    break;
		case 'c':
		    g_strlcpy (setpositionname, optarg,
			       sizeof (setpositionname));
		    break;
		case 'f':
		case 'n':
		    disableserialcl = TRUE;
		    break;
		case 's':
		    screen_height = strtol (optarg, NULL, 0);
		    break;
		case 'W':
		    switch (strtol (optarg, NULL, 0))
			{
			case 0:
			    egnosoff = TRUE;
			    break;
			case 1:
			    egnoson = TRUE;
			    break;
			}
		    break;
		case 'l':
		    if (!strcmp (optarg, "english"))
			voicelang = english;
		    else if (!strcmp (optarg, "german"))
			voicelang = german;
		    else if (!strcmp (optarg, "spanish"))
			voicelang = spanish;
		    else
			{
			    usage ();
			    g_print (_
				     ("\nYou can only choose between english, spanish and german\n\n"));
			    exit (0);
			}
		    break;
		case 'o':
		    nmeaout = opennmea (optarg);
		    break;
		case 'h':
		    usage ();
		    exit (0);
		    break;
		case 'H':
		    normalnull = strtol (optarg, NULL, 0);
		    break;
		case '?':
		    usage ();
		    exit (0);
		    break;
		case 'r':
		    screen_width = strtol (optarg, NULL, 0);
		    break;
		case 'z':
		    zoomscale = FALSE;
		    break;
		case 'F':
		    forcehavepos = TRUE;
		    break;
		case 'P':
		    posmode = TRUE;
		    break;
		    
		}
	}
    while (i != -1);
	
    if ((strlen (friendsname) == 0))
	g_strlcpy (friendsname, _("EnterYourName"),
		   sizeof (friendsname));

    load_icons ();

    /*  Build array for earth radii */
    for (i = -100; i <= 100; i++)
	lat2RadiusArray[i + 100] = calcR (i);

    gethostname (hostname, 256);
    proxyport = 80;
    haveproxy = FALSE;

    if ( mydebug > 0 )
	printf ("\ngpsdrive (c) 2001-2004 Fritz Ganter <ganter@ganter.at>\n"
		"\nVersion %s\n%s\n\n", VERSION, rcsid);
    

    get_proxy_from_env();

    if ( mydebug > 0 )
	g_print ("\nGpsDrive version %s\n%s\n", VERSION, rcsid);

    /*  show splash screen */
    if (!nosplash)
	splash ();
    { // Set locale for the use of atof()  
	gchar buf[5];  
	sprintf(buf,"%.1f",1.2);  
	localedecimal=buf[1];  
    }  
  
    /* init sql support */
    if (usesql)
	usesql = sqlinit ();

    if (!usesql)
	wp_from_sql = FALSE;

    /* Create toplevel window */

    PSIZE = 50;
    SMALLMENU = 0;
    PADDING = 1;
    if (screen_height >= 1024)		 /* > 1280x1024 */
	{
	    real_screen_x = min(1280,screen_width-300);
	    real_screen_y = min(1024,screen_height-200);
	    if ( mydebug > 0 )
		g_print ("Set real Screen size to %d,%d\n", 
			 real_screen_x,real_screen_y);
		
	}
    else if (screen_height >= 768)	/* 1024x768 */
	{
	    real_screen_x = 800;
	    real_screen_y = 540;
	}
    else if (screen_height >= 600)	/* 800x600 */
	{
	    real_screen_x = 690;
	    real_screen_y = 455;
	    PADDING = 0;
	}
    else if (screen_height >= 480)	/* 640x480 */
	{
	    if (screen_width == 0)
		real_screen_x = 630;
	    else
		real_screen_x = screen_width - XMINUS;
	    real_screen_y = screen_height - YMINUS;
	}
    else if (screen_height < 480)
	{
	    if (screen_width == 0)
		real_screen_x = 220;
	    else
		real_screen_x = screen_width - XMINUS;
	    real_screen_y = screen_height - YMINUS;
	    PSIZE = 25;
	    SMALLMENU = 1;
	    PADDING = 0;
	}

    if ((extrawinmenu) && (screen_width != 0))
	{
	    real_screen_x += XMINUS - 10;
	    real_screen_y += YMINUS - 30;
	}

    if (   ((screen_width  == 240) && (screen_height == 320)) 
	   || ((screen_height == 240) && (screen_width  == 320)) )
	{
	    pdamode = TRUE;
	    /* SMALLMENU = 1; */
	    real_screen_x = screen_width - 8;
	    real_screen_y = screen_height - 70;
	}
    if (pdamode)
	{
	    extrawinmenu = TRUE;
	    PADDING = 0;
	    g_strlcpy (wplabelfont, "Sans 8", sizeof (wplabelfont));
	    g_strlcpy (bigfont, "Sans bold 16", sizeof (bigfont));
	    g_print ("\nPDA mode\n");
	}

    if (real_screen_x < real_screen_y)
	borderlimit = real_screen_x / 5;
    else
	borderlimit = real_screen_y / 5;

    if (borderlimit < 30)
	borderlimit = 30;

    SCREEN_X_2 = SCREEN_X / 2;
    SCREEN_Y_2 = SCREEN_Y / 2;
    PSIZE = 50;
    posx = SCREEN_X_2;
    posy = SCREEN_Y_2;

    // TODO: decide if this is needed
    //KCFX
    /*   if (pdamode) mod_setupcounter++;  */
    // Fritz commented out above line

#ifndef NOPLUGINS
    useplugins = TRUE;
#endif
    if (pdamode)
	useplugins = FALSE;

    if (usesql)
	{
	    mod_setupcounter++;
	    setupfunction[mod_setupcounter] = &(sqlsetup);
	    sqlplace = mod_setupcounter;
	}
    mod_setupcounter++;
    setupfunction[mod_setupcounter] = &(friendssetup);
    friendsplace = mod_setupcounter;

#ifndef NOPLUGINS
    if (useplugins)
	{
	    /*  fly module */
	    modpath = g_module_build_path (LIBDIR, "libfly.so");
	    mod1 = g_module_open (".libs/libfly.so", 0);
	    if (mod1 == NULL)
		mod1 = g_module_open (modpath, 0);
	    if (mod1 != NULL)
		{
		    gint *modulever;
		    mod_setupcounter++;

		    // warning: dereferencing type-punned pointer will break strict-aliasing rules
		    i = g_module_symbol (mod1, "moduleversion",
					 (gpointer *) & modulever);
		    g_print (" (Version %d)", *modulever);
		    // warning: dereferencing type-punned pointer will break strict-aliasing rules
		    i = g_module_symbol (mod1, "modulesetup",
					 (gpointer *) & modulefunction);
		    setupfunction[mod_setupcounter] = modulefunction;
		}

	    /*  nautic module */
	    modpath = g_module_build_path (LIBDIR, "libnautic.so");
	    mod1 = g_module_open (".libs/libnautic.so", 0);
	    if (mod1 == NULL)
		mod1 = g_module_open (modpath, 0);
	    if (mod1 != NULL)
		{
		    gint *modulever;
		    mod_setupcounter++;
		    // warning: dereferencing type-punned pointer will break strict-aliasing rules
		    i = g_module_symbol (mod1, "moduleversion",
					 (gpointer *) & modulever);
		    g_print (" (Version %d)", *modulever);
		    // warning: dereferencing type-punned pointer will break strict-aliasing rules
		    i = g_module_symbol (mod1, "modulesetup",
					 (gpointer *) & modulefunction);
		    setupfunction[mod_setupcounter] = modulefunction;
		}
	}
#endif
    fprintf (stderr, "\n");



    // ==================================================================
    // Unit Tests
    if ( do_unit_test ) {
	unit_test();
    }


    mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    if (!nosplash)
	gtk_window_set_transient_for (GTK_WINDOW (splash_window),
				      GTK_WINDOW (mainwindow));



    g_snprintf (maintitle, sizeof (maintitle),
		"%s v%s  \xc2\xa9 2001-2004 Fritz Ganter", "GpsDrive",
		VERSION);

    gtk_window_set_title (GTK_WINDOW (mainwindow), maintitle);
    gtk_container_set_border_width (GTK_CONTAINER (mainwindow), 0);
    gtk_window_set_position (GTK_WINDOW (mainwindow), GTK_WIN_POS_CENTER);
    gtk_signal_connect (GTK_OBJECT (mainwindow), "delete_event",
			GTK_SIGNAL_FUNC (quit_program), NULL);

    gtk_signal_connect (GTK_OBJECT (mainwindow), "key_press_event",
			GTK_SIGNAL_FUNC (key_cb), NULL);

    frame_status = gtk_statusbar_new ();
    statusid =
	gtk_statusbar_get_context_id (GTK_STATUSBAR (frame_status), "main");

    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid,
			_("Gpsdrive-2 (c)2001-2004 F.Ganter"));
    if (!useflite)
	havespeechout = speech_out_init ();
    else
	havespeechout = TRUE;

    if (havespeechout)
	gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid,
			    _("Using speech output"));
    if (!useflite)
	switch (voicelang)
	    {
	    case english:
		speech_out_speek_raw (FESTIVAL_ENGLISH_INIT);
		break;
	    case spanish:
		speech_out_speek_raw (FESTIVAL_SPANISH_INIT);
		break;
	    case german:
		speech_out_speek_raw (FESTIVAL_GERMAN_INIT);
		break;
	    }

    vbig = gtk_vbox_new (FALSE, 0 * PADDING);
    vmenubig = gtk_vbox_new (FALSE, 0 * PADDING);
    hbig = gtk_hbox_new (FALSE, 0 * PADDING);
    item_factory =
	gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", NULL);
    /*  Uebersetzen laut Bluefish Code */
    gtk_item_factory_set_translate_func (item_factory, menu_translate,
					 "<main>", NULL);
    gtk_item_factory_create_items (item_factory, nmenu_items, main_menu,
				   NULL);

    menubar = gtk_item_factory_get_widget (item_factory, "<main>");

    wi = gtk_item_factory_get_item (item_factory,
				    N_("/Misc. Menu/Waypoint Manager"));
    if (!debug)
	gtk_widget_set_sensitive (wi, FALSE);


    if (havespeechout)
	{
	    mutebt = gtk_check_button_new_with_label (_("M_ute"));
	    gtk_button_set_use_underline (GTK_BUTTON (mutebt), TRUE);
	    if (muteflag)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (mutebt), TRUE);

	    gtk_signal_connect (GTK_OBJECT (mutebt),
				"clicked", GTK_SIGNAL_FUNC (mute_cb),
				(gpointer) 1);
	}

    // Checkbox ---- POI Draw
    poi_draw_bt = gtk_check_button_new_with_label (_("draw PO_I"));
    gtk_button_set_use_underline (GTK_BUTTON (poi_draw_bt), TRUE);
    if (!poi_draw)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poi_draw_bt),
				      TRUE);
    gtk_signal_connect (GTK_OBJECT (poi_draw_bt), "clicked",
			GTK_SIGNAL_FUNC (poi_draw_cb), (gpointer) 1);
    /*
     * gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), poi_draw_bt,
     * _("This will show Point of interrest located in mySQL Database"),
     * NULL);
     */

    // Checkbox ---- STREETS Draw
    streets_draw_bt =
	gtk_check_button_new_with_label (_("draw _Streets"));
    gtk_button_set_use_underline (GTK_BUTTON (streets_draw_bt), TRUE);
    if (!streets_draw)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (streets_draw_bt), TRUE);
    gtk_signal_connect (GTK_OBJECT (streets_draw_bt), "clicked",
			GTK_SIGNAL_FUNC (streets_draw_cb), (gpointer) 1);
    /*
     * gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), streets_draw_bt,
     * _("This will show Streets Data located in mySQL Database"),
     * NULL);
     */

    // Checkbox ---- TRACK Draw
    tracks_draw_bt =
	gtk_check_button_new_with_label (_("draw _Track"));
    gtk_button_set_use_underline (GTK_BUTTON (tracks_draw_bt), TRUE);
    if (!tracks_draw)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (tracks_draw_bt), TRUE);
    gtk_signal_connect (GTK_OBJECT (tracks_draw_bt), "clicked",
			GTK_SIGNAL_FUNC (tracks_draw_cb), (gpointer) 1);
    /*
     * gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), tracks_draw_bt,
     * _("This will show Track Data located in mySQL Database"),
     * NULL);
     */


    // Checkbox ----   Use SQL
    sqlbt = gtk_check_button_new_with_label (_("Use SQ_L"));
    gtk_button_set_use_underline (GTK_BUTTON (sqlbt), TRUE);
    if (wp_from_sql)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sqlbt),
				      TRUE);

    gtk_signal_connect (GTK_OBJECT (sqlbt),
			"clicked", GTK_SIGNAL_FUNC (sql_cb),
			(gpointer) 1);



    // Checkbox ---- Show WP
    wpbt = gtk_check_button_new_with_label (_("Show _WP"));
    gtk_button_set_use_underline (GTK_BUTTON (wpbt), TRUE);
    if (wpflag)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wpbt), TRUE);
    gtk_signal_connect (GTK_OBJECT (wpbt),
			"clicked", GTK_SIGNAL_FUNC (wp_cb), (gpointer) 1);


    if (wp_from_sql)
	{
	    get_sql_type_list ();
	    getsqldata ();
	}
    else
	loadwaypoints ();

    for (i = 0; i < maxwp; i++)
	{
	    if (strlen (setpositionname) > 0)
		{
		    if (!(strcasecmp ((wayp + i)->name, setpositionname)))
			{
			    current_lat = (wayp + i)->lat;
			    current_lon = (wayp + i)->lon;
			    target_lon = current_lon + 0.00001;
			    target_lat = current_lat + 0.00001;
			}
		    if (!(strcasecmp ((wayp + i)->name, _("HomeBase"))))
			{
			    alarm_lat = (wayp + i)->lat;
			    alarm_lon = (wayp + i)->lon;
			}
		}
	}

    posbt = gtk_check_button_new_with_label (_("Pos. _mode"));
    gtk_button_set_use_underline (GTK_BUTTON (posbt), TRUE);

    gtk_signal_connect (GTK_OBJECT (posbt),
			"clicked", GTK_SIGNAL_FUNC (pos_cb),
			(gpointer) 1);

    trackbt = gtk_check_button_new_with_label (_("Show _Track"));
    gtk_button_set_use_underline (GTK_BUTTON (trackbt), TRUE);
    if (trackflag)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trackbt),
				      TRUE);
    gtk_signal_connect (GTK_OBJECT (trackbt), "clicked",
			GTK_SIGNAL_FUNC (track_cb), (gpointer) 1);

    /*   importbt = gtk_button_new_with_label (_("Import")); */
    /*   gtk_signal_connect (GTK_OBJECT (importbt), */
    /*                "clicked", GTK_SIGNAL_FUNC (import1_cb), (gpointer) 1); */
    /*    GTK_WIDGET_SET_FLAGS (importbt, GTK_CAN_DEFAULT); */

    /*   loadtrackbt = gtk_button_new_from_stock (GTK_STOCK_OPEN); */
    /*   gtk_signal_connect (GTK_OBJECT (loadtrackbt), */
    /*                "clicked", GTK_SIGNAL_FUNC (loadtrack_cb), */
    /*                (gpointer) 1); */
    /*    GTK_WIDGET_SET_FLAGS (loadtrackbt, GTK_CAN_DEFAULT); */

#ifdef USETELEATLAS
    {
	GtkWidget *image3, *hbox3, *alignment3, *label;

	ta_init ();
	navibt = gtk_button_new ();
	gtk_signal_connect (GTK_OBJECT (navibt),
			    "clicked", GTK_SIGNAL_FUNC (navi_cb),
			    (gpointer) 1);
	gtk_widget_show (navibt);
	alignment3 = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_widget_show (alignment3);
	gtk_container_add (GTK_CONTAINER (navibt), alignment3);


	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_container_add (GTK_CONTAINER (alignment3), hbox3);

	image3 = create_pixmap (mainwindow, "pixmaps/gpsiconbt.png");
	gtk_widget_show (image3);
	gtk_box_pack_start (GTK_BOX (hbox3), image3, FALSE, FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("_Navigation"));
	gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);


    }
#endif

    /* PORTING */
    /*   setupbt = gtk_button_new_with_label (_("Setup")); */
    setupbt = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);

    gtk_signal_connect (GTK_OBJECT (setupbt),
			"clicked", GTK_SIGNAL_FUNC (setup_cb),
			(gpointer) 0);

    // Checkbox ---- Best Map
    bestmapbt = gtk_check_button_new_with_label (_("Auto _best map"));
    gtk_button_set_use_underline (GTK_BUTTON (bestmapbt), TRUE);

    if (!scaleprefered)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmapbt),
				      TRUE);
    gtk_signal_connect (GTK_OBJECT (bestmapbt), "clicked",
			GTK_SIGNAL_FUNC (bestmap_cb), (gpointer) 1);


    // Checkbox ---- Save Track
    savetrackfile (0);
    g_snprintf (s1, sizeof (s1), "%s", _("Save track"));
    savetrackbt = gtk_check_button_new_with_label (s1);
    if (savetrack)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrackbt),
				      TRUE);
    gtk_signal_connect (GTK_OBJECT (savetrackbt), "clicked",
			GTK_SIGNAL_FUNC (savetrack_cb), (gpointer) 1);

    g_snprintf (s1, sizeof (s1), "%s", savetrackfn);
    lab1 = gtk_label_new (s1);


    frame_toogles = gtk_frame_new (NULL);
    vbox4 = gtk_vbox_new (TRUE, 1 * PADDING);
    gtk_container_add (GTK_CONTAINER (frame_toogles), vbox4);

    frame_maptype = make_display_map_checkboxes();

    /*  Zoom in button */
    zoomin = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
    gtk_signal_connect (GTK_OBJECT (zoomin),
			"clicked", GTK_SIGNAL_FUNC (zoom_cb),
			(gpointer) 1);
    /*    GTK_WIDGET_SET_FLAGS (zoomin, GTK_CAN_DEFAULT); */
    /*  Zoom out button */
    zoomout = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
    gtk_signal_connect (GTK_OBJECT (zoomout),
			"clicked", GTK_SIGNAL_FUNC (zoom_cb),
			(gpointer) 2);
    /*    GTK_WIDGET_SET_FLAGS (zoomout, GTK_CAN_DEFAULT); */


    scalerrbt = gtk_button_new_with_label (">>");
    gtk_signal_connect (GTK_OBJECT (scalerrbt),
			"clicked", GTK_SIGNAL_FUNC (scalerbt_cb),
			(gpointer) 1);
    /*    GTK_WIDGET_SET_FLAGS (scalerrbt, GTK_CAN_DEFAULT); */
    scalerlbt = gtk_button_new_with_label ("<<");
    gtk_signal_connect (GTK_OBJECT (scalerlbt),
			"clicked", GTK_SIGNAL_FUNC (scalerbt_cb),
			(gpointer) 2);
    /*    GTK_WIDGET_SET_FLAGS (scalerlbt, GTK_CAN_DEFAULT); */

    /*  Select target button */
    /*    if (maxwp > 0) */
    {
	/* PORTING */
	/*     sel_target = gtk_button_new_with_label (_("Select target")); */
	sel_target = gtk_button_new_from_stock (GTK_STOCK_FIND);
	/*     gtk_label_set_text(GTK_BUTTON(sel_target),_("Select target")); */
	gtk_signal_connect (GTK_OBJECT (sel_target),
			    "clicked",
			    GTK_SIGNAL_FUNC (sel_target_cb),
			    (gpointer) 2);
	/*        GTK_WIDGET_SET_FLAGS (sel_target, GTK_CAN_DEFAULT); */
    }




    /*    gtk_window_set_default (GTK_WINDOW (mainwindow), zoomin); */
    /*    if we want NMEA mode, gpsd must be running and we connect to port 2222 */
    /*    An alternate gpsd server may be on 2947, we try it also */

    initgps ();
    /*if (simmode)
      {
      if ((!disableserial) && (!disableserialcl))
      {
      haveserial = gpsserialinit ();
      if (haveserial)
      {
      simmode = FALSE;
      haveNMEA = TRUE;
      gtk_widget_set_sensitive (startgpsbt, FALSE);
      }
      }
      }*/

    if (haveGARMIN)
	gtk_widget_set_sensitive (startgpsbt, FALSE);

    friendsinit ();


    if (usesql)
	initkismet ();

    if( havekismet )
	{
	    g_print (_("\nkismet server found\n"));
	    g_snprintf( buf, sizeof(buf), speech_kismet_found[voicelang] );
	    speech_out_speek (buf);
	}

    load_friends_icon ();

    /*  Area for map */
    frame_map_area = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_map_area), GTK_SHADOW_IN);
    drawing_area = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), SCREEN_X,
			   SCREEN_Y);
    gtk_container_add (GTK_CONTAINER (frame_map_area), drawing_area);

    gtk_box_pack_start (GTK_BOX (vbig), frame_map_area, TRUE, TRUE, 0 * PADDING);
    gtk_widget_add_events (GTK_WIDGET (drawing_area),
			   GDK_BUTTON_PRESS_MASK);
    gtk_signal_connect_object (GTK_OBJECT (drawing_area),
			       "button-press-event",
			       GTK_SIGNAL_FUNC (mapclick_cb),
			       GTK_OBJECT (drawing_area));

    /* Area for navigation pointer */
    drawing_bearing = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_bearing), PSIZE + 2,
			   PSIZE + 2);
    gtk_signal_connect (GTK_OBJECT (drawing_bearing),
			"expose_event", GTK_SIGNAL_FUNC (expose_compass),
			NULL);

    /* Area for mini map */
    /* With small displays, this isn't really necessary! */
    if (SMALLMENU == 0)
	{
	    drawing_miniimage = gtk_drawing_area_new ();
	    gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_miniimage),
				   128, 103);
	    gtk_signal_connect (GTK_OBJECT (drawing_miniimage),
				"expose_event",
				GTK_SIGNAL_FUNC (expose_mini_cb), NULL);
	    gtk_widget_add_events (GTK_WIDGET (drawing_miniimage),
				   GDK_BUTTON_PRESS_MASK);
	    gtk_signal_connect_object (GTK_OBJECT (drawing_miniimage),
				       "button-press-event",
				       GTK_SIGNAL_FUNC (minimapclick_cb),
				       GTK_OBJECT (drawing_miniimage));
	}
    hbox2 = gtk_hbox_new (FALSE, 1 * PADDING);
    hbox2a = gtk_hbox_new (FALSE, 1 * PADDING);
    hbox2b = gtk_vbox_new (FALSE, 1 * PADDING);
    hbox3 = gtk_hbox_new (FALSE, 1 * PADDING);

    frame_bearing = gtk_frame_new (_("Bearing"));
    /*   gtk_container_add (GTK_CONTAINER (frame_bearing), drawing_bearing); */
    compasseventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (compasseventbox), drawing_bearing);
    alignment1 = gtk_alignment_new (0.6, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment1), compasseventbox);
    gtk_container_add (GTK_CONTAINER (frame_bearing), alignment1);

    gtk_box_pack_start (GTK_BOX (hbox2), frame_bearing, FALSE, FALSE,
			1 * PADDING);

    /* Area for field strength, we have data only in NMEA mode */
    drawing_sats = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_sats), PSIZE + 10,
			   PSIZE + 6);
    gtk_signal_connect (GTK_OBJECT (drawing_sats), "expose_event",
			GTK_SIGNAL_FUNC (expose_sats_cb), NULL);
    gtk_widget_add_events (GTK_WIDGET (drawing_sats),
			   GDK_BUTTON_PRESS_MASK);
    gtk_signal_connect (GTK_OBJECT (drawing_sats), "button-press-event",
			GTK_SIGNAL_FUNC (satpos_cb), NULL);

    if(!pdamode)
	{
	    frame_sats = gtk_frame_new (_("GPS Info"));
	    sateventbox = gtk_event_box_new ();
	    gtk_container_add (GTK_CONTAINER (sateventbox), drawing_sats);
	    alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
	    gtk_container_add (GTK_CONTAINER (alignment2), sateventbox);
	    satsvbox = gtk_vbox_new (FALSE, 1 * PADDING);
	    satshbox = gtk_hbox_new (FALSE, 1 * PADDING);
	    gtk_container_add (GTK_CONTAINER (frame_sats), satshbox);
	    gtk_box_pack_start (GTK_BOX (satshbox), alignment2, FALSE, FALSE,	1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (satshbox), satsvbox, FALSE, FALSE, 1 * PADDING);

	    satslabel1 = gtk_entry_new ();
	    satslabel1eventbox = gtk_event_box_new ();
	    gtk_container_add (GTK_CONTAINER (satslabel1eventbox), satslabel1);
	    gtk_entry_set_text (GTK_ENTRY (satslabel1), _("n/a"));
	    gtk_box_pack_start (GTK_BOX (satsvbox), satslabel1eventbox, TRUE, FALSE, 0 * PADDING);

	    satslabel2 = gtk_entry_new ();
	    satslabel2eventbox = gtk_event_box_new ();
	    gtk_container_add (GTK_CONTAINER (satslabel2eventbox), satslabel2);
	    gtk_entry_set_text (GTK_ENTRY (satslabel2), _("n/a"));
	    gtk_box_pack_start (GTK_BOX (satsvbox), satslabel2eventbox, TRUE, FALSE, 0 * PADDING);

	    satslabel3 = gtk_entry_new ();
	    satslabel3eventbox = gtk_event_box_new ();
	    gtk_container_add (GTK_CONTAINER (satslabel3eventbox), satslabel3);
	    gtk_entry_set_text (GTK_ENTRY (satslabel3), _("n/a"));
	    gtk_box_pack_start (GTK_BOX (satsvbox), satslabel3eventbox, TRUE, FALSE, 0 * PADDING);

	    gtk_entry_set_editable (GTK_ENTRY (satslabel1), FALSE);
	    gtk_widget_set_usize (satslabel1, 38, 20);
	    gtk_entry_set_editable (GTK_ENTRY (satslabel2), FALSE);
	    gtk_widget_set_usize (satslabel2, 38, 20);
	    gtk_entry_set_editable (GTK_ENTRY (satslabel3), FALSE);
	    gtk_widget_set_usize (satslabel3, 38, 20);

	    gtk_box_pack_start (GTK_BOX (hbox2), frame_sats, FALSE, FALSE, 1 * PADDING);
	}

    create_temperature_widget(hbox2);
    create_battery_widget(hbox2);


    if (pdamode)
	gtk_box_pack_start (GTK_BOX (hbox2), hbox2b, TRUE, TRUE,
			    1 * PADDING);

    /*  displays distance to destination */
    distlabel = gtk_label_new ("---");
    gtk_label_set_use_markup (GTK_LABEL (distlabel), TRUE);
    gtk_label_set_justify (GTK_LABEL (distlabel), GTK_JUSTIFY_RIGHT);

    /*  displays speed over ground */
    speedlabel = gtk_label_new (_("---"));
    gtk_label_set_justify (GTK_LABEL (speedlabel), GTK_JUSTIFY_RIGHT);

    /*  displays zoom factor of map */
    altilabel = gtk_label_new (_("n/a"));
    if (pdamode)
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"5000\">%s</span>",
			bluecolor, _("n/a"));
	}
    else
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
			bluecolor, _("n/a"));
	}
    gtk_label_set_markup (GTK_LABEL (altilabel), s3);
    gtk_label_set_justify (GTK_LABEL (altilabel), GTK_JUSTIFY_RIGHT);

    /*  displays waypoints number */
    wplabeltable = gtk_table_new (2, 6, TRUE);

    /* selected waypoints */
    wplabel1 = gtk_entry_new ();
    wp1eventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (wp1eventbox), wplabel1);
    gtk_entry_set_text (GTK_ENTRY (wplabel1), "--");
    gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp1eventbox, 0,
			       2, 0, 1);

    /* waypoints in range */
    wplabel2 = gtk_entry_new ();
    wp2eventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (wp2eventbox), wplabel2);
    gtk_entry_set_text (GTK_ENTRY (wplabel2), "--");
    gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp2eventbox, 2,
			       4, 0, 1);

    /* range in km */
    wplabel3 = gtk_entry_new ();
    wp3eventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (wp3eventbox), wplabel3);
    gtk_entry_set_text (GTK_ENTRY (wplabel3), "--");
    gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp3eventbox, 0,
			       3, 1, 2);

    /* gps time */
    wplabel4 = gtk_entry_new ();
    wp4eventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (wp4eventbox), wplabel4);
    gtk_entry_set_text (GTK_ENTRY (wplabel4), _("n/a"));
    gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp4eventbox, 3,
			       6, 1, 2);

    wplabel5 = gtk_entry_new ();
    wp5eventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (wp5eventbox), wplabel5);
    gtk_entry_set_text (GTK_ENTRY (wplabel5), _("n/a"));
    gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp5eventbox, 4,
			       6, 0, 1);


    gtk_entry_set_editable (GTK_ENTRY (wplabel1), FALSE);
    gtk_entry_set_editable (GTK_ENTRY (wplabel2), FALSE);
    gtk_entry_set_editable (GTK_ENTRY (wplabel3), FALSE);
    gtk_entry_set_editable (GTK_ENTRY (wplabel4), FALSE);
    gtk_entry_set_editable (GTK_ENTRY (wplabel5), FALSE);
    gtk_widget_set_usize (wplabel1, USIZE_X, USIZE_Y);
    gtk_widget_set_usize (wplabel2, USIZE_X, USIZE_Y);
    gtk_widget_set_usize (wplabel3, USIZE_X, USIZE_Y);
    gtk_widget_set_usize (wplabel4, USIZE_X, USIZE_Y);
    gtk_widget_set_usize (wplabel5, USIZE_X, USIZE_Y);



#ifdef AFDAFDA
    if (pdamode)
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Sans\"  size=\"7000\">%s %d\n%d %s %.1fkm</span>",
			bluecolor, _("Selected:"), wptotal, wpselected,
			_("within"), dbdistance);
	}
    else
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Sans\"  size=\"10000\">%s %d\n%d %s %.1fkm</span>",
			bluecolor, _("Selected:"), wptotal, wpselected,
			_("within"), dbdistance);
	}
    gtk_label_set_markup (GTK_LABEL (wplabel), s3);
    gtk_label_set_justify (GTK_LABEL (wplabel), GTK_JUSTIFY_LEFT);
#endif

    /*  create frames for labels */
    if(!pdamode)
	frame_target = gtk_frame_new (_("Distance to target"));
    else
	frame_target = gtk_frame_new (_("Distance"));
    destframe = frame_target;
    gtk_container_add (GTK_CONTAINER (frame_target), distlabel);
    if (pdamode)
	{
	    if (milesflag)
		g_snprintf (s1, sizeof (s1), "[%s]", _("mi/h"));
	    else if (nauticflag)
		g_snprintf (s1, sizeof (s1), "[%s]", _("knots"));
	    else
		g_snprintf (s1, sizeof (s1), "[%s]", _("km/h"));
	}
    else
	{
	    if (milesflag)
		g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
			    _("mi/h"));
	    else if (nauticflag)
		g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
			    _("knots"));
	    else
		g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
			    _("km/h"));
	}
    frame_speed = gtk_frame_new (s1);
    gtk_container_add (GTK_CONTAINER (frame_speed), speedlabel);
    /*** Mod by Arms */
    /*   if (!pdamode) */
    /*     gtk_box_pack_start (GTK_BOX (hbox2), frame_speed, TRUE, TRUE, */
    /*                  1 * PADDING); */
    if (!pdamode)
	{
	    frame_altitude = gtk_frame_new (_("Altitude"));
	    gtk_container_add (GTK_CONTAINER (frame_altitude), altilabel);
	}
    /*** Mod by Arms */
    /*   if (!pdamode) */
    /*     gtk_box_pack_start (GTK_BOX (hbox2), frame_altitude, FALSE, TRUE, */
    /*                  1 * PADDING); */

    frame_wp = gtk_frame_new (_("Waypoints"));
    /*   gtk_container_add (GTK_CONTAINER (frame_wp), wplabel); */
    /*** Mod by Arms */
    /*   if (!pdamode) */
    /*     gtk_box_pack_start (GTK_BOX (hbox2), frame_wp, FALSE, TRUE, 1 * PADDING); */

    if (!pdamode)
	vtable = gtk_table_new (1, 20, TRUE);
    else
	vtable = gtk_table_new (1, 20, FALSE);
    gtk_table_attach_defaults (GTK_TABLE (vtable), frame_target,    0,  6, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (vtable), frame_speed,     6, 12, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (vtable), frame_altitude, 12, 15, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (vtable), frame_wp,       15, 20, 0, 1);
    gtk_box_pack_start (GTK_BOX (hbox2), vtable, TRUE, TRUE, 2 * PADDING);
    gtk_container_add (GTK_CONTAINER (frame_wp), wplabeltable);




    /*    gtk_box_pack_start (GTK_BOX (vbig), hbox2, TRUE, TRUE, 2); */
    /*** Lots of Mods by Arms */
    vbox = gtk_vbox_new (TRUE, 3 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 1 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox), zoomin, FALSE, FALSE,  1 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox), zoomout, FALSE, FALSE, 1 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox), hbox3, FALSE, FALSE, 1 * PADDING);
    gtk_box_pack_start (GTK_BOX (hbox3), scalerlbt, TRUE, TRUE,	1 * PADDING);
    gtk_box_pack_start (GTK_BOX (hbox3), scalerrbt, TRUE, TRUE,	1 * PADDING);
    /*  only if we have read in waypoints we have the select target button */
    /*    if (maxwp > 0) */
    gtk_box_pack_start (GTK_BOX (vbox), sel_target, FALSE, FALSE, 1 * PADDING);
    /*   gtk_box_pack_start (GTK_BOX (vbox), importbt, FALSE, FALSE, 1 * PADDING);  */
    /*   gtk_box_pack_start (GTK_BOX (vbox), loadtrackbt, FALSE, FALSE, 1 * PADDING); */
    gtk_box_pack_start (GTK_BOX (vbox), startgpsbt, FALSE, FALSE, 1 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox), setupbt, FALSE, FALSE,    1 * PADDING);
#ifdef USETELEATLAS
    gtk_box_pack_start (GTK_BOX (vbox), navibt, FALSE, FALSE,     1 * PADDING);
#endif
    hboxlow = vbox2 = NULL;
    if (!extrawinmenu)
	{
	    vbox2 = gtk_vbox_new (FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox2), vbox, TRUE, TRUE,	1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox2), frame_toogles, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox2), frame_maptype, TRUE,	TRUE, 1 * PADDING);
	    if (SMALLMENU == 0)
		gtk_box_pack_start (GTK_BOX (vbox2),
				    GTK_WIDGET (drawing_miniimage),    TRUE, FALSE, 0 * PADDING);
	}
    else
	{
	    vboxlow = gtk_vbox_new (FALSE, 0 * PADDING);
	    hboxlow = gtk_hbox_new (FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vboxlow), frame_toogles, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vboxlow), frame_maptype, TRUE,	TRUE, 1 * PADDING);
	    if (SMALLMENU == 0)
		gtk_box_pack_start (GTK_BOX (vboxlow),
				    GTK_WIDGET (drawing_miniimage),   TRUE, FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (hboxlow), vbox, TRUE, TRUE,	1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (hboxlow), vboxlow, TRUE, TRUE,	1 * PADDING);

	}


    if (havespeechout)
	gtk_box_pack_start (GTK_BOX (vbox4), mutebt, FALSE, FALSE,    0 * PADDING);
    if (usesql)
	{
	    gtk_box_pack_start (GTK_BOX (vbox4), poi_draw_bt, FALSE,	FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox4), streets_draw_bt, FALSE,FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox4), tracks_draw_bt, FALSE,	FALSE, 0 * PADDING);
	}
    if (usesql)
	gtk_box_pack_start (GTK_BOX (vbox4), sqlbt, FALSE, FALSE,
			    0 * PADDING);
    /*    if (maxwp > 0) */
    gtk_box_pack_start (GTK_BOX (vbox4), wpbt, FALSE, FALSE, 0 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox4), posbt, FALSE, FALSE,0 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox4), trackbt, FALSE, FALSE,	0 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox4), bestmapbt, FALSE, FALSE,0 * PADDING);
    gtk_box_pack_start (GTK_BOX (vbox4), savetrackbt, FALSE, FALSE,0 * PADDING);
    //KCFX
    /*   if (!pdamode) */
    {
	scaler_adj = gtk_adjustment_new (slistsize / 2, 0, slistsize - 1, 1,
				  slistsize / 4, 1 / slistsize);
	scaler = gtk_hscale_new (GTK_ADJUSTMENT (scaler_adj));
	gtk_signal_connect (GTK_OBJECT (scaler_adj), "value_changed",
			    GTK_SIGNAL_FUNC (scaler_cb), NULL);
	gtk_scale_set_draw_value (GTK_SCALE (scaler), FALSE);
    }

    if (pdamode)
	table1 = gtk_table_new (5, 3, FALSE);
    else
	{
	    if (SMALLMENU)
		table1 = gtk_table_new (4, 3, FALSE);
	    else
		table1 = gtk_table_new (8, 2, FALSE);
	}
    frame_lat = gtk_frame_new (_("Latitude"));
    frame_lon = gtk_frame_new (_("Longitude"));
    if ( mydebug >10 )
	frame_mapfile = gtk_frame_new (_("Map file"));
    frame_mapscale = gtk_frame_new (_("Map scale"));
    frame_heading = gtk_frame_new (_("Heading"));
    frame_bearing = gtk_frame_new (_("Bearing"));
    frame_timedest = gtk_frame_new (_("Time at Dest."));
    frame_prefscale = gtk_frame_new (_("Pref. scale"));

    etch = !etch;
    etch_cb (NULL, 0);

    label_lat = gtk_label_new (_("000,00000N"));
    gtk_container_add (GTK_CONTAINER (frame_lat), label_lat);

    label_lon = gtk_label_new (_("000,00000E"));
    gtk_container_add (GTK_CONTAINER (frame_lon), label_lon);

    if ( mydebug >10 )
	{
	    label_map_filename = gtk_label_new (_("---"));
	    gtk_container_add (GTK_CONTAINER (frame_mapfile), label_map_filename);
	}

    label_map_scale = gtk_label_new (_("---"));
    gtk_container_add (GTK_CONTAINER (frame_mapscale), label_map_scale);

    label_heading = gtk_label_new (_("0000"));
    gtk_container_add (GTK_CONTAINER (frame_heading), label_heading);

    label_baering = gtk_label_new (_("0000"));
    gtk_container_add (GTK_CONTAINER (frame_bearing), label_baering);

    label_timedest = gtk_label_new (_("---"));
    gtk_container_add (GTK_CONTAINER (frame_timedest), label_timedest);

    label_prefscale = gtk_label_new (_("---"));
    gtk_container_add (GTK_CONTAINER (frame_prefscale), label_prefscale);

    if (pdamode)
	{
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_bearing, 0, 1, 0, 1);
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_heading, 1, 2, 0, 1);
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_mapscale, 2, 3, 0, 1);
	    
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_lat, 0, 1, 1, 2);
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_lon, 1, 2, 1, 2);
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_prefscale, 2, 3, 1, 2);
	    
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_timedest, 0, 1, 2, 3);
	    if ( mydebug >10 )
		gtk_table_attach_defaults (GTK_TABLE (table1), frame_mapfile, 1, 3, 2, 3);
	    //KCFX
	    gtk_table_attach_defaults (GTK_TABLE (table1), scaler, 0, 3, 3, 4);
	    gtk_table_attach_defaults (GTK_TABLE (table1), frame_status, 0, 3, 4, 5);
	}
    else
	{
	    if (SMALLMENU)
		{
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_bearing, 0, 1, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_heading, 1, 2, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_lat, 2,  3, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_lon, 3,  4, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_timedest, 0, 1, 1, 2);
		    if ( mydebug >10 )
			gtk_table_attach_defaults (GTK_TABLE (table1), frame_mapfile, 1, 2, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_mapscale, 2, 3, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_prefscale, 3, 4, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_status, 0, 4, 3, 4);
		    gtk_table_attach_defaults (GTK_TABLE (table1), scaler, 0, 4, 2, 3);
		}
	    else
		{
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_bearing, 0,    1, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_heading, 1,    2, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_lat, 2,      3, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_lon, 3,  4, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_timedest, 4,  5, 0, 1);
		    if ( mydebug >10 )
			gtk_table_attach_defaults (GTK_TABLE (table1), frame_mapfile, 5,  6, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_mapscale, 6,  7, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_prefscale, 7, 8, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1), frame_status,	   0, 4, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1), scaler,   4, 8, 1, 2);
		}
	}
    /*    gtk_box_pack_start (GTK_BOX (vbig), table1, FALSE, FALSE, 1); */
    /*  all position calculations are made in the expose callback */
    /*   if (!pdamode) */
    gtk_signal_connect (GTK_OBJECT (drawing_area),
			"expose_event", GTK_SIGNAL_FUNC (expose_cb),
			NULL);

    if (!pdamode)
	{
	    if (extrawinmenu)
		{
		    menuwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		    /*       gdk_window_lower((GdkWindow *)menuwin); */

		    gtk_window_set_title (GTK_WINDOW (menuwin),
					  _("Menu"));
		    /*** Mod by Arms */
		    gtk_container_set_border_width (GTK_CONTAINER
						    (menuwin),
						    2 * PADDING);
		    gtk_container_add (GTK_CONTAINER (menuwin), hboxlow);
		    gtk_signal_connect (GTK_OBJECT (menuwin),
					"delete_event",
					GTK_SIGNAL_FUNC (quit_program),
					NULL);
		    menuwin2 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		    /*       gdk_window_lower((GdkWindow *)menuwin2); */
		    gtk_window_set_title (GTK_WINDOW (menuwin2),
					  _("Status"));
		    /*** Mod by Arms */
		    gtk_container_set_border_width (GTK_CONTAINER
						    (menuwin2),
						    2 * PADDING);
		    vbig1 = gtk_vbox_new (FALSE, 2);
		    gtk_container_add (GTK_CONTAINER (menuwin2), vbig1);
		    /*** Mod by Arms */
		    gtk_box_pack_start (GTK_BOX (vbig1), hbox2, TRUE,
					TRUE, 2 * PADDING);
		    /*** Mod by Arms */
		    gtk_box_pack_start (GTK_BOX (vbig1), table1, TRUE,
					TRUE, 2 * PADDING);
		    gtk_signal_connect (GTK_OBJECT (menuwin2),
					"delete_event",
					GTK_SIGNAL_FUNC (quit_program),
					NULL);
		}
	    else
		{
		    /*** Mod by Arms */
		    //        gtk_box_pack_start (GTK_BOX (vmenubig), menubar, TRUE, TRUE,
		    //                            2 * PADDING);
		    //        gtk_box_pack_start (GTK_BOX (vmenubig), hbig, TRUE, TRUE,
		    //                            2 * PADDING);

		    gtk_box_pack_start (GTK_BOX (hbig), vbox2, TRUE, TRUE,
					1 * PADDING);
		    /*** Mod by Arms */
		    gtk_box_pack_start (GTK_BOX (vbig), hbox2, TRUE, TRUE,
					2 * PADDING);
		    /*** Mod by Arms */
		    gtk_box_pack_start (GTK_BOX (vbig), table1, FALSE,
					FALSE, 1 * PADDING);
		}
	}

    /*   if pdamode is set, we use gtk-notebook add arrange the elements */
    /*   in pdamode, extrawinmenu is also set! */
    mainnotebook = NULL;
    if (pdamode)
	{
	    GtkWidget *l1, *l2, *label_status;
	    l1 = gtk_label_new (NULL);
	    l2 = gtk_label_new (NULL);
	    label_status = gtk_label_new (NULL);
	    /* for a better usability in onemousebutton mode */
	    if (onemousebutton)
		{
		    /* gtk_misc_set_padding (GTK_MISC (l1), x, y); */
		    gtk_misc_set_padding (GTK_MISC (l1), 20, 1);
		    gtk_misc_set_padding (GTK_MISC (l2), 20, 1);
		    gtk_misc_set_padding (GTK_MISC (label_status), 20, 1);
		    
		    /* http://developer.gnome.org/doc/API/2.0/pango/PangoMarkupFormat.html */
		    
		    char *markup;
		    markup = g_markup_printf_escaped
			("<span font_desc='10'>%s</span>",
			 _("Map"));
		    gtk_label_set_markup (GTK_LABEL (l1), markup);
		    markup = g_markup_printf_escaped
			("<span font_desc='10'>%s</span>",
			 _("Menu"));
		    gtk_label_set_markup (GTK_LABEL (l2), markup);
		    markup = g_markup_printf_escaped
			("<span font_desc='10'>%s</span>",
			 _("Status"));
		    gtk_label_set_markup (GTK_LABEL (label_status), markup);
		    
		    g_free (markup);
		    
		}
	    else
		{
		    gtk_label_set_text (GTK_LABEL (l1), _("Map"));
		    gtk_label_set_text (GTK_LABEL (l2), _("Menu"));
		    gtk_label_set_text (GTK_LABEL (label_status), _("Status"));
		}
	    //KCFX
	    vbig1 = gtk_vbox_new (FALSE, 2);
	    //      gtk_container_add (GTK_CONTAINER (menuwin2), vbig1);
	    gtk_box_pack_start (GTK_BOX (vbig1), hbox2, TRUE, TRUE,
				2 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbig1), hbox2a, TRUE, TRUE,
				2 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbig1), hbox2b, TRUE, TRUE,
				2 * PADDING);
	    /*
	      gtk_box_pack_start (GTK_BOX (hbox2b), frame_speed, TRUE, TRUE,
	      1 * PADDING);
	      gtk_box_pack_start (GTK_BOX (hbox2b), frame_altitude, TRUE,
	      TRUE, 1 * PADDING);*/
	    //KCFX          
	    gtk_box_pack_start (GTK_BOX (hbox2a), frame_wp, TRUE, TRUE,
				1 * PADDING);

	    gtk_box_pack_start (GTK_BOX (vbig1), table1, TRUE, TRUE,
				2 * PADDING);

	    mainnotebook = gtk_notebook_new ();
	    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (mainnotebook),
				      GTK_POS_TOP);
	    gtk_box_pack_start (GTK_BOX (hbig), vbig, TRUE, TRUE,
				1 * PADDING);
	    gtk_container_add (GTK_CONTAINER (mainwindow), mainnotebook);
	    gtk_widget_show_all (hboxlow);
	    gtk_widget_show_all (vbig1);
	    gtk_widget_show_all (hbig);
	    gtk_widget_show_all (vbig);
	    gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), hbig,
				      l1);
	    gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook),
				      hboxlow, l2);
	    gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), vbig1,
				      label_status);
	    gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);
	    gtk_widget_show_all (mainnotebook);
	}
    else
	{
	    gtk_box_pack_start (GTK_BOX (hbig), vbig, TRUE, TRUE,
				1 * PADDING);
	    //      gtk_container_add (GTK_CONTAINER (mainwindow), vmenubig);
	    gtk_container_add (GTK_CONTAINER (mainwindow), hbig);
	}

    /*** Mod by Arms */
    /* This one should position the windows in the corners, */
    /* so that gpsdrive can be run w/o a xwm (stand-alone mode) */
    /* With a xwm, you should be able to reposition the */
    /* windows afterwards... */
    if (extrawinmenu && SMALLMENU && !pdamode)
	{
	    reqptr = &requ;
	    gtk_widget_size_request (GTK_WIDGET (menuwin2), reqptr);
	    gtk_widget_set_uposition (GTK_WIDGET (menuwin2), gdk_screen_width () - requ.width, gdk_screen_height () - requ.height);	/* rechts unten */
	    gtk_widget_size_request (GTK_WIDGET (mainwindow), reqptr);
	    gtk_widget_set_uposition (GTK_WIDGET (mainwindow), gdk_screen_width () - requ.width, 0);	/* rechts oben */
	    gtk_widget_size_request (GTK_WIDGET (menuwin), reqptr);
	    gtk_widget_set_uposition (GTK_WIDGET (menuwin), 0, 0);	/* links oben */
	}

    /*  Now show all Widgets */
    //KCFX
    if ((extrawinmenu) && (!pdamode))
	{
	    gtk_widget_show_all (menuwin);
	    gtk_widget_show_all (menuwin2);

	}
    gtk_widget_show_all (mainwindow);

    mainwindow_icon_pixbuf = create_pixbuf ("pixmaps/gpsicon.png");
    if (mainwindow_icon_pixbuf)
	{
	    gtk_window_set_icon (GTK_WINDOW (mainwindow),
				 mainwindow_icon_pixbuf);
	    gdk_pixbuf_unref (mainwindow_icon_pixbuf);
	}


    /*  now we know the drawables */
    if (pdamode)
	gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);

    drawable =
	gdk_pixmap_new (mainwindow->window, SCREEN_X, SCREEN_Y, -1);
    /* gtk_widget_set_double_buffered(drawable, FALSE);  */
    /*    drawable = drawing_area->window; */
    if (pdamode)
	gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 2);

    drawable_sats = drawing_sats->window;
    drawable_bearing = drawing_bearing->window;
    /*  gtk_widget_set_double_buffered(GTK_WIDGET(drawable_sats), TRUE);   */
    /*  gtk_widget_set_double_buffered(GTK_WIDGET(drawable_bearing), TRUE);   */

    //KCFX
    // if (!pdamode) 
    kontext = gdk_gc_new (mainwindow->window);
    //  else 
    //    kontext = gdk_gc_new (mainnotebook->window); 

    gdk_gc_set_clip_origin (kontext, 0, 0);
    rectangle.width = SCREEN_X;
    rectangle.height = SCREEN_Y;

    gdk_gc_set_clip_rectangle (kontext, &rectangle);
    cmap = gdk_colormap_get_system ();
    gdk_color_alloc (cmap, &red);
    gdk_color_alloc (cmap, &black);
    gdk_color_alloc (cmap, &blue);
    gdk_color_alloc (cmap, &nightcolor);
    gdk_color_alloc (cmap, &lcd);
    gdk_color_alloc (cmap, &lcd2);
    gdk_color_alloc (cmap, &green);
    gdk_color_alloc (cmap, &green2);
    gdk_color_alloc (cmap, &white);
    gdk_color_alloc (cmap, &mygray);
    gdk_color_alloc (cmap, &yellow);
    gdk_color_alloc (cmap, &darkgrey);
    gdk_color_alloc (cmap, &grey);
    gdk_color_alloc (cmap, &textback);
    gdk_color_alloc (cmap, &textbacknew);
    gdk_color_alloc (cmap, &orange2);
    gdk_color_alloc (cmap, &orange);
    gdk_color_alloc (cmap, &trackcolorv);

    /* fill window with color */
    gdk_gc_set_function (kontext, GDK_COPY);
    gdk_gc_set_foreground (kontext, &lcd2);
    gdk_draw_rectangle (drawing_area->window, kontext, 1, 0, 0, SCREEN_X,
			SCREEN_Y);
    {
	GtkStyle *style;
	style = gtk_rc_get_style (mainwindow);
	defaultcolor = style->bg[GTK_STATE_NORMAL];
    }

    if (pdamode)
	gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 1);
    if (SMALLMENU == 0)
	gdk_window_set_cursor (drawing_miniimage->window, cursor);

    if (pdamode)
	gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);
    cursor = gdk_cursor_new (GDK_CROSS);
    gdk_window_set_cursor (drawing_area->window, cursor);

    /*  Tooltips */
    tooltips = gtk_tooltips_new ();


    temperature_get_values ();
    battery_get_values ();

    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), sateventbox,
			  _
			  ("Click here to switch betwen satetellite level and satellite position display. A rotating globe is shown in simulation mode"),
			  NULL);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), satslabel1eventbox,
			  _
			  ("Number of used satellites/satellites in view"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), satslabel2eventbox,
			  _
			  ("EPE (Estimated Precision Error), if available"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), satslabel3eventbox,
			  _
			  ("PDOP (Position Dilution Of Precision). PDOP less than 4 gives the best accuracy, between 4 and 8 gives acceptable accuracy and greater than 8 gives unacceptable poor accuracy. "),
			  NULL);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), compasseventbox,
			  _
			  ("On top of the compass you see the direction to which you move. The pointer shows the target direction on the compass."),
			  NULL);
    wi = NULL;
    wi = gtk_item_factory_get_item (item_factory, N_("/Misc. Menu"));
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wi,
			  _
			  ("Here you find extra functions for maps, tracks and messages"),
			  NULL);

    if (havespeechout)
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), mutebt,
			      _("Disable output of speech"), NULL);
    if (usesql)
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), sqlbt,
			      _("Use SQL server for waypoints"),
			      NULL);
    /*    if (maxwp > 0) */
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wpbt,
			  _("Show waypoints on the map"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), posbt,
			  _("Turn position mode on. You can move on the map with the left mouse button click. Clicking near the border switches to the proximate map."),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), trackbt,
			  _("Show tracking on the map"), NULL);
    /*   gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), importbt, */
    /*                  _("Let you import and calibrate your own map"), NULL); */
#ifdef USETELEATLAS
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), navibt,
			  _("Navigation menu. Enter here your destination."),
			  NULL);
#endif
    if (haveNMEA)
	{
	    gtk_button_set_label (GTK_BUTTON (startgpsbt),
				  _("Stop GPSD"));
	    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), startgpsbt,
				  _("Stop GPSD and switch to simulation mode"),
				  NULL);
	    gpson = TRUE;
	}
    else
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), startgpsbt,
			      _("Starts GPSD for NMEA mode"), NULL);

    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), setupbt,
			  _("Settings for GpsDrive"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), zoomin,
			  _("Zoom into the current map"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), zoomout,
			  _("Zooms out off the current map"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scalerlbt,
			  _("Select the next more detailed map"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scalerrbt,
			  _("Select the next less detailed map"), NULL);
    /*    if (maxwp > 0) */
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips),
			  sel_target,
			  _
			  ("Select here a destination from the waypoint list"),
			  NULL);
    if (scaler)
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scaler,
			      _
			      ("Select the map scale of avail. maps."),
			      NULL);
    /*   gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), loadtrackbt, */
    /*                  _("Load and display a previous stored track file"), */
    /*                  NULL); */
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), bestmapbt,
			  _
			  ("Always select the most detailed map available"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), poi_draw_bt,
			  _("Draw Point Of Interrests found in mySQL"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), streets_draw_bt,
			  _("Draw Streets found in mySQL"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), tracks_draw_bt,
			  _("Draw Tracks found in mySQL"), NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), savetrackbt,
			  _
			  ("Save the track to given filename at program exit"),
			  NULL);


    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp1eventbox,
			  _
			  ("Number of waypoints selected from SQL server"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp2eventbox,
			  _
			  ("Number of selected waypoints, which are in range"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp3eventbox,
			  _
			  ("Range for waypoint selection in kilometers"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp4eventbox,
			  _("This shows the time from your GPS receiver"),
			  NULL);
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp5eventbox,
			  _
			  ("Number of mobile targets within timeframe/total received from friendsserver"),
			  NULL);


    /*    gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips),,_(""),NULL); */
    gtk_tooltips_set_delay (GTK_TOOLTIPS (tooltips), 1000);
    g_strlcpy (mapfilename, "***", sizeof (mapfilename));
    /*  set the timers */
    if (haveserial)
	timerto =
	    gtk_timeout_add (TIMERSERIAL,
			     (GtkFunction) get_position_data_cb,
			     NULL);
    else
	timerto =
	    gtk_timeout_add (TIMER,
			     (GtkFunction) get_position_data_cb,
			     NULL);
    gtk_timeout_add (WATCHWPTIMER, (GtkFunction) watchwp_cb, NULL);


    redrawtimeout =
	gtk_timeout_add (100, (GtkFunction) calldrawmarker_cb, NULL);

    /*  if we started in simulator mode we have a little move roboter */
    if (simmode)
	simpos_timeout =
	    gtk_timeout_add (300, (GtkFunction) simulated_pos, 0);
    if (nmeaout)
	gtk_timeout_add (1000, (GtkFunction) write_nmea_cb, NULL);
    gtk_timeout_add (10000, (GtkFunction) testconfig_cb, 0);
    gtk_timeout_add (600000, (GtkFunction) speech_saytime_cb, 0);
    gtk_timeout_add (1000, (GtkFunction) storetrack_cb, 0);
    gtk_timeout_add (10000, (GtkFunction) masteragent_cb, 0);
    gtk_timeout_add (15000, (GtkFunction) getsqldata, 0);
    gtk_timeout_add (2000, (GtkFunction) nav_doit, NULL);
    if ( battery_get_values () )
	gtk_timeout_add (5000, (GtkFunction) expose_display_battery,
			 NULL);

    if ( temperature_get_values () )
	gtk_timeout_add (5000, (GtkFunction) expose_display_temperature,
			 NULL);

    gtk_timeout_add (15000, (GtkFunction) friendsagent_cb, 0);

    if (havespeechout)
	{
	    speech_saytime_cb (NULL, 1);
	    gtk_timeout_add (SPEECHOUTINTERVAL,
			     (GtkFunction) speech_out_cb, 0);
	}

    /*  To set the checkboxes to the right values */
    bestmap_cb (NULL, 0);
    drawgrid_cb (NULL, 0);
    poi_draw_cb (poi_draw_bt, 0);
    streets_draw_cb (streets_draw_bt, 0);
    tracks_draw_cb (NULL, 0);
    needtosave = FALSE;

    poi_init ();
    streets_init ();
    tracks_init ();

    update_posbt();

    /*
     * setup TERM signal handler so that we can save evrything nicely when the
     * machine is shutdown.
     */
    void termhandler (int sig)
	{
	    gtk_main_quit ();
	}
    signal (SIGTERM, termhandler);


    /*  Mainloop */
    gtk_main ();


    g_timer_destroy (timer);
    writeconfig ();
    gdk_pixbuf_unref (friendspixbuf);


    gpsserialquit ();
    unlink ("/tmp/cammain.pid");
    unlink ("/tmp/gpsdrivetext.out");
    unlink ("/tmp/gpsdrivepos");
    if (savetrack)
	savetrackfile (2);
    sqlend ();
    free (friends);
    free (fserver);
    free_route_list ();
    if (kismetsock != -1)
	close (kismetsock);
    gpsd_close();
    if (sockfd != -1)
	close (sockfd);
    speech_out_close ();
    cleanup_nasa_mapfile ();
    fprintf (stderr, _("\n\nThank you for using GpsDrive!\n\n"));
    return 0;
}
