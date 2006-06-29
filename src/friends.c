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
#include <math.h>

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

extern char friendsidstring[40], friendsname[40];
extern int needtosave, maxfriends, statusid;
extern friendsstruct *friends, *fserver;
int actualfriends = 0;
extern long int maxfriendssecs;
extern gint zone;
extern gdouble milesconv;
extern gint milesflag, metricflag, nauticflag;
extern GtkWidget *drawing_area, *drawing_bearing;
extern gint pdamode;
extern gint zoom;
extern GdkPixbuf *friendsimage, *friendspixbuf;

/*
 * conn.c
 */
int sockfd = -1;
int pleasepollme = 0;
extern GtkItemFactory *item_factory;
extern int debug, statuslock;
extern GtkWidget *frame_status;
extern int errno;
extern gchar messagename[40], messagesendtext[1024], messageack[100];

#define MAXLINE 512

void
setnonblocking (int sock)
{
  int opts;

  opts = fcntl (sock, F_GETFL);
  if (opts < 0)
    {
      perror ("fcntl(F_GETFL)");
      exit (EXIT_FAILURE);
    }
  opts = (opts | O_NONBLOCK);
  if (fcntl (sock, F_SETFL, opts) < 0)
    {
      perror ("fcntl(F_SETFL)");
      exit (EXIT_FAILURE);
    }
  return;
}

