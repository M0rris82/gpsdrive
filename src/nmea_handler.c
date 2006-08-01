/***********************************************************************
 *
 * Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>
 *
 * Website: www.gpsdrive.de
 * 
 * Disclaimer: Please do not use for navigation. 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **********************************************************************
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gpsdrive.h>
#include <time.h>
#include <config.h>
#include <math.h>
#include <termios.h>
#include "gps_handler.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint simmode, zoom, iszoomed;
extern gint maploaded;
extern gint importactive;
extern gint zoom;
extern gint showroute, routeitems;
extern gint nightmode, isnight, disableisnight;

extern gint mydebug;
gint nmea_handler_debug = 0;

extern gint posmode;
extern gchar utctime[20], loctime[20];
extern gint forcehavepos;
extern gint havepos, haveposcount;
extern gint blink, gblink, xoff, yoff, crosstoogle;
extern gint zone;
extern gdouble current_lon, current_lat, old_lon, old_lat, groundspeed;
extern gdouble milesconv;
extern gint oldsatfix, oldsatsanz, havealtitude;
extern gdouble altitude, precision, gsaprecision;
extern gchar localedecimal;
extern gdouble gbreit, glang, milesconv, olddist;
extern gchar mapfilename[1024];
extern gdouble pixelfact, posx, posy;
extern gdouble angle_to_destination, direction, bearing;
extern gint satlist[MAXSATS][4], satlistdisp[MAXSATS][4], satbit;
extern gint newsatslevel, testgarmin;
extern gint satfix, usedgps;
extern gint numsats, satsavail;
extern gchar *buffer, *big;
extern fd_set readmask;
extern struct timeval timeout;
extern gdouble earthr;
extern GTimer *timer, *disttimer;
extern gchar serialdev[80];
extern int newdata;
extern pthread_mutex_t mutex;
extern GtkWidget *startgpsbt;
extern int messagenumber,  didrootcheck, haveserial;
extern gint statusid, messagestatusbarid, timeoutcount;
extern gint simpos_timeout;
extern int gotneverserial, timerto, serialspeed;
extern GtkWidget *drawing_sats;
extern GtkTooltips *temptooltips;
extern GtkWidget *satslabel1, *satslabel2, *satslabel3;
extern GdkPixbuf *satsimage;
extern gchar dgpsserver[80], dgpsport[10];
extern gchar activewpfile[200], gpsdservername[200], setpositionname[80];
extern GtkWidget *mainwindow, *status, *messagestatusbar;
extern GtkWidget *pixmapwidget, *gotowindow;
extern gint statuslock, gpson;
extern gint earthmate;
extern int disableserial, disableserialcl;
static gchar gradsym[] = "\xc2\xb0";

/* variables */
extern gint ignorechecksum;
//, mapistopo;
extern gdouble lat2RadiusArray[201];
extern gdouble zero_lon, zero_lat, target_lon, target_lat, dist;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu,
  int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;
extern gdouble pixelfact, posx, posy, angle_to_destination, direction,
  bearing;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle;
extern FILE *nmeaout;
// ---------------------- NMEA
gint haveRMCsentence = FALSE;
gchar nmeamodeandport[50];
gdouble NMEAsecs = 0.0;
gint NMEAoldsecs = 0;
FILE *nmeaout = NULL;
/*  if we get data from gpsd in NMEA format haveNMEA is TRUE */
/*  haveGARMIN is TRUE if we get data from program garble in GARMIN we get only long and lat */
gint haveNMEA, haveGARMIN;
extern gint sock;


/* ******************************************************************
 * check NMEA checksum
 * ARGS:    NMEA String
 * RETURNS: TRUE if Checksumm is ok
 */
gint
checksum (gchar * text)
{
  gchar t[120], t2[10];
  gint i = 1, checksum = 0, j, orig;

  if (ignorechecksum)
    return TRUE;

  strncpy (t, text, 100);
  t[100] = 0;
  j = strlen (t) - 3;
  while (('\0' != t[i]) && (i < j))
    checksum = checksum ^ t[i++];
  g_strlcpy (t2, (t + j + 1), sizeof (t2));
  sscanf (t2, "%X", &orig);
  if (mydebug + nmea_handler_debug > 50)
    {
      g_print ("nmea_handler: gpsd: %s\n", t);
      g_print ("nmea_handler: gpsd: origchecksum: %X, my:%X\n", orig, checksum);
    }

  if (orig == checksum)
    {
      g_strlcpy (text, t, 1000);
      return TRUE;
    }
  else
    {
      g_print
	("\n"
	 "*** nmea_handler: NMEA checksum error!\n"
	 "*** nmea_handler: NMEA: %s\n"
	 "*** nmea_handler: Checksum is: %X, should be: %X\n", t,
	 orig, checksum);
      return FALSE;
    }
}


