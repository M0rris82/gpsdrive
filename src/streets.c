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
Revision 1.1  2005/02/13 14:06:54  tweety
start street randering functions. reading from the database streets and displayi
ng it on the screen
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
extern GdkColor grey;

extern gdouble current_long, current_lat;
extern gint debug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats, *drawing_miniimage;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;

#include "mysql.h"

MYSQL mysql;
MYSQL_RES *res;
MYSQL_ROW row;

// keep actual visible Streets in Memory
streets_struct *streets_list;
glong streets_list_count;        // current number of streets to count
glong streets_list_limit;     // max allowed imdex (if you need more you have to alloc memory)

gint  streets_draw = FALSE;

gchar streets_label_font[100];
GdkColor streets_colorv;

/* ******************************************************************   */

void streets_rebuild_list (void);
void get_type_list(void);


/* ****************************************************************** 
   check if the given position is inside the screen
*/
int
posxy_on_screen(gdouble posx, gdouble posy) {
  if ( (posx >= 0) && (posx < SCREEN_X)    &&  
       (posy >= 0) && (posy < SCREEN_Y)
       )  
    return TRUE;
  return FALSE;
}
  


/* *******************************************************
 */
void
streets_init (void)
{
  streets_list_limit = 10000;
  streets_list = g_new (streets_struct, streets_list_limit);
  streets_rebuild_list ();
  get_type_list();
}


/* *********************************************************
*/
gdouble streets_lat_lr=0, streets_lon_lr=0;
gdouble streets_lat_ul=0, streets_lon_ul=0;

