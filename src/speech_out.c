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


$Log$
Revision 1.3  2005/04/10 20:47:49  tweety
added src/speech_out.h
update configure and po Files

Revision 1.2  2005/03/25 14:38:46  tweety
change flite usage so no file is generated inbetween. This also fixes problem with flite having
problems speaking a file without a trailing \n

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.37  2004/03/02 00:53:35  ganter
v2.09pre1
added new gpsfetchmap.pl (works again with Expedia)
added sound settings in settings menu
max serial device string is now 40 char

Revision 1.36  2004/02/08 17:16:25  ganter
replacing all strcat with g_strlcat to avoid buffer overflows

Revision 1.35  2004/02/08 16:35:10  ganter
replacing all sprintf with g_snprintf to avoid buffer overflows

Revision 1.34  2004/02/06 22:29:24  ganter
updated README and man page

Revision 1.33  2004/02/02 03:38:32  ganter
code cleanup

Revision 1.32  2004/01/29 04:42:17  ganter
after valgrind

Revision 1.31  2004/01/17 17:41:48  ganter
replaced all gdk_pixbuf_render_to_drawable (obsolet) with gdk_draw_pixbuf

Revision 1.30  2004/01/16 13:19:59  ganter
update targetlist if goto button pressed

Revision 1.29  2004/01/11 10:01:57  ganter
gray border for .dsc file text

Revision 1.28  2004/01/05 05:52:58  ganter
changed all frames to respect setting

Revision 1.27  2004/01/01 09:07:33  ganter
v2.06
trip info is now live updated
added cpu temperature display for acpi
added tooltips for battery and temperature

Revision 1.26  2003/12/17 02:17:56  ganter
added donation window
waypoint describtion (.dsc files) works again
added dist_alarm ...

Revision 1.25  2003/10/04 17:43:58  ganter
translations don't need to be utf-8, but the .po files must specify the
correct coding (ie, UTF-8, iso8859-15)

Revision 1.24  2003/05/11 21:15:46  ganter
v2.0pre7
added script convgiftopng
This script converts .gif into .png files, which reduces CPU load
run this script in your maps directory, you need "convert" from ImageMagick

Friends mode runs fine now
Added parameter -H to correct the alitude

Revision 1.23  2003/05/07 10:52:23  ganter
...

Revision 1.22  2003/01/15 15:30:28  ganter
before dynamically loading mysql

Revision 1.21  2002/11/02 12:38:55  ganter
changed website to www.gpsdrive.de

Revision 1.20  2002/07/30 20:49:55  ganter
1.26pre3
added support for festival lite (flite)
changed http request to HTTP1.1 and added correct servername

Revision 1.19  2002/06/23 17:09:35  ganter
v1.23pre9
now PDA mode looks good.

Revision 1.18  2002/06/02 20:54:10  ganter
added navigation.c and copyrights

Revision 1.17  2002/05/11 15:45:31  ganter
v1.21pre1
degree,minutes,seconds should work now

Revision 1.16  2002/05/04 10:48:24  ganter
v1.20pre2

Revision 1.15  2002/05/04 09:17:37  ganter
added new intl subdir

Revision 1.14  2002/05/02 01:34:11  ganter
added speech output of waypoint description

Revision 1.13  2002/04/14 23:06:26  ganter
v1.17

Revision 1.12  2001/11/01 20:17:59  ganter
v1.0 added spanish voice output

Revision 1.11  2001/09/30 18:45:27  ganter
v0.29
added choice of map type

Revision 1.10  2001/09/30 12:09:43  ganter
added help menu

Revision 1.9  2001/09/28 15:43:56  ganter
v0.28 changed layout, some bugfixes

Revision 1.8  2001/09/25 23:49:44  ganter
v 0.27

Revision 1.7  2001/09/23 22:31:14  ganter
v0.26

Revision 1.6  2001/09/18 05:33:06  ganter
..

Revision 1.5  2001/09/17 00:29:38  ganter
added speech output of bearing

Revision 1.4  2001/09/16 21:36:05  ganter
speech output is working

Revision 1.3  2001/09/16 19:12:35  ganter
...

*/


/*  There must be the software "festival" running in server mode */
/*  http://www.speech.cs.cmu.edu/festival */


/*  Include Dateien */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <sys/time.h>
#include <gpsdrive.h>
#include <speech_out.h>

