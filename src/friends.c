/* friendsd client
 * Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>
 * 
 * Website: www.gpsdrive.de
 * 
 * 
 * 
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *     *********************************************************************
 */


/*
 * inet.h - Definitions for TCP and UDP client/server programs.
 */
#include <config.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_LINUX_INET_H
#include "linux/inet.h"
#endif
#include <unistd.h>
#include <time.h>
#include <gpsdrive.h>
#include <gpsdrive_config.h>
#include <math.h>
#include "gui.h"
#include "poi.h"
#include "main_gui.h"
#include "os_specific.h"

#define	SERV_UDP_PORT	50123
/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

/* #define	SERV_HOST_ADDR	"213.203.231.23"   */
#define	SERV_HOST_ADDR	"127.0.0.1"

extern int maxfriends;
extern friendsstruct *friends, *fserver;
int actualfriends = 0;
extern int messagenumber;
extern long int maxfriendssecs;
extern gdouble milesconv;
extern GtkWidget *map_drawingarea;
extern GdkPixbuf *friendsimage, *friendspixbuf;
extern gint mydebug;
extern poi_type_struct poi_type_list[poi_type_list_max];
extern color_struct colors;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern GdkGC *kontext_map;

/*
 * conn.c
 */
SOCKET_TYPE sockfd = -1;
int pleasepollme = 0;
extern GtkItemFactory *item_factory;
extern int debug, statuslock;
extern GtkWidget *frame_statusbar;
extern gchar messagename[40], messagesendtext[1024], messageack[100];

#define MAXLINE 512


/* ****************************************************************************
 * Friends agent
 */
gint
friendsagent_cb (GtkWidget * widget, guint * datum)
{
	time_t tii;
	gchar buf[MAXMESG], buf2[40], la[20], lo[20], num[5];
	gint i;

	if ( mydebug >50 ) fprintf(stderr , "friendsagent_cb()\n");

	/* Don't allow spaces in name */
	for (i = 0; (size_t) i < strlen (local_config.friends_name); i++)
		if (local_config.friends_name[i] == ' ')
			local_config.friends_name[i] = '_';

	/*  send position to friendsserver */

	if (local_config.showfriends)
	{
		if (strlen (messagesendtext) > 0)
		{
			/* send message to server */
			if (messagenumber < 99)
				messagenumber++;
			else
				messagenumber = 0;
			current.needtosave = TRUE;
			g_snprintf (num, sizeof (num), "%02d", messagenumber);
			g_strlcpy (buf2, local_config.friends_id,
				sizeof (buf2));
			buf2[0] = 'M';
			buf2[1] = 'S';
			buf2[2] = 'G';
			buf2[3] = num[0];
			buf2[4] = num[1];
			g_snprintf (buf, sizeof (buf), "SND: %s %s %s\n",
				    buf2, messagename, messagesendtext);
			if ( mydebug > 3 )
				fprintf (stderr, "friendsagent: sending to"
					" %s:\nfriendsagent: %s\n",
					local_config.friends_serverip, buf);
			if (sockfd != -1)
				close (sockfd);
			sockfd = -1;
			friends_sendmsg (local_config.friends_serverip, buf);
			g_snprintf (messageack, sizeof (messageack),
				"SND: %s", buf2);
		}
		else
		{
			/* send position to server */
			if (gui_status.posmode)
			{
				g_snprintf (la, sizeof (la), "%10.6f",
					coords.posmode_lat);
				g_snprintf (lo, sizeof (lo), "%10.6f",
					coords.posmode_lon);
			}
			else
			{
				g_snprintf (la, sizeof (la), "%10.6f",
					coords.current_lat);
				g_snprintf (lo, sizeof (lo), "%10.6f",
					coords.current_lon);
			}
			g_strdelimit (la, ",", '.');
			g_strdelimit (lo, ",", '.');
			tii = time (NULL);
			g_snprintf (buf, sizeof (buf),
				"POS: %s %s %s %s %ld %.0f %.0f %d",
				local_config.friends_id,
				local_config.friends_name, la, lo, tii,
				current.groundspeed / milesconv,
				180.0 * current.heading / M_PI,
				local_config.travelmode);
			if ( mydebug > 3 )
				fprintf (stderr,
					"friendsagent: sending to"
					" %s:\nfriendsagent: %s\n",
					local_config.friends_serverip, buf);
			if (sockfd != -1)
				close (sockfd);
			sockfd = -1;
			friends_sendmsg (local_config.friends_serverip, buf);
		}
	}

	return TRUE;
}