int
streets_check_if_moved(void)
{
  gdouble lat_lr, lon_lr;
  gdouble lat_ul, lon_ul;

  if ( streets_lat_lr == 0 && streets_lon_lr == 0 &&
       streets_lat_ul == 0 && streets_lon_ul == 0    ) 
      return 1;

  calcxytopos (SCREEN_X , SCREEN_Y , &lat_lr, &lon_lr, zoom);
  calcxytopos (0        , 0        , &lat_ul, &lon_ul, zoom);

  if ( streets_lat_lr == lat_lr && streets_lon_lr == lon_lr &&
       streets_lat_ul == lat_ul && streets_lon_ul == lon_ul    ) 
      return 0;
  if ( debug ) 
    printf("Streets Display moved\n");
  return 1;
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

  gdouble display_level;

  if (!maploaded)
    return;


  if (importactive)
    return;

  if ( debug ) 
    printf("streets_rebuild_list()\n");

  if ( ! streets_draw ) {
    if ( debug ) 
      printf("STREETS_draw is off\n");
    return;
  }
  

  // calculate which levels to display
  display_level=20;
  if ( mapscale <= 1000000 )  display_level=1;
  if ( mapscale <= 500000 )   display_level=20;
  if ( mapscale <= 100000 )   display_level=30;
  if ( mapscale <= 50000 )    display_level=60;
  if ( mapscale <= 8000 )     display_level=99;
  


  { // calculate the start and stop for lat/lon according to the displayed section
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
  }

  gdouble streets_posx1, streets_posy1;
  gdouble streets_posx2, streets_posy2;

  if ( debug) {
    printf ("=============================================================================\n");
    printf ("rebuild_streets_list()\n");
  }

  if ( debug) {
    gettimeofday (&t, NULL);
    ti = t.tv_sec + t.tv_usec / 1000000.0;
  }

  { // gernerate mysql ORDER string
    char sql_order_numbers[5000];
    g_snprintf (sql_order_numbers, sizeof (sql_order),
		"(abs(%.6f - lat1)+abs(%.6f - lon1))"
		,lat_mid,lon_mid);
    g_strdelimit (sql_order_numbers, ",", '.'); // For different LANG
    
    g_snprintf (sql_order, sizeof (sql_order),
		"order by type_id,level,%s ",sql_order_numbers);
    g_snprintf (sql_order, sizeof (sql_order),
		"order by %s ",sql_order_numbers);
    /*
      g_snprintf (sql_order, sizeof (sql_order),
      "order by level ");
    */
    if (debug)
      printf ("STREETS mysql order: %s\n", sql_order);
  }
  
  { // Limit the select with WHERE min_lat<lat<max_lat AND min_lon<lon<max_lon
    g_snprintf (sql_where, sizeof (sql_where),
		"WHERE "
		" ( "
		"   ( ( lat1 BETWEEN %.6f AND %.6f ) AND ( lon1 BETWEEN %.6f AND %.6f ) ) "
		"   OR "
		"   ( ( lat2 BETWEEN %.6f AND %.6f ) AND ( lon2 BETWEEN %.6f AND %.6f ) ) "
		" ) "
		" AND "
		" ( %f BETWEEN level_min AND level_max) ",
		lat_min,lat_max,lon_min,lon_max,
		lat_min,lat_max,lon_min,lon_max,
		display_level);
    g_strdelimit (sql_where, ",", '.'); // For different LANG
    if (debug) {
      printf ("STREETS mysql where: %s\n", sql_where );
      printf ("STREETS mapscale: %d\n", (gdouble)mapscale );
      printf ("STREETS level %f\n", (gdouble)display_level );
    }
  }


  

  // Diplay ONLY those STREETS which are streets.level_min <= level <=streets.level_max for actual scale
  g_snprintf (sql_query, sizeof (sql_query),
	      // "SELECT lat,lon,alt,type_id,proximity "
	      "SELECT lat1,lon1,lat2,lon2,name,type_id "
	      "FROM streets "
	      "%s %s LIMIT 10000",
	      sql_where,sql_order);

  if (debug)
    printf ("STREETS mysql query: %s\n", sql_query);

  if (dl_mysql_query (&mysql, sql_query))
    {
      perror ( "mysql_error in query");
      return;
    }

  if (!(res = dl_mysql_store_result (&mysql)))
    {
      perror("mysql_error in query");
      return;
    }

  rges = r = 0;
  streets_list_count = -1;
  while ((row = dl_mysql_fetch_row (res)))
    {
      rges++;
      gdouble lat1,lon1;
      gdouble lat2,lon2;
      lat1 = g_strtod(row[0], NULL);
      lon1 = g_strtod(row[1], NULL);
      calcxy (&streets_posx1, &streets_posy1,    lon1,lat1,     zoom);
      
      lat2 = g_strtod(row[2], NULL);
      lon2 = g_strtod(row[3], NULL);
      calcxy (&streets_posx2, &streets_posy2,    lon2,lat2,     zoom);

      if ( posxy_on_screen(streets_posx1,streets_posy1) ||
	   posxy_on_screen(streets_posx2,streets_posy2)
	   )
	{
	  // get next free mem for streets
	  streets_list_count++;
	  if (streets_list_count > streets_list_limit) {
	    streets_list_limit +=  10000;
	    streets_list =
	      g_renew (streets_struct, streets_list, streets_list_limit );
	    // TODO: check if g_renew failed
	  }

	  // Save retrieved streets information into structure
	  (streets_list + streets_list_count)->lat1        = lat1;
	  (streets_list + streets_list_count)->lon1        = lon1;
	  (streets_list + streets_list_count)->lat2        = lat2;
	  (streets_list + streets_list_count)->lon2        = lon2;
	  (streets_list + streets_list_count)->x1          = streets_posx1;
	  (streets_list + streets_list_count)->y1          = streets_posy1;
	  (streets_list + streets_list_count)->x2          = streets_posx2;
	  (streets_list + streets_list_count)->y2          = streets_posy2;
	  g_strlcpy ((streets_list + streets_list_count)->name, row[4], sizeof ((streets_list + streets_list_count)->name));
	  (streets_list + streets_list_count)->type_id = (gint)g_strtod(row[5], NULL);

	  if (debug) 
	    g_snprintf ((streets_list + streets_list_count)->name, sizeof ((streets_list + streets_list_count)->name),
			"(%.4f ,%.4f) (%.4f ,%.4f) %s",
			//			(streets_list + streets_list_count)->type_id,
			lat1,lon1,
			lat2,lon2,
			row[2]);
	  
	}
    }
  

  if ( debug )
    { // print time for getting Data
      gettimeofday (&t, NULL);
      ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
      printf (_("%d(%d) rows read in %.2f seconds\n"), streets_list_count, rges, ti);
    }
  
  {  /* remember where the data belongs to */
    streets_lat_lr = lat_lr;
    streets_lon_lr = lon_lr;
    streets_lat_ul = lat_ul; 
    streets_lon_ul = lon_ul;
  }

  if (!dl_mysql_eof (res))
    return;

  dl_mysql_free_result (res);

  printf ("=============================================================================\n");
}




