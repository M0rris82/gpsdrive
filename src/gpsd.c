/****************************************************************************
COPYRIGHT
=========

This software (gpsd) is Copyrighted 1997, 1998, 1999, 2000 by Remco Treffkorn,
I am releasing it under the terms and conditions of the

        GNU GENERAL PUBLIC LICENSE Version 2, June 1991

or later. A copy off which is included in the file COPYING.


Patch for file input: Iwo Mergler <Iwo.Mergler@soton.sc.philips.com>

****************************************************************************/

/*
  $Log$
  Revision 1.4  2005/04/20 23:33:49  tweety
  reformatted source code with anjuta
  So now we have new indentations

  Revision 1.3  2005/04/13 19:58:31  tweety
  renew indentation to 4 spaces + tabstop=8

  Revision 1.2  2005/04/10 21:50:49  tweety
  reformatting c-sources
	
  Revision 1.1.1.1  2004/12/23 16:03:24  commiter
  Initial import, straight from 2.10pre2 tar.gz archive

  Revision 1.16  2004/02/03 08:34:16  ganter
  added gpsd patches from David Clayton which fixes some bufferoverflows
  and added support for GPGLL sentence

  Revision 1.15  2003/12/28 00:15:47  ganter
  added gpsd patch from Mina Naguib <webmaster@topfx.com>
  that allows GPSD to accept serial speeds up to 115200 via the -s commandline
  switch.

  Revision 1.14  2003/01/23 14:05:34  ganter
  added greek translation
  added geocache scripts
  added geocache icon
  improved search for libmysqlclient.so

  Revision 1.13  2002/11/06 05:29:14  ganter
  fixed most warnings

  Revision 1.12  2002/10/14 08:38:59  ganter
  v1.29pre3
  added SQL support

  Revision 1.11  2002/09/19 07:33:36  ganter
  added FAQ

  Revision 1.10  2002/09/17 20:57:58  ganter
  added copyright and README for gpsd
  v1.27

  Revision 1.9  2002/09/12 10:10:14  ganter
  v1.27pre2

  Revision 1.7  2002/06/27 19:51:28  ganter
  patches from Marco Molteni <molter@tin.it>

  Revision 1.6  2002/04/19 18:26:14  ganter
  Thanks to the patch of Derrick J Brashear now some more receivers
  are providing the altitude.

  Revision 1.5  2002/04/18 22:50:37  ganter
  added turkish translation
  added display number of satellites
  v1.18pre2

  Revision 1.4  2002/04/06 17:08:56  ganter
  v1.16pre8
  cleanup of gpsd files

  Revision 1.3  2002/04/06 00:36:01  ganter
  changing filelist

  Revision 1.2  2002/04/05 19:08:29  ganter
  added comment

  Version modified by Fritz Ganter <ganter@ganter.at>

*/

#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#if defined (HAVE_PATH_H)
#include <paths.h>
#else
#if !defined (_PATH_DEVNULL)
#define _PATH_DEVNULL    "/dev/null"
#endif
#endif

#if defined (HAVE_STRINGS_H)
#include <strings.h>
#endif


#if defined (HAVE_SYS_TERMIOS_H)
#include <sys/termios.h>
#else
#if defined (HAVE_TERMIOS_H)
#include <termios.h>
#endif
#endif

#if defined (HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

#if defined (HAVE_NETDB_H)
#include <netdb.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>

#include "nmea.h"
#include "gpsd.h"
#include "version.h"

#define QLEN		5
#define BUFSIZE		4096
#define GPS_TIMEOUT	5	/* Consider GPS connection loss after 5 sec */

int debug = 0;
int device_speed = B4800;
int device_type;
int nfds;			/* number of file descriptors to select() for */
char *device_name = 0;
char *latitude = 0;
char *longitude = 0;
char latd = 'N';
char lond = 'W';
int keep_gps = 0;		/* If non-zero, don't close gps device ever */

