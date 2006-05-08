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
extern int poi_type_list_count ;


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
  int  wx, wy;
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
	      wx = gdk_pixbuf_get_width  (icons_buffer[i].icon);
	      wy = gdk_pixbuf_get_height (icons_buffer[i].icon);
	      gdk_draw_pixbuf (drawable, kontext,
			       (icons_buffer + i)->icon, 0, 0,
			       posxdest - wx / 2, posydest - wy / 2, 
			       wx, wy,
			       GDK_RGB_DITHER_NONE, 0, 0);
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
*/
GdkPixbuf *
read_icon (gchar * icon_name)
{
  gchar filename[255];
  GdkPixbuf *icons_buffer;
  if (mydebug > 50)
    printf ("read_icon(%s)\n", icon_name);

  // Try complete Path
  icons_buffer = gdk_pixbuf_new_from_file (icon_name, NULL);


  if ( ! ( strcasestr(icon_name,".gif") || strcasestr(icon_name,".png") ) ) 
      {
	  if ( !icons_buffer ) // Try as .png
	      {
		  g_snprintf (filename, sizeof (filename), "%s.png",icon_name);
		  icons_buffer = read_icon (filename);
	      }
	  if ( !icons_buffer ) // Try as .gif
	      {
		  g_snprintf (filename, sizeof (filename), "%s.gif",icon_name);
		  icons_buffer = read_icon (filename);
	      }
      }  
  
  if (!icons_buffer)		// Try in ./data/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "./data/pixmaps/%s",
		  icon_name);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if (mydebug > 98)
	  printf ("read_icon(%s): try in %s\n", icon_name,filename);
    }
  if (!icons_buffer)		// Try in ./data/icons
    {
      g_snprintf (filename, sizeof (filename), "./data/icons/%s",
		  icon_name);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if (mydebug > 98)
	  printf ("read_icon(%s): try in %s\n", icon_name,filename);
    }
  if (!icons_buffer)		// Try in ./data/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "./data/pixmaps/%s",
		   icon_name);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if (mydebug > 98)
	  printf ("read_icon(%s): try in %s\n", icon_name,filename);
    }
  if (!icons_buffer)		// Try in Homedir/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "%spixmaps/%s",
		  homedir, icon_name);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if (mydebug > 98)
	  printf ("read_icon(%s): try in %s\n", icon_name,filename);
    }
  if (!icons_buffer)		// Try in homedir/icons
    {
      g_snprintf (filename, sizeof (filename), "%sicons/%s",
		  homedir, icon_name);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if (mydebug > 98)
	  printf ("read_icon(%s): try in %s\n", icon_name,filename);
    }


  if (!icons_buffer)		// Try in DATADIR/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "%s/gpsdrive/pixmaps/%s",
		  DATADIR, icon_name);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if (mydebug > 98)
	  printf ("read_icon(%s): try in %s\n", icon_name,filename);
    }
  if (!icons_buffer)		// Try in DATADIR/icons
    {
      g_snprintf (filename, sizeof (filename), "%s/gpsdrive/icons/%s",
		  DATADIR, icon_name);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if (mydebug > 98)
	  printf ("read_icon(%s): try in %s\n", icon_name,filename);
    }

  if (!icons_buffer)		// None Found
    {
      fprintf (stderr, "** Failed to open \"%s\"\n", icon_name);
      g_snprintf (filename, sizeof (filename),
		  "%s/gpsdrive/icons/unknown.png", DATADIR);
      icons_buffer = gdk_pixbuf_new_from_file (filename, NULL);
      if ( do_unit_test ) {
	  exit(-1);
      }
    }
  else
    {
      if (mydebug > 5)
	printf ("read_icon(%s)\t --> %s OK\n", icon_name, filename);
    }

  return icons_buffer;
}


