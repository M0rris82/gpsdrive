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
  Revision 1.21  1994/06/08 13:02:31  tweety
  adjust debug levels

  Revision 1.20  1994/06/07 11:25:45  tweety
  set debug levels more detailed

  Revision 1.19  2005/11/06 17:24:26  tweety
  shortened map selection code
  coordinate_string2gdouble:
   - fixed missing format
   - changed interface to return gdouble
  change -D option to reflect debuglevels
  Added more debug Statements for Level>50
  move map handling to to seperate file
  speedup memory reservation for map-structure
  Add code for automatic loading of maps from system DATA/maps/.. Directory
  changed length of mappath from 400 to 2048 chars

  Revision 1.18  2005/10/11 08:28:35  tweety
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

  Revision 1.17  2005/08/14 09:47:17  tweety
  seperate tracks into it own table in geoinfo database
  move Info's from TODO abaout geoinfo DB to Man Page
  rename poi.pl to geoinfo.pl

  Revision 1.16  2005/05/24 08:35:25  tweety
  move track splitting to its own function +sub track_add($)
  a little bit more error handling
  earth_distance somtimes had complex inumbers as result
  implemented streets_check_if_moved_reset which is called when you toggle the draw streets button
  this way i can re-read all currently displayed streets from the DB
  fix minor array iindex counting bugs
  add some content to the comment column

  Revision 1.15  2005/05/15 07:00:51  tweety
  new Keystroke p adds an instant waypoint at cursor position
  new Keystroke q querys information for thenearest waypoints and street endpoints

  Revision 1.14  2005/05/13 21:27:48  tweety
  delete order by to speed up database actions

  Revision 1.13  2005/04/20 23:33:49  tweety
  reformatted source code with anjuta
  So now we have new indentations

  Revision 1.12  2005/04/13 19:58:31  tweety
  renew indentation to 4 spaces + tabstop=8

  Revision 1.11  2005/04/12 06:14:20  tweety
  poi.c:
  Added Full Path Search for Icons
  streets.c:
  implement basic colors for streets
  added text to streets in debug mode

  Revision 1.10  2005/04/10 21:50:50  tweety
  reformatting c-sources

  Revision 1.9  2005/04/10 00:10:32  tweety
  added gpsd: to gpsd related debug output
  changed plus to a small + in streets.c

  Revision 1.8  2005/04/07 06:35:01  tweety
  Error handling for g_renew
  correct to extern MYSQL mysql;
  start getting color from streets_type

  Revision 1.7  2005/03/29 01:59:01  tweety
  another set of minor Bugfixes

  Revision 1.6  2005/03/27 21:25:46  tweety
  separating map_import from gpsdrive.c

  Revision 1.5  2005/03/27 00:44:42  tweety
  separated poi_type_list and streets_type_list
  and therefor renaming the fields
  added drop index before adding one
  poi.*: a little bit more error handling
  disabling poi and streets if sql is disabled
  changed som print statements from \n.... to ...\n
  changed some debug statements from debug to mydebug

  Revision 1.4  2005/02/22 08:18:51  tweety
  change leveling system to simpler scale marking for decission what to show on display
  column_names(DBFuncs.pm get data from Database
  added functions add_index drop_index
  added language to type Database
  for some Data split unpack and mirror Directories
  for some add lat/lon min/max to get faster import for testing
  added POI::DBFuncs::segments_add; this will later be the point to do 
  some excerptions and combinations on the street data

  Revision 1.3  2005/02/17 09:46:34  tweety
  minor changes

  Revision 1.2  2005/02/13 22:57:00  tweety
  WDB Support

  Revision 1.1  2005/02/13 14:06:54  tweety
  start street randering functions. reading from the database streets
  and displaying it on the screen
  improve a little bit in the sql-queries
  fixed linewidth settings in draw_cross

  Revision 0.0  2005/02/12 20:14:14  tweety

*/


/*
 * streets_ support module: display
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gpsdrive.h"
#include "streets.h"
#include "config.h"
#include "gettext.h"
#include <time.h>
#include <sys/time.h>
#include "icons.h"

#if HAVE_LOCALE_H
#include <locale.h>
#else
# define setlocale(Category, Locale)
#endif
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
extern gint mydebug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats,
  *drawing_miniimage;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;

#include "mysql.h"

extern MYSQL mysql;
extern MYSQL_RES *res;
extern MYSQL_ROW row;

// keep actual visible Streets in Memory
streets_struct *streets_list;
glong streets_list_count;	// current number of streets to count
glong streets_list_limit = -1;	// max allowed imdex (if you need more you have to alloc memory)

gint streets_draw = FALSE;

gchar streets_label_font[100];
//GdkColor streets_colorv;



typedef struct
{
  gint streets_type_id;
  gchar name[80];
  gchar icon_name[80];
  GdkColor *color;
  GdkPixbuf *icon;
} streets_type_struct;
#define streets_type_list_max 500
streets_type_struct streets_type_list[streets_type_list_max];
int streets_type_list_count = 0;

/* ******************************************************************   */

void streets_rebuild_list (void);
void get_streets_type_list (void);


/* ****************************************************************** 
   check if the given position is inside the screen
*/
int
posxy_on_screen (gdouble posx, gdouble posy)
{
  if ((posx >= 0) && (posx < SCREEN_X) && (posy >= 0) && (posy < SCREEN_Y))
    return TRUE;
  return FALSE;
}



/* *******************************************************
 */
void
streets_init (void)
{
  if ( mydebug > 50 )
    printf ("Streets init\n");

  get_streets_type_list ();

  streets_list_limit = 10000;
  streets_list = g_new (streets_struct, streets_list_limit);
  if (streets_list == NULL)
    {
      g_print ("Error: Cannot allocate Memory for %ld streets\n",
	       streets_list_limit);
      streets_list_limit = -1;
      return;
    }

  streets_rebuild_list ();
}


/* *********************************************************
 */
gdouble streets_lat_lr = 0, streets_lon_lr = 0;
gdouble streets_lat_ul = 0, streets_lon_ul = 0;

void
streets_check_if_moved_reset (void)
{
  streets_lat_lr = 0, streets_lon_lr = 0;
  streets_lat_ul = 0, streets_lon_ul = 0;
}

int
streets_check_if_moved (void)
{
  gdouble lat_lr, lon_lr;
  gdouble lat_ul, lon_ul;

  if (streets_lat_lr == 0 && streets_lon_lr == 0 &&
      streets_lat_ul == 0 && streets_lon_ul == 0)
    return 1;

  calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, zoom);
  calcxytopos (0, 0, &lat_ul, &lon_ul, zoom);

  if (streets_lat_lr == lat_lr && streets_lon_lr == lon_lr &&
      streets_lat_ul == lat_ul && streets_lon_ul == lon_ul)
    return 0;
  if ( mydebug > 50 )
    printf ("streets_check_if_moved: Streets Display moved\n");
  return 1;
}


