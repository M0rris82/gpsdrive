/***********************************************************************

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
     Guenther Meyer <d.s.e@sordidmusic.com>
     J.D. Schmidt <jdsmobile@gmail.com>
     Jan-Benedict Glaw <jbglaw@lug-owl.de>
     Joerg Ostertag <gpsdrive@ostertag.name>
     John Hay <jhay@icomtek.csir.co.za>
     Johnny Cache <johnycsh@hick.org>
     Miguel Angelo Rozsas <miguel@rozsas.xx.nom.br>
     Mike Auty
     Oddgeir Kvien <oddgeir@oddgeirkvien.com>
     Oliver Kuehlert <Oliver.Kuehlert@mpi-hd.mpg.de>
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
/* #include <gpskismet.h> <-- prototypes are declared also in gpsproto.h */

#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

		
#include <locale.h>

#include "gettext.h"

#include <dirent.h>

#include <getopt.h>

#include "LatLong-UTMconversion.h"
#include "gpsdrive.h"
#include "battery.h"
#include "poi.h"
#include "streets.h"
#include "wlan.h"
#include "track.h"
#include "waypoint.h"
#include "routes.h"
#include "gps_handler.h"
#include "nmea_handler.h"
#include <speech_strings.h>
#include <speech_out.h>

#include "import_map.h"
#include "download_map.h"
#include "icons.h"
#include "gui.h"
#include "poi_gui.h"

#include "mapnik.h"

#ifndef NOPLUGINS
#include "gmodule.h"
#endif

/*  Defines for gettext I18n */
#include <libintl.h>
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
extern gdouble wp_saved_target_lat;
extern gdouble wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat;
extern gdouble wp_saved_posmode_lon;
coordinate_struct coords;
currentstatus_struct current;

gdouble dist;
gdouble long_diff = 0, lat_diff = 0;
GdkGC *kontext;

GtkWidget *drawing_area, *drawing_bearing;
GtkWidget *drawing_sats, *drawing_miniimage;

GtkWidget *distlabel, *speedlabel, *altilabel, *miles;
//GtkWidget *startgps_bt;
GdkDrawable *drawable, *drawable_bearing, *drawable_sats;
gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle = 0;
gdouble pixelfact, posx, posy;
GdkPixbuf *image = NULL, *tempimage = NULL, *miniimage = NULL;

extern GdkPixbuf *friendsimage, *friendspixbuf;

extern mapsstruct *maps;

extern GtkWidget *drawing_battery, *drawing_temp;


/* action=1: radar (speedtrap) */
wpstruct *wayp;

friendsstruct *friends, *fserver;

/* socket for friends  */
extern int sockfd;


gchar oldfilename[2048];
GString *tempmapfile;
gdouble earthr;
/*Earth radius calibrated with GARMIN III */
/*  gdouble R = 6383254.445; */


#define R earthr


gint maploaded = FALSE;
gint zoom, iszoomed;
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

gdouble milesconv;
gdouble gbreit, glang, olddist = 99999.0;
GTimer *timer, *disttimer;
gint gcount;
gchar localedecimal;
gchar language[] = "en";

glong mapscale = 1000;
gint scaleprefered_not_bestmap = 1;
gint scalewanted = 100000;

gint importactive;
extern gint downloadwindowactive;
extern gint downloadactive;

GtkWidget *add_wp_name_text, *wptext2;
gdouble wplat, wplon;
gchar oldangle[100];
GdkCursor *cursor_cross;

// Uncomment this (or add a make flag?) to only have scales for expedia maps
//#define EXPEDIA_SCALES_ONLY

#ifdef EXPEDIA_SCALES_ONLY
gint slistsize = 12;
gchar *slist[] = { "5000", "15000", "20000", "50000", 
		   "100000", "200000", "750000", "3000000", "7500000", "75000000",
		   "88067900","90000000"
};
gint nlist[] = { 5000, 15000, 20000, 50000, 
		 100000, 200000, 750000, 3000000, 7500000, 75000000,
		 88067900,90000000,
};
#else
gint slistsize = 33;
gchar *slist[] = { "500", 
		   "1000", "1500", "2000", "3000", "5000", "7500", 
		   "10000", "15000", "20000", "30000", "50000", "75000",
		   "100000", "150000", "200000", "300000", "500000", "750000", 
		   "1000000", "1500000", "2000000", "3000000", "5000000", "7500000", 
		   "10000000", "15000000", "20000000", "30000000", "50000000", "75000000",
		   "88067900","90000000"
};
gint nlist[] = { 500,
		 1000, 1500, 2000, 3000, 5000, 7500,
		 10000, 15000, 20000, 30000, 50000, 75000,
		 100000, 150000, 200000, 300000, 500000, 750000,
		 1000000, 1500000, 2000000, 3000000, 5000000, 7500000,
		 10000000, 15000000, 20000000, 30000000, 50000000, 75000000,
		 88067900,90000000
};
#endif

GtkWidget *label_lat, *label_lon;
GtkWidget *eventbox_lat, *eventbox_lon;
GtkWidget *label_map_filename, *label_map_scale;
GtkWidget *label_heading, *label_baering, *label_timedest;
GtkWidget *label_prefscale, *mute_bt;
GtkWidget *wp_bt;
GtkWidget *bestmap_bt, *poi_draw_bt, *streets_draw_bt, *wlan_draw_bt;

GtkWidget *track_bt;
GtkWidget *savetrack_bt;
GtkWidget *loadtrack_bt;
gint savetrack = 0;
gint trackflag = 1;
GdkSegment *track, *trackshadow;
glong tracknr;
trackcoordstruct *trackcoord;
extern glong trackcoordnr, tracklimit, trackcoordlimit,old_trackcoordnr;
gchar savetrackfn[256];

GtkWidget *scaler_left_bt, *scaler_right_bt, *scaler_widget;
GtkObject *scaler_adj;

GtkWidget *setup_bt;
gint havespeechout, hours, minutes, speechcount = 0;
gint muteflag = 0;
gchar lastradar[40], lastradar2[40]; 
gint foundradar;
gdouble radarbearing;
gint errortextmode = TRUE;
gchar serialdev[80];
gint extrawinmenu = FALSE;
gint haveproxy, proxyport;
gchar proxy[256], hostname[256];

/*** Mod by Arms */
gint real_screen_x, real_screen_y, real_psize, real_smallmenu, int_padding;
gint SCREEN_X_2, SCREEN_Y_2;
gint showallmaps = TRUE;
/* guint selwptimeout; */
extern gint setwpactive;
gint selected_wp_mode = FALSE;
gint onemousebutton = FALSE;


GtkWidget *askwindow;
extern time_t maptxtstamp;
gint simpos_timeout = 0;
gint setdefaultpos = TRUE;
extern gint markwaypoint;
GtkWidget *addwaypointwindow;
gint oldbat = 125, oldloading = FALSE;
gint bat, loading;
gint disableapm = FALSE;
typedef struct
{
	gchar n[200];
}
namesstruct;
namesstruct *names;
gchar gpsdservername[200], setpositionname[80];
gint newsatslevel = TRUE, testgarmin = TRUE;
gint needtosave = FALSE;
GtkWidget *serial_bt, *mapdirbt;
GtkWidget *slowcpubt;
GtkWidget *add_wp_lon_text, *add_wp_lat_text;

gdouble altitude = 0.0, precision = (-1.0), gsaprecision = (-1.0);
gint oldsatfix = 0, oldsatsanz = -1, havealtitude = FALSE;
gint satfix = 0, usedgps = FALSE;
gchar dgpsserver[80], dgpsport[10];
GtkWidget *posbt;
GtkWidget *mapnik_bt;
GtkWidget *cover;
extern gint needreloadmapconfig;
GdkPixbuf *batimage = NULL;
GdkPixbuf *temimage = NULL;
GdkPixbuf *satsimage = NULL;
gint numsats = 0, satsavail = 0;
gint numgrids = 4, scroll = TRUE;
gint satposmode = FALSE;
gint printoutsats = FALSE;
extern gchar *displaytext;
extern gint do_display_dsc, textcount;
gint isnight = FALSE, disableisnight;
gint nighttimer;
GtkWidget *setupentry[50], *setupentrylabel[50];
void (*setupfunction[50]) ();
gchar utctime[20], loctime[20];
gint mod_setupcounter = 4;
gint redrawtimeout;
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
gint friends_poi_id[TRAVEL_N_MODES];

/* Give more debug informations */
gint mydebug = 0;

char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
char dbtable[MAXDBNAME], dbname[MAXDBNAME],wlantable[MAXDBNAME];
char poitypetable[MAXDBNAME];
char wp_typelist[MAXPOITYPES][50];
char dbpoifilter[5000];
double dbdistance;
gint usesql = FALSE, dbusedist = FALSE;
extern gint sqlselects[MAXPOITYPES], sqlplace, friendsplace, kismetsock, havekismet;

extern GdkPixbuf *kismetpixbuf,	*iconpixbuf[50];
gint earthmate = FALSE;

extern gint wptotal, wpselected;

extern routestatus_struct route;
extern color_struct colors;

GdkFont *font_text, *font_verysmalltext, *font_smalltext, *font_bigtext, *font_wplabel;
PangoFontDescription *pfd_text, *pfd_verysmalltext, *pfd_smalltext, *pfd_bigtext, *pfd_wplabel;
gchar font_s_text[100], font_s_verysmalltext[100], font_s_smalltext[100];

static gchar gradsym[] = "\xc2\xb0";
gdouble normalnull = 0;
extern gint poi_draw;
extern gint streets_draw;
extern gint wlan_draw;
gint drawmarkercounter = 0, loadpercent = 10, globruntime = 30;
extern int pleasepollme;


gint forcehavepos = FALSE;
extern gchar cputempstring[20], batstring[20];
extern GtkWidget *tempeventbox, *batteventbox;
GtkWidget *sateventbox = NULL, *compasseventbox = NULL;
GtkWidget *wplabel1, *wplabel2, *wplabel3, *wplabel4, *wplabel5;
GtkWidget *wp1eventbox, *wp2eventbox, *wp3eventbox, *wp4eventbox;
GtkWidget *wp5eventbox, *satsvbox, *satshbox, *satslabel1eventbox;
GtkWidget *satslabel2eventbox, *satslabel3eventbox;
GtkWidget *satslabel1, *satslabel2, *satslabel3;
GtkWidget *frame_maptype;
GtkWidget *frame_mapcontrol;
GtkWidget *frame_status;
GtkWidget *frame_speed;
GtkWidget *frame_sats;
GtkWidget *frame_lat, *frame_lon, *frame_mapfile, *frame_mapscale;
GtkWidget *frame_heading, *frame_bearing, *frame_timedest, *frame_prefscale;
GtkWidget *frame_map_area, *frame_bearing, *frame_target, *frame_altitude;
GtkWidget *frame_wp;
GtkWidget *frame_poi,*frame_track, *lab1;
GtkWidget *menubar;
gchar messagename[40], messagesendtext[1024], messageack[100];
GtkItemFactory *item_factory;
gint statuslock = 0, gpson = FALSE;
int messagenumber = 0, didrootcheck = 0, haveserial = 0;
int gotneverserial = TRUE, timerto = 0, serialspeed = 1;
int disableserial = 1, disableserialcl = 0;
GtkTextBuffer *getmessagebuffer;

extern int newdata;
extern pthread_mutex_t mutex;
//int mapistopo = FALSE;
extern int havenasa;
int nosplash = FALSE;
int havedefaultmap = TRUE;

int storetz = FALSE;
int egnoson = 0, egnosoff = 0;
extern char actualstreetname[200];

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
    {N_("/_Menu"),                    NULL, NULL,                     0, "<Branch>"},
    {N_("/_Menu/_Maps"),               NULL, NULL,                     0, "<Branch>"},
    {N_("/_Menu/_Maps/_Import"),   NULL, (gpointer) import1_cb,    1, NULL},
    {N_("/_Menu/_Maps/_Download"), NULL, (gpointer) download_cb,   0, NULL},
    {N_("/_Menu/_Reinitialize GPS"),  NULL, (gpointer) reinitgps_cb,  0, NULL},
    //    {N_("/_Menu/_Start gpsd"),        NULL, (gpointer) startgpsd_cb,  0, NULL},
    {N_("/_Menu/_Load track file"),   NULL, (gpointer) loadtrack_cb,  0, "<StockItem>", GTK_STOCK_OPEN},
    {N_("/_Menu/M_essages"),           NULL, NULL,                     0, "<Branch>"},
    {N_("/_Menu/M_essages/_Send message to mobile target"), 
                                            NULL, (gpointer) sel_message_cb,0,     NULL},
    {N_("/_Menu/S_ettings"), 
                                            NULL, (gpointer) settings_main_cb,0,     NULL},
    {N_("/_Menu/_Help"),               NULL, NULL,                     0, "<LastBranch>"},
    {N_("/_Menu/_Help/_About"),         NULL, (gpointer) about_cb,      0, "<StockItem>", GTK_STOCK_ABOUT},
    {N_("/_Menu/_Help/_Topics"),        NULL, (gpointer) help_cb,       0, "<StockItem>", GTK_STOCK_HELP},
    {N_("/_Menu/_Quit"),               NULL, (gpointer) quit_program,  0, NULL }
};

void sql_load_lib();
void unit_test(void);
void drawdownloadrectangle (gint big);
GtkWidget * make_display_map_checkboxes();
GtkWidget * make_display_map_controls();
void draw_grid (GtkWidget * widget);
gint bestmap_cb (GtkWidget * widget, guint datum);



