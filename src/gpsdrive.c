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
     Christoph Metz <loom@mopper.de>
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
#include "wlan.h"
#include "track.h"
#include "waypoint.h"
#include "routes.h"
#include "gps_handler.h"
#include "nmea_handler.h"
#include "map_handler.h"
#include <speech_strings.h>
#include <speech_out.h>

#include "import_map.h"
#include "download_map.h"
#include "icons.h"
#include "gui.h"
#include "poi_gui.h"
#include "main_gui.h"

#include "screenshot.h"


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
gint timeoutcount;
GtkWidget *pixmapwidget, *gotowindow;
GtkWidget *messagewindow;
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

gdouble long_diff = 0, lat_diff = 0;
GdkGC *kontext_map;

extern GtkWidget *drawing_minimap;

// TODO: should be moved away...
extern GtkWidget *drawing_gps, *drawing_compass;
extern GdkGC *kontext_compass, *kontext_gps, *kontext_minimap;
extern GdkDrawable *drawable_compass, *drawable_gps, *drawable_minimap;

GtkWidget *miles;
GdkDrawable *drawable;
gint haveposcount, blink, gblink, xoff, yoff, crosstoggle = 0;
gdouble pixelfact;
GdkPixbuf *image = NULL, *tempimage = NULL, *pixbuf_minimap = NULL;

extern GdkPixbuf *friendsimage, *friendspixbuf;

extern mapsstruct *maps;

extern GtkWidget *drawing_battery, *drawing_temp;

extern GtkWidget *map_drawingarea;


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
gint iszoomed;
static gchar const rcsid[] =
	"$Id$";
gint thisline;
gint maxwp, maxfriends = 0;
GtkStyle *style = NULL;
GtkRcStyle *mainstyle;
gint satlist[MAXSATS][4], satlistdisp[MAXSATS][4], satbit = 0;
GtkWidget *mylist;

extern gchar mapfilename[2048];

gdouble milesconv;
gdouble gbreit, glang, olddist = 99999.0;
GTimer *timer, *disttimer;
gint gcount;
gchar localedecimal;
gchar language[] = "en";

extern gint downloadwindowactive;
extern gint downloadactive;

GtkWidget *add_wp_name_text, *wptext2;
gchar oldangle[100];

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
		 88067900,90000000
};
#else
gint slistsize = 32;
gchar *slist[] = { "1000", "1500", "2000", "3000", "5000", "7500", 
		   "10000", "15000", "20000", "30000", "50000", "75000",
		   "100000", "150000", "200000", "300000", "500000", "750000", 
		   "1000000", "1500000", "2000000", "3000000", "5000000", "7500000", 
		   "10000000", "15000000", "20000000", "30000000", "50000000", "75000000",
		   "88067900","90000000"
};
gint nlist[] = { 1000, 1500, 2000, 3000, 5000, 7500,
		 10000, 15000, 20000, 30000, 50000, 75000,
		 100000, 150000, 200000, 300000, 500000, 750000,
		 1000000, 1500000, 2000000, 3000000, 5000000, 7500000,
		 10000000, 15000000, 20000000, 30000000, 50000000, 75000000,
		 88067900,90000000
};
#endif


GtkWidget *label_map_filename;
GtkWidget *label_timedest;
GtkWidget *wp_bt;
GtkWidget *bestmap_bt, *poi_draw_bt, *wlan_draw_bt;

GtkWidget *track_bt;
GtkWidget *savetrack_bt;
GtkWidget *loadtrack_bt;
GdkSegment *track, *trackshadow;
glong tracknr;
trackcoordstruct *trackcoord;
extern glong trackcoordnr, tracklimit, trackcoordlimit,old_trackcoordnr;
gchar savetrackfn[256];

gint havespeechout, hours, minutes, speechcount = 0;
gchar lastradar[40], lastradar2[40]; 
gint foundradar;
gdouble radarbearing;
gint errortextmode = TRUE;
gint haveproxy, proxyport;
gchar proxy[256], hostname[256];

/*** Mod by Arms */
gint real_psize, real_smallmenu;
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
typedef struct
{
	gchar n[200];
}
namesstruct;
namesstruct *names;
gchar gpsdservername[200], setpositionname[80];
gint newsatslevel = TRUE;
GtkWidget *mapdirbt;
GtkWidget *slowcpubt;
GtkWidget *add_wp_lon_text, *add_wp_lat_text;

gdouble precision = (-1.0), gsaprecision = (-1.0);
gint oldsatfix = 0, oldsatsanz = -1, havealtitude = FALSE;
gint satfix = 0, usedgps = FALSE;
gchar dgpsserver[80], dgpsport[10];
GtkWidget *posbt;
GtkWidget *cover;
extern gint PSIZE;
extern gint needreloadmapconfig;
GdkPixbuf *batimage = NULL;
GdkPixbuf *temimage = NULL;
GdkPixbuf *satsimage = NULL;
gint sats_used = 0, sats_in_view = 0;
gint numgrids = 4, scroll = TRUE;
gint satposmode = FALSE;
gint printoutsats = FALSE;
extern gchar *displaytext;
gint isnight = FALSE, disableisnight;
gint nighttimer;
GtkWidget *setupentry[50], *setupentrylabel[50];
gchar utctime[20], loctime[20];
gint redrawtimeout;
gint borderlimit;
gint pdamode = FALSE;
gint exposecounter = 0, exposed = FALSE;
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
double dbdistance;
gint usesql = FALSE, dbusedist = FALSE;
extern gint sqlselects[MAXPOITYPES], kismetsock, havekismet;

extern GdkPixbuf *kismetpixbuf,	*iconpixbuf[50];
gint earthmate = FALSE;

extern GdkPixbuf *posmarker_img;

extern gint wptotal, wpselected;

extern routestatus_struct route;
extern color_struct colors;

GdkFont *font_wplabel;
gchar font_text[100];

gint drawmarkercounter = 0, loadpercent = 10, globruntime = 30;
extern int pleasepollme;


gint forcehavepos = FALSE;
extern gchar cputempstring[20], batstring[20];
extern GtkWidget *tempeventbox, *batteventbox;
GtkWidget *sateventbox = NULL;
GtkWidget *satsvbox, *satshbox, *satslabel1eventbox;
GtkWidget *satslabel2eventbox, *satslabel3eventbox;
GtkWidget *satslabel1, *satslabel2, *satslabel3;
GtkWidget *frame_map_area;
GtkWidget *frame_wp;
GtkWidget *frame_poi,*frame_track, *lab1;
GtkWidget *menubar;
gchar messagename[40], messagesendtext[1024], messageack[100];
gint statuslock = 0, gpson = FALSE;
int messagenumber = 0, didrootcheck = 0;
int timerto = 0;
GtkTextBuffer *getmessagebuffer;

int newdata = FALSE;
extern pthread_mutex_t mutex;
//int mapistopo = FALSE;
int nosplash = FALSE;
int havedefaultmap = TRUE;

int storetz = FALSE;
int egnoson = 0, egnosoff = 0;

// ---------------------- for nmea_handler.c
extern gint haveRMCsentence;
extern gchar nmeamodeandport[50];
extern gdouble NMEAsecs;
extern gint NMEAoldsecs;
extern FILE *nmeaout;
/*  if we get data from gpsd in NMEA format haveNMEA is TRUE */
extern gint haveNMEA;
#ifdef DBUS_ENABLE
extern gint useDBUS;
#endif
extern int nmeaverbose;
extern gint bigp , bigpGGA , bigpRME , bigpGSA, bigpGSV;
extern gint lastp, lastpGGA, lastpRME, lastpGSA, lastpGSV;

