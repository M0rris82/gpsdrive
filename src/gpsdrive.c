/***********************************************************************

Copyright (c) 2001-2006 Fritz Ganter <ganter  ganter at>

Website: www.gpsdrive.de

Disclaimer: !!! Do not use as a primary source of navigation !!!

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

/**************************************************************************
   Contributors:
     <molter  gufi org>
     <wulf  netbsd org>
     Aart Koelewijn <aart  mtack.xs4all nl>
     Belgabor <belgabor  gmx de>
     Blake Swadling <blake  swadling com>
     Christoph Metz <loom  mopper de>
     Chuck Gantz <chuck.gantz  globalstar com>
     Dan Egnor <egnor  ofb net>
     Daniel Hiepler <rigid  akatash de>
     Darazs Attila <zumi  freestart hu>
     Fritz Ganter <ganter  ganter at>
     Guenther Meyer <d.s.e  sordidmusic com>
     Hamish Bowman <hamish_b  yahoo com>
     J.D. Schmidt <jdsmobile  gmail com>
     Jan-Benedict Glaw <jbglaw  lug-owl de>
     Joerg Ostertag <gpsdrive  ostertag name>
     John Hay <jhay  icomtek.csir.co za>
     Johnny Cache <johnycsh  hick org>
     Miguel Angelo Rozsas <miguel  rozsas.xx.nom br>
     Mike Auty
     Oddgeir Kvien <oddgeir  oddgeirkvien com>
     Oliver Kuehlert <Oliver.Kuehlert  mpi-hd.mpg de>
     Olli Salonen <olli  cabbala net>
     Philippe De Swert
     Richard Scheffenegger <rscheff  chello at>
     Rob Stewart <rob  groupboard com>
     Russell Harding <hardingr  billingside com>
     Russell Mirov <russell.mirov  sun com>
     Wilfried Hemp <Wilfried.Hemp  t-online de>
     <pdana  mail.utexas edu>
     <timecop  japan.co jp>
*/

/*  Include Dateien */
#include "config.h"
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
#include "gpsdrive_config.h"
/* #include <gpskismet.h> <-- prototypes are declared also in gpsproto.h */

#include <dlfcn.h>
#include <semaphore.h>

		
#include <locale.h>

#include "gettext.h"

#include <dirent.h>

#include "LatLong-UTMconversion.h"
#include "gpsdrive.h"
#include "battery.h"
#include "poi.h"
#include "track.h"
#include "waypoint.h"
#include "routes.h"
#include "gps_handler.h"
#include "nmea_handler.h"
#include "map_handler.h"

#include "import_map.h"
#include "map_download.h"
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
GtkWidget *gotowindow;
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

guint id_timeout_track;
guint id_timeout_autotracksave;

gdouble long_diff = 0, lat_diff = 0;
GdkGC *kontext_map;

extern GtkWidget *drawing_minimap;

// TODO: should be moved away...
extern GtkWidget  *drawing_compass;
extern GdkGC *kontext_compass, *kontext_gps, *kontext_minimap;
extern GdkDrawable *drawable_compass, *drawable_minimap;

GtkWidget *miles;
GdkDrawable *drawable;
gint haveposcount, blink, gblink, xoff, yoff, crosstoggle = 0;
gdouble pixelfact;
GdkPixbuf *image = NULL, *tempimage = NULL, *pixbuf_minimap = NULL;

extern GdkPixbuf *friendsimage, *friendspixbuf;

extern mapsstruct *maps;

extern GtkWidget *drawing_battery, *drawing_temp;

extern GtkWidget *map_drawingarea;

extern GtkListStore *friends_list;

wpstruct *wayp;

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
gint maxwp;
GtkStyle *style = NULL;
GtkRcStyle *mainstyle;
gint satlist[MAXSATS][4], satlistdisp[MAXSATS][4], satbit = 0;
GtkWidget *mylist;

extern gchar mapfilename[2048];

gdouble milesconv;
gdouble olddist = 99999.0;
GTimer *simulation_timer, *disttimer;
gint gcount;
gchar localedecimal;
gchar language[] = "en";

extern gboolean mapdl_active;

GtkWidget *add_wp_name_text, *wptext2;
gchar oldangle[100];

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


GtkWidget *label_map_filename;
GtkWidget *label_timedest;
GtkWidget *wp_bt;
GtkWidget *bestmap_bt, *poi_draw_bt, *wlan_draw_bt;

GtkWidget *savetrack_bt;
GtkWidget *loadtrack_bt;
GdkSegment *track, *trackshadow;
glong tracknr;
trackdata_struct *trackcoord;
extern glong trackcoordnr, tracklimit, trackcoordlimit,old_trackcoordnr;
gchar savetrackfn[256];

gint hours, minutes;
gint errortextmode = TRUE;

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
gchar gpsdservername[200], setpositionname[80];

gint havealtitude = FALSE;
gint usedgps = FALSE;
gchar dgpsserver[80], dgpsport[10];
GtkWidget *posbt;
extern gint PSIZE;
GdkPixbuf *batimage = NULL;
GdkPixbuf *temimage = NULL;
GdkPixbuf *satsimage = NULL;
gint sats_used = 0, sats_in_view = 0;
gint numgrids = 4, scroll = TRUE;
gchar utctime[20], loctime[20];
gint redrawtimeout;
gint borderlimit = 100;
gint pdamode = FALSE;
gint exposecounter = 0, exposed = FALSE;
gint lastnotebook = 0;
gint ignorechecksum = FALSE;

/* Give more debug informations */
gint mydebug = 0;

char wp_typelist[MAXPOITYPES][50];
double dbdistance;
gint dbusedist = FALSE;

extern GdkPixbuf *iconpixbuf[50];
gint earthmate = FALSE;

extern GdkPixbuf *posmarker_img;

extern routestatus_struct route;
extern color_struct colors;

GdkFont *font_wplabel;
gchar font_text[100];

gint drawmarkercounter = 0, loadpercent = 10, globruntime = 30;
extern int pleasepollme;


