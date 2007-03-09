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
#include "mysql.h"
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


 /* Formats for coordinate display  */
enum
{
	LATLON_DEGDEC,
	LATLON_DMS,
	LATLON_MINDEC,
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

/* #define SHADOWGREY 0xD000  */
#define SHADOWGREY 0xA000 

#define ROUTEREACH (0.02+10*groundspeed/(3600*milesconv))
/* #define ROUTEREACH 0.05 */
#define ROUTEREACHFACT 10

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

extern gchar local_config_homedir[500];
extern gchar savetrackfn[256];

extern gint real_screen_x;
extern gint real_screen_y;
extern gint shadow;

extern GdkGC *kontext;
extern GdkDrawable *drawable;
extern GtkWidget *track_bt;

extern GdkColor red;
extern GdkColor black;
extern GdkColor white;
extern GdkColor blue ;
extern GdkColor nightcolor;
extern GdkColor lcd;
extern GdkColor lcd2;
extern GdkColor yellow;
extern GdkColor green;
extern GdkColor green2;
extern GdkColor mygray;
extern GdkColor textback;
extern GdkColor textbacknew;
extern GdkColor grey;
extern GdkColor orange;
extern GdkColor orange2;
extern GdkColor darkgrey;
extern GdkColor defaultcolor;


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
void addwaypoint (gchar * wp_name, gchar * wp_type, gdouble wp_lat, gdouble wp_lon);
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
  char lat[40], lon[40];
  char timesec[40], speed[10], heading[10];
}
friendsstruct;

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
