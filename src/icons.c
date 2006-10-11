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


***********************************************************************/

/*  Include Dateien */
#include "../config.h"
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <semaphore.h>
#include <gpsdrive_config.h>

#include "gettext.h"

#include "LatLong-UTMconversion.h"
#include "gpsdrive.h"
#include "battery.h"
#include "track.h"
#include "poi.h"
#include "icons.h"

#ifndef NOPLUGINS
#include "gmodule.h"
#endif

#ifdef USETELEATLAS
#include "gpsnavlib.h"
#endif

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


#define	MAX_ICONS	MAXWPTYPES

extern gint do_unit_test;
extern gint debug;
extern gint mydebug;
extern gint muteflag, wp_from_sql, trackflag;
extern GdkColor red;
extern GdkColor black;
extern GdkColor white;
extern GdkColor blue;
extern poi_type_struct poi_type_list[poi_type_list_max];
extern int poi_type_list_count;


GdkPixbuf *friendsimage = NULL;
GdkPixbuf *friendspixbuf = NULL;
GdkPixbuf *kismetpixbuf;

icons_buffer_struct icons_buffer[MAX_ICONS];
gint icons_buffer_max = MAX_ICONS;
gint icons_buffer_last = 0;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


/* *****************************************************************************
 * draw a + Sign and its shaddow */
void
draw_plus_sign (gdouble posxdest, gdouble posydest)
{
  gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
  if (shadow)
    {				/*  draw shadow of + sign */
      gdk_gc_set_foreground (kontext, &darkgrey);
      gdk_gc_set_function (kontext, GDK_AND);
      gdk_draw_line (drawable, kontext,
		     posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 - 5 + SHADOWOFFSET,
		     posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 + 5 + SHADOWOFFSET);
      gdk_draw_line (drawable, kontext,
		     posxdest + 1 + 5 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET,
		     posxdest + 1 - 5 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET);
      gdk_gc_set_function (kontext, GDK_COPY);
    }

  /*  draw + sign at destination */
  gdk_gc_set_foreground (kontext, &red);
  gdk_draw_line (drawable, kontext,
		 posxdest + 1, posydest + 1 - 5,
		 posxdest + 1, posydest + 1 + 5);
  gdk_draw_line (drawable, kontext,
		 posxdest + 1 + 5, posydest + 1,
		 posxdest + 1 - 5, posydest + 1);

}

/* *****************************************************************************
 * draw a small + Sign and its shaddow */
void
draw_small_plus_sign (gdouble posxdest, gdouble posydest)
{
  gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
  if (shadow)
    {				/*  draw shadow of + sign */
      gdk_gc_set_foreground (kontext, &darkgrey);
      gdk_gc_set_function (kontext, GDK_AND);
      gdk_draw_line (drawable, kontext,
		     posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 - 2 + SHADOWOFFSET,
		     posxdest + 1 + SHADOWOFFSET,
		     posydest + 1 + 2 + SHADOWOFFSET);
      gdk_draw_line (drawable, kontext,
		     posxdest + 1 + 2 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET,
		     posxdest + 1 - 2 + SHADOWOFFSET,
		     posydest + 1 + SHADOWOFFSET);
      gdk_gc_set_function (kontext, GDK_COPY);
    }

  /*  draw + sign at destination */
  gdk_gc_set_foreground (kontext, &red);
  gdk_draw_line (drawable, kontext,
		 posxdest + 1, posydest + 1 - 2,
		 posxdest + 1, posydest + 1 + 2);
  gdk_draw_line (drawable, kontext,
		 posxdest + 1 + 2, posydest + 1,
		 posxdest + 1 - 2, posydest + 1);

}