gint forcehavepos = FALSE;
extern gchar cputempstring[20], batstring[20];
extern GtkWidget *tempeventbox, *batteventbox;
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
//int mapistopo = FALSE;
int nosplash = FALSE;
int havedefaultmap = TRUE;

int storetz = FALSE;

// ---------------------- for nmea_handler.c
extern FILE *nmeaout;
/*  if we get data from gpsd in NMEA format haveNMEA is TRUE */
extern gint haveNMEA, useDBUS;
extern gint bigp , bigpGGA , bigpRME , bigpGSA, bigpGSV;
extern gint lastp, lastpGGA, lastpRME, lastpGSA, lastpGSV;

extern GtkWidget *main_window;
extern GtkWidget *frame_statusbar;
extern GtkWidget *main_table;

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
#ifdef _WIN32
    char *last_slash;
#endif

    if ( mydebug >5 ) fprintf(stderr , " check_and_create_files()\n");

    // Create .gpsdrive dir if not exist
    g_snprintf (file_path, sizeof (file_path),"%s",local_config.dir_home);
#ifdef _WIN32
    /* strip off trailing slash, stat on Win32 don't like it */
    last_slash = strrchr(file_path, '/');
    if(last_slash) {
        *last_slash = 0;
    }
#endif
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR))
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
#ifdef _WIN32
    /* strip off trailing slash, stat on Win32 don't like it */
    last_slash = strrchr(file_path, '/');
    if(last_slash) {
        *last_slash = 0;
    }
#endif
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR))
	{
	    if (g_mkdir (file_path, 0700))
		{
		    printf("Error creating %s\n",file_path);
		} else {
		    printf("created %s\n",file_path);
		}
	}

    // Create tracks/ Directory if not exist
    g_strlcpy (file_path, local_config.dir_tracks, sizeof (file_path)); 
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR))
	{
	    if (g_mkdir (file_path, 0700))
		{
		    printf("Error creating %s\n",file_path);
		} else {
		    printf("created %s\n",file_path);
		}
	}

    // Create routes/ Directory if not exist
    g_strlcpy (file_path, local_config.dir_routes, sizeof (file_path)); 
    if(!g_file_test (file_path, G_FILE_TEST_IS_DIR))
	{
	    if (g_mkdir (file_path, 0700))
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
    if ( !g_file_test (file_path, G_FILE_TEST_IS_REGULAR) ) {
	gchar copy_command[2048];
	g_snprintf (copy_command, sizeof (copy_command),
		    "cp %s/gpsdrive/map_koord.txt %s",
		    (gchar *) DATADIR,
		    file_path);
#ifdef _WIN32
    printf("Win32 port: Creation of map_koord.txt ommited for now!\n");
#else
    if ( system (copy_command))
	    {
		fprintf(stderr,"Error Creating %s\nwith command: '%s'\n",
			file_path,copy_command);
		exit(-1);
	    }
	else 
	    fprintf(stderr,"Created map_koord.txt %s\n",file_path);
#endif
    }

}


/* *****************************************************************************
 */
