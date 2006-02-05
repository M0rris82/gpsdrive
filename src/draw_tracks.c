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
  Revision 1.7  2006/02/05 16:38:05  tweety
  reading floats with scanf looks at the locale LANG=
  so if you have a locale de_DE set reading way.txt results in clearing the
  digits after the '.'
  For now I set the LC_NUMERIC always to en_US, since there we have . defined for numbers

  Revision 1.6  2006/01/03 14:24:10  tweety
  eliminate compiler Warnings
  try to change all occurences of longi -->lon, lati-->lat, ...i
  use  drawicon(posxdest,posydest,"w-lan.open") instead of using a seperate variable
  rename drawgrid --> do_draw_grid
  give the display frames usefull names frame_lat, ...
  change handling of WP-types to lowercase
  change order for directories reading icons
  always read inconfile

  Revision 1.5  1994/06/07 11:25:45  tweety
  set debug levels more detailed

  Revision 1.4  2005/11/06 18:37:38  tweety
  shortened map selection code
  coordinate_string2gdouble:
   - fixed missing format
    - changed interface to return gdouble
    change -D option to reflect debuglevels
    Added more debug Statements for Level>50
    move map handling to to seperate file
    speedup memory reservation for map-structure
    Add code for automatic loading of maps from system
    DATA/maps/.. Directory
    changed length of mappath from 400 to 2048 chars

  Revision 1.3  2005/10/30 06:57:07  tweety
  eliminate ols tracks.h file

  Revision 1.2  2005/10/11 08:28:35  tweety
  gpsdrive:
  - add Tracks(MySql) displaying
  - reindent files modified
  - Fix setting of Color for Grid
  - poi Text is different in size depending on Number of POIs shown on
    screen

  geoinfo:
   - get Proxy settings from Environment
   - create tracks Table in Database and fill it
     this separates Street Data from Track Data
   - make geoinfo.pl download also Opengeodb Version 2
   - add some poi-types
   - Split off Filling DB with example Data
   - extract some more Funtionality to Procedures
   - Add some Example POI for Kirchheim(Munich) Area
   - Adjust some Output for what is done at the moment
   - Add more delayed index generations 'disable/enable key'
   - If LANG=*de_DE* then only impert europe with --all option
   - WDB will import more than one country if you wish
   - add more things to be done with the --all option

  Revision 1.1  2005/08/14 10:05:24  tweety
  added new Files src/draw_tracks.?

  Revision 1.16  2005/05/24 08:35:25  tweety

*/


/*
 * tracks_ support module: display
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gpsdrive.h"
#include "draw_tracks.h"
#include "config.h"
#include "gettext.h"
#include <time.h>
#include <sys/time.h>
#include "icons.h"

#include "gettext.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint maploaded;
extern gint importactive;
extern gint zoom;
extern gint showroute, routeitems;
extern gint nightmode, isnight, disableisnight;
extern GdkColor red;
extern GdkColor black;
extern GdkColor white;
extern GdkColor blue;
extern GdkColor nightcolor;
extern GdkColor mygray;
extern GdkColor textback;
extern GdkColor textbacknew;
extern GdkColor darkgrey;
extern GdkColor grey;

extern gdouble current_long, current_lat;
extern gint debug, mydebug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats,
  *drawing_miniimage;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;

#include "mysql.h"

extern MYSQL mysql;
extern MYSQL_RES *res;
extern MYSQL_ROW row;

// keep actual visible Tracks in Memory
tracks_struct *tracks_list;
glong tracks_list_count;	// current number of tracks to count
glong tracks_list_limit = -1;	// max allowed imdex (if you need more you have to alloc memory)

gint tracks_draw = FALSE;

gchar tracks_label_font[100];
//GdkColor tracks_colorv;



typedef struct
{
  gint track_type_id;
  gchar name[80];
  gchar icon_name[80];
  GdkColor *color;
  GdkPixbuf *icon;
} tracks_type_struct;
#define tracks_type_list_max 500
tracks_type_struct tracks_type_list[tracks_type_list_max];
int tracks_type_list_count = 0;

/* ******************************************************************   */

