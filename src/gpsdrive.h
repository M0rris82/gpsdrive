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

    *********************************************************************
$Log$
Revision 1.6  2005/03/27 21:25:46  tweety
separating map_import from gpsdrive.c

Revision 1.5  2005/02/17 09:46:34  tweety
minor changes

Revision 1.4  2005/02/06 21:18:05  tweety
more cleanup: extracted more functionality to functions

Revision 1.3  2005/02/06 17:52:44  tweety
extract icon handling to icons.c

Revision 1.2  2005/02/02 17:42:54  tweety
Add some comments
extract some code into funktions
added POI mySQL Support

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.29  2004/03/02 01:36:04  ganter
added German Expedia map server (expedia.de),
this should be used for european users
this is the real 2.09pre1 beta

Revision 1.28  2004/02/12 17:42:53  ganter
added -W switch for enable/disable WAAS/EGNOS (for SiRF II only?)

Revision 1.27  2004/02/11 21:59:22  ganter
max. number of sats is now MAXSATS

Revision 1.26  2004/02/06 14:55:54  ganter
added support for user-defined icons
create the directory: $HOME/.gpsdrive/icons
place your icons (type must be png) into this directory, with the name of
the waypoint type, filename must be lowercase
i.e. for waypoint type "HOTEL" the file must have the name "hotel.png"

Revision 1.25  2004/02/05 19:47:31  ganter
replacing strcpy with g_strlcpy to avoid bufferoverflows
USB receiver does not send sentences in direct serial mode,
so I first send a "\n" to it

Revision 1.24  2004/02/04 14:47:10  ganter
added GPGSA sentence for PDOP (Position Dilution Of Precision).

Revision 1.23  2004/02/03 23:19:27  ganter
fixed wrong string size

Revision 1.22  2004/02/02 03:38:32  ganter
code cleanup

Revision 1.21  2004/01/27 05:25:59  ganter
added gpsserial.c

gpsdrive now detects a running gps receiver
You don't need to start gpsd now, serial connection is handled by GpsDrive directly

Revision 1.20  2004/01/12 21:52:02  ganter
added friends message service

Revision 1.19  2004/01/09 00:00:43  ganter
added topomap download patch from Russell Harding <hardingr@billingside.com>
Thanks for the lot of work!

Revision 1.18  2004/01/03 04:32:18  ganter
translations


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

/*
 * Mod by Rick Richardson:
 *
 * There ought to be a 3-way selection for lat/lon display:
 * 	DMS	DD MM SS.SS	(quaint, but is it useful?)
 * 	MinDec	DD MM.MMM	(e.g. Geocaching)
 * 	DegDec	DD.DDDDDD
 * But the current gpsdrive GUI has only a 2-way toggle.
 *
 * Set this to 1 to prefer MinDec vs. DMS
 */
#define PREFER_MinDec	0

/*  size of the bearing pointer, default is 50 */
#define PSIZE real_psize

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#define MAXBIG 50000

/*  How oft do we redraw the screen (in milliseconds) */
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
#define PIXELFACT 2817.947378
#define KM2MILES 0.62137119
#define KM2NAUTIC  0.54

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


/* highest satellite number */
#define MAXSATS 80


/* Maximum number of waypoint types and also userdefined icons  */
#define MAXWPTYPES 100

/*
 * Declarations.
 */

extern gchar homedir[500];
extern gchar savetrackfn[256];

extern gint real_screen_x;
extern gint real_screen_y;
extern gint shadow;

extern GdkGC *kontext;
extern GdkDrawable *drawable;
extern GtkWidget *trackbt;

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
gint posmodeoff_cb (GtkWidget * widget, guint * datum);
gint scaler_cb (GtkAdjustment * adj, gdouble * datum);
gint mapclick_cb (GtkWidget * widget, GdkEventButton * event);
gint scalerbt_cb (GtkWidget * widget, guint datum);


typedef struct
{
  char id[30];
  char name[40];
  char lat[40], longi[40];
  char timesec[40], speed[10], heading[10];
}
friendsstruct;

/* typedef struct
 * {
 * 	  char id[30], name[40], lat[40], longi[40], timesec[40], speed[10],
 *     heading[10];
 * }
 * friendsstruct;
 */

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
}
mapsstruct;

#endif /* GPSDRIVE_GPSDRIVE_H */