/* *****************************************************************************
 * open serial port or pty master or file for NMEA output 
 */
FILE *
opennmea (const char *name)
{
	struct termios tios;

	if (mydebug + nmea_handler_debug >50) 
	    printf ("nmea_handler: opennmea()\n");

	FILE *const out = fopen (name, "w");
	if (out == NULL)
	{
		perror (_("can't open NMEA output file"));
		exit (1);
	}

	if (tcgetattr (fileno (out), &tios))
		return out;	/* not a terminal, oh well */

	tios.c_iflag = 0;
	tios.c_oflag = 0;
	tios.c_cflag = CS8 | CLOCAL;
	tios.c_lflag = 0;
	tios.c_cc[VMIN] = 1;
	tios.c_cc[VTIME] = 0;
	cfsetospeed (&tios, B4800);
	tcsetattr (fileno (out), TCSAFLUSH, &tios);
	return out;
}

/* *****************************************************************************
 */
void
write_nmea_line (const char *line)
{
	int checksum = 0;
	fprintf (nmeaout, "$%s*", line);
	while ('\0' != *line)
		checksum = checksum ^ *line++;
	fprintf (nmeaout, "%02X\r\n", checksum);
	fflush (nmeaout);
}

/* *****************************************************************************
 */
void
gen_nmea_coord (char *out)
{
	gdouble lat = fabs (current_lat), lon = fabs (current_lon);
	g_snprintf (out, sizeof (out), ",%02d%07.5f,%c,%03d%07.5f,%c",
		    (int) floor (lat), 60 * (lat - floor (lat)),
		    (current_lat < 0 ? 'S' : 'N'),
		    (int) floor (lon), 60 * (lon - floor (lon)),
		    (current_lon < 0 ? 'W' : 'E'));
}

/* *****************************************************************************
 */
gint
write_nmea_cb (GtkWidget * widget, guint * datum)
{
	char buffer[180];
	time_t now = time (NULL);
	struct tm *st = gmtime (&now);

	strftime (buffer, sizeof (buffer), "GPGGA,%H%M%S.000", st);
	gen_nmea_coord (buffer + strlen (buffer));
	g_strlcpy (buffer + strlen (buffer), ",1,00,0.0,,M,,,,0000",
		   sizeof (buffer) - strlen (buffer));
	write_nmea_line (buffer);

	g_strlcpy (buffer, "GPGLL", sizeof (buffer));
	gen_nmea_coord (buffer + strlen (buffer));
	strftime (buffer + strlen (buffer), 80, ",%H%M%S.000,A", st);
	write_nmea_line (buffer);

	strftime (buffer, sizeof (buffer), "GPRMC,%H%M%S.000,A", st);
	gen_nmea_coord (buffer + strlen (buffer));
	g_snprintf (buffer + strlen (buffer), sizeof (buffer), ",%.2f,%.2f",
		    groundspeed / milesconv / 1.852,
		    direction * 180.0 / M_PI);
	strftime (buffer + strlen (buffer), 80, ",%d%m%y,,", st);
	write_nmea_line (buffer);

	g_snprintf (buffer, sizeof (buffer), "GPVTG,%.2f,T,,M,%.2f,N,%.2f,K",
		    direction * 180.0 / M_PI,
		    groundspeed / milesconv / 1.852, groundspeed / milesconv);
	write_nmea_line (buffer);

	return TRUE;
}


/* ******************************************************************
 * show  HDOP in meters 
 */
void
convertGSA (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, j = 0, start = 0;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if ( mydebug + nmea_handler_debug > 80 )
    {
      g_print ("nmea_handler: gpsd: GSA Fields: ");
      for (i = 0; i < j; i++)
	  {
	      g_print ("%d:%s$", i, field[i]);
	  }
      g_print ("\n");
    }
  if (havepos)
    {

      gsaprecision = g_strtod (field[15], 0);
      if ( mydebug + nmea_handler_debug > 80 )
	g_print ("nmea_handler: gpsd: GSA PDOP: %.1f\n", gsaprecision);
    }
}