/* File input stuff */
char *infile_name = 0;
int infile = 0;
double InfileLineTime = 0.2;

/* command line option defaults */
static char *default_device_name = "/dev/gps";
static char *default_latitude = "3600.000";
static char *default_longitude = "-12300.000";

static int dsock;		/* socket to dpgs server */

/* If set, close (and reopen?) the serial interface. Triggered by SIGUSR1. */
static int reopen = 0;


static int handle_input (int input, fd_set * afds, fd_set * nmea_fds);
static int handle_request (int fd, fd_set * fds);

extern int handle_EMinput (int input, fd_set * afds, fd_set * nmea_fds);
extern int connectsock (char *host, char *service, char *protocol);
extern void em_send_rtcm (unsigned short *rtcmbuf, int rtcmbytes);


static void
onsig (int sig)
{
	serial_close ();
	close (dsock);
	syslog (LOG_NOTICE, "Received signal %d. Exiting...", sig);
	exit (10 + sig);
}

static void
sigusr1 (int sig)
{
	reopen = 1;
}

int
daemonize ()
{
	int fd;
	pid_t pid;

	pid = fork ();

	switch (pid)
	{
	case -1:
		return -1;
	case 0:
		break;
	default:
		_exit (pid);
	}

	if (setsid () == -1)
		return -1;
	chdir ("/");
	fd = open (_PATH_DEVNULL, O_RDWR, 0);
	if (fd != -1)
	{
		dup2 (fd, STDIN_FILENO);
		dup2 (fd, STDOUT_FILENO);
		dup2 (fd, STDERR_FILENO);
		if (fd > 2)
			close (fd);
	}
	return 0;
}

void
send_dgps ()
{
	char buf[BUFSIZE];

	sprintf (buf, "R %0.2f %0.2f %0.2f\r\n", gNMEAdata.latitude,
		 gNMEAdata.longitude, gNMEAdata.altitude);
	write (dsock, buf, strlen (buf));
}

