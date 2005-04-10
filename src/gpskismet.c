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

$Log$
Revision 1.2  2005/04/10 21:50:50  tweety
reformatting c-sources

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.24  2004/04/05 18:45:41  ganter
added patch for kismet hangs and Mac-OS mouse
patches provided by Ulrich Hecht from SUSE.

Revision 1.23  2004/02/08 17:16:25  ganter
replacing all strcat with g_strlcat to avoid buffer overflows

Revision 1.22  2004/02/08 16:35:10  ganter
replacing all sprintf with g_snprintf to avoid buffer overflows

Revision 1.21  2004/02/07 15:53:38  ganter
replacing strcpy with g_strlcpy to avoid bufferoverflows

Revision 1.20  2004/02/02 03:38:32  ganter
code cleanup

Revision 1.19  2004/01/28 15:31:43  ganter
initialize FDs to -1

Revision 1.18  2004/01/28 09:32:57  ganter
tested for memory leaks with valgrind, looks good :-)

Revision 1.17  2004/01/05 05:52:58  ganter
changed all frames to respect setting

Revision 1.16  2004/01/01 09:07:33  ganter
v2.06
trip info is now live updated
added cpu temperature display for acpi
added tooltips for battery and temperature

Revision 1.15  2003/08/31 17:37:58  ganter
v 2.04: better Kismet support, read end of README.kismet

Revision 1.14  2003/08/12 14:21:18  ganter
v2.03
fixed kismet bug (wrong GPS position)
compiles also on SuSE 8.1
compiles on GTK+ >= 2.0.6
fixed wrong font (Sans 10 Bold 10 message)

Revision 1.13  2003/01/15 17:03:17  ganter
MySQL is now loaded dynamically on runtime, no mysql needed for compile.
Needs only libmysqlclient.so now.

Revision 1.12  2003/01/15 15:30:28  ganter
before dynamically loading mysql

Revision 1.11  2002/12/08 03:18:26  ganter
shortly before 1.31

Revision 1.10  2002/11/27 00:02:27  ganter
1.31pre2

Revision 1.9  2002/11/14 00:05:55  ganter
added README.kismet
v 1.30pre5

Revision 1.8  2002/11/12 20:37:15  ganter
v30pre4
added more icons, fix for kismet w/o mysql

Revision 1.7  2002/11/06 05:29:15  ganter
fixed most warnings

Revision 1.6  2002/11/06 01:44:15  ganter
v1.30pre2

Revision 1.5  2002/11/05 17:04:04  ganter
...

Revision 1.4  2002/11/05 02:29:59  ganter
bugfixes for kismet mode

Revision 1.3  2002/11/05 00:02:42  ganter
...

Revision 1.2  2002/11/05 00:00:14  ganter
gpskismet seems to work

Revision 1.1  2002/11/04 18:01:53  ganter
added gpskismet.c


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


#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char dbtable[MAXDBNAME], dbname[MAXDBNAME];
extern char dbwherestring[5000];
extern char dbtypelist[100][40];
extern double dbdistance;
extern int dbtypelistcount;
extern int usesql;
extern int debug, dbusedist;
extern gchar homedir[500], mapdir[500];

extern MYSQL mysql;
MYSQL_RES *res;
MYSQL_ROW row;

static char macaddr[30], name[80], tbuf[500], lastmacaddr[30];
static int nettype, channel, wep;

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

/* variables */
extern gdouble current_long, current_lat, old_long, old_lat, groundspeed;
extern gdouble zero_long, zero_lat, target_long, target_lat, dist;
int kismetsock = -1, havekismet;
static char kbuffer[20010];
static int bc = 0;
fd_set kismetreadmask;
struct timeval kismettimeout;
static char lat[30], lon[30], bestlat[30], bestlon[30];
enum
	{ english, german, spanish }
voicelang;

#define KISMETSERVERNAME "localhost"