#if HAVE_LOCALE_H
#include <locale.h>
#else
# define setlocale(Category, Locale)
#endif
#include "gettext.h"

#include <time.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern gint statusid, debug, havespeechout, posmode, muteflag;
gint speechsock = -1;
gchar *displaytext = NULL;
extern enum
{ english, german, spanish }
voicelang;
extern GdkFont *textfont, *smalltextfont, *bigtextfont;
extern GdkColor white, red, mygray, blue;
extern GdkDrawable *drawable;
extern gint real_screen_y, real_screen_x;
gint do_display_dsc = FALSE, textcount;
extern gint useflite, pdamode,milesflag,foundradar,importactive,speechcount,simmode,havepos;
extern gchar targetname[40], oldangle[100];
extern gdouble dist,bearing,groundspeed;
extern GtkWidget *drawing_area;
extern int sound_direction, sound_distance, sound_speed, sound_gps;


#define SPEECHOUTSERVER "127.0.0.1"


gint
speech_out_init ()
{
  struct sockaddr_in server;
  struct hostent *server_data;

/*  open socket to port 1314 */
  if ((speechsock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      return (FALSE);
    }
  server.sin_family = AF_INET;
/*  We retrieve the IP address of the server from its name: */
  if ((server_data = gethostbyname (SPEECHOUTSERVER)) == NULL)
    {
      return (FALSE);
    }
  memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
  server.sin_port = htons (1314);
/*  We initiate the connection  */
  if (connect (speechsock, (struct sockaddr *) &server, sizeof server) < 0)
    {
      return (FALSE);
    }


  return TRUE;
}


gint
speech_out_speek (char *text)
{
  gint e;
  gchar out[2000];

  if (!havespeechout)
    return 0;
  if (posmode)
    return 0;

  if (debug)
    g_print (text);
  if (!useflite)
    {
      g_snprintf (out, sizeof (out), "(SayText \"%s\")\n", text);
      e = write (speechsock, out, strlen (out));
    }
  else
    {
      g_strlcat (text, ".\n", sizeof (text));
      g_snprintf (out, sizeof (out), "flite -t '%s'&",text);
      if ( debug ) 
	printf("speech with flite: %s\n",out);
      system (out);
    }
  return e;
}

gint
speech_out_speek_raw (char *text)
{
  gint e;

  if (!havespeechout)
    return 0;
  if (posmode)
    return 0;

  if (debug)
    g_print (text);

  e = write (speechsock, text, strlen (text));

  return e;
}

/*  if second parameter is TRUE, then also greeting is spoken */
gint
speech_saytime_cb (GtkWidget * widget, guint datum)
{
  time_t t;
  struct tm *ts;
  gchar buf[200];

  if (muteflag)
    return TRUE;

  time (&t);
  ts = localtime (&t);

  if (havespeechout)
    switch (voicelang)
      {
      case english:
	if (datum == 1)
	  {
	    if ((ts->tm_hour >= 0) && (ts->tm_hour < 12))
	      g_snprintf (buf, sizeof (buf), "Good Morning,");
	    if ((ts->tm_hour >= 12) && (ts->tm_hour < 18))
	      g_snprintf (buf, sizeof (buf), "Good afternoon,");
	    if (ts->tm_hour >= 18)
	      g_snprintf (buf, sizeof (buf), "Good evening,");
	    speech_out_speek (buf);
	  }
	g_snprintf (buf, sizeof (buf),
		    "it is %d o clock and %d minutes.", ts->tm_hour,
		    ts->tm_min);
	speech_out_speek (buf);
	break;
      case spanish:
	if (datum == 1)
	  {
	    if ((ts->tm_hour >= 0) && (ts->tm_hour < 12))
	      g_snprintf (buf, sizeof (buf), "buenos dÂŽías.");
	    if ((ts->tm_hour >= 12) && (ts->tm_hour < 18))
	      g_snprintf (buf, sizeof (buf), "buenos tardes.");
	    if (ts->tm_hour >= 18)
	      g_snprintf (buf, sizeof (buf), "buenas noches");
	    speech_out_speek (buf);
	  }
	if (ts->tm_hour == 1)
	  g_snprintf (buf, sizeof (buf), "es la una y %d minutos.",
		      ts->tm_min);
	else
	  g_snprintf (buf, sizeof (buf),
		      "son las %d horas y %d minutos.", ts->tm_hour,
		      ts->tm_min);
	speech_out_speek (buf);
	break;
      case german:
	if (datum == 1)
	  {
	    if ((ts->tm_hour >= 0) && (ts->tm_hour < 12))
	      g_snprintf (buf, sizeof (buf), "Guten Morgen.");
	    if ((ts->tm_hour >= 12) && (ts->tm_hour < 18))
	      g_snprintf (buf, sizeof (buf), "Guten Tag.");
	    if (ts->tm_hour >= 18)
	      g_snprintf (buf, sizeof (buf), "Guten Abend.");
	    speech_out_speek (buf);
	  }
	if (ts->tm_hour == 1)
	  g_snprintf (buf, sizeof (buf), "es ist ein Uhr %d", ts->tm_min);
	else
	  g_snprintf (buf, sizeof (buf), "es ist %d Uhr %d", ts->tm_hour,
		      ts->tm_min);
	speech_out_speek (buf);

      }

  return TRUE;

}

void
saytargettext (gchar * filename, gchar * tg)
{
  gchar file[500];
  gint fd, e;
  gchar *start, *end;
  struct stat buf;
  gchar *data, *b, *tg2, target[100];

/* build .dsc filename */
  g_strlcpy (file, filename, sizeof (file));
  file[strlen (file) - 3] = 0;
  g_strlcat (file, "dsc", sizeof (file));
/* get size */
  e = stat (file, &buf);
  if (e != 0)
    return;

  fd = open (file, O_RDONLY);
/* map +2000 bytes to get 0 at the end */
  data = mmap (0, buf.st_size + 2000, PROT_READ, MAP_SHARED, fd, 0);
  g_strlcpy (target, "$", sizeof (target));
  tg2 = g_strdelimit (tg, " ", '_');
  g_strlcat (target, tg2, sizeof (target));
  start = strstr (data, target);
  if (start != NULL)
    {
      start = strstr (start, "\n");
      end = strstr (start, "$");
      if (end == NULL)
	end = start + strlen (start);
      b = calloc (end - start + 50, 1);
      if (displaytext != NULL)
	free (displaytext);
      displaytext = calloc (end - start + 50, 1);
      strncpy (displaytext, start, end - start);
      displaytext[end - start + 1] = 0;
      g_strlcpy (b, displaytext, end - start + 50);
      displaytext = g_strdelimit (displaytext, "\n", ' ');
      do_display_dsc = TRUE;
      textcount = 0;
      speech_out_speek (b);
      free (b);
    }
  munmap (data, buf.st_size + 2000);
}

void
display_dsc (void)
{
  GdkGC *kontext;
  gint len;
  gchar *text;
  PangoFontDescription *pfd;
  PangoLayout *wplabellayout;


  if (!do_display_dsc)
    return;
  if ((textcount >= (int) strlen (displaytext)))
    {
      do_display_dsc = FALSE;
      free (displaytext);
      displaytext = NULL;
      return;
    }
  if (textcount > 20)
    text = displaytext + textcount;
  else
    text = displaytext;
  kontext = gdk_gc_new (drawable);
  len = strlen (text);
/*   if (len>10) */
/*       len=10; */


/*   gdk_gc_set_function (kontext, GDK_OR); */

  gdk_gc_set_foreground (kontext, &mygray);
  gdk_draw_rectangle (drawable, kontext, 1, 0, SCREEN_Y - 40, SCREEN_X, 40);
  gdk_gc_set_function (kontext, GDK_COPY);
/*   gdk_gc_set_foreground (kontext, &blue); */
/*   gdk_draw_text (drawable, bigtextfont, kontext, */
/* 		 11, SCREEN_Y - 30, text, len); */

  /* prints in pango */

  wplabellayout = gtk_widget_create_pango_layout (drawing_area, text);
//KCFX  
  if (pdamode)
    pfd = pango_font_description_from_string ("Sans 8");
  else
    pfd = pango_font_description_from_string ("Sans bold 14");
  pango_layout_set_font_description (wplabellayout, pfd);
/* 		pango_layout_get_pixel_size (wplabellayout, &width, &height); */
  gdk_draw_layout_with_colors (drawable, kontext, 11, SCREEN_Y - 30,
			       wplabellayout, &blue, NULL);

  if (wplabellayout != NULL)
    g_object_unref (G_OBJECT (wplabellayout));
  /* freeing PangoFontDescription, cause it has been copied by prev. call */
  pango_font_description_free (pfd);



  textcount += 2;

}

void
speech_out_close (void)
{
  if (speechsock != -1)
    close (speechsock);
}


gint
speech_out_cb (GtkWidget * widget, guint * datum)
{
  gchar buf[500], s2[100];
  gint angle;

  if (strcmp (oldangle, "XXX"))
    {
      if (muteflag)
	return TRUE;
      if (foundradar)
	return TRUE;
      if (importactive)
	return TRUE;
    }

  speechcount++;
  angle = bearing * 180.0 / M_PI;

  if (!simmode && !havepos)
    {
      switch (voicelang)
	{
	case english:
	  g_snprintf (buf, sizeof (buf), "Not enough satellites in view");
	  break;
	case spanish:
	  g_snprintf (buf, sizeof (buf), "El GPS Fix no estÂŽá disponible");
	  break;
	case german:
	  g_snprintf (buf, sizeof (buf), "Zuwenig Satelliten in Sicht");
	}
      if ((speechcount == 1) && sound_gps)
	speech_out_speek (buf);
      return TRUE;
    }


  switch (voicelang)
    {
    case english:
      if ((angle >= 338) || (angle < 22))
	g_strlcpy (s2, "in front of you", sizeof (s2));
      if ((angle >= 22) && (angle < 68))
	g_strlcpy (s2, "ahead of you to the right", sizeof (s2));
      if ((angle >= 68) && (angle < 112))
	g_strlcpy (s2, "to your right", sizeof (s2));
      if ((angle >= 112) && (angle < 158))
	g_strlcpy (s2, "behind you to the right", sizeof (s2));
      if ((angle >= 158) && (angle < 202))
	g_strlcpy (s2, "behind you", sizeof (s2));
      if ((angle >= 202) && (angle < 248))
	g_strlcpy (s2, "behind you to the left", sizeof (s2));
      if ((angle >= 248) && (angle < 292))
	g_strlcpy (s2, "to your left", sizeof (s2));
      if ((angle >= 292) && (angle < 338))
	g_strlcpy (s2, "ahead of you to the left", sizeof (s2));
      break;
    case spanish:
      if ((angle >= 338) || (angle < 22))
	g_strlcpy (s2, "delante de usted", sizeof (s2));
      if ((angle >= 22) && (angle < 68))
	g_strlcpy (s2, "delante de usted a la derecha", sizeof (s2));
      if ((angle >= 68) && (angle < 112))
	g_strlcpy (s2, "a la derecha", sizeof (s2));
      if ((angle >= 112) && (angle < 158))
	g_strlcpy (s2, "de tras de usted a la derecha", sizeof (s2));
      if ((angle >= 158) && (angle < 202))
	g_strlcpy (s2, "de tras de usted", sizeof (s2));
      if ((angle >= 202) && (angle < 248))
	g_strlcpy (s2, "de tras de usted a la izquierda", sizeof (s2));
      if ((angle >= 248) && (angle < 292))
	g_strlcpy (s2, "a la izquierda", sizeof (s2));
      if ((angle >= 292) && (angle < 338))
	g_strlcpy (s2, "delante de usted a la izquierda", sizeof (s2));
      break;
    case german:
      if ((angle >= 338) || (angle < 22))
	g_strlcpy (s2, "vor ihnen", sizeof (s2));

      if ((angle >= 22) && (angle < 68))
	g_strlcpy (s2, "rechts vor ihnen", sizeof (s2));
      if ((angle >= 68) && (angle < 112))
	g_strlcpy (s2, "rechts", sizeof (s2));
      if ((angle >= 112) && (angle < 158))
	g_strlcpy (s2, "rechts hinter ihnen", sizeof (s2));
      if ((angle >= 158) && (angle < 202))
	g_strlcpy (s2, "hinter ihnen", sizeof (s2));
      if ((angle >= 202) && (angle < 248))
	g_strlcpy (s2, "links hinter ihnen", sizeof (s2));
      if ((angle >= 248) && (angle < 292))
	g_strlcpy (s2, "links", sizeof (s2));
      if ((angle >= 292) && (angle < 338))
	g_strlcpy (s2, "links vor ihnen", sizeof (s2));
    }
  if ((speechcount == 1) || (strcmp (s2, oldangle)))
    {
      switch (voicelang)
	{
	case english:
	  g_snprintf (buf, sizeof (buf), "Destination is %s", s2);
	  break;
	case spanish:
	  g_snprintf (buf, sizeof (buf), "Su destinaciÂŽón estÂŽá %s", s2);
	  break;
	case german:
	  g_snprintf (buf, sizeof (buf), "Das Ziel ist %s", s2);
	}
      if (sound_direction)
      speech_out_speek (buf);
      g_strlcpy (oldangle, s2, sizeof (oldangle));
    }
  if (speechcount == 3 && groundspeed >= 20)
    {
      switch (voicelang)
	{
	case english:
	  if (milesflag)
	    g_snprintf (buf, sizeof (buf),
			"The current speed is %d miles per hour",
			(int) (groundspeed));
	  else
	    g_snprintf (buf, sizeof (buf),
			"The current speed is %d kilometers per hour",
			(int) groundspeed);
	  break;
	case spanish:
	  if (milesflag)
	    g_snprintf (buf, sizeof (buf),
			"La velocidad actual es %d milla por hora",
			(int) (groundspeed));
	  else
	    g_snprintf (buf, sizeof (buf),
			"La velocidad actual es %d kilometros por hora",
			(int) groundspeed);
	  break;
	case german:
	  if (milesflag)
	    g_snprintf (buf, sizeof (buf),
			"Die momentane Geschwindigkeit ist %d Meilen pro Stunde",
			(int) (groundspeed));
	  else
	    g_snprintf (buf, sizeof (buf),
			"Die momentane Geschwindigkeit ist %d kmh",
			(int) groundspeed);
	}
      if (sound_speed)
      speech_out_speek (buf);
    }

  if (speechcount > 10)
    speechcount = 0;


  if ((speechcount == 2) || ((dist < 1.2) && (speechcount == 7)))
    {
      if (milesflag)
	{
	  switch (voicelang)
	    {
	    case english:
	      if (dist <= 1.2)
		g_snprintf (s2, sizeof (s2), "%.0f yards", dist * 1760.0);
	      else
		g_snprintf (s2, sizeof (s2), "%.0f miles", dist);
	      break;
	    case spanish:
	      if (dist <= 1.2)
		g_snprintf (s2, sizeof (s2), "%.0f yards", dist * 1760.0);
	      else
		g_snprintf (s2, sizeof (s2), "%.0f millas", dist);
	      break;
	    case german:
	      if (dist <= 1.2)
		g_snprintf (s2, sizeof (s2), "%.0f yard", dist * 1760.0);
	      else
		g_snprintf (s2, sizeof (s2), "%.0f Meilen", dist);
	    }
	}
      else
	{
	  switch (voicelang)
	    {
	    case english:
	      if (dist <= 1.2)
		g_snprintf (s2, sizeof (s2), "%.0f meters", dist * 1000.0);
	      else
		g_snprintf (s2, sizeof (s2), "%.0f kilometers", dist);
	      break;
	    case spanish:
	      if (dist <= 1.2)
		g_snprintf (s2, sizeof (s2), "%.0f metros", dist * 1000.0);
	      else
		g_snprintf (s2, sizeof (s2), "%.0f kilometros", dist);
	      break;
	    case german:
	      if (dist <= 1.2)
		g_snprintf (s2, sizeof (s2), "%.0f meter", dist * 1000.0);
	      else if ((int) dist == 1)
		g_snprintf (s2, sizeof (s2), "ein kilometer");
	      else
		g_snprintf (s2, sizeof (s2), "%d kilometer", (int) dist);
	    }
	}

      switch (voicelang)
	{
	case english:
	  g_snprintf (buf, sizeof (buf), "Distance to %s is %s", targetname,
		      s2);
	  break;
	case spanish:
	  g_snprintf (buf, sizeof (buf), "La distancia a la %s es %s",
		      targetname, s2);
	  break;
	case german:
	  g_snprintf (buf, sizeof (buf), "Die Entfernung bis %s ist %s",
		      targetname, s2);
	}
      if (sound_distance)
      speech_out_speek (buf);
    }

  return TRUE;
}
