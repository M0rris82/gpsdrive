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

*********************************************************************/

#include "config.h"

#include "config.h"
#include "gettext.h"
#include "gpsdrive.h"
#include "icons.h"
#include "gps_handler.h"
#include "nmea_handler.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

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

#ifdef DBUS_ENABLE
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>
static DBusHandlerResult dbus_signal_handler (
		DBusConnection* connection, DBusMessage* message);
static DBusHandlerResult dbus_handle_gps_fix (DBusMessage* message);
#endif

extern gint mydebug;
gint gps_handler_debug = 0;


extern gint maploaded;
extern gint isnight, disableisnight;

extern gchar utctime[20], loctime[20];
extern gint forcehavepos;
extern gint havepos, haveposcount;
extern gint blink, gblink, xoff, yoff;
extern gint zone;
extern gint oldsatfix, oldsatsanz;
extern gdouble precision;
extern gchar localedecimal;
extern gdouble gbreit, glang, milesconv, olddist;
extern gchar mapfilename[1024];
extern gint satlist[MAXSATS][4], satlistdisp[MAXSATS][4], satbit;
extern gint newsatslevel;
extern gint satfix, usedgps;
extern gint sats_used, sats_in_view;
extern gchar *buffer, *big;
extern fd_set readmask;
extern struct timeval timeout;
extern gdouble earthr;
extern GTimer *timer, *disttimer;
extern int newdata;
extern int didrootcheck;
extern gint timeoutcount;
extern gint simpos_timeout;
extern int timerto;
extern GtkWidget *satslabel1, *satslabel2, *satslabel3;
extern GdkPixbuf *satsimage;
extern gchar dgpsserver[80], dgpsport[10];
extern gchar gpsdservername[200];
extern GtkWidget *frame_statusbar;
extern GtkWidget *pixmapwidget, *gotowindow;
extern gint statuslock, gpson;
extern gint earthmate;
extern coordinate_struct coords;
extern currentstatus_struct current;

// ---------------------- NMEA
extern gint haveRMCsentence;
extern gdouble NMEAsecs;
extern gint NMEAoldsecs;
extern FILE *nmeaout;
/*  if we get data from gpsd in NMEA format haveNMEA is TRUE */
gchar nmeamodeandport[50];
gint haveNMEA;

#ifdef DBUS_ENABLE
gint useDBUS;
DBusError dbuserror;
DBusConnection* connection;
struct dbus_gps_fix {
	gdouble	time;		/* Time as time_t with optional fractional seconds */
	gint32	mode;		/* Fix type
				 * 0 = Not seen. 1 = No fix. 2/3 = 2D/3D fix
				 * We use -1 to identify that the current fix
				 * is * already processed */
	gdouble ept;		/* Expected time uncertainty */
	gdouble latitude;	/* Latitude in degrees (valid if mode >= 2) */
	gdouble longitude;	/* Longitude in degrees (valid if mode >= 2) */
	gdouble eph;		/* Horizontal position uncertainty, meters */
	gdouble altitude;	/* Altitude in meters (valid if mode == 3) */
	gdouble epv;		/* Vertical position uncertainty, meters */
	gdouble track;		/* Course made good (relative to true north) */
	gdouble epd;		/* Track uncertainty, degrees */
	gdouble speed;		/* Speed over ground, meters/sec */
	gdouble eps;		/* Speed uncertainty, meters/sec */
	gdouble climb;		/* Vertical speed, meters/sec */
	gdouble epc;		/* Vertical speed uncertainty */
};
struct dbus_gps_fix dbus_old_fix, dbus_current_fix;
#ifndef NAN
#define NAN (0.0/0.0)
#endif
#endif
gint bigp = 0, bigpGGA = 0, bigpRME = 0, bigpGSA = 0, bigpGSV = 0;
gint lastp = 0, lastpGGA = 0, lastpRME = 0, lastpGSA = 0, lastpGSV = 0;
gint sock = -1;

gint convertGSV (char *f);


/* *****************************************************************************
 */
void
gpsd_close ()
{
    if (sock != -1) {
        socket_close (sock);
        sock = -1;
    }
}

