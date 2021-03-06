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
#include "gpsproto.h"

/*  set this to 0 for normal use, 1 for small screens */
#define SMALLMENU real_smallmenu
/*** Mod by Arms */
#define PADDING 1

/*** Number of elements in an array */
#define ARRAY_SIZE(x)  ((sizeof (x))/(sizeof ((x)[0])))


/* GPS modes (as defined by libgps) */
enum
{
	GPSMODE_NOT_SEEN,
	GPSMODE_NO_FIX,
	GPSMODE_2D,
	GPSMODE_3D
};

/* GPS fix status (as definde by libgps) */
enum
{
	GPS_NO_FIX,
	GPS_FIX,
	GPS_DGPS_FIX
};

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

/* Simulationmode settings */
enum
{
	SIM_OFF,
	SIM_ON,
	SIM_AUTO,
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

/* Friends data list */
enum
{
	FRIENDS_ID,
	FRIENDS_NAME,
	FRIENDS_TYPE,
	FRIENDS_LAT,
	FRIENDS_LON,
	FRIENDS_TIMESEC,
	FRIENDS_SPEED,
	FRIENDS_COURSE,
	FRIENDS_ICON,
	FRIENDS_DIST,
	FRIENDS_DIST_TEXT,
	FRIENDS_LAT_TEXT,
	FRIENDS_LON_TEXT,
	FRIENDS_N_ITEMS
};

/* GUI Modes  */
enum
{
	GUI_DESKTOP,
	GUI_PDA,
	GUI_CAR,
	GUI_N_FORMATS
};

/* GPX Parsing Modes */
enum gpx_mode
{
	GPX_INFO,	/* get only file info */
	GPX_WPT,	/* get waypoint data */
	GPX_TRK,	/* get track data */
	GPX_RTE,	/* get route data */
	GPX_TRKSEG	/* internally used, do not use in call directly */
};

/*  size of the bearing pointer, default is 50 */
//#define PSIZE real_psize


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#define MAXBIG 50000

/*  How often do we redraw the screen (in milliseconds) */
#define REDRAWTIMER 300

/* maximum number of waypoints to draw at one time */
 /* HB Nov 2008:  is this really needed? */
#define MAXSHOWNWP 5000

/*  If speech output is used, the interval of spoken messages in milliseconds */
#define SPEECHOUTINTERVAL 10000

/*  defines offset and color of the shadows */
#define SHADOWOFFSET 7

#define ROUTEREACH (0.02+10*current.groundspeed/(3600*local_config.distfactor))
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

#define FESTIVAL_ENGLISH_INIT "(voice_ked_diphone)\n"
#define FESTIVAL_GERMAN_INIT "(voice_german_de3_os)\n"
#define FESTIVAL_SPANISH_INIT "(voice_el_diphone)\n"
#define EXPEDIAFACT 3950
/* #define EXPEDIAFACT 1378.6 */
#define TRIPMETERTIMEOUT 2

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

#define TOOLTIP_DELAY 1000

#define DEG2RAD(x) (x*M_PI/180.0)
#define RAD2DEG(x) (x/M_PI*180.0)

#define ZOOM_MIN 1
#define ZOOM_MAX 16

#define DEGREE "\xc2\xb0"

/*
 * Declarations.
 */

extern gchar savetrackfn[256];

extern GdkGC *kontext;
extern GdkDrawable *drawable;


gint line_crosses_rectangle(gdouble li_lat1, gdouble li_lon1, gdouble li_lat2, gdouble li_lon2,
			    gdouble sq_lat1, gdouble sq_lon1, gdouble sq_lat2, gdouble sq_lon2);
gdouble distance_line_point(gdouble x1, gdouble y1, gdouble x2, gdouble y2,
			    gdouble xp, gdouble yp);

#ifdef _MSC_VER
#define DL_MYSQL_IMPORT __stdcall
#else
#define DL_MYSQL_IMPORT 
#endif
gint scaler_cb (GtkAdjustment * adj, gdouble * datum);
gint mapclick_cb (GtkWidget * widget, GdkEventButton * event);
gint scalerbt_cb (GtkWidget * widget, guint datum);
gint explore_cb (GtkWidget * widget, guint datum);
gint toggle_mapnik_cb (GtkWidget * widget, guint datum);

void test_and_load_newmap ();
void map_koord_check_and_reload();
void coordinate_string2gdouble (const gchar * text,gdouble * dec);
void do_incremental_save();
gdouble lat2radius (gdouble lat);
gdouble lat2radius_pi_180 (gdouble lat);

void draw_text_with_box (gdouble posx, gdouble posy, gchar * name);
int posxy_on_screen (gdouble posx, gdouble posy);


void init_lat2RadiusArray();
int display_background_map ();

/* struct for current route status */
typedef struct
{
	gchar name[255];
	gchar desc[255];
	gchar src[255];
	GtkWidget *label;
	GtkWidget *icon;
	gint active;
	gint edit;
	gint pointer;
	gint show;
	gint items;
	gdouble distance;
	gboolean forcenext;
}
routestatus_struct;

/* struct to hold streets data */
typedef struct
{
  gchar name[255];
  gchar ref[50];
  gchar type[160];
}
street_struct;

/* struct for all coordinates that have to be used globally */
typedef struct
{
/* current position */
	gdouble current_lon;
	gdouble current_lat;
/* (next) target position */
	gdouble target_lon;
	gdouble target_lat;
/* saved "current position" while in explore mode */
	gdouble expmode_lon;
	gdouble expmode_lat;
/* target position */
	gdouble dest_lon;
	gdouble dest_lat;

/* ### What's the exact usage of for these variables???
 * TODO: Maybe they should be renamed to something more useful */
	gdouble zero_lon;
	gdouble zero_lat;
	gdouble old_lon;
	gdouble old_lat;
	gdouble wp_lat;
	gdouble wp_lon;
}
coordinate_struct;

/* struct for data about current position/movement/status data */
typedef struct
{
	gdouble groundspeed;
	gint pos_x;		/* position in map window in px */
	gint pos_y;		/* position in map window in px */
	gdouble course;		/* course on ground in radians */
	gdouble bearing;	/* bearing in radians */
	gdouble altitude;	/* current altitude */
	gint timezone;		/* current timezone */
	glong mapscale;		/* scale of map shown */
	gchar maptype[12];	/* type of map shown */
	gint zoom;		/* map zoom level */
	gchar target[80];	/* name of current target */
	gdouble dist;		/* distance to selected target */
	gint statusbar_id;	/* context_id of current statusbar message */
	gboolean simmode;	/* Status of Simulation mode */
	gint kismetsock;	/* Kismet socket, -1 if not available */
	gint gps_mode;		/* gps status */
	gint gps_status;	/* gps fix mode */
	gdouble gps_hdop;	/* gps horizontal dilution of position */
	gdouble gps_eph;	/* gps estimated horizontal error in meters */
	gdouble gps_epv;	/* gps estimated vertical error in meters */
	gint gps_sats_used;	/* number of satellites currently used */
	gint gps_sats_in_view;	/* number of satellites currently seen */
	gchar utc_time[20];	/* utc time from gps */
	gchar loc_time[20];	/* local time from gps */
	GTimeVal last3dfixtime;	/* saves time of last 3D Fix. Hack for jumping
				 * altitude display caused by bad nmea data */
	gboolean needtosave;	/* flag if config has to be saved */
	gboolean importactive;
	gboolean save_in_db;	/* flag if new waypoints should be saved in the database */
	gboolean poi_osm;	/* flag if sqlite file with osm poi data is available */
	gchar *poitype_path;
	gchar poifilter[20000];	/* sql string for filtering the POI display */
	gboolean have_speech;	/* flag if speech dispatcher is available */
	gint window_state;	/* holds the current GdkWindowState for the main window */
	gchar street_info[500];	/* holds info about the current street if available */
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
	gchar name[80];
	gdouble lat;
	gdouble lon;
	gdouble dist;
	gchar typ[160];
	gint proximity;
	gchar comment[255];
}
wpstruct;

enum map_projections { 
    proj_undef, 
    proj_top, 
    proj_map,
    proj_googlesat,
    proj_mapnik
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