/* -----------------------------------------------------------------------------
 * 
*/
int
drawicon (gint posxdest, gint posydest, char *icon_name)
{
  int symbol = 0, aux = -1, i;
  int wx, wy;
  gchar icon[80];

  //printf("drawicon %d %d %s\n", posxdest,  posydest,icon_name);

  g_strlcpy (icon, icon_name, sizeof (icon));
  if (!wp_from_sql)
    g_strup (icon);

  /* sweep through all icons and look for icon */
  for (i = 0; i < icons_buffer_last; i++)
    if ((strcmp (icon, icons_buffer[i].name)) == 0)
      {
	if ((posxdest >= 0) && (posxdest < SCREEN_X)
	    && (posydest >= 0) && (posydest < SCREEN_Y))
	  {
	    wx = gdk_pixbuf_get_width (icons_buffer[i].icon);
	    wy = gdk_pixbuf_get_height (icons_buffer[i].icon);
	    gdk_draw_pixbuf (drawable, kontext,
			     (icons_buffer + i)->icon, 0, 0,
			     posxdest - wx / 2, posydest - wy / 2,
			     wx, wy, GDK_RGB_DITHER_NONE, 0, 0);
	    aux = i;
	  }
	return 99999;
      }

  if (symbol == 0)
    {
      draw_plus_sign (posxdest, posydest);
      return 0;
    }

  return symbol;
}

/* -----------------------------------------------------------------------------
 * load icon into pixbuff from either system directory or user directory
 * if force is set we exit on non success
*/
GdkPixbuf *
read_icon (gchar * icon_name, int force)
{
  gchar filename[1024];
  gchar icon_filename[1024];
  GdkPixbuf *icons_buffer = NULL;
  if (mydebug > 50)
    printf ("read_icon(%s,%d)\n", icon_name,force);

  typedef struct
  {
    gchar *path;
    gchar *option;
  } path_definition;
  path_definition available_path[] = {
    {"", NULL},
    {"./data/icons/", NULL},
    {"../data/icons/", NULL},
    {"./data/pixmaps/", NULL},
    {"../data/pixmaps/", NULL},
    {"%spixmaps/", (gchar *) local_config_homedir},
    {"%sicons/", (gchar *) local_config_homedir},
    {"%s/gpsdrive/icons/", (gchar *) DATADIR},
    {"%s/gpsdrive/pixmaps/", (gchar *) DATADIR},
    {"%s/gpsdrive/icons/", "/usr/share"},
    {"%s/gpsdrive/pixmaps/", "/usr/share"},
    {"END", NULL}
  };

  gint i;
  for (i = 0;
       strncmp (available_path[i].path, "END",
		sizeof (available_path[i].path)); i++)
    {
      g_snprintf (filename, sizeof (filename), available_path[i].path,
		  available_path[i].option);
      g_snprintf (icon_filename, sizeof (icon_filename), "%s%s", filename,
		  icon_name);
      if (mydebug > 75)
	printf ("read_icon(%s): Try\t%s\n", icon_name, icon_filename);
      icons_buffer = gdk_pixbuf_new_from_file (icon_filename, NULL);
      if (NULL != icons_buffer)
	{
	  if (mydebug > 20)
	    printf ("read_icon(%s): FOUND\t%s\n", icon_name, icon_filename);
	  return icons_buffer;
	}
    }

  if ( NULL == icons_buffer && force ) {
      fprintf (stderr,"read_icon: No Icon '%s' found\n", icon_name);
      if ( strstr (icon_name, "Old") == NULL ) {
	  fprintf (stderr,
		   _("Please install the program as root with:\n"
		     "make install\n\n"));
      
	  fprintf(stderr,"I searched in :\n");
	  for (i = 0;
	       strncmp (available_path[i].path, "END",
			sizeof (available_path[i].path)); i++)
	      {
		  g_snprintf (filename, sizeof (filename), available_path[i].path,
			      available_path[i].option);
		  g_snprintf (icon_filename, sizeof (icon_filename), "%s%s", filename,
			      icon_name);
		  fprintf(stderr,"\t%s\n",icon_filename);
	      }

	  exit (-1);
      }
  }

  return icons_buffer;
}