gchar geometry[80];
gint usegeometry = FALSE;

/* 
 * ****************************************************************************
 * ****************************************************************************
 * ****************************************************************************
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

/* ******************************************************************
 * Check dirs and files
 */
void check_and_create_files(){
    gchar file_path[2048];
    struct stat buf;

    if ( mydebug >5 ) fprintf(stderr , " check_and_create_files()\n");

    // Create .gpsdrive dir if not exist
    g_snprintf (file_path, sizeof (file_path),
		"%s",
		local_config.dir_home);
    if(stat(file_path,&buf))
	{
	    if ( mkdir (file_path, 0700) )
		{
		    printf("Error creating %s\n",file_path);
		} else {
		    printf("created %s\n",file_path);
		}
	}

    // Create maps/ Directory if not exist
    g_strlcpy (file_path, local_config.dir_maps, sizeof (file_path)); 
    if(stat(file_path,&buf))
	{
	    if ( mkdir (file_path, 0700) )
		{
		    printf("Error creating %s\n",file_path);
		} else {
		    printf("created %s\n",file_path);
		}
	}
  
    // map_koord.txt
    // Copy from system if not exist
    g_snprintf (file_path, sizeof (file_path),
		"%s/map_koord.txt",
		local_config.dir_maps);
    if ( stat (file_path, &buf) ) {
	gchar copy_command[2048];
	g_snprintf (copy_command, sizeof (copy_command),
		    "cp %s/gpsdrive/map_koord.txt %s",
		    (gchar *) DATADIR,
		    file_path);
	if ( system (copy_command))
	    {
		fprintf(stderr,"Error Creating %s\nwith command: '%s'\n",
			file_path,copy_command);
		exit(-1);
	    }
	else 
	    fprintf(stderr,"Created map_koord.txt %s\n",file_path);
    };


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

void
display_status (char *message)
{
    gchar tok[200];
    
    if ( mydebug >20 ) 
	fprintf(stderr , "display_status(%s)\n",message);
    
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
 */
gint
lightoff (GtkWidget * widget, guint * datum)
{
	disableisnight = FALSE;
	/* gtk_widget_modify_bg (mainwindow, GTK_STATE_NORMAL, &colors.nightmode); */
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
	trip_lat = coords.current_lat;
	trip_lon = coords.current_lon;
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
	if (gui_status.posmode)
	    {		// Print out actual position of Mouse
		gdouble lat, lon;
		GdkModifierType state;
		gint x, y;
		
		gdk_window_get_pointer (drawing_area->window, &x, &y,&state);
		calcxytopos (x, y, &lat, &lon, zoom);
		if ( mydebug > 20 )
		    printf ("Actual mouse position: lat:%f,lon:%f (x:%d,y:%d)\n", lat, lon, x, y);
		// display position of Mouse in lat/lon Fields
		coordinate2gchar(s2, sizeof(s2), lat, TRUE,
			local_config.coordmode);
		gtk_label_set_text (GTK_LABEL (label_lat), s2);
		coordinate2gchar(s2, sizeof(s2), lon, FALSE,
			local_config.coordmode);
		gtk_label_set_text (GTK_LABEL (label_lon), s2);
	    }
	else 
	    {
		coordinate2gchar(s2, sizeof(s2), coords.current_lat, TRUE,
			local_config.coordmode);
		gtk_label_set_text (GTK_LABEL (label_lat), s2);
		coordinate2gchar(s2, sizeof(s2), coords.current_lon, FALSE,
			local_config.coordmode);
		gtk_label_set_text (GTK_LABEL (label_lon), s2);
	    }

	if ( mydebug > 10 )
	    {
		if (havepos)
		    g_print ("***Position: %f %f***\n", coords.current_lat,
			     coords.current_lon);
		else
		    g_print ("***no valid Position:\n");
	    }


	strncpy (mfmap_filename, g_basename (mapfilename), 59);
	mfmap_filename[59] = 0;
	if ( mydebug >10 )
	    gtk_label_set_text (GTK_LABEL (label_map_filename), mfmap_filename);

	g_snprintf (s2, sizeof (s2), "1:%ld", mapscale);
	gtk_label_set_text (GTK_LABEL (label_map_scale), s2);

	g_snprintf (s2, sizeof (s2), "%3.0f%s", RAD2DEG (current.heading),
		    gradsym);
	gtk_label_set_text (GTK_LABEL (label_heading), s2);

	g_snprintf (s2, sizeof (s2), "%3.0f%s", RAD2DEG (current.bearing),
		    gradsym);
	gtk_label_set_text (GTK_LABEL (label_baering), s2);

	if (h >= 99 && m>=99)
		g_snprintf (s2, sizeof (s2), "%s", _("unknown"));
	else
		g_snprintf (s2, sizeof (s2), "%2d:%02dh", h, m);
	gtk_label_set_text (GTK_LABEL (label_timedest), s2);

	if (scaleprefered_not_bestmap)
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
		if (	(local_config.nightmode == NIGHT_ON) ||
			(local_config.nightmode == NIGHT_AUTO && isnight)
		   )
		{
			switch_nightmode (TRUE);
		}
	}

	if (	local_config.nightmode == NIGHT_OFF
		|| disableisnight || downloadwindowactive
	   )
	{
		switch_nightmode (FALSE);
	}


	gettimeofday (&tv1, &tz);
	runtime2 = tv1.tv_usec - tv2.tv_usec;
	if (tv1.tv_sec != tv2.tv_sec)
		runtime2 += 1000000l * (tv1.tv_sec - tv2.tv_sec);

	/* we test if we have to load a new map because we are outside 
	 * the currently loaded map 
	 */
	test_and_load_newmap ();

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

	if (local_config.maxcpuload < 1)
		local_config.maxcpuload = 1;
	if (local_config.maxcpuload > 95)
		local_config.maxcpuload = 95;
	if (!haveNMEA)
		expose_sats_cb (NULL, 0);
	if (pleasepollme)
	{
		pleasepollme++;
		if (pleasepollme > 10)
		{
			pleasepollme = 1;
			friends_sendmsg (local_config.friends_serverip, NULL);
		}
	}
	period = 10 * globruntime / (10 * local_config.maxcpuload);
	drawmarkercounter++;
	/*   g_print("period: %d, drawmarkercounter %d\n", period, drawmarkercounter);  */
	

	if (local_config.MapnikStatusInt > 0 && drawmarkercounter >= 3) {
		drawmarkercounter = 0;
		drawmarker_cb (NULL, NULL);		
		return TRUE;
	}
	if (((drawmarkercounter > period) || (drawmarkercounter > 50))
	    && (drawmarkercounter >= 3))
	{
		drawmarkercounter = 0;
		drawmarker_cb (NULL, NULL);
	}
	return TRUE;
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

	testifnight ();


	if (!didrootcheck)
		if (getuid () == 0)
		{
			g_snprintf (buffer, sizeof (buffer),
				    "<span  weight=\"bold\">%s</span>\n%s",
				    _("Warning!"),
				    _
				    ("You should not start GpsDrive as user root!!!"));
			popup_warning (NULL, buffer);
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

    if (gui_status.posmode) 
	return TRUE;
	
#ifdef DBUS_ENABLE
	/* If we use DBUS track points are usually stored by the DBUS signal handler */
	/* Only store them by timer if we are in position mode */
	if ( useDBUS && !local_config.simmode )
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
	if ((!local_config.simmode && !havepos) || gui_status.posmode /*  ||((!local_config.simmode &&haveposcount<3)) */ )	/* we have no valid position */
	{
		(trackcoord + trackcoordnr)->lon = 1001.0;
		(trackcoord + trackcoordnr)->lat = 1001.0;
		(trackcoord + trackcoordnr)->alt = 1001.0;
	}
	else
	{
		(trackcoord + trackcoordnr)->lon = coords.current_lon;
		(trackcoord + trackcoordnr)->lat = coords.current_lat;
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
		gdk_gc_set_foreground (kontext, &colors.lcd);

		gdk_draw_rectangle (drawable_sats, kontext, 1, 3, 0,
				    PSIZE + 2, PSIZE + 5);
		gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
		gdk_gc_set_foreground (kontext, &colors.black);
		gdk_draw_rectangle (drawable_sats, kontext, 0, 3, 0,
				    PSIZE + 2, PSIZE + 5);

		gdk_gc_set_foreground (kontext, &colors.lcd);

		if (!satposmode)
		{
			if (havepos)
				gdk_gc_set_foreground (kontext, &colors.green);
			else
				gdk_gc_set_foreground (kontext, &colors.red);
			gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
			for (i = 3; i < (PSIZE + 5); i += 3)
				gdk_draw_line (drawable_sats, kontext, 4, i,
					       PSIZE + 4, i);


			gdk_gc_set_foreground (kontext, &colors.lcd2);
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
			gdk_gc_set_foreground (kontext, &colors.black);
		else
		{
			if (satfix == 0)	/* no Fix */
				gdk_gc_set_foreground (kontext, &colors.textback);
			else	/* Differntial Fix */
				gdk_gc_set_foreground (kontext, &colors.blue);
		}
		j = k = 0;

		if (satposmode)
		{
			gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
			gdk_gc_set_foreground (kontext, &colors.lcd2);
			gdk_draw_arc (drawable_sats, kontext, 0, 4,
				      4, PSIZE, PSIZE, 105 * 64, 330 * 64);
			gdk_draw_arc (drawable_sats, kontext, 0,
				      5 + PSIZE / 4, 4 + PSIZE / 4, PSIZE / 2,
				      PSIZE / 2, 0, 360 * 64);
			gdk_gc_set_foreground (kontext, &colors.darkgrey);
			{
				/* prints in pango */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;
				gint width;

				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_sats, "N");
				//KCFX  
				if (local_config.guimode == GUI_PDA)
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
							     &colors.grey, NULL);
				pango_layout_get_pixel_size (wplabellayout,
							     &width, NULL);
				/*          printf ("w: %d\n", width);  */
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}

			/*    gdk_draw_text (drawable_sats, font_verysmalltext, kontext,
			 *                   2 + (PSIZE) / 2, 9, "N", 1);
			 */
			gdk_gc_set_foreground (kontext, &colors.lcd2);

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
					az = DEG2RAD(satlistdisp[i][3]);

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
							(kontext, &colors.textback);
						break;
					case 2:
					case 3:
						gdk_gc_set_foreground
							(kontext, &colors.red);
						break;
					case 4:
					case 5:
					case 6:
						gdk_gc_set_foreground
							(kontext, &colors.yellow);
						break;
					case 7:
						gdk_gc_set_foreground
							(kontext, &colors.green2);
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
			if (local_config.distmode == DIST_MILES || local_config.distmode == DIST_NAUTIC)
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
      if( !muteflag && local_config.sound_gps )
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
				    "data/pixmaps/gpsdriveanim.gif");
			anim = gdk_pixbuf_animation_new_from_file (mappath,
								   NULL);
			if ( anim == NULL ) {
			    g_snprintf (mappath, sizeof (mappath),
					"%s/gpsdrive/%s", DATADIR,
					"pixmaps/gpsdriveanim.gif");
			    anim = gdk_pixbuf_animation_new_from_file (mappath,
								       NULL);
			}
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


/*
 * Draw the scale bar ( |-------------| ) into the map. Also add a textual
 * description of the currently used scale.
 */
void
draw_zoom_scale (void)
{
	gdouble factor[] = { 1.0, 2.5, 5.0, };
	gint exponent = 0;
	gdouble remains = 0.0;
	gint used_factor = 0;
	gint i;
	gint min_bar_length = SCREEN_X / 8; /* 1/8 of the display width */
	const gint dist_x = 20; /* distance to the right */
	const gint dist_y = 20; /* distance to bottom */
	const gint frame_width = 5; /* grey pixles around the scale bar */
	gint bar_length;
	gchar zoom_scale_txt[100];
	gchar *format;
	gchar *symbol;
	gdouble approx_displayed_value;
	gdouble conversion;
	gint l;
	// gint text_length_pixel;

	/*
	 * We want a bar with at least (min_bar_length) pixles in
	 * length.  Calculate the displayed value of this bar is whatever
	 * metric is requested. 
	 *
	 * The bar length' value l (in m), divided by "conversion", will
	 * result in what to display to the user, in terms of "symbol".
	 */
	approx_displayed_value /* m */ = min_bar_length * mapscale
					 / PIXELFACT / zoom;

	if (local_config.distmode == DIST_NAUTIC) {
		conversion /* m/nmi */ = 1000.0 /* m/km */ / KM2NAUTIC /* nmi/km */;
		symbol = "nmi";
		format = "%.2lf %s";
	} else if (local_config.distmode == DIST_METRIC) {
		conversion /* m/m */ = 1.0 /* m/m */;
		symbol = "m";
		format = "%.0lf %s";
		if (approx_displayed_value / conversion > 1000) {
			conversion /* m/km */ = 1000.0;
			symbol = "km";
			format = "%.0lf %s";
		}
	} else if (local_config.distmode == DIST_MILES) {
		conversion /* m/yd */ = 1000.0 /* m/km */
					/ 1760.0 /* yd/mi */
					/ KM2MILES /* mi/km */;
		symbol = "yd";
		format = "%.0lf %s";
		if (approx_displayed_value / conversion > 1760.0) {
			conversion /* m/mi */ = 1000.0 /* m/km */
						/ KM2MILES /* mi/km */;
			symbol = "mi";
			format = "%.0lf %s";
		}
	} else
		return;

	/*
	 * Now find a well-formed value that is about the expected size
	 * of the scale bar, or a bit longer.
	 */
	for (i = 0; i < ARRAY_SIZE (factor); i++) {
		gdouble rest;
		gdouble log_value;

		log_value = log10 (min_bar_length * mapscale / PIXELFACT
				   / conversion / zoom / factor[i]);

		if ((rest = log_value - floor (log_value)) >= remains) {
			remains = rest;
			used_factor = i;
			exponent = (gint) floor (log_value) + 1;
		}
	}
	bar_length = factor[used_factor] * pow (10.0, exponent)
		     * conversion / (mapscale / PIXELFACT) * zoom;

	g_snprintf (zoom_scale_txt, sizeof (zoom_scale_txt), format, factor[used_factor]
					       * pow (10.0, exponent),
					       symbol);

	/* Now bar_length is the length of the scaler-bar in pixel
	 * and zoom_scale_txt is the text for the scaler Bar. For example "10 Km"
	 */

	l = (SCREEN_X - 40) - (strlen (zoom_scale_txt) * 15);

	/* Draw greyish rectangle as background for the scale bar */
	gdk_gc_set_function (kontext, GDK_OR);
	gdk_gc_set_foreground (kontext, &colors.mygray);
	//gdk_gc_set_foreground (kontext, &textback);
	gdk_draw_rectangle (drawable, kontext, 1,
			    SCREEN_X - dist_x - bar_length - frame_width,
			    SCREEN_Y - dist_y - 2 * frame_width,
			    bar_length + 2 * frame_width, 
			    dist_y + 1 * frame_width);
	gdk_gc_set_function (kontext, GDK_COPY);
	gdk_gc_set_foreground (kontext, &colors.black);

	/* Print the meaning of the scale bar ("10 km") */
	{
	    /* prints in pango */
	    PangoLayout *wplabellayout;
	    
	    wplabellayout = gtk_widget_create_pango_layout (drawing_area, zoom_scale_txt);
	    pango_layout_set_font_description (wplabellayout, pfd_text);

	    gdk_draw_layout_with_colors (drawable, kontext, 
					 l, SCREEN_Y - dist_y -1 ,
					 wplabellayout, &colors.black, NULL);
	    if (wplabellayout != NULL)
		g_object_unref (G_OBJECT (wplabellayout));
	}

	/* Print the actual scale bar lines */
	gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	/* horizontal */
	gdk_draw_line (drawable, kontext,
		       (SCREEN_X - dist_x) - bar_length, SCREEN_Y - dist_y,
		       (SCREEN_X - dist_x), SCREEN_Y - dist_y );
	/* left */
	gdk_draw_line (drawable, kontext,
		       (SCREEN_X - dist_x) - bar_length, SCREEN_Y - dist_y - frame_width,
		       (SCREEN_X - dist_x) - bar_length, SCREEN_Y - dist_y + 10- frame_width);
	/* right */
	gdk_draw_line (drawable, kontext,
		       (SCREEN_X - dist_x), SCREEN_Y - dist_y - frame_width,
		       (SCREEN_X - dist_x), SCREEN_Y - dist_y + 10 - frame_width);

	/* Scale Bar Text */
	/* draw zoom factor */
	g_snprintf (zoom_scale_txt, sizeof (zoom_scale_txt), "%dx", zoom);

	l = (SCREEN_X - 15) - 14 - strlen (zoom_scale_txt) * 2;

	gdk_gc_set_function (kontext, GDK_OR);

	gdk_gc_set_foreground (kontext, &colors.mygray);
	gdk_draw_rectangle (drawable, kontext, 1, (SCREEN_X - 30), 0, 30, 30);
	gdk_gc_set_function (kontext, GDK_COPY);

	gdk_gc_set_foreground (kontext, &colors.blue);

	{
	    /* prints in pango */
	    PangoFontDescription *pfd;
	    PangoLayout *wplabellayout;
	    
	    wplabellayout = gtk_widget_create_pango_layout (drawing_area, zoom_scale_txt);
	    pfd = pango_font_description_from_string ("Sans 9");
	    pango_layout_set_font_description (wplabellayout, pfd);
	    
	    gdk_draw_layout_with_colors (drawable, kontext,
					 l, 2, wplabellayout, &colors.blue,
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
	/*    g_print("simmode: %d, nmea %d garmin %d\n",local_config.simmode,haveNMEA,haveGARMIN); */

	if (importactive)
	    return TRUE;

	if (local_config.showgrid)
		draw_grid (widget);


	if (usesql)
	{
	    streets_draw_list ();
	    poi_draw_list ();
	    wlan_draw_list ();
	}


	if (local_config.showwaypoints)
		draw_waypoints ();

	drawtracks ();
	
	if (route.show)
		draw_route ();

	if (local_config.showfriends && !usesql)
		drawfriends ();

	if (havekismet)
		readkismet ();

	if (zoomscale)
		draw_zoom_scale ();


	if (havekismet && (kismetsock>=0))
	    gdk_draw_pixbuf (drawable, kontext, kismetpixbuf, 0, 0,
			     10, SCREEN_Y - 42,
			     36, 20, GDK_RGB_DITHER_NONE, 0, 0);
	
	if (savetrack)
	{
		k = 100;
		gdk_gc_set_foreground (kontext, &colors.white);
		gdk_draw_rectangle (drawable, kontext, 1, 10,
				    SCREEN_Y - 21, k + 3, 14);
		gdk_gc_set_foreground (kontext, &colors.red);
		{
			/* prints in pango */
			PangoFontDescription *pfd;
			PangoLayout *wplabellayout;

			wplabellayout =
				gtk_widget_create_pango_layout (drawing_area,
								savetrackfn);
			//KCFX  
			if (local_config.guimode == GUI_PDA)
				pfd = pango_font_description_from_string
					("Sans 7");
			else
				pfd = pango_font_description_from_string
					("Sans 10");
			pango_layout_set_font_description (wplabellayout,
							   pfd);

			gdk_draw_layout_with_colors (drawable, kontext,
						     14, SCREEN_Y - 22,
						     wplabellayout, &colors.red,
						     NULL);
			if (wplabellayout != NULL)
				g_object_unref (G_OBJECT (wplabellayout));
			/* freeing PangoFontDescription, cause it has been copied by prev. call */
			pango_font_description_free (pfd);

		}


		/*    gdk_draw_text (drawable, smallfont_s_text, kontext,
		 *                   11, SCREEN_Y - 10, savetrackfn,
		 *                   strlen (savetrackfn));
		 */

		/*      gdk_draw_text (drawable, font_s_text, kontext, 10, */
		/*                     SCREEN_Y - 10, savetrackfn, strlen (savetrackfn)); */
	}

	if (gui_status.posmode)
	{
	    blink = TRUE;
	}

#define PFSIZE 55
#define PFSIZE2 45


	if (havepos || blink)
	{
		if (gui_status.posmode)
		{
			gdk_gc_set_foreground (kontext, &colors.blue);
			gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
			gdk_draw_rectangle (drawable, kontext, 0, posx - 10,
					    posy - 10, 20, 20);
		}
		else
		{
			if (local_config.showshadow)
			{
				/*  draw shadow of  position marker */
				gdk_gc_set_foreground (kontext, &colors.darkgrey);
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
				w = current.bearing + M_PI;
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

				w = current.heading + M_PI;
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

			gdk_gc_set_foreground (kontext, &colors.black);
			gdk_gc_set_line_attributes (kontext, 3, 0, 0, 0);
			gdk_draw_arc (drawable, kontext, 0, posx - 7,
				      posy - 7, 14, 14, 0, 360 * 64);
			/*  draw pointer to destination */

			w = current.bearing + M_PI;

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
			gdk_gc_set_foreground (kontext, &colors.red);
			gdk_draw_arc (drawable, kontext, 0, posx + 2 - 7,
				      posy + 2 - 7, 10, 10, 0, 360 * 64);
			w = current.heading + M_PI;
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

			gdk_gc_set_foreground (kontext, &colors.green);
			gdk_gc_set_line_attributes (kontext, 5, 0, 0, 0);
			gdk_draw_arc (drawable, kontext, 0, posxmarker - 10,
				      posymarker - 10, 20, 20, 0, 360 * 64);
		}
		/*  If we are in position mode we set direction to zero to see where is the  */
		/*  target  */
		if (gui_status.posmode)
			current.heading = 0.0;

		display_status2 ();

	}



	/*  now draw marker for destination point */

	calcxy (&posxdest, &posydest, coords.target_lon,
	coords.target_lat, zoom);

	gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
	if (local_config.showshadow)
	{
		/*  draw + sign at destination */
		gdk_gc_set_foreground (kontext, &colors.darkgrey);
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
		gdk_gc_set_foreground (kontext, &colors.blue);
	else
		gdk_gc_set_foreground (kontext, &colors.red);
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
	if (local_config.distmode == DIST_MILES)
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
	if (local_config.distmode == DIST_METRIC)
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
	if (local_config.distmode == DIST_NAUTIC)
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
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"%s\" font_desc=\"%s\">%s<span size=\"16000\">%s</span></span>",
		    local_config.color_bigdisplay,
		    local_config.font_bigdisplay, s2, s2a);
	gtk_label_set_markup (GTK_LABEL (distlabel), s3);
	/* gtk_label_set_text (GTK_LABEL (distlabel), s2);  */
	if ( mydebug > 5 ) 
	    fprintf(stderr, "groundspeed=%f\n", current.groundspeed);
	if (local_config.distmode == DIST_MILES)
		g_snprintf (s2, sizeof (s2), "%3.1f", current.groundspeed);
	if (local_config.distmode == DIST_METRIC)
		g_snprintf (s2, sizeof (s2), "%3.1f", current.groundspeed);
	if (local_config.distmode == DIST_NAUTIC)
		g_snprintf (s2, sizeof (s2), "%3.1f", current.groundspeed);
	g_snprintf (s3, sizeof (s3),
		    "<span color=\"%s\" font_desc=\"%s\">%s</span>",
		    local_config.color_bigdisplay,
		    local_config.font_bigdisplay, s2);
	gtk_label_set_markup (GTK_LABEL (speedlabel), s3);

	/* gtk_label_set_text (GTK_LABEL (speedlabel), s2); */

	if (havealtitude)
	{
		if (local_config.altmode == ALT_FEET)
		{
			g_snprintf (s2, sizeof (s2), "%.0f",
				    altitude * 3.2808399 + normalnull);
			g_strlcpy (s2a, "ft", sizeof (s2a));
		}
		else if (local_config.altmode == ALT_YARDS)
		{
			g_snprintf (s2, sizeof (s2), "%.0f",
				    altitude * 1.093613 + normalnull);
			g_strlcpy (s2a, "yd", sizeof (s2a));
		
		}
		else
		{
			g_snprintf (s2, sizeof (s2), "%.0f",
				    altitude + normalnull);
			g_strlcpy (s2a, "m", sizeof (s2a));
		}
		gtk_label_set_text (GTK_LABEL (altilabel), s2);

		if (local_config.guimode == GUI_PDA)
		{
			if (normalnull == 0.0)

				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>",
					    local_config.color_bigdisplay, s2, local_config.color_bigdisplay, s2a);
			else
				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" size=\"10000\">%s</span><span color=\"%s\" font_family=\"Arial\" size=\"5000\">%s</span>"
					    "<span color=\"red\" font_family=\"Arial\" size=\"5000\">\nNN %+.1f</span>",
					    local_config.color_bigdisplay, s2, local_config.color_bigdisplay, s2a,
					    normalnull);
		}
		else
		{
			if (normalnull == 0.0)

				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
					    local_config.color_bigdisplay, s2, local_config.color_bigdisplay, s2a);
			else
				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>"
					    "<span color=\"red\" font_family=\"Arial\" weight=\"bold\" size=\"8000\">\nNN %+.1f</span>",
					    local_config.color_bigdisplay, s2, local_config.color_bigdisplay, s2a,
					    normalnull);
		}
		gtk_label_set_markup (GTK_LABEL (altilabel), s3);
	}
	if (local_config.simmode)
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

		/*       if (local_config.nightmode && (isnight&& !disableisnight)) */
		/*  { */
		/*    gdk_gc_set_function (kontext, GDK_AND); */
		/*    gdk_gc_set_foreground (kontext, &colors.nightmode); */
		/*    gdk_draw_rectangle (drawing_miniimage->window, kontext, 1, 0, 0, 128, */
		/*                        103); */
		/*    gdk_gc_set_function (kontext, GDK_COPY); */
		/*  } */
		gdk_gc_set_foreground (kontext, &colors.red);
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
		compassimage = read_icon("compass.png",1);
	if (compassimage == NULL && do_unit_test ) {
	    exit (-1);
	}

	gdk_draw_pixbuf (drawable_bearing, compasskontext, compassimage, 0, 0,
			 0, 0, PSIZE, PSIZE, GDK_RGB_DITHER_NONE, 0, 0);


	if (foundradar)
	{
		gdk_gc_set_foreground (kontext, &colors.red);
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

	gdk_gc_set_foreground (kontext, &colors.black);

	/* compass */
	//      /* added by zwerg (Daniel Wernle)
	w = - current.heading + M_PI;

	j = 0;

	for (line_count = 0; line_count < 12; line_count++)
	{
		gdk_gc_set_foreground (compasskontext, &colors.red);

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
				if (local_config.guimode == GUI_PDA)
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
							     &colors.black, NULL);
				if (wplabellayout != NULL)
					g_object_unref (G_OBJECT
							(wplabellayout));
				/* freeing PangoFontDescription, cause it has been copied by prev. call */
				pango_font_description_free (pfd);

			}
			gdk_gc_set_foreground (compasskontext, &colors.red);
		}
		else
		{
			gdk_gc_set_foreground (compasskontext, &colors.black);
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

	w = current.bearing + M_PI;

#define TRIANGLEFACTOR 0.75
	gdk_gc_set_foreground (compasskontext, &colors.black);

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

	gdk_gc_set_foreground (compasskontext, &colors.black);
	gdk_gc_set_line_attributes (compasskontext, 2, 0, 0, 0);

	gdk_draw_polygon (drawable_bearing, compasskontext, 1,
			  (GdkPoint *) poly, 5);

	gdk_gc_set_foreground (kontext, &colors.black);

	return TRUE;

}

/* *****************************************************************************
 * Copy Image from loaded map
 */
gint
expose_cb (GtkWidget * widget, guint * datum)
{
	gint x, y, i, oldxoff, oldyoff, xoffmax, yoffmax, ok, okcount;
	gdouble tx, ty;
	gchar name[40], s1[40], *tn;

	if (mydebug >50) printf ("expose_cb()\n");

	/*    g_print("\nexpose_cb %d",exposecounter++);   */

	/*   fprintf (stderr, "lat: %f long: %f\n", coords.current_lat, coords.current_lon); */
	if (exposed && local_config.guimode == GUI_PDA)
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

		if (gui_status.posmode)
		{
			coords.current_lon = coords.posmode_lon;
			coords.current_lat = coords.posmode_lat;
		}


		/*  get pos for current position */
		calcxy (&posx, &posy, coords.current_lon,
			coords.current_lat, zoom);

		/*  do this because calcxy already substracted xoff and yoff */
		posx = posx + xoff;
		posy = posy + yoff;

		/*  Calculate Angle to destination */
		tx = (2 * R * M_PI / 360) * cos (DEG2RAD (coords.current_lat))
			* (coords.target_lon - coords.current_lon);
		ty = (2 * R * M_PI / 360)
			* (coords.target_lat - coords.current_lat);
		current.bearing = atan (tx / ty);

		if (TRUE)
		{
			/*  correct the value to be < 2*PI */
			if (ty < 0)
				current.bearing += M_PI;
			if (current.bearing >= (2 * M_PI))
				current.bearing -= 2 * M_PI;
			if (current.bearing < 0)
				current.bearing += 2 * M_PI;
		}

		if (local_config.showfriends && current.target[0] == '*')
			for (i = 0; i < maxfriends; i++)
			{
				g_strlcpy (name, "*", sizeof (name));

				g_strlcat (name, (friends + i)->name,
					   sizeof (name));
				tn = g_strdelimit (name, "_", ' ');
				if ((strcmp (current.target, tn)) == 0)
				{
				    coordinate_string2gdouble((friends + i)->lat, &coords.target_lat);
				    coordinate_string2gdouble((friends + i)->lon, &coords.target_lon);
				}
			}

		/*  Calculate distance to destination */
		dist = calcdist (coords.target_lon, coords.target_lat);

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
		if (local_config.nightmode && isnight)
		{
			gdk_gc_set_function (kontext, GDK_AND);
			gdk_gc_set_foreground (kontext, &colors.nightmode);
			gdk_draw_rectangle (drawable, kontext, 1, 0, 0,
					    SCREEN_X, SCREEN_Y);
			gdk_gc_set_function (kontext, GDK_COPY);
		}
	}

	if (usesql)
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
	if (local_config.showfriends)
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

	if (!local_config.simmode)
		return TRUE;

	ACCELMAX = 0.00002 + dist / 30000.0;
	ACCEL = ACCELMAX / 20.0;
	long_diff += ACCEL * sin (current.bearing);
	lat_diff += ACCEL * cos (current.bearing);
	if (long_diff > ACCELMAX)
		long_diff = ACCELMAX;
	if (long_diff < -ACCELMAX)
		long_diff = -ACCELMAX;
	if (lat_diff > ACCELMAX)
		lat_diff = ACCELMAX;
	if (lat_diff < -ACCELMAX)
		lat_diff = -ACCELMAX;


	coords.current_lat += lat_diff;
	coords.current_lon += long_diff;
	secs = g_timer_elapsed (timer, 0);
	if (secs >= 1.0)
	{
		g_timer_stop (timer);
		g_timer_start (timer);
		tx = (2 * R * M_PI / 360) * cos (M_PI * coords.current_lat / 180.0) *
			(coords.current_lon - coords.old_lon);
		ty = (2 * R * M_PI / 360) * (coords.current_lat - coords.old_lat);
#define MINSPEED 1.0
		if (((fabs (tx)) > MINSPEED) || (((fabs (ty)) > MINSPEED)))
		{
			lastdirection = current.heading;
			if (ty == 0)
				current.heading = 0.0;
			else
				current.heading = atan (tx / ty);
			if (!finite (current.heading))
				current.heading = lastdirection;

			if (ty < 0)
				current.heading = M_PI + current.heading;
			if (current.heading >= (2 * M_PI))
				current.heading -= 2 * M_PI;
			if (current.heading < 0)
				current.heading += 2 * M_PI;
			current.groundspeed =
				milesconv * sqrt (tx * tx +
						  ty * ty) * 3.6 / secs;
		}
		else
			current.groundspeed = 0.0;
		if (current.groundspeed > 999)
			current.groundspeed = 999;
		coords.old_lat = coords.current_lat;
		coords.old_lon = coords.current_lon;
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
		/* expose_mini_cb (NULL, 0); <-- already done by expose_cb */
	}
	return TRUE;
}

void
scaler_init()
{ // Search which scaler_pos is fitting scalewanted 
    gint scaler_pos=0;
    gint scale =0;
    while ( (scalewanted > scale ) && (scaler_pos <= slistsize ) ) 
	{
	    scaler_pos++;
	    scale = nlist[(gint) rint (scaler_pos)];
	};
    // set scale slider
    scaler_adj = gtk_adjustment_new ( scaler_pos,
				      // Low, upper, step
				      0, slistsize - 1, 1,
				      // page inc , page-size
				      slistsize / 4,	 1 / slistsize );
    scaler_widget = gtk_hscale_new (GTK_ADJUSTMENT (scaler_adj));
    gtk_signal_connect (GTK_OBJECT (scaler_adj), "value_changed",
			GTK_SIGNAL_FUNC (scaler_cb), NULL);
    gtk_scale_set_draw_value (GTK_SCALE (scaler_widget), FALSE);
}


/* *****************************************************************************
 * Increase/decrease displayed map scale
 * TODO: Improve finding of next apropriate map
 * datum:
 *    1 Zoom out
 *    2 Zoom in 
 */
gint
scalerbt_cb (GtkWidget * widget, guint datum)
{
	gint val;
	gchar oldfilename[2048];

	g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
	val = GTK_ADJUSTMENT (scaler_adj)->value;
	if (datum == 1 && val < slistsize - 1 ) {
		val +=1;
	} else if (datum == 2 && val > 0 ) {
		val -= 1;
	} else {
		/* nothing scaled -> return */
		return TRUE;
	}
	scalewanted = nlist[(gint) rint (val)];

	gtk_adjustment_set_value (GTK_ADJUSTMENT (scaler_adj), val);
	expose_cb (NULL, 0);
	expose_mini_cb (NULL, 0);

	needtosave = TRUE;

	return TRUE;
}
/* *****************************************************************************
 */
gint
setup2_cb (GtkWidget * widget, guint datum)
{
	gtk_widget_destroy (GTK_WIDGET (widget));
	return TRUE;
}



/* *****************************************************************************
 *  toggle coordinate format mode 
 */
gint
toggle_coords_cb (GtkWidget *widget)
{
	local_config.coordmode++;
	if (local_config.coordmode >= LATLON_N_FORMATS)
		local_config.coordmode = 0;
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
	local_config.simmode = !local_config.simmode;
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

	s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (serial_bt)));
	g_strlcpy (serialdev, s, sizeof (serialdev));
	needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 */
