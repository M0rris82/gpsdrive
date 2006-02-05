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

    *********************************************************************

$Log$
Revision 1.20  2006/02/05 16:38:06  tweety
reading floats with scanf looks at the locale LANG=
so if you have a locale de_DE set reading way.txt results in clearing the
digits after the '.'
For now I set the LC_NUMERIC always to en_US, since there we have . defined for numbers

Revision 1.19  2006/01/04 19:19:31  tweety
more unit tests
search for icons in the local directory data/icons and data/pixmaps first

Revision 1.18  2006/01/03 14:24:10  tweety
eliminate compiler Warnings
try to change all occurences of longi -->lon, lati-->lat, ...i
use  drawicon(posxdest,posydest,"w-lan.open") instead of using a seperate variable
rename drawgrid --> do_draw_grid
give the display frames usefull names frame_lat, ...
change handling of WP-types to lowercase
change order for directories reading icons
always read inconfile

Revision 1.17  1994/06/07 11:25:45  tweety
set debug levels more detailed

Revision 1.16  2005/08/15 13:10:31  tweety
*** empty log message ***

Revision 1.15  2005/08/15 12:59:34  tweety
use gchar/g_sprintf for filename

Revision 1.14  2005/08/14 18:46:42  tweety
remove unnedded xpm Files; read pixmaps with read_icon,
separate more pixmaps from icons

Revision 1.13  2005/08/09 01:08:31  tweety
Twist and bend in the Makefiles to install the DataDirectory more apropriate
move the perl Functions to Geo::Gpsdrive::POI in /usr/share/perl5/Geo/Gpsdrive/POI
adapt icons.txt loading according to these directories

Revision 1.12  2005/08/08 20:46:50  tweety
icons.txt support for non sql use
Autor:Daniel Hiepler <rigid@akatash.de>

Revision 1.11  2005/08/08 20:25:32  tweety
new indentation for src/icons.c

Revision 1.10  2005/04/20 23:33:49  tweety
reformatted source code with anjuta
So now we have new indentations

Revision 1.9  2005/04/13 19:58:31  tweety
renew indentation to 4 spaces + tabstop=8

Revision 1.8  2005/04/10 21:50:50  tweety
reformatting c-sources

Revision 1.7  2005/04/06 19:38:17  tweety
use disable/enable keys to improove spee in database creation
add draw_small_plus_sign, which is used if we would have too many waypoints to display
extract draw_text from draw_poi loop

Revision 1.6  2005/03/27 21:25:46  tweety
separating map_import from gpsdrive.c

Revision 1.5  2005/02/13 14:06:54  tweety
start street randering functions. reading from the database streets and
displaying it on the screen
improve a little bit in the sql-queries
fixed linewidth settings in draw_cross

Revision 1.4  2005/02/08 09:01:48  tweety
move loading of usericons to icons.c

Revision 1.3  2005/02/08 08:43:46  tweety
wrong dfinition for auxicons array

Revision 1.2  2005/02/06 21:18:05  tweety
more cleanup: extracted more functionality to functions

Revision 1.1  2005/02/06 17:56:18  tweety
icons.c is ectracted from gpsdrive.c


***********************************************************************/

/*  Include Dateien */
#include "../config.h"
#include <stdlib.h>
#include <string.h>
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

#include "gettext.h"


#include "LatLong-UTMconversion.h"
#include "gpsdrive.h"
#include <dirent.h>
#include "battery.h"
#include "track.h"
#include "poi.h"
#include "icons.h"
#include <pthread.h>
#include <semaphore.h>

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

extern gint debug;
extern gint mydebug;
extern gint muteflag, sqlflag, trackflag;
extern GdkColor red;
extern GdkColor black;
extern GdkColor white;
extern GdkColor blue;

GdkPixbuf *friendsimage = NULL, *friendspixbuf = NULL;
GdkPixbuf *kismetpixbuf;
GdkPixbuf *iconpixbuf[MAX_ICONS];