/* ****************************************************************************
 * Insert or update data coming from friendsd in database or way.txt file
 */
void
update_friends_data (friendsstruct *cf)
{
	glong current_poi_id = 0;
	gchar *result = NULL;
	
	if (local_config.use_database)
	{
		//(cf)->id,
		//(cf)->name,
		//(cf)->lat,
		//(cf)->lon,
		//(cf)->timesec,
		//(cf)->speed,
		//(cf)->heading,
		//(cf)->type,
		
		/* check, if friend is already present in database */
		current_poi_id = db_poi_extra_get (NULL, "friends_id", (cf)->id, result);

		if (current_poi_id)
		{
			if (mydebug > 30)
				fprintf (stderr, "--------> updating friend"
				" with poi_id = %ld\n", current_poi_id);
			db_poi_edit (current_poi_id, strtod((cf)->lat, NULL),
				strtod((cf)->lon, NULL), (cf)->name,
				(cf)->type, (cf)->heading, 7, TRUE);
			db_poi_extra_edit (&current_poi_id, "speed", (cf)->speed, TRUE);
			db_poi_extra_edit (&current_poi_id, "heading", (cf)->heading, TRUE);
			db_poi_extra_edit (&current_poi_id, "timesec", (cf)->timesec, TRUE);
		}
		else
		{
			current_poi_id = db_poi_edit (0, strtod((cf)->lat, NULL),
				strtod((cf)->lon, NULL), (cf)->name, (cf)->type,
				(cf)->heading, 7, FALSE);
			db_poi_extra_edit (&current_poi_id, "friends_id", (cf)->id, FALSE);
			db_poi_extra_edit (&current_poi_id, "speed", (cf)->speed, FALSE);
			db_poi_extra_edit (&current_poi_id, "heading", (cf)->heading, FALSE);
			db_poi_extra_edit (&current_poi_id, "timesec", (cf)->timesec, FALSE);
		}
	}
	else
	{
		// TODO: add entry to way.txt file
		return;
	}
}