void
display_status (char *message)
{
    gchar tok[200];
    
    if ( mydebug >20 ) 
	fprintf(stderr , "display_status(%s)\n",message);
    
    if (mapdl_active)
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

	if (mapdl_active)
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

#ifdef SPEECH
    if( !local_config.mute )
    {
      if( hours < 99 )
      {
        if( hours > 0 )
        {
          if( 1 == hours )
          {
            g_snprintf(
              buf, sizeof(buf), _("Arrival in approximately one hour and %d minutes."),
              minutes );
          }
          else
          {
            g_snprintf(
              buf, sizeof(buf), _("Arrival in approximately %d hours and %d minutes."),
              hours, minutes );
          }
        }
        else
        {
          g_snprintf(
            buf, sizeof(buf), _("Arrival in approximately %d minutes."), minutes );
        }

        speech_saytext (buf, 3);
      }
    }
#endif
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
 * check for new map
 * TODO: eventually split this callback or rename it
 */
gint
drawmarker_cb (GtkWidget * widget, guint * datum)
{
	static GTimeVal tv1, tv2;
	long runtime, runtime2;

	if ( mydebug >50 ) fprintf(stderr , "drawmacker_cb()\n");

	if (current.importactive)
		return TRUE;

	g_get_current_time(&tv1);
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

	g_get_current_time(&tv2);
	runtime = tv2.tv_usec - tv1.tv_usec;
	if (tv2.tv_sec != tv1.tv_sec)
		runtime += 1000000l * (tv2.tv_sec - tv1.tv_sec);
	globruntime = runtime / 1000;
	loadpercent = (int) (100.0 * (float) runtime / (runtime + runtime2));
	if ( mydebug>30 )
		g_print ("Computation time: %dms, %d%%\n", (int) (runtime / 1000),
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
	period = globruntime / local_config.maxcpuload;
	drawmarkercounter++;
	/*   g_print("period: %d, drawmarkercounter %d\n", period, drawmarkercounter);  */
	

	if (local_config.MapnikStatusInt > 0 && drawmarkercounter >= 3) {
		drawmarkercounter = 0;
		drawmarker_cb (NULL, NULL);		
		return TRUE;
	}
	if ((drawmarkercounter > period) && (drawmarkercounter >= 3))
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
#ifndef _WIN32
	gchar buffer[200];
#endif

	if ( mydebug >50 ) fprintf(stderr , "masteragent_cb()\n");

	if (current.needtosave)
	{
		writeconfig ();
		if (route.items && !route.active)
		{
			gchar t_buf[500];
			g_snprintf (t_buf, sizeof (t_buf), "%sroutesaved.gpx", local_config.dir_home);
			gpx_file_write (t_buf, GPX_RTE);
		}
	}

	if (local_config.MapnikStatusInt < 2)
		map_koord_check_and_reload();

#ifndef _WIN32
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
#endif

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
			g_renew (trackdata_struct, trackcoord,
				 trackcoordlimit + 100000);
		trackcoordlimit += 100000;
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
	PangoLayout *scalebar_layout;
	PangoFontDescription *pfd_scalebar;


	/* scale bar is not valid in Plate carrée projection, distance 
	    changes with cos(lat) so scalebar is only  valid in the
	    y direction (where 60 nautical miles == 1 degree lat) */
 /* TODO: show a (valid) scalebar with units as "degrees" for top_* maps? */
	if( map_proj == proj_top )
	    return;

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

	/* text label x-coordinate */
	/* TODO: how to use pango_layout_set_alignment(center) instead of -strlen()*4 kludge? */
	l = ((gui_status.mapview_x - dist_x) - bar_length) + bar_length/2 // horiz center of bar
		- strlen(scale_txt)*4;

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
	if (local_config.guimode == GUI_PDA)
		pfd_scalebar = pango_font_description_from_string ("Sans 8");
	else
		pfd_scalebar = pango_font_description_from_string ("Sans 11");

	scalebar_layout = gtk_widget_create_pango_layout
		(map_drawingarea, scale_txt);

	pango_layout_set_font_description (scalebar_layout, pfd_scalebar);

// ???	pango_layout_set_alignment(scalebar_layout, PANGO_ALIGN_CENTER);

	gdk_draw_layout_with_colors (drawable, kontext_map, 
		l, gui_status.mapview_y - dist_y -1,
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

	pango_font_description_free (pfd_scalebar);
}

/*******************************************************************************
 * Draw the zoom level into the map.
 */
void
draw_zoomlevel (void)
{
	gchar zoom_scale_txt[5];
	PangoFontDescription *pfd;
	PangoLayout *layout_zoom;
	gint cx, cy;

	/* draw zoom factor */
	g_snprintf (zoom_scale_txt, sizeof (zoom_scale_txt),
		"%dx", current.zoom);
	gdk_gc_set_function (kontext_map, GDK_OR);
	gdk_gc_set_foreground (kontext_map, &colors.mygray);
	gdk_draw_rectangle (drawable, kontext_map, 1, (gui_status.mapview_x - 30), 0, 30, 30);
	gdk_gc_set_function (kontext_map, GDK_COPY);

	gdk_gc_set_foreground (kontext_map, &colors.blue);

	/* prints in pango */
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

	if (current.importactive)
	    return TRUE;

	if (local_config.showgrid)
		draw_grid (widget);

	if (local_config.use_database)
	{
	    poi_draw_list (FALSE);
	    if (route.active)
		route_display_targetinfo ();
	}

	if (local_config.showwaypoints)
		draw_waypoints ();

	drawtracks ();
	
	if (route.show)
		draw_route ();

	//if (current.kismetsock)
		current.kismetsock = readkismet ();

	if (local_config.showscalebar)
		draw_scalebar ();

	if (local_config.showzoom && local_config.guimode != GUI_PDA)
		draw_zoomlevel ();

	if (!local_config.MapnikStatusInt)
	{
		if (local_config.showmaptype)
			draw_maptype ();	
		if (mydebug > 10)
			draw_infotext (g_path_get_basename (mapfilename));
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

	if (local_config.showdestline)
		draw_destination_line ();

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

	if (gui_status.dl_window)
	{
		drawdownloadrectangle (1);
		expose_mini_cb (NULL, 0);
	}

#ifdef SPEECH
	/* force to say new direction */
	if (!strcmp (oldangle, "XXX"))
		speech_out_cb (NULL, 0);
#endif

	/* experimental output of current street data */
#ifdef MAPNIK
	if (local_config.showway)
	{
		street_struct t_street;
		gchar t_buf[500];
		if (db_streets_get (coords.current_lat, coords.current_lon, 0, &t_street) > 0)
		{
			if (g_ascii_strcasecmp (t_street.ref, "NULL") == 0)
			{
				g_snprintf (t_buf, sizeof (t_buf), "%s [%s]",
					t_street.name, t_street.type);
			}
			else
			{
				g_snprintf (t_buf, sizeof (t_buf), "%s - %s [%s]",
					t_street.ref, t_street.name, t_street.type);
			}
		}
		else
		{
			g_strlcpy (t_buf, _("--- street name not available ---"), sizeof (t_buf));
		}
		draw_infotext (t_buf);
	}
#endif

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

	gdk_gc_set_foreground (kontext_compass, &colors.grey);
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

	if (local_config.rotating_compass)
	{
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
			size/2, size/2, size/2+diam*t_x1off, size/2+diam*t_y1off);
		gdk_gc_set_foreground (kontext_compass, &colors.green);
		gdk_gc_set_line_attributes (kontext_compass, 3, 0, 0, 0);
		gdk_draw_line (drawable_compass, kontext_compass,
			size/2, size/2, size/2-diam*t_x1off, size/2-diam*t_y1off);

		/* draw direction labels */
		layout_compass = gtk_widget_create_pango_layout (drawing_compass, _("N"));
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
		gdk_gc_set_line_attributes (kontext_compass, 2, 0, 0, 0);
		gdk_gc_set_foreground (kontext_compass, &colors.black);
		gdk_draw_polygon (drawable_compass, kontext_compass, TRUE, poly, 5);
		gdk_gc_set_foreground (kontext_compass, &colors.white);
		gdk_gc_set_line_attributes (kontext_compass, 1, 0, 0, 0);
		gdk_draw_polygon (drawable_compass, kontext_compass, FALSE, poly, 5);

		/* draw bearing pointer */
		b = current.bearing - current.heading;
		if (b > 2*M_PI)
			b -= 2*M_PI;
		gdk_gc_set_foreground (kontext_compass, &colors.red);
		poly[0].x = size/2-diam*cos(b + M_PI_2);
		poly[0].y = size/2-diam*sin(b + M_PI_2);
		poly[1].x = size/2-0.2*diam*cos(b + M_PI) + 0.7*diam*cos(b + M_PI_2);
		poly[1].y = size/2-0.2*diam*sin(b + M_PI) + 0.7*diam*sin(b + M_PI_2);
		poly[2].x = size/2+0.4*diam*cos(b + M_PI_2);
		poly[2].y = size/2+0.4*diam*sin(b + M_PI_2);
		poly[3].x = size/2+0.2*diam*cos(b + M_PI) +0.7*diam*cos(b + M_PI_2);
		poly[3].y = size/2+0.2*diam*sin(b + M_PI) +0.7*diam*sin(b + M_PI_2);
		poly[4].x = poly[0].x;
		poly[4].y = poly[0].y;
		gdk_draw_polygon (drawable_compass, kontext_compass, TRUE, poly, 5);
	}
	else
	{
		/* draw compass cross and scale */
		for (i=0; i<4; i++)
		{
			gdk_draw_line (drawable_compass, kontext_compass,
				size/2+0.8*diam*cos(M_PI/4+i*M_PI_2),
				size/2+0.8*diam*sin(M_PI/4+i*M_PI_2),
				size/2+1.0*diam*cos(M_PI/4+i*M_PI_2),
				size/2+1.0*diam*sin(M_PI/4+i*M_PI_2));
		}
		gdk_draw_line (drawable_compass, kontext_compass,
			size/2-diam, size/2,
			size/2+diam, size/2);
		gdk_draw_line (drawable_compass, kontext_compass,
			size/2, size/2, size/2, size/2+diam);
		gdk_gc_set_foreground (kontext_compass, &colors.green);
		gdk_gc_set_line_attributes (kontext_compass, 3, 0, 0, 0);
		gdk_draw_line (drawable_compass, kontext_compass,
			size/2, size/2, size/2, size/2-diam);

		/* draw direction labels */
		layout_compass = gtk_widget_create_pango_layout (drawing_compass, _("N"));
		pango_layout_set_font_description (layout_compass, pfd_compass);
		pango_layout_get_pixel_size (layout_compass, &cx, &cy);
		gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
			(size-cx)/2, size/2-diam-cy,
			layout_compass, &colors.red, NULL);

		pango_layout_set_text (layout_compass, _("S"), -1);
		pango_layout_get_pixel_size (layout_compass, &cx, &cy);
		gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
			(size-cx)/2, size/2+diam,
			layout_compass, &colors.red, NULL);

		pango_layout_set_text (layout_compass, _("W"), -1);
		pango_layout_get_pixel_size (layout_compass, &cx, &cy);
		gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
			size/2-diam-cx-2, (size-cy)/2,
			layout_compass, &colors.red, NULL);

		pango_layout_set_text (layout_compass, _("E"), -1);
		pango_layout_get_pixel_size (layout_compass, &cx, &cy);
		gdk_draw_layout_with_colors ( drawable_compass, kontext_compass,
			size/2+diam+2, (size-cy)/2,
			layout_compass, &colors.red, NULL);

		if (layout_compass != NULL)
			g_object_unref (G_OBJECT (layout_compass));
		pango_font_description_free (pfd_compass);

		/* draw heading pointer */
		gdk_gc_set_line_attributes (kontext_compass, 2, 0, 0, 0);
		gdk_gc_set_foreground (kontext_compass, &colors.black);
		poly[0].x = size/2-1.1*diam*cos(current.heading + M_PI_2);
		poly[0].y = size/2-1.1*diam*sin(current.heading + M_PI_2);
		poly[1].x = size/2-0.3*diam*cos(current.heading + M_PI)
			+ 0.8*diam*cos(current.heading + M_PI_2);
		poly[1].y = size/2-0.3*diam*sin(current.heading + M_PI)
			+ 0.8*diam*sin(current.heading + M_PI_2);
		poly[2].x = size/2+0.6*diam*cos(current.heading + M_PI_2);
		poly[2].y = size/2+0.6*diam*sin(current.heading + M_PI_2);
		poly[3].x = size/2+0.3*diam*cos(current.heading + M_PI)
			+0.8*diam*cos(current.heading + M_PI_2);
		poly[3].y = size/2+0.3*diam*sin(current.heading + M_PI)
			+0.8*diam*sin(current.heading + M_PI_2);
		poly[4].x = poly[0].x;
		poly[4].y = poly[0].y;
		gdk_draw_polygon (drawable_compass, kontext_compass, TRUE, poly, 5);
		gdk_gc_set_foreground (kontext_compass, &colors.white);
		gdk_gc_set_line_attributes (kontext_compass, 1, 0, 0, 0);
		gdk_draw_polygon (drawable_compass, kontext_compass, FALSE, poly, 5);

		/* draw bearing pointer */
		gdk_gc_set_foreground (kontext_compass, &colors.red);
		poly[0].x = size/2-diam*cos(current.bearing + M_PI_2);
		poly[0].y = size/2-diam*sin(current.bearing + M_PI_2);
		poly[1].x = size/2-0.2*diam*cos(current.bearing + M_PI)
			+ 0.7*diam*cos(current.bearing + M_PI_2);
		poly[1].y = size/2-0.2*diam*sin(current.bearing + M_PI)
			+ 0.7*diam*sin(current.bearing + M_PI_2);
		poly[2].x = size/2+0.4*diam*cos(current.bearing + M_PI_2);
		poly[2].y = size/2+0.4*diam*sin(current.bearing + M_PI_2);
		poly[3].x = size/2+0.2*diam*cos(current.bearing + M_PI)
			+0.7*diam*cos(current.bearing + M_PI_2);
		poly[3].y = size/2+0.2*diam*sin(current.bearing + M_PI)
			+0.7*diam*sin(current.bearing + M_PI_2);
		poly[4].x = poly[0].x;
		poly[4].y = poly[0].y;
		gdk_draw_polygon (drawable_compass, kontext_compass, TRUE, poly, 5);
	}
	gdk_gc_set_foreground (kontext_compass, &colors.black);
	gdk_draw_arc (drawable_compass, kontext_compass, TRUE,
		size/2-1, size/2-1, 2, 2, 0, 360 * 64);

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
            //printf("X %d Y %d X2 %d Y2 %d\n", SCREEN_X, SCREEN_Y, SCREEN_X_2, SCREEN_Y_2);
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
					g_print (_("\nLoop detected, please report!\n"));
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

	if (!gui_status.dl_window)
	{
		if (gui_status.nightmode)
		{
			gdk_gc_set_function (kontext_map, GDK_AND);
			gdk_gc_set_foreground (kontext_map, &colors.night);
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

	if (mydebug >50)
		g_print ("simulated_pos()\n");

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
	secs = g_timer_elapsed (simulation_timer, NULL);
	g_timer_start (simulation_timer);

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
	GtkWidget *sel_friend_dialog, *scrolledwindow_friends;
	GtkWidget *treeview_friends;
	GtkCellRenderer *renderer_friends;
	GtkTreeViewColumn *column_friends;
	GtkTreeSelection *friends_select;

	sel_friend_dialog = gtk_dialog_new_with_buttons (
		_("Please select message recipient"),
		GTK_WINDOW (main_window), GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_window_set_icon_name (GTK_WINDOW (sel_friend_dialog), "gtk-network");

	if (local_config.guimode == GUI_PDA)
		gtk_window_set_default_size (GTK_WINDOW (sel_friend_dialog),
			gui_status.mapview_x, gui_status.mapview_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (sel_friend_dialog), 400, 360);

	g_signal_connect_swapped (G_OBJECT (sel_friend_dialog), "delete_event",
		G_CALLBACK (gtk_widget_destroy), G_OBJECT (sel_friend_dialog));
	g_signal_connect_swapped (G_OBJECT (sel_friend_dialog), "response",
		G_CALLBACK (gtk_widget_destroy), G_OBJECT (sel_friend_dialog));

	treeview_friends = gtk_tree_view_new_with_model (GTK_TREE_MODEL (friends_list));
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview_friends), TRUE);

	renderer_friends = gtk_cell_renderer_pixbuf_new ();
	column_friends = gtk_tree_view_column_new_with_attributes ("_",
		renderer_friends, "pixbuf", FRIENDS_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_friends), column_friends);

	renderer_friends = gtk_cell_renderer_text_new ();
	column_friends = gtk_tree_view_column_new_with_attributes (_("Name"),
		renderer_friends, "text", FRIENDS_NAME, NULL);
	gtk_tree_view_column_set_expand (column_friends, TRUE);
	gtk_tree_view_column_set_sort_column_id (column_friends, FRIENDS_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_friends), column_friends);

	renderer_friends = gtk_cell_renderer_text_new ();
	column_friends = gtk_tree_view_column_new_with_attributes (_("Distance"),
		renderer_friends, "text", FRIENDS_DIST_TEXT, NULL);
	gtk_tree_view_column_set_sort_column_id (column_friends, FRIENDS_DIST);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_friends), column_friends);

	renderer_friends = gtk_cell_renderer_text_new ();
	column_friends = gtk_tree_view_column_new_with_attributes (_("Latitude"),
		renderer_friends, "text", FRIENDS_LAT, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_friends), column_friends);

	renderer_friends = gtk_cell_renderer_text_new ();
	column_friends = gtk_tree_view_column_new_with_attributes (_("Longitude"),
		renderer_friends, "text", FRIENDS_LON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_friends), column_friends);

	friends_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_friends));
	gtk_tree_selection_set_mode (friends_select, GTK_SELECTION_SINGLE);
