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

#include <gpsdrive.h>
/* #include <gpskismet.h> */
#include <poi.h>
#include <speech_out.h>
#include <speech_strings.h>
#include <time.h>
#include <errno.h>

#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char wlantable[MAXDBNAME], dbname[MAXDBNAME];
extern char dbpoifilter[5000];
extern double dbdistance;
extern int usesql;
extern int debug, dbusedist;
extern poi_type_struct poi_type_list[poi_type_list_max];

extern MYSQL mysql;
MYSQL_RES *res;
MYSQL_ROW row;

static char macaddr[30], name[80], tbuf[500], lastmacaddr[30];
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
int kismetsock = -1, havekismet;
static char kbuffer[20010];
static int bc = 0;
fd_set kismetreadmask;
struct timeval kismettimeout;
static char lat[30], lon[30], bestlat[30], bestlon[30];

int wlan_closed=0;
int wlan_open=0;

#define KISMETSERVERNAME "localhost"

time_t last_initkismet=0;


int
readkismet (void)
{
  signed char c;
  char q[1200], buf[300], tname[80], sqllat[30], sqllon[30];
  int e, r, have, i, j, sqlid = 0;

  // If Kismet server connection failed, Try to reconnect
  //    after at least 30 seconds
  if ((kismetsock<0) && ((time(NULL) - last_initkismet) > 30)) {
     if (debug) g_print(_("trying to re-connect to kismet server\n"));
     initkismet();
     if (kismetsock>=0) g_print(_("Kismet server connection re-established\n"));
     if (debug) g_print(_("done trying to re-connect: socket=%d\n"), kismetsock);
     }

  if (kismetsock < 0) return FALSE;

  do
    {
      e = 0;
      FD_ZERO (&kismetreadmask);
      FD_SET (kismetsock, &kismetreadmask);
      kismettimeout.tv_sec = 0;
      kismettimeout.tv_usec = 10000;

      if (select
	  (FD_SETSIZE, &kismetreadmask, NULL, NULL, &kismettimeout) < 0)
	{
	  perror ("readkismet: select() call");
	  return FALSE;
	}

      if ((have = FD_ISSET (kismetsock, &kismetreadmask)))
	{
	  int bytesread;
	  bytesread=0;
	  while ((e = read (kismetsock, &c, 1)) > 0)
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
		close(kismetsock);
		kismetsock=-1;
		return FALSE;
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
			  " %d  %s %s %s %s %d %[^\n]", tbuf,
			  macaddr, &nettype, name, &channel,
			  &wep, lat, lon, bestlat, bestlon, &cloaked, tbuf);

	    }
	  if (e == 11)
	    {
	      if (debug)
		g_print
		  ("\ne: %d mac: %s nettype: %d name: %s channel: %d wep: %d "
		   "lat: %s lon: %s bestlat: %s bestlon: %s cloaked: %d\n", e, macaddr,
		   nettype, name, channel, wep, lat, lon, bestlat, bestlon, cloaked);

	      /* insert waypoint only if we had not just inserted it */
	      /*        if ((strcmp (lastmacaddr, macaddr)) != 0) */
	      {
		/*                g_strlcpy (lastmacaddr, macaddr); */
		g_snprintf (q, sizeof (q),
			    "select wlan_id,lat,lon from %s where macaddr='%s'",
			    wlantable, macaddr);
		if (debug)
		  g_print ("\nquery: %s\n", q);
		if (dl_mysql_query (&mysql, q))
		  exiterr (3);
		if (!(res = dl_mysql_store_result (&mysql)))
		  exiterr (4);
		r = 0;
		while ((row = dl_mysql_fetch_row (res)))
		  {
		    sqlid = atol (row[0]);
		    g_strlcpy (sqllat, row[1], sizeof (sqllat));
		    g_strlcpy (sqllon, row[2], sizeof (sqllon));

		    r++;
		  }

		if (r > 1)
		  g_print
		    ("\n\a\a*** ERROR: duplicate macaddr in database ***\n");

		dl_mysql_free_result (res);
		if (debug)
		  g_print ("\nnum fields: %d", r);

		if ((strcmp (name, "<no ssid>")) == 0)
		  g_strlcpy (name, "no_ssid", sizeof (name));
		g_strdelimit (name, " ", '_');
		/* escape ' */
		j = 0;
		for (i = 0; i <= (int) strlen (name); i++)
		  {
		    if (name[i] != '\'' && name[i] != '\\' && name[i] != '\"')
		      tname[j++] = name[i];
		    else
		      {
			tname[j++] = '\\';
			tname[j++] = name[i];
			if (debug)
			  g_print ("Orig SSID: %s\nEscaped SSID: %s\n", name,
				   tname);
		      }
		  }


		/*                we have it in the database, but update bestlat and bestlong */
		if (r > 0)
		  if ((strcmp (sqllat, lat) !=
		       0) && (strcmp (sqllon, lon) != 0))
		    {
		      if ((atol (bestlat) != 0.0) && (atol (bestlon) != 0))
			if ((strcmp
			     (lat,
			      "90.000000")
			     != 0) && (strcmp (lon, "180.000000") != 0))
			  {
			    if (debug)
			      g_print
				("*** This is a changed waypoint: %s [%s]\n",
				 name, macaddr);

			    g_snprintf
			      (q,
			       sizeof
			       (q),
			       "UPDATE %s SET essid='%s',macaddr='%s',nettype='%d',lat='%s',lon='%s',poi_type_id='%d',wep='%d,cloaked=%d' WHERE wlan_id='%d'",
			       wlantable,
			       tname,
			       macaddr,
			       nettype,
			       bestlat,
			       bestlon,
			       (wep) ? wlan_closed : wlan_open , wep, cloaked, sqlid);
			    if (debug)
			      printf ("\nquery: %s\n", q);
			    if (dl_mysql_query (&mysql, q))
			      exiterr (3);
			  }
		    }


		/*                this is a new network, we store it in the database */
		if ((r == 0)
		    && (strcmp (lat, "90.000000") !=
			0) && (strcmp (lon, "180.000000") != 0))
		  {
		    g_strlcpy (lastmacaddr, macaddr, sizeof (lastmacaddr));
		    if (debug)
		      g_print ("*** This is a new waypoint: %s [%s]\n", name,
			       macaddr);


		    g_snprintf (q, sizeof (q),
				"INSERT INTO %s (essid,macaddr,nettype,lat,lon,poi_type_id,wep,cloaked)"
				" VALUES ('%s','%s','%d','%s','%s','%d','%d','%d')",
				wlantable, tname,
				macaddr, nettype,
				lat, lon,
				(wep) ? wlan_closed : wlan_open, wep, cloaked);
		    if (debug)
		      printf ("\nquery: %s\n", q);
		    if (dl_mysql_query (&mysql, q))
		      exiterr (3);

		    g_strdelimit (name, "_", ' ');

		    g_snprintf (buf, sizeof (buf),
				speech_found_access_point[voicelang],
				(wep) ? speech_access_closed[voicelang] :
				speech_access_open[voicelang], name, channel);
		    speech_out_speek (buf);
		    /* if (debug) */
		    /*                  printf (_("rows inserted: %d\n"), r); */
		    getsqldata ();
		  }
	      }
	    }

	  memset (kbuffer, 0, 20000);
	  g_strlcpy (kbuffer, "", sizeof (kbuffer));
	}

    }
  while (have != 0);

  return TRUE;
}

