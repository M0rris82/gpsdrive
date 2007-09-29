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
 * Track support module: display, disk loading/saving.
 * Based in Files track0001.sav
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "track.h"
#include "config.h"
#include "gui.h"
#include "main_gui.h"

/*  Defines for gettext I18n */
#include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern gint mydebug;
extern gint maploaded;
extern glong tracknr, tracklimit, trackcoordlimit;
glong trackcoordnr, tracklimit, trackcoordlimit,old_trackcoordnr;
extern trackcoordstruct *trackcoord;
extern GdkSegment *track;
extern GdkSegment *trackshadow;

extern wpstruct *routelist;
extern GdkColor blue;
extern gint isnight, disableisnight;
extern color_struct colors;
extern currentstatus_struct current;
extern coordinate_struct coords;
extern GdkGC *kontext_map;
extern gdouble milesconv;

tripdata_struct trip;

/* ----------------------------------------------------------------------------- */
/*  if zoom, xoff, yoff or map are changed */
void
rebuildtracklist (void)
{
  gdouble posxdest, posydest;
  gdouble posxsource, posysource;
  posxsource = -1000;
  posysource = -1000;
  gint i, so;

  if (!maploaded)
    return;

  if (!local_config.showtrack)
    return;
  if (current.importactive)
    return;

  tracknr = 0;
  for (i = 0; i < trackcoordnr; i++)
    {
      calcxy (&posxdest, &posydest, (trackcoord + i)->lon,
	      (trackcoord + i)->lat, current.zoom);

      if ((trackcoord + i)->lon > 1000.0)	/* Track Break ? */
	{
	  posxsource = posysource = -1000;
	}
      else
	{
	  if (((posxdest > -50) && (posxdest < (SCREEN_X + 50))
	       && (posydest > -50)
	       && (posydest < (SCREEN_Y + 50))
	       && (posxsource > -50)
	       && (posxsource < (SCREEN_X + 50))
	       && (posysource > -50) && (posysource < (SCREEN_Y + 50))))
	    {
	      if ((posxdest != posxsource) || (posydest != posysource))
		{
		  /*              so=(int)(((trackcoord + i)->alt))>>5; */
		  so = SHADOWOFFSET;
		  /*
		   * printf("x: %8f %8f Y:%.6f %.6f \t",
		   * posxsource,posxdest,
		   * posysource,posydest);
		   * printf("%.6f, %.6f (%.6f)\n",
		   * (trackcoord + i)->lon,
		   * (trackcoord + i)->lat, current.zoom);
		   */
		  (track + tracknr)->x1 = posxsource;
		  (track + tracknr)->x2 = posxdest;
		  (track + tracknr)->y1 = posysource;
		  (track + tracknr)->y2 = posydest;
		  (trackshadow + tracknr)->x1 = posxsource + so;
		  (trackshadow + tracknr)->x2 = posxdest + so;
		  (trackshadow + tracknr)->y1 = posysource + so;
		  (trackshadow + tracknr)->y2 = posydest + so;

		  tracknr += 1;
		}
	    }
	  posxsource = posxdest;
	  posysource = posydest;
	}
    }
}



/* ------------------------------------------------------------------------- *
 * draw track on image
 */
void
drawtracks (void)
{
	gint t=0;

	//    if (!maploaded)
	//      return;
	if (!local_config.showtrack)
		return;
	if (current.importactive)
		return;

	t = 2 * (tracknr >> 1) - 1;
	/*     t=tracknr; */
	if (t < 1)
    	return;

    gdk_gc_set_line_attributes (kontext_map, 4, 0, 0, 0);
	if (local_config.showshadow) {
 		gdk_gc_set_foreground (kontext_map, &colors.shadow);
		gdk_gc_set_function (kontext_map, GDK_AND);
		gdk_draw_segments (drawable, kontext_map, (GdkSegment *) trackshadow, t);
		gdk_gc_set_function (kontext_map, GDK_COPY);
	}
	if ((!disableisnight) && 
		((local_config.nightmode == NIGHT_ON) ||
		((local_config.nightmode == NIGHT_AUTO) && isnight)))
		gdk_gc_set_foreground (kontext_map, &colors.red);
	else
		gdk_gc_set_foreground (kontext_map, &colors.track);

	gdk_draw_segments (drawable, kontext_map, (GdkSegment *) track, t);

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
  gchar filename_track[400], lat[30], alt[30], lon[30];
  FILE *st;

  if ( mydebug > 11 )
      g_print ("savetrack(%d)\n", mode);

  if (mode == 0)
    {
      i = 0;
      do
	{
	  g_snprintf (buff, sizeof (buff), "%strack%04d.sav",
	  	local_config.dir_home, i++);
	  e = stat (buff, &sbuf);
	}
      while (e == 0);
      g_strlcpy (savetrackfn, g_basename (buff), sizeof (savetrackfn));
      return;
    }

  /* save in new file */
  g_strlcpy (filename_track, local_config.dir_home, sizeof (filename_track));
  g_strlcat (filename_track, savetrackfn, sizeof (filename_track));
  if ( mydebug > 11 )
      g_print ("savetrack(%d,%s)\n", mode,filename_track);
  st = fopen (filename_track, "w");
  if (st == NULL)
    {
      perror (filename_track);
      return;
    }

  for (i = 0; i < trackcoordnr; i++)
    {
      g_snprintf (lat, sizeof (lat), "%10.6f", (trackcoord + i)->lat);
      g_strdelimit (lat, ",", '.');
      g_snprintf (lon, sizeof (lon), "%10.6f", (trackcoord + i)->lon);
      g_strdelimit (lon, ",", '.');
      g_snprintf (alt, sizeof (alt), "%10.0f", (trackcoord + i)->alt);

      fprintf (st, "%s %s %s %s\n", lat, lon, alt,
	       (trackcoord + i)->postime);
    }
  fclose (st);

  if (mode == 1)
    return;

  /* append to existing backup file */
  g_strlcpy (filename_track, local_config.dir_home, sizeof (filename_track));
  g_strlcat (filename_track, "track-ALL.sav", sizeof (filename_track));
  st = fopen (filename_track, "a");
  if (st == NULL)
    {
      perror (filename_track);
      return;
    }

  for (i = 0; i < trackcoordnr; i++)
    {
      g_snprintf (lat, sizeof (lat), "%10.6f", (trackcoord + i)->lat);
      g_strdelimit (lat, ",", '.');
      g_snprintf (lon, sizeof (lon), "%10.6f", (trackcoord + i)->lon);
      g_strdelimit (lon, ",", '.');
      g_snprintf (alt, sizeof (alt), "%10.0f", (trackcoord + i)->alt);

      fprintf (st, "%s %s %s %s\n", lat, lon, alt,
	       (trackcoord + i)->postime);
    }
  fclose (st);

}

