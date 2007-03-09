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
/*
  $Log$
  Revision 1.6  2006/08/02 07:48:24  tweety
  rename variable mapdir --> local_config_mapdir

  Revision 1.5  2006/08/01 06:06:50  tweety
  try to reduce errors while downloading maps from expedia

  Revision 1.4  2006/05/09 08:29:52  tweety
  move proxy fetching from environment to download_map.c

  Revision 1.3  2006/02/17 20:54:34  tweety
  http://bugzilla.gpsdrive.cc/show_bug.cgi?id=73
  Downloading maps doesn't allow Longitude select by mouse

  Revision 1.2  2006/02/05 16:38:05  tweety
  reading floats with scanf looks at the locale LANG=
  so if you have a locale de_DE set reading way.txt results in clearing the
  digits after the '.'
  For now I set the LC_NUMERIC always to en_US, since there we have . defined for numbers

  Revision 1.1  2006/02/05 15:01:59  tweety
  extract map downloading

  Revision 1.0  2006/01/03 14:24:10  tweety
*/


/*  Include Dateien */
#include "../config.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <errno.h>

#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

#include "gettext.h"

#include <dirent.h>


#ifndef NOPLUGINS
#include "gmodule.h"
#endif

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


#if GTK_MINOR_VERSION < 2
#define gdk_draw_pixbuf _gdk_draw_pixbuf
#endif


#include <gpsdrive.h>

extern GtkWidget *mainwindow, *frame_status, *messagestatusbar;
extern gint statusid, messagestatusbarid, timeoutcount;
extern gint haveproxy, proxyport;
extern gchar proxy[256];
extern gint mydebug;
extern gchar local_config_mapdir[500];
extern mapsstruct *maps;
extern struct timeval timeout;
extern int havenasa;
extern gint needtosave;
extern gint slistsize;
extern gchar *slist[];
extern GtkWidget *cover;
extern gdouble current_lon, current_lat, old_lon, old_lat, groundspeed;
extern gdouble zero_lon, zero_lat;
extern gdouble target_lon, target_lat;
extern gint minsecmode;
extern gint scaleprefered, scalewanted;
extern gint milesflag, metricflag, nauticflag;
extern gdouble milesconv;

char actualhostname[200];

gint dlsock = -1;
int expedia_de = 0;
gint expedia = TRUE;
GtkWidget *downloadwindow;

GtkWidget *radio1, *radio2;
gint downloadwindowactive=0;
gint downloadactive=0;
gchar writebuff[2000];
fd_set readmask;
gchar *dlbuff, downloadfilename[512];
gint downloadfilelen=0;
GtkWidget *dl_text_lat, *dl_text_lon, *dltext3, *dltext4;
gchar *dlpstart;
gint nrmaps = 0, dldiff;
gint dlcount;
GtkWidget *myprogress;
gint defaultserver = 0;
gchar newmaplat[100], newmaplon[100], newmapsc[100];

gint downloadaway_cb (GtkWidget * widget, guint datum);
gint defaultserver_cb (GtkWidget * widget, guint datum);
gint dlscale_cb (GtkWidget * widget, guint datum);
gint other_select_cb (GtkWidget * widget, guint datum);

/* *****************************************************************************
 */
gint
dlstatusaway_cb (GtkWidget * widget, guint datum)
{
	downloadwindowactive = downloadactive = FALSE;

	return FALSE;
}

/* *****************************************************************************
 */