int
initkismet (void)
{
  struct sockaddr_in server;
  struct hostent *server_data;
  char buf[180];

  last_initkismet=time(NULL);

  if (debug) g_print(_("Trying Kismet server\n"));

  g_strlcpy (lastmacaddr, "", sizeof (lastmacaddr));
  /*  open socket to port */
  if ((kismetsock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror (_("can't open socket for port "));
      return -1;
    }
  server.sin_family = AF_INET;
  /*  We retrieve the IP address of the server from its name: */
  if ((server_data = gethostbyname (KISMETSERVERNAME)) == NULL)
    {
      fprintf (stderr, "%s: unknown host", KISMETSERVERNAME);
      close (kismetsock);
      kismetsock=-1;
      return -1;
    }
  memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
  server.sin_port = htons (2501);
  /*  We initiate the connection  */
  if (connect (kismetsock, (struct sockaddr *) &server, sizeof server) < 0)
    {
      close (kismetsock);
      kismetsock=-1;
      return -1;
    }
  else
    {
      havekismet = TRUE;
      g_strlcpy (buf,
		 "!0 ENABLE NETWORK bssid,type,ssid,channel,wep,minlat,minlon,bestlat,bestlon,cloaked\n",
		 sizeof (buf));
      write (kismetsock, buf, strlen (buf));
    }

  return TRUE;
}


void get_poi_type_id_for_wlan() {
    int i;
    for (i = 0; i < poi_type_list_max; i++)
      {
	poi_type_list[i].icon = NULL;
	if (strcmp (poi_type_list[i].name,"wlan.closed") == 0){
	    wlan_closed=i;
	} else if (strcmp (poi_type_list[i].name,"wlan.open") == 0){
	    wlan_open=i;
	}
      }
}
