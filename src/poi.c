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
Revision 1.9  2005/03/15 08:40:13  tweety
eliminate doublicate pfd description

Revision 1.8  2005/02/22 08:18:51  tweety
change leveing system to simpler scale marking for decission what to show on display
column_names(DBFuncs.pm get data from Database
added functions add_index drop_index
added language to type Database
for some Data split unpack and mirror Directories
for some add lat/lon min/max to get faster import for testing
added POI::DBFuncs::segments_add; this will later be the point to do some excerptions and combinations
on the street data

Revision 1.7  2005/02/17 09:46:34  tweety
minor changes

Revision 1.6  2005/02/13 22:57:00  tweety
WDB Support

Revision 1.5  2005/02/13 14:06:54  tweety
start street randering functions. reading from the database streets and displayi
ng it on the screen
improve a little bit in the sql-queries
fixed linewidth settings in draw_cross

Revision 1.4  2005/02/10 06:22:25  tweety
added primitive drawing of icons to POI

Revision 1.3  2005/02/08 20:18:39  tweety
small fixes in poi.c

Revision 1.2  2005/02/07 07:53:39  tweety
added check_if_moved inti function poi_rebuild_list

Revision 1.1  2005/02/02 18:11:02  tweety
Add Point Of Interrest Support with mySQL

Revision 0.0  2005/01/11 20:14:14  tweety

*/


/*
 * poi_ support module: display
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "gpsdrive.h"
#include "poi.h"
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
extern GdkColor grey;

extern gdouble current_long, current_lat;
extern gint debug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats, *drawing_miniimage;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;

char txt[5000];
PangoLayout *poi_label_layout;
#include "mysql.h"

MYSQL mysql;
MYSQL_RES *res;
MYSQL_ROW row;

// keep actual visible POIs in Memory
poi_struct *poi_list;
glong poi_nr;        // current number of poi to count
glong poi_max;       // max index of POIs actually in memory
glong poi_limit;     // max allowed imdex (if you need more you have to alloc memory)
gint  poi_draw = FALSE;

gchar poi_label_font[100];
GdkColor poi_colorv;
PangoFontDescription *pfd;
PangoLayout *poi_label_layout;


typedef struct
{
  gint    type_id;
  gchar   name[80];
  gchar   icon_name[80];
  GdkPixbuf *icon;
}
type_struct;
#define type_list_max 1000
type_struct type_list[type_list_max];
int type_list_count=0;

/* ******************************************************************   */

void poi_rebuild_list (void);
void get_type_list(void);

/* *******************************************************
 */
void
poi_init (void)
{
  poi_limit = 10000;
  poi_list = g_new (poi_struct, poi_limit);
  poi_rebuild_list ();
  get_type_list();
}


/* *********************************************************
*/
gdouble poi_lat_lr=0, poi_lon_lr=0;
gdouble poi_lat_ul=0, poi_lon_ul=0;

int
poi_check_if_moved(void)
{
  gdouble lat_lr, lon_lr;
  gdouble lat_ul, lon_ul;

  if ( poi_lat_lr == 0 && poi_lon_lr == 0 &&
       poi_lat_ul == 0 && poi_lon_ul == 0    ) 
      return 1;

  calcxytopos (SCREEN_X , SCREEN_Y , &lat_lr, &lon_lr, zoom);
  calcxytopos (0        , 0        , &lat_ul, &lon_ul, zoom);

  if ( poi_lat_lr == lat_lr && poi_lon_lr == lon_lr &&
       poi_lat_ul == lat_ul && poi_lon_ul == lon_ul    ) 
      return 0;
  return 1;
}

/* ****************************************************************** */
/* get a list of all possible types and load there icons */
void
get_type_list (void)
{
  char sql_query[3000];
  if ( debug ) 
    printf("get_type_list ()\n");
  g_snprintf (sql_query, sizeof (sql_query),
	      "SELECT type_id,name,symbol FROM type ORDER BY type_id");

  if (dl_mysql_query (&mysql, sql_query))
    {
      fprintf(stderr,"Error in query: %s\n",dl_mysql_error (&mysql) );
      return;
    }
  
  if (!(res = dl_mysql_store_result (&mysql)))
    {
      fprintf(stderr,"Error in sotr results: %s\n",dl_mysql_error (&mysql) );
      return;
    }

  while ((row = dl_mysql_fetch_row (res)))
    {
      type_list_count = (gint)g_strtod(row[0], NULL);
      if ( type_list_count < type_list_max ) {
	type_list[type_list_count].type_id =  type_list_count;
	g_strlcpy ( type_list[type_list_count].name, row[1],
		    sizeof (type_list[type_list_count].name));
	g_strlcpy ( type_list[type_list_count].icon_name, row[2],
		    sizeof (type_list[type_list_count].icon_name));
	{ // load icon
	  gchar icon_name[80];
	  GdkPixbuf *icon;
	  gchar path[1024];
	  g_strlcpy ( icon_name, row[2], sizeof (icon_name));

	  g_snprintf (path, sizeof (path), "%sicons/%s.png", homedir, icon_name);
	  icon =  gdk_pixbuf_new_from_file (path, NULL);
	
	  if ( icon == NULL) {
	    g_snprintf (path, sizeof (path), "%s/gpsdrive/icons/%s.png", DATADIR,icon_name);
	    icon =  gdk_pixbuf_new_from_file (path, NULL);
	  } 
	  type_list[type_list_count].icon = icon;
	  if ( icon == NULL) {
	    printf("Icon (%s) not found\n",icon_name);
	  } else {
	    if (debug) 
	      printf("Icon (%s) loaded from %s\n",icon_name,path);
	  }
	}
      } else {
	printf("Typet_list: ype_list_count >type_list_max\n");
      }
    }
  if (!dl_mysql_eof (res))
    return;

  dl_mysql_free_result (res);

}

/* *******************************************************
 * if zoom, xoff, yoff or map are changed 
 TODO: use the real datatype for reading from database
       (dont convert string to double)
 */
void
poi_rebuild_list (void)
{
  char sql_query[5000];
  char sql_order[5000];
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

  if ( ! poi_draw ) {
    if ( debug ) 
      printf("POI_draw is off\n");
    return;
  }
  

  // calculate the start and stop for lat/lon according to the displayed section
  calcxytopos (0        , 0        , &lat_ul, &lon_ul, zoom);
  calcxytopos (0        , SCREEN_Y , &lat_ll, &lon_ll, zoom);
  calcxytopos (SCREEN_X , 0        , &lat_ur, &lon_ur, zoom);
  calcxytopos (SCREEN_X , SCREEN_Y , &lat_lr, &lon_lr, zoom);

  lat_min = min(lat_ll, lat_ul);
  lat_max = max(lat_lr, lat_ur);
  lon_min = min(lon_ll, lon_ul);
  lon_max = max(lon_lr, lon_ur);

  lat_mid = (lat_min + lat_max)/2;
  lon_mid = (lon_min + lon_max)/2;

  gdouble poi_posx, poi_posy;

  if ( debug) {
    printf ("=============================================================================\n");
    printf ("rebuild_poi_list()\n");
  }

  if (!maploaded)
    return;

  //  if (!poi_flag)
  // return;
  if (importactive)
    return;

  gettimeofday (&t, NULL);
  ti = t.tv_sec + t.tv_usec / 1000000.0;

  { // gernerate mysql ORDER string
    char sql_order_numbers[5000];
    /*
      g_snprintf (sql_order_numbers, sizeof (sql_order),
      "(abs(%.6f - lat)+abs(%.6f - lon))"
      ,lat_mid,lon_mid);
      g_strdelimit (sql_order_numbers, ",", '.'); // For different LANG
    
      g_snprintf (sql_order, sizeof (sql_order),
      "order by scale_min,%s ",sql_order_numbers);
    */
    g_snprintf (sql_order, sizeof (sql_order),
		"order by scale_min,scale_max ");
    /*
      g_snprintf (sql_order, sizeof (sql_order),
      "order by scale ");
    */
    if (debug)
      printf ("POI mysql order: %s\n", sql_order);
  }
  
  { // Limit the select with WHERE min_lat<lat<max_lat AND min_lon<lon<max_lon
    g_snprintf (sql_where, sizeof (sql_where),
		"WHERE ( lat BETWEEN %.6f AND %.6f ) "
		"AND   ( lon BETWEEN %.6f AND %.6f ) "
		"AND   ( %f  BETWEEN scale_min AND scale_max) ",
		lat_min,lat_max,
		lon_min,lon_max,
		mapscale);
    g_strdelimit (sql_where, ",", '.'); // For different LANG
    if (debug) {
      //printf ("POI mysql where: %s\n", sql_where );
      printf ("POI mapscale: %d\n", mapscale );
    }
  }


  

  // TODO: Diplay ONLY those POI which are poi.scale_min <= scale <=poi.scale_max for actual scale
  // TODO: reread really and only when scale/map-pos changes 

  g_snprintf (sql_query, sizeof (sql_query),
	      // "SELECT lat,lon,alt,type_id,proximity "
	      "SELECT lat,lon,name,type_id "
	      "FROM poi "
	      //	      "LEFT JOIN oi_ type ON poi_.type_id = type.type_id "
	      "%s %s LIMIT 1000",
	      sql_where,sql_order);
  /*    dbwherestring,sql_order,lat,lon);  */
  if (debug)
    printf ("POI mysql query: %s\n", sql_query);

  if (dl_mysql_query (&mysql, sql_query))
    {
      fprintf(stderr,"Error in query: %s\n",dl_mysql_error (&mysql) );
      return;
    }

  if (!(res = dl_mysql_store_result (&mysql)))
    {
      fprintf(stderr,"Error in store results: %s\n",dl_mysql_error (&mysql) );
      return;
    }

  rges = r = 0;
  poi_nr = 0;
  while ((row = dl_mysql_fetch_row (res)))
    {
      rges++;
      gdouble lat,lon;
      lat = g_strtod(row[0], NULL);
      lon = g_strtod(row[1], NULL);
      calcxy (&poi_posx, &poi_posy, 
	      lon,lat,
	      zoom);

      if ( (poi_posx > -50) && (poi_posx < (SCREEN_X + 50)) &&
	   (poi_posy > -50) && (poi_posy < (SCREEN_Y + 50)) 
	   )
	{
	  // get next free mem for point
	  poi_nr++;
	  if (poi_nr > poi_limit) {
	    poi_limit +=  10000;
	    poi_list =
	      g_renew (poi_struct, poi_list, poi_limit );
	    // TODO: check if g_renew failed
	  }

	  // Save retrieved poi information into structure
	  (poi_list + poi_nr)->lat        = lat;
	  (poi_list + poi_nr)->lon        = lon;
	  (poi_list + poi_nr)->x          = poi_posx;
	  (poi_list + poi_nr)->y          = poi_posy;
	  g_strlcpy ((poi_list + poi_nr)->name, row[2], sizeof ((poi_list + poi_nr)->name));
	  (poi_list + poi_nr)->type_id    = (gint)g_strtod(row[3], NULL);
	  if (debug) { 
	    g_snprintf ((poi_list + poi_nr)->name, sizeof ((poi_list + poi_nr)->name),
			"%s\n(%.4f ,%.4f)",
			//			(poi_list + poi_nr)->type_id,
			row[2],
			lat,lon);
	    /*
	      `type_id` int(11) NOT NULL default \'0\',
	      `alt` double default \'0\',
	      `proximity` float default \'0\',
	      `comment` varchar(255) default NULL,
	      `scale_min` smallint(6) NOT NULL default \'0\',
	      `scale_max` smallint(6) NOT NULL default \'0\',
	      `last_modified` date NOT NULL default \'0000-00-00\',
	      `url` varchar(160) NULL ,
	      `address_id` int(11) default \'0\',
	      `source_id` int(11) NOT NULL default \'0\',
	      */
	  }
	  //(poi_list + poi_nr)->wp_id      = g_strtol (row[0], NULL);
	  //      (poi_list + poi_nr)->alt        = g_strtod(row[2], NULL);
	  //(poi_list + poi_nr)->name[80] = row[4];
	  //(poi_list + poi_nr)->type_id    = g_strtod(row[5], NULL); 
	  //(poi_list + poi_nr)->proximity  = g_strtod(row[6], NULL);
	  //(poi_list + poi_nr)->comment[255 ] = row[7]; 
	  //(poi_list + poi_nr)->scale_min     = row[8];  
	  //(poi_list + poi_nr)->scale_max     = row[8];  
	  //(poi_list + poi_nr)->last_modified = row[8] 
	  //(poi_list + poi_nr)->url[160]      = row[10]; 
	  //(poi_list + poi_nr)->address_id    = row[11];
	  //(poi_list + poi_nr)->source_id     = row[12]; 

	  /*
	    if (debug) 
	    printf ("DB: %f %f \t( x:%f, y:%f )\t%s\n",
		    (poi_list + poi_nr)->lat, (poi_list + poi_nr)->lon, 
		    (poi_list + poi_nr)->x, (poi_list + poi_nr)->y, 
		    (poi_list + poi_nr)->name
		    );
	  */
	}
    }
  

  poi_max=poi_nr;

  // print time for getting Data
  gettimeofday (&t, NULL);
  ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
  if ( debug )
    printf (_("%d(%d) rows read in %.2f seconds\n"), poi_max, rges, ti);


  /* remember where the data belongs to */
  poi_lat_lr = lat_lr;
  poi_lon_lr = lon_lr;
  poi_lat_ul = lat_ul; 
  poi_lon_ul = lon_ul;

  if (!dl_mysql_eof (res))
    return;

  dl_mysql_free_result (res);

  printf ("=============================================================================\n");
}





/* *******************************************************
 * draw poi_ on image 
 TODO: find free space on drawing area. So the Text doesn't overlap
 */
void
poi_draw_list (void)
{
  //  gint t;
  //  GdkSegment *routes;
  gint width, height;

  gint i;
  gint k, k2;

  if (importactive)
    return;

  if ( ! ( poi_draw ) ) {
    if ( debug ) 
      printf("POI_draw is off\n");
    return;
  }

  if ( poi_check_if_moved() )
    poi_rebuild_list();  

  /*    if (!maploaded) */
  /*      return; */
  /*
    if (!poi_flag)
    return;
  */


  /* ------------------------------------------------------------------ */
  /*  draw poi_list points */
  for (i = 0; i < poi_max; i++)
    {
      gdouble posx, posy;

      posx = (poi_list + i)->x;
      posy = (poi_list + i)->y;

      // TODO: These calculations have a slight offset
      // Probably ist the wrong funktion to calculate

      if ( (posx >= 0) && (posx < SCREEN_X)  &&
	   (posy >= 0) && (posy < SCREEN_Y) 
	   )
	{
	  
	  /*
	  if (debug) 
	    printf ("POI Draw: %f %f \t( x:%f, y:%f )\t%s\n",
		    (poi_list + poi_nr)->lat, (poi_list + poi_nr)->lon, 
		    (poi_list + poi_nr)->x, (poi_list + poi_nr)->y, 
		    (poi_list + poi_nr)->name
		    );
	  */
	  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	
	  g_strlcpy (txt, (poi_list + i)->name, sizeof (txt));
	
	  
	  //	  if ((drawicon (posx, posy, "HOTEL")) == 0)
	  {
	    GdkPixbuf *icon;
	    icon = type_list[(poi_list + i)->type_id].icon;
	    if ( icon != NULL ) 
	      {
		gdk_draw_pixbuf (drawable, kontext, icon,
				 0, 0,
				 posx - 12, posy - 12,
				 24, 24, GDK_RGB_DITHER_NONE, 0, 0);
	      }
	    else 
	      { /*  draw + sign at destination if no matching Icon found*/
		gdk_gc_set_foreground (kontext, &red);
		draw_plus_sign ( posx,posy);
	      }
	  }  
	  gdk_gc_set_foreground (kontext, &textback);
	    
	  poi_label_layout =
	    gtk_widget_create_pango_layout (drawing_area, txt);
	  if (pdamode)
	    pfd = pango_font_description_from_string ("Sans 8");
	  else
	    pfd = pango_font_description_from_string ("Sans 11");
	  pango_layout_set_font_description (poi_label_layout, pfd);
	  pango_layout_get_pixel_size (poi_label_layout, 
				       &width, &height);
	  k = width + 4;
	  k2 = height;
	  
	  gdk_gc_set_function (kontext, GDK_COPY);
	
	
	  gdk_gc_set_function (kontext, GDK_AND);
	
	  { // Draw rectangle arround Text
	    //gdk_gc_set_foreground (kontext, &textbacknew);
	    gdk_draw_rectangle (drawable, kontext, 1, 
				posx + 13, posy - k2 / 2, 
				k + 1, k2);
	    gdk_gc_set_function (kontext, GDK_COPY);
	    gdk_gc_set_foreground (kontext, &black);
	    gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
	    gdk_draw_rectangle (drawable, kontext, 0, 
				posx + 12, posy - k2 / 2 - 1,
				k + 2, k2);
	  }

	  /* prints in pango */

	  poi_label_layout =
	    gtk_widget_create_pango_layout (drawing_area, txt);
	  pango_layout_set_font_description (poi_label_layout, pfd);

	  gdk_draw_layout_with_colors (drawable, kontext,
				       posx + 15, posy - k2 / 2,
				       poi_label_layout, &black, NULL);
	  if (poi_label_layout != NULL)
	    g_object_unref (G_OBJECT (poi_label_layout));
	  /* freeing PangoFontDescription, cause it has been copied by prev. call */
	  pango_font_description_free (pfd);
	}
    }
}
