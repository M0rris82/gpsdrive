/***********************************************************************

Copyright (c) 2001-2008 Fritz Ganter and the GpsDrive Development Teamp

Website: www.gpsdrive.de

Disclaimer: Please do not use as a primary source of navigation.

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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <gps.h>
#include "config.h"
#include "gpsdrive.h"
#include "gps_handler.h"
#include "gpsdrive_config.h"
#include "gui.h"

#include "gettext.h"
#include "os_specific.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


/*  How often do we ask for positioning data */
#define GPS_TIMER 500

extern gint mydebug;

extern coordinate_struct coords;
extern currentstatus_struct current;

gps_satellite_struct *gps_sats;

static gint gps_timeout_source = 0;

static struct gps_data_t *gpsdata;
//    double online;		/* NZ if GPS is on line, 0 if not. */
//
//    struct gps_fix_t	fix;
//    {
//	    int    mode;	/* Mode of fix */
//	#define MODE_NOT_SEEN	0	/* mode update not seen yet */
//	#define MODE_NO_FIX	1	/* none */
//	#define MODE_2D  	2	/* good for latitude/longitude */
//	#define MODE_3D  	3	/* good for altitude/climb too */
//	    double latitude;	/* Latitude in degrees (valid if mode >= 2) */
//	    double longitude;	/* Longitude in degrees (valid if mode >= 2) */
//	    double eph;  	/* Horizontal position uncertainty, meters */
//	    double altitude;	/* Altitude in meters (valid if mode == 3) */
//	    double epv;  	/* Vertical position uncertainty, meters */
//	    double speed;	/* Speed over ground, meters/sec */
//	    double climb;       /* Vertical speed, meters/sec */
//    };
//
//    /* GPS status -- always valid */
//    int    status;		/* Do we have a fix? */
//#define STATUS_NO_FIX	0	/* no */
//#define STATUS_FIX	1	/* yes, without DGPS */
//#define STATUS_DGPS_FIX	2	/* yes, with DGPS */
//
//    /* satellite status -- valid when satellites > 0 */
//    int satellites_used;	/* Number of satellites used in solution */
//    int used[MAXCHANNELS];	/* PRNs of satellites used in solution */
//    int satellites;		/* # of satellites in view */
//    int PRN[MAXCHANNELS];	/* PRNs of satellite */
//    int elevation[MAXCHANNELS];	/* elevation of satellite */
//    int azimuth[MAXCHANNELS];	/* azimuth */
//    int ss[MAXCHANNELS];	/* signal-to-noise ratio (dB) */


/* *****************************************************************************
 * close connection to gpsd
 */
void
gps_hook_cb (struct gps_data_t *data, gchar *buf)
{
	gint i;
	struct tm t_tim;
	time_t t_gpstime;

	if (mydebug > 20)
		g_print ("gps_hook_cb ()\n");

	if (data->status == STATUS_NO_FIX || data->fix.mode == MODE_NOT_SEEN)
		return;

	if (mydebug > 40)
	{
		g_print ("  Fix Status: %d\n  Fix Mode: %d\n", data->status, data->fix.mode);
		g_print ("  Sats used: %d of %d\n", data->satellites_used, data->satellites);
		g_print ("  Position: %.6f / %.6f\n", data->fix.latitude, data->fix.longitude);
		g_print ("  Heading: %.2f\n  GPS time: %.0f\n",
			data->fix.track, data->fix.time);
		g_print ("  Speed: %.1f m/s\n  Altitude: %.1f m\n\n",
			data->fix.speed, data->fix.altitude);
	}

	current.gps_status = data->status;
	current.gps_mode = data->fix.mode;
	coords.current_lat = data->fix.latitude;
	coords.current_lon = data->fix.longitude;
	current.altitude = data->fix.altitude;
	current.gps_sats_used = data->satellites_used;
	current.gps_sats_in_view = data->satellites;
	if (data->set & TRACK_SET) 
		current.heading = data->fix.track * DEG_2_RAD;
	if (data->set & SPEED_SET)
		current.groundspeed = data->fix.speed * MPS_TO_KPH;
	current.gps_hdop = data->hdop;
	current.gps_eph = data->fix.eph;
	current.gps_epv = data->fix.epv;

	t_gpstime = (time_t) data->fix.time;
	gmtime_r (&t_gpstime, &t_tim);
	strftime (current.utc_time, sizeof (current.utc_time), "%H:%M:%S", &t_tim);
	localtime_r(&t_gpstime, &t_tim);
	strftime (current.loc_time, sizeof (current.loc_time), "%H:%M", &t_tim);

	if ((data->set & SATELLITE_SET) && (data->satellites > 0))
	{
		for (i=0;i<data->satellites;i++)
		{
			gps_sats[i].used = data->used[i];
			gps_sats[i].prn = data->PRN[i];
			gps_sats[i].elevation = data->elevation[i];
			gps_sats[i].azimuth = data->azimuth[i];
			gps_sats[i].snr = data->ss[i];
		}
	}

	data->set = 0;

#ifdef SPEECH
	if (!local_config.mute && local_config.sound_gps)
		speech_say_gpsfix ();
#endif
}


gboolean
gps_query_data_cb (gpointer data)
{
	if (current.importactive)
		return TRUE;

	/* query gpsd for data:
	 * o - get time/position/velocity data
	 * y - get satellite info
	 * s - get fix status (must be after "o" else its value is overwritten)
	 */
	gpsdata->set = 0;
	gps_query (gpsdata, "oys\n");

	return (TRUE);
}


/* *****************************************************************************
 * close connection to gpsd
 */
void
gpsd_disconnect (void)
{
	if (gpsdata)
		gps_close (gpsdata);

	g_free (gps_sats);
}


/* *****************************************************************************
 * open connection to gpsd
 */
gboolean
gpsd_connect (gboolean reconnect)
{
	/* close connection first on reconnect request */
	if (reconnect && gpsdata)
	{
		g_source_remove (gps_timeout_source);
		gps_close (gpsdata);
	}

	/* try to open connection */
	gpsdata =  gps_open (local_config.gpsd_server, local_config.gpsd_port);
	if (!gpsdata)
	{
		g_print ("Can't connect to gps daemon on %s:%s, disabling GPS support!\n",
			local_config.gpsd_server, local_config.gpsd_port);
		current.gps_mode = MODE_NOT_SEEN;
		if (local_config.simmode == SIM_AUTO)
			current.simmode = TRUE;
		return FALSE;
	}
	g_print ("Connection to GPS Daemon on %s:%s established.\n",
		local_config.gpsd_server, local_config.gpsd_port);
	if (local_config.simmode == SIM_AUTO)
		current.simmode = FALSE;

	/* allocate struct to hold satellite data */
	gps_sats = g_new (gps_satellite_struct, MAXCHANNELS);

	/* set hook function to handle gps data */
	gps_set_raw_hook (gpsdata, (gpointer) gps_hook_cb);

	/* set up function to poll periodically for new data */
	gps_timeout_source = g_timeout_add (GPS_TIMER, gps_query_data_cb, NULL);

	return TRUE;
}
