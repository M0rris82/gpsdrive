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
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#if GTK_MINOR_VERSION < 2
#define gdk_draw_pixbuf _gdk_draw_pixbuf
#endif

#include <speech_out.h>
#include <speech_strings.h>

extern gint mydebug;
extern gint debug;

/* Global Values */
gchar cputempstring[20], batstring[20];


/* --------------------*/
gint batlevel, batlevel_old = 125;	/* battery level, range 0..100 */
gint batloading, batloading_old = FALSE;	/* is the battery charging? */
gint batcharge = FALSE;
gint cputemp;


static gchar gradsym[] = "\xc2\xb0";
extern GtkWidget *tempeventbox, *batteventbox;
extern GtkTooltips *temptooltips;
static GdkPixbuf *img_powercharges = NULL, *img_powercord =
  NULL, *img_battery = NULL;

#ifdef __linux__
/*  ******************************************************************
 * Return TRUE on success, FALSE on error.
 */
static int
battery_get_values_linux (int *blevel, int *bloading, int *bcharge)
{
  FILE *battery = NULL;
  gint i, e, e1;
  gint ret = FALSE;
  gint v1, v2, vtemp;
  gchar b[200], t[200], t2[200], t3[200];
  DIR *dir;
  struct dirent *ent;
  struct stat buf;
  char fn[200];
  *bcharge = FALSE;
  *bloading = FALSE;

  ret = FALSE;
  battery = fopen ("/proc/apm", "r");
  if (battery != NULL)
    {
      int count = 0;
      if (mydebug > 99)
	fprintf (stderr, "APM\n");

      count = fscanf (battery, "%s %s %s %x %s %x %d%% %s %s",
		      b, b, b, bloading, b, &i, blevel, b, b);
      /*     1.16 1.2 0x03 0x01      0x00 0x01 99%      -1  ?    */
      fclose (battery);

      if (9 != count)
	{
	  if (mydebug > 99)
	    fprintf (stderr, "Wrong Number (%d) of values in /proc/apm\n",
		     count);
	  return FALSE;
	}

      /*
       * Bit 7 is set if we have a battery (laptop). If it isn't set,
       * (desktop) then we don't want to display the battery.
       */
      if ((i & 0x80) != 0)
	ret = FALSE;

      return ret;
    }
  else
    {
      /* we try if we have acpi */
      v1 = v2 = 0;

      battery = fopen ("/proc/acpi/battery/0/info", "r");
      if (battery == NULL)
	{
	  if (mydebug > 99)
	    fprintf (stderr, "ACPI\n");

	  /* search for info file */
	  dir = opendir ("/proc/acpi/battery/");
	  if (dir == NULL)
	    {
	      ret = FALSE;
	      return ret;
	    }
	  while ((ent = readdir (dir)) != NULL)
	    {
	      if (ent->d_name[0] != '.')
		{
		  g_snprintf (fn, sizeof (fn), "/proc/acpi/battery/");
		  g_strlcat (fn, ent->d_name, sizeof (fn));
		  g_strlcat (fn, "/info", sizeof (fn));
		  if (mydebug > 99)
		    fprintf (stderr, "ACPI: File %s\n", fn);
		  stat (fn, &buf);
		  if (S_ISREG (buf.st_mode) == TRUE)
		    {
		      if (mydebug > 30)
			fprintf (stderr, "\nfound file %s\n", fn);
		      battery = fopen (fn, "r");
		      if (battery != NULL)
			do
			  {
			    e = fscanf
			      (battery,
			       "%s %s %s %s %[^\n]", t, t2, b, t3, b);
			    if (e != EOF)
			      {
				if (((strstr (t, "ast")) != NULL)
				    && ((strstr (t2, "ull")) != NULL))
				  {
				    e1 = sscanf (t3, "\n%d\n", &vtemp);
				    if (e1 == 1)
				      v1 += vtemp;
				    ret = TRUE;
				  }
			      }
			  }
			while (e != EOF);
		      if (battery != NULL)
			fclose (battery);

		    }
		}
	    }
	  closedir (dir);
	}

      /*       v1 = 3400; */

      battery = fopen ("/proc/acpi/battery/0/status", "r");
      if (battery == NULL)
	{
	  /* search for info file */
	  dir = opendir ("/proc/acpi/battery/");
	  if (dir == NULL)
	    {
	      ret = FALSE;
	      return ret;
	    }
	  while ((ent = readdir (dir)) != NULL)
	    {
	      if (ent->d_name[0] != '.')
		{
		  g_snprintf (fn, sizeof (fn), "/proc/acpi/battery/");
		  g_strlcat (fn, ent->d_name, sizeof (fn));
		  g_strlcat (fn, "/state", sizeof (fn));
		  stat (fn, &buf);
		  if (S_ISREG (buf.st_mode) == TRUE)
		    {
		      if (mydebug > 30)
			fprintf (stderr, "\nfound file %s\n", fn);
		      battery = fopen (fn, "r");
		      if (battery == NULL)
			{
			  closedir (dir);
			  ret = FALSE;
			  return ret;
			}

		      do
			{
			  e = fscanf (battery,
				      "%s %s %s %s %[^\n]", t, t2, t3, b, b);
			  if (e != EOF)
			    {
			      if (((strstr
				    (t,
				     "emaining"))
				   != NULL)
				  && ((strstr (t2, "apacity")) != NULL))
				{
				  e1 = sscanf (t3, "\n%d\n", &vtemp);
				  if (e1 == 1)
				    v2 += vtemp;
				  ret = TRUE;
				}
			    }
			}
		      while (e != EOF);
		      fseek (battery, 0, SEEK_SET);
		      do
			{
			  e = fscanf (battery, "%s%[^\n]", t, t2);
			  if (mydebug > 30)
			    fprintf (stderr, "t: %s, t2: %s\n", t, t2);
			  if ((strstr (t, "Status:")) != NULL)
			    {
			      if ((strstr (t2, "on-line")) != NULL)
				*bloading = TRUE;
			      else
				*bloading = FALSE;
			      ret = TRUE;
			    }
			  if ((strstr (t, "charging")) != NULL)
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
		      while (e != EOF);
		      fclose (battery);

		    }
		}
	    }
	  closedir (dir);

	}

      if (mydebug > 60)
	fprintf (stderr, "v1: %d, v2:%d\n", v1, v2);
      if (v2 != 0)
	*blevel = (int) (((double) v2 / v1) * 100.0);
      /*       fprintf(stderr,"blevel: %d\n",*blevel); */
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
  struct stat buf;
  char fn[200];
  gchar b[200];

  /* search for temperature file */
  temperature = NULL;
  dir = opendir ("/proc/acpi/thermal_zone/");
  if (dir != NULL)
    {
      while ((ent = readdir (dir)) != NULL)
	{
	  if (ent->d_name[0] != '.')
	    {
	      g_snprintf (fn, sizeof (fn), "/proc/acpi/thermal_zone/");
	      g_strlcat (fn, ent->d_name, sizeof (fn));
	      g_strlcat (fn, "/temperature", sizeof (fn));
	      stat (fn, &buf);
	      if (S_ISREG (buf.st_mode) == TRUE)
		{
		  if (mydebug > 30)
		    fprintf (stderr, "\nfound file %s\n", fn);
		  temperature = fopen (fn, "r");
		  if (temperature != NULL)
		    havetemperature = TRUE;
		}
	    }
	}
      closedir (dir);
    }
  if (havetemperature)
    {
      // ############### SigSeg HERE ??!!
      fscanf (temperature, "%s %d %s", b, temper, b);
      fclose (temperature);
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
  if (! havetemperature)
      return FALSE;

  mydrawable = drawing_temp->window;
  if (temkontext == NULL)
      temkontext = gdk_gc_new (mydrawable);
  gdk_gc_set_foreground (temkontext, &mygray);
  gdk_draw_rectangle (mydrawable, temkontext, 1, 0, 0, 25, 50);
  if (temimage == NULL)
      temimage = read_icon ("gauge.png");
  gdk_gc_set_function (temkontext, GDK_AND);
  gdk_draw_pixbuf (mydrawable, temkontext, temimage, 0, 0, 0, 0,
		   17, 50, GDK_RGB_DITHER_NONE, 0, 0);
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

  extern GtkWidget *drawing_battery, *drawing_temp;
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
  gdk_draw_pixbuf (mydrawable, battkontext, batimage, 0, 0, 0,
		   0, 17, 50, GDK_RGB_DITHER_NONE, 0, 0);
  gdk_gc_set_function (battkontext, GDK_COPY);

  /*       gdk_pixbuf_unref (batimage); */

  if (((batlevel - 1) / 10 != (batlevel_old - 1) / 10) && (!batloading))
  {
    if (mydebug > 30)
      g_print ("\nBattery: %d%%\n", batlevel);

    /* This is for Festival, so we cannot use gettext() for i18n */
    g_snprintf (bbuf, sizeof (bbuf), speech_remaining_battery[voicelang],
		batlevel);
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
  int ret;
  gint havebattery = FALSE;	/* Battery level and loading flag */

  if (disableapm)
    {
      return FALSE;
    }
#if defined(__linux__)
  havebattery = battery_get_values_linux (&batlevel, &batloading, &batcharge);
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
      if (temptooltips != NULL)
	gtk_tooltips_set_tip (GTK_TOOLTIPS (temptooltips),
			      batteventbox, batstring, NULL);
      if (mydebug > 30)
	fprintf (stderr, "batstring %s\n", batstring);
    }

  if (mydebug > 20)
    fprintf (stderr, "batt: %d\n", havebattery);

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
      return FALSE;
    }

#if defined(__linux__)
  havetemperature = temperature_get_values_linux (&cputemp);
#else
  /* add support for your favourite OS here */
  return FALSE;
#endif

  if (havetemperature)
    {
      g_snprintf (cputempstring, sizeof (cputempstring), "%s %d%sC",
		  "CPU-Temp", cputemp, gradsym);
      if (temptooltips != NULL)
	gtk_tooltips_set_tip (GTK_TOOLTIPS (temptooltips),
			      tempeventbox, cputempstring, NULL);
      if (mydebug > 30)
	fprintf (stderr, "cputempstring %s\n", cputempstring);
    }

  if (mydebug > 20)
    fprintf (stderr, "temp: %d\n", havetemperature);

  return havetemperature;

}