extern GtkWidget *main_window;
extern GtkWidget *frame_statusbar;
extern GtkWidget *main_table;

void sql_load_lib();
void unit_test(void);
void drawdownloadrectangle (gint big);
void draw_grid (GtkWidget * widget);


gchar geometry[80];
gint usegeometry = FALSE;
gboolean takescreenshots = FALSE;

/* 
 * ****************************************************************************
 * ****************************************************************************
 * ****************************************************************************
*/



/* ******************************************************************
 * Check dirs and files
 */
void check_and_create_files(){
    gchar file_path[2048];
    struct stat buf;

    if ( mydebug >5 ) fprintf(stderr , " check_and_create_files()\n");

    // Create .gpsdrive dir if not exist
    g_snprintf (file_path, sizeof (file_path),"%s",local_config.dir_home);
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

    // Create tracks/ Directory if not exist
    g_strlcpy (file_path, local_config.dir_tracks, sizeof (file_path)); 
    if(stat(file_path,&buf))
	{
	    if ( mkdir (file_path, 0700) )
		{
		    printf("Error creating %s\n",file_path);
		} else {
		    printf("created %s\n",file_path);
		}
	}

    // Create routes/ Directory if not exist
    g_strlcpy (file_path, local_config.dir_routes, sizeof (file_path)); 
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
 */
void
display_status (char *message)
{
    gchar tok[200];
    
    if ( mydebug >20 ) 
	fprintf(stderr , "display_status(%s)\n",message);
    
    if (downloadactive)
	return;
    if (current.importactive)
	return;
    if (statuslock)
	return;
    g_snprintf (tok, sizeof (tok), "%s", message);
    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, tok);
}


/* *****************************************************************************
 */
gint
lightoff (GtkWidget * widget, guint * datum)
{
	disableisnight = FALSE;
	/* gtk_widget_modify_bg (main_window, GTK_STATE_NORMAL, &colors.nightmode); */
	return FALSE;
}

/* *****************************************************************************
 */
gint
lighton (void)
{
	disableisnight = TRUE;
	/*   nighttimer = gtk_timeout_add (30000, (GtkFunction) lightoff, 0); */

	/*   gtk_widget_restore_default_style(main_window); */
	return TRUE;
}


/* *****************************************************************************
 * display upper status line 
 * speak how long it takes till we reach our destination
 */
