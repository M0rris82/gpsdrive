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
Revision 1.3  2005/02/08 20:12:59  tweety
savetrackfile got 3 modi

Revision 1.2  2005/01/11 20:14:14  tweety
rearanges track handling a little bit
added suport for interupted tracks

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.18  2004/02/08 17:16:25  ganter
replacing all strcat with g_strlcat to avoid buffer overflows

Revision 1.17  2004/02/07 17:46:10  ganter
...

Revision 1.16  2004/02/05 19:47:31  ganter
replacing strcpy with g_strlcpy to avoid bufferoverflows
USB receiver does not send sentences in direct serial mode,
so I first send a "\n" to it

Revision 1.15  2004/02/02 03:38:32  ganter
code cleanup

Revision 1.14  2004/01/20 17:39:19  ganter
working on import function

Revision 1.13  2004/01/17 06:11:04  ganter
added color setting for track color

Revision 1.12  2004/01/01 09:07:33  ganter
v2.06
trip info is now live updated
added cpu temperature display for acpi
added tooltips for battery and temperature

Revision 1.11  2003/05/31 20:12:35  ganter
new UDP friendsserver build in, needs some work

*/


/*
 * Track support module: display, disk loading/saving.
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gpsdrive.h"
#include "track.h"
#include "config.h"

extern gint maploaded;
extern gint trackflag;
extern gint importactive;
extern glong tracknr, tracklimit, trackcoordlimit;
extern glong trackcoordnr;
extern trackcoordstruct *trackcoord;
extern gint zoom;
extern GdkSegment *track;
extern GdkSegment *trackshadow;

extern gint showroute, routeitems;
typedef struct
{
  gchar name[40];
  gdouble lat;
  gdouble longitude;
  gdouble dist;
  gchar typ[40];
  gint wlan;
  gint action;
  gint sqlnr;
}
wpstruct;

extern wpstruct *routelist;
extern GdkColor blue;
extern gint nightmode, isnight, disableisnight;
extern GdkColor red;
extern GdkColor black;
extern GdkColor white;
extern GdkColor blue;
extern GdkColor nightcolor;
extern GdkColor trackcolorv;


/* ----------------------------------------------------------------------------- */ 
/*  if zoom, xoff, yoff or map are changed */
void
rebuildtracklist (void)
{
  gdouble posxdest, posydest;
  gdouble posxsource, posysource;
  posxsource=-1000;
  posysource=-1000;
  gint i, so;

  if (!maploaded)
    return;

  if (!trackflag)
    return;
  if (importactive)
    return;

  tracknr = 0;
  for (i = 0; i < trackcoordnr; i++)
    {
      calcxy (&posxdest, &posydest, (trackcoord + i)->longi,
	      (trackcoord + i)->lat, zoom);

      if ( (trackcoord + i)->longi > 1000.0 ) /* Track Break ? */
	{
	  posxsource = posysource = -1000;
	} 
      else
	{
	  if (
	      ( (posxdest > -50) && (posxdest < (SCREEN_X + 50)) &&
		(posydest > -50) && (posydest < (SCREEN_Y + 50)) &&
		(posxsource > -50) && (posxsource < (SCREEN_X + 50))&&
		(posysource > -50) && (posysource < (SCREEN_Y + 50))
		)
	      )
	    {
	      if ( (posxdest != posxsource) || 
		   (posydest != posysource)
		   )
		{
		  /*  		so=(int)(((trackcoord + i)->alt))>>5; */
		  so = SHADOWOFFSET;
		  /*
		  printf("x: %8f %8f Y:%.6f %.6f \t",
			 posxsource,posxdest,
			 posysource,posydest);
		  printf("%.6f, %.6f (%.6f)\n",
			 (trackcoord + i)->longi,
			 (trackcoord + i)->lat, zoom);
		  */
		  (track + tracknr)->x1       = posxsource;
		  (track + tracknr)->x2       = posxdest;
		  (track + tracknr)->y1       = posysource;
		  (track + tracknr)->y2       = posydest;
		  (trackshadow + tracknr)->x1 = posxsource + so;
		  (trackshadow + tracknr)->x2 = posxdest   + so;
		  (trackshadow + tracknr)->y1 = posysource + so;
		  (trackshadow + tracknr)->y2 = posydest   + so;

		  tracknr += 1;
		}
	    }
	  posxsource = posxdest;
	  posysource = posydest;
	}
    }
}



/* ----------------------------------------------------------------------------- */ 
/* draw track on image */
void
drawtracks (void)
{
  gint t;
  GdkSegment *routes;

  gdouble posxdest, posydest;
  gint i, j;

/*    if (!maploaded) */
/*      return; */
  if (!trackflag)
    return;
  if (importactive)
    return;
  if (showroute)
    {
      if (routeitems > 0)
	{
	  i = (routeitems + 5);
	  routes = g_new0 (GdkSegment, i);

	  for (j = 0; j <= routeitems; j++)
	    {
	      calcxy (&posxdest, &posydest, (routelist + j)->longitude,
		      (routelist + j)->lat, zoom);

	      if (j > 0)
		{
		  (routes + j - 1)->x2 = posxdest;
		  (routes + j - 1)->y2 = posydest;
		}
	      (routes + j)->x1 = posxdest;
	      (routes + j)->y1 = posydest;
	    }
	  t = routeitems - 1;
	  gdk_gc_set_line_attributes (kontext, 4, GDK_LINE_ON_OFF_DASH, 0, 0);

	  gdk_gc_set_foreground (kontext, &blue);
	  gdk_draw_segments (drawable, kontext, (GdkSegment *) routes, t);
	  g_free (routes);
	}
    }
  t = 2 * (tracknr >> 1) - 1;
/*     t=tracknr;  */
  if (t < 1)
    return;


  gdk_gc_set_line_attributes (kontext, 4, 0, 0, 0);
  if (shadow)
    {
      gdk_gc_set_foreground (kontext, &darkgrey);
      gdk_gc_set_function (kontext, GDK_AND);
      gdk_draw_segments (drawable, kontext, (GdkSegment *) trackshadow, t);
      gdk_gc_set_function (kontext, GDK_COPY);
    }
  if ((!disableisnight)
      && ((nightmode == 1) || ((nightmode == 2) && isnight)))
    gdk_gc_set_foreground (kontext, &red);
  else
    gdk_gc_set_foreground (kontext, &trackcolorv);

  gdk_draw_segments (drawable, kontext, (GdkSegment *) track, t);

  return;

}


