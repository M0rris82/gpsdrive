/*************************************************************************

Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>

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

*********************************************************************/


/*
 * speech support module using speech-dispatcher (speechd)
 */


#ifdef SPEECH


#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include <sys/time.h>
#include <gpsdrive.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>



#include "gettext.h"

#include <time.h>
#include "gui.h"
#include "gpsdrive_config.h"
#include "os_specific.h"



#include <libspeechd.h>
#include "speech.h"


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint mydebug;
extern currentstatus_struct current;


gchar **speech_modules = NULL;
GtkListStore *speech_voices_list;

static SPDConnection *speechd_conn = NULL;


/* TODO: check these variables and maybe clean up */
extern gchar oldangle[100];
gint speechcount = 0;



/* *****************************************************************************
 * function to speak a given text string
 * 'priority' sets the message priority from 1 (highest) to 5 (lowest):
 * (2) is for important messages like turn instructions while routing
 * (3) should be the standard
 * (4) is for less important information like saying the time regularly
 */
gint
speech_saytext (gchar *text, gint priority)
{
	if (!current.have_speech)
		return FALSE;

	if (!local_config.speech || local_config.mute)
		return FALSE;

	if (mydebug > 30)
		g_print ("speech_saytext: %s\n", text);

	if (spd_say (speechd_conn, priority, text) == -1)
	{
		g_print ("ERROR: speech output failed!\n");
		return FALSE;
	}

	return TRUE;
}


/* *****************************************************************************
 *  say the current time. if 'greeting' is TRUE, then also a greeting is spoken
 */
gint
speech_saytime (gboolean greeting)
{
	gchar t_greeting[100];
	gchar t_buf[200];
	time_t t;
	struct tm *ts;

	time (&t);
	ts = localtime (&t);

	if (greeting)
	{
		if( (ts->tm_hour >= 0) && (ts->tm_hour < 12) )
			g_snprintf (t_greeting, sizeof(t_greeting), _("Good morning,"));
		if( (ts->tm_hour >= 12) && (ts->tm_hour < 18) )
			g_snprintf (t_greeting, sizeof(t_greeting), _("Good afternoon,"));
		if( ts->tm_hour >= 18 )
			g_snprintf (t_greeting, sizeof(t_greeting), _("Good evening,"));
	}
	else
		g_snprintf (t_greeting, sizeof (t_greeting), "");

	if (1 == ts->tm_hour)
		g_snprintf (t_buf, sizeof(t_buf), _("%s It is one %d."),
			t_greeting, ts->tm_min);
	else if (0 == ts->tm_min)
		g_snprintf (t_buf, sizeof(t_buf), _("%s It is %d o clock."),
			t_greeting,ts->tm_hour);
	else
		g_snprintf (t_buf, sizeof(t_buf), _("%s It is %d %d."),
			t_greeting, ts->tm_hour, ts->tm_min );

	if (greeting)
		speech_saytext (t_buf, 2);
	else
		speech_saytext (t_buf, 4);


	return TRUE;
}


/* *****************************************************************************
 *  say the current gps fix mode
 */
void
speech_say_gpsfix (void)
{
	static gint old_gps_status = -1;

	if (current.gps_status == old_gps_status)
		return;

	switch (current.gps_status)
	{
		case GPS_NO_FIX:
			speech_saytext (_("Not enough satellites in view."), 3);
			break;
		case GPS_FIX:
			if ( old_gps_status == GPS_DGPS_FIX )
				speech_saytext (_("Differential GPS signal lost."), 3);
			else
				speech_saytext (_("GPS signal is good."), 3);
			break;
		case GPS_DGPS_FIX:
			speech_saytext (_("Differential GPS signal found."), 3);
			break;
	}

	old_gps_status = current.gps_status;
}


/* *****************************************************************************
 */
void
speech_set_parameters (void)
{
	/* set voice parameters */
	spd_set_voice_rate (speechd_conn, local_config.speech_speed);
	spd_set_voice_pitch (speechd_conn, local_config.speech_pitch);
	//spd_set_volume(SPDConnection* connection, local_config.speech.volume);
}


/* *****************************************************************************
 */
void
speech_set_module ()
{
	SPDVoice **t_buf;
	GtkTreeIter t_iter;
	gint i = 0;

	/* set output module */
	spd_set_output_module (speechd_conn, local_config.speech_module);

	/* retrieve available voices from chosen output module */
	gtk_list_store_clear (speech_voices_list);
	t_buf = spd_list_synthesis_voices(speechd_conn);
	if (t_buf)
	{
		while (t_buf[i] !=NULL)
		{
			if (mydebug > 20)
				g_print ("--- %s --- %s/%s\n", t_buf[i]->name,
					t_buf[i]->language, t_buf[i]->variant);

			gtk_list_store_append (speech_voices_list, &t_iter);
			gtk_list_store_set (speech_voices_list, &t_iter,
				VOICE_NAME, t_buf[i]->name,
				VOICE_LANGUAGE, t_buf[i]->language,
				VOICE_VARIANT, t_buf[i]->variant,
				-1);
			if (strcmp (t_buf[i]->variant, "none") == 0)
				gtk_list_store_set (speech_voices_list, &t_iter,
				VOICE_VARIANT, "", -1);
			i++;
		}
		g_free (t_buf);
	}
}


/* *****************************************************************************
 */
void
speech_set_voice (void)
{
	spd_set_synthesis_voice(speechd_conn, local_config.speech_voice);
}


/* *****************************************************************************
 */
void
speech_close (void)
{
	if (!local_config.mute)
		speech_saytext (_("Good bye. Thank you for using G P S drive!"), 2);

	g_free (speech_modules);

	if (speechd_conn)
		spd_close (speechd_conn);
}