/* ****************************************************************** */
void draw_text_with_box(gdouble posx,gdouble posy, gchar name[120]) {
  gint width, height;
  char txt[500];
  PangoFontDescription *pfd;
  PangoLayout *streets_label_layout;
  gint k, k2;

  // Draw Text Label with name
  g_strlcpy (txt, name, sizeof (name));
  gdk_gc_set_foreground (kontext, &textback);
	    
  streets_label_layout =
    gtk_widget_create_pango_layout (drawing_area, txt);
  if (pdamode)
    pfd = pango_font_description_from_string ("Sans 8");
  else
    pfd = pango_font_description_from_string ("Sans bold 11");
  pango_layout_set_font_description (streets_label_layout, pfd);
  pango_layout_get_pixel_size (streets_label_layout, 
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

  streets_label_layout =
    gtk_widget_create_pango_layout (drawing_area, txt);
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
  //  gint t;

  GdkSegment *gdks_streets;
  gint gdks_streets_count = -1;
  gint gdks_streets_max =10000;

  gint i;

  if (importactive)
    return;

  if ( ! ( streets_draw ) ) {
    if ( debug ) 
      printf("STREETS_draw is off\n");
    return;
  }

  if ( debug ) 
      printf("*****************************************************************************\nSTREETS_draw\n");

  if ( streets_check_if_moved() )
    streets_rebuild_list();  

  if ( debug ) 
    printf("streets_draw_list()\n");


  gdks_streets= g_new0 (GdkSegment, gdks_streets_max);

  if ( debug ) 
    printf("streets_draw_list():2\n");

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
      
      printf("    a1 %f,%f\n",  posx1,posy1);
      printf("    a2 %f,%f\n",  posx2,posy2);

      printf ("STREETS Draw: %f %f -> %f, %f            %s\n",
	      (streets_list + i)->lat1, (streets_list + i)->lon1,
	      (streets_list + i)->lat2, (streets_list + i)->lon2,
	      (streets_list + i)->name
	      );

      if ( posxy_on_screen(posx1,posy1) ||
	   posxy_on_screen(posx2,posy2) 
	   )
	{
	  if (debug) {
	    printf ("       Draw: %f %f -> %f, %f\n",
		    (streets_list + i)->x1,   (streets_list + i)->y1,
		    (streets_list + i)->x2,   (streets_list + i)->y2
		    );
	  }

	  // Alloc Memory if we need more
	  gdks_streets_count++;
	  if ( gdks_streets_count >= gdks_streets_max ) {
	    gdks_streets_max += 10000;
	    g_renew (GdkSegment, gdks_streets, gdks_streets_max);
	  }
	  
	  (gdks_streets + gdks_streets_count)->x1 = posx1;
	  (gdks_streets + gdks_streets_count)->y1 = posy1;
	  (gdks_streets + gdks_streets_count)->x2 = posx2;
	  (gdks_streets + gdks_streets_count)->y2 = posy2;
	  
	  draw_plus_sign ( posx1,posy1);
	  draw_plus_sign ( posx2,posy2);

	  /*
	    draw_text_with_box(posx1,posy1,(streets_list + i)->name);
	    draw_text_with_box(posx2,posy2,(streets_list + i)->name);
	  */

	  printf("    1 %f,%f\n",  posx1,posy1);
	  printf("    2 %f,%f\n",  posx2,posy2);

	}
      
      // draw it if last or type_id changes 
      if ( ( i == streets_list_count-1 ) ||
	   ( (streets_list + i)->type_id != (streets_list + i + 1)->type_id )
	   )
      {
	if ( debug )
	  printf("Drawing %d segments\n",gdks_streets_count);

	if ((streets_list + i)->type_id == 1) {
	  gdk_gc_set_foreground (kontext, &red);
	  gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
	} else 	if ((streets_list + i)->type_id == 2) {
	  gdk_gc_set_foreground (kontext, &blue);
	  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	} else 	if ((streets_list + i)->type_id == 3) {
	  gdk_gc_set_foreground (kontext, &blue);
	  gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
  	} else {
	  gdk_gc_set_foreground (kontext, &blue);
	  gdk_gc_set_line_attributes (kontext, 1, 0, 0, 0);
	}
	
	gdk_draw_segments (drawable, kontext, 
		     (GdkSegment *) gdks_streets, gdks_streets_count+1);
	gdks_streets_count=-1;
      }
    }
  
  g_free (gdks_streets);
  
  if ( debug ) 
    printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}
