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

    /*
     * $Log$
     * Revision 1.6  2006/03/10 08:37:09  tweety
     * - Replace Street/Track find algorithmus in Query Funktion
     *   against real Distance Algorithm (distance_line_point).
     * - Query only reports Track/poi/Streets if currently displaying
     *   on map is selected for these
     * - replace old top/map Selection by a MapServer based selection
     * - Draw White map if no Mapserver is selected
     * - Remove some useless Street Data from Examples
     * - Take the real colors defined in Database to draw Streets
     * - Add a frame to the Streets to make them look nicer
     * - Added Highlight Option for Tracks/Streets to see which streets are
     *   displayed for a Query output
     * - displaymap_top und displaymap_map removed and replaced by a
     *   Mapserver centric approach.
     * - Treaked a little bit with Font Sizes
     * - Added a very simple clipping to the lat of the draw_grid
     *   Either the draw_drid or the projection routines still have a slight
     *   problem if acting on negative values
     * - draw_grid with XOR: This way you can see it much better.
     * - move the default map dir to ~/.gpsdrive/maps
     * - new enum map_projections to be able to easily add more projections
     *   later
     * - remove history from gpsmisc.c
     * - try to reduce compiler warnings
     * - search maps also in ./data/maps/ for debugging purpose
     * - cleanup and expand unit_test.c a little bit
     * - add some more rules to the Makefiles so more files get into the
     *   tar.gz
     * - DB_Examples.pm test also for ../data and data directory to
     *   read files from
     * - geoinfo.pl: limit visibility of Simple POI data to a zoom level of 1-20000
     * - geoinfo.pl NGA.pm: Output Bounding Box for read Data
     * - gpsfetchmap.pl:
     *   - adapt zoom levels for landsat maps
     *   - correct eniro File Download. Not working yet, but gets closer
     *   - add/correct some of the Help Text
     * - Update makefiles with a more recent automake Version
     * - update po files
     *
     * Revision 1.5  2006/01/03 14:24:10  tweety
     * eliminate compiler Warnings
     * try to change all occurences of longi -->lon, lati-->lat, ...i
     * use  drawicon(posxdest,posydest,"w-lan.open") instead of using a seperate variable
     * rename drawgrid --> do_draw_grid
     * give the display frames usefull names frame_lat, ...
     * change handling of WP-types to lowercase
     * change order for directories reading icons
     * always read inconfile
     *
     * Revision 1.4  2005/04/20 23:33:49  tweety
     * reformatted source code with anjuta
     * So now we have new indentations
     *
     * Revision 1.3  2005/04/13 19:58:31  tweety
     * renew indentation to 4 spaces + tabstop=8
     * 
     * Revision 1.2  2005/04/10 21:50:50  tweety
     * reformatting c-sources
     * 
     * Revision 1.1.1.1  2004/12/23 16:03:24  commiter
     * Initial import, straight from 2.10pre2 tar.gz archive
     * 
     * Revision 1.21  2004/02/12 17:42:53  ganter
     * added -W switch for enable/disable WAAS/EGNOS (for SiRF II only?)
     * 
     * Revision 1.20  2004/02/09 17:06:40  ganter
     * fixed timeout behavior for direct serial connection
     * 
     * Revision 1.19  2004/02/08 17:39:46  ganter
     * v2.08pre12
     * 
     * Revision 1.18  2004/02/08 16:35:10  ganter
     * replacing all sprintf with g_snprintf to avoid buffer overflows
     * 
     * Revision 1.17  2004/02/07 17:46:10  ganter
     * ...
     * 
     * Revision 1.16  2004/02/07 15:53:38  ganter
     * replacing strcpy with g_strlcpy to avoid bufferoverflows
     * 
     * Revision 1.15  2004/02/07 00:02:16  ganter
     * added "store timezone" button in settings menu
     * 
     * Revision 1.14  2004/02/06 16:41:40  ganter
     * added -E parameter, which prints out the NMEA messages received
     * 
     * Revision 1.13  2004/02/06 12:09:31  ganter
     * added select again, so we can check if data is coming
     * 
     * Revision 1.12  2004/02/05 22:15:53  ganter
     * code in gpsserial.c didn't work with USB receivers, because the send characters too fast
     * this code now should be ok
     * 
     * Revision 1.11  2004/02/05 19:47:31  ganter
     * replacing strcpy with g_strlcpy to avoid bufferoverflows
     * USB receiver does not send sentences in direct serial mode,
     * so I first send a "\n" to it
     * 
     * Revision 1.10  2004/02/02 18:20:00  ganter
     * ..
     * 
     * Revision 1.9  2004/02/02 03:38:32  ganter
     * code cleanup
     * 
     * Revision 1.8  2004/01/31 13:43:57  ganter
     * nasamaps are working better, but still bugs
     * 
     * Revision 1.7  2004/01/30 17:54:57  ganter
     * i have to add gdk_threads_enter()/gdk_threads_leave() into all timeouts :-(
     * 
     * Revision 1.6  2004/01/28 09:32:57  ganter
     * tested for memory leaks with valgrind, looks good :-)
     * 
     * Revision 1.5  2004/01/28 05:36:29  ganter
     * added #include <sys/select.h>
     * to gpsserial.c
     * 
     * Revision 1.4  2004/01/27 22:51:59  ganter
     * added "direct serial connection" button in settings menu
     * 
     * Revision 1.3  2004/01/27 09:44:29  ganter
     * removed double defines
     * 
     * Revision 1.2  2004/01/27 06:59:14  ganter
     * The baudrate is now selectable in settings menu
     * GpsDrive now connects to the GPS receiver in following order:
     * Try to connect to gpsd
     * Try to find Garble-mode Garmin
     * Try to read data directly from serial port
     * 
     * If this all fails, it falls back into simulation mode
     * 
     * Revision 1.1  2004/01/27 05:25:59  ganter
     * added gpsserial.c
     * 
     * gpsdrive now detects a running gps receiver
     * You don't need to start gpsd now, serial connection is handled by GpsDrive directly
     * 
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
