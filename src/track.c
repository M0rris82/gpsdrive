/***********************************************************************

Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>
Copyright (c) 2007 Guenther Meyer <d.s.e (at) sordidmusic.com>

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

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "track.h"
#include "gui.h"
#include "main_gui.h"
#include <math.h>

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
extern trackdata_struct *trackcoord;
extern GdkSegment *track;
extern GdkSegment *trackshadow;

extern wpstruct *routelist;
extern GdkColor blue;
extern color_struct colors;
extern currentstatus_struct current;
extern coordinate_struct coords;
extern GdkGC *kontext_map;
extern gdouble milesconv;
extern GtkWidget *main_window;
extern gdouble hdop;
extern gint sats_used;

tripdata_struct trip;


/* *****************************************************************************
 * Load track file and displays it 
 */
gint
loadtrack_cb (GtkWidget * widget, gpointer datum)
{
	GtkWidget *fdialog;
	gchar buf[1000];
	GtkWidget *ok_bt;
	GtkWidget *cancel_bt;

	fdialog = gtk_file_chooser_dialog_new (_("Select a track file"),
		GTK_WINDOW (main_window), GTK_FILE_CHOOSER_ACTION_OPEN,
		NULL, NULL);

	gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
	
	cancel_bt = gtk_dialog_add_button (GTK_DIALOG (fdialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	ok_bt = gtk_dialog_add_button (GTK_DIALOG (fdialog), GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT);

	g_signal_connect (GTK_OBJECT (ok_bt), "clicked",
		G_CALLBACK (gettrackfile), GTK_OBJECT (fdialog));
	g_signal_connect_swapped (GTK_OBJECT (cancel_bt), "clicked",
		G_CALLBACK (gtk_widget_destroy), GTK_OBJECT (fdialog));

	g_strlcpy (buf, local_config.dir_tracks, sizeof (buf));
	g_strlcat (buf, "track*.sav", sizeof (buf));

	gtk_file_chooser_select_filename (GTK_FILE_CHOOSER (fdialog), buf);

	gtk_widget_show (fdialog);

	return TRUE;
}


void
add_trackpoint
	(gdouble lat, gdouble lon, gdouble alt, gdouble course,
		gdouble speed, gdouble hdop, gint fix, gint sat, gchar *time)
{
	(trackcoord + trackcoordnr)->lat = lat;
	(trackcoord + trackcoordnr)->lon = lon;
	(trackcoord + trackcoordnr)->alt = alt;
	(trackcoord + trackcoordnr)->course = course;
	(trackcoord + trackcoordnr)->speed = speed;
	(trackcoord + trackcoordnr)->hdop = hdop;
	(trackcoord + trackcoordnr)->fix = fix;
	(trackcoord + trackcoordnr)->sat = sat;

	if (time == NULL)
	{
		GTimeVal current_time;
		g_get_current_time (&current_time);
		g_strlcpy ((trackcoord + trackcoordnr)->postime,
			g_time_val_to_iso8601 (&current_time), 30);
	}
	else
		g_strlcpy ((trackcoord + trackcoordnr)->postime, time, 30);

	trackcoordnr++;

}


/* *****************************************************************************
 * add new trackpoint to  'trackdata_struct list' to draw track on image 
 */
gint
storetrack_cb (GtkWidget * widget, guint * datum)
{
    if ( mydebug >50 ) 
	fprintf(stderr , "storetrack_cb()\n");

    if (gui_status.posmode) 
	return TRUE;
	
#ifdef DBUS_ENABLE
	/* If we use DBUS track points are usually stored by the DBUS signal handler */
	/* Only store them by timer if we are in position mode */
	if ( useDBUS && !current.simmode )
	    return TRUE;
#endif

	storepoint();

	return TRUE;
}


void
storepoint ()
{
	gint so;

	/*    g_print("Havepos: %d\n", current.gpsfix); */
	if ((!current.simmode && current.gpsfix < 2) || gui_status.posmode /*  ||((!local_config.simmode &&haveposcount<3)) */ )	/* we have no valid position */
	{
		add_trackpoint (1001.0, 1001.0, 1001.0, 1001.0, -1.0, -1.0, 0, 0, NULL);
	}
	else
	{
		add_trackpoint
			(coords.current_lat, coords.current_lon, current.altitude,
			 RAD2DEG (current.heading), current.groundspeed, current.gps_hdop,
			 current.gpsfix, sats_used, NULL);
		if (local_config.savetrack)
			do_incremental_save();
	}


	/*  The double storage seems to be silly, but I have to use  */
	/*  gdk_draw_segments instead of gdk_draw_lines.   */
	/*  gkd_draw_lines is dog slow because of a gdk-bug. */

	if (tracknr == 0)
	{
		if ((trackcoord + trackcoordnr - 1)->lon < 1000.0)
		{
			(track + tracknr)->x1 = current.pos_x;
			(track + tracknr)->y1 = current.pos_y;
			(trackshadow + tracknr)->x1 = current.pos_x + SHADOWOFFSET;
			(trackshadow + tracknr)->y1 = current.pos_y + SHADOWOFFSET;
			tracknr++;
		}
	}
	else
	{
		if ((trackcoord + trackcoordnr - 1)->lon < 1000.0)
		{
			if ((current.pos_x != (track + tracknr - 1)->x2)
			    || (current.pos_y != (track + tracknr - 1)->y2))
			{
				/* so=(int)(((trackcoord + trackcoordnr)->alt))>>5; */
				so = SHADOWOFFSET;
				(track + tracknr)->x1 =
					(track + tracknr - 1)->x2 = current.pos_x;
				(track + tracknr)->y1 =
					(track + tracknr - 1)->y2 = current.pos_y;
				(trackshadow + tracknr)->x1 =
					(trackshadow + tracknr - 1)->x2 =
					current.pos_x + so;
				(trackshadow + tracknr)->y1 =
					(trackshadow + tracknr - 1)->y2 =
					current.pos_y + so;
				tracknr += 1;
			}
		}
		else
			tracknr = tracknr & ((glong) - 2);
	}
}


/* ----------------------------------------------------------------------------- */
/*  if zoom, xoff, yoff or map are changed */
void
rebuildtracklist (void)
{
  gint posxdest, posydest;
  gint posxsource, posysource;
  gint i, so;
  posxsource = -1000;
  posysource = -1000;

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
	  if (((posxdest > -50) && (posxdest < (gui_status.mapview_x + 50))
	       && (posydest > -50)
	       && (posydest < (gui_status.mapview_y + 50))
	       && (posxsource > -50)
	       && (posxsource < (gui_status.mapview_x + 50))
	       && (posysource > -50) && (posysource < (gui_status.mapview_y + 50))))
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

    gdk_gc_set_line_attributes (kontext_map, 4, GDK_LINE_SOLID, 0, 0);
	if (local_config.showshadow) {
 		gdk_gc_set_foreground (kontext_map, &colors.shadow);
		gdk_gc_set_function (kontext_map, GDK_AND);
		gdk_draw_segments (drawable, kontext_map, (GdkSegment *) trackshadow, t);
		gdk_gc_set_function (kontext_map, GDK_COPY);
	}
	if ( gui_status.nightmode )
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
      GTimeVal current_time;
      gchar datestring[11];

      g_get_current_time (&current_time);
      g_strlcpy (datestring, g_time_val_to_iso8601 (&current_time), 11);

      i = 0;
      do
	{
	  g_snprintf (buff, sizeof (buff), "%strack_%s_%03d.sav",
	  	local_config.dir_tracks, datestring ,i++);
	  e = stat (buff, &sbuf);
	}
      while (e == 0);
      g_strlcpy (savetrackfn, g_basename (buff), sizeof (savetrackfn));
      return;
    }

  /* save in new file */
  g_strlcpy (filename_track, local_config.dir_tracks, sizeof (filename_track));
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
      g_snprintf (lat, sizeof (lat), "%.6f", (trackcoord + i)->lat);
      g_strdelimit (lat, ",", '.');
      g_snprintf (lon, sizeof (lon), "%.6f", (trackcoord + i)->lon);
      g_strdelimit (lon, ",", '.');
      g_snprintf (alt, sizeof (alt), "%.1f", (trackcoord + i)->alt);

      fprintf (st, "%s %s %s %s %.6f %.6f %.6f %d %d\n",
	       lat, lon, alt,
	       (trackcoord + i)->postime,
	       (trackcoord + i)->course,
	       (trackcoord + i)->speed,
	       (trackcoord + i)->hdop,
	       (trackcoord + i)->sat,
	       (trackcoord + i)->fix);
    }
  fclose (st);

  if (mode == 1)
    return;

  /* append to existing backup file */
  g_strlcpy (filename_track, local_config.dir_tracks, sizeof (filename_track));
  g_strlcat (filename_track, "track-ALL.sav", sizeof (filename_track));
  st = fopen (filename_track, "a");
  if (st == NULL)
    {
      perror (filename_track);
      return;
    }

  for (i = 0; i < trackcoordnr; i++)
    {
      g_snprintf (lat, sizeof (lat), "%.6f", (trackcoord + i)->lat);
      g_strdelimit (lat, ",", '.');
      g_snprintf (lon, sizeof (lon), "%.6f", (trackcoord + i)->lon);
      g_strdelimit (lon, ",", '.');
      g_snprintf (alt, sizeof (alt), "%.1f", (trackcoord + i)->alt);

      fprintf (st, "%s %s %s %s %.6f %.6f %.6f %d %d\n",
	       lat, lon, alt,
	       (trackcoord + i)->postime,
	       (trackcoord + i)->course,
	       (trackcoord + i)->speed,
	       (trackcoord + i)->hdop,
	       (trackcoord + i)->sat,
	       (trackcoord + i)->fix);
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
	g_strlcpy (mappath, local_config.dir_tracks, sizeof (mappath));
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
		
                for (i = old_trackcoordnr; i < trackcoordnr - 1; i++) {
		    g_snprintf (lat, sizeof (lat), "%.6f", (trackcoord + i)->lat);
		    g_strdelimit (lat, ",", '.');
		    g_snprintf (lon, sizeof (lon), "%.6f", (trackcoord + i)->lon);
		    g_strdelimit (lon, ",", '.');
		    g_snprintf (alt, sizeof (alt), "%.1f", (trackcoord + i)->alt);
		    
      		fprintf (st, "%s %s %s %s %.6f %.6f %.6f %d %d\n",
	       		lat, lon, alt,
	       		(trackcoord + i)->postime,
	       		(trackcoord + i)->course,
	       		(trackcoord + i)->speed,
	       		(trackcoord + i)->hdop,
	       		(trackcoord + i)->sat,
	       		(trackcoord + i)->fix);
                }
                fclose (st);
                old_trackcoordnr = trackcoordnr -1;
    }
}