void
display_status2 ()
{
	//gchar s2[100];
	gchar buf[200];
	gint h, m;
	gdouble secs, v;

	if ( mydebug >50 ) fprintf(stderr , "display_status2()\n");

	if (downloadactive)
		return;
	if (current.importactive)
		return;
	secs = g_timer_elapsed (disttimer, 0);
	h = hours;
	m = minutes;
	if (secs >= 300.0)
	{
		g_timer_stop (disttimer);
		g_timer_start (disttimer);
		v = 3600.0 * (olddist - current.dist) / secs;
		h = current.dist / v;
		m = 60 * (current.dist / v - h);
		if (mydebug)
			g_print ("secs: %.0fs,v:%.0f,h:%d,m:%d\n", secs, v, h,
				 m);
		if ((m < 0) || (h > 99))
		{
			h = 99;
			m = 99;
		}
		olddist = current.dist;
		hours = h;
		minutes = m;


    if( !local_config.mute )
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


	if ( mydebug > 10 )
	    {
		if (current.gpsfix > 1)
		    g_print ("***Position: %f %f***\n", coords.current_lat,
			     coords.current_lon);
		else
		    g_print ("***no valid Position:\n");
	    }



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

	if (current.importactive)
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

	if (gui_status.posmode)
	{
		coords.current_lon = coords.posmode_lon;
		coords.current_lat = coords.posmode_lat;
	}
	else
		update_route ();

	/* we test if we have to load a new map because we are outside 
	 * the currently loaded map 
	 */
	test_and_load_newmap ();

	/*   g_print("drawmarker_cb %d\n",drawmarkercounter++); */
	exposed = FALSE;
	/* The position calculation is made in expose_cb() */
	//gtk_widget_draw (map_drawingarea, NULL);	/* this  calls expose handler */

	if (!exposed)
		expose_cb (NULL, 0);
		
	gtk_widget_queue_draw_area (drawing_compass, 0,0,100,100);
	//expose_compass (NULL, 0);

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
		expose_gpsfix (NULL, 0);
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

	if (current.needtosave)
	{
		writeconfig ();
		if (route.items && !route.active)
			route_export_cb (NULL, TRUE);
	}

	if (local_config.MapnikStatusInt < 2)
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
	if ( useDBUS && !current.simmode )
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
	/*    g_print("Havepos: %d\n", current.gpsfix); */
	if ((!current.simmode && current.gpsfix < 2) || gui_status.posmode /*  ||((!local_config.simmode &&haveposcount<3)) */ )	/* we have no valid position */
	{
		(trackcoord + trackcoordnr)->lon = 1001.0;
		(trackcoord + trackcoordnr)->lat = 1001.0;
		(trackcoord + trackcoordnr)->alt = 1001.0;
	}
	else
	{
		(trackcoord + trackcoordnr)->lon = coords.current_lon;
		(trackcoord + trackcoordnr)->lat = coords.current_lat;
		(trackcoord + trackcoordnr)->alt = current.altitude;
		if (local_config.savetrack) do_incremental_save();
	}


	/*  The double storage seems to be silly, but I have to use  */
	/*  gdk_draw_segments instead of gdk_draw_lines.   */
	/*  gkd_draw_lines is dog slow because of a gdk-bug. */

	if (tracknr == 0)
	{
		if ((trackcoord + trackcoordnr)->lon < 1000.0)
		{
			(track + tracknr)->x1 = current.pos_x;
			(track + tracknr)->y1 = current.pos_y;
			(trackshadow + tracknr)->x1 = current.pos_x + SHADOWOFFSET;
			(trackshadow + tracknr)->y1 = current.pos_y + SHADOWOFFSET;
			tracknr++;
		}
	}
	else
	{
		if ((trackcoord + trackcoordnr)->lon < 1000.0)
		{
			if ((current.pos_x != (track + tracknr - 1)->x2)
			    || (current.pos_y != (track + tracknr - 1)->y2))
			{
				/* so=(int)(((trackcoord + trackcoordnr)->alt))>>5; */
				so = SHADOWOFFSET;
				(track + tracknr)->x1 =
					(track + tracknr - 1)->x2 = current.pos_x;
				(track + tracknr)->y1 =
					(track + tracknr - 1)->y2 = current.pos_y;
				(trackshadow + tracknr)->x1 =
					(trackshadow + tracknr - 1)->x2 =
					current.pos_x + so;
				(trackshadow + tracknr)->y1 =
					(trackshadow + tracknr - 1)->y2 =
					current.pos_y + so;
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
	gchar t[10], t1[20], buf[300], txt[10];
	static GdkPixbufAnimation *anim = NULL;
	static GdkPixbufAnimationIter *animiter = NULL;
	GTimeVal timeresult;
#define SATX 5
	/*  draw satellite level (field strength) only in NMEA modus */
return TRUE;
	//if ( mydebug >50 )
	fprintf(stderr , "expose_sats_cb()\n");


	if (haveNMEA)
	{
		gdk_gc_set_foreground (kontext_gps, &colors.lcd);

		gdk_draw_rectangle (drawable_gps, kontext_gps, 1, 3, 0,
				    PSIZE + 2, PSIZE + 5);
		gdk_gc_set_line_attributes (kontext_gps, 1, 0, 0, 0);
		gdk_gc_set_foreground (kontext_gps, &colors.black);
		gdk_draw_rectangle (drawable_gps, kontext_gps, 0, 3, 0,
				    PSIZE + 2, PSIZE + 5);

		gdk_gc_set_foreground (kontext_gps, &colors.lcd);

		if (!satposmode)
		{
			if (current.gpsfix > 1)
				gdk_gc_set_foreground (kontext_gps, &colors.green);
			else
				gdk_gc_set_foreground (kontext_gps, &colors.red);
			gdk_gc_set_line_attributes (kontext_gps, 2, 0, 0, 0);
			for (i = 3; i < (PSIZE + 5); i += 3)
				gdk_draw_line (drawable_gps, kontext_gps, 4, i,
					       PSIZE + 4, i);


			gdk_gc_set_foreground (kontext_gps, &colors.lcd2);
			gdk_gc_set_line_attributes (kontext_gps, 5, 0, 0, 0);
			k = 0;
			for (i = 0; i < 16; i++)
			{
				if (i > 5)
					yabs = 2 + PSIZE;
				else
					yabs = 1 + PSIZE / 2;
				h = PSIZE / 2 - 2;
				gdk_draw_line (drawable_gps, kontext_gps,
					       6 + 7 * k + SATX, yabs,
					       6 + 7 * k + SATX, yabs - h);
				k++;
				if (k > 5)
					k = 0;
			}
		}
		if (satfix == 1)	/* normal Fix */
			gdk_gc_set_foreground (kontext_gps, &colors.black);
		else
		{
			if (satfix == 0)	/* no Fix */
				gdk_gc_set_foreground (kontext_gps, &colors.textback);
			else	/* Differntial Fix */
				gdk_gc_set_foreground (kontext_gps, &colors.blue);
		}
		j = k = 0;

		if (satposmode)
		{
			gdk_gc_set_line_attributes (kontext_gps, 1, 0, 0, 0);
			gdk_gc_set_foreground (kontext_gps, &colors.lcd2);
			gdk_draw_arc (drawable_gps, kontext_gps, 0, 4,
				      4, PSIZE, PSIZE, 105 * 64, 330 * 64);
			gdk_draw_arc (drawable_gps, kontext_gps, 0,
				      5 + PSIZE / 4, 4 + PSIZE / 4, PSIZE / 2,
				      PSIZE / 2, 0, 360 * 64);
			gdk_gc_set_foreground (kontext_gps, &colors.darkgrey);
			{
				/* prints in pango */
				PangoFontDescription *pfd;
				PangoLayout *wplabellayout;
				gint width;

				wplabellayout =
					gtk_widget_create_pango_layout
					(drawing_gps, "N");
				//KCFX  
				if (local_config.guimode == GUI_PDA)
					pfd = pango_font_description_from_string ("Sans 7");
				else
				if (local_config.guimode == GUI_CAR)
					pfd = pango_font_description_from_string ("Sans 7");
				else
					pfd = pango_font_description_from_string ("Sans bold 10");
				pango_layout_set_font_description
					(wplabellayout, pfd);

				gdk_draw_layout_with_colors (drawable_gps,
							     kontext_gps,
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

			/*    gdk_draw_text (drawable_gps, font_verysmalltext, kontext_gps,
			 *                   2 + (PSIZE) / 2, 9, "N", 1);
			 */
			gdk_gc_set_foreground (kontext_gps, &colors.lcd2);

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
							(kontext_gps, &colors.textback);
						break;
					case 2:
					case 3:
						gdk_gc_set_foreground
							(kontext_gps, &colors.red);
						break;
					case 4:
					case 5:
					case 6:
						gdk_gc_set_foreground
							(kontext_gps, &colors.yellow);
						break;
					case 7:
						gdk_gc_set_foreground
							(kontext_gps, &colors.green2);
						break;
					}
					gdk_draw_arc (drawable_gps, kontext_gps,
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
					gdk_draw_line (drawable_gps, kontext_gps,
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
		g_snprintf (txt, sizeof (txt), "%d/%d", sats_used, sats_in_view);
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
      if( !local_config.mute && local_config.sound_gps )
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
		gdk_gc_set_function (kontext_gps, GDK_AND);
		gdk_draw_pixbuf (drawable_gps, kontext_gps, satsimage, 0, 0,
				 SATX, 0, 50, 50, GDK_RGB_DITHER_NONE, 0, 0);
		gdk_gc_set_function (kontext_gps, GDK_COPY);
	}
	return TRUE;
}


/*
 * Draw the scale bar ( |-------------| ) into the map. Also add a textual
 * description of the currently used scale.
 */
void
draw_scalebar (void)
{
	gchar scale_txt[100];
	gdouble factor[] = { 1.0, 2.5, 5.0, };
	gint exponent = 0;
	gdouble remains = 0.0;
	gint used_factor = 0;
	gint i;
	gint min_bar_length = gui_status.mapview_x / 8; /* 1/8 of the display width */
	const gint dist_x = 20; /* distance to the right */
	const gint dist_y = 20; /* distance to bottom */
	const gint frame_width = 5; /* grey pixles around the scale bar */
	gint bar_length;
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
	approx_displayed_value /* m */ = min_bar_length * current.mapscale
					 / PIXELFACT / current.zoom;

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

		log_value = log10 (min_bar_length * current.mapscale / PIXELFACT
				   / conversion / current.zoom / factor[i]);

		if ((rest = log_value - floor (log_value)) >= remains) {
			remains = rest;
			used_factor = i;
			exponent = (gint) floor (log_value) + 1;
		}
	}
	bar_length = factor[used_factor] * pow (10.0, exponent)
		     * conversion / (current.mapscale / PIXELFACT) * current.zoom;

	g_snprintf (scale_txt, sizeof (scale_txt), format, factor[used_factor]
					       * pow (10.0, exponent),
					       symbol);

	/* Now bar_length is the length of the scaler-bar in pixel
	 * and scale_txt is the text for the scaler Bar. For example "10 Km"
	 */

	l = (gui_status.mapview_x - 40) - (strlen (scale_txt) * 15);

	/* Draw greyish rectangle as background for the scale bar */
	gdk_gc_set_function (kontext_map, GDK_OR);
	gdk_gc_set_foreground (kontext_map, &colors.mygray);
	//gdk_gc_set_foreground (kontext_map, &textback);
	gdk_draw_rectangle (drawable, kontext_map, 1,
			    gui_status.mapview_x - dist_x - bar_length - frame_width,
			    gui_status.mapview_y - dist_y - 2 * frame_width,
			    bar_length + 2 * frame_width, 
			    dist_y + 1 * frame_width);
	gdk_gc_set_function (kontext_map, GDK_COPY);
	gdk_gc_set_foreground (kontext_map, &colors.black);

	/* Print the meaning of the scale bar ("10 km") */
	PangoLayout *scalebar_layout;
	PangoFontDescription *pfd_scalebar;

	if (local_config.guimode == GUI_PDA)
		pfd_scalebar = pango_font_description_from_string ("Sans 8");
	else
		pfd_scalebar = pango_font_description_from_string ("Sans 11");

	scalebar_layout = gtk_widget_create_pango_layout
		(map_drawingarea, scale_txt);
	pango_layout_set_font_description (scalebar_layout, pfd_scalebar);

	gdk_draw_layout_with_colors (drawable, kontext_map, 
		l, gui_status.mapview_y - dist_y -1 ,
		scalebar_layout, &colors.black, NULL);
	if (scalebar_layout != NULL)
		g_object_unref (G_OBJECT (scalebar_layout));

	/* Print the actual scale bar lines */
	gdk_gc_set_line_attributes (kontext_map, 2, 0, 0, 0);
	/* horizontal */
	gdk_draw_line (drawable, kontext_map,
		       (gui_status.mapview_x - dist_x) - bar_length, gui_status.mapview_y - dist_y,
		       (gui_status.mapview_x - dist_x), gui_status.mapview_y - dist_y );
	/* left */
	gdk_draw_line (drawable, kontext_map,
		       (gui_status.mapview_x - dist_x) - bar_length, gui_status.mapview_y - dist_y - frame_width,
		       (gui_status.mapview_x - dist_x) - bar_length, gui_status.mapview_y - dist_y + 10- frame_width);
	/* right */
	gdk_draw_line (drawable, kontext_map,
		       (gui_status.mapview_x - dist_x), gui_status.mapview_y - dist_y - frame_width,
		       (gui_status.mapview_x - dist_x), gui_status.mapview_y - dist_y + 10 - frame_width);

}

/*******************************************************************************
 * Draw the zoom level into the map.
 */
void
draw_zoomlevel (void)
{
	gchar zoom_scale_txt[5];

	/* draw zoom factor */
	g_snprintf (zoom_scale_txt, sizeof (zoom_scale_txt),
		"%dx", current.zoom);
	gdk_gc_set_function (kontext_map, GDK_OR);
	gdk_gc_set_foreground (kontext_map, &colors.mygray);
	gdk_draw_rectangle (drawable, kontext_map, 1, (gui_status.mapview_x - 30), 0, 30, 30);
	gdk_gc_set_function (kontext_map, GDK_COPY);

	gdk_gc_set_foreground (kontext_map, &colors.blue);

	/* prints in pango */
	PangoFontDescription *pfd;
	PangoLayout *layout_zoom;
	gint cx, cy;
	layout_zoom = gtk_widget_create_pango_layout
		(map_drawingarea, zoom_scale_txt);
	pfd = pango_font_description_from_string ("Sans 9");
	pango_layout_set_font_description (layout_zoom, pfd);
	pango_layout_get_pixel_size (layout_zoom, &cx, &cy);
	gdk_draw_layout_with_colors (drawable, kontext_map,
		gui_status.mapview_x-15-cx/2, 15-cy/2, layout_zoom, &colors.blue, NULL);
	if (layout_zoom != NULL)
		g_object_unref (G_OBJECT (layout_zoom));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);
}


/*******************************************************************************
 * Draw the filename of the current map into the map.
 */
void
draw_infotext (gchar *text)
{
	gchar  mfmap_filename[60];
	PangoFontDescription *pfd;
	PangoLayout *layout_filename;
	gint cx, cy;

	strncpy (mfmap_filename, text, 59);
	mfmap_filename[59] = 0;
	layout_filename = gtk_widget_create_pango_layout
		(map_drawingarea, mfmap_filename);
	pfd = pango_font_description_from_string ("Sans 9");
	pango_layout_set_font_description (layout_filename, pfd);
	pango_layout_get_pixel_size (layout_filename, &cx, &cy);

	gdk_gc_set_function (kontext_map, GDK_OR);
	gdk_gc_set_foreground (kontext_map, &colors.mygray);
	gdk_draw_rectangle (drawable, kontext_map, 1,
		0, gui_status.mapview_y-cy-10, cx+10, gui_status.mapview_y);
	gdk_gc_set_function (kontext_map, GDK_COPY);
	gdk_gc_set_foreground (kontext_map, &colors.blue);

	gdk_draw_layout_with_colors (drawable, kontext_map,
		5, gui_status.mapview_y-cy-5, layout_filename, &colors.blue, NULL);
	if (layout_filename != NULL)
		g_object_unref (G_OBJECT (layout_filename));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);
}


/* *****************************************************************************
 * draw the markers on the map 
 * And many more 
 * TODO: sort out
 */
gint
drawmarker (GtkWidget * widget, guint * datum)
{
	gint posxdest, posydest, posxmarker, posymarker;
	gint k;

	gblink = !gblink;
	/*    g_print("simmode: %d, nmea %d\n", current.simmode,haveNMEA); */

	if (current.importactive)
	    return TRUE;

	if (local_config.showgrid)
		draw_grid (widget);

	if (usesql)
	{
	    poi_draw_list (FALSE);
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

	if (local_config.showscalebar)
		draw_scalebar ();

	if (local_config.showzoom && local_config.guimode != GUI_PDA)
		draw_zoomlevel ();

	if (mydebug > 10 && !local_config.MapnikStatusInt)
		draw_infotext (g_path_get_basename (mapfilename));

	if (havekismet && (kismetsock>=0))
	{
		gdk_draw_pixbuf (drawable, kontext_map, kismetpixbuf, 0, 0,
			10, gui_status.mapview_y - 42,
			36, 20, GDK_RGB_DITHER_NONE, 0, 0);
	}
	
	if (local_config.savetrack)
	{
		k = 100;
		gdk_gc_set_foreground (kontext_map, &colors.white);
		gdk_draw_rectangle (drawable, kontext_map, 1, 10,
				    gui_status.mapview_y - 21, k + 3, 14);
		gdk_gc_set_foreground (kontext_map, &colors.red);
		{
			/* prints in pango */
			PangoFontDescription *pfd;
			PangoLayout *wplabellayout;

			wplabellayout =
				gtk_widget_create_pango_layout (map_drawingarea,
								savetrackfn);
			//KCFX  
			if (local_config.guimode == GUI_PDA)
				pfd = pango_font_description_from_string
					("Sans 7");
			else
			if (local_config.guimode == GUI_CAR)
				pfd = pango_font_description_from_string
					("Sans 7");
			else
				pfd = pango_font_description_from_string
					("Sans 10");
			pango_layout_set_font_description (wplabellayout,
							   pfd);

			gdk_draw_layout_with_colors (drawable, kontext_map,
						     14, gui_status.mapview_y - 22,
						     wplabellayout, &colors.red,
						     NULL);
			if (wplabellayout != NULL)
				g_object_unref (G_OBJECT (wplabellayout));
			/* freeing PangoFontDescription, cause it has been
			    copied by prev. call */
			pango_font_description_free (pfd);
		}


		/*    gdk_draw_text (drawable, smallfont_s_text, kontext_map,
		 *                   11, gui_status.mapview_y - 10, savetrackfn,
		 *                   strlen (savetrackfn));
		 */

		/*      gdk_draw_text (drawable, font_s_text, kontext_map, 10, */
		/*                     gui_status.mapview_y - 10, savetrackfn, strlen (savetrackfn)); */
	}

	if (gui_status.posmode)
	{
	    blink = TRUE;
	}

	if (current.gpsfix > 1 || blink)
	{
		if (gui_status.posmode)
		{
			gdk_gc_set_foreground (kontext_map, &colors.blue);
			gdk_gc_set_line_attributes (kontext_map, 4, 0, 0, 0);
			gdk_draw_rectangle (drawable, kontext_map, 0,
				current.pos_x - 10, current.pos_y - 10, 20, 20);
		}
		else
		{
			if (local_config.showshadow)
			{
				draw_posmarker (
					current.pos_x + SHADOWOFFSET,
					current.pos_y + SHADOWOFFSET,
					current.heading, &colors.darkgrey,
					local_config.posmarker, TRUE, FALSE);
			}

			/*  draw pointer to destination */
			if (local_config.posmarker == 0)
			{
				draw_posmarker (current.pos_x, current.pos_y, current.bearing,
					&colors.red, 1, FALSE, FALSE);
			}
			else
			{
				draw_posmarker (current.pos_x, current.pos_y, current.bearing,
					&colors.red, local_config.posmarker, FALSE, FALSE);
			}

			/*  draw pointer to direction of motion */
			draw_posmarker (current.pos_x, current.pos_y, current.heading,
				&colors.black, local_config.posmarker,
				FALSE, TRUE);
		}
		if (markwaypoint)
		{
			calcxy (&posxmarker, &posymarker,
				coords.wp_lon, coords.wp_lat, current.zoom);

			gdk_gc_set_foreground (kontext_map, &colors.green);
			gdk_gc_set_line_attributes (kontext_map, 5, 0, 0, 0);
			gdk_draw_arc (drawable, kontext_map, 0, posxmarker - 10,
				      posymarker - 10, 20, 20, 0, 360 * 64);
		}
		/*  If we are in position mode we set direction to zero to see where is the  */
		/*  target  */
		if (gui_status.posmode)
			current.heading = 0.0;

		display_status2 ();

	}


	/*  draw + sign at destination */
	calcxy (&posxdest, &posydest, coords.target_lon,
	coords.target_lat, current.zoom);
	if (local_config.showshadow)
	{
		/*  draw + sign at destination */
		draw_posmarker (
			posxdest + SHADOWOFFSET, posydest + SHADOWOFFSET,
			0, &colors.darkgrey, 3, TRUE, FALSE);
	}
	if (crosstoggle)
		draw_posmarker (posxdest, posydest, 0, &colors.blue, 3, FALSE, FALSE);
	else
		draw_posmarker (posxdest, posydest, 0, &colors.red, 3, FALSE, FALSE);
	crosstoggle = !crosstoggle;


	/* display messages on map */
	display_dsc ();
	
	// TODO: move all status updates to the update function...
	update_statusdisplay ();
	



/*			if (local_config.normalnull == 0.0)

				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
					    local_config.color_dashboard, s2, local_config.color_dashboard, s2a);
			else
				g_snprintf (s3, sizeof (s3),
					    "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"15000\">%s</span><span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>"
					    "<span color=\"red\" font_family=\"Arial\" weight=\"bold\" size=\"8000\">\nNN %+.1f</span>",
					    local_config.color_dashboard, s2, local_config.color_dashboard, s2a,
					    local_config.normalnull);

*/
	
	if (current.simmode)
		blink = TRUE;
	else
	{
		if (current.gpsfix < 2)
			blink = !blink;
	}

	if (newsatslevel)
		expose_gpsfix (NULL, 0);

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
	if (mydebug >50)
	fprintf (stdout, "expose_mini_cb()\n");

	/*  draw the minimap */
	if (!pixbuf_minimap)
		return TRUE;
	if (local_config.guimode == GUI_CAR)
		return TRUE;
	

	drawable_minimap = drawing_minimap->window;
	if (!drawable_minimap) {
		return 0;
	}
	kontext_minimap = gdk_gc_new (drawable_minimap);

	if (SMALLMENU == 0)
	{
		gdk_draw_pixbuf (drawing_minimap->window,
			kontext_minimap, pixbuf_minimap, 0, 0, 0, 0, 128, 103,
			GDK_RGB_DITHER_NONE, 0, 0);

		/*       if (local_config.nightmode && (isnight&& !disableisnight)) */
		/*  { */
		/*    gdk_gc_set_function (kontext_minimap, GDK_AND); */
		/*    gdk_gc_set_foreground (kontext_minimap, &colors.nightmode); */
		/*    gdk_draw_rectangle (drawing_minimap->window, kontext_minimap, 1, 0, 0, 128, */
		/*                        103); */
		/*    gdk_gc_set_function (kontext_minimap, GDK_COPY); */
		/*  } */
		gdk_gc_set_foreground (kontext_minimap, &colors.red);
		gdk_gc_set_line_attributes (kontext_minimap, 1, 0, 0, 0);

		gdk_draw_rectangle (drawing_minimap->window,
			kontext_minimap, 0,
			(64 - (MAP_X_2 / 10) / current.zoom) +
			xoff / (current.zoom * 10),
			(50 - (MAP_Y_2 / 10) / current.zoom) +
			yoff / (current.zoom * 10),
			gui_status.mapview_x / (current.zoom * 10),
			gui_status.mapview_y / (current.zoom * 10));
		drawdownloadrectangle (0);
	}
	return TRUE;
}


/* *****************************************************************************
 */
gint
expose_compass (GtkWidget *widget, guint *datum)
{
	gdouble t_x1off, t_y1off, t_x2off, t_y2off;
	PangoFontDescription *pfd_compass;
	PangoLayout *layout_compass;
	gint i, cx, cy, nx, ny, sx, sy, wx, wy, ex, ey;
	GdkPoint poly[16];
	gdouble w, b;
	/* This is the size fo the compass in pixels. Mabye this should
	 * depend on the frame size or the gui mode */
	const gint size = 100;
	const gint diam = size/2-12;

	if (mydebug >50)
		fprintf (stdout, "expose_compass()\n");

	drawable_compass = drawing_compass->window;
	kontext_compass = gdk_gc_new (drawable_compass);
	pfd_compass = pango_font_description_from_string ("Sans 8");

	gdk_gc_set_foreground (kontext_compass, &colors.darkgrey);
	gdk_gc_set_line_attributes (kontext_compass, 1, 0, 0, 0);
	gdk_draw_arc (drawable_compass, kontext_compass, TRUE,
		12, 12, size-24, size-24, 0, 360 * 64);
	gdk_gc_set_foreground (kontext_compass, &colors.black);
	gdk_draw_arc (drawable_compass, kontext_compass, FALSE,
		12, 12, size-24, size-24, 0, 360 * 64);
	gdk_draw_arc (drawable_compass, kontext_compass, FALSE,
		6+size/4, 6+size/4, size/2-12, size/2-12, 0, 360 * 64);

	w = - current.heading;
	
	t_x1off = cos(w + M_PI_2);
	t_y1off = sin(w + M_PI_2);
	t_x2off = cos(w);
	t_y2off = sin(w);
	if (t_x1off < 0)
	{
		nx = 0; sx = -1;
		wy = 1; ey = 0;
	}
	else
	{
		nx = 1; sx = 0;
		wy = 0; ey = -1;
	}
	if (t_y1off < 0)
	{
		ny = 0; sy = -1;
		wx = 0; ex = -1;
	}
	else
	{
		ny = 1; sy = 0;
		wx = 1; ex = 0;
	}

	/* draw compass cross and scale */
	for (i=0; i<4; i++)
	{
		gdk_draw_line (drawable_compass, kontext_compass,
			size/2+0.8*diam*cos(w+M_PI/4+i*M_PI_2),
			size/2+0.8*diam*sin(w+M_PI/4+i*M_PI_2),
			size/2+1.0*diam*cos(w+M_PI/4+i*M_PI_2),
			size/2+1.0*diam*sin(w+M_PI/4+i*M_PI_2));
	}
	gdk_draw_line (drawable_compass, kontext_compass,
		size/2-diam*t_x2off, size/2-diam*t_y2off,
		size/2+diam*t_x2off, size/2+diam*t_y2off);
	gdk_draw_line (drawable_compass, kontext_compass,
		size/2, size/2,
		size/2+diam*t_x1off, size/2+diam*t_y1off);
	gdk_gc_set_foreground (kontext_compass, &colors.green);
	gdk_gc_set_line_attributes (kontext_compass, 2, 0, 0, 0);
	gdk_draw_line (drawable_compass, kontext_compass,
		size/2, size/2,
		size/2-diam*t_x1off, size/2-diam*t_y1off);

	/* draw direction labels */
	layout_compass = gtk_widget_create_pango_layout
		(drawing_compass, _("N"));
	pango_layout_set_font_description (layout_compass, pfd_compass);
	pango_layout_get_pixel_size (layout_compass, &cx, &cy);
	gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
		size/2-diam*t_x1off-cx*nx, size/2-diam*t_y1off-cy*ny,
		layout_compass, &colors.red, NULL);

	pango_layout_set_text (layout_compass, _("S"), -1);
	pango_layout_get_pixel_size (layout_compass, &cx, &cy);
	gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
		size/2+diam*t_x1off+cx*sx, size/2+diam*t_y1off+cy*sy,
		layout_compass, &colors.red, NULL);

	pango_layout_set_text (layout_compass, _("W"), -1);
	pango_layout_get_pixel_size (layout_compass, &cx, &cy);
	gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
		size/2-diam*t_x2off-cx*wx, size/2-diam*t_y2off-cx*wy,
		layout_compass, &colors.red, NULL);

	pango_layout_set_text (layout_compass, _("E"), -1);
	pango_layout_get_pixel_size (layout_compass, &cx, &cy);
	gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
		size/2+diam*t_x2off+cx*ex, size/2+diam*t_y2off+cy*ey,
		layout_compass, &colors.red, NULL);

	if (layout_compass != NULL)
		g_object_unref (G_OBJECT (layout_compass));
	pango_font_description_free (pfd_compass);

	/* draw heading pointer */
	gdk_gc_set_foreground (kontext_compass, &colors.black);
	poly[0].x = size/2;
	poly[0].y = size/2-1.1*diam;
	poly[1].x = size/2+0.3*diam;
	poly[1].y = size/2+0.8*diam;
	poly[2].x = size/2;
	poly[2].y = size/2+0.6*diam;
	poly[3].x = size/2-0.3*diam;
	poly[3].y = size/2+0.8*diam;
	poly[4].x = poly[0].x;
	poly[4].y = poly[0].y;
	gdk_draw_polygon (drawable_compass, kontext_compass, FALSE, poly, 5);

	/* draw bearing pointer */
	b = current.bearing - current.heading;
	if (b > 2*M_PI)
		b -= 2*M_PI;
	gdk_gc_set_foreground (kontext_compass, &colors.red);
	poly[0].x = size/2-diam*cos(b + M_PI_2);
	poly[0].y = size/2-diam*sin(b + M_PI_2);
	poly[1].x = size/2
		- 0.2*diam*cos(b + M_PI) + 0.7*diam*cos(b + M_PI_2);
	poly[1].y = size/2
		- 0.2*diam*sin(b + M_PI) + 0.7*diam*sin(b + M_PI_2);
	poly[2].x = size/2+0.4*diam*cos(b + M_PI_2);
	poly[2].y = size/2+0.4*diam*sin(b + M_PI_2);
	poly[3].x = size/2
		+0.2*diam*cos(b + M_PI) +0.7*diam*cos(b + M_PI_2);
	poly[3].y = size/2
		+0.2*diam*sin(b + M_PI) +0.7*diam*sin(b + M_PI_2);
	poly[4].x = poly[0].x;
	poly[4].y = poly[0].y;
	gdk_draw_polygon (drawable_compass, kontext_compass, TRUE, poly, 5);

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
	gchar name[40], *tn;

	if (mydebug >50) printf ("expose_cb()\n");

	/*    g_print("\nexpose_cb %d",exposecounter++);   */

	/*   fprintf (stderr, "lat: %f long: %f\n", coords.current_lat, coords.current_lon); */
	if (exposed && local_config.guimode == GUI_PDA)
		return TRUE;

	set_mapviewsize_cb ();

	errortextmode = FALSE;
	if (!current.importactive)
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
		calcxy (&current.pos_x, &current.pos_y, coords.current_lon,
			coords.current_lat, current.zoom);

		/*  do this because calcxy already substracted xoff and yoff */
		current.pos_x += xoff;
		current.pos_y += yoff;

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
		current.dist = calcdist (coords.target_lon, coords.target_lat);

		if ( display_background_map() ) 
		    {
			/*  correct the shift of the map */
			oldxoff = xoff;
			oldyoff = yoff;
			/* now we test if the marker fits into the map and set the shift of the 
			 * little map_x x map_y region in relation to the real 1280x1024 map 
			 */
			okcount = 0;
			do
			    {
				ok = TRUE;
				okcount++;
				x = current.pos_x - xoff;
				y = current.pos_y - yoff;
				
				if (x < borderlimit)
				    xoff -= 2 * borderlimit;
				if (x > (gui_status.mapview_x - borderlimit))
				    xoff += 2 * borderlimit;
				if (y < borderlimit)
				    yoff -= 2 * borderlimit;
				if (y > (gui_status.mapview_y - borderlimit))
				    yoff += 2 * borderlimit;

				if (x < borderlimit)
				    ok = FALSE;
				if (x > (gui_status.mapview_x - borderlimit))
				    ok = FALSE;
				if (y < borderlimit)
				    ok = FALSE;
				if (y > (gui_status.mapview_y - borderlimit))
				    ok = FALSE;
				if (okcount > 2000)
				    {
					g_print ("\nloop detected, please report!\n");
					ok = TRUE;
				    }
			    }
			while (!ok);

			xoffmax = (640 * current.zoom) - MAP_X_2;
			yoffmax = (512 * current.zoom) - MAP_Y_2;
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
				g_print ("x: %d  xoff: %d oldxoff: %d Zoom: %d xoffmax: %d\n", x, xoff, oldxoff, current.zoom, xoffmax);
				g_print ("y: %d  yoff: %d oldyoff: %d Zoom: %d yoffmax: %d\n", y, yoff, oldyoff, current.zoom, yoffmax);
			    }
			current.pos_x -= xoff;
			current.pos_y -= yoff;
		    }
	}


	/*  zoom from to 1280x1024 map to the map_x x map_y region */
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
					  640 - xoff - 640 * current.zoom,
					  512 - yoff - 512 * current.zoom,
					  current.zoom, current.zoom,
					  GDK_INTERP_BILINEAR);

			/*       image=gdk_pixbuf_scale_simple(tempimage,640 - xoff - 640 * current.zoom,
			 *                            512 - yoff - 512
			 *                            * current.zoom, 
			 *                            GDK_INTERP_BILINEAR);
			 */

		}

		if ( mydebug > 0 )
			g_print ("map zoomed!\n");
		iszoomed = TRUE;
		expose_mini_cb (NULL, 0);

	}

	gdk_draw_pixbuf (drawable, kontext_map, tempimage,
			 640 - MAP_X_2,
			 512 - MAP_Y_2, 0, 0,
			 gui_status.mapview_x, gui_status.mapview_y, GDK_RGB_DITHER_NONE, 0, 0);

	if ((!disableisnight) && (!downloadwindowactive))
	{
		if (local_config.nightmode && isnight)
		{
			gdk_gc_set_function (kontext_map, GDK_AND);
			gdk_gc_set_foreground (kontext_map, &colors.nightmode);
			gdk_draw_rectangle (drawable, kontext_map, 1, 0, 0,
					    gui_status.mapview_x, gui_status.mapview_y);
			gdk_gc_set_function (kontext_map, GDK_COPY);
		}
	}

	drawmarker (0, 0);

	gdk_draw_drawable (GTK_LAYOUT (map_drawingarea)->bin_window, kontext_map, drawable, 0,
			 0, 0, 0, gui_status.mapview_x, gui_status.mapview_y);
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

	if (!current.simmode)
		return TRUE;

	ACCELMAX = 0.00002 + current.dist / 30000.0;
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
 *  switching sat level/sat position display 
 */