char *
getexpediaurl (GtkWidget * widget)
{
    struct sockaddr_in server;
    struct hostent *server_data;
    gchar str[100], sn[1000];
    gchar tmpbuff[9000];
    gint e;
    static char url[8000];

    /*  open socket to port80 */
    if ((dlsock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    perror (_("can't open socket for port 80"));
	    if (expedia_de)
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"),
			    (expedia) ? WEBSERVER4 : WEBSERVER);
	    else
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"),
			    (expedia) ? WEBSERVER2 : WEBSERVER);
	    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    close (dlsock);
	    return (NULL);
	}

    server.sin_family = AF_INET;
    /*  We retrieve the IP address of the server from its name: */
    if (haveproxy)
	g_strlcpy (sn, proxy, sizeof (sn));
    else
	{
	    if (expedia_de)
		g_strlcpy (sn, (expedia) ? WEBSERVER4 : WEBSERVER,
			   sizeof (sn));
	    else
		g_strlcpy (sn, (expedia) ? WEBSERVER2 : WEBSERVER,
			   sizeof (sn));
	}
    if ((server_data = gethostbyname (sn)) == NULL)
	{
	    perror (_("Can't resolve webserver address"));
	    if (expedia_de)
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"),
			    (expedia) ? WEBSERVER4 : WEBSERVER);
	    else
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"),
			    (expedia) ? WEBSERVER2 : WEBSERVER);
	    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    close (dlsock);
	    return (NULL);
	}
    memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
    server.sin_port = htons (proxyport);
    /*  We initiate the connection  */
    if (connect (dlsock, (struct sockaddr *) &server, sizeof server) < 0)
	{
	    perror (_("unable to connect to Website"));
	    if (expedia_de)
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"),
			    (expedia) ? WEBSERVER4 : WEBSERVER);
	    else
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"),
			    (expedia) ? WEBSERVER2 : WEBSERVER);
	    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    close (dlsock);
	    return (NULL);
	}

    if ( write (dlsock, writebuff, strlen (writebuff))<0){
	close (dlsock);
	return (NULL);
    };

    FD_ZERO (&readmask);
    FD_SET (dlsock, &readmask);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    if (select (FD_SETSIZE, &readmask, NULL, NULL, &timeout) < 0)
	{
	    perror ("select() call");
	}

    g_strlcpy (url, "Fehler!!!!", sizeof (url));
    memset (tmpbuff, 0, 8192);
    if ((e = read (dlsock, tmpbuff, 8000)) < 0)
	perror (_("read from Webserver"));
    if ( mydebug > 3 )
	g_print ("Loaded %d Bytes\n", e);
    if (e > 0)
	g_strlcpy (url, tmpbuff, sizeof (url));
    else
	{
	    perror ("getexpediaurl");
	    fprintf (stderr, "error while reading from exedia\n");
	    close (dlsock);
	    return (NULL);
	    // exit (1);
	}
    close (dlsock);
    return url;

}

/* *****************************************************************************
 */
gint
downloadstart_cb (GtkWidget * widget, guint datum)
{
    struct sockaddr_in server;
    struct hostent *server_data;
    gchar str[100], sn[1000];


    downloadfilelen = 0;
    downloadactive = TRUE;
    if (!expedia)
	g_snprintf (str, sizeof (str), _("Connecting to %s"),
		    WEBSERVER);
    if (expedia)
	{
	    if (expedia_de)
		g_snprintf (str, sizeof (str), _("Connecting to %s"),
			    WEBSERVER4);
	    else
		g_snprintf (str, sizeof (str), _("Connecting to %s"),
			    WEBSERVER2);
	}

    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
    while (gtk_events_pending ())
	gtk_main_iteration ();
    /*  open socket to port80 */
    if ((dlsock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
	    perror (_("can't open socket for port 80"));
	    if (!expedia)
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"), WEBSERVER);
	    if (expedia)
		{
		    if (expedia_de)
			g_snprintf (str, sizeof (str),
				    _("Connecting to %s FAILED!"),
				    WEBSERVER4);
		    else
			g_snprintf (str, sizeof (str),
				    _("Connecting to %s FAILED!"),
				    WEBSERVER2);
		}

	    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (6000, (GtkFunction) dlstatusaway_cb, widget);
	    return (FALSE);
	}

    server.sin_family = AF_INET;
    /*  We retrieve the IP address of the server from its name: */
    if (haveproxy)
	g_strlcpy (sn, proxy, sizeof (sn));
    else
	{
	    if (expedia)
		{
		    if (expedia_de)
			g_strlcpy (sn, WEBSERVER4, sizeof (sn));
		    else
			g_strlcpy (sn, WEBSERVER2, sizeof (sn));
		}

	    if (!expedia)
		g_strlcpy (sn, WEBSERVER, sizeof (sn));
	}

    if (expedia == TRUE && haveproxy == FALSE)
	g_strlcpy (sn, actualhostname, sizeof (sn));

    if ((server_data = gethostbyname (sn)) == NULL)
	{
	    perror (_("Can't resolve webserver address"));
	    if (!expedia)
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"), WEBSERVER);
	    if (expedia)
		{
		    if (expedia_de)
			g_snprintf (str, sizeof (str),
				    _("Connecting to %s FAILED!"),
				    WEBSERVER4);
		    else
			g_snprintf (str, sizeof (str),
				    _("Connecting to %s FAILED!"),
				    WEBSERVER2);
		}

	    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    return (FALSE);
	}
    memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
    server.sin_port = htons (proxyport);
    /*  We initiate the connection  */
    if (connect (dlsock, (struct sockaddr *) &server, sizeof server) < 0)
	{
	    perror (_("unable to connect to Website"));
	    if (!expedia)
		g_snprintf (str, sizeof (str),
			    _("Connecting to %s FAILED!"), WEBSERVER);
	    if (expedia)
		{
		    if (expedia_de)
			g_snprintf (str, sizeof (str),
				    _("Connecting to %s FAILED!"),
				    WEBSERVER4);
		    else
			g_snprintf (str, sizeof (str),
				    _("Connecting to %s FAILED!"),
				    WEBSERVER2);
		}

	    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    return (FALSE);
	}

    write (dlsock, writebuff, strlen (writebuff));
    dlbuff = g_new0 (gchar, 8192);
    dlpstart = NULL;
    dldiff = dlcount = 0;
    if (!expedia)
	g_snprintf (str, sizeof (str), _("Now connected to %s"),
		    WEBSERVER);
    if (expedia)
	{
	    if (expedia_de)
		g_snprintf (str, sizeof (str),
			    _("Now connected to %s"), WEBSERVER4);
	    else
		g_snprintf (str, sizeof (str),
			    _("Now connected to %s"), WEBSERVER2);
	}

    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid, str);
    gtk_timeout_add (100, (GtkFunction) downloadslave_cb, widget);
    return TRUE;
}

