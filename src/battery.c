/***********************************************************************

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

*********************************************************************/
/*
  $Log$
  Revision 1.17  2006/02/16 09:52:44  tweety
  rearrange acpi handling and displaying of battery and temperature display

  Revision 1.16  2006/02/14 08:29:39  tweety
  eliminate warnings

  Revision 1.15  2006/02/10 22:33:26  tweety
  fix more in the ACPI/APM handling. write unti tests for this part of Code

  Revision 1.14  2006/02/10 17:36:04  tweety
  rearrange ACPI handling

  Revision 1.13  2006/02/07 22:28:13  tweety
  more moving anf separating of battery and temperature code

  Revision 1.12  2006/02/07 07:06:51  tweety
  split apart the battery and temperature handling a little bit

  Revision 1.10  2006/01/12 15:01:19  tweety
  check if number of parameters in batteryfile is ok

  Revision 1.33  2004/01/18 16:09:37  ganter
  fixed last memleak in battery.c (I hope)

  Revision 1.31  2004/01/17 17:49:38  ganter
  no need to create batimage always new, made it static

  Revision 1.30  2004/01/17 17:41:48  ganter
  replaced all gdk_pixbuf_render_to_drawable (obsolet) with gdk_draw_pixbuf

  Revision 1.29  2004/01/15 22:02:40  ganter
  added openbsd patches
  real 2.07pre9

  Revision 1.19  2003/12/28 20:12:29  ganter
  better acpi-temperature support

  Revision 1.18  2003/12/28 19:48:01  ganter
  added patch from Jaap Hogenberg for temperature display

  Revision 1.16  2003/12/27 23:11:36  ganter
  battery.c now reads all batteries

  Revision 1.15  2003/12/17 21:30:02  ganter
  acpi battery status works now again (tested with 2.4.22ac4)

  Revision 1.11  2002/12/30 17:58:06  molter
  APM is i386 only, allow compilation on FreeBSD alpha too

  Revision 1.6  2002/07/01 00:45:00  ganter
  added trip info (in settings menu)
  ACPI fixes (close battery fd)

*/

/*
 *
 * Power-related (battery, APM) functions.
 *
 * This module exports an interface that is operating system independent.
 * The supported OSes are Linux, FreeBSD and NetBSD.
 *
 * Code modularization and support for FreeBSD by Marco Molteni
 * <molter@gufi.org>.
 *
 * NetBSD support by Berndt Josef Wulf
 * <wulf@netbsd.org>
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <gpsdrive.h>
#include <icons.h>
#include <config.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

/* APM is i386-specific. */
#if defined(__FreeBSD__) && defined(__i386__)
#include <fcntl.h>
#include <machine/apm_bios.h>
#endif /* __FreeBSD__ && __i386__ */

#if defined (__NetBSD__) || (__OpenBSD__)
#include <fcntl.h>
#include <machine/apmvar.h>
#include <sys/ioctl.h>
#endif /* __NetBSD__ */


#include "battery.h"


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#if GTK_MINOR_VERSION < 2
#define gdk_draw_pixbuf _gdk_draw_pixbuf
#endif

#include <speech_out.h>
#include <speech_strings.h>

extern gint mydebug;
extern gint debug;

extern gint int_padding;

/* Global Values */
gchar cputempstring[20] = "??";
gchar batstring[20] = "??";
gchar dir_proc[200] = "/proc";	// make it flexible for unit -tests
GtkWidget *tempeventbox = NULL, *batteventbox = NULL;
GtkTooltips *tooltip_temperature = NULL;
GtkTooltips *tooltip_battery = NULL;

/* --------------------*/
gint batlevel = 125;
gint battime = 0;
gint batlevel_old = 125;	/* battery level, range 0..100 */
gint batloading = FALSE;
gint bnatloading_old = FALSE;	/* is the battery charging? */
gint batcharge = FALSE;
gint cputemp = -99;

static gchar gradsym[] = "\xc2\xb0";
extern GtkWidget *tempeventbox, *batteventbox;
extern GtkTooltips *tooltip_temperature;
static GdkPixbuf *img_powercharges = NULL, *img_powercord =
  NULL, *img_battery = NULL;
GtkWidget *frame_battery = NULL;
GtkWidget *frame_temperature = NULL;
GtkWidget *drawing_battery = NULL, *drawing_temp = NULL;

#ifdef __linux__
/*  ******************************************************************
 * Return TRUE on success, FALSE on error.
 */