/* *******************************************************
 * read icon in the selected theme.
 * this includes reducing the icon path if this icon is not found 
 * until the first parent icon is found
 */

GdkPixbuf *
read_themed_icon (gchar * icon_name)
{
  gchar themed_icon_filename[2048];
  gchar icon_file_name[2048];
  GdkPixbuf *icon = NULL;

  g_strlcpy (icon_file_name, icon_name, sizeof (icon_file_name));
  g_strdelimit (icon_file_name, ".", '/');

  char *p_pos;
  do
    {
      g_snprintf (themed_icon_filename, sizeof (themed_icon_filename),
		  "%s/%s.png", local_config.icon_theme, icon_file_name);
      if (mydebug > 90)
	fprintf (stderr, "read_themed_icon(%s) => Themed File %s\n",
		 icon_name, themed_icon_filename);
      icon = read_icon (themed_icon_filename,0);
      if (icon != NULL)
	return icon;

      p_pos = rindex (icon_file_name, '/');
      if (p_pos)
	{
	  p_pos[0] = '\0';
	  if (mydebug > 3)
	    fprintf (stderr, "read_themed_icon(%s) reduced to => %s\n",
		     icon_name, icon_file_name);
	}
    }
  while (p_pos != NULL);
  if ( NULL == icon ) {
      fprintf (stderr,"read_themed_icon: No Icon '%s' found for theme %s\n", 
	       icon_name,local_config.icon_theme);
      //exit (-1);
  }
  return NULL;
}



/* -----------------------------------------------------------------------------
 * load icons specified in icons.txt for non sql users
*/
void
load_icons (void)
{
  /* icons.txt */
  FILE *fh_icons_txt = NULL;
  char filename[255];

  /* hardcoded kismet-stuff */
  kismetpixbuf = read_icon ("kismet.png",1);

  /* load icons defined in icons.txt */
  if (!fh_icons_txt)
    {
      snprintf ((char *) &filename, sizeof (filename), "./data/icons.txt");
      fh_icons_txt = fopen (filename, "r");
      if (mydebug > 3)
	{
	  printf ("load_icons(): Trying icons.txt: \"%s\"\n", filename);
	}
    }
  if (!fh_icons_txt)
    {
      snprintf ((char *) &filename, sizeof (filename), "%s/icons.txt",
		local_config_homedir);
      fh_icons_txt = fopen (filename, "r");
      if (mydebug > 3)
	{
	  printf ("load_icons(): Trying icons.txt: \"%s\"\n", filename);
	}
    }
  /* if there is no icons.txt, try to open it  from datadir */
  if (!fh_icons_txt)
    {
      snprintf ((char *) &filename, 255, "%s/gpsdrive/icons.txt", DATADIR);
      if (mydebug > 3)
	{
	  printf ("load_icons(): Trying default icons.txt: \"%s\"\n",
		  filename);
	}
      fh_icons_txt = fopen (filename, "r");
    }
  if (fh_icons_txt)
    {
      if (mydebug > 3)
	printf ("load_icons(): Icons will be read from: \"%s\"\n", filename);

      int index = 0;
      int errors = 0;
      while (fscanf (fh_icons_txt, "%s %d %d\n",
		     (char *) &poi_type_list[index].name,
		     &poi_type_list[index].scale_min,
		     &poi_type_list[index].scale_max))
	{

	  if (index >= MAX_ICONS)
	    {
	      break;
	    }

	  if (poi_type_list_count < poi_type_list_max)
	    {
	      poi_type_list_count++;
	      poi_type_list[poi_type_list_count].poi_type_id = index;
	      poi_type_list[poi_type_list_count].icon =
		read_themed_icon (poi_type_list[index].icon_name);
	    }


	  if (mydebug > 3)
	    printf ("load_icons(): Waypoint-type %d gets \"%s\"\n", index,
		    poi_type_list[index].name);
	  icons_buffer[index].icon =
	    read_themed_icon (poi_type_list[index].icon_name);
	  if (!icons_buffer[index].icon)	// None Found
	    {
	      if (do_unit_test)
		{
		  if (!strncmp ((char *) icons_buffer[index].name, "Old.", 4))
		    {
		      printf ("load_icons(): ** Failed to open \"%s\"\n",
			      (char *) &poi_type_list[index].name);
		      errors++;
		    }
		  else
		    {
		      printf ("load_icons(): Not opening \"%s\"\n",
			      (char *) &poi_type_list[index].name);
		      icons_buffer[index].icon = read_icon ("unknown.png",1);
		    }
		}
	      else
		{
		  printf ("load_icons(): ** Failed to open \"%s\"\n",
			  (char *) &poi_type_list[index].name);
		  icons_buffer[index].icon = read_icon ("unknown.png",1);
		}
	    }
	  if (!icons_buffer[index].icon)	// None Found
	    {
	      icons_buffer[index].icon = NULL;
	      icons_buffer[index].name[0] = 0;
	    }
	  if (feof (fh_icons_txt))
	    break;
	  index++;

	}
      if (errors)
	{
	  exit (-1);
	}
      fclose (fh_icons_txt);
    }
}