gint
downloadslave_cb (GtkWidget * widget, guint datum)
{
    gchar tmpbuff[9000], str[100], *p;
    gint e, fd;
    gchar nn[] = "\r\n\r\n";
    gdouble f;
    G_CONST_RETURN gchar *s;

    if (!downloadwindowactive)
	return FALSE;


    FD_ZERO (&readmask);
    FD_SET (dlsock, &readmask);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    if (select (FD_SETSIZE, &readmask, NULL, NULL, &timeout) < 0)
	{
	    perror ("select() call");
	}

    if (FD_ISSET (dlsock, &readmask))
	{
	    memset (tmpbuff, 0, 8192);
	    if ((e = read (dlsock, tmpbuff, 8000)) < 0)
		perror (_("read from Webserver"));
	    if ( mydebug > 3 )
		g_print ("Loaded %d Bytes\n", e);
	    if (e > 0)
		{
		    /*  in dlbuff we have all download data */
		    memcpy ((dlbuff + dlcount), tmpbuff, e);
		    /*  in dlcount we have the number of download bytes */
		    dlcount += e;
		    /* now we try to get the filelength and begin of the gif image data */
		    if (dlpstart == NULL)
			{
			    /*  CONTENT-LENGTH string should hopefully be in the first 4kB */
			    memcpy (tmpbuff, dlbuff, 4096);
			    /*  We make of this a null terminated string */
			    tmpbuff[4096] = 0;
			    g_strup (tmpbuff);
			    p = strstr (tmpbuff, "CONTENT-LENGTH:");
			    if (p != NULL)
				{
				    sscanf (p, "%s %d", str,
					    &downloadfilelen);
				    /*  now we look for 2 cr/lf which is the end of the header */
				    dlpstart = strstr (tmpbuff, nn);
				    dldiff = dlpstart - tmpbuff + 4;
				    /*            g_print("content-length: %d\n", downloadfilelen); */
				}
			    else if (dlcount > 1000)
				{
				    /*  Seems there is no CONTENT-LENGTH field in expedia.com */
				    dlpstart = strstr (tmpbuff, nn);
				    dldiff = dlpstart - tmpbuff + 4;
				    downloadfilelen = 200000;
				    /*            g_print("\ncontent-length: %d", downloadfilelen); */
				}
			}
		    /*  Now we have the length and begin of the gif image data */
		    if ((downloadfilelen != 0) && (dlpstart != NULL))
			{
			    dlbuff = g_renew (gchar, dlbuff,
					      dlcount + 8192);
			    f = (dlcount -
				 dldiff) / (gdouble) downloadfilelen;
			    if (f > 1.0)
				f = 1.0;
			    gtk_progress_bar_update (GTK_PROGRESS_BAR
						     (myprogress), f);
			    g_snprintf (str, sizeof (str),
					_("Downloaded %d kBytes"),
					(dlcount - dldiff) / 1024);
			    gtk_statusbar_pop (GTK_STATUSBAR (frame_status),
					       statusid);
			    gtk_statusbar_push (GTK_STATUSBAR (frame_status),
						statusid, str);
			    while (gtk_events_pending ())
				gtk_main_iteration ();
			}

		}
	    if ((e == 0) || ((downloadfilelen + dldiff) == dlcount))
		{

		    if (downloadfilelen == 0)
			g_snprintf (str, sizeof (str),
				    _("Download FAILED!"));
		    else
			g_snprintf (str, sizeof (str),
				    _("Download finished, got %dkB"),
				    dlcount / 1024);
		    gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
		    gtk_statusbar_push (GTK_STATUSBAR (frame_status), statusid,
					str);
		    close (dlsock);
		    if (downloadfilelen != 0)
			{
			    s = gtk_entry_get_text (GTK_ENTRY (dltext4));
			    if (local_config_mapdir[strlen (local_config_mapdir) - 1] != '/')
				g_strlcat (local_config_mapdir, "/",
					   sizeof (local_config_mapdir));

			    g_strlcpy (downloadfilename, local_config_mapdir,
				       sizeof (downloadfilename));

			    g_strlcat (downloadfilename, s,
				       sizeof (downloadfilename));
			    fd = open (downloadfilename,
				       O_RDWR | O_TRUNC | O_CREAT, 0644);
			    if (fd < 1)
				{
				    perror (downloadfilename);
				    gtk_timeout_add (3000,(GtkFunction) dlstatusaway_cb, widget);

				    return FALSE;
				}
			    write (fd, dlbuff + dldiff, dlcount - dldiff);
			    close (fd);
			    /* g_free (maps); */
			    loadmapconfig ();
			    maps = g_renew (mapsstruct, maps,
					    (nrmaps + 2));
			    g_strlcpy ((maps + nrmaps)->filename,
				       g_basename (downloadfilename),
				       200);
			    coordinate_string2gdouble (newmaplat, &((maps + nrmaps)->lat));
			    coordinate_string2gdouble (newmaplon, &((maps + nrmaps)->lon));
			    (maps + nrmaps)->scale =
				strtol (newmapsc, NULL, 0);
			    nrmaps++;
			    havenasa = -1;
			    savemapconfig ();
			}
		    downloadwindowactive = FALSE;
		    gtk_widget_destroy (downloadwindow);
		    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);

		    return FALSE;
		}
	}
    else
	{

	    return TRUE;
	}


    return TRUE;
}

