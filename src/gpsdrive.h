/***********************************************************************

Copyright (c) 2001,2002 Fritz Ganter <ganter@ganter.at>

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

#ifndef GPSDRIVE_GPSDRIVE_H
#define GPSDRIVE_GPSDRIVE_H

#include <gmodule.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#include "mysql/mysql.h"
#include "gpsproto.h"

/*  adapt this section for the size of your screen */

/*  width of the map on screen, default is 640 */
#define SCREEN_X real_screen_x
/*  height of the map on screen, default is 512 */
#define SCREEN_Y real_screen_y

/*  set this to 0 for normal use, 1 for small screens */
#define SMALLMENU real_smallmenu
/*** Mod by Arms */
#define PADDING int_padding
/*** Mod by Arms */
#define XMINUS 60
/*** Mod by Arms (move) */
#define YMINUS 67

/*** Number of elements in an array */
#define ARRAY_SIZE(x)  ((sizeof (x))/(sizeof ((x)[0])))


/* Coordinate formats  */
enum
{
	LATLON_DEGDEC,
	LATLON_DMS,
	LATLON_MINDEC,
	LATLON_N_FORMATS
};

/* Distance formats */
enum
{
	DIST_MILES,
	DIST_METRIC,
	DIST_NAUTIC,
	DIST_N_FORMATS
};

/* Altitude formats */
enum
{
	ALT_FEET,
	ALT_METERS,
	ALT_YARDS,
	ALT_N_FORMATS
};

/* Nightmode settings */
enum
{
	NIGHT_OFF,
	NIGHT_ON,
	NIGHT_AUTO,
};

/* Definiton for travelmode used in local_config */
enum
{
	TRAVEL_CAR,
	TRAVEL_BIKE,
	TRAVEL_WALK,
	TRAVEL_BOAT,
	TRAVEL_AIRPLANE,
	TRAVEL_N_MODES
};

/* GUI Modes  */
enum
{
	GUI_CLASSIC,
	GUI_PDA,
	GUI_XWIN,
	GUI_N_FORMATS
};


/*  size of the bearing pointer, default is 50 */
#define PSIZE real_psize

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#define MAXBIG 50000

/*  How often do we redraw the screen (in milliseconds) */
#define REDRAWTIMER 300
/*  How often do we ask for positioning data */
#define TIMER 500
#define TIMERSERIAL 50

#define MAXSHOWNWP 100

/*  timer for watching waypoints (Radar) */
#define WATCHWPTIMER 2000

/*  If speech output is used, the intervall of spoken messages in milliseconds */
#define SPEECHOUTINTERVAL 10000

/*  defines offset and color of the shadows */
#define SHADOWOFFSET 7

#define ROUTEREACH (0.02+10*current.groundspeed/(3600*milesconv))
/* #define ROUTEREACH 0.05 */

#define SERVERNAME gpsdservername

#define SERVERPORT 2222
#define SERVERPORT2 2947


#define MAPSCALE 20000
/* Mapscale / pixelfact is meter / pixel */
#define PIXELFACT  2817.947378
#define KM2MILES   0.62137119 /* international_mile / km */
#define KM2NAUTIC  0.5399568 /* nautic_mile / km */

#define MAXMESG 8192


#define LOCALTESTXXX
#ifdef LOCALTEST
#define WEBSERVER "wuffi.ganter.at"
#else
// R.I.P. Borged by Microsoft, April, 2003:
// #define WEBSERVER "www.mapblast.com"
#define WEBSERVER "www.vicinity.com"
#endif
/* #define WEBSERVER2 "msrvmaps.mappoint.net" */
#define WEBSERVER2 "www.expedia.com"
#define WEBSERVER3 "host21.216.235.245.nedatavault.net"
#define WEBSERVER4 "www.expedia.de"

#define FESTIVAL_ENGLISH_INIT "(voice_ked_diphone)\n"
#define FESTIVAL_GERMAN_INIT "(voice_german_de3_os)\n"
#define FESTIVAL_SPANISH_INIT "(voice_el_diphone)\n"
#define EXPEDIAFACT 3950
/* #define EXPEDIAFACT 1378.6 */
#define MAXLISTENTRIES 500
#define TRIPMETERTIMEOUT 5

#define USIZE_X 15
#define USIZE_Y 24

/* 
$PSRF103,05,00,00,01*21  VTG off 
$PSRF103,05,00,01,01*20  VTG on
$PSRF108,1*33            WAAS/EGNOS on
$PSRF108,0*32            WAAS/EGNOS off

*/
#define EGNOSON  "$PSRF108,1*33\r\n"
#define EGNOSOFF "$PSRF108,0*32\r\n"


/* highest satellite number
 * WAAS/EGNOS sats are higher than the others.
 * Current highest known is "Anik" PRN=138
 * GPGSV (should) reports PRN number, so MAXSATS refers to PRN as well.
 * note SatID = PRN-87.
 */
#define MAXSATS 160


/* Maximum number of waypoint types and also userdefined icons  */
#define MAXPOITYPES 500

#define MAXDBNAME 30