static int
battery_get_values_linux_apm (int *blevel, int *bloading, int *bcharge)
{
  FILE *battery = NULL;
  gint i;
  gint ret = FALSE;
  gint p_bat_full = 0;
  gint p_bat_current = 0;
  gchar b[200];
  char fn[200];
  *bcharge = FALSE;
  *bloading = FALSE;

  if (mydebug > 99)
    fprintf (stderr, "battery_get_values_linux_apm()\n");

  // -------------------------------------------- apm
  g_snprintf (fn, sizeof (fn), dir_proc);
  g_strlcat (fn, "/apm", sizeof (fn));
  battery = fopen (fn, "r");
  if (battery != NULL)
    {
      int count = 0;
      if (mydebug > 99)
	fprintf (stderr, "battery_get_values_linux_apm(): APM\n");

      count = fscanf (battery, "%s %s %s %x %s %x %d%% %s %s",
		      b, b, b, bloading, b, &i, blevel, b, b);
      /*     1.16 1.2 0x03 0x01      0x00 0x01 99%      -1  ?    */
      fclose (battery);

      if (9 != count)
	{
	  if (mydebug > 9)
	    fprintf (stderr, "Wrong Number (%d) of values in %s\n",
		     count, fn);
	  ret = FALSE;
	}
      else
	{
	  ret = TRUE;
	}

      /*
       * Bit 7 is set if we have a battery (laptop). If it isn't set,
       * (desktop) then we don't want to display the battery.
       */
      if ((i & 0x80) != 0)
	ret = FALSE;

    };

  if (mydebug > 60)
    fprintf (stderr,
	     "battery_get_values_linux_apm(): p_bat_full: %d, "
	     "p_bat_current:%d\n", p_bat_full, p_bat_current);
  if (p_bat_current != 0)
    *blevel = (int) (((double) p_bat_current / p_bat_full) * 100.0);
  /*       fprintf(stderr,"blevel: %d\n",*blevel); */

  return ret;

}



/* ******************************************************************
 * we try if we have acpi 
 */
static int
battery_get_values_linux_acpi (int *blevel, int *bloading, int *bcharge,
			       int *btime)
{
  FILE *battery = NULL;
  gint e, e1;
  gint ret = FALSE;
  gint p_bat_full = 0;
  gint p_bat_current = 0;
  gint p_bat_rate = 0;
  gint vtemp;
  gchar b[200], t[200], t2[200], t3[200];
  DIR *dir;
  struct dirent *ent;
  char fn[200];
  *bcharge = FALSE;
  *bloading = FALSE;
  gchar line[200];

  g_snprintf (fn, sizeof (fn), dir_proc);
  g_strlcat (fn, "/acpi/battery/", sizeof (fn));
  dir = opendir (fn);
  if (dir != NULL)
    {
      if (mydebug > 99)
	fprintf (stderr, "battery_get_values_linux(): ACPI\n");
      while ((ent = readdir (dir)) != NULL)
	{
	  if (ent->d_name[0] != '.')
	    {
	      // ---------------------- /acpi/.../info
	      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/%s/info",
			  dir_proc, ent->d_name);
	      battery = fopen (fn, "r");
	      if (battery != NULL)
		{
		  if (mydebug > 99)
		    fprintf (stderr,
			     "battery_get_values_linux(): found file %s\n",
			     fn);
		  while (fgets (line, sizeof (line), battery))
		    {
		      if (strcasestr (line, "last full"))
			{
			  sscanf (line, "%s %s %s %s %[^\n]", t, t2, b, t3,
				  b);
			  e1 = sscanf (t3, "\n%d\n", &vtemp);
			  if (e1 == 1)
			    p_bat_full += vtemp;
			  ret = TRUE;
			}
		    }
		  fclose (battery);
		}
	      else
		{
		  if (mydebug > 99)
		    fprintf (stderr,
			     "battery_get_values_linux(): missing File: '%s'\n",
			     fn);
		}


	      // ---------------------- /acpi/.../state
	      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/%s/state",
			  dir_proc, ent->d_name);
	      battery = fopen (fn, "r");
	      if (NULL != battery)
		{
		  if (mydebug > 30)
		    fprintf (stderr,
			     "battery_get_values_linux(): found file %s\n",
			     fn);
		  while (fgets (line, sizeof (line), battery))
		    {
		      if (strcasestr (line, "remaining capacity"))
			{
			  e = sscanf (line,
				      "%s %s %s %s %[^\n]", t, t2, t3, b, b);

			  e1 = sscanf (t3, "\n%d\n", &vtemp);
			  if (e1 == 1)
			    p_bat_current += vtemp;
			  ret = TRUE;
			}

		      if (strcasestr (line, "present rate"))
			{
			  e = sscanf (line, "%s %s %d %s", t, t2, &vtemp, b);
			  p_bat_rate += vtemp;
			}

		      sscanf (line, "%s%[^\n]", t, t2);
		      if (mydebug > 30)
			fprintf (stderr,
				 "battery_get_values_linux(): line: %s",
				 line);
		      if (strstr (t, "Status:"))
			{
			  if (strstr (t2, "on-line"))
			    *bloading = TRUE;
			  else
			    *bloading = FALSE;
			  ret = TRUE;
			}
		      if (strstr (t, "charging"))
			{
			  /*  assume we are charging, unless
			   * discharging or unknown. */
			  *bloading = TRUE;
			  *bcharge = TRUE;
			  if ((strstr (t2, "discharging")) != NULL)
			    {
			      *bcharge = FALSE;
			      *bloading = FALSE;
			    }
			  if ((strstr (t2, "unknown")) != NULL)
			    *bcharge = FALSE;
			  ret = TRUE;
			}
		    }
		}		// if open(...state)
	      else
		{
		  if (mydebug > 99)
		    fprintf (stderr,
			     "battery_get_values_linux(): missing File: '%s'\n",
			     fn);
		}
	    };			// if ent->d_name[0] != '.'
	};
      closedir (dir);

    }

  if (mydebug > 60)
    fprintf (stderr, "battery_get_values_linux(): "
	     "p_bat_full: %d, p_bat_current:%d p_bat_rate: %d\n",
	     p_bat_full, p_bat_current, p_bat_rate);
  if (p_bat_current != 0)
    *blevel = (int) (((double) p_bat_current / p_bat_full) * 100.0);

  if (p_bat_rate != 0)
    {
      *btime = (int) (((double) p_bat_current / p_bat_rate * 60));
    }

  return ret;
}

