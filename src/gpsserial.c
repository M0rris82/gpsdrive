/*

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


gpsserial.c 

routines to read GPS data from serial device

*/



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <gpsdrive.h>

    /*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif



    /* variables */

pthread_t threadid = 0;
extern char serialdev[80];
static int fd, didinit = 0;
int newdata = FALSE;
char serialdata[4096];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static struct termios oldtio, newtio;
extern int timeoutcount, debug, haveserial, serialspeed, nmeaverbose;
extern int egnoson, egnosoff;

int
readinput_init (void)
{
	/* 
	 * Open modem device for reading and writing and not as controlling tty
	 * because we don't want to get killed if linenoise sends CTRL-C.
	 */

	fd = open (serialdev, O_RDWR | O_NOCTTY);

	if (fd < 0)
	{
		perror (serialdev);
		return FALSE;
	}
	didinit = TRUE;
	tcgetattr (fd, &oldtio);	/* save current serial port settings */
	bzero (&newtio, sizeof (newtio));	/* clear struct for new port settings */

	/* 
	 * BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	 * CRTSCTS : output hardware flow control (only used if the cable has
	 * all necessary lines. See sect. 7 of Serial-HOWTO)
	 * CS8     : 8n1 (8bit,no parity,1 stopbit)
	 * CLOCAL  : local connection, no modem contol
	 * CREAD   : enable receiving characters
	 */
	newtio.c_cflag = (11 + serialspeed) | CS8 | CLOCAL | CREAD;
	newtio.c_cflag &= ~(PARENB | CRTSCTS);
	/*
	 * IGNPAR  : ignore bytes with parity errors
	 * ICRNL   : map CR to NL (otherwise a CR input on the other computer
	 * will not terminate input)
	 * otherwise make device raw (no other input processing)
	 */
	newtio.c_iflag = IGNPAR | ICRNL;

	/*
	 * Raw output.
	 */
	/*   newtio.c_oflag = 0; */
	newtio.c_iflag = newtio.c_oflag = newtio.c_lflag = (tcflag_t) 0;
	newtio.c_oflag = (ONLCR);
	/*
	 * ICANON  : enable canonical input
	 * disable all echo functionality, and don't send signals to calling program
	 */
	/*    newtio.c_lflag = ICANON;  */

	/* 
	 * initialize all control characters 
	 * default values can be found in /usr/include/termios.h, and are given
	 * in the comments, but we don't need them here
	 */
	newtio.c_cc[VINTR] = 0;	/* Ctrl-c */
	newtio.c_cc[VQUIT] = 0;	/* Ctrl-\ */
	newtio.c_cc[VERASE] = 0;	/* del */
	newtio.c_cc[VKILL] = 0;	/* @ */
	newtio.c_cc[VEOF] = 4;	/* Ctrl-d */
	newtio.c_cc[VTIME] = 0;	/* inter-character timer unused */
	newtio.c_cc[VMIN] = 1;	/* blocking read until 1 character arrives */
	newtio.c_cc[VSTART] = 0;	/* Ctrl-q */
	newtio.c_cc[VSTOP] = 0;	/* Ctrl-s */
	newtio.c_cc[VSUSP] = 0;	/* Ctrl-z */
	newtio.c_cc[VEOL] = 0;	/* '\0' */
	newtio.c_cc[VREPRINT] = 0;	/* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0;	/* Ctrl-u */
	newtio.c_cc[VWERASE] = 0;	/* Ctrl-w */
	newtio.c_cc[VLNEXT] = 0;	/* Ctrl-v */
	newtio.c_cc[VEOL2] = 0;	/* '\0' */

	/* 
	 * now clean the modem line and activate the settings for the port
	 */
	tcflush (fd, TCIFLUSH);
	tcsetattr (fd, TCSANOW, &newtio);


	/*
	 * terminal settings done, now handle input
	 * In this example, inputting a 'z' at the beginning of a line will 
	 * exit the program.
	 */


	/* restore the old port settings */
	/*  tcsetattr (fd, TCSANOW, &oldtio); */
	return TRUE;
}