//	g_signal_connect (G_OBJECT (friends_select), "changed", G_CALLBACK (select_friend_cb), NULL);
//gtk_signal_connect (mylist, "select-row", setmessage_cb, mylist);


	messagewindow = sel_friend_dialog;


	scrolledwindow_friends = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (sel_friend_dialog)->vbox),
		scrolledwindow_friends, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_friends),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolledwindow_friends), treeview_friends);

	gtk_widget_show_all (sel_friend_dialog);

	return TRUE;
}


/* *****************************************************************************
 * on a TERM signal, do a clean shutdown
 */
#ifndef _WIN32
void signalhandler_term (int sig)
{
	if (mydebug > 0)
		g_print ("\nCatched SIGTERM - shutting down !\n");
	gtk_main_quit ();
}
#endif


/* *****************************************************************************
 * on a INT signal, do a clean shutdown
 */
#ifndef _WIN32
void signalhandler_int (int sig)
{
	if (mydebug > 0)
		g_print ("\nCatched SIGINT - shutting down !\n");
	gtk_main_quit ();
}
#endif


/* *****************************************************************************
 * on a USR2 signal, re-start the GPS connection  
 */
void
usr2handler (int sig)
{
	g_print (_("\nGot SIGUSR2; restarting GPS connection.\n"));
	initgps ();
}