/* *****************************************************************************
 */
#ifdef DBUS_ENABLE
void
init_dbus_current_fix()
{
	// Preserve the time
	dbus_current_fix.mode = 0;
	dbus_current_fix.ept = NAN;
	dbus_current_fix.latitude = NAN;
	dbus_current_fix.longitude = NAN;
	dbus_current_fix.eph = NAN;
	dbus_current_fix.altitude = NAN;
	dbus_current_fix.epv = NAN;
	dbus_current_fix.track = NAN;
	dbus_current_fix.epd = NAN;
	dbus_current_fix.speed = NAN;
	dbus_current_fix.eps = NAN;
	dbus_current_fix.climb = NAN;
	dbus_current_fix.epc = NAN;
}

void init_dbus(){
	haveNMEA = TRUE;
	
	memset(&dbus_old_fix, 0, sizeof(struct dbus_gps_fix));
	init_dbus_current_fix();
	dbus_current_fix.time = 0.0; // Time is not set in init_dbus_current_time
	
	dbus_error_init (&dbuserror);
	
	connection = dbus_bus_get (DBUS_BUS_SYSTEM, &dbuserror);	
	if (dbus_error_is_set (&dbuserror)) {
		g_print ("%s: %s", dbuserror.name, dbuserror.message);
		exit(0);
	}
	
	dbus_bus_add_match (connection, "type='signal'", &dbuserror);
	if (dbus_error_is_set (&dbuserror)) {
		g_print (_("unable to add match for signals %s: %s"), dbuserror.name, dbuserror.message);
		exit(0);
	}

	if (!dbus_connection_add_filter (connection, (DBusHandleMessageFunction)dbus_signal_handler, NULL, NULL)) {
		g_print (_("unable to register filter with the connection"));
		exit(0);
	}
	
	g_strlcpy (nmeamodeandport,
		_("DBUS Mode"), sizeof (nmeamodeandport));
	dbus_connection_setup_with_g_main (connection, NULL);
}

#endif

/* ******************************************************************
 * Initialize nmea socket to 
 * look if we have an open socket "sock" and close it
 * the try to reconnect to gpsd
 * If we're successfull we set
 *    nmeamodeandport = "Sting with description of connection"
 *    sock = the open socket
 * if we're not successfull we set
 *       simmode = TRUE
 *       haveNMEA = FALSE
 */
void
init_nmea_socket ()
{
  struct sockaddr_in server;
  struct hostent *server_data;
  {
    /*  open socket to port */
    if (sock != -1)
      {
	socket_close (sock);
	sock = -1;
      }
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
      {
	perror (_("can't open socket for port "));
	fprintf (stderr, "error: %d\n", errno);
	if (local_config.simmode == SIM_AUTO)
		current.simmode = TRUE;
	haveNMEA = FALSE;
	newsatslevel = TRUE;
	if (simpos_timeout == 0)
	  simpos_timeout =
	    gtk_timeout_add (300, (GtkFunction) simulated_pos, 0);
	memset (satlist, 0, sizeof (satlist));
	memset (satlistdisp, 0, sizeof (satlist));
	sats_used = sats_in_view = 0;
	if (satsimage != NULL)
	  g_object_unref (satsimage);
	satsimage = NULL;
	return;
      }
    server.sin_family = AF_INET;
    /*  We retrieve the IP address of the server from its name: */
    if ((server_data = gethostbyname (SERVERNAME)) == NULL)
      {
	fprintf (stderr,
		 _("\nCannot connect to %s: unknown host\n"), SERVERNAME);
	exit (1);
      }
    memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
    server.sin_port = htons (SERVERPORT2);

    /*  We initiate the connection  */
    if (connect (sock, (struct sockaddr *) &server, sizeof server) < 0)
      {
	server.sin_port = htons (SERVERPORT);	/* We try second port */
	/*  We initiate the connection  */
	if (connect (sock, (struct sockaddr *) &server, sizeof server) < 0)
	{
	    haveNMEA = FALSE;
	    if (local_config.simmode == SIM_AUTO)
	    	current.simmode = TRUE;
	}
	else
	{
	    timeoutcount = 0;
	    haveNMEA = TRUE;
	    if (local_config.simmode == SIM_AUTO)
	    	current.simmode = FALSE;
	    g_strlcpy (nmeamodeandport,
		       _("NMEA Mode, Port 2222"), sizeof (nmeamodeandport));
	    g_strlcat (nmeamodeandport, "/", sizeof (nmeamodeandport));
	    g_strlcat (nmeamodeandport, gpsdservername,
		       sizeof (nmeamodeandport));
	  }
      }
    else
      {
	g_strlcpy (nmeamodeandport, _("NMEA Mode, Port 2947"),
		   sizeof (nmeamodeandport));
	g_strlcat (nmeamodeandport, "/", sizeof (nmeamodeandport));
	g_strlcat (nmeamodeandport, gpsdservername, sizeof (nmeamodeandport));
	send (sock, "R\n", 2, 0);
	timeoutcount = 0;
	haveNMEA = TRUE;
	if (local_config.simmode == SIM_AUTO)
		current.simmode = FALSE;
      }
  }
}