/* -----------------------------------------------------------------------------
 * load icons specified in icons.txt for non sql useers
*/
void
load_icons (void)
{
  /* icons.txt */
  FILE *fh_icons_txt = NULL;
  char filename[255], wptype[64], icon_name[1024];
  int iconcounter;

  /* hardcoded kismet-stuff */
  kismetpixbuf = read_icon ("kismet.png");

  /* load icons defined in icons.txt */
  if (!fh_icons_txt)
    {
	snprintf ((char *) &filename, sizeof (filename), "./data//icons.txt");
	fh_icons_txt = fopen (filename, "r");
	if (mydebug > 3)
	    {
		printf ("Trying icons.txt: \"%s\"\n", filename);
	    }
    }
  if (!fh_icons_txt)
    {
	snprintf ((char *) &filename, sizeof (filename), "%s/icons.txt", homedir);
	fh_icons_txt = fopen (filename, "r");
	if (mydebug > 3)
	    {
		printf ("Trying icons.txt: \"%s\"\n", filename);
	    }
    }
  /* if there is no icons.txt, try to open it  from datadir */
  if (!fh_icons_txt)
    {
      snprintf ((char *) &filename, 255, "%s/gpsdrive/icons.txt", DATADIR);
      if (mydebug > 3)
	{
	  printf ("Trying default icons.txt: \"%s\"\n", filename);
	}
      fh_icons_txt = fopen (filename, "r");
    }
  if (fh_icons_txt)
    {
      if (mydebug > 3)
	printf ("Icons will be read from: \"%s\"\n", filename);

      for (iconcounter = 0; iconcounter < MAX_ICONS; iconcounter++)
	{
	  fscanf (fh_icons_txt, "%s %s\n", (char *) &wptype, (char *) &icon_name);
	  if (mydebug > 3)
	    printf ("Waypoint-type %d \"%s\" gets \"%s\"\n", iconcounter,
		    wptype, (char *) &icon_name);
	  icons_buffer[iconcounter].icon = read_icon (icon_name);
	  if ( ! icons_buffer[iconcounter].icon )	// None Found
	    {
	      printf ("** Failed to open \"%s\"\n", (char *) &icon_name);
	      icons_buffer[iconcounter].icon = NULL;
	      icons_buffer[iconcounter].name[0] = 0;
	      if ( do_unit_test ) {
		  exit (-1);
	      }
	    }
	  else
	    {
		if(  poi_type_list_count < poi_type_list_max ) {
		    poi_type_list_count++;
		    strncpy ((char *) &poi_type_list[poi_type_list_count].name, wptype,
			     sizeof (poi_type_list[poi_type_list_count].name));
		    strncpy ((char *) &poi_type_list[poi_type_list_count].icon_name, icon_name,
			     sizeof (poi_type_list[poi_type_list_count].icon_name));
		    poi_type_list[poi_type_list_count].icon = icons_buffer[iconcounter].icon;
		    poi_type_list[poi_type_list_count].poi_type_id = iconcounter;
		}
	    }
	  if (feof (fh_icons_txt))
	    break;
	}
      fclose (fh_icons_txt);
    }
}

/* -----------------------------------------------------------------------------

*/
void
load_friends_icon (void)
{
  gchar mappath[400];

  g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/%s", DATADIR,
	      "pixmaps/friendsicon.png");
  friendsimage = gdk_pixbuf_new_from_file (mappath, NULL);
  if (friendsimage == NULL)
    {
      friendsimage =
	gdk_pixbuf_new_from_file ("pixmaps/friendsicon.png", NULL);
    }
  if (friendsimage == NULL)
    {
      GString *error;
      error = g_string_new (NULL);
      g_string_sprintf (error, "\n%s\n%s\n",
			_(" Friendsicon could not be loaded:"), mappath);
      fprintf (stderr,
	       _
	       ("\nWarning: unable to load friendsicon!\nPlease install the program as root with:\nmake install\n\n"));

      error_popup ((gpointer *) error->str);
      g_string_free (error, TRUE);
    }
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

  g_snprintf (path, sizeof (path), "%sicons/%s.png", homedir, icon_name);
  icons_buffer[icons_buffer_last].icon = gdk_pixbuf_new_from_file (path, NULL);

  if (icons_buffer[icons_buffer_last].icon == NULL)
    {
      g_snprintf (path, sizeof (path), "%s/gpsdrive/icons/%s.png",
		  DATADIR, icon_name);
      icons_buffer[icons_buffer_last].icon = gdk_pixbuf_new_from_file (path, NULL);
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