/*
	'c':
		g_strlcpy (setpositionname, optarg, sizeof (setpositionname));
		
		_("-c WP     set start position in simulation mode to this waypoint (WP)\n")
*/

/* *****************************************************************************
 */
gboolean
parse_options_cb  (gchar *option, gchar *value, gpointer data, GError **error)
{
	if (g_ascii_strncasecmp (option, "-M", 2) == 0 ||
	    g_ascii_strncasecmp (option, "--gui-mode", 10) == 0)
	{
		if (g_ascii_strncasecmp (value, "car", 3) == 0)
			local_config.guimode = GUI_CAR;
		else if (g_ascii_strncasecmp (value, "pda", 3) == 0)
			local_config.guimode = GUI_PDA;
		else if (g_ascii_strncasecmp (value, "desktop", 7) == 0)
			local_config.guimode = GUI_DESKTOP;
		else
		{
			g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
				_("GUI mode '%s' not supported"), value);
			return FALSE;
		}
	}
	else if (g_ascii_strncasecmp (option, "-G", 2) == 0 ||
	    g_ascii_strncasecmp (option, "--geometry", 10) == 0)
	{
		g_strlcpy (geometry, value, sizeof (geometry));
		usegeometry = TRUE;
	}
	else if (g_ascii_strncasecmp (option, "-C", 2) == 0 ||
	    g_ascii_strncasecmp (option, "--config-file", 13) == 0)
	{
		if (g_file_test(value, G_FILE_TEST_EXISTS))
			g_strlcpy (local_config.config_file, value, sizeof (local_config.config_file));
		else
		{
			g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
				_("config file '%s' not found"), value);
			return FALSE;
		}
	}
	else if (g_ascii_strncasecmp (option, "-S", 2) == 0 ||
	    g_ascii_strncasecmp (option, "--screenshot", 12) == 0)
	{
		if (g_file_test(value, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)))
		{
			g_strlcpy (local_config.screenshot_dir, value,
				sizeof (local_config.screenshot_dir));
			takescreenshots = TRUE;
		}
		else
		{
			g_set_error (error, G_OPTION_ERROR, G_OPTION_ERROR_BAD_VALUE,
				_("screenshot directory '%s' not found"), value);
			return FALSE;
		}
	}
	else if (g_ascii_strncasecmp (option, "-N", 2) == 0 ||
	    g_ascii_strncasecmp (option, "--nmeaout", 9) == 0)
	{
		nmeaout = opennmea (value);
	}

	return TRUE;
}


