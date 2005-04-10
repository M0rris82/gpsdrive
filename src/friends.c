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
 $Log$
 Revision 1.2  2005/04/10 21:50:49  tweety
 reformatting c-sources

 Revision 1.1.1.1  2004/12/23 16:03:24  commiter
 Initial import, straight from 2.10pre2 tar.gz archive

 Revision 1.37  2004/02/08 16:35:10  ganter
 replacing all sprintf with g_snprintf to avoid buffer overflows

 Revision 1.36  2004/02/07 15:53:38  ganter
 replacing strcpy with g_strlcpy to avoid bufferoverflows

 Revision 1.35  2004/02/02 03:38:31  ganter
 code cleanup

 Revision 1.34  2004/01/28 15:31:43  ganter
 initialize FDs to -1

 Revision 1.33  2004/01/28 09:32:57  ganter
 tested for memory leaks with valgrind, looks good :-)

 Revision 1.32  2004/01/26 11:55:19  ganter
 just indented some files

 Revision 1.31  2004/01/22 07:13:27  ganter
 ...

 Revision 1.30  2004/01/22 06:44:12  ganter
 ...

 Revision 1.29  2004/01/22 06:38:02  ganter
 working on friendsd

 Revision 1.28  2004/01/15 22:02:40  ganter
 added openbsd patches
 real 2.07pre9

 Revision 1.27  2004/01/14 03:47:36  ganter
 removed some debug output

 Revision 1.26  2004/01/14 03:31:37  ganter
 now message acknoledge is done to and from friendsserver

 Revision 1.25  2004/01/14 00:48:49  ganter
 fixed bug if no crypt is avail.

 Revision 1.24  2004/01/14 00:06:27  ganter
 ...

 Revision 1.23  2004/01/13 23:38:30  ganter
 added new field in waypoints display for number of friends received

 Revision 1.22  2004/01/13 19:30:41  ganter
 changed "operations menu" do "Misc. menu"

 Revision 1.21  2004/01/13 14:31:31  ganter
 status bar

 Revision 1.20  2004/01/13 14:09:43  ganter
 ...

 Revision 1.19  2004/01/13 00:31:41  ganter
 fixed multiline message bug

 Revision 1.18  2004/01/12 23:53:08  ganter
 grrrrrrrrrrrr

 Revision 1.17  2004/01/12 23:52:15  ganter
 grrr

 Revision 1.16  2004/01/12 23:41:34  ganter
 fixed name bug again, upload tar and cvs again

 Revision 1.15  2004/01/12 23:22:34  ganter
 fixed wrong sender name in message

 Revision 1.14  2004/01/12 22:22:06  ganter
 make message menu entry insensitive if message is not yet send

 Revision 1.13  2004/01/12 21:52:02  ganter
 added friends message service

 Revision 1.12  2004/01/01 09:07:31  ganter
 v2.06
 trip info is now live updated
 added cpu temperature display for acpi
 added tooltips for battery and temperature

 Revision 1.11  2003/10/04 17:43:58  ganter
 translations don't need to be utf-8, but the .po files must specify the
 correct coding (ie, UTF-8, iso8859-15)

 Revision 1.10  2003/08/12 12:47:15  ganter
 v2.03 workaround for missing crypt()

 Revision 1.9  2003/07/25 12:17:14  ganter
 2.00

 Revision 1.8  2003/06/08 13:31:49  ganter
 release 2.0pre9
 Added setting of timeperiod in friends mode (see settings menu)

 Revision 1.7  2003/05/31 20:32:01  ganter
 friendsd2 works fine with sven's server

 Revision 1.4  2003/05/31 18:25:57  ganter
 starting buildin new server and client

 Revision 1.3  2003/05/30 18:59:15  ganter
 client server working, but not perfectly

 Revision 1.2  2003/05/30 15:35:16  ganter
 testing

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
#include <sys/socket.h>
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
/*
 * conn.c
 */