gint
satpos_cb (GtkWidget * widget, guint datum)
{
	satposmode = !satposmode;
	current.needtosave = TRUE;
	expose_sats_cb (NULL, 0);
	return TRUE;
}


/* *****************************************************************************
 *  should I use DGPS-IP? 
 */
gint
usedgps_cb (GtkWidget * widget, guint datum)
{
	usedgps = !usedgps;
	current.needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 */
gint
earthmate_cb (GtkWidget * widget, guint datum)
{
	earthmate = !earthmate;
	current.needtosave = TRUE;
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
	
	set_cursor_style(CURSOR_DEFAULT);

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



/* *****************************************************************************
 */
gint
accepttext (GtkWidget * widget, gpointer data)
{
	GtkTextIter start, end;
	gchar *p;

	gtk_text_buffer_get_bounds (getmessagebuffer, &start, &end);

	gtk_text_buffer_apply_tag_by_name (getmessagebuffer, "word_wrap",
					   &start, &end);

	p = gtk_text_buffer_get_text (getmessagebuffer, &start, &end, FALSE);

	strncpy (messagesendtext, p, 300);
	messagesendtext[301] = 0;
	if ( mydebug > 8 )
		fprintf (stderr, "friends: message:\n%s\n", messagesendtext);
	gtk_widget_destroy (widget);
//	wi = gtk_item_factory_get_item (item_factory,
//					N_("/Menu/Messages"));
	statuslock = TRUE;
	gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
		current.statusbar_id,
		_("Sending message to friends server..."));
//	gtk_widget_set_sensitive (wi, FALSE);
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

	gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar),
			   current.statusbar_id);
	gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
			    current.statusbar_id, str);
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
				      GTK_WINDOW (main_window));

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

	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->vbox), vbox, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox), vpaned, TRUE, TRUE, 3);

	/*   gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3); */
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (window)->action_area),
			    hbox, TRUE, TRUE, 2);

	gtk_box_pack_start (GTK_BOX (hbox), ok, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (hbox), cancel, TRUE, TRUE, 3);
	view1 = gtk_text_view_new ();

	getmessagebuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view1));
	g_signal_connect (GTK_TEXT_BUFFER (getmessagebuffer),
			  "changed", G_CALLBACK (textstatus), frame_statusbar);

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
//	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), ok,
//			      _
//			      ("Sends your text to to selected computer using //the friends server"),
//			      NULL);

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
				      GTK_WINDOW (main_window));
	messagewindow = window;
	gtk_window_set_title (GTK_WINDOW (window),
			      _("Please select message recipient"));

	gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);

	if (local_config.guimode == GUI_PDA)
		gtk_window_set_default_size (GTK_WINDOW (window),
					     gui_status.mapview_x, gui_status.mapview_y);
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
				      GTK_WINDOW (main_window));

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
					     gui_status.mapview_x, gui_status.mapview_y);
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
	if (local_config.guimode == GUI_PDA ||
	    local_config.guimode == GUI_CAR )
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
    
    g_print ("%s%s%s%s%s%s%s%s"
#ifdef DBUS_ENABLE
	     "%s"
#endif
	     "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
	     "\nCopyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>"
	     "\n         Website: http://www.gpsdrive.de\n\n",
	     _("-v        show version\n"),
	     _("-h        print this help\n"),
	     _("-d        turn on debug info\n"),
	     _("-D X      set debugging level to X\n"),
	     _("-T        do some internal unit tests (don't start gpsdrive)\n"),
	     _("-e        use Festival-Lite (flite) for speech output\n"),
	     _("-o thing  serial device, pty master, or file to use for NMEA output\n"),
	     _("-f server select friends server, e.g. friendsd.gpsdrive.de\n"),
#ifdef DBUS_ENABLE
	     _("-X        use DBUS for communication with gpsd; this disables socket communication\n"),
#endif
	     _("-l lang.  select language of the voice output;\n"
	       "          language may be 'english', 'spanish', or 'german'\n"),
	     _("-g geom.  set window geometry, e.g. 800x600\n"),
	     _("-1        have only 1 button mouse, for example using touchscreen\n"),
	     _("-a        display APM stuff (battery status, temperature)\n"),
	     _("-b server servername for NMEA server (if gpsd runs on another host)\n"),
	     _("-c WP     set start position in simulation mode to this waypoint (WP)\n"),
	     _("-M mode   set GUI mode; mode may be 'desktop' (default), 'pda' or 'car'\n"),
	     _("-i        ignore NMEA checksum (risky, only for broken GPS receivers\n"),
	     _("-q        disable SQL support\n"),
	     _("-F        force display of position even it is invalid\n"),
	     _("-s        don't show splash screen\n"),
	     _("-S path   take auto screenshots of different window (don't touch gpsdrive!)\n"),
	     _("-P        start in Position Mode\n"),
	     _("-W X      switch WAAS/EGNOS on (X=1), or off (X=0)\n"),
	     _("-H ALT    correct the altitude by adding this value (ALT) to it\n"),
	     _("-C file   use this config file\n"));

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
				GTK_WINDOW (main_window),
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

