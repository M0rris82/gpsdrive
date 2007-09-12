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
 * wlan_ support module: display
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include <gmodule.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"

#include "gpsdrive.h"
#include "poi.h"
#include "wlan.h"
#include "config.h"
#include "gettext.h"
#include "icons.h"
#include <gpsdrive_config.h>
#include "gui.h"

#include "gettext.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern poi_type_struct poi_type_list[poi_type_list_max];

extern gint do_unit_test;
extern gint maploaded;
extern gint isnight, disableisnight;
extern color_struct colors;
extern currentstatus_struct current;
extern gint debug, mydebug;
extern gint usesql;
extern glong mapscale;
extern GdkGC *kontext_map;

char txt[5000];
PangoLayout *wlan_label_layout;
#include "mysql/mysql.h"

extern MYSQL mysql;
extern MYSQL_RES *res;
extern MYSQL_ROW row;

// keep actual visible WLAN Points in Memory
wlan_struct *wlan_list;
glong wlan_nr;			// current number of wlan to count
glong wlan_max;			// max index of WLANs actually in memory
glong wlan_limit = -1;		// max allowed index (if you need more you have to alloc memory)

int wlan_closed=19; // these will be filled later from the Database
int wlan_open=19;
int wlan_wep=19;

gchar wlan_label_font[100];
GdkColor wlan_colorv;
PangoFontDescription *pfd;
PangoLayout *wlan_label_layout;

/* ******************************************************************   */

void wlan_rebuild_list (void);


/* *******************************************************
 */
void
wlan_init (void)
{
  wlan_limit = 40000;
  wlan_list = g_new (wlan_struct, wlan_limit);
  if (wlan_list == NULL)
    {
      g_print ("Error: Cannot allocate Memory for %ld wlan\n", wlan_limit);
      wlan_limit = -1;
      return;
    }

  wlan_rebuild_list ();
}


/* *********************************************************
 */
gdouble wlan_lat_lr = 0, wlan_lon_lr = 0;
gdouble wlan_lat_ul = 0, wlan_lon_ul = 0;

int
wlan_check_if_moved (void)
{
  gdouble lat_lr, lon_lr;
  gdouble lat_ul, lon_ul;

  if (wlan_lat_lr == 0 && wlan_lon_lr == 0 &&
      wlan_lat_ul == 0 && wlan_lon_ul == 0)
    return 1;

  calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, current.zoom);
  calcxytopos (0, 0, &lat_ul, &lon_ul, current.zoom);

  if (wlan_lat_lr == lat_lr && wlan_lon_lr == lon_lr &&
      wlan_lat_ul == lat_ul && wlan_lon_ul == lon_ul)
    return 0;
  return 1;
}

/* *******************************************************
 * if zoom, xoff, yoff or map are changed 
 * TODO: use the real datatype for reading from database
 * (don't convert string to double)
 */
