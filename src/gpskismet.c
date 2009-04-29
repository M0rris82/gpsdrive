/****************************************************************

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

    *********************************************************************


reads info from kismet server and insert waypoints into database

*****************************************************************/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>
#include <glib/gstdio.h>

#include <gpsdrive.h>
#include "gpsdrive_config.h"
/* #include <gpskismet.h> */
#include <poi.h>
#include <time.h>
#include <errno.h>
#include "database.h"
#include "gpskismet.h"

#ifdef SPEECH
#include "speech.h"
#endif

extern int mydebug, debug;
extern currentstatus_struct current;

static char macaddr[30], tbuf[1024], lastmacaddr[30];
static int nettype, channel, wep, cloaked;

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

/* variables */
static char kbuffer[20210];
static int bc = 0;
fd_set kismetreadmask;
struct timeval kismettimeout;
static char lat[30], lon[30], bestlat[30], bestlon[30];
time_t last_initkismet=0;


int
readkismet ()
{
	//TODO:	source_id for kismet is currently hardcoded to 9
	//	maybe we should ask geoinfo.db the value at startup

  signed char c;
  char buf[4*300], tname[4*80];
  // make buffers 4 times as large as expeted, since I think kismet 
  // encodes a \001 as 4 ASCII characters. And this would trigger a
  // buffer overflow.
  int e, have;
  glong sqlid = 0;
  gchar *result = NULL;
  gchar *t_ptype = NULL;
  gchar t_buf[5];
  gdouble bestlat_dbl, bestlon_dbl;
  const gchar *type_string[] = {"infrastructure","ad-hoc","probe","data","turbocell","unknown"};
  const gchar *poi_type[] = {"wlan.open","wlan.wep","wlan.closed"};

  // If Kismet server connection failed, Try to reconnect
  //    after at least 30 seconds
  if ((current.kismetsock<0) && ((time(NULL) - last_initkismet) > 30)) {
     if (debug) g_print(_("trying to re-connect to kismet server\n"));
     current.kismetsock = initkismet();
     if (current.kismetsock>=0) g_print(_("Kismet server connection re-established\n"));
     if (debug) g_print(_("done trying to re-connect: socket=%d\n"), current.kismetsock);
     }

  if (current.kismetsock < 0) return -1;

  do
    {
      e = 0;
      FD_ZERO (&kismetreadmask);
      FD_SET (current.kismetsock, &kismetreadmask);
      kismettimeout.tv_sec = 0;
      kismettimeout.tv_usec = 10000;

      if (select
	  (FD_SETSIZE, &kismetreadmask, NULL, NULL, &kismettimeout) < 0)
	{
	  perror ("readkismet: select() call");
	  return FALSE;
	}

      if ((have = FD_ISSET (current.kismetsock, &kismetreadmask)))
	{
	  int bytesread;
	  bytesread=0;
	  while ((e = read (current.kismetsock, &c, 1)) > 0)
	    {
	      bytesread++;
	      if (c != '\n')
		*(kbuffer + bc++) = c;
	      else
		{
		  c = -1;
		  g_strlcat (kbuffer, "\n", sizeof (kbuffer));
		  /*  g_print("\nfinished: %d",bc); */
		  break;
		}
	      if (bc > 20000)
		{
		  bc = 0;
		  g_print ("kbuffer overflow!\n");
		}

	    }

	  // the file descriptor was ready for read but no data available...
	  // this means the connection was lost.
	  if (bytesread==0) {
		g_print(_("Kismet server connection lost\n"));
		close(current.kismetsock);
		return -1;
	     }
	}

      if (c == -1)
	{
	  /* have read a line */
	  bc = c = 0;
	  if ((strstr (kbuffer, "*NETWORK:")) == kbuffer)
	    {
	      if (debug)
		g_print ("\nkbuffer:%s\n", kbuffer);
	      e = sscanf (kbuffer,
			  "%s %s %d \001%255[^\001]\001 %d"
			  " %d  %s %s %s %s %d %[^\n]", 
			  tbuf, macaddr, &nettype, tname, &channel,
			  &wep, lat, lon, bestlat, bestlon, &cloaked, tbuf);
	      if (debug)
	      {
		  printf ("tbuf: %s\n", tbuf);
		  printf ("macaddr: %s\n",macaddr);
		  printf ("nettype: %d\n",nettype);
		  printf ("name: %s\n", tname );
		  printf ("channel: %d, wep:%d\n", channel, wep);
		  printf ("lat/lon: %s/%s  best lat/lon:%s/%s\n", lat, lon, bestlat, bestlon);
		  printf ("cloaked: %d \n", cloaked);
	      }
	    }
	  if (e == 11)
	    {
		if (mydebug >10)
		{
			g_print ("e: %d mac: %s nettype: %d name: %s channel: %d wep: %d "
				"lat: %s lon: %s bestlat: %s bestlon: %s cloaked: %d\n", e, macaddr,
				nettype, tname, channel, wep, lat, lon, bestlat, bestlon, cloaked);
		}

		/* check, if waypoint is already present in database */
		sqlid = db_poi_extra_get (NULL, "macaddr", macaddr, result);

//		if (r > 1)
//			g_print ("\n\a\a*** ERROR: duplicate macaddr in database ***\n");

		bestlat_dbl = g_strtod (bestlat, NULL);
		bestlon_dbl = g_strtod (bestlon, NULL);
		if (nettype > 5)
			nettype = 5;
		if (wep > 2)
			t_ptype = (gchar *) poi_type[2];
		else
			t_ptype = (gchar *) poi_type[wep];

		/* we have it in the database, but update bestlat and bestlong */
		if (sqlid > 0)
		{
		      if ((bestlat_dbl != 0.0) && (bestlon_dbl != 0))
			if ((strcmp (lat, "90.000000") != 0) && (strcmp (lon, "180.000000") != 0))
			{
				if (mydebug > 20)
					g_printf ("readkismet: updating network"
						" '%s' with macaddr = %s\n", tname, macaddr);
				db_poi_edit (sqlid, bestlat_dbl, bestlon_dbl,
					tname, t_ptype, macaddr, 9, TRUE);
				db_poi_extra_edit (&sqlid, "nettype", type_string[nettype], TRUE);
				g_snprintf (t_buf, sizeof (t_buf), "%d", cloaked);
				db_poi_extra_edit (&sqlid, "cloaked", t_buf, TRUE);
				g_snprintf (t_buf, sizeof (t_buf), "%d", wep);
				db_poi_extra_edit (&sqlid, "wep", t_buf, TRUE);
				g_snprintf (t_buf, sizeof (t_buf), "%d", channel);
				db_poi_extra_edit (&sqlid, "channel", t_buf, TRUE);
			}
		}
		else
		/* this is a new network, we store it in the database */
		if ((strcmp (lat, "90.000000") != 0) && (strcmp (lon, "180.000000") != 0))
		{
			g_strlcpy (lastmacaddr, macaddr, sizeof (lastmacaddr));

			if (mydebug > 20)
				g_printf ("readkismet: adding network '%s' with macaddr = %s\n",
					tname, macaddr);

			sqlid = db_poi_edit (0, g_strtod (lat, NULL), g_strtod (lon, NULL),
				tname, t_ptype, macaddr, 9, FALSE);
			db_poi_extra_edit (&sqlid, "macaddr", macaddr, FALSE);
			db_poi_extra_edit (&sqlid, "nettype", type_string[nettype], FALSE);
			g_snprintf (t_buf, sizeof (t_buf), "%d", cloaked);
			db_poi_extra_edit (&sqlid, "cloaked", t_buf, FALSE);
			g_snprintf (t_buf, sizeof (t_buf), "%d", wep);
			db_poi_extra_edit (&sqlid, "wep", t_buf, FALSE);
			g_snprintf (t_buf, sizeof (t_buf), "%d", channel);
			db_poi_extra_edit (&sqlid, "channel", t_buf, FALSE);

			g_snprintf (buf, sizeof (buf),
				_("Found new %s access point: %s"),
				(wep) ? _("encrypted") : _("open"), tname);
#ifdef SPEECH
			speech_saytext (buf, 3);
#endif
		}
	    }

	  memset (kbuffer, 0, 20000);
	  g_strlcpy (kbuffer, "", sizeof (kbuffer));
	}

    }
  while (have != 0);

  return current.kismetsock;
}

int
initkismet (void)
{
  struct sockaddr_in server;
  struct hostent *server_data;
  char buf[180];
  gint t_sock;
  last_initkismet=time(NULL);

  if (debug) g_print(_("Trying Kismet server\n"));

  g_strlcpy (lastmacaddr, "", sizeof (lastmacaddr));
  /*  open socket to port */
  if ((t_sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror (_("can't open socket for port "));
      return -1;
    }
  server.sin_family = AF_INET;
  /*  We retrieve the IP address of the server from its name: */
  if ((server_data = gethostbyname(local_config.kismet_servername)) == NULL)
    {
      fprintf (stderr, "%s: unknown host", local_config.kismet_servername);
      close (t_sock);
      return -1;
    }
  memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
  server.sin_port = htons (local_config.kismet_serverport);
  /*  We initiate the connection  */
  if (connect (t_sock, (struct sockaddr *) &server, sizeof server) < 0)
    {
      close (t_sock);
      return -1;
    }
  else
    {
      g_strlcpy (buf,
		 "!0 ENABLE NETWORK bssid,type,ssid,channel,wep,minlat,minlon,bestlat,bestlon,cloaked\n",
		 sizeof (buf));
      write (t_sock, buf, strlen (buf));
    }

  return t_sock;
}