/* ******************************************************************
 * Allocate memory for track storage
 *  If clear=TRUE then the track data in memory will be cleared.
 *  This function should be called every time with clear=FALSE, whenever
 *  there are more than 30000 points to be stored.
 */
void
init_track (gboolean clear)
{
	if (clear)
	{
		g_free (trackcoord);
		g_free (track);
		g_free (trackshadow);
		track = g_new (GdkSegment, 30000);
		trackshadow = g_new (GdkSegment, 30000);
		tracknr = 0;
		tracklimit = 30000;
		trackcoord = g_new (trackdata_struct, 30000);
		trackcoordnr = 0;
		trackcoordlimit = 30000;
	}
	else
	{
		trackcoord = g_renew (trackdata_struct, trackcoord, trackcoordlimit + 30000);
		trackcoordlimit += 30000;
		track = g_renew (GdkSegment, track, tracklimit + 30000);
		trackshadow = g_renew (GdkSegment, trackshadow, tracklimit + 30000);
		tracklimit += 30000;
	}
}


/* *****************************************************************************
 */
gint
gettrackfile (GtkWidget * widget, gpointer datum)
{
  //TODO:
  // - adjust this function to convert the date field
  //   to iso8601 when importing old gpsdrive track files
  // - add reading of new fields (course/speed/hdop/sat/fix)
  // - maybe duplicate this function to support pre 2.10 format
  //   AND post 2.10 format...

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

  init_track (TRUE);

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
          init_track (FALSE);
        }
    }
  (trackcoord + i)->lat = 1001.0;
  (trackcoord + i)->lon = 1001.0;

  trackcoordnr++;

  rebuildtracklist ();
  fclose (st);
  gtk_widget_destroy (datum);

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