/* ******************************************************************
 * JH Added temperature readout code here
 */
static int
temperature_get_values_linux (int *temper)
{
  FILE *temperature = NULL;
  gint havetemperature = FALSE;
  DIR *dir;
  struct dirent *ent;
  char fn[200];
  gchar b[200];

  /* search for temperature file */
  temperature = NULL;
  g_snprintf (fn, sizeof (fn), dir_proc);
  g_strlcat (fn, "/acpi/thermal_zone/", sizeof (fn));
  dir = opendir (fn);
  if (dir != NULL)
    {
      while (!havetemperature && (ent = readdir (dir)) != NULL)
	{
	  if (ent->d_name[0] != '.'
	      && strcmp(ent->d_name,"THRS"))
	    {
	      g_snprintf (fn, sizeof (fn), dir_proc);
	      g_strlcat (fn, "/acpi/thermal_zone/", sizeof (fn));
	      g_strlcat (fn, ent->d_name, sizeof (fn));
	      g_strlcat (fn, "/temperature", sizeof (fn));
	      if (mydebug > 30)
		fprintf (stderr, "checking File %s\n", fn);
	      temperature = fopen (fn, "r");
	      if (temperature != NULL)
		{
		  havetemperature = TRUE;
		  // ############### SigSeg Was HERE ??!!
		  int count = fscanf (temperature, "%s %d %s", b,
				      temper, b);
		  if (3 != count)
		    {
		      if (mydebug > 9)
			fprintf (stderr,
				 "Wrong Number (%d) of values in %s\n",
				 count, fn);
		      havetemperature = FALSE;
		    }
		  fclose (temperature);
		}
	    }
	}
      closedir (dir);
    }
  return havetemperature;
}
#endif /* Linux */


/* ******************************************************************
 * Return TRUE on success, FALSE on error.
 */