int
main (int argc, char *argv[])
{
	char *default_service = "gpsd";
	char *default_dgpsserver = "dgps.wsrcc.com";
	char *default_dgpsport = "rtcm-sc104";
	char *service = 0;	/* port number to listen to */
	char *dgpsport = 0;
	char *dgpsserver = 0;
	struct sockaddr_in fsin;
	int msock;		/* listening socket fd */
	fd_set rfds;
	fd_set afds;
	fd_set nmea_fds;
	int alen;
	int fd;
	int input;		/* GPS device fd */
	int need_gps, need_dgps = 0, need_init = 1;
	extern char *optarg;
	int option;
	double baud;
	char buf[BUFSIZE];
	int sentdgps = 0, fixcnt = 0;
	time_t curtime;

	while ((option =
		getopt (argc, argv, "D:L:S:T:hncl:p:s:d:r:F:f:K")) != -1)
	{
		switch (option)
		{
		case 'T':
			switch (*optarg)
			{
			case 't':
				device_type = DEVICE_TRIPMATE;
				break;
			case 'e':
				device_type = DEVICE_EARTHMATE;
				break;
			default:
				fprintf (stderr,
					 "Invalid device type \"%s\"\n"
					 "Using GENERIC instead\n", optarg);
				break;
			}
			break;
		case 'D':
			debug = (int) strtol (optarg, 0, 0);
			break;
		case 'd':
			dgpsserver = optarg;
			break;
		case 'L':
			if (optarg[strlen (optarg) - 1] == 'W'
			    || optarg[strlen (optarg) - 1] == 'w'
			    || optarg[strlen (optarg) - 1] == 'E'
			    || optarg[strlen (optarg) - 1] == 'e')
			{
				lond = toupper (optarg[strlen (optarg) - 1]);
				longitude = optarg;
				longitude[strlen (optarg) - 1] = '\0';
			}
			else
				fprintf (stderr,
					 "skipping invalid longitude (-L) option;  %s must end in W or E\n",
					 optarg);
			break;
		case 'S':
			service = optarg;
			break;
		case 'r':
			dgpsport = optarg;
			break;
		case 'l':
			if (optarg[strlen (optarg) - 1] == 'N'
			    || optarg[strlen (optarg) - 1] == 'n'
			    || optarg[strlen (optarg) - 1] == 'S'
			    || optarg[strlen (optarg) - 1] == 's')
			{
				latd = toupper (optarg[strlen (optarg) - 1]);
				latitude = optarg;
				latitude[strlen (optarg) - 1] = '\0';
			}
			else
				fprintf (stderr,
					 "skipping invalid latitude (-l) option;  %s must end in N or S\n",
					 optarg);
			break;
		case 'p':
			device_name = optarg;
			break;
		case 's':
			baud = strtod (optarg, 0);
			if (baud < 200)
				baud *= 1000;
			if (baud < 2400)
				device_speed = B1200;
			else if (baud < 4800)
				device_speed = B2400;
			else if (baud < 9600)
				device_speed = B4800;
			else if (baud < 19200)
				device_speed = B9600;
			else if (baud < 38400)
				device_speed = B19200;
			else if (baud < 57600)
				device_speed = B38400;
			else if (baud < 115200)
				device_speed = B57600;
			else
				device_speed = B115200;
			break;
		case 'c':
			need_dgps = 1;
			break;
		case 'n':
			need_init = 0;
			break;
		case 'F':
			infile_name = optarg;
			infile = 1;
			break;
		case 'f':
			InfileLineTime = strtod (optarg, NULL);
			break;
		case 'K':
			keep_gps = 1;
			break;
		case 'h':
		case '?':
		default:
			fputs ("usage:  gpsd [options] \n\
  options include: \n\
  -D level     [ set debug level. If >= 2, remain in foreground ] \n\
  -L longitude [ set longitude ] \n\
  -l latitude  [ set latitude ] \n\
  -S port      [ set gpsd listening port ] \n\
  -T type      [ set DeLorme models. e = Earthmate, t = Tripmate ] \n\
  -h           [ help message ] \n\
  -p path      [ set gps device pathname ] \n\
  -s baud_rate [ set baud rate on gps device ] \n\
  -c           [ use dgps service for corrections ] \n\
  -d host      [ set dgps server hostname] \n\
  -r port      [ set dgps server port ] \n\
  -n           [ disable setting default values for longitute and latitute ] \n\
  -F filename  [ read input from file ] \n\
  -f seconds   [ delay per input file line in seconds (default=0.2)] \n\
  -K           [ keep gps device open all the time (linux USB workaround) ] \n\
", stderr);
			exit (0);
		}
	}
	if (!device_name)
		device_name = default_device_name;
	if (need_init && !latitude)
		latitude = default_latitude;
	if (need_init && !longitude)
		longitude = default_longitude;
	if (!service)
	{
		if (!getservbyname (default_service, "tcp"))
		{
			service = "2947";
		}
		else
		{
			service = default_service;
		}
	}
	if (need_dgps && !dgpsserver)
		dgpsserver = default_dgpsserver;
	if (need_dgps && !dgpsport)
		dgpsport = default_dgpsport;
	if (debug > 0)
	{
		fprintf (stderr, "command line options:\n");
		fprintf (stderr, "  debug level:        %d\n", debug);
		if (infile)
		{
			fprintf (stderr, "  gps file name:      %s\n",
				 infile_name);
			fprintf (stderr, "  file line timing:   %.3f sec\n",
				 InfileLineTime);
		}
		else
		{
			fprintf (stderr, "  gps device name:    %s\n",
				 device_name);
			fprintf (stderr, "  gps device speed:   %d\n",
				 device_speed);
		}
		fprintf (stderr, "  gpsd port:          %s\n", service);
		if (need_dgps)
		{
			fprintf (stderr, "  dgps server:        %s\n",
				 dgpsserver);
			fprintf (stderr, "  dgps port:        %s\n",
				 dgpsport);
		}
		fprintf (stderr, "  latitude:           %s%c\n", latitude,
			 latd);
		fprintf (stderr, "  longitude:          %s%c\n", longitude,
			 lond);
	}
	if (debug < 2)
		daemonize ();

	/* Handle some signals */
	signal (SIGUSR1, sigusr1);
	signal (SIGINT, onsig);
	signal (SIGHUP, onsig);
	signal (SIGTERM, onsig);
	signal (SIGQUIT, onsig);

	openlog ("gpsd", LOG_PID, LOG_USER);
	syslog (LOG_NOTICE, "Gpsd started (Version %s)", GPSDVERSION);
	if (need_dgps)
		syslog (LOG_NOTICE, "Gpsd in DGPS-IP mode");

	syslog (LOG_NOTICE, "Gpsd listening on port %s", service);

	msock = passiveTCP (service, QLEN);

	if (need_dgps)
	{
		if (!getservbyname (dgpsport, "tcp"))
			dgpsport = "2101";
		dsock = connectsock (dgpsserver, dgpsport, "tcp");
		if (dsock < 0)
		{
			fprintf (stderr, "Can't connect to dgps server\n");
			need_dgps = 0;
		}
	}
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
	/* still wrong, but at least do not coredump */
	nfds = min (getdtablesize (), FD_SETSIZE);

	FD_ZERO (&afds);
	FD_ZERO (&nmea_fds);
	FD_SET (msock, &afds);
	if (need_dgps)
	{
		char hn[256];
		gethostname (hn, sizeof (hn));

		sprintf (buf, "HELO %s gpsdrive-%s/gpsd %s\r\nR\r\n", hn,
			 VERSION, GPSDVERSION);
		write (dsock, buf, strlen (buf));
		FD_SET (dsock, &afds);
	}

	input = -1;

	while (1)
	{
		memcpy (&rfds, &afds, sizeof (rfds));

		if (select (nfds, &rfds, NULL, NULL, NULL) < 0)
		{
			if (errno == EINTR)
				continue;
			errexit ("select");
		}
		curtime = time (NULL);
		if (curtime > gNMEAdata.last_update + GPS_TIMEOUT)
		{
			gNMEAdata.mode = 0;
			gNMEAdata.status = 0;
		}
		/* data received on dgps socket */
		if (FD_ISSET (dsock, &rfds))
		{
			int rtcmbytes, cnt;
			char *ptr;

			rtcmbytes = read (dsock, buf, BUFSIZE);

			if (device_type == DEVICE_EARTHMATEb)
			{
				ptr = buf;
				while (rtcmbytes > 0)
				{
					cnt = (rtcmbytes <
					       65) ? rtcmbytes : 64;
					em_send_rtcm ((unsigned short *) buf,
						      cnt);
					rtcmbytes -= cnt;
					ptr += cnt;
				}
			}
			else
			{
				/*
				 * device must need generic RTCM-104 serial data.
				 * We can send these one character at a time. 
				 */
				if (rtcmbytes > 0)
				{
#if 0
					fprintf (stderr,
						 "\n\nSending %d rtcm bytes out\n",
						 rtcmbytes);
					for (cnt = 0; cnt < rtcmbytes; cnt++)
						fprintf (stderr, "%x",
							 (unsigned char)
							 buf[cnt]);
					fprintf (stderr, "\n");
#endif
					ptr = buf;
					while (rtcmbytes > 0)
					{
						cnt = write (gNMEAdata.fdout,
							     ptr, rtcmbytes);
						if (cnt == rtcmbytes)
							rtcmbytes = 0;	/* stops the loop */

						if (cnt > 0)
						{
							/* Set up for next iteration */
							rtcmbytes -= cnt;
							ptr += cnt;
						}
					}
					rtcmbytes = 0;
				}
			}
		}
		/* data received on client socket */
		if (FD_ISSET (msock, &rfds))
		{
			int ssock;

			alen = sizeof (fsin);
			ssock = accept (msock, (struct sockaddr *) &fsin,
					&alen);

			if (ssock < 0)
				errexit ("accept");

			FD_SET (ssock, &afds);
		}
		/* data available on serial port */
		if (input >= 0 && FD_ISSET (input, &rfds))
		{
			if (device_type == DEVICE_EARTHMATEb)
				handle_EMinput (input, &afds, &nmea_fds);
			else
				handle_input (input, &afds, &nmea_fds);
		}
		/* misc gps stuff ... */
		if (gNMEAdata.status > 0)
			fixcnt++;
		if (fixcnt > 10)
			if (!sentdgps)
			{
				sentdgps++;
				if (need_dgps)
					send_dgps ();
			}
		need_gps = keep_gps;
		if (reopen)
		{
			FD_CLR (input, &afds);
			input = -1;
			gNMEAdata.fdin = input;
			gNMEAdata.fdout = input;
			serial_close ();
			if (device_type == DEVICE_EARTHMATEb)
				device_type = DEVICE_EARTHMATE;
			syslog (LOG_NOTICE, "Closed gps");
			gNMEAdata.mode = 1;
			gNMEAdata.status = 0;
			need_gps++;
		}
		for (fd = 0; fd < nfds; fd++)
		{
			if (fd != msock && fd != input && fd != dsock &&
			    FD_ISSET (fd, &rfds))
			{
				if (input == -1)
				{
					if (infile)
					{
						input = open (infile_name,
							      O_RDONLY);
						if (input < 0)
							errexit ("File open: ");
					}
					else
					{
						if ((input =
						     serial_open ()) < 0)
							errexit ("serial open: ");
					}
					syslog (LOG_NOTICE, "Opened gps");
					FD_SET (input, &afds);
					gNMEAdata.fdin = input;
					gNMEAdata.fdout = input;
				}
				if (handle_request (fd, &nmea_fds) == 0)
				{
					(void) close (fd);
					FD_CLR (fd, &afds);
					FD_CLR (fd, &nmea_fds);
				}
			}
			if (fd != msock && fd != input
			    && FD_ISSET (fd, &afds))
			{
				need_gps++;
			}
		}

		if (!need_gps && input != -1)
		{
			FD_CLR (input, &afds);
			input = -1;
			gNMEAdata.fdin = input;
			gNMEAdata.fdout = input;
			serial_close ();
			if (device_type == DEVICE_EARTHMATEb)
				device_type = DEVICE_EARTHMATE;
			syslog (LOG_NOTICE, "Closed gps");
			gNMEAdata.mode = 1;
			gNMEAdata.status = 0;
		}
	}
}