int sockfd = -1;
int pleasepollme = 0;
extern GtkItemFactory *item_factory;
extern int debug, statuslock;
extern GtkWidget *status;
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

				if (bind (sockfd, (struct sockaddr *) &cli_addr, sizeof (cli_addr)) <
						0)
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
										(struct sockaddr *) 0, (int *) 0);
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
					/* 	  if (debug) */
					/* 	    printf ("received...%d bytes: %s\n=======\n", n, recvline); */
					/* scanning reply  */
					if ((strncmp (recvline, "POS: ", 5)) == 0)
						{
							e =
								sscanf (recvline, "POS: %s %s %s %s %s %s %s", (f + fc)->id,
												(f + fc)->name, (f + fc)->lat, (f + fc)->longi,
												(f + fc)->timesec, (f + fc)->speed,
												(f + fc)->heading);
							/* 		    printf("\nreceived %d arguments\n",e);  */
							fc++;
						}
					if ((strncmp (recvline, "SRV: ", 5)) == 0)
						{
							e =
								sscanf (recvline, "SRV: %s %s %s %s %s %s %s", fserver->id,
												fserver->name, fserver->lat, fserver->longi,
												fserver->timesec, fserver->speed, fserver->heading);
							/* 		    printf("\nreceived %d arguments\n",e);  */
						}
					if ((strncmp (recvline, "SND: ", 5)) == 0)
						{
							if ((strlen (messageack) > 0)
									&& (strncmp (recvline, messageack, strlen (messageack)) ==
											0))
								{
									g_strlcpy (messagename, "", sizeof (messagename));
									g_strlcpy (messageack, "", sizeof (messageack));
									g_strlcpy (messagesendtext, "", sizeof (messagesendtext));
									wi =
										gtk_item_factory_get_item (item_factory,
																							 N_("/Misc. Menu/Messages"));
									gtk_widget_set_sensitive (wi, TRUE);
									gtk_statusbar_pop (GTK_STATUSBAR (status), statusid);
									statuslock = FALSE;
								}
							else
								{
									e =
										sscanf (recvline, "SND: %s %s %[^\n]", msgid, msgname,
														msgtext);
									if (e == 3)
										if (strcmp ((msgname), (friendsname)) == 0)
											{
												int j, k = 0, fsmessage = 0;

												g_strlcpy (msgname, _("unknown"), sizeof (msgname));
												if (strcmp ((msgid + 5), ((fserver->id) + 5)) == 0)
													{
														g_snprintf (msgname, sizeof (msgname),
																				fserver->name);
														fsmessage = TRUE;
													}
												for (j = 0; j < fc; j++)
													if (strcmp ((msgid + 5), (((f + j)->id) + 5)) == 0)
														g_strlcpy (msgname, (f + j)->name,
																			 sizeof (msgname));
												for (j = 0; j < (int) strlen (recvline); j++)
													{
														if (*(recvline + j) == ' ')
															k++;
														if (k >= 3)
															break;
													}
												g_strlcpy (msgtext, (recvline + j + 1),
																	 sizeof (msgtext));
												/* 			if (debug) */
												/* 			  fprintf (stderr, "\ne: %d, received from %s: %s\n", */
												/* 				   e, msgname, msgtext); */
												message_cb (msgid, msgname, msgtext, fsmessage);
											}
								}
						}

					if (debug)
						fprintf (stderr, recvline);

				}
			/*  	printf("\ni: %d, endflag: %d\n",i,endflag);   */
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
      g_snprintf (friendsidstring, sizeof (friendsidstring), "nocrypt%015ld",
									labs (r));
#endif
      printf ("\nKey: %s,id: %s %d bytes, time: %ld\n", key,
							friendsidstring, strlen (friendsidstring), ti);
      needtosave = 1;
    }
  friends = malloc (MAXLISTENTRIES * sizeof (friendsstruct));
  fserver = malloc (1 * sizeof (friendsstruct));

  return (0);
}