gint
dotripmeter (GtkWidget * widget, guint datum)
{
	gdouble d;

	d = calcdist (trip_lon, trip_lat);
	trip_lon = coords.current_lon;
	trip_lat = coords.current_lat;
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
	if (current.groundspeed / milesconv > tripmaxspeed)
		tripmaxspeed = current.groundspeed / milesconv;
	tripavspeedcount++;
	tripavspeed += current.groundspeed / milesconv;
	return TRUE;
}


/* *****************************************************************************
 */
gint
savetrack_cb (GtkWidget * widget, guint datum)
{
	savetrack = !savetrack;
	if (savetrack)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrack_bt),
					      TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrack_bt),
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
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mute_bt),
					      TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mute_bt),
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
 *  switching WLAN on/off 
 */
gint
wlan_draw_cb (GtkWidget * widget, guint datum)
{
    if ( NULL == widget ) 
	widget = wlan_draw_bt;

    if ( datum)
	wlan_draw = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),wlan_draw );
    
    

    if (wlan_draw)
	wlan_draw_list ();
    
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

    if ( mydebug > 1 )
	g_print ("streets_draw_cb=%d\n", datum);

    if ( datum )
	streets_draw = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),streets_draw );
    

    streets_draw_list ();
    needtosave = TRUE;
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
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track_bt),
					      TRUE);
		rebuildtracklist ();
	}
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track_bt),
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
	local_config.showwaypoints = !local_config.showwaypoints;
	if (local_config.showwaypoints)
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wp_bt), TRUE);
	else
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wp_bt), FALSE);
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
	g_print ("posmode=%d\n", gui_status.posmode);

    if (gui_status.posmode)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt),   TRUE);
    else
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt),   FALSE);
}