/* *****************************************************************************
 */
gint
downloadsetparm (GtkWidget * widget, guint datum)
{
	G_CONST_RETURN gchar *s, *sc;
	gchar lon[100], lat[100], hostname[100], region[10];
	gdouble f, nlon, nlat;
	gint ns;

	char sctext[40];

	if (!downloadwindowactive)
		return TRUE;

	expedia_de = FALSE;

	if (GTK_TOGGLE_BUTTON (radio2)->active)
		expedia = TRUE;
	else
		expedia = FALSE;
	if (GTK_TOGGLE_BUTTON (radio1)->active)
	{
		expedia = TRUE;
		expedia_de = TRUE;
	}



	s = gtk_entry_get_text (GTK_ENTRY (dl_text_lat));
	g_strlcpy (lat, s, sizeof (lat));
	g_strdelimit (lat, ",", '.');
	g_strlcpy (newmaplat, lat, sizeof (newmaplat));
	coordinate_string2gdouble(lat, &nlat);
	
	s = gtk_entry_get_text (GTK_ENTRY (dl_text_lon));
	g_strlcpy (lon, s, sizeof (lon));
	g_strdelimit (lon, ",", '.');
	g_strlcpy (newmaplon, lon, sizeof (newmaplon));
	coordinate_string2gdouble(lon,&nlon);

	sc = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (dltext3)->entry));
	g_strlcpy (sctext, sc, sizeof (sctext));
	g_strlcpy (newmapsc, sctext, sizeof (newmapsc));

	/*   g_print("newmaplat: %s, newmaplon: %s newmapsc: %s\n", newmaplat, */
	/*     newmaplon, newmapsc); */

	if (datum == 0)
		return TRUE;
	if (expedia)
	{
		if (expedia_de)
			g_snprintf (hostname, sizeof (hostname), "%s",
				    WEBSERVER4);
		else
			g_snprintf (hostname, sizeof (hostname), "%s",
				    WEBSERVER2);
	}

	if (!expedia)
		g_snprintf (hostname, sizeof (hostname), "%s", WEBSERVER);

	/*   if (expedia) */
	/*     { */
	/*       f = g_strtod (sc, NULL); */
	/*       ns = f / EXPEDIAFACT; */
	/*       g_snprintf ((char *) sc, "%d", ns); */
	/*       g_snprintf (newmapsc, "%d", (int) (ns * EXPEDIAFACT)); */
	/*     } */
	if (expedia)
	{
		int scales[11] =
			{ 1, 3, 6, 12, 25, 50, 150, 800, 2000, 7000, 12000 };
		int i, found = 5;
		double di = 999999;
		f = g_strtod (sctext, NULL);
		ns = f / EXPEDIAFACT;
		for (i = 0; i < 11; i++)
			if (abs (ns - scales[i]) < di)
			{
				di = abs (ns - scales[i]);
				found = i;
			}
		ns = scales[found];
		g_snprintf (sctext, sizeof (sctext), "%d", ns);
		g_snprintf (newmapsc, sizeof (newmapsc), "%d",
			    (int) (ns * EXPEDIAFACT));
	}
	if ( mydebug > 0 )
		printf ("sctext: %s,newmapsc: %s\n", sctext, newmapsc);

	/*   new URL (08/28/2002) */
	/* http://www.mapblast.com/myblastd/MakeMap.d?&CT=48.0:12.2:100000&IC=&W=1280&H=1024&FAM=myblast&LB= */
	/*   new URL (April 2003)
	 *   http://www.vicinity.com/gif?&CT=45:-93:10000&IC=&W=1024&H=800&FAM=myblast&LB=
	 */

	if (!expedia)
		g_snprintf (writebuff, sizeof (writebuff),
			    "GET http://%s/gif?&CT=%s:%s:%s&IC=&W=1280&H=1024&FAM=myblast&LB= HTTP/1.0\r\nUser-Agent: Wget/1.6\r\nHost: %s\r\nAccept: */*\r\nConnection: Keep-Alive\r\n\r\n",
			    WEBSERVER, lat, lon, sctext, hostname);
	if (expedia)
	{
		if (nlon > (-30))
			g_strlcpy (region, "EUR0809", sizeof (region));
		else
			g_strlcpy (region, "USA0409", sizeof (region));

		if (expedia_de)
			g_snprintf (writebuff, sizeof (writebuff),
				    "GET http://%s/pub/agent.dll?"
				    "qscr=mrdt&ID=3XNsF.&CenP=%f,%f&Lang=%s&Alti=%s"
				    "&Size=1280,1024&Offs=0.000000,0.000000& HTTP/1.1\r\n"
				    "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)\r\n"
				    "Host: %s\r\nAccept: */*\r\nCookie: jscript=1\r\n\r\n",
				    WEBSERVER4, nlat, nlon, region, sctext,
				    hostname);
		else
			g_snprintf (writebuff, sizeof (writebuff),
				    "GET http://%s/pub/agent.dll?qscr=mrdt&ID=3XNsF.&CenP=%f,%f&Lang=%s&Alti=%s"
				    "&Size=1280,1024&Offs=0.000000,0.000000& HTTP/1.1\r\n"
				    "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)\r\n"
				    "Host: %s\r\nAccept: */*\r\nCookie: jscript=1\r\n\r\n",
				    WEBSERVER2, nlat, nlon, region, sctext,
				    hostname);
	}

	if ( mydebug > 0 )
		g_print ("Download URL: %s\n", writebuff);

	if (!expedia)
		downloadstart_cb (widget, 0);
	else
	{
		char url[2000], url2[2000], hn[200], *p;

		p = getexpediaurl (widget);
		if (p == NULL)
		{
			return FALSE;
		}

		g_strlcpy (url, p, sizeof (url));
		if ( mydebug > 3 )
			printf ("%s\n", url);
		p = strstr (url, "Location: ");
		if (p == NULL)
		{
			if ( mydebug > 0 )
				printf ("http data error, could not find 'Location:' sub string\n");
			return FALSE;
		}
		g_strlcpy (url2, (p + 10), sizeof (url2));
		p = strstr (url2, "\n");
		if (p == NULL)
		{
			if ( mydebug > 0 )
				printf ("http data error, could not find new line\n");
			return FALSE;
		}

		url2[p - url2] = 0;
		if ( mydebug > 3 )
			printf ("**********\n%s\n", url2);
		g_strlcpy (hn, (url2 + 7), sizeof (hn));
		p = strstr (hn, "/");
		if (p == NULL)
		{
			if ( mydebug > 0 )
				printf ("http request error, could not find forward slash\n");
			return FALSE;
		}

		hn[p - hn] = 0;
		g_strlcpy (url, (url2 + strlen (hn) + 7), sizeof (url));
		url[strlen (url) - 1] = 0;
		g_strlcpy (actualhostname, hn, sizeof (actualhostname));
		if ( mydebug > 3 )
			printf ("hn: %s, url: %s\n", hn, url);

		if (haveproxy == TRUE)
		{
			// Format the GET request correctly for the proxy server
			g_snprintf (url2, sizeof (url2),
				    "GET http://%s/%s HTTP/1.1\r\n", hn, url);
		}
		else
		{
			g_snprintf (url2, sizeof (url2),
				    "GET %s HTTP/1.1\r\n", url);
		}

		g_strlcat (url2, "Host: ", sizeof (url2));
		g_strlcat (url2, hn, sizeof (url2));
		g_strlcat (url2, "\r\n", sizeof (url2));
		g_strlcat (url2,
			   "User-Agent: Mozilla/5.0 Galeon/1.2.8 (X11; Linux i686; U;) Gecko/20030317\r\n",
			   sizeof (url2));
		g_strlcat (url2,
			   "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,video/x-mng,image/png,image/jpeg,image/gif;q=0.2,text/css,*/*;q=0.1\r\n",
			   sizeof (url2));
		g_strlcat (url2, "Accept-Language: de, en;q=0.50\r\n",
			   sizeof (url2));
		g_strlcat (url2,
			   "Accept-Encoding: gzip, deflate, compress;q=0.9\r\n",
			   sizeof (url2));
		g_strlcat (url2,
			   "Accept-Charset: ISO-8859-15, utf-8;q=0.66, *;q=0.66\r\n",
			   sizeof (url2));
		g_strlcat (url2, "Keep-Alive: 300\r\n", sizeof (url2));
		g_strlcat (url2, "Connection: keep-alive\r\n\r\n",
			   sizeof (url2));

		g_strlcpy (writebuff, url2, sizeof (writebuff));
		if ( mydebug > 3 )
			printf ("\nurl2:\n%s\n**********\n\n%s\n-----------------\n", url2, writebuff);

		downloadstart_cb (widget, 0);

		/*       exit (1); */
	}
	return TRUE;
}