void
wlan_rebuild_list (void)
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

  if ( current.mapscale > 100000000)
      return;

  if (!local_config.showwlan)
    {
      if (mydebug > 20)
	printf ("wlan_rebuild_list: WLAN_draw is off\n");
      return;
    }

  if (mydebug > 20)
    {
      printf
	("wlan_rebuild_list: Start\t\t\t\t\t\tvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
    }


  if (!maploaded)
    return;

  if (current.importactive)
    return;


  // calculate the start and stop for lat/lon according to the displayed section
  calcxytopos (0, 0, &lat_ul, &lon_ul, current.zoom);
  calcxytopos (0, SCREEN_Y, &lat_ll, &lon_ll, current.zoom);
  calcxytopos (SCREEN_X, 0, &lat_ur, &lon_ur, current.zoom);
  calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, current.zoom);

  lat_min = min (lat_ll, lat_ul);
  lat_max = max (lat_lr, lat_ur);
  lon_min = min (lon_ll, lon_ul);
  lon_max = max (lon_lr, lon_ur);

  lat_mid = (lat_min + lat_max) / 2;
  lon_mid = (lon_min + lon_max) / 2;

  gdouble wlan_posx, wlan_posy;
  gettimeofday (&t, NULL);
  ti = t.tv_sec + t.tv_usec / 1000000.0;

  { // Limit the select with WHERE min_lat<lat<max_lat AND min_lon<lon<max_lon
    g_snprintf (sql_where, sizeof (sql_where),
		" WHERE ( lat BETWEEN %.6f AND %.6f ) "
		" AND   ( lon BETWEEN %.6f AND %.6f ) "
		, lat_min, lat_max, lon_min, lon_max
      );
    g_strdelimit (sql_where, ",", '.');	// For different LANG
    if (mydebug > 20)
      {
	printf ("wlan_rebuild_list: WLAN mysql where: %s\n", sql_where);
      }
  }


  g_snprintf (sql_query, sizeof (sql_query),
	      "SELECT lat,lon,macaddr,essid,wep,nettype,cloaked FROM wlan "
	      "%s LIMIT 40000", sql_where);

  if (mydebug > 20)
    printf ("wlan_rebuild_list: WLAN mysql query: %s\n", sql_query);

  if (dl_mysql_query (&mysql, sql_query)) {
      printf ("wlan_rebuild_list: Error in query: \n");
      fprintf (stderr, "wlan_rebuild_list: Error in query: %s\n",
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

  rges = r = 0;
  wlan_nr = 0;
  while ((row = dl_mysql_fetch_row (res)))
    {
      rges++;
      gdouble lat, lon;

      if (mydebug > 20)
	fprintf (stderr, "WLAN Query Result: lat:%s\tlon:%s\t%s\t%s\t%s\t%s\t%s\n",
		 row[0], row[1], row[2], row[3], row[4], row[5], row[6]);

      lat = g_strtod (row[0], NULL);
      lon = g_strtod (row[1], NULL);
      calcxy (&wlan_posx, &wlan_posy, lon, lat, current.zoom);

      if ((wlan_posx > -50) && (wlan_posx < (SCREEN_X + 50)) &&
	  (wlan_posy > -50) && (wlan_posy < (SCREEN_Y + 50)))
	{
	  // get next free mem for wlan_list
	  if (wlan_nr > wlan_limit) {
	      wlan_limit = wlan_nr + 10000;
	      if (mydebug > 20)
		g_print ("Try to allocate Memory for %ld wlan\n", wlan_limit);
	      
	      wlan_list = g_renew (wlan_struct, wlan_list, wlan_limit);
	      if (NULL == wlan_list) {
		  g_print ("Error: Cannot allocate Memory for %ld wlan\n",  wlan_limit);
		  wlan_limit = -1;
		  return;
	      }
	  }

	  // Save retrieved wlan information into structure
	  (wlan_list + wlan_nr)->lat = lat;
	  (wlan_list + wlan_nr)->lon = lon;
	  (wlan_list + wlan_nr)->x = wlan_posx;
	  (wlan_list + wlan_nr)->y = wlan_posy;

	  g_snprintf ((wlan_list + wlan_nr)->name,
		      sizeof ((wlan_list + wlan_nr)->name), "%s ( %s )"
		      , row[2], row[3] );
	  (wlan_list + wlan_nr)->nettype = (gint) g_strtod (row[5], NULL);
	  int icon_index = 19;
	  if ( (wlan_list + wlan_nr)->nettype == 0 )	icon_index = wlan_open;
	  if ( (wlan_list + wlan_nr)->nettype == 1 )	icon_index = wlan_closed;
	  if ( (wlan_list + wlan_nr)->nettype == 2 )	icon_index = wlan_wep;
	  (wlan_list + wlan_nr)->poi_type_id = icon_index;

	  if ( mydebug > 20 ) 
	      printf ("DB: %f %f \t( x:%f, y:%f )\t%s\n",
		      (wlan_list + wlan_nr)->lat, (wlan_list + wlan_nr)->lon, 
		      (wlan_list + wlan_nr)->x, (wlan_list + wlan_nr)->y, 
		      (wlan_list + wlan_nr)->name
		      );
	  wlan_nr++;
	} else {
	  if ( mydebug > 20 ) 
	      fprintf( stderr ,"Ignoring Point, becuause it's out of bound\n");
	}
    }


  wlan_max = wlan_nr;

  // print time for getting Data
  gettimeofday (&t, NULL);
  ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
  if (mydebug > 20)
    printf (_("%ld(%d) rows read in %.2f seconds\n"), wlan_max,
	    rges, (gdouble) ti);


  /* remember where the data belongs to */
  wlan_lat_lr = lat_lr;
  wlan_lon_lr = lon_lr;
  wlan_lat_ul = lat_ul;
  wlan_lon_ul = lon_ul;

  if (!dl_mysql_eof (res))
    {
      fprintf (stderr, "wlan_rebuild_list: Error in dl_mysql_eof: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
      return;
    }

  dl_mysql_free_result (res);
  res = NULL;

  if (mydebug > 20)
    {
      printf ("wlan_rebuild_list: End \t\t\t\t\t\t");
      printf ("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
    }
}





/* *******************************************************
 * draw wlan_ on image 
 TODO: find free space on drawing area. So the Text doesn't overlap
*/
void
wlan_draw_list (void)
{
  //  gint t;
  //  GdkSegment *routes;

  gint i;

  if (!usesql)
    return;

  if (current.importactive)
    return;

  if (!maploaded)
    return;

  if (!local_config.showwlan)
    {
      if (mydebug > 20)
	printf ("wlan_draw_list: WLAN_draw is off\n");
      return;
    }


  if (mydebug > 20)
    printf
      ("wlan_draw_list: Start\t\t\tvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");

  if (wlan_check_if_moved ())
    wlan_rebuild_list ();


  /* ------------------------------------------------------------------ */
  /*  draw wlan_list wlan points */
  if (mydebug > 20)
    printf ("wlan_draw_list: drawing %ld wlans\n", wlan_max);

  for (i = 0; i < wlan_max; i++)
    {
      gdouble posx, posy;

      posx = (wlan_list + i)->x;
      posy = (wlan_list + i)->y;

      if ((posx >= 0) && (posx < SCREEN_X) &&
	  (posy >= 0) && (posy < SCREEN_Y))
	{
	  gdk_gc_set_line_attributes (kontext_map, 2, 0, 0, 0);

	  g_strlcpy (txt, (wlan_list + i)->name, sizeof (txt));

	  {
	    GdkPixbuf *icon;
	    int icon_index = (wlan_list + i)->poi_type_id;
	    if (mydebug > 20)
		printf( "icon_index: %d\n",icon_index);

	    icon = poi_type_list[icon_index].icon;

	    if (icon != NULL && icon_index > 0)
	      {
		if (wlan_max < 2000)
		  {
		    int wx = gdk_pixbuf_get_width (icon);
		    int wy = gdk_pixbuf_get_height (icon);

		    gdk_draw_pixbuf (drawable, kontext_map, icon,
				     0, 0,
				     posx - wx / 2,
				     posy - wy / 2,
				     wx, wy, GDK_RGB_DITHER_NONE, 0, 0);
		  }
	      } else {
		  gdk_gc_set_foreground (kontext_map, &colors.red);
		  if (wlan_max < 20000) { // Only draw small + if more than ... Wlan Points
		      draw_plus_sign (posx, posy);
		  } else {
		      draw_small_plus_sign (posx, posy);
		  }
	      }
	    
	    // Only draw Text if less than 1000 Wlan Points are to be displayed
	    if (wlan_max < 1000)
	      {
		draw_label (txt, posx, posy);
	      }
	  }
	}


    }
  if (mydebug > 20)
    printf
      ("wlan_draw_list: End\t\t\t^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
}


/* *******************************************************
 * query all Info for Wlan Points in area arround lat/lon
*/
void
wlan_query_area (gdouble lat1, gdouble lon1, gdouble lat2, gdouble lon2)
{
  gint i;
  printf ("Query: %f ... %f , %f ... %f\n", lat1, lat2, lon1, lon2);

  for (i = 0; i < wlan_max; i++)
    {
      //printf ("check WLAN: %ld: %f %f :%s\n",i,(wlan_list + i)->lat, (wlan_list + i)->lon,(wlan_list + i)->name);
      if ((lat1 <= (wlan_list + i)->lat) &&
	  ((wlan_list + i)->lat <= lat2) &&
	  (lon1 <= (wlan_list + i)->lon) && ((wlan_list + i)->lon <= lon2))
	{
	  printf ("Query WLAN: %d: %f %f :%s\n",
		  i, (wlan_list + i)->lat, (wlan_list + i)->lon,
		  (wlan_list + i)->name);
	}
    }
}

/* *******************************************************
 * get IDs for the different wlan types
 */
void get_poi_type_id_for_wlan() {
    int i;
    for (i = 0; i < poi_type_list_max; i++)
      {
	poi_type_list[i].icon = NULL;
	if (strcmp (poi_type_list[i].name,"wlan.closed") == 0){
	    wlan_closed=i;
	} else if (strcmp (poi_type_list[i].name,"wlan.open") == 0){
	    wlan_open=i;
	} else if (strcmp (poi_type_list[i].name,"wlan.wep") == 0){
	    wlan_wep=i;
	}
      }
}