/* ****************************************************************************
 * toggle checkbox for Pos-Mode
 */
gint
pos_cb (GtkWidget *widget, guint datum)
{

	if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (posbt)) )
		gui_status.posmode = TRUE;
	else 
		gui_status.posmode = FALSE;

	/* change cursor in map area in pos mode */
	if (gui_status.posmode == TRUE)
	{
		gdk_window_set_cursor (drawing_area->window, cursor_cross);
	}
	else
	{
		gdk_window_set_cursor (drawing_area->window, NULL);
	}

	/* if waypoint select mode is enabled and waypoint
	 * selected then take target_lat/lon
	 * and save current_lon/lat for cancel */
	if (setwpactive && selected_wp_mode)
	{
		coords.posmode_lon = coords.target_lon;
		coords.posmode_lat = coords.target_lat;
		wp_saved_posmode_lon = coords.current_lon;
		wp_saved_posmode_lat = coords.current_lat;
	}
	else
	{
		coords.posmode_lon = coords.current_lon;
		coords.posmode_lat = coords.current_lat;
	}

	return TRUE;
}


/* ****************************************************************************
 * toggle checkbox for mapnik mode
 */
gint
toggle_mapnik_cb (GtkWidget *widget, guint datum)
{
	if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)) )
	{
		local_config.MapnikStatusInt = 1;
		gtk_widget_set_sensitive (GTK_WIDGET (frame_maptype), FALSE);
	}
	else 
	{
		local_config.MapnikStatusInt = 0;
		gtk_widget_set_sensitive (GTK_WIDGET (frame_maptype), TRUE);
	}
	
	// TODO: reload/update map display.....

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
					N_("/Menu/Messages"));
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
		local_config.showgrid = !local_config.showgrid;
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
		local_config.showfriends = !local_config.showfriends;
		needtosave = TRUE;
	}

	// Add Waypoint at current gps location
	if ((toupper (event->keyval)) == 'X')
	{
		wplat = coords.current_lat;
		wplon = coords.current_lon;
		addwaypoint_cb (NULL, NULL);
	}

	// Add Waypoint at current gps location without asking
	if ((toupper (event->keyval)) == 'W')
	{
		gchar wp_name[100], wp_type[100], wp_comment[100];
		time_t t;
		struct tm *ts;
		time (&t);
		ts = localtime (&t);
		g_snprintf (wp_name, sizeof (wp_name), "%s", asctime (ts));
		g_snprintf (wp_type, sizeof (wp_type), "waypoint.wpttemp.wpttemp-green");
		g_snprintf (wp_comment, sizeof (wp_comment), _("Quicksaved Waypoint"));

		if (usesql)
			addwaypoint (wp_name, wp_type, wp_comment,
				coords.current_lat, coords.current_lon, TRUE);
		else
			addwaypoint (wp_name, wp_type, wp_comment,
				coords.current_lat, coords.current_lon, FALSE);
	}

	// Add waypoint at current mouse location
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
		gchar wp_name[100], wp_type[100], wp_comment[100];
		time_t t;
		struct tm *ts;
		time (&t);
		ts = localtime (&t);
		g_snprintf (wp_name, sizeof (wp_name), "%s", asctime (ts));
		g_snprintf (wp_type, sizeof (wp_type), "waypoint.wpttemp.wpttemp-yellow");
		g_snprintf (wp_comment, sizeof (wp_comment), _("Temporary Waypoint"));

		gdk_window_get_pointer (drawing_area->window, &x, &y, &state);
		calcxytopos (x, y, &lat, &lon, zoom);
		//		if ( mydebug > 0 )
		printf ("Add Waypoint: %s lat:%f,lon:%f (x:%d,y:%d)\n", wp_name, lat, lon, x, y);
		if (usesql)
			addwaypoint (wp_name, wp_type, wp_comment, lat, lon, TRUE);
		else
			addwaypoint (wp_name, wp_type, wp_comment, lat, lon, FALSE);
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

	    if ( wlan_draw )
		wlan_query_area     ( min(lat1,lat2), min(lon1,lon2), max(lat1,lat2), max(lon1,lon2) );

	    gdouble lat,lon;
	    calcxytopos (x, y, &lat, &lon, zoom);
	    gdouble dist=lat2-lat1;
	    dist = dist>0?dist:-dist;
	    if ( streets_draw )
		streets_query_point ( lat,lon, dist );
	}

	// In Route mode Force next Route Point
	if (((toupper (event->keyval)) == 'J') && route.active)
	{
		forcenextroutepoint = TRUE;
	}

	// Switch Night Mode
	if ((toupper (event->keyval)) == 'N')
	{
		if (local_config.nightmode)
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
	    if (local_config.guimode == GUI_PDA)
		{
		    if (event->keyval == 0xFF52)
			scalerbt_cb (NULL, 1);	/* RNM */
		    if (event->keyval == 0xFF54)
			scalerbt_cb (NULL, 2);	/* RNM */
		}
	    
	    if ((toupper (event->keyval)) == '-' || (event->keyval == 0xFFad))	// Zoom out
		{
		    scalerbt_cb (NULL, 1);
		}
	    if ((toupper (event->keyval)) == '+' || (event->keyval == 0xFFab))	// Zoom in
		{
		    scalerbt_cb (NULL, 2);
		}
	    
	}
	if ( mydebug>10 ) 
	    fprintf(stderr,"Key pressed: %0x\n",event->keyval);
	
	
	if (gui_status.posmode)
	    {
		gdouble x, y;

		if ( (event->keyval == 0xff52))	// Up
		    {
			calcxy (&x, &y, coords.current_lon,
				coords.current_lat, zoom);
			calcxytopos (x, 0, &coords.current_lat,
				&coords.current_lon, zoom);
		    }
		
		if ( (event->keyval == 0xff54 )) // Down
		    {
			calcxy (&x, &y, coords.current_lon,
				coords.current_lat, zoom);
			calcxytopos (x, SCREEN_Y, &coords.current_lat,
				&coords.current_lon, zoom);
		    }
		if ( (event->keyval == 0xff51 )) // Left
		    {
			calcxy (&x, &y, coords.current_lon,
				coords.current_lat, zoom);
			calcxytopos (0, y, &coords.current_lat,
				&coords.current_lon, zoom);
		    }
		if ( (event->keyval == 0xff53 )) //  Right
			    
		    {
			calcxy (&x, &y, coords.current_lon, coords.current_lat, zoom);
			calcxytopos (SCREEN_X, y, &coords.current_lat, &coords.current_lon, zoom);
		    }
		coords.posmode_lon = coords.current_lon;
		coords.posmode_lat = coords.current_lat;
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

	minimap_xy2latlon(px, py, &lon, &lat, &dif);

	/*        g_print("\nstate: %x x:%d y:%d\n", state, x, y); */

	/*  Left mouse button */
	if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
	{
	    if (gui_status.posmode)
		{
		    coords.posmode_lon = lon;
		    coords.posmode_lat = lat;
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

	if (local_config.guimode == GUI_PDA)
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
		_("Name"), _("Type"), _("Latitude"), _("Longitude"), _("Distance"),
		NULL
	};
	GtkWidget *scrwindow, *vbox, *button, *hbox, *deletebt;
	GtkTooltips *tooltips;

	if (setwpactive)
		return TRUE;

	/* save old target/posmode for cancel event */
	wp_saved_target_lat = coords.target_lat;
	wp_saved_target_lon = coords.target_lon;
	if (gui_status.posmode) {
		wp_saved_posmode_lat = coords.posmode_lat;
		wp_saved_posmode_lon = coords.posmode_lon;
	}
	

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
	if (local_config.guimode == GUI_PDA)
		gtk_window_set_default_size (GTK_WINDOW (window),
					     real_screen_x, real_screen_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (window), 400, 360);

	mylist = gtk_clist_new_with_titles (6, tabeltitel1);
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
		if (route.active)
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
	/* disable jump button when in routingmode */
	if (route.active) gtk_widget_set_sensitive (gotobt, FALSE);

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
	/* sel_target_destroy event */
	gtk_signal_connect (GTK_OBJECT(window),
					"destroy",
					GTK_SIGNAL_FUNC(sel_target_destroy_cb),
					0);

	/* Font aendern falls PDA-Mode und Touchscreen */
	if (local_config.guimode == GUI_PDA)
	{
	    if (onemousebutton)
	    {
		/* Change default font throughout the widget */
		PangoFontDescription *font_desc;
		font_desc = pango_font_description_from_string ("Sans 20");
		gtk_widget_modify_font (mylist, font_desc);
		pango_font_description_free (font_desc);
	    }
	}

	insertwaypoints (FALSE);
	gtk_clist_set_column_justification (GTK_CLIST (mylist), 5,
					    GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_justification (GTK_CLIST (mylist), 0,
					    GTK_JUSTIFY_RIGHT);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 0, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 1, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 2, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 3, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 4, TRUE);
	gtk_clist_set_column_auto_resize (GTK_CLIST (mylist), 5, TRUE);

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
	if (!route.edit)
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
	     "\nCopyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>"
	     "\n         Website: http://www.gpsdrive.de\n\n",
	     _("-v        show version\n"),
	     _("-h        print this help\n"),
	     _("-d        turn on debug info\n"),
	     _("-D X      set debug Level to X\n"),
	     _("-T        do some internal unit Tests(don't start gpsdrive)\n"),
	     _("-e        use Festival-Lite (flite) for speech output\n"),
	     _("-t        set serial device for GPS i.e. /dev/ttyS1\n"),
	     _("-o        serial device, pty master, or file for NMEA *output*\n"),
	     _("-f X      Select friends server, X is i.e. friendsd.gpsdrive.de\n"),
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
	GtkWidget *ok_button;
	GtkWidget *cancel_button;
	
	fdialog = gtk_file_chooser_dialog_new (_("Select a track file"),
				GTK_WINDOW (mainwindow),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				NULL, NULL);
	
	gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
	
	cancel_button = gtk_dialog_add_button (GTK_DIALOG (fdialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	ok_button = gtk_dialog_add_button (GTK_DIALOG (fdialog), GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT);
		
	gtk_signal_connect (GTK_OBJECT
			    (ok_button),
			    "clicked", GTK_SIGNAL_FUNC (gettrackfile),
			    GTK_OBJECT (fdialog));
	gtk_signal_connect_object (GTK_OBJECT
				   (cancel_button), "clicked",
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   GTK_OBJECT (fdialog));


	g_strlcpy (buf, local_config.dir_home, sizeof (buf));
	g_strlcat (buf, "tracks/", sizeof (buf));          
	g_strlcat (buf, "track*.sav", sizeof (buf));

	gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), buf);

	gtk_widget_show (fdialog);

	return TRUE;
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
    GtkWidget *hbig_contr_map, *vbig1, *vbox;
    GtkWidget *vbox_controls; // The vbox on the left side, which holds the menu and control buttons
    GtkWidget *vbox_poi,*vbox_track;
    GtkWidget *vbig, *hbox_displays;
    GtkWidget *hbox_displays_a, *hbox_displays_b;
    GtkWidget *zoomin_bt, *zoomout_bt, *hbox_zoom;
    GtkWidget *hbox_scaler;
    GtkWidget *vboxlow, *hboxlow;
    GtkWidget *menuwin = NULL, *menuwin2 = NULL;
    GtkWidget *vtable, *wplabeltable, *alignment1;
    GtkWidget *alignment2;
    gchar maintitle[100];
    /*   GdkColor farbe;   */
    GdkRectangle rectangle = {
	0, 0, SCREEN_X, SCREEN_Y
    };
    gchar buf[500];

    /*** Mod by Arms */
    gint i, screen_height, screen_width;
    GtkWidget *table1_displays, *wi;
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
    
    tzset ();
    gmt_time = time (NULL);
       
    lt = gmtime (&gmt_time);
    local_time = mktime (lt);
    zone = lt->tm_isdst + (gmt_time - local_time) / 3600;
    /*   fprintf(stderr,"\n zeitzone: %d\n",zone); */
    

    /*   zone = st->tm_gmtoff / 3600; */
    /*  initialize variables */
    /*  Munich */
    srand (gmt_time);
    f = 0.02 * (0.5 - rand () / (RAND_MAX + 1.0));
    coords.current_lat = coords.zero_lat = 48.13706 + f;
    f = 0.02 * (0.5 - rand () / (RAND_MAX + 1.0));
    coords.current_lon = coords.zero_lon = 11.57532 + f;
    /*    zero_lat and zero_lon are overwritten by gpsdriverc,  */
    tripreset ();
    
    g_strlcpy (dgpsserver, "dgps.wsrcc.com", sizeof (dgpsserver));
    g_strlcpy (dgpsport, "2104", sizeof (dgpsport));
    g_strlcpy (gpsdservername, "127.0.0.1", sizeof (gpsdservername));
    g_strlcpy (current.target, "     ", sizeof (current.target));
    g_strlcpy (utctime, "n/a", sizeof (utctime));
    g_strlcpy (oldangle, "none", sizeof (oldangle));
    pixelfact = MAPSCALE / PIXELFACT;
    g_strlcpy (oldfilename, "", sizeof (oldfilename));
    maploaded = FALSE;
    haveNMEA = FALSE;
    havepos = gblink = blink = FALSE;
    haveposcount = haveGARMIN = debug = 0;
    current.heading = current.bearing = 0.0;
    zoom = 1;
    iszoomed = FALSE;
    find_poi_bt = NULL;
#ifdef DBUS_ENABLE
    useDBUS = FALSE;
#endif

    { /* Set fonts */
	// GdkFont *font_text, *font_verysmalltext, *font_smalltext;
	// font_s_text, font_s_verysmalltext, font_s_smalltext;
	if (local_config.guimode == GUI_PDA)
        {
            g_strlcpy (local_config.font_bigdisplay, "Sans bold 12",
            	sizeof (local_config.font_bigdisplay));
            g_strlcpy (font_s_smalltext, "Sans 10", sizeof (font_s_smalltext));
            g_strlcpy (font_s_text, "Sans 8", sizeof (font_s_text));
            g_strlcpy (font_s_verysmalltext, "Sans 6", sizeof (font_s_verysmalltext));
            g_strlcpy (local_config.font_wplabel, "Sans 8", sizeof (local_config.font_wplabel));
            printf("pdamode for fonts\n");
        } else {
            g_strlcpy (font_s_smalltext, "Sans 10", sizeof (font_s_smalltext));
            g_strlcpy (font_s_text, "Sans 11", sizeof (font_s_text));
            g_strlcpy (font_s_verysmalltext, "Sans 6", sizeof (font_s_verysmalltext));
        }
	pfd_text = pango_font_description_from_string (font_s_text);
	pfd_verysmalltext = pango_font_description_from_string (font_s_verysmalltext);
	pfd_smalltext = pango_font_description_from_string (font_s_smalltext);
	pfd_bigtext = pango_font_description_from_string
		(local_config.font_bigdisplay);
	pfd_wplabel = pango_font_description_from_string
		(local_config.font_wplabel);
    }


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
    g_strlcpy (messagename, "", sizeof (messagename));
    g_strlcpy (messageack, "", sizeof (messageack));
    g_strlcpy (messagesendtext, "", sizeof (messagesendtext));
    
    downloadwindowactive = downloadactive = importactive = FALSE;
    g_strlcpy (lastradar, "", sizeof (lastradar));
    g_strlcpy (lastradar2, "", sizeof (lastradar2));
    g_strlcpy (dbhost, "localhost", sizeof (dbhost));
    g_strlcpy (dbuser, "gast", sizeof (dbuser));
    g_strlcpy (dbpass, "gast", sizeof (dbpass));
    g_strlcpy (dbname, "geoinfo", sizeof (dbname));
    g_strlcpy (dbtable, "poi", sizeof (dbtable));
    g_strlcpy (wlantable, "wlan", sizeof (wlantable));
	g_strlcpy (poitypetable, "poi_type", sizeof (poitypetable));
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

    earthr = calcR (coords.current_lat);


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
	
	/* get language, used for POI titles and descriptions */
	if (!g_strlcpy (language,lstr2,3))
		g_strlcpy (language, "en", sizeof(language));
	
	/*    needed for right decimal delimiter ('.' or ',') */
	// setlocale(LC_NUMERIC, "en_US");
	setlocale(LC_NUMERIC, "C");
    }

	/* init config struct with default values */
	config_init ();
	
	check_and_create_files();
	
	/* update config struct with settings from config file if possible */
	readconfig ();

    real_screen_x = 640;
    real_screen_y = 512;
    coords.target_lon = coords.current_lon + 0.00001;
    coords.target_lat = coords.current_lat + 0.00001;


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
    screen_height = gdk_screen_height ();
    if ( mydebug >5 ) 
	fprintf(stderr , "gdk screen height : %d\n", screen_height);
    screen_width = gdk_screen_width ();
    if ( mydebug >5 ) 
	fprintf(stderr , "gdk screen width : %d\n", screen_width);

    /* parse cmd args */
    /* long options for use of --geometry and -g */
     int option_index = 0;
     static struct option long_options[] =
             {
              {"geometry", required_argument, 0, 'g'},
              {0, 0, 0, 0}
             };
    do
	{
	    /* long options plus --geometry and -g */
            i = getopt_long (argc, argv,
			"W:ESA:ab:c:zXx1qivPdD:TFepH:hnf:l:t:s:o:r:g:?",
			long_options, &option_index);
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
		    local_config.guimode = GUI_PDA;
		    break;
		case '1':
		    onemousebutton = TRUE;
		    break;
		case 'v':
		    printf ("\ngpsdrive (c) 2001-2006 Fritz Ganter <ganter@ganter.at>\n" "\nVersion %s\n%s\n\n", VERSION, rcsid);
		    exit (0);
		    break;
		case 't':
		    g_strlcpy (serialdev, optarg, sizeof (serialdev));
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
		    gui_status.posmode = TRUE;
		    break;
                /* Allows command line declaration of -g or --geometry */
		case 'g':
		    g_strlcpy (geometry, optarg, sizeof (geometry));
		    usegeometry = TRUE;
		    break;
		}
	}
    while (i != -1);

    if ( mydebug >99 ) fprintf(stderr , "options parsed\n");

	
    init_lat2RadiusArray();

    gethostname (hostname, 256);
    proxyport = 80;
    haveproxy = FALSE;

    if ( mydebug > 0 )
	printf ("\ngpsdrive (c) 2001-2006 Fritz Ganter <ganter@ganter.at>\n"
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

    /* Create toplevel window */

    get_window_sizing (geometry, usegeometry, screen_height, screen_width);

    if ( mydebug >19 )
	fprintf(stderr , "screen size %d,%d\n",SCREEN_X,SCREEN_Y);

#ifndef NOPLUGINS
    useplugins = TRUE;
#endif
    if (local_config.guimode == GUI_PDA)
	useplugins = FALSE;

    if (usesql)
	{
	    mod_setupcounter++;
	    setupfunction[mod_setupcounter] = &(setup_poi);
	    sqlplace = mod_setupcounter;
	}
    mod_setupcounter++;
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



    if ( mydebug >99 ) fprintf(stderr , "create Main Window\n");

    mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    if (!nosplash)
	gtk_window_set_transient_for (GTK_WINDOW (splash_window),
				      GTK_WINDOW (mainwindow));



    g_snprintf (maintitle, sizeof (maintitle),
		"%s v%s  \xc2\xa9 2001-2006 Fritz Ganter", "GpsDrive",
		VERSION);

    gtk_window_set_title (GTK_WINDOW (mainwindow), maintitle);
    gtk_container_set_border_width (GTK_CONTAINER (mainwindow), 0);
    gtk_window_set_position (GTK_WINDOW (mainwindow), GTK_WIN_POS_CENTER);
    gtk_signal_connect (GTK_OBJECT (mainwindow), "delete_event",
			GTK_SIGNAL_FUNC (quit_program), NULL);

    gtk_signal_connect (GTK_OBJECT (mainwindow), "key_press_event",
			GTK_SIGNAL_FUNC (key_cb), NULL);

    frame_status = gtk_statusbar_new ();
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (frame_status), FALSE);
    statusid =
	gtk_statusbar_get_context_id (GTK_STATUSBAR (frame_status), "main");

    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid,
			_("Gpsdrive-2 (c)2001-2006 F.Ganter"));
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

    hbig_contr_map = gtk_hbox_new (FALSE, 0 * PADDING);
    vbig = gtk_vbox_new (FALSE, 0 * PADDING);
    item_factory =
	gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", NULL);
    /*  Uebersetzen laut Bluefish Code */
    gtk_item_factory_set_translate_func (item_factory, menu_translate,
					 "<main>", NULL);
    gtk_item_factory_create_items (item_factory, nmenu_items, main_menu,
				   NULL);

    menubar = gtk_item_factory_get_widget (item_factory, "<main>");

    /* disabled, not needed any more
    wi = gtk_item_factory_get_item (item_factory,
				    N_("/Menu/Waypoint Manager"));
    if (!debug)
	gtk_widget_set_sensitive (wi, FALSE);
    */


    if (havespeechout)
	{
	    mute_bt = gtk_check_button_new_with_label (_("M_ute"));
	    gtk_button_set_use_underline (GTK_BUTTON (mute_bt), TRUE);
	    if (muteflag)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
					      (mute_bt), TRUE);

	    gtk_signal_connect (GTK_OBJECT (mute_bt),
				"clicked", GTK_SIGNAL_FUNC (mute_cb),
				(gpointer) 1);
	}
    if (usesql)
	{
	    getsqldata ();
	}
    else
	loadwaypoints ();


	/* set start position for simulation mode
	 * (only available in waypoints mode) */
	if (strlen (setpositionname) > 0 && !usesql)
	{
		for (i = 0; i < maxwp; i++)
		{
			if (!(strcasecmp ((wayp + i)->name, setpositionname)))
			{
				coords.current_lat = (wayp + i)->lat;
				coords.current_lon = (wayp + i)->lon;
				coords.target_lon = coords.current_lon + 0.001;
				coords.target_lat = coords.current_lat + 0.001;
			}
		}
	}

    {	// Frame POI
	if ( mydebug >99 ) fprintf(stderr , "create POI Frame\n");
	GtkTooltips *tooltips;
	tooltips = gtk_tooltips_new ();
	frame_poi = gtk_frame_new (_("Points"));
	vbox_poi = gtk_vbox_new (TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_poi), vbox_poi);

	// Checkbox ---- POI Draw
	poi_draw_bt = gtk_check_button_new_with_label (_("PO_I"));
	gtk_button_set_use_underline (GTK_BUTTON (poi_draw_bt), TRUE);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), poi_draw_bt,
			      _("Draw Points Of Interest found in mySQL"),
			      NULL);
	if (!poi_draw)
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poi_draw_bt), TRUE);
	gtk_signal_connect (GTK_OBJECT (poi_draw_bt), "clicked",
			    GTK_SIGNAL_FUNC (poi_draw_cb), (gpointer) 1);
	if (usesql)
	    gtk_box_pack_start (GTK_BOX (vbox_poi),   poi_draw_bt,    FALSE, FALSE, 0 * PADDING);

	// Checkbox ---- WLAN Draw
	if ( mydebug >99 ) fprintf(stderr , "create WLAN Frame \n");
	wlan_draw_bt = gtk_check_button_new_with_label (_("_WLAN"));
	if ( mydebug >99 ) fprintf(stderr , "create WLAN Frame \n");
	gtk_button_set_use_underline (GTK_BUTTON (wlan_draw_bt), TRUE);
	if ( mydebug >99 ) fprintf(stderr , "create WLAN Frame \n");
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wlan_draw_bt,
			      _("Wlan"),
			      NULL);
	if ( mydebug >99 ) fprintf(stderr , "create WLAN Frame \n");
	if (!wlan_draw)
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wlan_draw_bt), TRUE);
	if ( mydebug >99 ) fprintf(stderr , "create WLAN Frame \n");
	gtk_signal_connect (GTK_OBJECT (wlan_draw_bt), "clicked",
			    GTK_SIGNAL_FUNC (wlan_draw_cb), (gpointer) 1);
	if ( mydebug >99 ) fprintf(stderr , "create WLAN Frame \n");
	if (usesql)
	    gtk_box_pack_start (GTK_BOX (vbox_poi),   wlan_draw_bt,    FALSE, FALSE, 0 * PADDING);
	if ( mydebug >99 ) fprintf(stderr , "create WLAN Frame \n");

	// Checkbox ---- Show WP
	wp_bt = gtk_check_button_new_with_label (_("_WP"));
	gtk_button_set_use_underline (GTK_BUTTON (wp_bt), TRUE);
	if (local_config.showwaypoints)
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wp_bt), TRUE);
	gtk_signal_connect (GTK_OBJECT (wp_bt),
			    "clicked", GTK_SIGNAL_FUNC (wp_cb), (gpointer) 1);
	gtk_box_pack_start (GTK_BOX (vbox_poi), wp_bt, FALSE, FALSE, 0 * PADDING);

    }

    /* PORTING */
    /*   setup_bt = gtk_button_new_with_label (_("Setup")); */
    setup_bt = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);

    /*
    gtk_signal_connect (GTK_OBJECT (setup_bt),
			"clicked", GTK_SIGNAL_FUNC (setup_cb),
			(gpointer) 0);
    */
    { // Frame Track
	GtkTooltips *tooltips;
	tooltips = gtk_tooltips_new ();
	if ( mydebug >9 ) fprintf(stderr , "create Track Frame\n");
	frame_track = gtk_frame_new (_("Track"));
	vbox_track = gtk_vbox_new (TRUE, 1 * PADDING);
	gtk_container_add (GTK_CONTAINER (frame_track), vbox_track);

	/* Checkbox ---- Show Track */
	track_bt = gtk_check_button_new_with_label (_("Show"));
	gtk_button_set_use_underline (GTK_BUTTON (track_bt), TRUE);
	if (trackflag)
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track_bt),  TRUE);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), track_bt,
			      _("Show tracking on the map"), NULL);
	gtk_signal_connect (GTK_OBJECT (track_bt), "clicked",
			    GTK_SIGNAL_FUNC (track_cb), (gpointer) 1);
	gtk_box_pack_start (GTK_BOX (vbox_track), track_bt, FALSE, FALSE,	0 * PADDING);

	/* Checkbox ---- Save Track */
	savetrackfile (0);
	savetrack_bt = gtk_check_button_new_with_label (_("Save"));
	if (savetrack)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrack_bt), TRUE);
	gtk_signal_connect (GTK_OBJECT (savetrack_bt), "clicked",
		GTK_SIGNAL_FUNC (savetrack_cb), (gpointer) 1);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), savetrack_bt,
		_("Save the track to given filename at program exit"),
		NULL);
	gtk_box_pack_start (GTK_BOX (vbox_track), savetrack_bt, FALSE, FALSE,0 * PADDING);
	
	g_snprintf (s1, sizeof (s1), "%s", savetrackfn);
	lab1 = gtk_label_new (s1);
    }

    {
	if ( mydebug >99 ) fprintf(stderr , "create Buttons at upper left side\n");

	/*  Zoom in button */
	zoomin_bt = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (zoomin_bt), gtk_image_new_from_stock (GTK_STOCK_ZOOM_IN, GTK_ICON_SIZE_BUTTON));
	gtk_signal_connect (GTK_OBJECT (zoomin_bt),
			    "clicked", GTK_SIGNAL_FUNC (zoom_cb),
			    (gpointer) 1);

	/*  Zoom out button */
	zoomout_bt = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (zoomout_bt), gtk_image_new_from_stock (GTK_STOCK_ZOOM_OUT, GTK_ICON_SIZE_BUTTON));
	gtk_signal_connect (GTK_OBJECT (zoomout_bt),
			    "clicked", GTK_SIGNAL_FUNC (zoom_cb),
			    (gpointer) 2);

	/* Scaler right */
	scaler_right_bt = gtk_button_new_with_label (">>");
	gtk_signal_connect (GTK_OBJECT (scaler_right_bt),
			    "clicked", GTK_SIGNAL_FUNC (scalerbt_cb),
			    (gpointer) 1);

	/* Scaler left */
	scaler_left_bt = gtk_button_new_with_label ("<<");
	gtk_signal_connect (GTK_OBJECT (scaler_left_bt),
			    "clicked", GTK_SIGNAL_FUNC (scalerbt_cb),
			    (gpointer) 2);
    }

    /*  Find POI button */
    /*    if (maxwp > 0) */
    {
	find_poi_bt = gtk_button_new_from_stock (GTK_STOCK_FIND);
	if (!usesql)
	{
		gtk_signal_connect (GTK_OBJECT (find_poi_bt), "clicked",
				GTK_SIGNAL_FUNC (sel_target_cb), (gpointer) 2);
	}
	else
	{
		gtk_signal_connect (GTK_OBJECT (find_poi_bt), "clicked",
				GTK_SIGNAL_FUNC (poi_lookup_cb), (gpointer) 2);
	}
    }


    /*    gtk_window_set_default (GTK_WINDOW (mainwindow), zoomin_bt); */
    /*    if we want NMEA mode, gpsd must be running and we connect to port 2222 */
    /*    An alternate gpsd server may be on 2947, we try it also */

    initgps ();

    //if (haveGARMIN)
    //	gtk_widget_set_sensitive (startgps_bt, FALSE);

    if (usesql) 
	{
	    initkismet ();
	    get_poi_type_id_for_wlan();
	};


    if( havekismet )
	{
	    g_print (_("\nkismet server found\n"));
	    g_snprintf( buf, sizeof(buf), speech_kismet_found[voicelang] );
	    speech_out_speek (buf);
	}

    
    /*  Area for map */
    if ( mydebug >99 ) fprintf(stderr , "create Map Area\n");
    frame_map_area = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame_map_area), GTK_SHADOW_IN);
    drawing_area = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), SCREEN_X,
			   SCREEN_Y);
    gtk_container_add (GTK_CONTAINER (frame_map_area), drawing_area);

    gtk_widget_add_events (GTK_WIDGET (drawing_area),
			   GDK_BUTTON_PRESS_MASK);
    gtk_signal_connect_object (GTK_OBJECT (drawing_area),
			       "button-press-event",
			       GTK_SIGNAL_FUNC (mapclick_cb),
			       GTK_OBJECT (drawing_area));
	gtk_signal_connect_object (GTK_OBJECT (drawing_area),
			       "scroll_event",
			       GTK_SIGNAL_FUNC (mapscroll_cb),
			       GTK_OBJECT (drawing_area));

    /* Area for navigation pointer */
    drawing_bearing = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_bearing), PSIZE + 2,
			   PSIZE + 2);
    gtk_signal_connect (GTK_OBJECT (drawing_bearing),
			"expose_event", GTK_SIGNAL_FUNC (expose_compass),
			NULL);

    // Frame --- Area for mini map
    /* With small displays, this isn't really necessary! */
    if ( mydebug >99 ) fprintf(stderr , "create Mini Map Area\n");
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
    hbox_displays  = gtk_hbox_new (FALSE, 1 * PADDING);
    hbox_displays_a = gtk_hbox_new (FALSE, 1 * PADDING);
    hbox_displays_b = gtk_vbox_new (FALSE, 1 * PADDING);
    hbox_zoom = gtk_hbox_new (FALSE, 1 * PADDING);
    hbox_scaler  = gtk_hbox_new (FALSE, 1 * PADDING);

    if (!extrawinmenu)
	    if (SMALLMENU == 0)
		gtk_box_pack_start (GTK_BOX (hbox_displays),
				    GTK_WIDGET (drawing_miniimage),    TRUE, FALSE, 0 * PADDING);

    // Frame --- Bearing
    if ( mydebug >99 ) fprintf(stderr , "create Bearing Frame\n");
    frame_bearing = gtk_frame_new (_("Bearing"));
    compasseventbox = gtk_event_box_new ();
    gtk_container_add (GTK_CONTAINER (compasseventbox), drawing_bearing);
    alignment1 = gtk_alignment_new (0.6, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER (alignment1), compasseventbox);
    gtk_container_add (GTK_CONTAINER (frame_bearing), alignment1);

    gtk_box_pack_start (GTK_BOX (hbox_displays), frame_bearing, FALSE, FALSE,
			1 * PADDING);

    // Frame --- Sat levels
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

    //if(local_config.guimode != GUI_PDA)
	{
	    frame_sats = gtk_frame_new (_("GPS Info"));
	    sateventbox = gtk_event_box_new ();
	    gtk_container_add (GTK_CONTAINER (sateventbox), drawing_sats);
	    alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
	    if (SMALLMENU == 0) 
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

	    gtk_box_pack_start (GTK_BOX (hbox_displays), frame_sats, FALSE, FALSE, 1 * PADDING);
	}

    // Frame --- ACPI / Temperature / Battery
    if ( mydebug >99 ) fprintf(stderr , "create ACPI Frames\n");
    create_temperature_widget(hbox_displays);
    create_battery_widget(hbox_displays);


    if (local_config.guimode != GUI_PDA)
		gtk_box_pack_start (GTK_BOX (hbox_displays), hbox_displays_b, TRUE, TRUE, 1 * PADDING);

    if ( mydebug >99 ) fprintf(stderr , "create DISTANCE Frames\n");
    // Frame --- distance to destination 
    distlabel = gtk_label_new ("---");
    gtk_label_set_use_markup (GTK_LABEL (distlabel), TRUE);
    gtk_label_set_justify (GTK_LABEL (distlabel), GTK_JUSTIFY_RIGHT);

    // Frame --- speed over ground 
    speedlabel = gtk_label_new (_("---"));
    gtk_label_set_justify (GTK_LABEL (speedlabel), GTK_JUSTIFY_RIGHT);

    // Frame ---   displays zoom factor of map
    altilabel = gtk_label_new (_("n/a"));
    if (local_config.guimode == GUI_PDA)
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"5000\">%s</span>",
			local_config.color_bigdisplay, _("n/a"));
	}
    else
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
			local_config.color_bigdisplay, _("n/a"));
	}

    // Frame --- Altitude
    gtk_label_set_markup (GTK_LABEL (altilabel), s3);
    gtk_label_set_justify (GTK_LABEL (altilabel), GTK_JUSTIFY_RIGHT);


    if ( mydebug >99 ) fprintf(stderr , "create WP-Navigation Frames\n");
    // Frame --- 
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
    if (local_config.guimode == GUI_PDA)
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Sans\"  size=\"7000\">%s %d\n%d %s %.1fkm</span>",
			local_config.color_bigdisplay, _("Selected:"), wptotal, wpselected,
			_("within"), dbdistance);
	}
    else
	{
	    g_snprintf (s3, sizeof (s3),
			"<span color=\"%s\" font_family=\"Sans\"  size=\"10000\">%s %d\n%d %s %.1fkm</span>",
			local_config.color_bigdisplay, _("Selected:"), wptotal, wpselected,
			_("within"), dbdistance);
	}
    gtk_label_set_markup (GTK_LABEL (wplabel), s3);
    gtk_label_set_justify (GTK_LABEL (wplabel), GTK_JUSTIFY_LEFT);