/* *****************************************************************************
 */
gint
defaultserver_cb (GtkWidget * widget, guint datum)
{

	expedia_de = FALSE;
	switch (datum)
	{
	case 1:
		expedia_de = TRUE;
		defaultserver = 0;
		break;
	case 2:
		defaultserver = 1;
		break;
	}
	needtosave = TRUE;
	return TRUE;
}

/* *****************************************************************************
 */


/* *****************************************************************************
 */
gint
download_cb (GtkWidget * widget, guint datum)
{
	GtkWidget *mainbox;
	GtkWidget *knopf2, *knopf, *knopf_lat, *knopf_lon, *knopf_scale, *knopf_map_filename,
		*knopf_help_text;
	GtkWidget *table, *table2, *knopf8;
	gchar buff[300], mappath[2048];
	GList *list = NULL;
	GSList *gr;
	gint i, e;
	struct stat buf;
	gchar scalewanted_str[100];
	GtkTooltips *tooltips;

	for (i = 0; i < slistsize; i++)
		list = g_list_append (list, slist[i]);

	downloadwindow = gtk_dialog_new ();
	gtk_window_set_title (GTK_WINDOW (downloadwindow),
			      _("Select coordinates and scale"));
	gtk_container_set_border_width (GTK_CONTAINER (downloadwindow), 5);
	mainbox = gtk_vbox_new (TRUE, 2);
	knopf = gtk_button_new_with_label (_("Download map"));
	gtk_signal_connect (GTK_OBJECT (knopf), "clicked",
			    GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 1);
	knopf2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_signal_connect_object (GTK_OBJECT (knopf2), "clicked",
				   GTK_SIGNAL_FUNC
				   (downloadaway_cb),
				   GTK_OBJECT (downloadwindow));
	gtk_signal_connect_object (GTK_OBJECT (downloadwindow),
				   "delete_event",
				   GTK_SIGNAL_FUNC (downloadaway_cb),
				   GTK_OBJECT (downloadwindow));
	cover = gtk_entry_new ();
	gtk_editable_set_editable (GTK_EDITABLE (cover), FALSE);
	gtk_signal_connect (GTK_OBJECT (cover), "changed",
			    GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (downloadwindow)->
			     action_area), knopf, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (downloadwindow)->
			     action_area), knopf2, TRUE, TRUE, 2);
	GTK_WIDGET_SET_FLAGS (knopf, GTK_CAN_DEFAULT);
	GTK_WIDGET_SET_FLAGS (knopf2, GTK_CAN_DEFAULT);
	table = gtk_table_new (8, 2, FALSE);
	gtk_box_pack_start (GTK_BOX
			    (GTK_DIALOG (downloadwindow)->vbox),
			    table, TRUE, TRUE, 2);
	knopf_lat = gtk_label_new (_("Latitude"));
	gtk_table_attach_defaults (GTK_TABLE (table), knopf_lat, 0, 1, 0, 1);
	knopf_lon = gtk_label_new (_("Longitude"));
	gtk_table_attach_defaults (GTK_TABLE (table), knopf_lon, 0, 1, 1, 2);
	knopf8 = gtk_label_new (_("Map covers"));
	gtk_table_attach_defaults (GTK_TABLE (table), knopf8, 0, 1, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (table), cover, 1, 2, 2, 3);

	knopf_scale = gtk_label_new (_("Scale"));
	gtk_table_attach_defaults (GTK_TABLE (table), knopf_scale, 0, 1, 3, 4);
	knopf_map_filename = gtk_label_new (_("Map file name"));
	gtk_table_attach_defaults (GTK_TABLE (table), knopf_map_filename, 0, 1, 4, 5);
	dl_text_lat = gtk_entry_new ();
	gtk_signal_connect (GTK_OBJECT (dl_text_lat), "changed",
			    GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);

	gtk_table_attach_defaults (GTK_TABLE (table), dl_text_lat, 1, 2, 0, 1);
	coordinate2gchar(buff, sizeof(buff), current_lat, TRUE, minsecmode);
	gtk_entry_set_text (GTK_ENTRY (dl_text_lat), buff);
	dl_text_lon = gtk_entry_new ();
	gtk_signal_connect (GTK_OBJECT (dl_text_lon), "changed",
			    GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);
	gtk_table_attach_defaults (GTK_TABLE (table), dl_text_lon, 1, 2, 1, 2);
	coordinate2gchar(buff, sizeof(buff), current_lon, FALSE, minsecmode);
	gtk_entry_set_text (GTK_ENTRY (dl_text_lon), buff);
	dltext3 = gtk_combo_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), dltext3, 1, 2, 3, 4);
	gtk_combo_set_popdown_strings (GTK_COMBO (dltext3), (GList *) list);
	g_snprintf (scalewanted_str, sizeof (scalewanted_str), "%d",
		    scalewanted);
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dltext3)->entry),
			    scalewanted_str);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO (dltext3)->entry),
			    "changed", GTK_SIGNAL_FUNC (downloadsetparm),
			    (gpointer) 0);

	dltext4 = gtk_entry_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), dltext4, 1, 2, 4, 5);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO (dltext3)->entry),
			    "changed", GTK_SIGNAL_FUNC (dlscale_cb), 0);
	gtk_signal_connect (GTK_OBJECT (dltext4), "changed",
			    GTK_SIGNAL_FUNC (dlscale_cb), 0);

	table2 = gtk_table_new (2, 1, FALSE);	//nested table w/ three columns
	gtk_table_attach_defaults (GTK_TABLE (table), table2, 0, 3, 5, 6);
	gtk_widget_show (table2);

	radio1 = gtk_radio_button_new_with_label (NULL, _("Expedia Germany"));
	gtk_table_attach_defaults (GTK_TABLE (table2), radio1, 0, 1, 0, 1);
	gr = gtk_radio_button_group (GTK_RADIO_BUTTON (radio1));
	gtk_signal_connect (GTK_OBJECT (radio1), "clicked",
			    GTK_SIGNAL_FUNC (other_select_cb), 0);

	radio2 = gtk_radio_button_new_with_label (gr, _("Expedia USA"));
	gtk_table_attach_defaults (GTK_TABLE (table2), radio2, 1, 2, 0, 1);
	gr = gtk_radio_button_group (GTK_RADIO_BUTTON (radio2));
	gtk_signal_connect (GTK_OBJECT (radio2), "clicked",
			    GTK_SIGNAL_FUNC (other_select_cb), 0);



	tooltips = gtk_tooltips_new ();
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), radio1,
			      _
			      ("If selected, you download the map from the german expedia server (expedia.de)"),
			      NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), radio2,
			      _
			      ("If selected, you download the map from the U.S. expedia server (expedia.com)"),
			      NULL);

	/* disable mapblast */
	/*   gtk_widget_set_sensitive (radio1, FALSE); */
	/*   defaultserver = 1; */

	/*   gtk_table_attach_defaults (GTK_TABLE (table), radio2, 1, 2, 5, 6); */
	if ((defaultserver < 0) && (defaultserver > 1))
		defaultserver = 0;
	switch (defaultserver)
	{
	case 0:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio1),
					      TRUE);
		other_select_cb (NULL, 0);
		break;
	case 1:
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio2),
					      TRUE);
		other_select_cb (NULL, 0);
		break;
	}

	if (!haveproxy)
		g_snprintf (buff, sizeof (buff), "%s",
			    _("You can also select the position\n"
			      "with a mouse click on the map."));
	else
		g_snprintf (buff, sizeof (buff), "%s\n\n%s    %s %d",
			    _("You can also select the position\n"
			      "with a mouse click on the map."),
			    _("Using Proxy and port:"), proxy, proxyport);
	knopf_help_text = gtk_label_new (buff);
	gtk_table_attach_defaults (GTK_TABLE (table), knopf_help_text, 0, 2, 6, 7);

	myprogress = gtk_progress_bar_new ();
	gtk_progress_set_format_string (GTK_PROGRESS (myprogress), "%p%%");
	gtk_progress_set_show_text (GTK_PROGRESS (myprogress), TRUE);
	gtk_progress_bar_update (GTK_PROGRESS_BAR (myprogress), 0.0);
	gtk_table_attach_defaults (GTK_TABLE (table), myprogress, 0, 2, 7, 8);
	gtk_label_set_justify (GTK_LABEL (knopf_map_filename), GTK_JUSTIFY_RIGHT);
	gtk_label_set_justify (GTK_LABEL (knopf_lat), GTK_JUSTIFY_RIGHT);
	gtk_label_set_justify (GTK_LABEL (knopf_lon), GTK_JUSTIFY_RIGHT);
	gtk_label_set_justify (GTK_LABEL (knopf_scale), GTK_JUSTIFY_RIGHT);
	gtk_label_set_justify (GTK_LABEL (knopf_map_filename), GTK_JUSTIFY_RIGHT);
	i = 0;
	do
	{
		if (local_config_mapdir[strlen (local_config_mapdir) - 1] != '/')
			g_strlcat (local_config_mapdir, "/", sizeof (local_config_mapdir));

		g_strlcpy (mappath, local_config_mapdir, sizeof (mappath));

		g_snprintf (downloadfilename, sizeof (downloadfilename),
			    "%smap_file%04d.gif", mappath, i++);
		e = stat (downloadfilename, &buf);
	}
	while (e == 0);
	g_snprintf (buff, sizeof (buff), "map_file%04d.gif", i - 1);
	gtk_entry_set_text (GTK_ENTRY (dltext4), buff);
	gtk_window_set_default (GTK_WINDOW (downloadwindow), knopf);
	gtk_window_set_transient_for (GTK_WINDOW (downloadwindow),
				      GTK_WINDOW (mainwindow));
	gtk_window_set_position (GTK_WINDOW (downloadwindow),
				 GTK_WIN_POS_CENTER);
	gtk_widget_show_all (downloadwindow);
	downloadwindowactive = TRUE;
	downloadsetparm (NULL, 0);

	/*    cursor = gdk_cursor_new (GDK_CROSS); */
	/*    gdk_window_set_cursor (drawing_area->window, cursor); */
	return TRUE;
}