int
friends_sendmsg (char *serverip, char *message)
{
  int n, nosent, endflag, e;
  char recvline[MAXLINE + 1];
  int i, fc, type;
  struct sockaddr_in cli_addr;
  struct sockaddr_in serv_addr;
  struct sockaddr *pserv_addr;
  socklen_t servlen;
  friendsstruct *f;
  char msgname[40], msgid[40], msgtext[1024];

  if (serverip == NULL)
    {
      fprintf (stderr, "error in friends_sendmsg: serverip=NULL\n");
      return 0;
    }

  if (message != NULL)
    if (strlen (message) == 0)
      {
	fprintf (stderr, "error in friends_sendmsg: message=empty\n");
	return 0;
      }

  f = friends;
  g_strlcpy (msgname, "", sizeof (msgname));
  g_strlcpy (msgtext, "", sizeof (msgtext));

  /*   skip if we already have an sockfd  */
  if (message != NULL)
    if (sockfd == -1)
      {
	memset(&serv_addr, 0, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;

	serv_addr.sin_addr.s_addr = inet_addr (serverip);
	serv_addr.sin_port = htons (SERV_UDP_PORT);
	pserv_addr = (struct sockaddr *) &serv_addr;

	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	  {
	    perror ("friendsclient local socket");
	    return (1);
	  }
	socket_nonblocking (sockfd);

	servlen = sizeof (serv_addr);
	memset(&cli_addr, 0, sizeof (cli_addr));

	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = htons (INADDR_ANY);
	cli_addr.sin_port = htons (0);

	if (bind
	    (sockfd, (struct sockaddr *) &cli_addr, sizeof (cli_addr)) < 0)
	  {
	    perror ("friendsclient bind local address");
	    return (2);
	  }

	n = strlen (message);
	/*   printf ("sending...\n");  */
	if ((nosent =
	     sendto (sockfd, message, n, 0, pserv_addr, servlen)) != n)
	  {
	    perror ("friendsclient sendto");
	    return (3);
	  }
	else
	  {
	    pleasepollme = TRUE;
	  }

	/*     end skip if we already have an sockfd  */

	/*       return, so we read the next time */
	return 0;
      }
  endflag = i = 0;

  fc = 0;
  do
    {
      n = recvfrom (sockfd, recvline, MAXLINE, 0 /* MSG_WAITALL */ ,
		    (struct sockaddr *) 0, (socklen_t *) 0);
      if (n < 0)
	{
	  i++;
	  g_usleep (100000);
	  fprintf (stderr, "errno %d:", errno);
	  perror ("recv");
	}
      else
	i = 0;
      if (n > 0)
	{
	  if ((strncmp (recvline, "$END:$", 6)) == 0)
	    endflag = 1;
	  recvline[n] = 0;
	  /*    if (debug) */
	  /*      printf ("received...%d bytes: %s\n=======\n", n, recvline); */
	  /* scanning reply  */
	  if ((strncmp (recvline, "POS: ", 5)) == 0)
	    {
	      e = sscanf (recvline,
			  "POS: %s %s %s %s %s %s %s %d",
			  (f + fc)->id, (f + fc)->name,
			  (f + fc)->lat, (f + fc)->lon,
			  (f + fc)->timesec,
			  (f + fc)->speed, (f + fc)->heading,
			  &type);
	      /*              printf("\nreceived %d arguments\n",e);  */
		if (type == TRAVEL_CAR)
			g_snprintf ((f + fc)->type, sizeof ((f + fc)->type),
				"people.friendsd.car");
		else if (type == TRAVEL_AIRPLANE)
			g_snprintf ((f + fc)->type, sizeof ((f + fc)->type),
				"people.friendsd.airplane");
		else if (type == TRAVEL_BIKE)
			g_snprintf ((f + fc)->type, sizeof ((f + fc)->type),
				"people.friendsd.bike");
		else if (type == TRAVEL_BOAT)
			g_snprintf ((f + fc)->type, sizeof ((f + fc)->type),
				"people.friendsd.boat");
		else if (type == TRAVEL_WALK)
			g_snprintf ((f + fc)->type, sizeof ((f + fc)->type),
				"people.friendsd.walk");
		else
			g_snprintf ((f + fc)->type, sizeof ((f + fc)->type),
				"people.friendsd");
		
		update_friends_data ((f + fc));
		fc++;
	    }
	  if ((strncmp (recvline, "SRV: ", 5)) == 0)
	    {
	      e = sscanf (recvline,
			  "SRV: %s %s %s %s %s %s %s",
			  fserver->id, fserver->name,
			  fserver->lat, fserver->lon,
			  fserver->timesec, fserver->speed, fserver->heading);
	      /*              printf("\nreceived %d arguments\n",e);  */
	    }
	  if ((strncmp (recvline, "SND: ", 5)) == 0)
	    {
	      if ((strlen (messageack) > 0)
		  &&
		  (strncmp (recvline, messageack, strlen (messageack)) == 0))
		{
		  g_strlcpy (messagename, "", sizeof (messagename));
		  g_strlcpy (messageack, "", sizeof (messageack));
		  g_strlcpy (messagesendtext, "", sizeof (messagesendtext));
//		  wi = gtk_item_factory_get_item
//		    (item_factory, N_("/Misc. Menu/Messages"));
//		  gtk_widget_set_sensitive (wi, TRUE);
		  gtk_statusbar_pop (GTK_STATUSBAR (frame_statusbar),
		  	current.statusbar_id);
		  statuslock = FALSE;
		}
	      else
		{
		  e = sscanf (recvline,
			      "SND: %s %s %[^\n]", msgid, msgname, msgtext);
		  if (e == 3)
		    if (strcmp ((msgname), (local_config.friends_name)) == 0)
		      {
			int j, k = 0, fsmessage = 0;

			g_strlcpy (msgname, _("unknown"), sizeof (msgname));
			if (strcmp ((msgid + 5), ((fserver->id) + 5)) == 0)
			  {
			    g_snprintf
			      (msgname, sizeof (msgname), fserver->name);
			    fsmessage = TRUE;
			  }
			for (j = 0; j < fc; j++)
			  if (strcmp ((msgid + 5), (((f + j)->id) + 5)) == 0)
			    g_strlcpy
			      (msgname, (f + j)->name, sizeof (msgname));
			for (j = 0; j < (int) strlen (recvline); j++)
			  {
			    if (*(recvline + j) == ' ')
			      k++;
			    if (k >= 3)
			      break;
			  }
			g_strlcpy (msgtext,
				   (recvline + j + 1), sizeof (msgtext));
			/*                      if (debug) */
			/*                        fprintf (stderr, "\ne: %d, received from %s: %s\n", */
			/*                                 e, msgname, msgtext); */
			message_cb (msgid, msgname, msgtext, fsmessage);
		      }
		}
	    }

	  if (debug)
	    fprintf (stderr, "%s", recvline);

	}
      /*          printf("\ni: %d, endflag: %d\n",i,endflag);   */
    }
  while ((n > 0) && (!endflag));

  /* printf("\nafter while i: %d, endflag: %d\n",i,endflag);  */
  if (endflag)
    {
      close (sockfd);
      sockfd = -1;
      pleasepollme = FALSE;
      if (fc != 0)
	maxfriends = fc;
    }


  return 0;
}


int
friends_init ()
{

  char *key, buf2[20];
  int f, i, j;
  long int r;
  time_t ti, tii;

  if ((strcmp (local_config.friends_id, "XXX")) == 0)
    {
      r = 0x12345678;
      f = open ("/dev/random", O_RDONLY);
      if (f >= 0)
	{
	  read (f, &r, 4);
	  close (f);
	}
      tii = ti = time (NULL);
      ti = ti & 0xffffff;
      r += ti;

      g_snprintf (buf2, sizeof (buf2), "$1$%08lx$", r);
      key = "havenocrypt";
#ifdef HAVE_CRYPT_H
      key = crypt ("fritz", buf2);
      g_strlcpy (local_config.friends_id, (key + 12), sizeof (local_config.friends_id));
#else
      r = r * r;
      g_snprintf (local_config.friends_id, sizeof (local_config.friends_id),
		  "nocrypt%015ld", labs (r));
#endif
      printf ("\nKey: %s,id: %s %Zu bytes, time: %ld\n", key,
	      local_config.friends_id, strlen (local_config.friends_id), ti);
      current.needtosave = TRUE;
    }
  friends = malloc (MAXLISTENTRIES * sizeof (friendsstruct));
  fserver = malloc (1 * sizeof (friendsstruct));

  return (0);
}


/* *****************************************************************************
 */
void
drawfriends (void)
{
  gint i;
  gint posxdest, posydest;
  gdouble clong, clat, heading;
  gint width, height;
  struct tm *t;
  time_t ti, tif;

  actualfriends = 0;
  /*   g_print("Maxfriends: %d\n",maxfriends); */
  for (i = 0; i < maxfriends; i++)
    {

      /* return if too old  */
      ti = time (NULL);
      tif = atol ((friends + i)->timesec);
      if (!(tif > 1000000000))
	fprintf (stderr,
		 "Format error! timesec: %s, Name: %s, i: %d\n",
		 (friends + i)->timesec, (friends + i)->name, i);
      if ((ti - local_config.friends_maxsecs) > tif)
	continue;
      actualfriends++;
      coordinate_string2gdouble ((friends + i)->lon, &clong);
      coordinate_string2gdouble ((friends + i)->lat, &clat);

      calcxy (&posxdest, &posydest, clong, clat, current.zoom);

      /* If Friend is visible inside SCREEN display him/her */
      if ((posxdest >= 0) && (posxdest < gui_status.mapview_x))
	{

	  if ((posydest >= 0) && (posydest < gui_status.mapview_y))
	    {

	      gdk_draw_pixbuf (drawable, kontext_map,
			       friendspixbuf, 0, 0,
			       posxdest - 18, posydest - 12,
			       39, 24, GDK_RGB_DITHER_NONE, 0, 0);

	      /*  draw pointer to direction */
	      heading =
		strtod ((friends + i)->heading, NULL) * M_PI / 180.0;
	      draw_posmarker
	      	(posxdest, posydest, heading, &colors.blue, 1, FALSE, FALSE);

	      {	/* print friends name / speed on map */
		PangoFontDescription *pfd;
		PangoLayout *wplabellayout;
		gchar txt[200], txt2[100], s1[10];
		time_t sec;
		char *as, day[20], dispname[40];
		int speed, ii;

		sec = atol ((friends + i)->timesec);
		sec += 3600 * current.timezone;
		t = gmtime (&sec);

		as = asctime (t);
		sscanf (as, "%s", day);
		sscanf ((friends + i)->speed, "%d", &speed);

		/* replace _ with  spaces in name */
		g_strlcpy (dispname, (friends + i)->name, sizeof (dispname));
		for (ii = 0; (size_t) ii < strlen (dispname); ii++)
		  if (dispname[ii] == '_')
		    dispname[ii] = ' ';

		g_snprintf (txt, sizeof (txt),
			    "%s, %d ", dispname, (int) (speed * milesconv));
		if (local_config.distmode == DIST_MILES)
		  g_snprintf (s1, sizeof (s1), "%s", _("mi/h"));
		else if (local_config.distmode == DIST_NAUTIC)
		  g_snprintf (s1, sizeof (s1), "%s", _("knots"));
		else
		  g_snprintf (s1, sizeof (s1), "%s", _("km/h"));
		g_strlcat (txt, s1, sizeof (txt));
		g_snprintf (txt2, sizeof (txt2),
			    "\n%s, %2d:%02d\n", day, t->tm_hour, t->tm_min);
		g_strlcat (txt, txt2, sizeof (txt));
		wplabellayout =
			gtk_widget_create_pango_layout (map_drawingarea, txt);
		if (local_config.guimode == GUI_PDA)
			pfd = pango_font_description_from_string ("Sans 8");
		else
		if (local_config.guimode == GUI_CAR)
			pfd = pango_font_description_from_string ("Sans 8");
		else
			pfd = pango_font_description_from_string
				(local_config.font_friends);
		pango_layout_set_font_description (wplabellayout, pfd);
		pango_layout_get_pixel_size (wplabellayout, &width, &height);
		gdk_gc_set_foreground (kontext_map, &colors.textbacknew);
		/*              gdk_draw_rectangle (drawable, kontext_map, 1, posxdest + 18,
		 *                                  posydest - height/2 , width + 2,
		 *                                  height + 2);
		 */

		gdk_draw_layout_with_colors (drawable,
			kontext_map,
			posxdest + 21,
			posydest - height / 2 + 1,
			wplabellayout, &colors.black, NULL);
		gdk_draw_layout_with_colors (drawable,
			kontext_map,
			posxdest + 20,
			posydest - height / 2,
			wplabellayout, &colors.friends, NULL);

		if (wplabellayout != NULL)
		  g_object_unref (G_OBJECT (wplabellayout));
		/* freeing PangoFontDescription, cause it has been copied by prev. call */
		pango_font_description_free (pfd);

	      }


	    }
	}
    }
}