/*******************************************************************************
 *                                                                             *
 *                             Main program                                    *
 *                                                                             *
 *******************************************************************************/
int
main (int argc, char *argv[])
{
    GError *error = NULL;
    gboolean show_version = FALSE;

    GOptionContext *opt_context = g_option_context_new (_("Navigation System"));
    const gchar opt_desc[] = N_("Website: http://www.gpsdrive.de");

    GOptionEntry opt_entries[] =
    {
	{"embedded", 'e', 0, G_OPTION_ARG_NONE, &local_config.embeddable_gui,
		_("don't show GUI; for use in external GTK apps"), NULL},
	{"force-position", 'f', 0, G_OPTION_ARG_NONE, &forcehavepos,
		_("force display of position even it is invalid"), NULL},
	{"ignore-checksum", 'i', 0, G_OPTION_ARG_NONE, &ignorechecksum,
		_("ignore NMEA checksum (risky, only for broken GPS receivers)"), NULL},
	{"position-mode", 'p', 0, G_OPTION_ARG_NONE, &gui_status.posmode,
		_("start in Position Mode"), NULL},
	{"nosplash", 's', 0, G_OPTION_ARG_NONE, &nosplash,
		_("don't show splash screen"), NULL},
	{"touchscreen", 't', 0, G_OPTION_ARG_NONE, &onemousebutton,
		_("having only a single button, for example when using a touchscreen"), NULL},
	{"verbose", 'v', 0, G_OPTION_ARG_NONE, &debug,
		_("show some debug info"), NULL},
	{"use-DBUS", 'x', 0, G_OPTION_ARG_NONE, &useDBUS,
		_("use DBUS for communication with gpsd; this disables socket communication"), NULL},
	{"alt-offset", 'A', 0, G_OPTION_ARG_INT, &local_config.normalnull,
		_("correct the altitude by adding this value"), _("<OFFSET>")},
	{"gpsd-server", 'B', 0, G_OPTION_ARG_STRING, &gpsdservername,
		_("servername for NMEA server (if gpsd runs on another host)"), _("<SERVER>")},
	{"config-file", 'C', 0, G_OPTION_ARG_CALLBACK, parse_options_cb,
		_("set config file to use"), _("<FILE>")},
	{"debug", 'D', 0, G_OPTION_ARG_INT, &mydebug,
		_("set debugging level"), _("<LEVEL>")},
	{"friends-server", 'F', 0, G_OPTION_ARG_FILENAME, &local_config.friends_serverfqn,
		_("select friends server, e.g. friendsd.gpsdrive.de"), _("<SERVERFILE>")},
	{"geometry", 'G', 0, G_OPTION_ARG_CALLBACK, parse_options_cb,
		_("set window geometry, e.g. 800x600"), _("<GEOMETRY>")},
	{"gui-mode", 'M', 0, G_OPTION_ARG_CALLBACK, parse_options_cb,
		_("set GUI mode, 'desktop' is the default"), "[car|pda|desktop]"},
	{"nmeaout", 'N', 0, G_OPTION_ARG_CALLBACK, parse_options_cb,
		_("set serial device, pty master, or file to use for NMEA output"), _("<FILE>")},
	{"screenshot", 'S', 0, G_OPTION_ARG_CALLBACK, parse_options_cb,
		_("take auto screenshots of different window (don't touch gpsdrive!)"), _("<PATH>")},
	{"run-test", 'T', 0, G_OPTION_ARG_NONE, &do_unit_test,
		_("do some internal unit tests (don't start gpsdrive)"), NULL},
	{"version", 0, 0, G_OPTION_ARG_NONE, &show_version,
		_("show version info"), NULL},
	{NULL}
    };

    g_option_context_set_description (opt_context, _(opt_desc));
    g_option_context_add_main_entries (opt_context, opt_entries, PACKAGE);
    g_option_context_add_group (opt_context, gtk_get_option_group (TRUE));


    gchar t_buf[500];

    gint i;

    struct tm *lt;
    time_t local_time, gmt_time;

    gdouble f;


#ifdef ENABLE_NLS
    /*  Activate localization -- L10n */
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    current.needtosave = FALSE;

    tzset ();
    gmt_time = time (NULL);

    lt = gmtime (&gmt_time);
    local_time = mktime (lt);
    current.timezone = lt->tm_isdst + (gmt_time - local_time) / 3600;

    /*   current.timezone = st->tm_gmtoff / 3600; */
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
    g_strlcpy (utctime, _("n/a"), sizeof (utctime));
    g_strlcpy (oldangle, _("none"), sizeof (oldangle));
    pixelfact = MAPSCALE / PIXELFACT;
    g_strlcpy (oldfilename, "", sizeof (oldfilename));
    maploaded = FALSE;
    haveNMEA = FALSE;
    current.gpsfix = 0;
    current.gps_precision = (-1.0);
    current.gps_hdop = (-1.0);
    gblink = blink = FALSE;
    haveposcount = debug = 0;
    current.heading = current.bearing = 0.0;
    current.zoom = 1;
    iszoomed = FALSE;

    /* init poityope_path as null */
    current.poitype_path = NULL;

#ifdef _WIN32
    {
		/* init Winsock */
        WSADATA 	       wsaData;
        WSAStartup( MAKEWORD( 1, 1 ), &wsaData );
    }
#else
    signal (SIGUSR2, usr2handler);
#endif


    simulation_timer = g_timer_new ();
    disttimer = g_timer_new ();

    memset (satlist, 0, sizeof (satlist));
    memset (satlistdisp, 0, sizeof (satlist));
    buffer = g_new (char, 2010);
    big = g_new0 (char, MAXBIG + 10);
    //big[0] = 0;
    
    timeoutcount = lastp = bigp = bigpRME = bigpGSA = bigpGSV = bigpGGA = 0;
    lastp = lastpGGA = lastpGSV = lastpRME = lastpGSA = 0;
    gcount = xoff = yoff = 0;
    hours = minutes = 99;
    milesconv = 1.0;
    g_strlcpy (messagename, "", sizeof (messagename));
    g_strlcpy (messageack, "", sizeof (messageack));
    g_strlcpy (messagesendtext, "", sizeof (messagesendtext));
    
    current.importactive = FALSE;
    dbdistance = 2000.0;
    dbusedist = TRUE;
    g_strlcpy (loctime, "n/a", sizeof (loctime));
    track = g_new0 (GdkSegment, 100000);
    trackshadow = g_new0 (GdkSegment, 100000);
    tracknr = 0;
    trackcoord = g_new0 (trackdata_struct, 100000);
    trackcoordnr = 0;
    tracklimit = trackcoordlimit = 100000;
    init_route_list ();

    earthr = calcR (coords.current_lat);

    /*  all default values must be set BEFORE readconfig! */
    g_strlcpy (setpositionname, "", sizeof (setpositionname));

    /* setup signal handler */
#ifndef _WIN32
    signal (SIGUSR1, signalposreq);
#endif

    /*  Detect the language for voice output */
    {
	gchar **lstr, lstr2[200];
	gchar *localestring;

	localestring = setlocale (LC_ALL, "");
#ifndef _WIN32
	if (localestring == NULL)
	    localestring = setlocale (LC_MESSAGES, "");
#endif
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
	    
	
	/* get language, used for POI titles and descriptions */
	if (!g_strlcpy (language,lstr2,3))
		g_strlcpy (language, "en", sizeof(language));
	
	/*    needed for right decimal delimiter ('.' or ',') */
	setlocale(LC_NUMERIC, "C");
    }

	/* init config struct with default values */
	config_init ();
	
	check_and_create_files();

	mapdl_init ();

	/*  initialization for GTK+ */
	gtk_init (&argc, &argv);

	/* setting numeric locale to "C"
	 * this is needed here because we use "." as decimal delimiter throughout
	 * the application and gtk_init also initializes the locale */
	setlocale(LC_NUMERIC, "C");

	/* we need to parse command args 2 times, because we need the config file param */
	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_print (_("Parsing of options failed: %s\n"), error->message);
		exit (EXIT_FAILURE);
	}
	if (show_version)
	{
		g_print (_("\nGpsDrive (C) 2001-2008 Fritz Ganter and the GpsDrive Development Team\n"
			"This program is Free Software licensed under the terms of the GNU GPL\n"
			"\nVersion %s\n%s\n\n"), VERSION, rcsid);
		exit (EXIT_SUCCESS);
	}


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


    /* 2. run see comment at first run */
	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_print (_("Parsing of options failed: %s\n"), error->message);
		exit (EXIT_FAILURE);
	}

	if (ignorechecksum)
		g_print ("\nWARNING: NMEA checksum test switched off!\n\n");

