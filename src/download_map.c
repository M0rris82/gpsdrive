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
#include "config.h"
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
#include <dirent.h>

#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

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


#if GTK_MINOR_VERSION < 2
#define gdk_draw_pixbuf _gdk_draw_pixbuf
#endif


#include <gpsdrive.h>
#include <map_handler.h>
#include "gpsdrive_config.h"

extern GtkWidget *frame_statusbar;
extern gint haveproxy, proxyport;
extern gchar proxy[256];
extern gint mydebug;
extern mapsstruct *maps;
extern struct timeval timeout;
extern int havenasa;
extern gint slistsize;
extern gchar *slist[];
GtkWidget *cover;
extern gdouble milesconv;
extern coordinate_struct coords;
extern currentstatus_struct current;

char actualhostname[200];

SOCKET_TYPE dlsock = -1;
int expedia_de = 0;
gint expedia = TRUE;
GtkWidget *downloadwindow;

GtkWidget *radio1, *radio2;
gint downloadwindowactive=0;
gint downloadactive=0;
gchar writebuff[2000];
fd_set readmask;
gchar *dlbuff;
gint downloadfilelen=0;
gchar *dlpstart;
gint nrmaps = 0, dldiff;
gint dlcount;
GtkWidget *myprogress;
GtkWidget *dl_text_lat, *dl_text_lon, *dl_text_scale;
gdouble new_dl_lon, new_dl_lat;
gint new_dl_scale;