/*
 * parse command arguments
 */
int
parse_cmd_args(int argc, char *argv[]) {
	int i = 0;
    /* parse cmd args */
    /* long options for use of --geometry and -g */
     int option_index = 0;
     static struct option long_options[] =
             {
              {"geometry", required_argument, 0, 'g'},
              {"config-file", required_argument, 0, 'C'},
              {"screenshot", required_argument, 0, 'S'},
              {0, 0, 0, 0}
             };
    do
	{
	    /* long options plus --geometry and -g */
            i = getopt_long (argc, argv,
			"W:ES:A:ab:c:X1qivPdD:TFepC:H:hnf:l:t:so:g:M:?",
			long_options, &option_index);
	    switch (i)
		{
	    case 'C':
	    	g_strlcpy(local_config.config_file, optarg, sizeof(local_config.config_file));
	    	if (!g_file_test(local_config.config_file, G_FILE_TEST_EXISTS)) {
				fprintf(stderr,"Config file '%s' not found.\n", local_config.config_file);
				exit(-1);
	    	}
		case 'a':
		    local_config.enableapm = TRUE;
		    break;
		case 's':
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
		case 'X':
#ifdef DBUS_ENABLE
		    useDBUS = TRUE;
#else
		    g_print ("\nWARNING: You need to enable DBUS support with './configure --enable-dbus'!\n");
#endif
		    break;
		case 'M':
			if (!strcmp(optarg, "desktop"))
				local_config.guimode = GUI_DESKTOP;
			else if (!strcmp(optarg, "car"))
				local_config.guimode = GUI_CAR;
			else if (!strcmp(optarg, "pda"))
				local_config.guimode = GUI_PDA;
			else {
				fprintf(stderr,"%s-mode not supported.\n", optarg);
				exit(-1);
			}
			break;
		case '1':
		    onemousebutton = TRUE;
		    break;
		case 'v':
		    printf ("\ngpsdrive (c) 2001-2006 Fritz Ganter <ganter@ganter.at>\n" "\nVersion %s\n%s\n\n", VERSION, rcsid);
		    exit (0);
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
				("\nYou can currently only choose between "
				"english, spanish and german\n\n"));
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
		    local_config.normalnull = strtol (optarg, NULL, 0);
		    break;
		case '?':
		    usage ();
		    exit (0);
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
		case 'S':
		    g_strlcpy (local_config.screenshot_dir, optarg, sizeof (local_config.screenshot_dir));
	    	if (!g_file_test(local_config.screenshot_dir, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
				fprintf(stderr,"Screenshot directory '%s' not found.\n", local_config.screenshot_dir);
				exit(-1);
	    	} else
	    		takescreenshots = TRUE;
		    break;
		}
	}
    while (i != -1);
    return 0;
}