int
readkismet (void)
{
  signed char c;
  char q[1200], buf[300], tname[80], sqllat[30], sqllon[30];
  int e, r, have, i, j, sqlid = 0;

  e = 0;
  FD_ZERO (&kismetreadmask);
  FD_SET (kismetsock, &kismetreadmask);
  kismettimeout.tv_sec = 0;
  kismettimeout.tv_usec = 10000;

  do
    {
      if (select (FD_SETSIZE, &kismetreadmask, NULL, NULL, &kismettimeout) <
					0)
				{
					perror ("select() call");
					return FALSE;
				}

      if ((have = FD_ISSET (kismetsock, &kismetreadmask)))
				{
					while ((e = read (kismetsock, &c, 1)) > 0)
						{
							if (c != '\n')
								*(kbuffer + bc++) = c;
							else
								{
									c = -1;
									g_strlcat (kbuffer, "\n", sizeof(kbuffer));
									/* 	g_print("\nfinished: %d",bc); */
									break;
								}
							if (bc > 20000)
								{
									bc = 0;
									g_print ("kbuffer overflow!\n");
								}

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
							e =
								sscanf (kbuffer,
												"%s %s %d \001%255[^\001]\001 %d"
												" %d  %s %s %s %s %[^\n]", tbuf, macaddr, &nettype,
												name, &channel, &wep, lat, lon, bestlat, bestlon,
												tbuf);

						}
					if (e == 10)
						{
							if (debug)
								g_print
									("\ne: %d mac: %s nettype: %d name: %s channel: %d wep: %d "
									 "lat: %s lon: %s bestlat: %s bestlon: %s\n", e, macaddr,
									 nettype, name, channel, wep, lat, lon, bestlat, bestlon);

							/* insert waypoint only if we had not just inserted it */
							/* 	      if ((strcmp (lastmacaddr, macaddr)) != 0) */
							{
								/* 		  g_strlcpy (lastmacaddr, macaddr); */
								g_snprintf (q, sizeof (q),
														"select id,lat,lon from %s where macaddr='%s'",
														dbtable, macaddr);
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
										if (name[i] != '\'')
											tname[j++] = name[i];
										else
											{
												tname[j++] = '\\';
												tname[j++] = '\'';
											}
									}


								/* 		  we have it in the database, but update bestlat and bestlong */
								if (r > 0)
									if ((strcmp (sqllat, lat) != 0) &&
											(strcmp (sqllon, lon) != 0))
										{
											if ((atol (bestlat) != 0.0) && (atol (bestlon) != 0))
												if ((strcmp (lat, "90.000000") != 0) &&
														(strcmp (lon, "180.000000") != 0))
													{
														if (debug)
															g_print
																("*** This is a changed waypoint: %s [%s]\n",
																 name, macaddr);

														g_snprintf (q, sizeof (q),
																				"UPDATE %s SET name='%s',macaddr='%s',nettype='%d',lat='%s',lon='%s',type='%s',wep='%d' WHERE id='%d'",
																				dbtable, tname, macaddr, nettype,
																				bestlat, bestlon,
																				(wep) ? "WLAN-WEP" : "WLAN", wep,
																				sqlid);
														if (debug)
															printf ("\nquery: %s\n", q);
														if (dl_mysql_query (&mysql, q))
															exiterr (3);
													}
										}


								/* 		  this is a new network, we store it in the database */
								if ((r == 0) && (strcmp (lat, "90.000000") != 0) &&
										(strcmp (lon, "180.000000") != 0))
									{
										g_strlcpy (lastmacaddr, macaddr, sizeof (lastmacaddr));
										if (debug)
											g_print ("*** This is a new waypoint: %s [%s]\n", name,
															 macaddr);

										g_snprintf (q, sizeof (q),
																"INSERT INTO %s (name,macaddr,nettype,lat,lon,type,wep)"
																" VALUES ('%s','%s','%d','%s','%s','%s','%d')",
																dbtable, tname, macaddr, nettype, lat, lon,
																(wep) ? "WLAN-WEP" : "WLAN", wep);
										if (debug)
											printf ("\nquery: %s\n", q);
										if (dl_mysql_query (&mysql, q))
											exiterr (3);

										g_strdelimit (name, "_", ' ');
										switch (voicelang)
											{
											case english:
												g_snprintf (buf, sizeof (buf),
																		"Found new %s access point: %s",
																		(wep) ? "crypted" : "open", name);
												break;
											case spanish:
												g_snprintf (buf, sizeof (buf),
																		"Found new %s access point: %s",
																		(wep) ? "closed" : "open", name);
												break;
											case german:
												g_snprintf (buf, sizeof (buf),
																		"Es wurde ein neuer  %s exses point gefunden: %s",
																		(wep) ? "verschlüsselter" : "offener",
																		name);
												break;
											}
										speech_out_speek (buf);
										/* if (debug) */
										/* 		    printf (_("rows inserted: %d\n"), r); */
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
      return -1;
    }
  memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
  server.sin_port = htons (2501);
	/*  We initiate the connection  */
  if (connect (kismetsock, (struct sockaddr *) &server, sizeof server) < 0)
    {
      close (kismetsock);

      return -1;
    }
  else
    {
      havekismet = TRUE;
      g_strlcpy (buf,
								 "!0 ENABLE NETWORK bssid,type,ssid,channel,wep,minlat,minlon,bestlat,bestlon\n",
								 sizeof (buf));
      write (kismetsock, buf, strlen (buf));
    }

  return TRUE;
}