void tracks_rebuild_list (void);
void get_tracks_type_list (void);


/* *******************************************************
 */
void
tracks_init (void)
{
  if (mydebug >50)
    printf ("Tracks init\n");

  get_tracks_type_list ();

  tracks_list_limit = 10000;
  tracks_list = g_new (tracks_struct, tracks_list_limit);
  if (tracks_list == NULL)
    {
      g_print ("Error: Cannot allocate Memory for %ld tracks\n",
	       tracks_list_limit);
      tracks_list_limit = -1;
      return;
    }

  tracks_rebuild_list ();
}


/* *********************************************************
 */
gdouble tracks_lat_lr = 0, tracks_lon_lr = 0;
gdouble tracks_lat_ul = 0, tracks_lon_ul = 0;

void
tracks_check_if_moved_reset (void)
{
  tracks_lat_lr = 0, tracks_lon_lr = 0;
  tracks_lat_ul = 0, tracks_lon_ul = 0;
}

int
tracks_check_if_moved (void)
{
  gdouble lat_lr, lon_lr;
  gdouble lat_ul, lon_ul;

  if (tracks_lat_lr == 0 && tracks_lon_lr == 0 &&
      tracks_lat_ul == 0 && tracks_lon_ul == 0)
    return 1;

  calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, zoom);
  calcxytopos (0, 0, &lat_ul, &lon_ul, zoom);

  if (tracks_lat_lr == lat_lr && tracks_lon_lr == lon_lr &&
      tracks_lat_ul == lat_ul && tracks_lon_ul == lon_ul)
    return 0;
  if (mydebug >50)
    printf ("tracks_check_if_moved: Tracks Display moved\n");
  return 1;
}