/*******************************************************************************
 *                                                                             *
 *                             Main program                                    *
 *                                                                             *
 *******************************************************************************/
int
main (int argc, char *argv[])
{

    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "utf8");
    textdomain (GETTEXT_PACKAGE);

    gchar buf[500];

    current.needtosave = FALSE;

    gint i;

    struct tm *lt;
    time_t local_time, gmt_time;
    /*   GtkAccelGroup *accel_group; */

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
    /*    zero_lat and zero_lon are overwritten by config file,  */

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
    current.gpsfix = 0;
    gblink = blink = FALSE;
    haveposcount = debug = 0;
    current.heading = current.bearing = 0.0;
    current.zoom = 1;
    iszoomed = FALSE;
#ifdef DBUS_ENABLE
    useDBUS = FALSE;
#endif

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
    
    downloadwindowactive = downloadactive = current.importactive = FALSE;
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
	
	/* we need to parse command args 2 times, because we need the config file param */
	parse_cmd_args(argc, argv);
	
	/* update config struct with settings from config file if possible */
	readconfig ();

	test_loaded_map_names ();

	if (local_config.simmode == SIM_ON)
		current.simmode = TRUE;

    coords.target_lon = coords.current_lon + 0.00001;
    coords.target_lat = coords.current_lat + 0.00001;


    /*  load waypoints before locale is set! */
    /*  Attention! In this file the decimal point is always a '.' !! */

    /*  load mapfile configurations */
    /*  Attention! In this file the decimal point is that what locale says, 
     * i.e. '.' in english, ',' in german!! 
     */
    loadmapconfig ();

    /*    Setting locale for correct Umlauts */
    gtk_set_locale ();

    /*  initialization for GTK+ */
    gtk_init (&argc, &argv);

    /* 2. run see comment at first run */
    parse_cmd_args(argc, argv);

    if ( mydebug >99 )
	fprintf(stderr , "options parsed\n");

	/* print version info */
	if ( mydebug > 0 )
		printf ("\ngpsdrive (c) 2001-2006 Fritz Ganter"
		" <ganter@ganter.at>\n\nVersion %s\n%s\n\n", VERSION, rcsid);

	/*  show splash screen */
	if (!nosplash)
		show_splash ();

    init_lat2RadiusArray();

    gethostname (hostname, 256);
    proxyport = 80;
    haveproxy = FALSE;

    get_proxy_from_env();

    { // Set locale for the use of atof()  
	gchar buf[5];  
	sprintf(buf,"%.1f",1.2);  
	localedecimal=buf[1];  
    }  
  
    /* init sql support */
    if (usesql)
	usesql = sqlinit ();

    if (!useflite)
	havespeechout = speech_out_init ();
    else
	havespeechout = TRUE;

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


    /*    gtk_window_set_default (GTK_WINDOW (main_window), zoomin_bt); */
    /*    if we want NMEA mode, gpsd must be running and we connect to port 2222 */
    /*    An alternate gpsd server may be on 2947, we try it also */

    initgps ();

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




    // Frame --- Sat levels
    /* Area for field strength, we have data only in NMEA mode */