/* *****************************************************************************
 */
gint
speech_init (void)
{
	if (speechd_conn == NULL)
		speechd_conn = spd_open ( "GpsDrive", NULL, NULL, SPD_MODE_SINGLE);

	if (speechd_conn == NULL)
	{
		g_print ("Speech Dispatcher not available, disabling speech output!\n");
		local_config.speech = FALSE;
		return FALSE;
	}
	speech_modules = spd_list_modules (speechd_conn);
	g_print ("Connection to Speech Dispatcher established.\n");

	speech_voices_list = gtk_list_store_new (VOICE_N_COLUMNS,
		G_TYPE_STRING,		/* VOICE_NAME */
		G_TYPE_STRING,		/* VOICE_LANGUAGE */
		G_TYPE_STRING		/* VOICE_VARIANT */
		);

	speech_set_parameters ();
	speech_set_module ();
	speech_set_voice ();

	return TRUE;
}


/* *****************************************************************************
 * TODO: split the parts of this function to separate ones and clean up...
 */
gint
speech_out_cb (GtkWidget * widget, guint * datum)
{
	gchar buf[500], s2[100];
	gint angle;

	/* destroy timeout function when speech output is disabled */
	if (!local_config.speech)
		return FALSE;

	if (local_config.mute)
		return TRUE;

	if (strcmp (oldangle, "XXX"))
	{
		if (local_config.mute)
			return TRUE;
		if (current.importactive)
			return TRUE;
	}

	speechcount++;

	angle = RAD2DEG (current.bearing) - RAD2DEG (current.heading);
	if (angle < 0.0)
		angle = 360 + angle;

	if (!current.simmode && current.gps_status == GPS_NO_FIX)
	{
		if( (1 == speechcount) && local_config.sound_gps )
    {
      g_snprintf( buf, sizeof(buf), _("Not enough satellites in view.") );
			speech_saytext (buf, 4);
    }

		return TRUE;
	}

  if( (angle >= 338) || (angle < 22) )
  {
    g_strlcpy( s2, _("in front of you."), sizeof(s2) );
  }
  else if( (angle >= 22) && (angle < 68) )
  {
    g_strlcpy( s2, _("ahead of you to the right."), sizeof(s2) );
  }
  else if( (angle >= 68) && (angle < 112) )
  {
    g_strlcpy( s2, _("to your right."), sizeof(s2) );
  }
  else if( (angle >= 112) && (angle < 158) )
  {
    g_strlcpy( s2, _("behind you to the right."), sizeof(s2) );
  }
  else if( (angle >= 158) && (angle < 202) )
  {
    g_strlcpy( s2, _("behind you."), sizeof(s2) );
  }
  else if( (angle >= 202) && (angle < 248) )
  {
    g_strlcpy( s2, _("behind you to the left."), sizeof(s2) );
  }
  else if( (angle >= 248) && (angle < 292) )
  {
    g_strlcpy( s2, _("to your left."), sizeof(s2) );
  }
  else if( (angle >= 292) && (angle < 338) )
  {
    g_strlcpy( s2, _("ahead of you to the left."), sizeof(s2) );
  }

  //printf ("angle: %u\n", angle);

	if( (1 == speechcount) || (strcmp (s2, oldangle)) )
	{
		if (local_config.sound_direction)
    {
      g_snprintf( buf, sizeof(buf), ("Destination is %s"), s2 );
			speech_saytext (buf, 3);
    }

		g_strlcpy (oldangle, s2, sizeof (oldangle));
	}
	if( (3 == speechcount) && (current.groundspeed >= 20) )
	{
    if (local_config.sound_speed)
    {
      if (local_config.distmode == DIST_MILES)
      {
        g_snprintf(
          buf, sizeof(buf), _("The current speed is %d miles per hour."), (int) current.groundspeed );
      }
      else if (local_config.distmode == DIST_NAUTIC)
      {
        g_snprintf(
          buf, sizeof(buf), _("The current speed is %d knots."), (int) current.groundspeed );
      }
      else
      {
        g_snprintf(
          buf, sizeof(buf), _("The current speed is %d kilometers per hour."), (int) current.groundspeed );
      }

      speech_saytext (buf, 2);
    }
	}

	if (speechcount > 10)
		speechcount = 0;

  if( (2 == speechcount) || ((current.dist < 1.2) && (7 == speechcount)) )
  {
    if (local_config.sound_distance)
    {
      if (local_config.distmode == DIST_MILES)
      {
        if( current.dist <= 1.2 )
        {
          g_snprintf( s2, sizeof(s2), _("%.0f yards."), current.dist * 1760.0 );
        }
        else
        {
          g_snprintf( s2, sizeof(s2), _("%.0f miles."), current.dist );
        }
      }
      else
      {
        if( current.dist <= 1.2 )
        {
          g_snprintf( s2, sizeof(s2), _("%d meters."), (int) (current.dist * 1000) );
        }
        else if( 1 == (int) current.dist )
        {
          g_snprintf( s2, sizeof(s2), _("one kilometer.") );
        }
        else
        {
          g_snprintf(
            s2, sizeof(s2), _("%d kilometers."), (int) current.dist );
        }
      }

      if(strcmp(current.target, "     ") == 0) {
          g_snprintf(
            buf, sizeof(buf), _("Distance to unnamed target is %s"), s2 );
      } else {
          g_snprintf(
            buf, sizeof(buf), _("Distance to %s is %s"), current.target, s2 );
      }

      speech_saytext (buf, 4);
    }
  }

	return TRUE;
}

#endif