#endif

    /*  create frames for labels */
    if(local_config.guimode != GUI_PDA)
	frame_target = gtk_frame_new (_("Distance to target"));
    else
	frame_target = gtk_frame_new (_("Distance"));
    destframe = frame_target;
    gtk_container_add (GTK_CONTAINER (frame_target), distlabel);
    if (local_config.guimode == GUI_PDA)
	{
	    if (local_config.distmode == DIST_MILES)
		g_snprintf (s1, sizeof (s1), "[%s]", _("mi/h"));
	    else if (local_config.distmode == DIST_NAUTIC)
		g_snprintf (s1, sizeof (s1), "[%s]", _("knots"));
	    else
		g_snprintf (s1, sizeof (s1), "[%s]", _("km/h"));
	}
    else
	{
	    if (local_config.distmode == DIST_MILES)
		g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
			    _("mi/h"));
	    else if (local_config.distmode == DIST_NAUTIC)
		g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
			    _("knots"));
	    else
		g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"),
			    _("km/h"));
	}

    frame_speed = gtk_frame_new (s1);
    gtk_container_add (GTK_CONTAINER (frame_speed), speedlabel);

	frame_altitude = gtk_frame_new (_("Altitude"));
    gtk_container_add (GTK_CONTAINER (frame_altitude), altilabel);

    frame_wp = gtk_frame_new (_("Waypoints"));


    if (local_config.guimode != GUI_PDA)
	vtable = gtk_table_new (1, 20, TRUE);
    else
	vtable = gtk_table_new (1, 20, FALSE);
    gtk_table_attach_defaults (GTK_TABLE (vtable), frame_target,    0,  6, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (vtable), frame_speed,     6, 12, 0, 1);
    gtk_table_attach_defaults (GTK_TABLE (vtable), frame_altitude, 12, 15, 0, 1);
    if(local_config.guimode != GUI_PDA)
        gtk_table_attach_defaults (GTK_TABLE (vtable), frame_wp,       15, 20, 0, 1);
    gtk_box_pack_start (GTK_BOX (hbox_displays_b), vtable, TRUE, TRUE, 2 * PADDING);         // target speed and altitude on trip table
    
    gtk_container_add (GTK_CONTAINER (frame_wp), wplabeltable);

    vbox = gtk_vbox_new (TRUE, 3 * PADDING);
    
    /* menubar */
    gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 1 * PADDING);
    
    
    /* zoom in/out */
    gtk_box_pack_start (GTK_BOX (vbox), hbox_zoom, FALSE, FALSE, 1 * PADDING);
    gtk_box_pack_start (GTK_BOX (hbox_zoom), zoomout_bt, TRUE, TRUE, 1 * PADDING);
    gtk_box_pack_start (GTK_BOX (hbox_zoom), zoomin_bt, TRUE, TRUE,  1 * PADDING);

    /* scaler */
    gtk_box_pack_start (GTK_BOX (vbox), hbox_scaler, FALSE, FALSE, 1 * PADDING);
    gtk_box_pack_start (GTK_BOX (hbox_scaler), scaler_left_bt, TRUE, TRUE,	1 * PADDING);
    gtk_box_pack_start (GTK_BOX (hbox_scaler), scaler_right_bt, TRUE, TRUE,	1 * PADDING);

    /* search poi */
    gtk_box_pack_start (GTK_BOX (vbox), find_poi_bt, FALSE, FALSE, 1 * PADDING);
    //gtk_box_pack_start (GTK_BOX (vbox), startgps_bt, FALSE, FALSE, 1 * PADDING);
    //    gtk_box_pack_start (GTK_BOX (vbox), setup_bt, FALSE, FALSE,    1 * PADDING);
    hboxlow = vbox_controls = NULL;
        

    if ( mydebug >99 ) fprintf(stderr , "create map-checkboxes Frames\n");
    frame_maptype = make_display_map_checkboxes();
    if ( mydebug >99 ) fprintf(stderr , "create map-controls Frames\n");
    frame_mapcontrol = make_display_map_controls();

    if ( mydebug >99 ) fprintf(stderr , "create extra-win-menus\n");
    if (!extrawinmenu)
	{
	    vbox_controls = gtk_vbox_new (FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox_controls), vbox, TRUE, TRUE,	1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox_controls), frame_poi, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox_controls), frame_track, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox_controls), frame_mapcontrol, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vbox_controls), frame_maptype, TRUE,	TRUE, 1 * PADDING);
	}
    else
	{
	    vboxlow = gtk_vbox_new (FALSE, 0 * PADDING);
	    hboxlow = gtk_hbox_new (FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vboxlow), frame_poi, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vboxlow), frame_track, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vboxlow), frame_mapcontrol, TRUE,	TRUE, 1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (vboxlow), frame_maptype, TRUE,	TRUE, 1 * PADDING);
	    if (SMALLMENU == 0)
		gtk_box_pack_start (GTK_BOX (vboxlow),
				    GTK_WIDGET (drawing_miniimage),   TRUE, FALSE, 0 * PADDING);
	    gtk_box_pack_start (GTK_BOX (hboxlow), vbox, TRUE, TRUE,	1 * PADDING);
	    gtk_box_pack_start (GTK_BOX (hboxlow), vboxlow, TRUE, TRUE,	1 * PADDING);

	}

    if (havespeechout)
	gtk_box_pack_start (GTK_BOX (vbox_poi), mute_bt, FALSE, FALSE,    0 * PADDING);

    if ( mydebug >99 ) fprintf(stderr , "scaler init\n");
    scaler_init();

    if (local_config.guimode == GUI_PDA)
	table1_displays = gtk_table_new (5, 3, FALSE);
    else
	{
	    if (SMALLMENU)
		table1_displays = gtk_table_new (4, 3, FALSE);
	    else
		table1_displays = gtk_table_new (8, 2, FALSE);
	}
    frame_lat = gtk_frame_new (_("Latitude"));
    frame_lon = gtk_frame_new (_("Longitude"));
    if ( mydebug >10 )
	frame_mapfile = gtk_frame_new (_("Map file"));
    frame_mapscale = gtk_frame_new (_("Map scale"));
    frame_heading = gtk_frame_new (_("Heading"));
    frame_bearing = gtk_frame_new (_("Bearing"));
    frame_timedest = gtk_frame_new (_("Time to Dest."));
    frame_prefscale = gtk_frame_new (_("Pref. scale"));

    if ( mydebug >99 ) fprintf(stderr , "Frame lat/lon\n");
	label_lat = gtk_label_new (_("000,00000N"));
	eventbox_lat = gtk_event_box_new ();
	gtk_widget_add_events (eventbox_lat, GDK_BUTTON_PRESS_MASK);
	g_signal_connect (eventbox_lat, "button_press_event",
				GTK_SIGNAL_FUNC (toggle_coords_cb), NULL);
	gtk_container_add (GTK_CONTAINER (eventbox_lat), label_lat);
	gtk_container_add (GTK_CONTAINER (frame_lat), eventbox_lat);

	label_lon = gtk_label_new (_("000,00000E"));
	eventbox_lon = gtk_event_box_new ();
	gtk_widget_add_events (eventbox_lon, GDK_BUTTON_PRESS_MASK);
	g_signal_connect (eventbox_lon, "button_press_event",
				GTK_SIGNAL_FUNC (toggle_coords_cb), NULL);
	gtk_container_add (GTK_CONTAINER (eventbox_lon), label_lon);
	gtk_container_add (GTK_CONTAINER (frame_lon), eventbox_lon);

    if ( mydebug >10 )
	{
	    label_map_filename = gtk_label_new (_("---"));
	    gtk_container_add (GTK_CONTAINER (frame_mapfile), label_map_filename);
	}

    if ( mydebug >99 ) fprintf(stderr , "Frame MapScale\n");
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

    if (local_config.guimode == GUI_PDA)
	{
	    //status bottom table 5 x 3
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_bearing, 0, 1, 0, 1);	// (left,right,top,bottom) 
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_heading, 1, 2, 0, 1);
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_mapscale, 2, 3, 0, 1);
	    
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_lat, 0, 1, 1, 2);
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_lon, 1, 2, 1, 2);
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_prefscale, 2, 3, 1, 2);
	    
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_timedest, 0, 1, 2, 3);
	    if ( mydebug >10 )
		gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_mapfile, 1, 3, 2, 3);
	    //KCFX
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), scaler_widget, 0, 3, 3, 4);
	    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_status, 0, 3, 4, 5);
	}
    else
	{
	    if (SMALLMENU)
		{
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_bearing, 0, 1, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_heading, 1, 2, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_lat, 2,  3, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_lon, 3,  4, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_timedest, 0, 1, 1, 2);
		    if ( mydebug >10 )
			gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_mapfile, 1, 2, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_mapscale, 2, 3, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_prefscale, 3, 4, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_status, 0, 4, 3, 4);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), scaler_widget, 0, 4, 2, 3);
		}
	    else
		{
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_bearing, 0,    1, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_heading, 1,    2, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_lat, 2,      3, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_lon, 3,  4, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_timedest, 4,  5, 0, 1);
		    if ( mydebug >10 )
			gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_mapfile, 5,  6, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_mapscale, 6,  7, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_prefscale, 7, 8, 0, 1);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), frame_status,	   0, 4, 1, 2);
		    gtk_table_attach_defaults (GTK_TABLE (table1_displays), scaler_widget,   4, 8, 1, 2);
		}
	}
    /*  all position calculations are made in the expose callback */
    gtk_signal_connect (GTK_OBJECT (drawing_area),
			"expose_event", GTK_SIGNAL_FUNC (expose_cb), NULL);

    if ( mydebug >99 ) fprintf(stderr , "Menu\n");
    if (local_config.guimode != GUI_PDA)
	if ( !extrawinmenu)
	    gtk_box_pack_start (GTK_BOX (hbig_contr_map), vbox_controls,  TRUE,  FALSE,  1 * PADDING);
    gtk_box_pack_start (GTK_BOX (hbig_contr_map), frame_map_area, TRUE, TRUE, 0 * PADDING);

    if (local_config.guimode != GUI_PDA)
	{
	    gtk_box_pack_start (GTK_BOX (vbig), hbig_contr_map, TRUE, TRUE,1 * PADDING);
	    if (extrawinmenu)
		{
		    menuwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);

		    gtk_window_set_title (GTK_WINDOW (menuwin),  _("Menu"));
		    gtk_container_set_border_width (GTK_CONTAINER (menuwin), 2 * PADDING);
		    gtk_container_add (GTK_CONTAINER (menuwin), hboxlow);
		    gtk_signal_connect (GTK_OBJECT (menuwin),
					"delete_event",
					GTK_SIGNAL_FUNC (quit_program),
					NULL);
		    menuwin2 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		    gtk_window_set_title (GTK_WINDOW (menuwin2),  _("Status"));
		    gtk_container_set_border_width (GTK_CONTAINER (menuwin2), 2 * PADDING);
		    vbig1 = gtk_vbox_new (FALSE, 2);
		    gtk_container_add (GTK_CONTAINER (menuwin2), vbig1);
		    gtk_box_pack_start (GTK_BOX (vbig1), hbox_displays,  TRUE,  FALSE, 2 * PADDING);
		    gtk_box_pack_start (GTK_BOX (vbig1), table1_displays, TRUE, FALSE, 2 * PADDING);
		    gtk_signal_connect (GTK_OBJECT (menuwin2),
					"delete_event",
					GTK_SIGNAL_FUNC (quit_program), NULL);
		}
	    else
		{
		    gtk_box_pack_start (GTK_BOX (vbig), hbox_displays,  TRUE,  TRUE,  2 * PADDING);
		    gtk_box_pack_start (GTK_BOX (vbig), table1_displays, FALSE, FALSE, 1 * PADDING);
		}
	    gtk_container_add (GTK_CONTAINER (mainwindow), vbig);
	}

    /*   if pdamode is set, we use gtk-notebook add arrange the elements */
    /*   in pdamode, extrawinmenu is also set! */
    mainnotebook = NULL;
    if (local_config.guimode == GUI_PDA)
	{
	    GtkWidget *l1, *l2, *label_status, *trip_label;     // tabs label for pda mode
	    l1 = gtk_label_new (NULL);
	    l2 = gtk_label_new (NULL);
	    label_status = gtk_label_new (NULL);
            trip_label = gtk_label_new (NULL);
	    /* for a better usability in onemousebutton mode */
	    if (onemousebutton)
		{
			/* gtk_misc_set_padding (GTK_MISC (l1), x, y); */
			gtk_misc_set_padding (GTK_MISC (l1), 10, 1);
			gtk_misc_set_padding (GTK_MISC (l2), 10, 1);
			gtk_misc_set_padding (GTK_MISC (label_status), 10, 1);
                        gtk_misc_set_padding (GTK_MISC (trip_label), 10, 1);
			
			/* http://developer.gnome.org/doc/API/2.0/pango/PangoMarkupFormat.html */
			
			char *markup;
			markup = g_markup_printf_escaped
			("<span font_desc='8'>%s</span>",
				_("Map"));
			gtk_label_set_markup (GTK_LABEL (l1), markup);
			markup = g_markup_printf_escaped
			("<span font_desc='8'>%s</span>",
				_("Menu"));
			gtk_label_set_markup (GTK_LABEL (l2), markup);
			markup = g_markup_printf_escaped
			("<span font_desc='8'>%s</span>",
				_("Status"));
			gtk_label_set_markup (GTK_LABEL (label_status), markup);
                        markup = g_markup_printf_escaped
			("<span font_desc='8'>%s</span>",
				_("Trip"));
			gtk_label_set_markup (GTK_LABEL (trip_label), markup);
			
			g_free (markup);
			
		}
	    else
		{
		    gtk_label_set_text (GTK_LABEL (l1), _("Map"));
		    gtk_label_set_text (GTK_LABEL (l2), _("Menu"));
		    gtk_label_set_text (GTK_LABEL (label_status), _("Status"));
                    gtk_label_set_text (GTK_LABEL (trip_label), _("Trip"));
		}
	    vbig1 = gtk_vbox_new (FALSE, 2);	// box for status tab
	    gtk_box_pack_start (GTK_BOX (vbig1), hbox_displays, TRUE, TRUE,  2 * PADDING);     // bearing, satellites, temperature, battery on status tab
	    gtk_box_pack_start (GTK_BOX (hbox_displays_b), hbox_displays_a, TRUE, TRUE, 2 * PADDING);    // wp on trip tab
	    gtk_box_pack_start (GTK_BOX (hbox_displays_a), frame_wp, TRUE, TRUE,1 * PADDING);

	    gtk_box_pack_start (GTK_BOX (vbig1), table1_displays, TRUE, TRUE,2 * PADDING);

	    mainnotebook = gtk_notebook_new ();					// create the main notebook window
	    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (mainnotebook),GTK_POS_TOP);	// tabs are on at the top edge
	    gtk_box_pack_start (GTK_BOX (vbig), hbig_contr_map, TRUE, TRUE,1 * PADDING);
	    gtk_container_add (GTK_CONTAINER (mainwindow), mainnotebook);
	    gtk_widget_show_all (hboxlow);
	    gtk_widget_show_all (vbig1);
	    gtk_widget_show_all (vbig);
	    gtk_widget_show_all (hbig_contr_map);
            gtk_widget_show_all (hbox_displays_b);
	    
	    gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), vbig,
				      l1);				// add 1st tab to notebook : map
	    gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook),
				      hboxlow, l2);			// add 2nd tab to notebook : menu
	    gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), vbig1,
				      label_status);			// add 3rd tab to notebook : status
            gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), hbox_displays_b,
				      trip_label);			// add 4th tab to notebook : trip
	    
	    gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);	// set map tab as selected one
	    gtk_widget_show_all (mainnotebook);				// show notebook window
	}

    /*** Mod by Arms */
    /* This one should position the windows in the corners, */
    /* so that gpsdrive can be run w/o a xwm (stand-alone mode) */
    /* With a xwm, you should be able to reposition the */
    /* windows afterwards... */
    if (extrawinmenu && SMALLMENU && local_config.guimode != GUI_PDA)
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
    if ((extrawinmenu) && (local_config.guimode != GUI_PDA))
	{
	    gtk_widget_show_all (menuwin);
	    gtk_widget_show_all (menuwin2);

	}

	gtk_window_set_auto_startup_notification (TRUE);
    gtk_widget_show_all (mainwindow);


    mainwindow_icon_pixbuf = read_icon ("gpsicon.png",1);
    if (mainwindow_icon_pixbuf)
	{
	    gtk_window_set_icon (GTK_WINDOW (mainwindow),
				 mainwindow_icon_pixbuf);
	    gdk_pixbuf_unref (mainwindow_icon_pixbuf);
	}

    /*  now we know the drawables */
    if (local_config.guimode == GUI_PDA)
	gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);

    drawable =
	gdk_pixmap_new (mainwindow->window, SCREEN_X, SCREEN_Y, -1);
    /* gtk_widget_set_double_buffered(drawable, FALSE);  */
    /*    drawable = drawing_area->window; */
    if (local_config.guimode == GUI_PDA)
	gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 2);

    drawable_sats = drawing_sats->window;
    drawable_bearing = drawing_bearing->window;
    /*  gtk_widget_set_double_buffered(GTK_WIDGET(drawable_sats), TRUE);   */
    /*  gtk_widget_set_double_buffered(GTK_WIDGET(drawable_bearing), TRUE);   */

    //KCFX
    // if (local_config.guimode != GUI_PDA) 
    kontext = gdk_gc_new (mainwindow->window);
    //  else 
    //    kontext = gdk_gc_new (mainnotebook->window); 

    gdk_gc_set_clip_origin (kontext, 0, 0);
    rectangle.width = SCREEN_X;
    rectangle.height = SCREEN_Y;

    gdk_gc_set_clip_rectangle (kontext, &rectangle);

    /* fill window with color */
    gdk_gc_set_function (kontext, GDK_COPY);
    gdk_gc_set_foreground (kontext, &colors.lcd2);
    gdk_draw_rectangle (drawing_area->window, kontext, 1, 0, 0, SCREEN_X,
			SCREEN_Y);
    {
	GtkStyle *style;
	style = gtk_rc_get_style (mainwindow);
	colors.defaultcolor = style->bg[GTK_STATE_NORMAL];
    }

    /* set cross cursor for map posmode */
    cursor_cross = gdk_cursor_new (GDK_TCROSS);
    
    /* we use the normal cursor now
    gdk_window_set_cursor (drawing_area->window, cursor_arrow);
    if (SMALLMENU == 0) 
        gdk_window_set_cursor (drawing_miniimage->window, cursor_arrow);
    */
    
    if (local_config.guimode == GUI_PDA)
	   gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 1);
    
    

    if (local_config.guimode == GUI_PDA)
	   gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);
    
    /*  Tooltips */
    tooltips = gtk_tooltips_new ();


    temperature_get_values ();
    battery_get_values ();

    if(local_config.guimode != GUI_PDA) {
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
	wi = gtk_item_factory_get_item (item_factory, N_("/Menu"));
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wi,
			      _("Here you find extra functions for maps, tracks and messages"),
			      NULL);
	if (havespeechout)
	    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), mute_bt,
				  _("Disable output of speech"), NULL);
    
	/*    if (maxwp > 0) */
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp_bt,
			      _("Show waypoints on the map"), NULL);

	//	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), setup_bt,
	//_("Settings for GpsDrive"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), zoomin_bt,
			      _("Zoom into the current map"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), zoomout_bt,
			      _("Zooms out off the current map"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scaler_left_bt,
			      _("Select the next more detailed map"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scaler_right_bt,
			      _("Select the next less detailed map"), NULL);
	/*    if (maxwp > 0) */
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips),
			      find_poi_bt,
			      _("Find Points of Interest and select as destination"),
			      NULL);
	if (scaler_widget)
	    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scaler_widget,
				  _("Select the map scale of avail. maps."),
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
	gtk_tooltips_set_delay (GTK_TOOLTIPS (tooltips), TOOLTIP_DELAY);
    }
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
	gtk_timeout_add (200, (GtkFunction) calldrawmarker_cb, NULL);

    /*  if we started in simulator mode we have a little move roboter */
    if (local_config.simmode)
	simpos_timeout =
	    gtk_timeout_add (300, (GtkFunction) simulated_pos, 0);
    if (nmeaout)
	gtk_timeout_add (1000, (GtkFunction) write_nmea_cb, NULL);
    gtk_timeout_add (10000, (GtkFunction) testconfig_cb, 0);
    gtk_timeout_add (600000, (GtkFunction) speech_saytime_cb, 0);
    gtk_timeout_add (1000, (GtkFunction) storetrack_cb, 0);
    gtk_timeout_add (10000, (GtkFunction) masteragent_cb, 0);
    gtk_timeout_add (15000, (GtkFunction) getsqldata, 0);
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
    streets_draw_cb (streets_draw_bt, 0);
    poi_draw_cb (poi_draw_bt, 0);
    wlan_draw_cb (wlan_draw_bt, 0);
    needtosave = FALSE;

	/* do all the basic initalisation for the specific sections */
	poi_init ();
	friends_init ();
	route_init ();
    wlan_init ();
    streets_init ();

	load_friends_icon ();

	// When all the basic gui stuff is finally moved into the *gui.c files,
	// this will call the necessary functions for graphical initialization.
	gui_init ();

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


    /* gtk2 requires these functions in the order below do not change */
    if (usegeometry) {
	GdkGeometry size_hints = {200, 200, 0, 0, 200, 200, 10, 10, 0.0, 0.0, GDK_GRAVITY_NORTH_WEST};

	gtk_window_set_geometry_hints(GTK_WINDOW (mainwindow), mainwindow, &size_hints,
                                  GDK_HINT_MIN_SIZE |
                                  GDK_HINT_BASE_SIZE |
                                  GDK_HINT_RESIZE_INC);

	if (!gtk_window_parse_geometry(GTK_WINDOW (mainwindow), geometry)) {
	    fprintf(stderr, "Failed to parse %s\n", geometry);
	}
    }

    // ==================================================================
    // Unit Tests
    if ( do_unit_test ) {
	unit_test();
    }

#ifdef MAPNIK
    init_mapnik(1);
    //render_mapnik(1);
#endif

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

    if ( do_unit_test ) {
	printf ("\n\nAll Unit Tests successfull\n\n");
	exit(0);
    }

    return 0;
}