/* -----------------------------------------------------------------------------
*/
void
load_friends_icon (void)
{

  friendsimage =  read_icon("friendsicon.png",1);
  friendspixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 1, 8, 39, 24);
  gdk_pixbuf_scale (friendsimage, friendspixbuf, 0, 0, 39, 24,
		    0, 0, 1, 1, GDK_INTERP_BILINEAR);

}

/* ----------------------------------------------------------------------------- */
/* warning: still modifies icon_name 
 */
void
load_user_icon (char icon_name[200])
{
  int i;
  char path[1024];
  for (i = 0; i < (int) strlen (icon_name); i++)
    icon_name[i] = tolower (icon_name[i]);

  g_snprintf (path, sizeof (path), "%sicons/%s.png", local_config_homedir, icon_name);
  icons_buffer[icons_buffer_last].icon =
    gdk_pixbuf_new_from_file (path, NULL);

  if (icons_buffer[icons_buffer_last].icon == NULL)
    {
      g_snprintf (path, sizeof (path), "%s/gpsdrive/icons/%s.png",
		  DATADIR, icon_name);
      icons_buffer[icons_buffer_last].icon =
	gdk_pixbuf_new_from_file (path, NULL);
    }

  if ((icons_buffer + icons_buffer_last)->icon != NULL)
    {
      for (i = 0; i < (int) strlen (icon_name); i++)
	icon_name[i] = tolower (icon_name[i]);
      if ((strcmp (icon_name, "wlan") == 0)
	  || (strcmp (icon_name, "wlan-wep") == 0))
	{
	  fprintf (stderr, _("Loaded user defined icon %s\n"), path);
	}
      else
	{
	  g_strlcpy ((icons_buffer + icons_buffer_last)->name, icon_name,
		     sizeof (icons_buffer->name));
	  fprintf (stderr, _("Loaded user defined icon %s\n"), path);
	  icons_buffer_last++;
	}
      if (mydebug > 3)
	printf ("Icon for %s loaded:%s\n", icon_name, path);
    }
  else
    {
      if (mydebug > 3)
	printf ("No Icon for %s loaded\n", icon_name);
    }
}


/* *****************************************************************************
 * draw wlan Waypoints
 */
void
drawwlan (gint posxdest, gint posydest, gint wlan)
{
  /*  wlan=0: no wlan, 1:open wlan, 2:WEP crypted wlan */

  if (wlan == 0)
    return;

  if ((posxdest >= 0) && (posxdest < SCREEN_X))
    {
      if ((posydest >= 0) && (posydest < SCREEN_Y))
	{
	  if (wlan == 1)
	    drawicon (posxdest, posydest, "w-lan.open");
	  else
	    drawicon (posxdest, posydest, "w-lan.wep");
	}
    }
}
