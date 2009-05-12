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

#ifdef SPEECH
#include "speech.h"
#endif

extern gint mydebug;

extern coordinate_struct coords;
extern currentstatus_struct current;

gps_satellite_struct *gps_sats;




#ifdef MAEMO /* the code below utilizes liblocation found on maemo platforms */



#include <location/location-gps-device.h>
#include <location/location-gpsd-control.h>
 
LocationGPSDevice *gps_device;
LocationGPSDControl *gps_control;

/* SYMBOLS USED IN LIBLOCATION
 * 
 * gboolean online: Whether GPSD has been able to connect to the GPS device
 * 
 * enum status: The status of the device
 *	LOCATION_GPS_DEVICE_STATUS_NO_FIX: The device does not have a fix
 *	LOCATION_GPS_DEVICE_STATUS_FIX: The device has a fix
 *	LOCATION_GPS_DEVICE_STATUS_DGPS_FIX: The device has a DGPS fix
 *    
 * struct fix: The location fix
 * 	int mode: The mode of the fix
 *		LOCATION_GPS_DEVICE_MODE_NOT_SEEN: The device has not seen a satellite yet
 *		LOCATION_GPS_DEVICE_MODE_NO_FIX: The device has no fix
 *		LOCATION_GPS_DEVICE_MODE_2D: The device has latitude and longitude fix
 *		LOCATION_GPS_DEVICE_MODE_3D: The device has latitude, longitude and altitude
 *   	@fields: A bitfield representing what fields contain valid data
 *		LOCATION_GPS_DEVICE_NONE_SET
 *		LOCATION_GPS_DEVICE_ALTITUDE_SET
 *		LOCATION_GPS_DEVICE_SPEED_SET
 *		LOCATION_GPS_DEVICE_TRACK_SET
 *		LOCATION_GPS_DEVICE_CLIMB_SET
 *		LOCATION_GPS_DEVICE_LATLONG_SET
 *		LOCATION_GPS_DEVICE_TIME_SET
 *	@time: The timestamp of the update
 *	double ept: Estimated time uncertainty
 *	double latitude: Fix latitude
 *	double longitude: Fix longitude
 *	double eph: Horizontal position uncertainty
 *	double altitude: Fix altitude (in metres)
 *	double epv: Vertical position uncertainty
 *	double track: The current direction of motion (in degrees between 0 and 359)
 *	double epd: Track uncertainty
 *	double speed: Current speed (in km/h)
 *	double eps: Speed uncertainty
 *	double climb: Current rate of climb (in m/s)
 *	double epc: Climb uncertainty
 *
 * int satellites_in_view: Number of satellites the GPS device can see
 * int satellites_in_use: Number of satellites the GPS used in calculating @fix
 * 
 * struct satellites: Array containing #LocationGPSDeviceSatellite
 *	int prn: Satellite ID number
 *	int elevation: Elevation of the satellite
 *	int azimuth: Satellite azimuth
 *	int signal_strength: Signal/Noise ratio
 *	gboolean @in_use: Whether the satellite is being used to calculate the fix
 */
	

/* *****************************************************************************
 * update gps data when change is signalled
 */
static void
gps_hook_cb (LocationGPSDevice *device, gpointer data)
{
	if (mydebug > 20)
		g_print ("gps_hook_cb ()\n");
	
	if (device->status == LOCATION_GPS_DEVICE_STATUS_NO_FIX
	    || device->fix->mode == LOCATION_GPS_DEVICE_MODE_NOT_SEEN)
		return;

	if (mydebug > 40)
	{
		g_print ("----- GPS Data -----\n");
		g_print ("  Fix Status: %d\n  Fix Mode: %d\n",
			device->status, device->fix->mode);
		g_print ("  Sats used: %d of %d\n",
			device->satellites_in_use, device->satellites_in_view);
		g_print ("  Position: %.6f / %.6f\n",
			device->fix->latitude, device->fix->longitude);
		g_print ("  Course: %.2f\n  GPS time: %.0f\n",
			device->fix->track, device->fix->time);
		g_print ("  Speed: %.1f m/s\n  Altitude: %.1f m\n\n",
			device->fix->speed, device->fix->altitude);
	}

	if (device->fix->fields & LOCATION_GPS_DEVICE_NONE_SET)
		return;

	current.gps_status = device->status;
	current.gps_mode = device->fix->mode;
	current.gps_eph = device->fix->eph;
	current.gps_epv = device->fix->epv;
	current.gps_sats_used = device->satellites_in_use;
	current.gps_sats_in_view = device->satellites_in_view;
	if (device->fix->fields & LOCATION_GPS_DEVICE_LATLONG_SET)
	{
		coords.current_lat = device->fix->latitude;
		coords.current_lon = device->fix->longitude;
	}
	if (device->fix->fields & LOCATION_GPS_DEVICE_ALTITUDE_SET)
		current.altitude = device->fix->altitude;
	if (device->fix->fields & LOCATION_GPS_DEVICE_TRACK_SET) 
		current.course = device->fix->track * DEG_2_RAD;
	if (device->fix->fields & LOCATION_GPS_DEVICE_SPEED_SET)
		current.groundspeed = device->fix->speed * local_config.distfactor;
	if (device->fix->fields & LOCATION_GPS_DEVICE_TIME_SET)
	{
		struct tm t_tim;
		time_t t_gpstime;
		t_gpstime = (time_t) device->fix->time;
		gmtime_r (&t_gpstime, &t_tim);
		strftime (current.utc_time, sizeof (current.utc_time), "%H:%M:%S", &t_tim);
		localtime_r(&t_gpstime, &t_tim);
		strftime (current.loc_time, sizeof (current.loc_time), "%H:%M", &t_tim);
	}
/*	if (device->satellites_in_view > 0)
	{
		gint i;
		for (i=0;i<device->satellites_in_view;i++)
		{
			gps_sats[i].used = device->satellites[i].in_use;
			gps_sats[i].prn = device->satellites[i].prn;
			gps_sats[i].elevation = device->satellites[i].elevation;
			gps_sats[i].azimuth = device->satellites[i].azimuth;
			gps_sats[i].snr = device->satellites[i].signal_strength;
		}
	}
*/
#ifdef SPEECH
	if (!local_config.mute && local_config.sound_gps)
		speech_say_gpsfix ();
#endif

}