static int
handle_request (int fd, fd_set * fds)
{
	char buf[BUFSIZE];
	char reply[BUFSIZE];
	char *p;
	int cc;

	cc = read (fd, buf, sizeof (buf) - 1);
	if (cc < 0)
		return 0;

	buf[cc] = '\0';

	sprintf (reply, "GPSD");
	p = buf;
	while (*p)
	{
		switch (*p)
		{
		case 'P':
		case 'p':
			sprintf (reply + strlen (reply),
				 ",P=%f %f", gNMEAdata.latitude,
				 gNMEAdata.longitude);
			break;
		case 'D':
		case 'd':
			sprintf (reply + strlen (reply), ",D=%s",
				 gNMEAdata.utc);
			break;
		case 'A':
		case 'a':
			sprintf (reply + strlen (reply), ",A=%f",
				 gNMEAdata.altitude);
			break;
		case 'V':
		case 'v':
			sprintf (reply + strlen (reply), ",V=%f",
				 gNMEAdata.speed);
			break;
		case 'R':
		case 'r':
			if (FD_ISSET (fd, fds))
			{
				FD_CLR (fd, fds);
				sprintf (reply + strlen (reply), ",R=0");
			}
			else
			{
				FD_SET (fd, fds);
				sprintf (reply + strlen (reply), ",R=1");
			}
			break;
		case 'S':
		case 's':
			sprintf (reply + strlen (reply), ",S=%d",
				 gNMEAdata.status);
			break;
		case 'M':
		case 'm':
			sprintf (reply + strlen (reply), ",M=%d",
				 gNMEAdata.mode);
			break;
		case '\r':
		case '\n':
			*p = '\0';	/* ignore the rest */
			break;

		}
		p++;
	}
	strcat (reply, "\r\n");

	if (cc && write (fd, reply, strlen (reply) + 1) < 0)
		return 0;

	return cc;
}