/* ****************************************************************** */
/* get a list of all possible types and load their icons */
void
get_tracks_type_list (void)
{
  char sql_query[3000];

  if (!usesql)
    return;

  if (mydebug >50)
    printf ("get_tracks_type_list ()\n");

  g_snprintf (sql_query, sizeof (sql_query),
	      "SELECT track_type_id,name,color FROM tracks_type ORDER BY track_type_id");

  if (dl_mysql_query (&mysql, sql_query))
    {
      fprintf (stderr, "get_tracks_type_list: Error in query: %s\n",
	       dl_mysql_error (&mysql));
      return;
    }

  if (!(res = dl_mysql_store_result (&mysql)))
    {
      fprintf (stderr, "Error in store results: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
      return;
    }

  while ((row = dl_mysql_fetch_row (res)))
    {
      tracks_type_list_count = (gint) g_strtod (row[0], NULL);
      if (tracks_type_list_count < tracks_type_list_max)
	{
	  tracks_type_list[tracks_type_list_count].
	    track_type_id = tracks_type_list_count;

	  if (NULL == row[1])
	    {
	      tracks_type_list[tracks_type_list_count].name[0] = '\0';
	    }
	  else
	    {
	      // memorize name
	      g_strlcpy (tracks_type_list
			 [tracks_type_list_count].name,
			 row[1],
			 sizeof (tracks_type_list
				 [tracks_type_list_count].name));
	    }

	  // memorize color
	  if (tracks_type_list_count == 1)	// Actual Track
	    {
	      tracks_type_list[tracks_type_list_count].color = &blue;
	    }
	  else if (tracks_type_list_count == 2)	// Old Tracks 
	    {
	      tracks_type_list[tracks_type_list_count].color = &green;
	    }
	  else if (tracks_type_list_count == 3)	// Error in Track
	    {
	      tracks_type_list[tracks_type_list_count].color = &red;
	    }
	  else
	    {
	      tracks_type_list[tracks_type_list_count].color = &red;
	    }
	}
    }

  if (!dl_mysql_eof (res))
    {
      fprintf (stderr, "Error in dl_mysql_eof: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
    }

  dl_mysql_free_result (res);
  res = NULL;

}


/* *******************************************************
 * if zoom, xoff, yoff or map are changed 
 TODO: use the real datatype for reading from database
 (dont convert string to double)
*/
void
tracks_rebuild_list (void)
{
  char sql_query[5000];
  char sql_where[5000];
  struct timeval t;
  int r, rges;
  time_t ti;

  gdouble lat_ul, lon_ul;
  gdouble lat_ll, lon_ll;
  gdouble lat_ur, lon_ur;
  gdouble lat_lr, lon_lr;
  gdouble lat_min, lon_min;
  gdouble lat_max, lon_max;
  gdouble lat_mid, lon_mid;

  if (!usesql)
    return;

  if (!maploaded)
    return;

  if (importactive)
    return;

  if (mydebug >50)
    printf ("tracks_rebuild_list: Start\t\t\t\t\t\tvvvvvvvvvvvvvvvvvvvvvv\n");

  if (!tracks_draw)
    {
      if (mydebug >50)
	printf ("tracks_rebuild_list: tracks_draw is off\n");
      return;
    }

  {				// calculate the start and stop for lat/lon according to the displayed section
    calcxytopos (0, 0, &lat_ul, &lon_ul, zoom);
    calcxytopos (0, SCREEN_Y, &lat_ll, &lon_ll, zoom);
    calcxytopos (SCREEN_X, 0, &lat_ur, &lon_ur, zoom);
    calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, zoom);

    lat_min = min (lat_ll, lat_ul);
    lat_max = max (lat_lr, lat_ur);
    lon_min = min (lon_ll, lon_ul);
    lon_max = max (lon_lr, lon_ur);

    lat_mid = (lat_min + lat_max) / 2;
    lon_mid = (lon_min + lon_max) / 2;
  }

  gdouble tracks_posx1, tracks_posy1;
  gdouble tracks_posx2, tracks_posy2;


  gettimeofday (&t, NULL);
  ti = t.tv_sec + t.tv_usec / 1000000.0;


  {				// Limit the select with WHERE min_lat<lat<max_lat AND min_lon<lon<max_lon
    g_snprintf (sql_where, sizeof (sql_where),
		"WHERE \n"
		"\t\t ( \n"
		"\t\t   ( ( lat1 BETWEEN %.6f AND %.6f ) AND ( lon1 BETWEEN %.6f AND %.6f ) ) \n"
		"\t\t   OR \n"
		"\t\t   ( ( lat2 BETWEEN %.6f AND %.6f ) AND ( lon2 BETWEEN %.6f AND %.6f ) ) \n"
		"\t\t ) \n",
		lat_min, lat_max, lon_min, lon_max,
		lat_min, lat_max, lon_min, lon_max);
    g_strdelimit (sql_where, ",", '.');	// For different LANG
    if (mydebug >50)
      {
	// printf ("TRACKS mysql where: %s\n", sql_where );
	printf ("tracks_rebuild_list: TRACKS mapscale: %ld\n", mapscale);
      }
  }




  // Diplay ONLY those TRACKS which are tracks.scale_min <= level <=tracks.scale_max for actual scale
  g_snprintf (sql_query, sizeof (sql_query),
	      // "SELECT lat,lon,alt,track_type_id,proximity "
	      "SELECT lat1,lon1,lat2,lon2,name,track_type_id "
	      "FROM tracks " "%s LIMIT 200000", sql_where);

  if (mydebug >50)
    printf ("tracks_rebuild_list: TRACKS mysql query: %s\n", sql_query);

  if (dl_mysql_query (&mysql, sql_query))
    {
      fprintf (stderr, "tracks_rebuild_list: Error in query: %s\n",
	       dl_mysql_error (&mysql));
      return;
    }

  if (!(res = dl_mysql_store_result (&mysql)))
    {
      fprintf (stderr, "Error in store result: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
      return;
    }

  if (mydebug >50)
    printf ("tracks_rebuild_list: processing rows\n");
  rges = r = 0;
  tracks_list_count = -1;
  while ((row = dl_mysql_fetch_row (res)))
    {
      rges++;
      gdouble lat1, lon1;
      gdouble lat2, lon2;
      lat1 = g_strtod (row[0], NULL);
      lon1 = g_strtod (row[1], NULL);
      lat2 = g_strtod (row[2], NULL);
      lon2 = g_strtod (row[3], NULL);


      calcxy (&tracks_posx1, &tracks_posy1, lon1, lat1, zoom);
      calcxy (&tracks_posx2, &tracks_posy2, lon2, lat2, zoom);

      if (posxy_on_screen (tracks_posx1, tracks_posy1) ||
	  posxy_on_screen (tracks_posx2, tracks_posy2))
	{
	  // get next free mem for tracks
	  tracks_list_count++;
	  if (tracks_list_count >= tracks_list_limit)
	    {
	      tracks_list_limit = tracks_list_count + 1000;
	      if (mydebug >50)
		printf
		  ("tracks_rebuild_list: renewmemory for track list: %ld\n",
		   tracks_list_limit);

	      tracks_list =
		g_renew (tracks_struct, tracks_list, tracks_list_limit);
	      if (NULL == tracks_list)
		{
		  g_print ("Error: Cannot allocate Memory for %ld tracks\n",
			   tracks_list_limit);
		  tracks_list_limit = -1;
		  return;
		}
	    }
	  if (mydebug>50)
	    {
	      printf ("tracks_rebuild_list: %ld(%ld)\t", tracks_list_count,
		      tracks_list_limit);
	      printf ("pos: (%.4f ,%.4f) (%.4f ,%.4f)\n", lat1, lon1, lat2,
		      lon2);
	    }

	  // Save retrieved tracks information into structure
	  (tracks_list + tracks_list_count)->lat1 = lat1;
	  (tracks_list + tracks_list_count)->lon1 = lon1;
	  (tracks_list + tracks_list_count)->lat2 = lat2;
	  (tracks_list + tracks_list_count)->lon2 = lon2;
	  (tracks_list + tracks_list_count)->x1 = tracks_posx1;
	  (tracks_list + tracks_list_count)->y1 = tracks_posy1;
	  (tracks_list + tracks_list_count)->x2 = tracks_posx2;
	  (tracks_list + tracks_list_count)->y2 = tracks_posy2;
	  (tracks_list + tracks_list_count)->track_type_id =
	    (gint) g_strtod (row[5], NULL);

	  if (NULL == (tracks_list + tracks_list_count)->name)
	    (tracks_list + tracks_list_count)->name[0] = '\0';
	  else
	    g_strlcpy ((tracks_list + tracks_list_count)->name,
		       row[4],
		       sizeof ((tracks_list + tracks_list_count)->name));

	}
    }


  if (mydebug >50)
    {				// print time for getting Data
      gettimeofday (&t, NULL);
      ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
      g_print (_("%ld(%d) rows read in %.2f seconds\n"),
	       tracks_list_count, rges, (gdouble) ti);
    }

  {				/* remember where the data belongs to */
    tracks_lat_lr = lat_lr;
    tracks_lon_lr = lon_lr;
    tracks_lat_ul = lat_ul;
    tracks_lon_ul = lon_ul;
  }

  if (!dl_mysql_eof (res))
    {
      fprintf (stderr, "Error in dl_mysql_eof: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
    }

  dl_mysql_free_result (res);
  res = NULL;

  if (mydebug >50)
    printf ("tracks_rebuild_list: End\t\t\t\t\t\t^^^^^^^^^^^^^^^^^^^^^^\n");
}




/* *******************************************************
 * draw tracks_ on image 
 TODO: find free space on drawing area. So the Text doesn't overlap
*/
void
tracks_draw_list (void)
{
  //  gint t;

  GdkSegment *gdks_tracks;
  gint gdks_tracks_count = -1;
  gint gdks_tracks_max = tracks_list_count + 1;

  gint i;

  if (!usesql)
    return;

  if (importactive)
    return;

  if (!(tracks_draw))
    {
      tracks_check_if_moved_reset ();
      if (mydebug >50)
	printf ("tracks_draw is off\n");
      return;
    }

  if (mydebug >50)
    printf ("tracks_draw\n");

  if (tracks_check_if_moved ())
    tracks_rebuild_list ();

  gdks_tracks_max = tracks_list_count + 1;


  if (0 >= gdks_tracks_max)
    return;

  if (mydebug >50)
    printf ("tracks_draw %ld segments\n", tracks_list_count);

  gdks_tracks = g_new0 (GdkSegment, gdks_tracks_max);
  if (NULL == gdks_tracks)
    {
      printf ("Problem reserving Memory for %d track segments\n",
	      gdks_tracks_max);
      gdks_tracks_max = -1;
      return;
    }

  if (mydebug >50)
    printf ("created gdk struct for %d segments\n", gdks_tracks_max);

  /* ------------------------------------------------------------------ */
  /*  draw gdks_tracks_list tracks */
  for (i = 0; i < tracks_list_count; i++)
    {
      gdouble posx1, posy1;
      gdouble posx2, posy2;

      posx1 = (tracks_list + i)->x1;
      posy1 = (tracks_list + i)->y1;
      posx2 = (tracks_list + i)->x2;
      posy2 = (tracks_list + i)->y2;


      /*      if ( mydebug>50 ) {
       * //printf("    a1 %f,%f\n",  posx1,posy1);
       * //printf("    a2 %f,%f\n",  posx2,posy2);
       * 
       * printf ("TRACKS Draw: %f %f -> %f, %f            %s\n",
       * (tracks_list + i)->lat1, (tracks_list + i)->lon1,
       * (tracks_list + i)->lat2, (tracks_list + i)->lon2,
       * (tracks_list + i)->name
       * );
       * }
       */
      if (posxy_on_screen (posx1, posy1) || posxy_on_screen (posx2, posy2))
	{
	  /*
	   * if (mydebug >50) {
	   * printf ("       Draw: %f %f -> %f, %f\n",
	   * (tracks_list + i)->x1,   (tracks_list + i)->y1,
	   * (tracks_list + i)->x2,   (tracks_list + i)->y2
	   * );
	   * }
	   */

	  // Alloc Memory if we need more
	  gdks_tracks_count++;
	  if (gdks_tracks_count >= gdks_tracks_max)
	    {
	      gdks_tracks_max = gdks_tracks_count + 1000;
	      g_renew (GdkSegment, gdks_tracks, gdks_tracks_max);
	      if (NULL == tracks_list)
		{
		  g_print
		    ("Error: Cannot allocate Memory for %d track-gdk segments\n",
		     gdks_tracks_max);
		  gdks_tracks_max = -1;
		  return;
		}
	    }

	  (gdks_tracks + gdks_tracks_count)->x1 = posx1;
	  (gdks_tracks + gdks_tracks_count)->y1 = posy1;
	  (gdks_tracks + gdks_tracks_count)->x2 = posx2;
	  (gdks_tracks + gdks_tracks_count)->y2 = posy2;

	  if (mydebug >90)
	    {
	      char beschrift[120];
	      g_snprintf (beschrift, sizeof (beschrift),
			  "(%.4f ,%.4f)\n(%.4f ,%.4f)\n%s",
			  (tracks_list + i)->lat1,
			  (tracks_list + i)->lon1,
			  (tracks_list + i)->lat2, (tracks_list + i)->lon2,
			  //(tracks_list + i)->track_type_id,
			  (tracks_list + i)->name);

	      draw_text_with_box (posx1 +
				  (posx2 - posx1) / 2,
				  posy1 + (posy2 - posy1) / 2, beschrift);
	      //draw_text_with_box(posx1+(posx2-posx1)/2,posy1+(posy2-posy1)/2,(tracks_list + i)->name);
	      if (0)
		{
		  draw_small_plus_sign (posx1 +
					(posx2 -
					 posx1) / 2,
					posy1 + (posy2 - posy1) / 2);
		  draw_text_with_box (posx1, posy1 - 15,
				      (tracks_list + i)->name);
		  draw_text_with_box (posx2, posy2 + 15,
				      (tracks_list + i)->name);
		}
	      gdk_gc_set_foreground (kontext, &red);
	      draw_small_plus_sign (posx1, posy1);
	      draw_small_plus_sign (posx2, posy2);

	    }

	  if (mydebug >50)
	    {
	      //          printf("    1 %f,%f\n",  posx1,posy1);
	      //          printf("    2 %f,%f\n",  posx2,posy2);
	    }
	}

      // draw it if last or track_type_id changes 
      if ((i == tracks_list_count - 1) ||
	  ((tracks_list + i)->track_type_id !=
	   (tracks_list + i + 1)->track_type_id))
	{
	  /*
	   * if ( debug )
	   * printf("Drawing %d segments\n",gdks_tracks_count);
	   */
	  int track_id = (tracks_list + i)->track_type_id;
	  if (track_id < tracks_type_list_max)
	    {
	      if (NULL == tracks_type_list[track_id].color)
		{
		  if (mydebug >50)
		    fprintf (stderr,
			     "ERROR: Undefined Track Color for Segment %d, track_id %d\n",
			     i, track_id);

		  gdk_gc_set_foreground (kontext, &green);
		  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
		}
	      else
		{
		  gdk_gc_set_foreground (kontext,
					 tracks_type_list[track_id].color);
		  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
		}
	    }
	  else
	    {
	      gdk_gc_set_foreground (kontext, &red);
	      gdk_gc_set_line_attributes (kontext, 5, 0, 0, 0);
	    }

	  gdk_draw_segments (drawable, kontext,
			     (GdkSegment *) gdks_tracks,
			     gdks_tracks_count + 1);
	  gdks_tracks_count = -1;
	}
    }

  g_free (gdks_tracks);

  if (mydebug>50)
    printf
      ("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}


/* *******************************************************
 * query all Info for tracks in area arround lat/lon
*/
void
tracks_query_area (gdouble lat1, gdouble lon1, gdouble lat2, gdouble lon2)
{
  gint i;
  printf ("Query Tracks: %f ... %f , %f ... %f\n", lat1, lat2, lon1, lon2);

  for (i = 0; i < tracks_list_count; i++)
    {
      // TODO:: make real checks if in rectangle or crossing rectangle
      if (((lat1 <= (tracks_list + i)->lat1)
	   && ((tracks_list + i)->lat1 <= lat2)
	   && (lon1 <= (tracks_list + i)->lon1)
	   && ((tracks_list + i)->lon1 <= lon2))
	  || ((lat1 <= (tracks_list + i)->lat2)
	      && ((tracks_list + i)->lat2 <= lat2)
	      && (lon1 <= (tracks_list + i)->lon2)
	      && ((tracks_list + i)->lon2 <= lon2)))
	{
	  printf ("Tracks: %d: %f,%f --> %f,%f :%s\t",
		  i,
		  (tracks_list + i)->lat1, (tracks_list + i)->lon1,
		  (tracks_list + i)->lat2, (tracks_list + i)->lon2,
		  (tracks_list + i)->name);
	  gint track_type_id = (tracks_list + i)->track_type_id;

	  printf ("Type: %s\t", tracks_type_list[track_type_id].name);
	  printf ("%s\n", (tracks_list + i)->comment);
	}
    }

}