/* ****************************************************************** */
/* get a list of all possible types and load their icons */
void
get_streets_type_list (void)
{
  char sql_query[3000];

  if (!usesql)
    return;

  if ( mydebug > 50 )
    printf ("get_streets_type_list ()\n");

  g_snprintf (sql_query, sizeof (sql_query),
	      "SELECT streets_type_id,name,color FROM streets_type ORDER BY streets_type_id");

  if ( mydebug > 20)
    printf ("QUERY:%s\n",sql_query);

  if (dl_mysql_query (&mysql, sql_query))
    {
      fprintf (stderr, "get_streets_type_list: Error in query: %s\n",
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
	if ( mydebug > 50 )
	    printf ("got ROW %s,	%s,	%s\n",
		    row[0],row[1],row[2]);

      streets_type_list_count = (gint) g_strtod (row[0], NULL);
      if (streets_type_list_count < streets_type_list_max)
	{
	  streets_type_list[streets_type_list_count].
	    streets_type_id = streets_type_list_count;

	  if ( NULL == row[1] )
	    {
	      streets_type_list[streets_type_list_count].name[0] = '\0';
	    }
	  else
	    {
	      // memorize name
	      g_strlcpy (streets_type_list
			 [streets_type_list_count].name,
			 row[1],
			 sizeof (streets_type_list
				 [streets_type_list_count].name));
	    }

	  // memorize color
	  if (streets_type_list_count == 1)
	    {
	      streets_type_list[streets_type_list_count].color = &red;
	    }
	  else if (streets_type_list_count == 2)
	    {
	      streets_type_list[streets_type_list_count].color = &blue;
	    }
	  else if (streets_type_list_count == 3)
	    {
	      streets_type_list[streets_type_list_count].color = &green;
	    }
	  else if (streets_type_list_count == 4)
	    {
	      streets_type_list[streets_type_list_count].color = &yellow;
	    }
	  else if (NULL == row[2])
	    {
	      streets_type_list[streets_type_list_count].color = &red;
	    }
	  else if (!strcmp (row[2], "FF0000"))
	    {
	      streets_type_list[streets_type_list_count].color = &red;
	    }
	  else if (!strcmp (row[2], "00FF00"))
	    {
	      streets_type_list[streets_type_list_count].color = &green;
	    }
	  else if (!strcmp (row[2], "0000FF"))
	    {
	      streets_type_list[streets_type_list_count].color = &blue;
	    }
	  else if (!strcmp (row[2], "FFFF00"))
	    {
	      streets_type_list[streets_type_list_count].color = &yellow;
	    }
	  else if (!strcmp (row[2], "111111"))
	    {
	      streets_type_list[streets_type_list_count].color = &darkgrey;
	    }
	  else
	    {
	      streets_type_list[streets_type_list_count].color = &red;
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
streets_rebuild_list (void)
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

  if ( mydebug > 50 )
    printf
      ("streets_rebuild_list: Start\t\t\t\t\t\tvvvvvvvvvvvvvvvvvvvvvv\n");

  if (!streets_draw)
    {
      if ( mydebug > 50 )
	printf ("streets_rebuild_list: streets_draw is off\n");
      return;
    }

  { // calculate the start and stop for lat/lon according to the displayed section
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

  gdouble streets_posx1, streets_posy1;
  gdouble streets_posx2, streets_posy2;


  gettimeofday (&t, NULL);
  ti = t.tv_sec + t.tv_usec / 1000000.0;


  { // Limit the select with WHERE min_lat<lat<max_lat AND min_lon<lon<max_lon
    g_snprintf (sql_where, sizeof (sql_where),
		"WHERE \n"
		"\t\t ( \n"
		"\t\t   ( ( lat1 BETWEEN %.6f AND %.6f ) AND ( lon1 BETWEEN %.6f AND %.6f ) ) \n"
		"\t\t   OR \n"
		"\t\t   ( ( lat2 BETWEEN %.6f AND %.6f ) AND ( lon2 BETWEEN %.6f AND %.6f ) ) \n"
		"\t\t ) \n"
		"\t\t AND \n"
		"\t\t ( %ld BETWEEN scale_min AND scale_max) \n",
		lat_min, lat_max, lon_min, lon_max,
		lat_min, lat_max, lon_min, lon_max, mapscale);
    g_strdelimit (sql_where, ",", '.');	// For different LANG
    if ( mydebug > 50 )
      {
	// printf ("STREETS mysql where: %s\n", sql_where );
	printf ("streets_rebuild_list: STREETS mapscale: %ld\n", mapscale);
      }
  }




  // Diplay ONLY those STREETS which are streets.scale_min <= level <=streets.scale_max for actual scale
  g_snprintf (sql_query, sizeof (sql_query),
	      // "SELECT lat,lon,alt,streets_type_id,proximity "
	      "SELECT lat1,lon1,lat2,lon2,name,streets_type_id,comment "
	      "FROM streets " "%s LIMIT 200000", sql_where);

  if ( mydebug > 50 )
    printf ("streets_rebuild_list: STREETS mysql query: %s\n", sql_query);

  if (dl_mysql_query (&mysql, sql_query))
    {
      fprintf (stderr, "streets_rebuild_list: Error in query: %s\n",
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

  if ( mydebug > 50 )
    printf ("streets_rebuild_list: processing rows\n");
  rges = r = 0;
  streets_list_count = -1;
  while ((row = dl_mysql_fetch_row (res)))
    {
      rges++;
      gdouble lat1, lon1;
      gdouble lat2, lon2;
      lat1 = g_strtod (row[0], NULL);
      lon1 = g_strtod (row[1], NULL);
      lat2 = g_strtod (row[2], NULL);
      lon2 = g_strtod (row[3], NULL);


      calcxy (&streets_posx1, &streets_posy1, lon1, lat1, zoom);
      calcxy (&streets_posx2, &streets_posy2, lon2, lat2, zoom);

      if (posxy_on_screen (streets_posx1, streets_posy1) ||
	  posxy_on_screen (streets_posx2, streets_posy2))
	{
	  // get next free mem for streets
	  streets_list_count++;
	  if (streets_list_count >= streets_list_limit)
	    {
	      streets_list_limit = streets_list_count + 1000;
	      if ( mydebug > 50 )
		printf
		  ("streets_rebuild_list: renew memory for street list: %ld\n",
		   streets_list_limit);

	      streets_list =
		g_renew (streets_struct, streets_list, streets_list_limit);
	      if (NULL == streets_list)
		{
		  g_print ("Error: Cannot allocate Memory for %ld streets\n",
			   streets_list_limit);
		  streets_list_limit = -1;
		  return;
		}
	    }
	  if ( mydebug > 90)
	    {
		printf ("streets_rebuild_list: %ld(%ld)\t", streets_list_count, streets_list_limit);
		printf ("pos: (%.4f ,%.4f) (%.4f ,%.4f)\n", lat1, lon1, lat2, lon2);
	    }

	  // Save retrieved streets information into structure
	  (streets_list + streets_list_count)->lat1 = lat1;
	  (streets_list + streets_list_count)->lon1 = lon1;
	  (streets_list + streets_list_count)->lat2 = lat2;
	  (streets_list + streets_list_count)->lon2 = lon2;
	  (streets_list + streets_list_count)->x1 = streets_posx1;
	  (streets_list + streets_list_count)->y1 = streets_posy1;
	  (streets_list + streets_list_count)->x2 = streets_posx2;
	  (streets_list + streets_list_count)->y2 = streets_posy2;
	  (streets_list + streets_list_count)->streets_type_id =
	    (gint) g_strtod (row[5], NULL);

	  if ( NULL == row[4] )
	    (streets_list + streets_list_count)->name[0] = '\0';
	  else
	    g_strlcpy ((streets_list + streets_list_count)->name,
		       row[4],
		       sizeof ((streets_list + streets_list_count)->name));

	  if ( NULL == row[6] )
	    (streets_list + streets_list_count)->comment[0] = '\0';
	  else
	    g_strlcpy ((streets_list + streets_list_count)->comment,
		       row[6],
		       sizeof ((streets_list + streets_list_count)->comment));
	}
    }


  if ( mydebug > 50 )
    {				// print time for getting Data
      gettimeofday (&t, NULL);
      ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
      g_print (_("%ld(%d) rows read in %.2f seconds\n"),
	       streets_list_count, rges, (gdouble) ti);
    }

  {				/* remember where the data belongs to */
    streets_lat_lr = lat_lr;
    streets_lon_lr = lon_lr;
    streets_lat_ul = lat_ul;
    streets_lon_ul = lon_ul;
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

  if ( mydebug > 50 )
    printf ("streets_rebuild_list: End\t\t\t\t\t\t^^^^^^^^^^^^^^^^^^^^^^\n");
}




/* ****************************************************************** */
void
draw_text_with_box (gdouble posx, gdouble posy, gchar * name)
{
  gint width, height;
  char txt[500];
  PangoFontDescription *pfd;
  PangoLayout *streets_label_layout;
  gint k, k2;


  // Draw Text Label with name
  g_strlcpy (txt, name, sizeof (txt));
  gdk_gc_set_foreground (kontext, &textback);

  streets_label_layout = gtk_widget_create_pango_layout (drawing_area, txt);
  if (pdamode)
    pfd = pango_font_description_from_string ("Sans 8");
  else
    pfd = pango_font_description_from_string ("Sans 11");
  pango_layout_set_font_description (streets_label_layout, pfd);
  pango_layout_get_pixel_size (streets_label_layout, &width, &height);
  k = width + 4;
  k2 = height;

  gdk_gc_set_function (kontext, GDK_COPY);

  gdk_gc_set_function (kontext, GDK_AND);

  { // Draw rectangle arround Text
    //gdk_gc_set_foreground (kontext, &textbacknew);
    gdk_draw_rectangle (drawable, kontext, 1,
			posx + 13, posy - k2 / 2, k + 1, k2);
    gdk_gc_set_function (kontext, GDK_COPY);
    gdk_gc_set_foreground (kontext, &black);
    gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
    gdk_draw_rectangle (drawable, kontext, 0,
			posx + 12, posy - k2 / 2 - 1, k + 2, k2);
  }

  /* prints in pango */

  streets_label_layout = gtk_widget_create_pango_layout (drawing_area, txt);
  pfd = pango_font_description_from_string (streets_label_font);
  pango_layout_set_font_description (streets_label_layout, pfd);

  gdk_draw_layout_with_colors (drawable, kontext,
			       posx + 15, posy - k2 / 2,
			       streets_label_layout, &black, NULL);
  if (streets_label_layout != NULL)
    g_object_unref (G_OBJECT (streets_label_layout));
  /* freeing PangoFontDescription, cause it has been copied by prev. call */
  pango_font_description_free (pfd);
}




/* *******************************************************
 * draw streets_ on image 
 TODO: find free space on drawing area. So the Text doesn't overlap
*/
void
streets_draw_list (void)
{
  GdkSegment *gdks_streets;
  gint gdks_streets_count = -1;
  gint gdks_streets_max = streets_list_count + 1;

  gint i;

  if (!usesql)
    return;

  if (importactive)
    return;

  if (!(streets_draw))
    {
      streets_check_if_moved_reset ();
      if ( mydebug > 50 )
	printf ("streets_draw is off\n");
      return;
    }

  if ( mydebug > 50 )
    printf ("streets_draw\n");

  if (streets_check_if_moved ())
    streets_rebuild_list ();

  gdks_streets_max = streets_list_count + 1;

  if ( 0 >= gdks_streets_max )
      return;

  if ( mydebug > 50 )
    printf ("streets_draw %ld segments\n", streets_list_count);

  gdks_streets = g_new0 (GdkSegment, gdks_streets_max);
  if (NULL == gdks_streets)
    {
      printf ("Problem reserving Memory for %d Street segments\n",
	      gdks_streets_max);
      gdks_streets_max = -1;
      return;
    }

  if ( mydebug > 50 )
    printf ("created gdk struct for %d segments\n", gdks_streets_max);

  /* ------------------------------------------------------------------ */
  /*  draw gdks_streets_list streets */
  for (i = 0; i < streets_list_count; i++)
    {
      gdouble posx1, posy1;
      gdouble posx2, posy2;

      posx1 = (streets_list + i)->x1;
      posy1 = (streets_list + i)->y1;
      posx2 = (streets_list + i)->x2;
      posy2 = (streets_list + i)->y2;


      /*      if ( mydebug >90 ) {
       * //printf("    a1 %f,%f\n",  posx1,posy1);
       * //printf("    a2 %f,%f\n",  posx2,posy2);
       * 
       * printf ("STREETS Draw: %f %f -> %f, %f            %s\n",
       * (streets_list + i)->lat1, (streets_list + i)->lon1,
       * (streets_list + i)->lat2, (streets_list + i)->lon2,
       * (streets_list + i)->name
       * );
       * }
       */
      if (posxy_on_screen (posx1, posy1) || 
	  posxy_on_screen (posx2, posy2))
	{
	  /*
	   * if ( mydebug > 50 ) {
	   * printf ("       Draw: %f %f -> %f, %f\n",
	   * (streets_list + i)->x1,   (streets_list + i)->y1,
	   * (streets_list + i)->x2,   (streets_list + i)->y2
	   * );
	   * }
	   */

	  // Alloc Memory if we need more
	  gdks_streets_count++;
	  if (gdks_streets_count >= gdks_streets_max)
	    {
	      gdks_streets_max = gdks_streets_count + 1000;
	      g_renew (GdkSegment, gdks_streets, gdks_streets_max);
	      if (NULL == streets_list)
		{
		  g_print
		    ("Error: Cannot allocate Memory for %d street-gdk segments\n",
		     gdks_streets_max);
		  gdks_streets_max = -1;
		  return;
		}
	    }

	  (gdks_streets + gdks_streets_count)->x1 = posx1;
	  (gdks_streets + gdks_streets_count)->y1 = posy1;
	  (gdks_streets + gdks_streets_count)->x2 = posx2;
	  (gdks_streets + gdks_streets_count)->y2 = posy2;

	  if (mydebug >90)
	    {
	      char beschrift[120];
	      g_snprintf (beschrift, sizeof (beschrift),
			  "(%.4f ,%.4f)\n(%.4f ,%.4f)\n%s",
			  (streets_list + i)->lat1,
			  (streets_list + i)->lon1,
			  (streets_list + i)->lat2, (streets_list + i)->lon2,
			  //(streets_list + i)->streets_type_id,
			  (streets_list + i)->name);

	      draw_text_with_box (posx1 +
				  (posx2 - posx1) / 2,
				  posy1 + (posy2 - posy1) / 2, beschrift);
	      //draw_text_with_box(posx1+(posx2-posx1)/2,posy1+(posy2-posy1)/2,(streets_list + i)->name);
	      if (0)
		{
		  draw_small_plus_sign (posx1 +
					(posx2 -
					 posx1) / 2,
					posy1 + (posy2 - posy1) / 2);
		  draw_text_with_box (posx1, posy1 - 15,
				      (streets_list + i)->name);
		  draw_text_with_box (posx2, posy2 + 15,
				      (streets_list + i)->name);
		}
	      gdk_gc_set_foreground (kontext, &red);
	      draw_small_plus_sign (posx1, posy1);
	      draw_small_plus_sign (posx2, posy2);

	    }

	  if ( mydebug > 50 )
	    {
	      //          printf("    1 %f,%f\n",  posx1,posy1);
	      //          printf("    2 %f,%f\n",  posx2,posy2);
	    }
	}

      // draw it if last or streets_type_id changes 
      if ((i == streets_list_count - 1) ||
	  ((streets_list + i)->streets_type_id !=
	   (streets_list + i + 1)->streets_type_id))
	{
	  /*
	   * if ( mydebug > 50 )
	   * printf("Drawing %d segments\n",gdks_streets_count);
	   */
	  int streets_id = (streets_list + i)->streets_type_id;
	  if (streets_id < streets_type_list_max)
	    {
	      //gdk_gc_set_foreground (kontext, &red);
	      gdk_gc_set_foreground (kontext,
				     streets_type_list[streets_id].color);
	      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	    }
	  else
	    {
	      gdk_gc_set_foreground (kontext, &red);
	      gdk_gc_set_line_attributes (kontext, 5, 0, 0, 0);
	    }

	  gdk_draw_segments (drawable, kontext,
			     (GdkSegment *) gdks_streets,
			     gdks_streets_count + 1);
	  gdks_streets_count = -1;
	}
    }

  g_free (gdks_streets);

  if ( mydebug > 50 )
    printf
      ("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}


/* *******************************************************
 * query all Info for streets in area arround lat/lon
*/
void
streets_query_area (gdouble lat1, gdouble lon1, gdouble lat2, gdouble lon2)
{
  gint i;
  printf ("Query Streets: %f ... %f , %f ... %f\n", lat1, lat2, lon1, lon2);

  for (i = 0; i < streets_list_count; i++)
    {
      // TODO:: make real checks if in rectangle or crossing rectangle
      if (((lat1 <= (streets_list + i)->lat1)
	   && ((streets_list + i)->lat1 <= lat2)
	   && (lon1 <= (streets_list + i)->lon1)
	   && ((streets_list + i)->lon1 <= lon2))
	  || ((lat1 <= (streets_list + i)->lat2)
	      && ((streets_list + i)->lat2 <= lat2)
	      && (lon1 <= (streets_list + i)->lon2)
	      && ((streets_list + i)->lon2 <= lon2)))
	{
	  printf ("Streets: %d: %f,%f --> %f,%f :%s\t",
		  i,
		  (streets_list + i)->lat1, (streets_list + i)->lon1,
		  (streets_list + i)->lat2, (streets_list + i)->lon2,
		  (streets_list + i)->name);
	  gint streets_type_id = (streets_list + i)->streets_type_id;

	  printf ("Type: %s\t", streets_type_list[streets_type_id].name);
	  printf ("%s\n", (streets_list + i)->comment);
	}
    }

}