int
friends_sendmsg (char *serverip, char *message)
{
  int n, nosent, endflag, e;
  char recvline[MAXLINE + 1];
  int i, fc;
  struct sockaddr_in cli_addr;
  struct sockaddr_in serv_addr;
  struct sockaddr *pserv_addr;
  socklen_t servlen;
  friendsstruct *f;
  char msgname[40], msgid[40], msgtext[1024];
  GtkWidget *wi;

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
	bzero ((char *) &serv_addr, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;

	serv_addr.sin_addr.s_addr = inet_addr (serverip);
	serv_addr.sin_port = htons (SERV_UDP_PORT);
	pserv_addr = (struct sockaddr *) &serv_addr;

	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	  {
	    perror ("friendsclient local socket");
	    return (1);
	  }
	setnonblocking (sockfd);

	servlen = sizeof (serv_addr);
	bzero ((char *) &cli_addr, sizeof (cli_addr));

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
	  usleep (100000);
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
			  "POS: %s %s %s %s %s %s %s",
			  (f + fc)->id, (f + fc)->name,
			  (f + fc)->lat, (f + fc)->lon,
			  (f + fc)->timesec,
			  (f + fc)->speed, (f + fc)->heading);
	      /*              printf("\nreceived %d arguments\n",e);  */
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
		  wi = gtk_item_factory_get_item
		    (item_factory, N_("/Misc. Menu/Messages"));
		  gtk_widget_set_sensitive (wi, TRUE);
		  gtk_statusbar_pop (GTK_STATUSBAR (frame_status), statusid);
		  statuslock = FALSE;
		}
	      else
		{
		  e = sscanf (recvline,
			      "SND: %s %s %[^\n]", msgid, msgname, msgtext);
		  if (e == 3)
		    if (strcmp ((msgname), (friendsname)) == 0)
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
friendsinit ()
{

  char *key, buf2[20];
  int f;
  long int r;
  time_t ti, tii;

  if ((strcmp (friendsidstring, "XXX")) == 0)
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
      g_strlcpy (friendsidstring, (key + 12), sizeof (friendsidstring));
#else
      r = r * r;
      g_snprintf (friendsidstring, sizeof (friendsidstring),
		  "nocrypt%015ld", labs (r));
#endif
      printf ("\nKey: %s,id: %s %d bytes, time: %ld\n", key,
	      friendsidstring, strlen (friendsidstring), ti);
      needtosave = 1;
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
  gdouble posxdest, posydest, clong, clat, direction;
  gint width, height;
  gdouble w;
  GdkPoint poly[16];
  struct tm *t;
  time_t ti, tif;
#define PFSIZE 55

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
      if ((ti - maxfriendssecs) > tif)
	continue;
      actualfriends++;
      coordinate_string2gdouble ((friends + i)->lon, &clong);
      coordinate_string2gdouble ((friends + i)->lat, &clat);

      calcxy (&posxdest, &posydest, clong, clat, zoom);

      /* If Friend is visible inside SCREEN display him/her */
      if ((posxdest >= 0) && (posxdest < SCREEN_X))
	{

	  if ((posydest >= 0) && (posydest < SCREEN_Y))
	    {

	      gdk_draw_pixbuf (drawable, kontext,
			       friendspixbuf, 0, 0,
			       posxdest - 18, posydest - 12,
			       39, 24, GDK_RGB_DITHER_NONE, 0, 0);
	      gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);

	      /*  draw pointer to direction */
	      direction =
		strtod ((friends + i)->heading, NULL) * M_PI / 180.0;
	      w = direction + M_PI;
	      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	      poly[0].x = posxdest + (PFSIZE) / 2.3 * (cos (w + M_PI_2));
	      poly[0].y = posydest + (PFSIZE) / 2.3 * (sin (w + M_PI_2));
	      poly[1].x = posxdest + (PFSIZE) / 9 * (cos (w + M_PI));
	      poly[1].y = posydest + (PFSIZE) / 9 * (sin (w + M_PI));
	      poly[2].x = posxdest + PFSIZE / 10 * (cos (w + M_PI_2));
	      poly[2].y = posydest + PFSIZE / 10 * (sin (w + M_PI_2));
	      poly[3].x = posxdest - (PFSIZE) / 9 * (cos (w + M_PI));
	      poly[3].y = posydest - (PFSIZE) / 9 * (sin (w + M_PI));
	      poly[4].x = poly[0].x;
	      poly[4].y = poly[0].y;
	      gdk_gc_set_foreground (kontext, &blue);
	      gdk_draw_polygon (drawable, kontext, 0, (GdkPoint *) poly, 5);
	      gdk_draw_arc (drawable, kontext, 0,
			    posxdest + 2 - 7,
			    posydest + 2 - 7, 10, 10, 0, 360 * 64);

	      /*   draw + sign at destination   */
	      gdk_gc_set_foreground (kontext, &red);
	      gdk_draw_line (drawable, kontext,
			     posxdest + 1, posydest + 1 - 5,
			     posxdest + 1, posydest + 1 + 5);
	      gdk_draw_line (drawable, kontext,
			     posxdest + 1 + 5, posydest + 1,
			     posxdest + 1 - 5, posydest + 1);

	      {			/* print friends name / speed on map */
		PangoFontDescription *pfd;
		PangoLayout *wplabellayout;
		gchar txt[200], txt2[100], s1[10];
		time_t sec;
		char *as, day[20], dispname[40];
		int speed, ii;

		sec = atol ((friends + i)->timesec);
		sec += 3600 * zone;
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
			    "%s,%d", dispname, (int) (speed * milesconv));
		if (milesflag)
		  g_snprintf (s1, sizeof (s1), "%s", _("mi/h"));
		else if (nauticflag)
		  g_snprintf (s1, sizeof (s1), "%s", _("knots"));
		else
		  g_snprintf (s1, sizeof (s1), "%s", _("km/h"));
		g_strlcat (txt, s1, sizeof (txt));
		g_snprintf (txt2, sizeof (txt2),
			    "%s, %2d:%02d\n", day, t->tm_hour, t->tm_min);
		g_strlcat (txt, txt2, sizeof (txt));
		wplabellayout =
		  gtk_widget_create_pango_layout (drawing_area, txt);
		if (pdamode)
		  pfd = pango_font_description_from_string ("Sans 8");
		else
		  pfd = pango_font_description_from_string ("Sans bold 11");
		pango_layout_set_font_description (wplabellayout, pfd);
		pango_layout_get_pixel_size (wplabellayout, &width, &height);
		gdk_gc_set_foreground (kontext, &textbacknew);
		/*              gdk_draw_rectangle (drawable, kontext, 1, posxdest + 18,
		 *                                  posydest - height/2 , width + 2,
		 *                                  height + 2);
		 */

		gdk_draw_layout_with_colors (drawable,
					     kontext,
					     posxdest
					     + 21,
					     posydest
					     -
					     height /
					     2 + 1,
					     wplabellayout, &black, NULL);
		gdk_draw_layout_with_colors (drawable,
					     kontext,
					     posxdest
					     + 20,
					     posydest
					     -
					     height /
					     2, wplabellayout, &orange, NULL);

		if (wplabellayout != NULL)
		  g_object_unref (G_OBJECT (wplabellayout));
		/* freeing PangoFontDescription, cause it has been copied by prev. call */
		pango_font_description_free (pfd);

	      }


	    }
	}
    }
}