/* *****************************************************************************
 */
void
convertRMC (char *f)
{
  gchar field[50][100], b[100];
  gint start = 0;
  gint longdegree, latdegree;
  gchar langri, breitri;
  size_t i, j = 0;
  memset (b, 0, 100);

  /*  if simulation mode we display status and return */
  if (simmode && maploaded && !posmode)
    {
      display_status (_("Simulation mode"));
      return;
    }

  /*  get fields delimited with ',' */
  for (i = 0; i < strlen (f); i++)
    {
      if (f[i] == ',')
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if ((j != 11) && (j != 12))
    {
      g_print ("gpsd: GPRMC: wrong number of fields (%d)\n", (int) j);
      return;
    }
  if (!haveRMCsentence)
    {
      if ( mydebug + nmea_handler_debug > 80 )
	g_print ("nmea_handler: gpsd: got RMC data, using it\n");
      haveRMCsentence = TRUE;
    }

  if ( mydebug + nmea_handler_debug > 80 )
    {
      g_print ("nmea_handler: gpsd: RMC Fields: \n");
      for (i = 0; i < j; i++)
	g_print ("nmea_handler: gpsd: RMC Field %d:%s\n", (int) i, field[i]);
      g_print ("\n");
    }
  g_snprintf (b, sizeof (b), "%c%c:%c%c.%c%c ", field[1][0],
	      field[1][1], field[1][2], field[1][3], field[1][4],
	      field[1][5]);
  g_strlcpy (utctime, b, sizeof (utctime));
  if ( mydebug + nmea_handler_debug > 80 )
    g_print ("nmea_handler: gpsd: utctime: %s\n", utctime);
  if ((field[2][0] != 'A') && !forcehavepos)
    {
      havepos = FALSE;
      haveposcount = 0;
      return;
    }
  else
    {
      havepos = TRUE;
      haveposcount++;
      if (haveposcount == 3)
	{
	  rebuildtracklist ();
	}

    }
  /*  Latitude North / South */
  /* if field[3] is shorter than 9 characters, add zeroes in the beginning */
  if (strlen (field[3]) < 8)
    {
      if ( mydebug + nmea_handler_debug > 0 )
	{
	  g_print ("nmea_handler: Latitude field %s is shorter than 9 characters. (%d)\n",
		   field[3], strlen (field[3]));
	}
      for (i = 0; i < 9; i++)
	{
	  b[i] = '0';
	}
      b[9] = 0;
      g_strlcpy (b + (9 - strlen (field[3])), field[3], sizeof (b));
      g_strlcpy (field[3], b, sizeof (field[3]));
    }

  b[0] = field[3][0];
  b[1] = field[3][1];
  b[2] = 0;
  latdegree = atoi (b);
  if ( mydebug + nmea_handler_debug > 80 )
    g_print ("nmea_handler: gpsd: lat part1: %s\n", b);


  b[0] = field[3][2];
  b[1] = field[3][3];
  b[2] = '.';
  b[3] = field[3][5];
  b[4] = field[3][6];
  b[5] = field[3][7];
  b[6] = field[3][8];
  b[7] = 0;
  if ( mydebug + nmea_handler_debug > 80 )
    g_print ("nmea_handler: gpsd: lat part2: %s\n", b);
  if (!posmode)
    {
      gdouble cl;
      if ( mydebug + nmea_handler_debug > 80 )
	  g_print ("nmea_handler: gpsd: lat atof(%s):%f \n", b,atof(b));
      cl = latdegree + atof (b) / 60.0;
      if ( mydebug + nmea_handler_debug > 80 )
	  g_print ("nmea_handler: gpsd: cl: %f\n", cl);

      if (field[4][0] == 'S')
	cl = cl * -1;
      if ((cl >= -90.0) && (cl <= 90.0))
	current_lat = cl;
      breitri = field[4][0];
      g_snprintf (b, sizeof (b), " %8.5f%s%c", current_lat, gradsym, breitri);
      if ( mydebug + nmea_handler_debug > 60 )
	  {
	      g_print ("nmea_handler: RMC lat: %8.5f\n", current_lat);
	  }
    }

  /*  Longitude East / West */

  /* if field[5] is shorter than 10 characters, add zeroes in the beginning */
  if (strlen (field[5]) < 9)
    {
      if ( mydebug + nmea_handler_debug > 0 )
	{
	  g_print ("nmea_handler: Longitude field %s is shorter than 10 characters. (%d)\n",
		   field[5], strlen (field[5]));
	}
      for (i = 0; i < 10; i++)
	{
	  b[i] = '0';
	}
      b[10] = 0;
      g_strlcpy (b + (10 - strlen (field[5])), field[5], sizeof (b));
      g_strlcpy (field[5], b, sizeof (field[5]));
    }

  b[0] = field[5][0];
  b[1] = field[5][1];
  b[2] = field[5][2];
  b[3] = 0;
  longdegree = atoi (b);

  b[0] = field[5][3];
  b[1] = field[5][4];
  b[2] = '.';
  b[3] = field[5][6];
  b[4] = field[5][7];
  b[5] = field[5][8];
  b[6] = field[5][9];
  b[7] = 0;
  if (!posmode)
    {
      gdouble cl;
      cl = longdegree + atof (b) / 60.0;
      if ( mydebug + nmea_handler_debug > 60 )
	  g_print ("nmea_handler: RMC dir: %c\n", field[6][0]);
      if (field[6][0] == 'W')
	cl = cl * -1;
      if ((cl >= -180.0) && (cl <= 180.0))
	current_lon = cl;
      langri = field[6][0];
      g_snprintf (b, sizeof (b), " %8.5f%s%c", current_lon, gradsym, langri);
      if ( mydebug + nmea_handler_debug > 60 )
	  {
	      g_print ("nmea_handler: RMC lon: %8.5f\n", current_lon);
	  }
    }

  /*  speed */
  b[0] = field[7][0];
  b[1] = field[7][1];
  b[2] = field[7][2];
  b[3] = '.';
  b[4] = field[7][4];
  b[5] = 0;
  groundspeed = atof (b) * 1.852 * milesconv;

  // What hapens here with b or mapfilename?
  g_snprintf (b, sizeof (b), " %s: %s", _("Map"), mapfilename);

  /*    g_print("Field %s\n",field[8]); */
  b[0] = field[8][0];
  b[1] = field[8][1];
  b[2] = field[8][2];
  b[3] = '.';
  b[4] = field[8][4];
  b[5] = 0;
  /*  direction is the course we are driving */
  direction = atof (b);
  direction = direction * M_PI / 180;

  {
    int h, m, s;
    h = m = s = 0;
    if (strcmp (utctime, "n/a") != 0)
      {
	sscanf (utctime, "%d:%d.%d", &h, &m, &s);
	h += zone;
	if (h > 23)
	  h -= 24;
	if (h < 0)
	  h += 24;
	g_snprintf (loctime, sizeof (loctime), "%d:%02d.%02d", h, m, s);
      }
    else
      g_strlcpy (loctime, "n/a", sizeof (loctime));
  }

}

/* *****************************************************************************
 * show satellites signal level 
 */
gint
convertGSV (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, i2, j = 0, start = 0, n, db, anz, az, el;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if ( mydebug + nmea_handler_debug > 80 )
    {
      g_print ("nmea_handler: gpsd: GSV Fields:\n");
      g_print ("nmea_handler: gpsd: ");
      for (i = 0; i < j; i++)
	  {
	      g_print ("%d:%s$", i, field[i]);
	  }
      g_print ("\n");
    }
  if (j > 40)
    {
      g_print ("nmea_handler: gpsd: GPGSV: wrong number of fields (%d)\n", j);
      return FALSE;
    }

  if (field[2][0] == '1')
    satbit = satbit | 1;
  if (field[2][0] == '2')
    satbit = satbit | 2;
  if (field[2][0] == '3')
    satbit = satbit | 4;

  anz = atoi (field[3]);
  b[0] = field[1][0];
  b[1] = 0;
  i2 = atof (b);
  if (mydebug + nmea_handler_debug && ( i2 != satbit))
      g_print ("nmea_handler: gpsd: convertGSV(): bits should be: %d is: %d\n", i2, satbit);
  g_snprintf (b, sizeof (b), "Satellites: %d\n", anz);
  if (anz != oldsatsanz)
    newsatslevel = TRUE;
  oldsatsanz = anz;

  for (i = 4; i < j; i += 4)
    {
      n = atoi (field[i]);
      if (n > MAXSATS)
	{
	  fprintf (stderr,
		   "gpsd: illegal satellite number: %d, ignoring\n", n);
	  continue;
	}
      db = atoi (field[i + 3]);
      el = atoi (field[i + 1]);
      az = atoi (field[i + 2]);
      if ( mydebug + nmea_handler_debug > 80 )
	fprintf (stderr,
		 "nmea_handler: gpsd: satnumber: %2d elev: %3d azimut: %3d signal %3ddb\n",
		 n, el, az, db);

      satlist[n][0] = n;
      satlist[n][1] = db;
      satlist[n][2] = el;
      satlist[n][3] = az;
    }

  if (((pow (2, i2)) - 1) == satbit)
    {
      satsavail = 0;
      for (i = 0; i < MAXSATS; i++)
	if (satlist[i][0] != 0)
	  {
	    g_snprintf (b, sizeof (b), "% 2d: % 2ddb   ",
			satlist[i][0], satlist[i][1]);
	    satsavail++;
	  }
      satbit = 0;

      memcpy (satlistdisp, satlist, sizeof (satlist));
      memset (satlist, 0, sizeof (satlist));
      newsatslevel = TRUE;
      return TRUE;
    }
  return FALSE;
}


/* ******************************************************************
 * show altitude and satfix 
 */
void
convertGGA (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, j = 0, start = 0;
  gint longdegree, latdegree;
  gchar langri, breitri;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if ( mydebug + nmea_handler_debug > 80 )
    {
      g_print ("nmea_handler: gpsd: GGA Fields: ");
      for (i = 0; i < j; i++)
	  {
	      g_print ("%d:%s$", i, field[i]);
	  }
      g_print ("\n");
    }

  if ((j != 15) && (j != 16))
    {
      g_print ("GPGGA: wrong number of fields (%d)\n", j);
      return;
    }

  /*   the receiver sends no GPRMC, so we get the data from here */
  if (!haveRMCsentence)
    {
      gint mysecs;

      if ( mydebug + nmea_handler_debug > 80 )
	g_print ("nmea_handler: gpsd: got no RMC data, using GGA data\n");
      g_snprintf (b, sizeof (b), "%c%c", field[1][4], field[1][5]);
      sscanf (b, "%d", &mysecs);
      if (mysecs != NMEAoldsecs)
	{
	  NMEAsecs = mysecs - NMEAoldsecs;
	  if (NMEAsecs < 0)
	    NMEAsecs += 60;
	  NMEAoldsecs = mysecs;
	}
      /*       g_print("nmeasecs: %.2f mysecs: %d, nmeaoldsecs: %d\n", NMEAsecs, */
      /*         mysecs, NMEAoldsecs); */
      g_snprintf (b, sizeof (b), "%c%c:%c%c.%c%c ", field[1][0],
		  field[1][1], field[1][2], field[1][3],
		  field[1][4], field[1][5]);
      g_strlcpy (utctime, b, sizeof (utctime));

      if (field[6][0] == '0')
	{
	  havepos = FALSE;
	  haveposcount = 0;
	  return;
	}
      else
	{
	  havepos = TRUE;
	  haveposcount++;
	  if (haveposcount == 3)
	    {
	      rebuildtracklist ();
	    }
	}

      /*  Latitude North / South */
      /* if field[2] is shorter than 9 characters, add zeroes in beginning */
      if (strlen (field[2]) < 9)
	{
	  if ( mydebug + nmea_handler_debug > 0 )
	    {
	      g_print
		("nmea_handler: Latitude field %s is shorter than 9 characters. (%d)\n",
		 field[2], strlen (field[2]));
	    }
	  for (i = 0; i < 9; i++)
	    {
	      b[i] = '0';
	    }
	  b[9] = 0;
	  g_strlcpy (b + (9 - strlen (field[2])), field[2], sizeof (b));
	  g_strlcpy (field[2], b, sizeof (field[2]));
	}

      b[0] = field[2][0];
      b[1] = field[2][1];
      b[2] = 0;
      latdegree = atoi (b);


      b[0] = field[2][2];
      b[1] = field[2][3];
      b[2] = '.';
      b[3] = field[2][5];
      b[4] = field[2][6];
      b[5] = field[2][7];
      b[6] = field[2][8];
      b[7] = 0;
      if ( mydebug + nmea_handler_debug > 80 )
	fprintf (stderr, "nmea_handler: gpsd: posmode: %d\n", posmode);
      if (!posmode)
	{
	  gdouble cl;
	  cl = latdegree + atof (b) / 60.0;
	  if (field[3][0] == 'S')
	    cl = cl * -1;
	  if ((cl >= -90.0) && (cl <= 90.0))
	    current_lat = cl;

	  breitri = field[3][0];
	  /*    fprintf (stderr, "%8.5f%s%c cl:%f\n", current_lat, gradsym, breitri,cl); */
	  if ( mydebug + nmea_handler_debug > 0 )
	      {
		  g_print ("nmea_handler: lat: %8.5f\n", current_lat);
	      }
	}

      /*  Longitude East / West */
      /* if field[4] is shorter than 10 chars, add zeroes in the beginning */
      if (strlen (field[4]) < 10)
	{
	  if ( mydebug + nmea_handler_debug > 0 )
	    {
	      g_print
		("nmea_handler: Longitude field %s is shorter than 10 characters. (%d)\n",
		 field[4], strlen (field[4]));
	    }
	  for (i = 0; i < 10; i++)
	    {
	      b[i] = '0';
	    }
	  b[10] = 0;
	  g_strlcpy (b + (10 - strlen (field[4])), field[4], sizeof (b));
	  g_strlcpy (field[4], b, sizeof (field[4]));
	}

      b[0] = field[4][0];
      b[1] = field[4][1];
      b[2] = field[4][2];
      b[3] = 0;
      longdegree = atoi (b);

      b[0] = field[4][3];
      b[1] = field[4][4];
      b[2] = '.';
      b[3] = field[4][6];
      b[4] = field[4][7];
      b[5] = field[4][8];
      b[6] = field[4][9];
      b[7] = 0;

      if (!posmode)
	{
	  gdouble cl;
	  cl = longdegree + atof (b) / 60.0;
	  if (field[5][0] == 'W')
	    cl = cl * -1;
	  if ((cl >= -180.0) && (cl <= 180.0))
	    current_lon = cl;

	  langri = field[5][0];
	  /*    fprintf (stderr, "%8.5f%s%c cl:%f\n", current_lon, gradsym, langri,cl); */
	  if ( mydebug + nmea_handler_debug > 0 )
	      {
		  g_print ("nmea_handler: lon: %8.5f\n", current_lon);
	      }
	}

      if ( mydebug + nmea_handler_debug > 80 )
	g_print ("nmea_handler: gpsd: GGA pos: %f %f\n", current_lat, current_lon);
    }

  satfix = g_strtod (field[6], 0);
  numsats = g_strtod (field[7], 0);
  if (havepos)
    {
      havealtitude = TRUE;
      altitude = g_strtod (field[9], 0);
      if ( mydebug + nmea_handler_debug > 80 )
	g_print ("nmea_handler: gpsd: Altitude: %.1f, Fix: %d\n", altitude, satfix);
    }
  else
    {
      groundspeed = 0;
      numsats = 0;
    }
  {
    int h, m, s;
    h = m = s = 0;
    if (strcmp (utctime, "n/a") != 0)
      {
	sscanf (utctime, "%d:%d.%d", &h, &m, &s);
	h += zone;
	if (h > 23)
	  h -= 24;
	if (h < 0)
	  h += 24;
	g_snprintf (loctime, sizeof (loctime), "%d:%02d.%02d", h, m, s);
      }
    else
      g_strlcpy (loctime, "n/a", sizeof (loctime));
  }
}


/* ******************************************************************
 * show estimated position error $PGRME (Garmin only)  
 */
void
convertRME (char *f)
{
  gchar field[50][100], b[500];
  gint i, l, j = 0, start = 0;

  memset (b, 0, 100);
  l = strlen (f);
  for (i = 0; i < l; i++)
    {
      if ((f[i] == ',') || (f[i] == '*'))
	{
	  g_strlcpy (field[j], (f + start), 100);
	  field[j][i - start] = 0;
	  start = i + 1;
	  j++;
	}
    }
  if ( mydebug + nmea_handler_debug > 80 )
    {
      g_print ("nmea_handler: gpsd: RME Fields: ");
      for (i = 0; i < j; i++)
	g_print ("%d:%s$", i, field[i]);
      g_print ("\n");
    }
  if (havepos)
    {
      precision = g_strtod (field[1], 0);
      if ( mydebug + nmea_handler_debug > 80 )
	g_print ("nmea_handler: gpsd: RME precision: %.1f\n", precision);
    }
}