/* ----------------------------------------------------------------------------- */ 
/* stores the track into a file, if argument testname is true, no saving is 
   done, only savetrackfn is set 
mode=0: test Filename
mode=1: write actual track
mode=2: write actual track and append to all_track.sav
*/

void
savetrackfile (gint mode)
{
  struct stat sbuf;
  gchar buff[1024];
  gint e, i;
  gchar mappath[400], lat[30], alt[30], longi[30];
  FILE *st;

  if ( mode == 0 )
    {
      i = 0;
      do
	{
	  g_snprintf (buff, sizeof (buff), "%strack%04d.sav", homedir, i++);
	  e = stat (buff, &sbuf);
	}
      while (e == 0);
      g_strlcpy (savetrackfn, g_basename (buff), sizeof (savetrackfn));
      return;
    }

/* save in new file */
  g_strlcpy (mappath, homedir, sizeof (mappath));
  g_strlcat (mappath, savetrackfn, sizeof(mappath));
  st = fopen (mappath, "w");
  if (st == NULL)
    {
      perror (mappath);
      return;
    }

  for (i = 0; i < trackcoordnr; i++)
    {
      g_snprintf (lat, sizeof (lat), "%10.6f", (trackcoord + i)->lat);
      g_strdelimit (lat, ",", '.');
      g_snprintf (longi, sizeof (longi), "%10.6f", (trackcoord + i)->longi);
      g_strdelimit (longi, ",", '.');
      g_snprintf (alt, sizeof (alt), "%10.0f", (trackcoord + i)->alt);

      fprintf (st, "%s %s %s %s\n", lat, longi, alt,
	       (trackcoord + i)->postime);
    }
  fclose (st);

  if ( mode == 1 )
    return;
  
/* append to existing backup file */
  g_strlcpy (mappath, homedir, sizeof (mappath));
  g_strlcat (mappath, "track-ALL.sav", sizeof(mappath));
  st = fopen (mappath, "a");
  if (st == NULL)
    {
      perror (mappath);
      return;
    }

  for (i = 0; i < trackcoordnr; i++)
    {
      g_snprintf (lat, sizeof (lat), "%10.6f", (trackcoord + i)->lat);
      g_strdelimit (lat, ",", '.');
      g_snprintf (longi, sizeof (longi), "%10.6f", (trackcoord + i)->longi);
      g_strdelimit (longi, ",", '.');
      g_snprintf (alt, sizeof (alt), "%10.0f", (trackcoord + i)->alt);

      fprintf (st, "%s %s %s %s\n", lat, longi, alt,
	       (trackcoord + i)->postime);
    }
  fclose (st);

}


gint
gettrackfile (GtkWidget * widget, gpointer datum)
{
  gchar *fn, buf[520], lat[30], longi[30], alt[30], str[30];
  FILE *st;
  gint i;

  fn = gtk_file_selection_get_filename (datum);
  st = fopen (fn, "r");
  if (st == NULL)
    {
      perror (fn);
      return TRUE;
    }
  g_free (trackcoord);
  g_free (track);
  g_free (trackshadow);
  track = g_new (GdkSegment, 100000);
  trackshadow = g_new (GdkSegment, 100000);
  tracknr = 0;
  tracklimit = 100000;
  trackcoord = g_new (trackcoordstruct, 100000);
  trackcoordnr = 0;
  trackcoordlimit = 100000;
  i = 0;
  while (fgets (buf, 512, st))
    {
      sscanf (buf, "%s %s %s %[^\n]", lat, longi, alt, str);
      g_strlcpy ((trackcoord + i)->postime, str, 30);
      (trackcoord + i)->lat = g_strtod (lat, NULL);
      (trackcoord + i)->longi = g_strtod (longi, NULL);
      (trackcoord + i)->alt = g_strtod (alt, NULL);
      i++;
      trackcoordnr++;

      if ((trackcoordnr * 2) > (trackcoordlimit - 1000))
	{
	  trackcoord =
	    g_renew (trackcoordstruct, trackcoord, trackcoordlimit + 100000);
	  trackcoordlimit += 100000;
	  track = g_renew (GdkSegment, track, tracklimit + 100000);
	  trackshadow =
	    g_renew (GdkSegment, trackshadow, tracklimit + 100000);
	  tracklimit += 100000;
	}


    }
  (trackcoord + i)->lat = 1001.0;
  (trackcoord + i)->longi = 1001.0;

  trackcoordnr++;

  rebuildtracklist ();
  fclose (st);
  gtk_widget_destroy (datum);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trackbt), TRUE);

  return TRUE;
}