//    gtk_signal_connect (GTK_OBJECT (drawing_sats), "expose_event",
//			GTK_SIGNAL_FUNC (expose_sats_cb), NULL);
//    gtk_widget_add_events (GTK_WIDGET (drawing_sats),
//			   GDK_BUTTON_PRESS_MASK);
//    gtk_signal_connect (GTK_OBJECT (drawing_sats), "button-press-event",
//			GTK_SIGNAL_FUNC (satpos_cb), NULL);


    /*  all position calculations are made in the expose callback */
//    g_signal_connect (GTK_OBJECT (map_drawingarea),
//			"expose_event", GTK_SIGNAL_FUNC (expose_cb), NULL);




    //temperature_get_values ();
    //battery_get_values ();

    g_strlcpy (mapfilename, "***", sizeof (mapfilename));
    /*  set the timers */
	timerto =
	    gtk_timeout_add (TIMER,
			     (GtkFunction) get_position_data_cb,
			     NULL);
    gtk_timeout_add (WATCHWPTIMER, (GtkFunction) watchwp_cb, NULL);

    redrawtimeout =
	gtk_timeout_add (200, (GtkFunction) calldrawmarker_cb, NULL);

    /*  if we started in simulator mode we have a little move roboter */
    if (current.simmode)
	simpos_timeout =
	    gtk_timeout_add (300, (GtkFunction) simulated_pos, 0);
    if (nmeaout)
	gtk_timeout_add (1000, (GtkFunction) write_nmea_cb, NULL);
    gtk_timeout_add (600000, (GtkFunction) speech_saytime_cb, 0);
    gtk_timeout_add (1000, (GtkFunction) storetrack_cb, 0);
    gtk_timeout_add (TRIPMETERTIMEOUT*1000, (GtkFunction) update_tripdata_cb, 0);
    gtk_timeout_add (10000, (GtkFunction) masteragent_cb, 0);
    gtk_timeout_add (15000, (GtkFunction) getsqldata, 0);