/* *****************************************************************************
 * close connection to gps
 */
void
gpsd_disconnect (void)
{
	location_gpsd_control_stop (gps_control);

	g_free (gps_sats);
}


/* *****************************************************************************
 * open connection to gps
 */
gboolean
gpsd_connect (gboolean reconnect)
{
	/* close connection first on reconnect request */
/*	if (reconnect && gpsdata)
	{
		g_source_remove (gps_timeout_source);
		gps_close (gpsdata);
	}
*/

	/* try to open connection */
g_print ("--- connecting to gps control\n");
	gps_control = g_object_new (LOCATION_TYPE_GPSD_CONTROL, NULL);
g_print ("--- starting gps\n");
	location_gpsd_control_start (gps_control);

g_print ("--- connecting to gps device\n");
	gps_device = g_object_new (LOCATION_TYPE_GPS_DEVICE, NULL);
	if (!gps_device)
	{
		g_print ("Can't connect to GPS device, disabling GPS support!\n");
		current.gps_mode = MODE_NOT_SEEN;
		if (local_config.simmode == SIM_AUTO)
			current.simmode = TRUE;
		return FALSE;
	}

	g_print ("Connection to GPS Device established.\n");
	if (local_config.simmode == SIM_AUTO)
		current.simmode = FALSE;

	/* allocate struct to hold satellite data */
	gps_sats = g_new (gps_satellite_struct, MAXCHANNELS);

g_print ("--- connecting signal to gps\n");
	/* set signal function to handle gps data */
	g_signal_connect (gps_device, "changed", G_CALLBACK (gps_hook_cb), NULL);

	return TRUE;
}



#else /* the code below uses a standard gpsd */



#define GPS_TIMER 500

static gint gps_timeout_source = 0;
static struct gps_data_t *gpsdata;

/* SYMBOLS USED IN LIBGPS:
 * 
 * double online: NZ if GPS is on line, 0 if not.
 *
 * struct fix:
 * 	int mode: Mode of fix
 * 		MODE_NOT_SEEN: mode update not seen yet
 *		MODE_NO_FIX: none
 *		MODE_2D: good for latitude/longitude
 *		MODE_3D: good for altitude/climb too
 *	double latitude: Latitude in degrees (valid if mode >= 2)
 *	double longitude: Longitude in degrees (valid if mode >= 2)
 *	double eph: Horizontal position uncertainty, meters
 *	double altitude: Altitude in meters (valid if mode == 3)
 *	double epv: Vertical position uncertainty, meters
 *	double speed: Speed over ground, meters/sec
 *	double climb: Vertical speed, meters/sec
 *
 * int status: Do we have a fix? (always valid)
 *	STATUS_NO_FIX: no
 *	STATUS_FIX: yes, without DGPS
 *	STATUS_DGPS_FIX: yes, with DGPS
 *
 * struct satellite status: valid when satellites > 0
 *    int satellites_used: Number of satellites used in solution
 *    int used[MAXCHANNELS]: PRNs of satellites used in solutio
 *    int satellites: Number of satellites in vie
 *    int PRN[MAXCHANNELS]: PRNs of satellit
 *    int elevation[MAXCHANNELS]: elevation of satellit
 *    int azimuth[MAXCHANNELS]: azimut
 *    int ss[MAXCHANNELS]: signal-to-noise ratio (dB
 */


/* *****************************************************************************
 * hook function to get data from gpsd
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
		g_print ("  Course: %.2f\n  GPS time: %.0f\n",
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
		current.course = data->fix.track * DEG_2_RAD;
	if (data->set & SPEED_SET)
		current.groundspeed = data->fix.speed * MPS_TO_KPH * local_config.distfactor;
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


#endif