/* *****************************************************************************
 * this routine is called whenever a new position is processed.  Every
 * 30 seconds it writes the accumulated positions to a file "incremental.sav"
 * so that a pretty recent track is available even if the application or
 * system crashes.  It is expected that the user will manually delete the
 * file prior to a trip/hike.   It will only write to the file if the
 * "save_track" option is selected.  If gpsdrive has been running for
 * quite a while without "save_track" selected, this routine will capture
 * the entire track up to that point.
*/

void do_incremental_save() {
    gint i;
    gchar mappath[400], lat[30], alt[30], lon[30];
    FILE *st;
    
    if ((trackcoordnr % 30) == 29) { /* RNM: append to existing incremental file every 30 seconds */
	g_strlcpy (mappath, local_config.dir_home, sizeof (mappath));
	g_strlcat (mappath, "incremental.sav", sizeof(mappath));
	st = fopen (mappath, "a");
                if (st == NULL) {
		    st = fopen (mappath, "w");
		    if (st == NULL) {
			perror (mappath);
			printf("Can't open or create incremental.sav\n");
			return;
                        }
		    else printf("Creating incremental.sav\n");
                }
		
                for (i = old_trackcoordnr; i < trackcoordnr; i++) {
		    g_snprintf (lat, sizeof (lat), "%10.6f", (trackcoord + i)->lat);
		    g_strdelimit (lat, ",", '.');
		    g_snprintf (lon, sizeof (lon), "%10.6f", (trackcoord + i)->lon);
		    g_strdelimit (lon, ",", '.');
		    g_snprintf (alt, sizeof (alt), "%10.0f", (trackcoord + i)->alt);
		    
		    fprintf (st, "%s %s %s %s\n", lat, lon, alt, (trackcoord + i)->postime);
                }
                fclose (st);
                old_trackcoordnr = trackcoordnr ;
    }
}


/* *****************************************************************************
 */
gint
gettrackfile (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *fn;
  gchar buf[520], lat[30], lon[30], alt[30], str[30];
  FILE *st;
  gint i;

  fn = gtk_file_chooser_get_filename (datum);
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
      sscanf (buf, "%s %s %s %[^\n]", lat, lon, alt, str);
      g_strlcpy ((trackcoord + i)->postime, str, 30);
      (trackcoord + i)->lat = g_strtod (lat, NULL);
      (trackcoord + i)->lon = g_strtod (lon, NULL);
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
  (trackcoord + i)->lon = 1001.0;

  trackcoordnr++;

  rebuildtracklist ();
  fclose (st);
  gtk_widget_destroy (datum);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (track_bt), TRUE);

  return TRUE;
}


/* *****************************************************************************
 * Callback to handle togglebutton "save track"
 */
int
toggle_track_button_cb (GtkWidget *button, gboolean *value)
{
	*value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));
	
	if ( *value ) {
	    savetrackfile (1);
	}
	current.needtosave = TRUE;
	return TRUE;
}


/* *****************************************************************************
 * reset trip data
 */
gint
trip_reset_cb ()
{
	if (mydebug > 10)
		fprintf (stdout, "Resetting trip data\n");

	trip.odometer = trip.speed_avg = trip.speed_max = 0.0;
	trip.countavgspeed = 0;
	trip.lat = coords.current_lat;
	trip.lon = coords.current_lon;
	trip.time = time (NULL);

	return TRUE;
}


/* *****************************************************************************
 * update trip data
 */
gint
update_tripdata_cb (void)
{
	gdouble t_dist;

	t_dist = calcdist (trip.lon, trip.lat);
	trip.lon = coords.current_lon;
	trip.lat = coords.current_lat;
	
	if (mydebug > 20)
	{
		fprintf (stderr, "update_tripdata_cb: trip_dist = %f | trip_speed = %f\n",
			t_dist, t_dist / TRIPMETERTIMEOUT * 3600.0);
	}

	if (t_dist <= 0.0)
		return TRUE;

	 if (t_dist > (2000.0 * TRIPMETERTIMEOUT / 3600.0))
	{
		fprintf (stderr,
		_("distance jump is more then 2000km/h speed, ignoring\n"));
		return TRUE;
	}

	/* we want always have metric system stored */
	t_dist /= milesconv;
	if (trip.odometer > 0)
	{
		if (current.groundspeed / milesconv > trip.speed_max)
			trip.speed_max = current.groundspeed / milesconv;
		trip.countavgspeed++;
		trip.speed_avg += current.groundspeed / milesconv;
	}
	trip.odometer += t_dist;
	return TRUE;
}
