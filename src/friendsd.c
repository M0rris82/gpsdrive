/* ******************************************************************
 * friendsd server
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
 Revision 1.6  2006/01/03 14:24:10  tweety
 eliminate compiler Warnings
 try to change all occurences of longi -->lon, lati-->lat, ...i
 use  drawicon(posxdest,posydest,"w-lan.open") instead of using a seperate variable
 rename drawgrid --> do_draw_grid
 give the display frames usefull names frame_lat, ...
 change handling of WP-types to lowercase
 change order for directories reading icons
 always read inconfile

 Revision 1.5  2005/11/05 19:52:17  tweety
 Fix Bufferoverflow
 http://seclists.org/lists/fulldisclosure/2005/Nov/0129.html

 Revision 1.4  2005/04/20 23:33:49  tweety
 reformatted source code with anjuta
 So now we have new indentations

 Revision 1.3  2005/04/13 19:58:31  tweety
 renew indentation to 4 spaces + tabstop=8

 Revision 1.2  2005/04/10 21:50:49  tweety
 reformatting c-sources

 Revision 1.1.1.1  2004/12/23 16:03:24  commiter
 Initial import, straight from 2.10pre2 tar.gz archive

 Revision 1.21  2004/02/08 17:16:25  ganter
 replacing all strcat with g_strlcat to avoid buffer overflows

 Revision 1.20  2004/02/08 16:35:10  ganter
 replacing all sprintf with g_snprintf to avoid buffer overflows

 Revision 1.19  2004/02/06 22:29:24  ganter
 updated README and man page

 Revision 1.18  2004/01/26 11:55:19  ganter
 just indented some files

 Revision 1.17  2004/01/22 07:13:27  ganter
 ...

 Revision 1.16  2004/01/22 06:44:12  ganter
 ...

 Revision 1.15  2004/01/22 06:38:02  ganter
 working on friendsd

 Revision 1.14  2004/01/22 05:49:22  ganter
 friendsd now sends a receiving acknoledge

 Revision 1.13  2004/01/12 21:52:02  ganter
 added friends message service

 Revision 1.12  2004/01/11 17:35:48  ganter
 drop entries which are older than 1 week

 Revision 1.11  2004/01/01 09:07:31  ganter
 v2.06
 trip info is now live updated
 added cpu temperature display for acpi
 added tooltips for battery and temperature

 Revision 1.10  2003/10/10 06:50:53  ganter
 added security patch for friendsd

 Revision 1.9  2003/09/17 12:05:14  ganter
 2.05pre1
 fixed malloc problem in friends server
 force name in friendsmode to replace space with underscore

 Revision 1.8  2003/07/25 12:17:14  ganter
 2.00

 Revision 1.7  2003/06/01 17:27:33  ganter
 v2.0pre8
 friendsmode works fine and can be set in settings menu

 Revision 1.6  2003/05/31 20:12:35  ganter
 new UDP friendsserver build in, needs some work

 Revision 1.3  2003/05/31 18:25:57  ganter
 starting buildin new server and client

 Revision 1.2  2003/05/30 15:35:16  ganter
 testing

*/


#include "../config.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#define	SERV_UDP_PORT	50123

/* maximum age of data (1 week) */
#define MAXSEC 7*24*3600

/* max age of messages (2 days) */
#define MAXMSGTIME 48*3600

char *pname;