#if defined(__FreeBSD__) && defined(__i386__)
static int
battery_get_values_fbsd (int *blevel, int *bloading)
{
  int fd;
  struct apm_info ai;

  *blevel = -1;
  *bloading = FALSE;

  if ((fd = open ("/dev/apm", O_RDONLY)) == -1)
    {
      if (mydebug > 30)
	fprintf (stderr, "gpsdrive: open(/dev/apm): %s\n", strerror (errno));
      return FALSE;
    }
  if (ioctl (fd, APMIO_GETINFO, &ai) == -1)
    {
      if (mydebug > 30)
	fprintf (stderr,
		 "gpsdrive: ioctl(APMIO_GETINFO): %s\n", strerror (errno));
      close (fd);
      return FALSE;
    }

  /*
   * Battery level. If unknown or error we fail.
   */
  if (ai.ai_batt_life >= 0 && ai.ai_batt_life <= 100)
    {
      *blevel = ai.ai_batt_life;
    }
  else
    {
      if (ai.ai_batt_life == 255)
	{
	  fprintf (stderr, "gpsdrive: battery level is unknown\n");
	}
      else
	{
	  fprintf (stderr, "gpsdrive: battery level is invalid\n");
	}
      close (fd);
      return FALSE;
    }

  /*
   * Is the battery charging? If unknown or error we fail.
   */
  if (ai.ai_acline == 1)
    {				/* on-line */
      *bloading = TRUE;
    }
  else if (ai.ai_acline == 0)
    {				/* off-line */
      *bloading = FALSE;
    }
  else
    {
      if (ai.ai_acline == 255)
	{			/* unknown */
	  fprintf (stderr, "gpsdrive: battery charging status is unknown\n");
	}
      else
	{			/* error */
	  fprintf (stderr, "gpsdrive: battery charging status is invalid\n");
	}
      close (fd);
      return FALSE;
    }
  close (fd);
  return TRUE;
}
#endif /* __FreeBSD__ && __i386__ */

/* ******************************************************************
 * Return TRUE on success, FALSE on error.
 */
#if defined( __NetBSD__ ) || defined (__OpenBSD__)
static int
battery_get_values_nbsd (int *blevel, int *bloading)
{
  int fd;
  struct apm_power_info ai;

  memset (&ai, 0, sizeof (ai));

  *blevel = -1;
  *bloading = FALSE;

  if ((fd = open ("/dev/apm", O_RDONLY)) == -1)
    {
      if (mydebug > 30)
	fprintf (stderr, "gpsdrive: open(/dev/apm): %s\n", strerror (errno));
      return FALSE;
    }
  if (ioctl (fd, APM_IOC_GETPOWER, &ai) == -1)
    {
      if (mydebug > 30)
	fprintf (stderr,
		 "gpsdrive: ioctl(APM_IOC_GETPOWER): %s\n", strerror (errno));
      close (fd);
      return FALSE;
    }

  /*
   * Battery level. If unknown or error we fail.
   */
  if (ai.battery_life <= 100)
    {
      *blevel = ai.battery_life;
    }
  else
    {
      if (ai.battery_life == 255)
	{
	  fprintf (stderr, "gpsdrive: battery level is unknown\n");
	}
      else
	{
	  fprintf (stderr, "gpsdrive: battery level is invalid\n");
	}
      close (fd);
      return FALSE;
    }

  /*
   * Is the battery charging? If unknown or error we fail.
   */
  if (ai.ac_state == APM_AC_ON)
    {				/* on-line */
      *bloading = TRUE;
    }
  else if (ai.ac_state == APM_AC_OFF)
    {				/* off-line */
      *bloading = FALSE;
    }
  else
    {
      if (ai.ac_state == APM_AC_UNKNOWN)
	{			/* unknown */
	  fprintf (stderr, "gpsdrive: battery charging status is unknown\n");
	}
      else
	{			/* error */
	  fprintf (stderr, "gpsdrive: battery charging status is invalid\n");
	}
      close (fd);
      return FALSE;
    }
  close (fd);
  return TRUE;
}
#endif /* __NetBSD__ */


/* ******************************************************************
 * code to display temperature meter (JH)
 */
int
expose_display_temperature ()
{
  static GdkGC *temkontext = NULL;
  GdkDrawable *mydrawable;

  extern GtkWidget *drawing_temp;
  extern GdkColor mygray;

  extern GdkPixbuf *temimage;
  gint havetemperature = temperature_get_values ();
  if (!havetemperature)
    return FALSE;

  mydrawable = drawing_temp->window;
  if (temkontext == NULL)
    temkontext = gdk_gc_new (mydrawable);
  gdk_gc_set_foreground (temkontext, &mygray);
  gdk_draw_rectangle (mydrawable, temkontext, 1, 0, 0, 25, 50);
  if (temimage == NULL)
    temimage = read_icon ("gauge.png");
  gdk_gc_set_function (temkontext, GDK_AND);
  gdk_draw_pixbuf (mydrawable, temkontext, temimage, 0,
		   0, 0, 0, 17, 50, GDK_RGB_DITHER_NONE, 0, 0);
  gdk_gc_set_function (temkontext, GDK_COPY);
  /*       gdk_pixbuf_unref (temimage); */
  gdk_gc_set_foreground (temkontext, &mygray);
  /* We want to limit cputemp (79<cputemp< 40)    */
  if (cputemp > 79)
    cputemp = 79;
  if (cputemp < 40)
    cputemp = 40;
  gdk_draw_rectangle (mydrawable, temkontext, 1, 6, 1, 5, 79 - cputemp);

  return TRUE;
}

