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
#include <xpm_compass.h>
#include <errno.h>

#if HAVE_LOCALE_H
#include <locale.h>
#else
# define setlocale(Category, Locale)
#endif
#include "gettext.h"


#include "LatLong-UTMconversion.h"
#include "gpsdrive.h"
#include <dirent.h>
#include "battery.h"
#include "track.h"
#include "poi.h"
#include "xpm_icons.h"
#include "icons.h"
#include <dlfcn.h>
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

extern gint debug;
extern gint muteflag , sqlflag , trackflag ;
extern GdkColor red;
extern GdkColor black;
extern GdkColor white;
extern GdkColor blue;

GdkPixbuf *friendsimage = NULL, *friendspixbuf = NULL;
GdkPixbuf *kismetpixbuf, *openwlanpixbuf, *closedwlanpixbuf, *iconpixbuf[50];

gint maxauxicons = MAXWPTYPES, lastauxicon = 0;
auxiconsstruct auxicons[MAXWPTYPES];

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


/* draw a + Sign and its shaddow */
void
draw_plus_sign ( gdouble posxdest,   gdouble posydest )
{
  if (shadow)
    { /*  draw shadow of + sign */
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
		 posxdest + 1,     posydest + 1 - 5, 
		 posxdest + 1,     posydest + 1 + 5);
  gdk_draw_line (drawable, kontext, 
		 posxdest + 1 + 5, posydest + 1, 
		 posxdest + 1 - 5, posydest + 1);

}


/* -----------------------------------------------------------------------------

 */
int
drawicon (gint posxdest, gint posydest, char *ic)
{
  int symbol = 0, aux = -1, i, x, y;
  gchar icon[80];

  // if ( debug ) printf("draw_icon %d %d %s\n", posxdest,  posydest,ic);

  g_strlcpy (icon, ic, sizeof (icon));
  if (!sqlflag)
    g_strup (icon);

  if (! strcmp (icon, "REST"))
    symbol = 1;
  else if ((strcmp (icon, "MCDONALDS")) == 0)
    symbol = 2;
  else if ((strcmp (icon, "HOTEL")) == 0)
    symbol = 3;
  else if ((strcmp (icon, "BURGERKING")) == 0)
    symbol = 4;
  else if ((strcmp (icon, "SHOP")) == 0)
    symbol = 5;
  else if ((strcmp (icon, "MONU")) == 0)
    symbol = 6;
  else if ((strcmp (icon, "NIGHTCLUB")) == 0)
    symbol = 7;
  else if ((strcmp (icon, "SPEEDTRAP")) == 0)
    symbol = 8;
  else if ((strcmp (icon, "AIRPORT")) == 0)
    symbol = 9;
  else if ((strcmp (icon, "GOLF")) == 0)
    symbol = 10;
  else if ((strcmp (icon, "GASSTATION")) == 0)
    symbol = 11;
  else if ((strcmp (icon, "CAFE")) == 0)
    symbol = 12;
  else if ((strcmp (icon, "GEOCACHE")) == 0)
    symbol = 13;

  for (i = 0; i < lastauxicon; i++)
    if ((strcmp (icon, (auxicons + i)->name)) == 0)
      {
	if ((posxdest >= 0) && (posxdest < SCREEN_X)
	    && 
	    (posydest >= 0) && (posydest < SCREEN_Y))
	  {
	    x = gdk_pixbuf_get_width ((auxicons + i)->icon);
	    y = gdk_pixbuf_get_width ((auxicons + i)->icon);
	    gdk_draw_pixbuf (drawable, kontext, (auxicons + i)->icon,
			     0, 0,
			     posxdest - x / 2,
			     posydest - y / 2, x, y, GDK_RGB_DITHER_NONE,
			     0, 0);
	    aux = i;
	  }
	return 99999;
      }
  
  if (symbol == 0)
    {
      draw_plus_sign( posxdest, posydest );
      return 0;
    }

  if ( (posxdest >= 0) && (posxdest < SCREEN_X)
       &&    
       (posydest >= 0) && (posydest < SCREEN_Y))
    {
      gdk_draw_pixbuf (drawable, kontext, iconpixbuf[symbol - 1],
		       0, 0,
		       posxdest - 12, posydest - 12,
		       24, 24, GDK_RGB_DITHER_NONE, 0, 0);
    }
  return symbol;
}

/* -----------------------------------------------------------------------------

 */
void
load_icons(void)
{
  kismetpixbuf     = gdk_pixbuf_new_from_xpm_data ((const char **) kismet_xpm);
  openwlanpixbuf   = gdk_pixbuf_new_from_xpm_data ((const char **) open_xpm);
  closedwlanpixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) closed_xpm);
  iconpixbuf[0]    = gdk_pixbuf_new_from_xpm_data ((const char **) rest_xpm);
  iconpixbuf[1]    = gdk_pixbuf_new_from_xpm_data ((const char **) mcdonalds_xpm);
  iconpixbuf[2]    = gdk_pixbuf_new_from_xpm_data ((const char **) hotel_xpm);
  iconpixbuf[3]    = gdk_pixbuf_new_from_xpm_data ((const char **) burgerking_xpm);
  iconpixbuf[4]    = gdk_pixbuf_new_from_xpm_data ((const char **) shop_xpm);
  iconpixbuf[5]    = gdk_pixbuf_new_from_xpm_data ((const char **) monu_xpm);
  iconpixbuf[6]    = gdk_pixbuf_new_from_xpm_data ((const char **) girls_xpm);
  iconpixbuf[7]    = gdk_pixbuf_new_from_xpm_data ((const char **) speedtrap_xpm);
  iconpixbuf[8]    = gdk_pixbuf_new_from_xpm_data ((const char **) airport_xpm);
  iconpixbuf[9]    = gdk_pixbuf_new_from_xpm_data ((const char **) golf_xpm);
  iconpixbuf[10]   = gdk_pixbuf_new_from_xpm_data ((const char **) fuel_xpm);
  iconpixbuf[11]   = gdk_pixbuf_new_from_xpm_data ((const char **) cafe_xpm);
  iconpixbuf[12]   = gdk_pixbuf_new_from_xpm_data ((const char **) geocache_xpm);
}

/* -----------------------------------------------------------------------------

 */
void
load_friends_icon (void)
{
  gchar mappath[400];

  g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/%s", DATADIR,
	      "friendsicon.png");
  friendsimage = gdk_pixbuf_new_from_file (mappath, NULL);
  if (friendsimage == NULL)
    {
      friendsimage = gdk_pixbuf_new_from_file ("friendsicon.png", NULL);
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