/*
 * conn.c
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifdef HAVE_LINUX_INET_H
#include "linux/inet.h"
#endif
#include <fcntl.h>
#include <netdb.h>
#include <gpsdrive.h>



struct
{
	char id[31];
	char txt[1024];
	long int times;
} *list;

static int listnum = 0, messagecounter = 0;
char serverid[31];
char serverstring[40];

void
dg_echo (int sockfd, struct sockaddr *pcli_addr, int maxclilen)
{
	int n, e, i, nosent, l, newclient;
	char mesg[MAXMESG], txt[MAXMESG + 20];
	char *fromaddr, hname[256];
	struct in_addr iaddr;
	socklen_t clilen;
	struct hostent *hostname;
	struct sockaddr_in sin;
	char id[31], name[41], lat[41], lon[41], timesec[41], speed[11],
		heading[11];
	char msgname[40], msgtext[1024], ackid[40];

	for (;;)
	{
		clilen = maxclilen;
		memset (mesg, 0, MAXMESG);
		n = recvfrom (sockfd, mesg, MAXMESG, 0, pcli_addr, &clilen);

		if (n < 0)
		{
			perror ("recvfrom");
		}
		else
		{
			/*  got string  */
			g_strlcpy (msgname, "", sizeof (msgname));
			g_strlcpy (msgtext, "", sizeof (msgtext));
			newclient = 1;

			if ((strncmp (mesg, "SND: ", 5)) == 0)
			{
				e = sscanf (mesg, "SND: %s %s %[^\n]", id,
					    msgname, msgtext);
				if (e == 3)
				{
					fprintf (stderr,
						 "\ne: %d received for %s: %s\n",
						 e, msgname, msgtext);
					for (i = 0; i < listnum; i++)
					{
						/*  id is already here  */
						if ((strcmp
						     ((list + i)->id,
						      id)) == 0)
						{
							newclient = 0;
							strncpy ((list +
								  i)->txt,
								 mesg,
								 MAXMESG - 1);
							(list + i)->times =
								time (NULL) -
								MAXSEC +
								MAXMSGTIME;
						}
					}
					if (newclient)
					{
						/* new id found  */
						listnum++;
						if (listnum >= MAXLISTENTRIES)
							listnum = 0;
						strncpy ((list + i)->txt,
							 mesg, MAXMESG - 1);
						strncpy ((list + i)->id, id,
							 30);
						(list + i)->times =
							time (NULL) - MAXSEC +
							MAXMSGTIME;
					}
				}
			}
			if ((strncmp (mesg, "POS: ", 5)) == 0)
			{
				/*  found POS string  */

				e = sscanf (mesg,
					    "POS: %30s %40s %40s %40s %40s %10s %10s",
					    id, name, lat, lon, timesec,
					    speed, heading);
				/*              printf("\nGot %d arguments\n",e);  */
				if ((e == 7)
				    && (strstr (id, "queryqueryqueryqueryqu")
					== NULL))
				{
					/* string is a POS string  */
					for (i = 0; i < listnum; i++)
					{
						/*  id is already here  */
						if ((strcmp
						     ((list + i)->id,
						      id)) == 0)
						{
							newclient = 0;
							strncpy ((list +
								  i)->txt,
								 mesg, 200);
							(list + i)->times =
								atol
								(timesec);
						}
					}

					if (newclient)
					{
						/* new id found  */
						listnum++;
						if (listnum >= MAXLISTENTRIES)
							listnum = 0;
						strncpy ((list + i)->txt,
							 mesg, 200);
						strncpy ((list + i)->id, id,
							 30);
						(list + i)->times =
							atol (timesec);
					}
				}
			}
			/* send ack message to sender and delete messages where we got an ACK */
			if ((strncmp (mesg, "ACK: ", 5)) == 0)
			{
				char recname[80], tmp[80], msgname[80];
				g_strlcpy (recname, "", sizeof (recname));
				e = sscanf (mesg, "ACK: %s ", ackid);
				if (e == 1)
					for (i = 0; i < listnum; i++)
					{
						if ((strcmp
						     ((list + i)->id,
						      ackid)) == 0)
						{
							int j, own;
							char sid[40];
							/* find sender of orig message (msgname) */
							if (strcmp
							    ((ackid + 5),
							     (serverid +
							      5)) == 0)
								own = 1;
							else
								own = 0;

							if (own)
							{
								/*                          its the ack for myself, delete from list */
								for (j = i;
								     j <
								     listnum;
								     j++)
									*(list
									  +
									  j) =
									 *
									 (list
									  +
									  j +
									  1);
								listnum--;
								fprintf (stderr, "ack for my OWN msg %s, deleting entry\n", ackid);
								break;
							}
							else
							{
								for (j = 0;
								     j <
								     listnum;
								     j++)
									if (strcmp ((ackid + 5), (((list + j)->id) + 5)) == 0)
									{
										e = sscanf ((list + j)->txt, "POS: %s %s", tmp, msgname);
										break;
									}
								/* find receiver of message (recname) */
								e = sscanf ((list + i)->txt, "SND: %s %s", tmp, recname);
								g_snprintf
									(sid,
									 sizeof
									 (sid),
									 "MSG%02d%s",
									 messagecounter++,
									 (serverid
									  +
									  5));
								g_snprintf ((list + i)->id, sizeof (list->id), sid);
								g_snprintf ((list + i)->txt, sizeof (list->txt), "SND: %s %s \nConfirmation:\n The user %s has read your message!", sid, msgname, recname);
								(list +
								 i)->times =
								 time (NULL) -
								 MAXSEC +
								 MAXMSGTIME;
								fprintf (stderr, "received acknoledge for msg %s, deleting entry\n", ackid);
							}

						}
					}
			}

			/* sort out entries older than MAXSEC seconds */
			for (i = 0; i < listnum; i++)
			{
				time_t tii;

				if ((strncmp ((list + i)->txt, "SRV:", 4)) !=
				    0)
				{
					tii = time (NULL);
					if ((tii - (list + i)->times) >
					    MAXSEC)
					{
						int j;
						for (j = i; j < listnum; j++)
							*(list + j) =
								*(list + j +
								  1);
						listnum--;
					}
				}
			}
		}

		memcpy (&iaddr, (pcli_addr->sa_data + 2), 4);
		fromaddr = inet_ntoa (iaddr);

		bzero ((caddr_t *) & sin, sizeof (sin));	/* clear out the structure */
		sin.sin_family = AF_INET;

		sin.sin_addr.s_addr = inet_addr (fromaddr);



		hostname = gethostbyaddr ((char *) &(sin.sin_addr),
					  sizeof (sin.sin_addr),
					  (int) sin.sin_family);


		if (hostname == NULL)
		{
			perror ("hostname");
			g_strlcpy (hname, "unknown", sizeof (hname));
		}
		else
			g_strlcpy (hname, hostname->h_name, sizeof (hname));

		mesg[n - 1] = 0;

		for (i = -1; i <= listnum; i++)
		{
			if (i == -1)
				g_strlcpy (txt, "$START:$", sizeof (txt));
			else if (i == listnum)
				g_strlcpy (txt, "$END:$", sizeof (txt));
			else
				g_strlcpy (txt, (list + i)->txt,
					   sizeof (txt));

			g_strlcat (txt, "\n", sizeof (txt));
			l = strlen (txt);
			if (i == -1)
				fprintf (stderr,
					 "%d clients, last: %s[%s]:\n",
					 listnum, hname, fromaddr);

			fprintf (stderr, "%s",txt);
			/*       printf ("sende\n%s, Laenge %d, clilen %d", txt, l,clilen);  */
			if ((nosent =
			     sendto (sockfd, txt, l, 0, pcli_addr,
				     clilen)) != l)
			{
				perror ("sendto");
				return;
			}
		}
		fprintf (stderr, "\n");
	}
}