/* ******************************************************************
 * display battery meter
 */
int
expose_display_battery ()
{
  static GdkGC *battkontext = NULL;
  GdkDrawable *mydrawable;
  gchar bbuf[200];

  extern GdkColor mygray;
  extern GdkColor black;
  extern GdkColor green;
  extern GdkColor yellow;
  extern GdkColor orange;
  extern GdkColor red;
  extern GdkPixbuf *batimage;

  extern GdkPixbuf *temimage;

  gint havebattery = battery_get_values ();

  if (!havebattery)
    return FALSE;

  mydrawable = drawing_battery->window;
  if (battkontext == NULL)
    battkontext = gdk_gc_new (mydrawable);

  gdk_gc_set_foreground (battkontext, &mygray);
  gdk_draw_rectangle (mydrawable, battkontext, 1, 0, 0, 25, 50);
  gdk_gc_set_foreground (battkontext, &black);
  gdk_draw_rectangle (mydrawable, battkontext, 0, 19, 0, 6, 50);

  /* JH added limit to batlevel  */
  if (batlevel > 99)
    batlevel = 99;
  if (batlevel > 40)
    gdk_gc_set_foreground (battkontext, &green);
  else
    {
      if (batlevel > 25)
	gdk_gc_set_foreground (battkontext, &yellow);
      else
	{
	  if (batlevel > 15)
	    gdk_gc_set_foreground (battkontext, &orange);
	  else
	    gdk_gc_set_foreground (battkontext, &red);
	}
    }
  gdk_draw_rectangle (mydrawable, battkontext, 1, 20,
		      50 - batlevel / 2, 5, batlevel / 2);

  if (img_powercharges == NULL)
    {
      img_powercharges = read_icon ("powercharges.png");
      img_powercord = read_icon ("powercord.png");
      img_battery = read_icon ("battery.png");
    }

  if (batcharge)
    batimage = img_powercharges;
  else
    {
      if (batloading)
	batimage = img_powercord;
      else
	batimage = img_battery;
    }


  gdk_gc_set_function (battkontext, GDK_AND);
  gdk_draw_pixbuf (mydrawable, battkontext, batimage, 0,
		   0, 0, 0, 17, 50, GDK_RGB_DITHER_NONE, 0, 0);
  gdk_gc_set_function (battkontext, GDK_COPY);

  /*       gdk_pixbuf_unref (batimage); */

  if (((batlevel - 1) / 10 != (batlevel_old - 1) / 10) && (!batloading))
    {
      if (mydebug > 30)
	g_print ("\nBattery: %d%%\n", batlevel);

      /* This is for Festival, so we cannot use gettext() for i18n */
      g_snprintf (bbuf, sizeof (bbuf),
		  speech_remaining_battery[voicelang], batlevel);
      speech_out_speek (bbuf);

      batlevel_old = batlevel;
    }
  return TRUE;
}


/* ******************************************************************
 * Return TRUE on success, FALSE on error.
 */
int
battery_get_values (void)
{
  gint havebattery = FALSE;	/* Battery level and loading flag */

  if (disableapm)
    {
      return FALSE;
    }
#if defined(__linux__)
  havebattery =
    battery_get_values_linux_acpi (&batlevel, &batloading, &batcharge,
				   &battime);
  if (!havebattery)
    havebattery =
      battery_get_values_linux_apm (&batlevel, &batloading, &batcharge);
#elif defined(__FreeBSD__) && defined(__i386__)
  havebattery = battery_get_values_fbsd (&batlevel, &batloading);
#elif defined(__NetBSD__) || defined(__OpenBSD__)
  havebattery = battery_get_values_nbsd (&batlevel, &batloading);
#else
  /* add support for your favourite OS here */
  return FALSE;
#endif

  if (havebattery)
    {
      g_snprintf (batstring, sizeof (batstring), "%s %d%%", "Batt", batlevel);
      if (battime)
	g_snprintf (batstring, sizeof (batstring), "%s %d%%, %d min",
		    "Batt", batlevel, battime);

      if (NULL == tooltip_battery)
	tooltip_battery = gtk_tooltips_new ();

      if (tooltip_battery != NULL && batteventbox != NULL)
	gtk_tooltips_set_tip (GTK_TOOLTIPS
			      (tooltip_battery),
			      batteventbox, batstring, NULL);
    }
  else
    {
      g_snprintf (batstring, sizeof (batstring), "no Battery");
    }

  if (mydebug > 20)
    fprintf (stderr, "battery_get_values() Have battery: %d\n", havebattery);
  if (mydebug > 30)
    fprintf (stderr, "battery_get_values(): batstring %s\n", batstring);

  return havebattery;
}


