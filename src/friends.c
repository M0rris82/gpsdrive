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
#include "database.h"
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

typedef struct
{
	gchar id[30];
	gchar name[40];
	gchar type[40];
	gchar lat[40], lon[40];
	gchar timesec[40], speed[10], course[10];
}
friendsstruct;


gint maxfriends = 0;
int actualfriends = 0;
extern int messagenumber;
extern long int maxfriendssecs;
extern GtkWidget *map_drawingarea;
extern gint mydebug;
extern color_struct colors;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern GdkGC *kontext_map;
extern GtkTreeStore *poi_types_tree;
extern GHashTable *poi_types_hash;

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

/* global variables */
GtkListStore *friends_list;

/* local variables */
static friendsstruct *fserver;
static friendsstruct *friends_buf;


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
			if (gui_status.expmode)
			{
				g_snprintf (la, sizeof (la), "%10.6f",
					coords.expmode_lat);
				g_snprintf (lo, sizeof (lo), "%10.6f",
					coords.expmode_lon);
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
				current.groundspeed / local_config.distfactor,
				180.0 * current.course / M_PI,
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
 * Insert or update data coming from friendsd in database and update friends_list
 */
void
update_friends_data (friendsstruct *cf)
{
	glong current_poi_id = 0;
	GtkTreeIter iter, type_iter;
	gdouble t_val;
	gchar t_value[10], t_unit[10];
	gchar t_buf[20];
	gchar *result = NULL;
	gchar *t_path;
	GdkPixbuf *t_icon;

	t_val = calcdist (g_strtod((cf)->lon, NULL), g_strtod((cf)->lat, NULL));
	distance2gchar (t_val, t_value, sizeof (t_value), t_unit, sizeof (t_unit));
	g_snprintf (t_buf, sizeof (t_buf), "%s%s", t_value, t_unit);

	t_path = g_hash_table_lookup (poi_types_hash, (cf)->type);
	if (t_path == NULL)
		t_path = g_hash_table_lookup (poi_types_hash, "people");
	if (t_path == NULL)
		t_path = g_hash_table_lookup (poi_types_hash, "unknown");

	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (poi_types_tree), &type_iter, t_path);
	gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &type_iter,
		POITYPE_ICON, &t_icon, -1);

	gtk_list_store_append (friends_list, &iter);
	gtk_list_store_set (friends_list, &iter,
		FRIENDS_ID, (cf)->id,
		FRIENDS_NAME, (cf)->name,
		FRIENDS_TYPE, (cf)->type,
		FRIENDS_LAT, g_strtod((cf)->lat, NULL),
		FRIENDS_LON, g_strtod((cf)->lon, NULL),
		FRIENDS_TIMESEC, atoi ((cf)->timesec),
		FRIENDS_SPEED, atoi ((cf)->speed),
		FRIENDS_COURSE, atoi ((cf)->course),
		FRIENDS_ICON, t_icon,
		FRIENDS_DIST, t_val,
		FRIENDS_DIST_TEXT, t_buf,
		FRIENDS_LAT_TEXT, "lat",
		FRIENDS_LON_TEXT, "lon",
		-1);

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
				(cf)->type, (cf)->course, 7, TRUE);
			db_poi_extra_edit (&current_poi_id, "speed", (cf)->speed, TRUE);
			db_poi_extra_edit (&current_poi_id, "course", (cf)->course, TRUE);
			db_poi_extra_edit (&current_poi_id, "timesec", (cf)->timesec, TRUE);
		}
		else
		{
			current_poi_id = db_poi_edit (0, strtod((cf)->lat, NULL),
				strtod((cf)->lon, NULL), (cf)->name, (cf)->type,
				(cf)->course, 7, FALSE);
			db_poi_extra_edit (&current_poi_id, "friends_id", (cf)->id, FALSE);
			db_poi_extra_edit (&current_poi_id, "speed", (cf)->speed, FALSE);
			db_poi_extra_edit (&current_poi_id, "course", (cf)->course, FALSE);
			db_poi_extra_edit (&current_poi_id, "timesec", (cf)->timesec, FALSE);
		}
	}
}


int
friends_sendmsg (char *serverip, char *message)
{
  gint n, nosent, endflag, e;
  gchar recvline[MAXLINE + 1];
  gint i, fc, type;
  struct sockaddr_in cli_addr;
  struct sockaddr_in serv_addr;
  struct sockaddr *pserv_addr;
  socklen_t servlen;
  friendsstruct *f;
  gchar msgname[40], msgid[40], msgtext[1024];

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
  f = g_new (friendsstruct, 1);
  gtk_list_store_clear (friends_list);
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
			  f->id, f->name, f->lat, f->lon,
			  f->timesec, f->speed, f->course,
			  &type);
	      /*              printf("\nreceived %d arguments\n",e);  */
		if (type == TRAVEL_CAR)
			g_snprintf (f->type, sizeof (f->type),
				"people.friendsd.car");
		else if (type == TRAVEL_AIRPLANE)
			g_snprintf (f->type, sizeof (f->type),
				"people.friendsd.airplane");
		else if (type == TRAVEL_BIKE)
			g_snprintf (f->type, sizeof (f->type),
				"people.friendsd.bike");
		else if (type == TRAVEL_BOAT)
			g_snprintf (f->type, sizeof (f->type),
				"people.friendsd.boat");
		else if (type == TRAVEL_WALK)
			g_snprintf (f->type, sizeof (f->type),
				"people.friendsd.walk");
		else
			g_snprintf (f->type, sizeof (f->type),
				"people.friendsd");
		
		update_friends_data (f);
		fc++;
	    }
	  if ((strncmp (recvline, "SRV: ", 5)) == 0)
	    {
	      e = sscanf (recvline,
			  "SRV: %s %s %s %s %s %s %s",
			  fserver->id, fserver->name,
			  fserver->lat, fserver->lon,
			  fserver->timesec, fserver->speed, fserver->course);
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
			      (msgname, sizeof (msgname), "%s", fserver->name);
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

  g_free (f);

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

	fserver = g_new (friendsstruct, 1);
	friends_buf = g_new (friendsstruct, 1);

	friends_list = gtk_list_store_new (FRIENDS_N_ITEMS,
		G_TYPE_STRING,		/* ID */
		G_TYPE_STRING,		/* NAME */
		G_TYPE_STRING,		/* TYPE */
		G_TYPE_DOUBLE,		/* LAT */
		G_TYPE_DOUBLE,		/* LON */
		G_TYPE_INT,		/* TIMESEC */
		G_TYPE_INT,		/* SPEED */
		G_TYPE_INT,		/* COURSE */
		GDK_TYPE_PIXBUF,	/* ICON */
		G_TYPE_DOUBLE,		/* DIST */
		G_TYPE_STRING,		/* DIST TEXT */
		G_TYPE_STRING,		/* LAT TEXT */
		G_TYPE_STRING		/* LON TEXT */
		);


  return (0);
}


/* *****************************************************************************
 */
void
cleanup_friends (void)
{
    g_free (fserver);
    g_free (friends_buf);
}