//    if ( battery_get_values () )
//	gtk_timeout_add (5000, (GtkFunction) expose_display_battery,
//			 NULL);

//    if ( temperature_get_values () )
//	gtk_timeout_add (5000, (GtkFunction) expose_display_temperature,
//			 NULL);

    gtk_timeout_add (15000, (GtkFunction) friendsagent_cb, 0);

    current.needtosave = FALSE;

    if (havespeechout)
	speech_saytime_cb (NULL, 1);

	/* do all the basic initalisation for the specific sections */
	poi_init ();
	gui_init (geometry, usegeometry);
	friends_init ();
	route_init (NULL, NULL, NULL);

	wlan_init ();

	load_friends_icon ();
	load_kismet_icon ();

    update_posbt();

    if (havespeechout)
	{
		gtk_timeout_add (SPEECHOUTINTERVAL, (GtkFunction) speech_out_cb, 0);
		gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
			current.statusbar_id, _("Using speech output"));
	}

    /*
     * setup TERM signal handler so that we can save evrything nicely when the
     * machine is shutdown.
     */
    void termhandler (int sig)
	{
	    gtk_main_quit ();
	}
    signal (SIGTERM, termhandler);


    // Initialize Track Filename
    savetrackfile (0);

	trip_reset_cb ();

    // ==================================================================
    // Unit Tests
    if ( do_unit_test ) {
	unit_test();
    }

    if (takescreenshots) {
    	auto_take_screenshots();
    }
    /*  Mainloop */
    gtk_main ();


    g_timer_destroy (timer);
    if ( do_unit_test ) {
	coords.current_lat=48.000000;
	coords.current_lon=12.000000;
    }
    writeconfig ();
    gdk_pixbuf_unref (friendspixbuf);


    unlink ("/tmp/cammain.pid");
    unlink ("/tmp/gpsdrivetext.out");
    unlink ("/tmp/gpsdrivepos");
    if (local_config.savetrack)
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