/* ******************************************************************
 * Return TRUE on success, FALSE on error.
 */
int
temperature_get_values (void)
{
  gint havetemperature = FALSE;

  if (disableapm)
    {
      return havetemperature;
    }

  //  g_snprintf (dir_proc,sizeof(dir_proc),"/proc");

#if defined(__linux__)
  havetemperature = temperature_get_values_linux (&cputemp);
#else
  /* add support for your favourite OS here */
  return FALSE;
#endif

  if (havetemperature)
    {
      g_snprintf (cputempstring, sizeof (cputempstring),
		  "%s %d%sC", "CPU-Temp", cputemp, gradsym);
      if (mydebug > 30)
	fprintf (stderr, "cputempstring %s\n", cputempstring);



      if (NULL == tooltip_temperature)
	tooltip_temperature = gtk_tooltips_new ();

      if (tooltip_temperature != NULL && tempeventbox != NULL)
	gtk_tooltips_set_tip (GTK_TOOLTIPS
			      (tooltip_temperature),
			      tempeventbox, cputempstring, NULL);
    }
  else
    {
      g_snprintf (cputempstring, sizeof (batstring),
		  "no CPU Temperature available");
    }
  if (mydebug > 20)
    fprintf (stderr, "temp: %d\n", havetemperature);

  return havetemperature;

}


/* ******************************************************************
 */
void
create_battery_widget (GtkWidget * hbox2)
{
  if (!battery_get_values ())
    return;

  if (NULL == drawing_battery)
    {
      GtkWidget *alignment3;

      drawing_battery = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_battery), 27, 52);
      frame_battery = gtk_frame_new (_("Bat."));
      batteventbox = gtk_event_box_new ();
      gtk_container_add (GTK_CONTAINER (batteventbox), drawing_battery);
      alignment3 = gtk_alignment_new (0.5, 0.5, 0, 0);
      gtk_container_add (GTK_CONTAINER (alignment3), batteventbox);
      gtk_container_add (GTK_CONTAINER (frame_battery), alignment3);
      gtk_box_pack_start (GTK_BOX (hbox2),
			  frame_battery, FALSE, FALSE, 1 * PADDING);
      if (battery_get_values ())
	gtk_signal_connect (GTK_OBJECT
			    (drawing_battery),
			    "expose_event",
			    GTK_SIGNAL_FUNC (expose_display_battery), NULL);

    }
}

/* ******************************************************************
 */
void
create_temperature_widget (GtkWidget * hbox2)
{
  if (!temperature_get_values ())
    return;

  if (mydebug > 20)
    fprintf (stderr, "create_temperature_widget:\n");

  /* drawing area for cpu temp meter */
  if (NULL == drawing_temp)
    {
      GtkWidget *alignment4;

      drawing_temp = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_temp), 15, 52);
      frame_temperature = gtk_frame_new (_("TC"));
      tempeventbox = gtk_event_box_new ();
      gtk_container_add (GTK_CONTAINER (tempeventbox), drawing_temp);
      alignment4 = gtk_alignment_new (0.5, 0.5, 0, 0);
      gtk_container_add (GTK_CONTAINER (alignment4), tempeventbox);
      gtk_container_add (GTK_CONTAINER (frame_temperature), alignment4);
      gtk_box_pack_start (GTK_BOX (hbox2),
			  frame_temperature, FALSE, FALSE, 1 * PADDING);
      if (temperature_get_values ())
	gtk_signal_connect (GTK_OBJECT (drawing_temp),
			    "expose_event",
			    GTK_SIGNAL_FUNC
			    (expose_display_temperature), NULL);


    };

}