/* ******************************************************************
 */
gint
initgps ()
{

#ifdef DBUS_ENABLE
  if (useDBUS) {
      init_dbus();
  } else
#endif

  init_nmea_socket ();
  /*  We test for gpsd serving */

  if (haveNMEA)
    {
    	if (local_config.simmode == SIM_AUTO)
    		current.simmode = FALSE;
      if (simpos_timeout != 0)
	{
	  gtk_timeout_remove (simpos_timeout);
	  simpos_timeout = 0;
	}
    }

  return FALSE;			/* to remove timer */
}


/* *****************************************************************************
 */
#ifdef DBUS_ENABLE
static DBusHandlerResult dbus_signal_handler (
		DBusConnection* connection, DBusMessage* message) {
	/* dummy, need to use the variable for some reason */
	connection = NULL;
	
	if (dbus_message_is_signal (message, "org.gpsd", "fix")) 
		return dbus_handle_gps_fix (message);
	/*
	 * ignore all other messages
	 */
	
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void
dbus_process_fix(gint early)
{
	struct tm 	time;
	time_t		ttime;

	if (!early && (dbus_current_fix.mode==-1)) {
		/* We have handled this one, so clean the mode and bail out */
		dbus_current_fix.mode = 0;
		return;
	}
	
	/* 
	 * Handle the data
	 */
	/* Set utctime */
	ttime = dbus_current_fix.time;
	gmtime_r (&ttime, &time);
	g_snprintf (utctime, sizeof (utctime), "%02d:%02d.%02d ", time.tm_hour, time.tm_min, time.tm_sec);	
	g_snprintf (loctime, sizeof (loctime), "%02d:%02d ", (time.tm_hour+zone+24)%24, time.tm_min);
	NMEAsecs = dbus_current_fix.time;	// Use this value to judge timeout
	/* Bail out if we have no fix */
	current.gpsfix = 0;	// Handled later.
	if (dbus_current_fix.mode>1) {
		current.gpsfix = dbus_current_fix.mode;
		haveRMCsentence = TRUE;
		satfix = 1;
		haveposcount++;
		if (haveposcount == 3)
			rebuildtracklist();
	} else {
		current.gpsfix = 1;
		haveRMCsentence = FALSE;
		satfix = 0;
		haveposcount = 0;
		dbus_old_fix = dbus_current_fix;
		init_dbus_current_fix();
		if (early)
			dbus_current_fix.mode = -1;
		storepoint();
		return;
	}
	/* Handle latitude */
	if (!gui_status.posmode && !current.simmode)
		coords.current_lat = dbus_current_fix.latitude;
	/* Handle longitude */
	if (!gui_status.posmode && !current.simmode)
		coords.current_lon = dbus_current_fix.longitude;
	/* Handle speed */
	if (__finite(dbus_current_fix.speed))
		groundspeed = dbus_current_fix.speed * 3.6;	// Convert m/s to km/h
	else if (dbus_old_fix.mode>1) {
		gdouble timediff = dbus_current_fix.time-dbus_old_fix.time;
		groundspeed = (timediff>0)?(calcdist2(dbus_old_fix.longitude, dbus_old_fix.latitude) * 3600 / timediff) : 0.0;
	}
	/* Handle bearing */
	if (__finite(dbus_current_fix.track))
		direction = dbus_current_fix.track * M_PI / 180;	// Convert to radians
	else if (dbus_old_fix.mode>1) {
		gdouble lon2 = coords.current_lon * M_PI / 180;
		gdouble lon1 = dbus_old_fix.longitude * M_PI / 180;
		gdouble lat2 = coords.current_lat * M_PI /180;
		gdouble lat1 = dbus_old_fix.latitude * M_PI / 180;
		if ((lat1 != lat2) || (lon1 != lon2))
			direction = atan2(sin(lon2-lon1)*cos(lat2),
				cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1));
	}
	if ( mydebug + gps_handler_debug > 80 )
		g_print("gps_handler: DBUS fix: %6.0f %10.6f/%10.6f sp:%5.2f(%5.2f) crs:%5.1f(%5.2f)\n", dbus_current_fix.time, 
			dbus_current_fix.latitude, dbus_current_fix.longitude, dbus_current_fix.speed, groundspeed, 
			dbus_current_fix.track, direction * 180 / M_PI);
	/* Handle altitude */
	if (dbus_current_fix.mode>2) {
		current.gpsfix = 3;
		current.altitude = dbus_current_fix.altitude;
	}
	/* Handle positional error */
	precision = dbus_current_fix.eph;
	
	dbus_old_fix = dbus_current_fix;
	init_dbus_current_fix();
	if (early)
		dbus_current_fix.mode = -1;
	storepoint();
}