/*
 * server.c
 */




void
ignore_pipe (void)
{
	struct sigaction sig;

	sig.sa_handler = SIG_IGN;
	sig.sa_flags = 0;
	sigemptyset (&sig.sa_mask);
	sigaction (SIGPIPE, &sig, NULL);
}

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
friendsinit ()
{

	char *key, buf2[20];
	int f;
	long int r;
	time_t ti, tii;
	char friendsidstring[31];
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
	key = crypt ("servr", buf2);
	g_strlcpy (friendsidstring, (key + 12), sizeof (friendsidstring));
#else
	r = r * r;
	g_snprintf (friendsidstring, sizeof (friendsidstring),
		    "nocrypt%015ld", labs (r));
#endif
	printf ("\nKey: %s,id: %s %d bytes, time: %ld\n", key,
		friendsidstring, strlen (friendsidstring), ti);

	g_strlcpy (serverid, friendsidstring, sizeof (serverid));
	return (0);
}

int
main (int argc, char *argv[])
{
	int sockfd, bindno, i;
	struct sockaddr_in serv_addr, cli_addr;


	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "utf8");
	textdomain (GETTEXT_PACKAGE);
	textdomain (NULL);
	pname = argv[0];
	g_strlcpy (serverstring, "Friendsserver", sizeof (serverstring));

	if (geteuid () == 0)
	{
		fprintf (stderr, _("server: please don't run me as root\n"));
		exit (1);
	}
	i = getopt (argc, argv, "n:h?");
	switch (i)
	{
	case 'n':
		g_strlcpy (serverstring, optarg, sizeof (serverstring));
		break;
	case 'h':
	case '?':
		printf (_
			("\nUsage:\n   %s -n servername\nprovides a name for your server\n"),
			pname);
		exit (0);
		break;
	}
	fprintf (stderr,
		 "\nGpsDrive v%s friendsd server Version 2, listening on UDP port %d...\n",
		 VERSION, SERV_UDP_PORT);
	ignore_pipe ();

	friendsinit ();
	list = malloc (MAXLISTENTRIES * sizeof (*list));

	/* make the first entry */
	g_snprintf ((list + listnum)->id, sizeof (list->id), serverid);
	g_snprintf ((list + listnum)->txt, sizeof (list->txt),
		    "SRV: %s %s 53.566593   9.948155 %d 0 0", serverid,
		    serverstring, (int) time (NULL));
	(list + listnum)->times = time (NULL);
	listnum++;

	/*   printf ("\nsizeoflist: %d\n", sizeof (*list)); */
	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		fprintf (stderr, "server: errno = %d\n", errno);
		fprintf (stderr, "server: can't open datagram socket\n");
		exit (1);
	}
	/*  setnonblocking(sockfd);  */

	fprintf (stderr, "server: sockfd = %d\n", sockfd);
	bzero ((char *) &serv_addr, sizeof (serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	serv_addr.sin_port = htons (SERV_UDP_PORT);

	if ((bindno =
	     bind (sockfd, (struct sockaddr *) &serv_addr,
		   sizeof (serv_addr))) < 0)
	{
		fprintf (stderr, "server: errno = %d\n", errno);
		fprintf (stderr, "server: can't bind local address\n");
		exit (2);
	}

	fprintf (stderr, "server: bindno = %d\n", bindno);

	dg_echo (sockfd, (struct sockaddr *) &cli_addr, sizeof (cli_addr));
	return 0;
}
