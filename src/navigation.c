/****************************************************************

Copyright (c) 2001-2003 Fritz Ganter <ganter@ganter.at>

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
Revision 1.2  2005/04/10 21:50:50  tweety
reformatting c-sources

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.9  2004/02/18 13:24:19  ganter
navigation

Revision 1.8  2004/02/16 23:15:20  ganter
activated navigation.c for teleatlas street maps, need some months of work to
get it functional.

Revision 1.7  2004/02/08 16:35:10  ganter
replacing all sprintf with g_snprintf to avoid buffer overflows

Revision 1.6  2004/02/02 03:38:32  ganter
code cleanup

Revision 1.5  2003/05/11 21:15:46  ganter
v2.0pre7
added script convgiftopng
This script converts .gif into .png files, which reduces CPU load
run this script in your maps directory, you need "convert" from ImageMagick

Friends mode runs fine now
Added parameter -H to correct the alitude

Revision 1.4  2003/01/15 15:30:28  ganter
before dynamically loading mysql

Revision 1.3  2002/11/02 12:38:55  ganter
changed website to www.gpsdrive.de

Revision 1.2  2002/06/12 10:58:49  ganter
v1.23pre7

Revision 1.1  2002/06/02 20:54:10  ganter
added navigation.c and copyrights


navigation.c - street navigation for gpsdrive
*****************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <gpsdrive.h>

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

/* variables */
extern gdouble current_long, current_lat, old_long, old_lat, groundspeed;
extern gdouble zero_long, zero_lat, target_long, target_lat, dist;
char actualstreetname[200], oldstreetname[200];
extern int debug;


gint
nav_doit (GtkWidget * widget, guint * datum)
{
#ifdef USETELEATLAS
  int e;
  static char streetname[200], buf[220];

  e =
    ta_getstreetname (current_lat, current_long, streetname,
											sizeof (streetname));
  if (e)
    {
      g_strlcpy (actualstreetname, streetname, sizeof (actualstreetname));
      if (debug)
				fprintf (stderr, "Location: %s\n", streetname);
    }
  else
    {
      g_strlcpy (actualstreetname, "---", sizeof (actualstreetname));
      if (debug)
				fprintf (stderr, "unknown location\n");
    }
  if (strcmp (actualstreetname, oldstreetname) != 0)
    {
      g_strlcpy (oldstreetname, actualstreetname, sizeof (oldstreetname));
      g_snprintf (buf, sizeof (buf), "%s\n", actualstreetname);
      speech_out_speek (buf);
    }

#endif
  return TRUE;
}

gint
navi_cb (GtkWidget * widget, guint datum)
{

	return TRUE;
}