static DBusHandlerResult dbus_handle_gps_fix (DBusMessage* message) {
	DBusMessageIter	iter;
	//double		temp_time;
	//char 		b[100];
	struct dbus_gps_fix	fix;
	//gint32		mode;
	//gdouble		dump;
	
	if (!dbus_message_iter_init (message, &iter)) {
		/* we have a problem */
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	/* Fill the fix struct */
	fix.time		= floor(dbus_message_iter_get_double (&iter));
	dbus_message_iter_next (&iter);
	fix.mode		= dbus_message_iter_get_int32 (&iter);
	dbus_message_iter_next (&iter);
	fix.ept		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.latitude	= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.longitude	= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.eph		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.altitude		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.epv		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.track		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.epd		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.speed		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.eps		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.climb		= dbus_message_iter_get_double (&iter);
	dbus_message_iter_next (&iter);
	fix.epc		= dbus_message_iter_get_double (&iter);

	if ( mydebug + gps_handler_debug > 80 ) {
		g_print("gps_handler: DBUS raw: ti:%6.0f mode:%d ept:%f %10.6f/%10.6f eph:%f\n", fix.time, fix.mode, fix.ept, fix.latitude, fix.longitude, fix.eph);
		g_print("          alt:%6.2f epv:%f crs:%5.1f edp:%f sp:%5.2f eps:%f cl:%f epc:%f\n", fix.altitude, fix.epv, fix.track, fix.epd, fix.speed, fix.eps, fix.climb, fix.epc);
	}
		
	/* Unfortunately gpsd dbus data is sometimes split over several messages */
	/* so we have to accumulate them... */
	
	if (fix.time!=dbus_current_fix.time)
		dbus_process_fix(FALSE);
	// if mode is -1 we already processed it, so bail out
	if (dbus_current_fix.mode==-1)
		return DBUS_HANDLER_RESULT_HANDLED;
	dbus_current_fix.time = fix.time;
	if (__finite(fix.latitude))
		if (__isnan(dbus_current_fix.latitude) || (fix.mode>dbus_current_fix.mode))
			dbus_current_fix.latitude = fix.latitude;
	if (__finite(fix.longitude))
		if (__isnan(dbus_current_fix.longitude) || (fix.mode>dbus_current_fix.mode))
			dbus_current_fix.longitude = fix.longitude;
	if (__finite(fix.eph))
		if (__isnan(dbus_current_fix.eph) || (fix.mode>dbus_current_fix.mode))
			dbus_current_fix.eph = fix.eph;
	if (__finite(fix.altitude))
		if (__isnan(dbus_current_fix.altitude) || (fix.mode>dbus_current_fix.mode))
			dbus_current_fix.altitude = fix.altitude;
	if (__finite(fix.track))
		if (__isnan(dbus_current_fix.track) || (fix.mode>dbus_current_fix.mode))
			dbus_current_fix.track = fix.track;
	if (__finite(fix.speed))
		if (__isnan(dbus_current_fix.speed) || (fix.mode>dbus_current_fix.mode))
			dbus_current_fix.speed = fix.speed;
	if (fix.mode>dbus_current_fix.mode)
		dbus_current_fix.mode = fix.mode;
	
	/* Do an early process if we have all important data to prevent lag. */
	/* We do not consider positional error and altitude important for gpsdrive =) */
	/* To soothe everybody, usually a valid altitude comes with the fix */
	if ((dbus_current_fix.mode>1) && __finite(dbus_current_fix.latitude) && __finite(dbus_current_fix.longitude)
		&& __finite(dbus_current_fix.track) && __finite(dbus_current_fix.speed))
			dbus_process_fix(TRUE);
	
	return DBUS_HANDLER_RESULT_HANDLED;
}
#endif

gint
get_position_data_cb (GtkWidget * widget, guint * datum)
{
  gint e = 0, j = 0, i = 0, found = 0, foundGSV = 0, foundGGA =
    0, foundGSA = 0, foundRME = 0;
  gdouble secs = 0, tx, ty, lastdirection;
  typedef struct
  {
    gchar *a1;
    gchar *a2;
    gchar *a3;
  }
  argument;
  argument *argumente;
  char tok[1000];
  int tilimit;

  if (current.importactive)
    return TRUE;


  if ((timeoutcount > 30 - mydebug - gps_handler_debug))
    g_print ("*** %d. timeout getting data from GPS-Receiver!\n",
	     timeoutcount);

    tilimit = 10;

  if (timeoutcount > tilimit) initgps();
 
  if (timeoutcount > tilimit)
    {
      gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
      gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id,
		_("Timeout getting data from GPS-Receiver!"));
      current.gpsfix = 0;
      haveposcount = 0;
      memset (satlist, 0, sizeof (satlist));
      memset (satlistdisp, 0, sizeof (satlist));
    }


  argumente = NULL;
  if (!haveRMCsentence)
    {
	secs = NMEAsecs;

      if (secs >= 1.0)
	{
	  tx = (2 * earthr * M_PI / 360) * cos (M_PI * coords.current_lat /
						180.0) *
	    (coords.current_lon - coords.old_lon);
	  ty = (2 * earthr * M_PI / 360) * (coords.current_lat - coords.old_lat);
#define MINMOVE 4.0
	  if (((fabs (tx)) > MINMOVE) || (((fabs (ty)) > MINMOVE)))
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

	      current.groundspeed = milesconv * sqrt (tx * tx + ty * ty) * 3.6 / secs;
	      coords.old_lat = coords.current_lat;
	      coords.old_lon = coords.current_lon;
	    }
	  else if (secs > 4.0)
	    current.groundspeed = 0.0;

	  if (current.groundspeed > 2000)
	    current.groundspeed = 0;
	  if (current.groundspeed < 3.6)
	    current.groundspeed = 0;

	  if ( mydebug + gps_handler_debug > 80 )
	    g_print ("gps_handler: Time: %f\n", secs);
	}

      /*  display status line */
      if (gui_status.posmode)
	display_status (_("Press middle mouse button for navigation"));

    }

  /*    only if we have no NMEA, means no measured position */
  if (!haveNMEA)
    {
      /*  display status line */
      if (!current.simmode)
	display_status (_("No GPS used"));
      else if (maploaded && !gui_status.posmode)
	display_status (_("Simulation mode"));
      else if (gui_status.posmode)
	display_status (_("Press middle mouse button for sim mode"));


      return TRUE;
    }