gint downloadaway_cb (GtkWidget * widget, guint datum);
gint dlscale_cb (GtkWidget * widget, guint datum);

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
	    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar),
	    	current.statusbar_id);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
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
	    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    socket_close (dlsock);
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
	    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    socket_close (dlsock);
	    return (NULL);
	}

    if ( send (dlsock, writebuff, strlen (writebuff),0)<0){
	socket_close (dlsock);
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
    if ((e = recv (dlsock, tmpbuff, 8000, 0)) < 0)
	perror (_("read from Webserver"));
    if ( mydebug > 3 )
	g_print ("Loaded %d Bytes\n", e);
    if (e > 0)
	g_strlcpy (url, tmpbuff, sizeof (url));
    else
	{
	    perror ("getexpediaurl");
	    fprintf (stderr, "error while reading from exedia\n");
	    socket_close (dlsock);
	    return (NULL);
	    // exit (1);
	}
    socket_close (dlsock);
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

    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
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

	    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
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

	    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
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

	    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
	    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
	    gtk_widget_destroy (downloadwindow);
	    gtk_timeout_add (3000, (GtkFunction) dlstatusaway_cb, widget);
	    return (FALSE);
	}

    send (dlsock, writebuff, strlen (writebuff), 0);
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

    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id, str);
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
	    if ((e = recv (dlsock, tmpbuff, 8000, 0)) < 0)
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
			    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar),
					       current.statusbar_id);
			    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
						current.statusbar_id, str);
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
		    gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar), current.statusbar_id);
		    gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar), current.statusbar_id,
					str);
		    socket_close (dlsock);
		    if (downloadfilelen != 0)
			{
			    gchar map_filename[1024];
			    gchar map_file_w_path[1024];

			    g_snprintf( map_filename, sizeof (map_filename),
					"expedia/map_%d_%5.3f_%5.3f.gif",
					new_dl_scale,new_dl_lat,new_dl_lon);

			    if ( local_config.dir_maps[strlen (local_config.dir_maps) - 1] != '/' )
				g_strlcat( local_config.dir_maps, "/",sizeof (local_config.dir_maps) );
			    
			    g_snprintf (map_file_w_path, sizeof (map_file_w_path), 
					"%s%s",local_config.dir_maps,map_filename);

			    if ( mydebug > 1 ) {
				g_print("Saving Map File to: '%s'\n",
				       map_file_w_path);
			    }

			    // Create directory
				{
					struct stat buf;
					gchar map_dir[1024];
					g_snprintf (map_dir, sizeof (map_dir), 
						"%s%s",local_config.dir_maps,"expedia");
					if ( stat(map_dir,&buf) )
					{
						printf("Try creating %s\n",map_dir);
						if ( mkdir (map_dir, 0700) )
						{
							printf("Error creating %s\n",map_dir);
						}
					}
				}
			    
			    fd = open (map_file_w_path,
				       O_RDWR | O_TRUNC | O_CREAT | O_BINARY, 0644);
			    if (fd < 1)
				{
				    perror (map_filename);
				    gtk_timeout_add (3000,(GtkFunction) dlstatusaway_cb, widget);

				    return FALSE;
				}
			    write (fd, dlbuff + dldiff, dlcount - dldiff);
			    close (fd);
			    /* g_free (maps); */
			    loadmapconfig ();
			    maps = g_renew (mapsstruct, maps,
					    (nrmaps + 2));
			    g_strlcpy ((maps + nrmaps)->filename,map_filename, 200);
			    (maps + nrmaps)->map_dir = add_map_dir (map_filename);
			    (maps + nrmaps)->hasbbox = FALSE;
			    (maps + nrmaps)->lat = new_dl_lat;
			    (maps + nrmaps)->lon = new_dl_lon;
			    (maps + nrmaps)->scale =new_dl_scale;
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
	G_CONST_RETURN gchar *s;
	gchar lon[100], lat[100], hostname[100], region[10];
	gdouble f;
	gint ns;

	char sctext[40];

	if (!downloadwindowactive)
		return TRUE;

	s = gtk_entry_get_text (GTK_ENTRY (dl_text_lat));
	coordinate_string2gdouble(s, &new_dl_lat);
	if ( mydebug > 3 )
	    g_print("new map lat: %s\n",s);
	
	s = gtk_entry_get_text (GTK_ENTRY (dl_text_lon));
	coordinate_string2gdouble(s,&new_dl_lon);
	if ( mydebug > 3 )
	    g_print("new map lon: %s\n",s);

	s = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (dl_text_scale)->entry));
	new_dl_scale = strtol (s, NULL, 0);
	if ( mydebug > 3 )
	    g_print("new map scale: %d\n",new_dl_scale);
	
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

	if (expedia)
	{
		int scales[11] =
			{ 1, 3, 6, 12, 25, 50, 150, 800, 2000, 7000, 12000 };
		int i, found = 5;
		double di = 999999;
		f = new_dl_scale;
		ns = f / EXPEDIAFACT;
		for (i = 0; i < 11; i++)
			if (abs (ns - scales[i]) < di)
			{
				di = abs (ns - scales[i]);
				found = i;
			}
		ns = scales[found];
		g_snprintf (sctext, sizeof (sctext), "%d", ns);
		new_dl_scale =     (int) (ns * EXPEDIAFACT);
	}
	if ( mydebug > 0 )
		printf ("sctext: %s,new map scale: %d\n", sctext, new_dl_scale);

	if (!expedia)
		g_snprintf (writebuff, sizeof (writebuff),
			    "GET http://%s/gif?&CT=%s:%s:%s&IC=&W=1280&H=1024&FAM=myblast&LB="
			    " HTTP/1.0\r\n"
			    "User-Agent: Wget/1.6\r\n"
			    "Host: %s\r\n"
			    "Accept: */*\r\n"
			    "Connection: Keep-Alive\r\n"
			    "\r\n",
			    WEBSERVER, lat, lon, sctext, hostname);
	if (expedia)
	{
		if (new_dl_lon > (-30))
		    {
			g_strlcpy (region, "EUR0809", sizeof (region));
			expedia_de = TRUE;
		    }	
		else
		    {
			g_strlcpy (region, "USA0409", sizeof (region));
			expedia_de = FALSE;
		    }
		

        {
            /* localizing might use 48,0000 for floating point, */
            /* expedia doesn't like this */
            /* XXX - is there a better way to handle this? */
            gchar new_dl_lat_str[50];
            gchar new_dl_lon_str[50];

            g_snprintf(new_dl_lat_str, sizeof(new_dl_lat_str), "%f", new_dl_lat);
            g_strdelimit(new_dl_lat_str, ",", '.');
            g_snprintf(new_dl_lon_str, sizeof(new_dl_lon_str), "%f", new_dl_lon);
            g_strdelimit(new_dl_lon_str, ",", '.');

		if (expedia_de)
			g_snprintf (writebuff, sizeof (writebuff),
				    "GET http://%s/pub/agent.dll?"
				    "qscr=mrdt&ID=3XNsF.&CenP=%s,%s&Lang=%s&Alti=%s"
				    "&Size=1280,1024&Offs=0.000000,0.000000& HTTP/1.1\r\n"
				    "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)\r\n"
				    "Host: %s\r\nAccept: */*\r\nCookie: jscript=1\r\n\r\n",
				    WEBSERVER4, new_dl_lat_str, new_dl_lon_str, region, sctext,
				    hostname);
		else
			g_snprintf (writebuff, sizeof (writebuff),
				    "GET http://%s/pub/agent.dll?qscr=mrdt&ID=3XNsF.&CenP=%s,%s&Lang=%s&Alti=%s"
				    "&Size=1280,1024&Offs=0.000000,0.000000& HTTP/1.1\r\n"
				    "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)\r\n"
				    "Host: %s\r\nAccept: */*\r\nCookie: jscript=1\r\n\r\n",
				    WEBSERVER2, new_dl_lat_str, new_dl_lon_str, region, sctext,
				    hostname);
        }
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
download_cb (GtkWidget * widget, guint datum)
{
	GtkWidget *mainbox;
	GtkWidget *knopf2, *knopf, *knopf_lat, *knopf_lon, *knopf_scale, 
		*knopf_help_text;
	GtkWidget *table, *table2, *knopf8;
	gchar buff[300];
	GList *list = NULL;
	gint i;
	gchar scalewanted_str[100];

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
	dl_text_lat = gtk_entry_new ();
	gtk_signal_connect (GTK_OBJECT (dl_text_lat), "changed",
			    GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);

	gtk_table_attach_defaults (GTK_TABLE (table), dl_text_lat, 1, 2, 0, 1);
	coordinate2gchar(buff, sizeof(buff), coords.current_lat, TRUE,
		local_config.coordmode);
	gtk_entry_set_text (GTK_ENTRY (dl_text_lat), buff);
	dl_text_lon = gtk_entry_new ();
	gtk_signal_connect (GTK_OBJECT (dl_text_lon), "changed",
			    GTK_SIGNAL_FUNC (downloadsetparm), (gpointer) 0);
	gtk_table_attach_defaults (GTK_TABLE (table), dl_text_lon, 1, 2, 1, 2);
	coordinate2gchar(buff, sizeof(buff), coords.current_lon, FALSE,
		local_config.coordmode);
	gtk_entry_set_text (GTK_ENTRY (dl_text_lon), buff);
	dl_text_scale = gtk_combo_new ();
	gtk_table_attach_defaults (GTK_TABLE (table), dl_text_scale, 1, 2, 3, 4);
	gtk_combo_set_popdown_strings (GTK_COMBO (dl_text_scale), (GList *) list);
	g_snprintf (scalewanted_str, sizeof (scalewanted_str), "%d",
		    local_config.scale_wanted);
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dl_text_scale)->entry),
			    scalewanted_str);
	gtk_signal_connect (GTK_OBJECT (GTK_COMBO (dl_text_scale)->entry),
			    "changed", GTK_SIGNAL_FUNC (downloadsetparm),
			    (gpointer) 0);

	table2 = gtk_table_new (2, 1, FALSE);	//nested table w/ three columns
	gtk_table_attach_defaults (GTK_TABLE (table), table2, 0, 3, 5, 6);
	gtk_widget_show (table2);

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
	gtk_label_set_justify (GTK_LABEL (knopf_lat), GTK_JUSTIFY_RIGHT);
	gtk_label_set_justify (GTK_LABEL (knopf_lon), GTK_JUSTIFY_RIGHT);
	gtk_label_set_justify (GTK_LABEL (knopf_scale), GTK_JUSTIFY_RIGHT);

	gtk_window_set_default (GTK_WINDOW (downloadwindow), knopf);

	gtk_window_set_position (GTK_WINDOW (downloadwindow),
				 GTK_WIN_POS_CENTER);
	gtk_widget_show_all (downloadwindow);
	downloadwindowactive = TRUE;
	downloadsetparm (NULL, 0);

	/*    cursor = gdk_cursor_new (GDK_CROSS); */
	/*    gdk_window_set_cursor (map_drawingarea->window, cursor); */
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

	/*    gdk_window_set_cursor (map_drawingarea->window, 0); */
	/*    gdk_cursor_destroy (cursor); */
	return FALSE;
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
	sc = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (dl_text_scale)->entry));

	f = g_strtod (sc, NULL);

	g_strlcpy (t2, "km", sizeof (t2));

	if (local_config.distmode == DIST_MILES)
		g_strlcpy (t2, "mi", sizeof (t2));
	else if (local_config.distmode == DIST_NAUTIC)
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
			g_print (_("Using proxy: %s on port %d\n"),
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