gint maxauxicons = MAXWPTYPES, lastauxicon = 0;
auxiconsstruct auxicons[MAXWPTYPES];

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
  int symbol = 0, aux = -1, i, x, y, counter;
  gchar icon[80];
  GdkPixbuf *current_icon;

  //printf("draw_icon %d %d %s\n", posxdest,  posydest,icon_name);

  g_strlcpy (icon, icon_name, sizeof (icon));
  if (!sqlflag)
    g_strup (icon);

  /* sweep through all icons and look for icon */
  for (counter = 0; counter < MAX_ICONS; counter++)
    {
      if (0 == strcmp (icon, auxicons[counter].name))
	{
	  if (mydebug > 10)
	    printf ("Found icon %d \"%s\" (%dx%d)...\n", counter, icon,
		    gdk_pixbuf_get_width (auxicons[counter].icon),
		    gdk_pixbuf_get_height (auxicons[counter].icon));
	  current_icon = auxicons[counter].icon;
	  symbol = counter;
	  break;
	}
    }

  for (i = 0; i < lastauxicon; i++)
    if ((strcmp (icon, (auxicons + i)->name)) == 0)
      {
	if ((posxdest >= 0) && (posxdest < SCREEN_X)
	    && (posydest >= 0) && (posydest < SCREEN_Y))
	  {
	    x = gdk_pixbuf_get_width ((auxicons + i)->icon);
	    y = gdk_pixbuf_get_width ((auxicons + i)->icon);
	    gdk_draw_pixbuf (drawable, kontext,
			     (auxicons + i)->icon, 0, 0,
			     posxdest - x / 2,
			     posydest - y / 2, x, y,
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

  if ((posxdest >= 0) && (posxdest < SCREEN_X)
      && (posydest >= 0) && (posydest < SCREEN_Y))
    {
      if (mydebug > 20)
	printf ("Drawing icon %d loaded from icons.txt...\n", symbol);
      gdk_draw_pixbuf (drawable, kontext, iconpixbuf[symbol],
		       0, 0,
		       posxdest -
		       (gdk_pixbuf_get_width (GDK_PIXBUF (iconpixbuf[symbol]))
			/ 2),
		       posydest -
		       (gdk_pixbuf_get_height
			(GDK_PIXBUF (iconpixbuf[symbol])) / 2),
		       gdk_pixbuf_get_width (GDK_PIXBUF (iconpixbuf[symbol])),
		       gdk_pixbuf_get_height (GDK_PIXBUF
					      (iconpixbuf[symbol])),
		       GDK_RGB_DITHER_NONE, 0, 0);
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
  GdkPixbuf *iconpixbuf;
  if (mydebug > 50)
    printf ("read_icon(%s)\n", icon_name);

  // Try complete Path
  iconpixbuf = gdk_pixbuf_new_from_file (icon_name, NULL);


  if (!iconpixbuf)		// Try in ./data/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "./data/pixmaps/%s",
		  icon_name);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }
  if (!iconpixbuf)		// Try in ./data/icons
    {
      g_snprintf (filename, sizeof (filename), "./data/icons/%s",
		  icon_name);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }
  if (!iconpixbuf)		// Try in ./data/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "./data/pixmaps/%s",
		  homedir, icon_name);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }
  if (!iconpixbuf)		// Try in Homedir/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "%s/.gpsdrive/pixmaps/%s",
		  homedir, icon_name);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }
  if (!iconpixbuf)		// Try in homedir/icons
    {
      g_snprintf (filename, sizeof (filename), "%s/gpsdrive/icons/%s",
		  homedir, icon_name);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }


  if (!iconpixbuf)		// Try in DATADIR/pixmaps
    {
      g_snprintf (filename, sizeof (filename), "%s/gpsdrive/pixmaps/%s",
		  DATADIR, icon_name);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }
  if (!iconpixbuf)		// Try in DATADIR/icons
    {
      g_snprintf (filename, sizeof (filename), "%s/gpsdrive/icons/%s",
		  DATADIR, icon_name);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }

  if (!iconpixbuf)		// None Found
    {
      fprintf (stderr, "** Failed to open \"%s\"\n", icon_name);
      g_snprintf (filename, sizeof (filename),
		  "%s/gpsdrive/icons/unknown.png", DATADIR);
      iconpixbuf = gdk_pixbuf_new_from_file (filename, NULL);
    }
  else
    {
      if (mydebug > 5)
	printf ("read_icon(%s) --> %s OK\n", icon_name, filename);
    }

  return iconpixbuf;
}


/* -----------------------------------------------------------------------------
 * load icons specified in icons.txt for non sql useers
*/
void
load_icons (void)
{
  /* icons.txt */
  FILE *iconfile;
  char filename[255], wptype[64], icon_name[1024];
  int iconcounter;

  /* hardcoded kismet-stuff */
  kismetpixbuf = read_icon ("kismet.png");

  /* load icons defined in icons.txt */
  snprintf ((char *) &filename, sizeof (filename), "%s/icons.txt", homedir);
  iconfile = fopen (filename, "r");
  /* if there is no icons.txt, try to open it  from datadir */
  if (!iconfile)
    {
      snprintf ((char *) &filename, 255, "%s/gpsdrive/icons.txt", DATADIR);
      if (mydebug > 3)
	{
	  printf ("Trying default icons.txt: \"%s\"\n", filename);
	}
      iconfile = fopen (filename, "r");
    }
  if (iconfile)
    {
      if (mydebug > 3)
	printf ("Icons will be read from: \"%s\"\n", filename);

      for (iconcounter = 0; iconcounter < MAX_ICONS; iconcounter++)
	{
	  fscanf (iconfile, "%s %s\n", (char *) &wptype, (char *) &icon_name);
	  if (mydebug > 3)
	    printf ("Waypoint-type %d \"%s\" gets \"%s\"\n", iconcounter,
		    wptype, (char *) &icon_name);
	  iconpixbuf[iconcounter] = read_icon (icon_name);
	  if (!iconpixbuf[iconcounter])	// None Found
	    {
	      printf ("** Failed to open \"%s\"\n", (char *) &icon_name);
	      auxicons[iconcounter].icon = NULL;
	      auxicons[iconcounter].name[0] = 0;
	    }
	  else
	    {
	      strncpy ((char *) &auxicons[iconcounter].name, wptype,
		       sizeof (auxicons[iconcounter].name));
	      auxicons[iconcounter].icon = iconpixbuf[iconcounter];
	    }
	  if (feof (iconfile))
	    break;
	}
      fclose (iconfile);
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
  auxicons[lastauxicon].icon = gdk_pixbuf_new_from_file (path, NULL);

  if (auxicons[lastauxicon].icon == NULL)
    {
      g_snprintf (path, sizeof (path), "%s/gpsdrive/icons/%s.png",
		  DATADIR, icon_name);
      auxicons[lastauxicon].icon = gdk_pixbuf_new_from_file (path, NULL);
    }

  if ((auxicons + lastauxicon)->icon != NULL)
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
	  g_strlcpy ((auxicons + lastauxicon)->name, icon_name,
		     sizeof (auxicons->name));
	  fprintf (stderr, _("Loaded user defined icon %s\n"), path);
	  lastauxicon++;
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