#ifdef DBUS_ENABLE
  if (useDBUS) {
	if (NMEAoldsecs == floor(NMEAsecs)) {
		timeoutcount++;
		return TRUE;
	}
	NMEAoldsecs = floor(NMEAsecs);
	timeoutcount = 0;
	if (current.gpsfix > 1) {
		if (gui_status.posmode)
			display_status (_("Press middle mouse button for navigation"));
		else
			display_status (nmeamodeandport);
	} else 
		display_status(_("Not enough satellites in view!"));
  } else {
#endif
  /*  this is the NMEA reading part. data comes from port 2222 served by gpsd */
  FD_ZERO (&readmask);
  FD_SET (sock, &readmask);
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  if (select (FD_SETSIZE, &readmask, NULL, NULL, &timeout) < 0)
  {
      timeoutcount++;
      perror ("select() call");
  }
  if (FD_ISSET (sock, &readmask))
    {
      if ((e = recv(sock, buffer, 2000, 0)) <= 0) {
	perror ("read from gpsd connection");
	timeoutcount ++;
      }
      buffer[e] = 0;
      if ((bigp + e) < MAXBIG)
	{
	  if (mydebug + gps_handler_debug>30)
	      g_print ("gps_handler: gpsd: !!bigp:%d, e: %d!! , strlen big:%Zu\n"
		       , bigp, e, strlen (big));
	  g_strlcat (big, buffer, MAXBIG);
	  bigp += e;
	  bigpGSA = bigpRME = bigpGSV = bigpGGA = bigp;

	  for (i = bigp; i > lastp; i--)
	    {
	      if (big[i] == '$')
		{
		  /*  collect string for position data $GPRMC */
		  if ((big[i + 3] == 'R')
		      && (big[i + 4] == 'M') && (big[i + 5] == 'C'))
		    {
		      found = 0;
		      if ( mydebug + gps_handler_debug > 80 )
			g_print ("gps_handler: gpsd: found #RMC#\n");
		      for (j = i; j <= bigp; j++)
			if (big[j] == 13)
			  {
			    found = j;
			    break;
			  }
		      if (found != 0)
			{
			  strncpy (tok, (big + i), found - i);
			  tok[found - i] = 0;
			  lastp = found;
			  timeoutcount = 0;

			  /*  we have the $GPRMC string completed, now parse it */
			  if (checksum (tok) == TRUE)
			    convertRMC (tok);

			  /*  display the position and map in the statusline */
			  if (current.gpsfix > 1)
			    {
			      if (gui_status.posmode)
				display_status
				  (_
				   ("Press middle mouse button for navigation"));
			      else
				display_status (nmeamodeandport);
			    }
			  else
			    display_status
			      (_("Not enough satellites in view!"));
			}
		    }
		}
	    }

	  /*  collect string for satellite data $GPGSV */
	  for (i = bigpGSV; i > lastpGSV; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 3] == 'G')
		      && (big[i + 4] == 'S') && (big[i + 5] == 'V'))
		    {
		      foundGSV = 0;
		      if ( mydebug + gps_handler_debug > 80 )
			g_print ("gpsd: found #GSV#, bigpGSV: %d\n", bigpGSV);
		      for (j = i; j <= bigpGSV; j++)
			if (big[j] == 13)
			  {
			    foundGSV = j;
			    break;
			  }
		      if (foundGSV != 0)
			{
			  gint lenstr;
			  lenstr = foundGSV - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("Error in line %d, found GSV=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSV, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundGSV)
			    {
			      g_print
				("Error in line %d, found GSV=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSV, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr < 0)
			    {
			      g_print
				("Error in line %d, foundGSV=%d,i=%d, lenstr=%d\n",
				 __LINE__, foundGSV, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;

			      if (checksum (tok) == TRUE)
				{
				  if ((convertGSV (tok)) == TRUE)
				    lastpGSV = foundGSV;
				}
			    }
			}
		    }
		}
	    }


	  /*  collect string for altitude from  $GPGGA if available */
	  for (i = bigpGGA; i > lastpGGA; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 3] == 'G')
		      && (big[i + 4] == 'G') && (big[i + 5] == 'A'))
		    {
		      foundGGA = 0;
		      if ( mydebug + gps_handler_debug > 80 )
			g_print ("gps_handler: gpsd: found #GGA#\n");
		      timeoutcount = 0;

		      for (j = i; j <= bigpGGA; j++)
			if (big[j] == 13)
			  {
			    foundGGA = j;
			    break;
			  }
		      if (foundGGA != 0)
			{
			  gint lenstr;
			  lenstr = foundGGA - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("Error in line %d, foundGGA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGGA, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundGGA)
			    {
			      g_print
				("Error in line %d, foundGGA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGGA, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;
			      lastpGGA = foundGGA;

			      if (checksum (tok) == TRUE)
				convertGGA (tok);
			      if (current.gpsfix > 1)
				{
				  if (gui_status.posmode)
				    display_status
				      (_
				       ("Press middle mouse button for navigation"));
				  else
				    display_status (nmeamodeandport);
				}
			      else
				display_status
				  (_("Not enough satellites in view!"));

			    }
			}
		    }
		}
	    }

	  /*  collect string for precision from  $PGRME if available */
	  for (i = bigpRME; i > lastpRME; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 1] == 'P')
		      && (big[i + 2] == 'G')
		      && (big[i + 3] == 'R')
		      && (big[i + 4] == 'M') && (big[i + 5] == 'E'))
		    {
		      foundRME = 0;
		      if ( mydebug + gps_handler_debug > 80 )
			g_print ("gps_handler: gpsd: found #RME#\n");
		      for (j = i; j <= bigpRME; j++)
			if (big[j] == 13)
			  {
			    foundRME = j;
			    break;
			  }
		      if (foundRME != 0)
			{
			  gint lenstr;
			  lenstr = foundRME - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("Error in line %d, foundRME=%d,i=%d, diff=%d\n",
				 __LINE__, foundRME, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundRME)
			    {
			      g_print
				("Error in line %d, foundRME=%d,i=%d, diff=%d\n",
				 __LINE__, foundRME, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;
			      lastpRME = foundRME;

			      if (checksum (tok) == TRUE)
				convertRME (tok);
			    }
			}
		    }
		}
	    }

	  /*  collect string for precision from  $GPGSA if available */
	  for (i = bigpGSA; i > lastpGSA; i--)
	    {
	      if (big[i] == '$')
		{

		  if ((big[i + 1] == 'G')
		      && (big[i + 2] == 'P')
		      && (big[i + 3] == 'G')
		      && (big[i + 4] == 'S') && (big[i + 5] == 'A'))
		    {
		      foundGSA = 0;
		      if ( mydebug + gps_handler_debug > 80 )
			g_print ("gps_handler: gpsd: found #GSA#\n");
		      for (j = i; j <= bigpGSA; j++)
			if (big[j] == 13)
			  {
			    foundGSA = j;
			    break;
			  }
		      if (foundGSA != 0)
			{
			  gint lenstr;
			  lenstr = foundGSA - i;
			  if ((lenstr) > 200)
			    {
			      g_print
				("Error in line %d, foundGSA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSA, i, lenstr);
			      lenstr = 200;
			    }
			  if (i > foundGSA)
			    {
			      g_print
				("Error in line %d, foundGSA=%d,i=%d, diff=%d\n",
				 __LINE__, foundGSA, i, lenstr);
			      lenstr = 0;
			    }
			  if (lenstr != 0)
			    {
			      strncpy (tok, (big + i), lenstr);
			      tok[lenstr] = 0;
			      lastpGSA = foundGSA;

			      if (checksum (tok) == TRUE)
				convertGSA (tok);
			    }
			}
		    }
		}
	    }



	  if ( mydebug + gps_handler_debug > 80 )
	    {
	      g_print ("gps_handler: gpsd: size:%d lastp: %d \n", e, lastp);
	      g_print ("gps_handler: gpsd: buffer: %s\n", buffer);
	    }

	}
      else
	{
	  lastp = lastpGGA = lastpGSV = lastpRME = lastpGSA = 0;
	  bigp = e;
	  g_strlcpy (big, buffer, MAXBIG);
	}

    }
  else
    {
      timeoutcount++;
    }
#ifdef DBUS_ENABLE
  }
#endif


  return (TRUE);
}