void
gpsserialquit ()
{
	haveserial = FALSE;
	if (threadid != 0)
	{
		pthread_kill (threadid, SIGCHLD);

		fprintf (stderr, _("waiting for thread to stop\n"));
		fflush (stderr);
		pthread_join (threadid, NULL);
	}
	/* restore the old port settings */
	if (didinit)
		tcsetattr (fd, TCSANOW, &oldtio);
	if (fd >= 0)
		close (fd);

}

int
gpsserialinit (void)
{
	int e;
	sigset_t newmask;
	pthread_attr_t attr;
	char buf[40];

	e = readinput_init ();
	if (!e)
	{
		fprintf (stderr, _("\nerror opening %s(%d)\n"), serialdev, e);
		return FALSE;
	}
	fprintf (stderr, _("successfull opened %s\n"), serialdev);

	if (egnoson)
	{
		g_snprintf (buf, sizeof (buf), "%s", EGNOSON);
		write (fd, buf, sizeof (buf));
		fprintf (stderr, _("switching WAAS/EGNOS on\n"));
	}
	if (egnosoff)
	{
		g_snprintf (buf, sizeof (buf), "%s", EGNOSOFF);
		write (fd, buf, sizeof (buf));
		fprintf (stderr, _("switching WAAS/EGNOS off\n"));
	}

	memset (serialdata, 0, 4096);
	sigemptyset (&newmask);
	/*   sigaddset(&newmask, TEST_SIGNAL); */
	/*   sigaddset (&newmask, SIGCHLD); */
	sigaddset (&newmask, SIGHUP);
	sigaddset (&newmask, SIGPIPE);
	/*   sigaddset (&newmask, SIGINT); */
	/*   sigaddset (&newmask, SIGTERM); */
	pthread_sigmask (SIG_BLOCK, &newmask, NULL);

	pthread_attr_init (&attr);
	pthread_attr_setscope (&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_mutex_init (&mutex, NULL);

	pthread_create (&threadid, NULL, getserialdata, NULL);

	return TRUE;
}

void *
getserialdata (void *data)
{
	static int count = 0;
	unsigned char buf[4096];
	int e, ill, i, cc;
	char c;
	static fd_set readfs;
	struct timeval timeout;
	static int tidshown = FALSE;

	memset (buf, 0, 4095);
	do
	{
		if (!tidshown)
			if (threadid != 0)
			{
				fprintf (stderr, "new thread started: %d\n",
					 (int) threadid);
				tidshown = TRUE;
			}

		FD_ZERO (&readfs);
		FD_SET (fd, &readfs);
		/* block until input becomes available */
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		select (FD_SETSIZE, &readfs, NULL, NULL, &timeout);
		if (FD_ISSET (fd, &readfs))
		{

			do
			{
				e = read (fd, &c, 1);
			}
			while (c != '$');

			cc = 0;
			buf[cc++] = c;
			do
			{
				e = read (fd, &c, 1);
				buf[cc++] = c;
			}
			while (c != 10);

			buf[cc - 2] = 0;
			ill = 0;
			/* test for illegal characters */
			for (i = 0; i < e; i++)
			{
				if ((buf[i] > 127) || (buf[i] < 10))
				{
					if (debug)
						fprintf (stderr,
							 "illegal char(%i): %c\n",
							 i, buf[i]);
					ill = 1;
				}
				if ((buf[i] == 13) || (buf[i] == 10))
					buf[i] = 0;
			}
			if (ill == 1)
				fprintf (stderr, "illegal data(%d)\n", e);
			else
			{
				if (debug)
					fprintf (stderr, "data(%d):%s===\n",
						 e, buf);
				count = 0;
				pthread_mutex_lock (&mutex);
				g_strlcpy (serialdata, (gchar *)buf,
					   sizeof (serialdata));
				if (nmeaverbose)
					fprintf (stderr, "%s\n", buf);
				/*        timeoutcount = 0; */
				newdata = TRUE;
				pthread_mutex_unlock (&mutex);

			}
		}
		else
		{
			timeoutcount++;
			if (debug)
				fprintf (stderr,
					 "getserialdata: %d failed getting data\n",
					 count++);
		}

	}
	while (haveserial);
	threadid = 0;
	fprintf (stderr, "leaving thread\n");

	return 0;
}