#ifndef DBUS
	if (useDBUS)
		g_print ("\nWARNING: You need to enable DBUS support with 'cmake -DWITH_DBUS=ON ..\n");
#endif

	if ( mydebug >99 )
		g_print ("options parsed\n");

	/* print version info */
	if ( mydebug > 0 )
		g_print ("\ngpsdrive (c) 2001-2006 Fritz Ganter"
		" <ganter@ganter.at>\n\nVersion %s\n%s\n\n", VERSION, rcsid);

	/*  show splash screen */
	if (!nosplash)
		show_splash ();

    init_lat2RadiusArray();

#ifdef SPEECH
	current.have_speech = speech_init ();
#endif

	/* set start position for simulation mode
	 * (only available in waypoints mode) */
	if (strlen (setpositionname) > 0 && !local_config.use_database)
	{
		for (i = 0; i < maxwp; i++)
		{
			if (!(g_strcasecmp ((wayp + i)->name, setpositionname)))
			{
				coords.current_lat = (wayp + i)->lat;
				coords.current_lon = (wayp + i)->lon;
				coords.target_lon = coords.current_lon + 0.001;
				coords.target_lat = coords.current_lat + 0.001;
			}
		}
	}

    /*    if we want NMEA mode, gpsd must be running and we connect to port 2222 */
    /*    An alternate gpsd server may be on 2947, we try it also */
    initgps ();

    /*  all position calculations are made in the expose callback */
//    g_signal_connect (GTK_OBJECT (map_drawingarea),
//			"expose_event", GTK_SIGNAL_FUNC (expose_cb), NULL);

	if (local_config.showtemp)
		temperature_get_values ();
	if (local_config.showbatt)
		battery_get_values ();

    g_strlcpy (mapfilename, "***", sizeof (mapfilename));

    current.needtosave = FALSE;

#ifdef SPEECH
	if (local_config.speech)
		speech_saytime (TRUE);
#endif