/* *****************************************************************************
 * cancel button pressed or widget destroy in download_cb 
 */
gint
downloadaway_cb (GtkWidget * widget, guint datum)
{
	downloadwindowactive = downloadactive = FALSE;
	gtk_widget_destroy (widget);
	expose_mini_cb (NULL, 0);

	/*    gdk_window_set_cursor (drawing_area->window, 0); */
	/*    gdk_cursor_destroy (cursor); */
	return FALSE;
}


/* *****************************************************************************
 */
gint
other_select_cb (GtkWidget * widget, guint datum)
{
	gint i, e;
	gchar buff[300], mappath[2048];
	struct stat buf;

	i = 0;
	do
	{
		if (local_config_mapdir[strlen (local_config_mapdir) - 1] != '/')
			g_strlcat (local_config_mapdir, "/", sizeof (local_config_mapdir));

		g_strlcpy (mappath, local_config_mapdir, sizeof (mappath));

		g_snprintf (downloadfilename, sizeof (downloadfilename),
			    "%smap_file%04d.gif", mappath, i++);
		e = stat (downloadfilename, &buf);
	}
	while (e == 0);
	g_snprintf (buff, sizeof (buff), "map_file%04d.gif", i - 1);
	gtk_entry_set_text (GTK_ENTRY (dltext4), buff);

	return TRUE;
}