#define TOOLTIP_DELAY 1000

/*
 * Declarations.
 */

extern gchar savetrackfn[256];

extern gint real_screen_x;
extern gint real_screen_y;

extern GdkGC *kontext;
extern GdkDrawable *drawable;
extern GtkWidget *track_bt;


gint line_crosses_rectangle(gdouble li_lat1, gdouble li_lon1, gdouble li_lat2, gdouble li_lon2,
			    gdouble sq_lat1, gdouble sq_lon1, gdouble sq_lat2, gdouble sq_lon2);
gdouble distance_line_point(gdouble x1, gdouble y1, gdouble x2, gdouble y2,
			    gdouble xp, gdouble yp);


char *  (*dl_mysql_error)(MYSQL *mysql);
MYSQL * (*dl_mysql_init)(MYSQL *mysql);
MYSQL * (*dl_mysql_real_connect)(MYSQL *mysql, const char *host,
                                           const char *user,
                                           const char *passwd,
                                           const char *db,
                                           unsigned int port,
                                           const char *unix_socket,
                                           unsigned int clientflag);
void    (*dl_mysql_close)(MYSQL *sock);
int     (*dl_mysql_query)(MYSQL *mysql, const char *q);
my_ulonglong (*dl_mysql_affected_rows)(MYSQL *mysql);
MYSQL_RES * (*dl_mysql_store_result)(MYSQL *mysql);
MYSQL_ROW   (*dl_mysql_fetch_row)(MYSQL_RES *result);
void        (*dl_mysql_free_result)(MYSQL_RES *result);
my_bool (*dl_mysql_eof)(MYSQL_RES *res);
gint addwaypoint_cb (GtkWidget * widget, gpointer datum);
gint importaway_cb (GtkWidget * widget, guint datum);
gint scaler_cb (GtkAdjustment * adj, gdouble * datum);
gint mapclick_cb (GtkWidget * widget, GdkEventButton * event);
gint scalerbt_cb (GtkWidget * widget, guint datum);
gint pos_cb (GtkWidget * widget, guint datum);
gint toggle_mapnik_cb (GtkWidget * widget, guint datum);
gint streets_draw_cb (GtkWidget * widget, guint datum);

// Some of these shouldn't be necessary, once all the gui stuff is finally moved
GtkWidget *find_poi_bt;


/* I didn't want to start a friends.h ;-) */
void drawfriends (void);
extern int actualfriends;
/* End of friends.h stuff */

void test_and_load_newmap ();
void map_koord_check_and_reload();
void coordinate_string2gdouble (const gchar * text,gdouble * dec);
void do_incremental_save();
void addwaypoint (gchar * wp_name, gchar * wp_type, gchar * wp_comment, gdouble wp_lat, gdouble wp_lon, gint save_in_db);
gdouble lat2radius (gdouble lat);
gdouble lat2radius_pi_180 (gdouble lat);

void draw_text_with_box (gdouble posx, gdouble posy, gchar * name);
int posxy_on_screen (gdouble posx, gdouble posy);


void init_lat2RadiusArray();
int display_background_map ();

typedef struct
{
	char id[30];
	char name[40];
	char type[40];
	char lat[40], lon[40];
	char timesec[40], speed[10], heading[10];
}
friendsstruct;

/* struct for current route status */
typedef struct
{
 	gint active;
	gint edit;
	gint pointer;
	gint show;
	gint items;
	gdouble distance;
}
routestatus_struct;

/* struct for all coordinates that have to be used globally */
typedef struct
{
/* current position */
	gdouble current_lon;
	gdouble current_lat;
/* target position */
	gdouble target_lon;
	gdouble target_lat;
/* saved "current position" while in pos mode */
	gdouble posmode_lon;
	gdouble posmode_lat;
	
/* ### What's the exact usage of for these variables???
 * TODO: Maybe they should be renamed to something more useful */
	gdouble zero_lon;
	gdouble zero_lat;
	gdouble old_lon;
	gdouble old_lat;
}
coordinate_struct;

/* struct for data about current position/movement status */
typedef struct
{
	gdouble groundspeed;
	gdouble heading;
	gdouble bearing;
	gchar target[80];
}
currentstatus_struct;

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif



typedef struct
{
  gchar filename[200];
  gdouble lat;
  gdouble lon;
  gint hasbbox;
  gdouble minlat;
  gdouble minlon;
  gdouble maxlat;
  gdouble maxlon;
  glong scale;
  gint map_dir;
}
mapsstruct;

typedef struct
{
	gchar name[40];
	gdouble lat;
	gdouble lon;
	gdouble dist;
	gchar typ[40];
	gint wlan;
	gint action;
	gint sqlnr;
	gint proximity;
	gchar comment[80];
}
wpstruct;

enum map_projections { 
    proj_undef, 
    proj_top, 
    proj_map,
    proj_googlesat
};
extern enum map_projections map_proj;


typedef struct
{
    gchar name[200];
    GtkWidget *checkbox;
    int to_be_displayed;
    int count;
}
map_dir_struct;

#endif /* GPSDRIVE_GPSDRIVE_H */