/* do all the basic initalization for the specific sections */

	if (db_init () == FALSE)
	{
		g_print (_("Disabling database support!\n"));
		local_config.use_database = FALSE;
	}

	if (local_config.use_database) 
		current.kismetsock = initkismet ();

	if( current.kismetsock != -1 )
	{
	    g_print (_("\nKismet server found\n"));
#ifdef SPEECH
	    g_snprintf( t_buf, sizeof(t_buf), _("Found kismet. Happy wardriving!"));
	    speech_saytext (t_buf, 3);
#endif
	};

	poi_init ();
	gui_init (geometry, usegeometry);
	friends_init ();
	route_init (NULL, NULL, NULL);

	load_friends_icon ();

	update_posbt();

    /*
     * setup signal handler for SIGTERM and SIGINT so that we can save everything
     * nicely when the machine is shutdown or CTRL-C is pressed.
     */
#ifndef _WIN32
    signal (SIGTERM, signalhandler_term);
    signal (SIGINT, signalhandler_int);
#endif


    /* gtk2 requires these functions in the order below do not change */
#ifdef _WIN32
    if (usegeometry) {
	GdkGeometry size_hints = {200, 200, 0, 0, 200, 200, 10, 10, 0.0, 0.0, GDK_GRAVITY_NORTH_WEST};

	gtk_window_set_geometry_hints(GTK_WINDOW (main_window), main_window, &size_hints,
                                  GDK_HINT_MIN_SIZE |
                                  GDK_HINT_BASE_SIZE |
                                  GDK_HINT_RESIZE_INC);

	if (!gtk_window_parse_geometry(GTK_WINDOW (main_window), geometry)) {
	    fprintf(stderr, _("Failed to parse %s\n"), geometry);
	}
	}
#endif

    // Initialize Track Filename
    savetrackfile (0);

	trip_reset_cb ();

    // ==================================================================
    // Unit Tests
#ifndef _WIN32
    if ( do_unit_test ) {
	unit_test();
    }
#endif

    if (takescreenshots) {
    	auto_take_screenshots();
    }


    /*  set the timers */
	/* after a suitable amount of time has passed for newer versions of libglib2.0
	 *  to become commonplace (>2.14), replace g_timeout_add(1000,...) with
	 *  g_timeout_add_seconds(1, ...) etc., as appropriate.
	 */
	timerto = g_timeout_add (TIMER, (GtkFunction) get_position_data_cb, NULL);
	redrawtimeout = g_timeout_add (200, (GtkFunction) calldrawmarker_cb, NULL);

	/*  if we started in simulator mode we have a little move roboter */
	if (current.simmode)
	{
		g_print ("Enabling simulation mode\n");
		simpos_timeout = g_timeout_add (300, (GtkFunction) simulated_pos, 0);
	}
	if (nmeaout)
		g_timeout_add (1000, (GtkFunction) write_nmea_cb, NULL);
	id_timeout_track = g_timeout_add (local_config.track_interval *1000,
		(GtkFunction) storetrack_cb, 0);
	g_timeout_add (TRIPMETERTIMEOUT*1000, (GtkFunction) update_tripdata_cb, 0);
	g_timeout_add (10 *1000, (GtkFunction) masteragent_cb, 0);
	if (local_config.use_database)
		g_timeout_add (15 *1000, (GtkFunction) poi_rebuild_list, 0);
	if (local_config.guimode != GUI_PDA)
	{
		if (battery_get_values ())
			g_timeout_add (5 *1000, (GtkFunction) expose_display_battery, NULL);
		if (temperature_get_values ())
			g_timeout_add (5 *1000, (GtkFunction) expose_display_temperature, NULL);
	}
	g_timeout_add (15 *1000, (GtkFunction) friendsagent_cb, 0);
#ifdef SPEECH
	if (local_config.speech)
	{
		g_timeout_add (600 *1000, (GtkFunction) (speech_saytime), FALSE);
		g_timeout_add (SPEECHOUTINTERVAL, (GtkFunction) speech_out_cb, 0);
		gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
			current.statusbar_id, _("Using speech output"));
	}
#endif

	/* timer for switching nightmode on or off if set to AUTO */
	g_timeout_add (1200 *1000, (GtkFunction) check_if_night_cb, NULL);

	/* timer for automatic track saving if enabled */
	if (local_config.track_autointerval > 0)
		id_timeout_autotracksave = g_timeout_add (local_config.track_autointerval * 1800 * 1000,
			(GtkFunction) track_autosave_cb, FALSE);

	/*  Mainloop */
	gtk_main ();

    g_timer_destroy (simulation_timer);
    if ( do_unit_test ) {
	coords.current_lat=48.000000;
	coords.current_lon=12.000000;
    }
    writeconfig ();
    gdk_pixbuf_unref (friendspixbuf);


    unlink ("/tmp/gpsdrivepos");
    if (local_config.savetrack)
	savetrackfile (2);

	/* save route and track to gpx on shutdown */
	g_snprintf (t_buf, sizeof (t_buf),"%stracksaved.gpx",local_config.dir_home);
	gpx_file_write (t_buf, GPX_TRK);
	if (route.items)
	{
		g_snprintf (t_buf, sizeof (t_buf),"%sroutesaved.gpx",local_config.dir_home);
		gpx_file_write (t_buf, GPX_RTE);
	}

	/* close all database connections */
	db_close ();

	/* cleanup all friends related data */
	cleanup_friends ();

    free_route_list ();

	/* cleanup poi data */
	cleanup_poi ();

    if (current.kismetsock != -1)
	close (current.kismetsock);
    gpsd_close();
    if (sockfd != -1)
	close (sockfd);

#ifdef SPEECH
	speech_close ();
#endif

    cleanup_nasa_mapfile ();
    mapdl_cleanup ();

    /* it would be quicker to free the memory by just letting the OS do it
	as part of tearing down the process upon exit, but by explicitly
	freeing it we remove a big false positive in the valgrind analysis */
    g_object_unref(tempimage);

    g_print (_("\n\nThank you for using GpsDrive!\n\n"));

    if ( do_unit_test )
	g_print (_("\n\nAll Unit Tests were successful.\n\n"));

    return EXIT_SUCCESS;
}