/* *****************************************************************************
 */
gint
dlscale_cb (GtkWidget * widget, guint datum)
{
	G_CONST_RETURN gchar *sc;
	gchar t[100], t2[10];
	gdouble f;
	/* PORTING */
	sc = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (dltext3)->entry));

	f = g_strtod (sc, NULL);

	g_strlcpy (t2, "km", sizeof (t2));

	if (milesflag)
		g_strlcpy (t2, "mi", sizeof (t2));
	if (nauticflag)
		g_strlcpy (t2, "nmi", sizeof (t2));

	g_snprintf (t, sizeof (t), "%.3f x %.3f %s",
		    milesconv * 1.280 * f / PIXELFACT,
		    milesconv * 1.024 * f / PIXELFACT, t2);
	gtk_entry_set_text (GTK_ENTRY (cover), t);
	return TRUE;
}

/* *****************************************************************************
 * Get http_proxy  Variable from envirenment
 */
void get_proxy_from_env()
{ // Set http_proxy
    gint i;
    gchar s1[100], s2[100];
    const gchar *http_proxy;
    gchar *p;
    http_proxy = g_getenv ("HTTP_PROXY");
    if (http_proxy == NULL)
	http_proxy = g_getenv ("http_proxy");

    if (http_proxy)
	{
	    p = (char *) http_proxy;
	    g_strdelimit (p, ":/", ' ');

	    i = sscanf (p, "%s %s %d", s1, s2, &proxyport);
	    if (i == 3)
		{
		    haveproxy = TRUE;
		    g_strlcpy (proxy, s2, sizeof (proxy));
		    if ( mydebug > 0 )
			g_print (_("\nUsing proxy: %s on port %d"),
				 proxy, proxyport);
		}
	    else
		{
		    g_print (_
			     ("\nInvalid enviroment variable HTTP_PROXY, "
			      "must be in format: http://proxy.provider.de:3128"));
		}
	}
}