/* Returns time difference in seconds (accurate to 10^-3) */
static double
time_diff (struct timeval start, struct timeval end)
{
	double retval;

	retval = (double) (end.tv_sec - start.tv_sec);

	retval += (double) ((end.tv_usec - start.tv_usec) / 1000) / 1000.0;

	return (retval);
}

static int
handle_input (int input, fd_set * afds, fd_set * nmea_fds)
{
	static unsigned char buf[BUFSIZE];	/* that is more then a sentence */
	static int offset = 0;
	static int bincount;
	int fd;

	/* Reading from file. */
	static struct timeval Msg_Time = { 0, 0 };	/* Occurence of last line */
	struct timeval time;

	while (offset < BUFSIZE)
	{

		/* This delays the reading of another line from file */
		if (infile)
		{
			gettimeofday (&time, NULL);
			if (time_diff (Msg_Time, time) < InfileLineTime)
			{
				usleep (1);
				return 1;
			}
		}

		if (read (input, buf + offset, 1) != 1)
		{
			/* In case of file input, we must sleep for a while,
			 * otherwise gpsd runs the CPU at full tilt when the file is finished. */
			if (infile)
			{
				usleep (1000);
			}
			return 1;
		}

		if (buf[offset] == '\n' || buf[offset] == '\r')
		{
			buf[offset] = '\0';
			if (strlen (buf))
			{
				handle_message (buf);

				/* A new line arrived, remember time */
				if (infile)
				{
					gettimeofday (&time, NULL);
					Msg_Time = time;
				}

				strcat (buf, "\r\n");
				for (fd = 0; fd < nfds; fd++)
				{
					if (FD_ISSET (fd, nmea_fds))
					{
						if (write
						    (fd, buf,
						     strlen (buf)) < 0)
						{
							syslog (LOG_NOTICE,
								"Raw write: %s",
								strerror
								(errno));
							FD_CLR (fd, afds);
							FD_CLR (fd, nmea_fds);
						}
					}
				}
			}
			offset = 0;
			return 1;
		}
		/*
		 * The following tries to recognise if the EarthMate is
		 * in binary mode. If so, it will switch to EarthMate mode.
		 * 
		 * Tf.20000105: this block does not serve any purpose.
		 * Please look it over, and delete it if you agree.
		 */

		if (device_type == DEVICE_EARTHMATE)
		{
			if (offset)
			{
				if (buf[offset - 1] == (unsigned char) 0xff)
				{
					if (buf[offset] ==
					    (unsigned char) 0x81)
					{
						if (bincount++ == 5)
						{
							syslog (LOG_NOTICE,
								"Found an EarthMate (syn).");
							device_type =
								DEVICE_EARTHMATEb;
							return 0;
						}
					}
				}
			}
		}
		offset++;
		buf[offset] = '\0';
	}
	offset = 0;		/* discard input ! */
	return 1;
}

int
errexit (char *s)
{
	syslog (LOG_ERR, "%s: %s\nExit now...\n", s, strerror (errno));
	serial_close ();
	close (dsock);
	exit (2);
}
