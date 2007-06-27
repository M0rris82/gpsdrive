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
 * streets_ support module: display
 */

#include "config.h"
#include "gettext.h"
#include "gpsdrive.h"
#include "icons.h"
#include "streets.h"
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "gettext.h"

#include "gui.h"
#include "gpsdrive_config.h"

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
extern status_struct route;
extern gint isnight, disableisnight;
extern GdkColor grey;
extern color_struct colors;

extern gdouble current_long, current_lat;
extern gint mydebug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats,
  *drawing_miniimage;
extern gint usesql;
extern glong mapscale;

#include "mysql/mysql.h"

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
  GdkColor color;
  GdkColor color_bg;
  gint width;
  gint width_bg;
  gint scale_min;
  gint scale_max;
  GdkPixbuf *icon;
} streets_type_struct;
#define streets_type_list_max 1500
streets_type_struct streets_type_list[streets_type_list_max];
int streets_type_list_count = 0;

/* ******************************************************************   */

void streets_rebuild_list (void);
void get_streets_type_list (void);

/* **********************************************************************
 * Stopwatch
 */
double g_print_time( double t0, gchar *msg ){
    double ti;
    struct timeval t;

    gettimeofday (&t, NULL);
    ti = ( t.tv_sec + t.tv_usec / 1000000.0 );
    if ( t0 ) {
	g_print (msg, (gdouble) ti-t0);
    }
    return ti;
}

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
  if (mydebug > 50)
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
  if (mydebug > 50)
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

  if (mydebug > 50)
    printf ("get_streets_type_list ()\n");

  g_snprintf (sql_query, sizeof (sql_query),
	      "SELECT streets_type_id,name,color,color_bg,width,width_bg,scale_min,scale_max "
	      "FROM streets_type ORDER BY streets_type_id");

  if (mydebug > 20)
    printf ("QUERY:%s\n", sql_query);

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
      if (mydebug > 25)
	printf ("got ROW %s,	%s,	%s\n", row[0], row[1], row[2]);

      streets_type_list_count = (gint) g_strtod (row[0], NULL);
      if (streets_type_list_count >= streets_type_list_max)
	{
	  printf
	    ("------------------------------------------------------------\n");
	  printf ("ERROR: streets_type_list_count exeeded limit\n");

	  exit (-1);
	}


      streets_type_list[streets_type_list_count].streets_type_id
	= streets_type_list_count;

      // -------- street.name
      if (NULL == row[1])
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

      // read and convert Color Settings for this Street Type
      if (!gdk_color_parse ((gchar *) row[2],
			    &streets_type_list[streets_type_list_count].
			    color))
	{
	  printf ("-----------------------------------------\n");
	  printf ("ERROR: Invalid Street Color: %s for ID=%d\n", row[2],
		  streets_type_list_count);
	  streets_type_list[streets_type_list_count].color = colors.red;
	}
      if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
				     &streets_type_list
				     [streets_type_list_count].color,
				     FALSE, TRUE))
	{
	  printf ("------------------------------------------\n");
	  printf ("ERROR: Error in alloc: %s\n", row[2]);
	  streets_type_list[streets_type_list_count].color = colors.red;
	};

      // --------------------------------------------
      // read and convert bg Color Settings for this Street Type
      if (!gdk_color_parse ((gchar *) row[3],
			    &streets_type_list[streets_type_list_count].
			    color_bg))
	{
	  printf ("------------------------------------------\n");
	  printf ("ERROR: Invalid BG Street Color: %s for ID=%d\n", row[3],
		  streets_type_list_count);
	  streets_type_list[streets_type_list_count].color_bg = colors.black;
	}
      if (!gdk_colormap_alloc_color (gdk_colormap_get_system (),
				     &streets_type_list
				     [streets_type_list_count].color_bg,
				     FALSE, TRUE))
	{
	  printf ("-----------------------------------------\n");
	  printf ("ERROR: Error in alloc: %s\n", row[3]);
	  streets_type_list[streets_type_list_count].color_bg = colors.black;
	};

      streets_type_list[streets_type_list_count].width     = (gint) g_strtod (row[4], NULL);
      streets_type_list[streets_type_list_count].width_bg  = (gint) g_strtod (row[5], NULL);
      streets_type_list[streets_type_list_count].scale_min = (gint) g_strtod (row[6], NULL);
      streets_type_list[streets_type_list_count].scale_max = (gint) g_strtod (row[7], NULL);


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
  char sql_area[5000];
  char sql_in[5000];
  int r, rges;
  double t0;

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

  if (mydebug > 50)
    printf
      ("streets_rebuild_list: Start\t\t\t\t\t\tvvvvvvvvvvvvvvvvvvvvvv\n");

  if (!streets_draw)
    {
      if (mydebug > 50)
	printf ("streets_rebuild_list: streets_draw is off\n");
      return;
    }

  {				// calculate the start and stop for lat/lon according to the displayed section
    calcxytopos (0, 0, &lat_ul, &lon_ul, zoom);
    calcxytopos (0, SCREEN_Y, &lat_ll, &lon_ll, zoom);
    calcxytopos (SCREEN_X, 0, &lat_ur, &lon_ur, zoom);
    calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, zoom);

    lat_min = min (lat_ll, lat_ul)-0.01;
    lat_max = max (lat_lr, lat_ur)+0.01;
    lon_min = min (lon_ll, lon_ul)-0.01;
    lon_max = max (lon_lr, lon_ur)+0.01;

    lat_mid = (lat_min + lat_max) / 2;
    lon_mid = (lon_min + lon_max) / 2;
  }

  gdouble streets_posx1, streets_posy1;
  gdouble streets_posx2, streets_posy2;


  // ----------------- JMO
  // gettimeofday (&t, NULL);
  t0 = g_print_time(0,"");
  // t.tv_sec + t.tv_usec / 1000000.0;

  { // Limit the displayed streets_types
      g_snprintf (sql_in, sizeof (sql_in),"\t streets_type_id IN ( ");
    int i;
    for (i = 0; i < streets_type_list_max; i++)
      {
	  if ( streets_type_list[i].scale_min <= mapscale   &&
	       streets_type_list[i].scale_max >= mapscale 
	       ) {
	      gchar id_string[20];
	      g_snprintf (id_string, sizeof (id_string),"%d,",
			  streets_type_list[i].streets_type_id);
	      g_strlcat (sql_in, id_string, sizeof (sql_in));
	  }
      }
    g_strlcat (sql_in, " 0)" , sizeof (sql_in));
    if ( mydebug > 20 )
      {
	printf ("STREET mysql in: %s\n", sql_in );
      }
  }


  { // TODO: change the selection against real LINE crosses visible RECTANGLE
    // TODO: not only start or endpoint are in rectangle
    // TODO: This will hopefully be easy as soon as we use the geometric extentions 
    // TODO: with the geometric objects of mysql
    // Limit the select with WHERE min_lat<lat<max_lat AND min_lon<lon<max_lon
    g_snprintf (sql_area, sizeof (sql_area),
		"\t\t ( \n"
		"\t\t   ( ( lat1 BETWEEN %f AND %f ) AND ( lon1 BETWEEN %f AND %f ) ) \n"
		"\t\t   OR \n"
		"\t\t   ( ( lat2 BETWEEN %f AND %f ) AND ( lon2 BETWEEN %f AND %f ) ) \n"
		"\t\t ) \n",
		lat_min, lat_max, lon_min, lon_max,
		lat_min, lat_max, lon_min, lon_max
		);
    g_strdelimit (sql_area, ",", '.');	// For different LANG
    if (mydebug > 50)
      {
	// printf ("STREETS mysql where: %s\n", sql_area );
	printf ("streets_rebuild_list: STREETS mapscale: %ld\n", mapscale);
      }
  }

  // Combine the Query
  g_snprintf (sql_query, sizeof (sql_query),
	      "SELECT lat1,lon1,lat2,lon2,name,streets_type_id,comment "
	      "FROM streets "		"WHERE \n"
	      "%s\n"
	      "AND %s "
	      ,sql_area
	      ,sql_in
	      );

  if (mydebug > 5)
    printf ("streets_rebuild_list: STREETS mysql query: %s\n", sql_query);

  t0 = g_print_time(0,"");

  if (dl_mysql_query (&mysql, sql_query))
    {
      fprintf (stderr, "streets_rebuild_list: Error in query: %s\n",
	       dl_mysql_error (&mysql));
      return;
    }

  // --------------------- JMO
  if (mydebug > 5)
    {
	g_print_time(t0,"query finished after %.4f seconds\n");
    }



  if (!(res = dl_mysql_store_result (&mysql)))
    {
      fprintf (stderr, "Error in store result: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
      return;
    }

  if (mydebug > 50)
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
	      if (mydebug > 50)
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
	  if (mydebug > 90)
	    {
	      printf ("streets_rebuild_list: %ld(%ld)\t", streets_list_count,
		      streets_list_limit);
	      printf ("pos: (%.4f ,%.4f) (%.4f ,%.4f)\n", lat1, lon1, lat2,
		      lon2);
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
	  (streets_list + streets_list_count)->highlight = FALSE;

	  if (NULL == row[4])
	    (streets_list + streets_list_count)->name[0] = '\0';
	  else
	    g_strlcpy ((streets_list + streets_list_count)->name,
		       row[4],
		       sizeof ((streets_list + streets_list_count)->name));

	  if (NULL == row[6])
	    (streets_list + streets_list_count)->comment[0] = '\0';
	  else
	    g_strlcpy ((streets_list + streets_list_count)->comment,
		       row[6],
		       sizeof ((streets_list + streets_list_count)->comment));
	}
      else
	{

	  if (!posxy_on_screen (streets_posx1, streets_posy1))
	    {
	      if (mydebug > 50)
		printf ("pos1 of Street not on screen:(%g,%g)\n",
			streets_posx1, streets_posy1);
	    }
	  if (!posxy_on_screen (streets_posx2, streets_posy2))
	    {
	      if (mydebug > 50)
		printf ("pos2 of Street not on screen:(%g,%g)\n",
			streets_posx2, streets_posy2);
	    }
	}
    }


  if (mydebug > 5)
    {				// print time for getting Data
	g_print_time(t0,"rows read after %.4f seconds\n");
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

  if (mydebug > 5)
    {				// print time for getting Data
	g_print_time(t0,"rows read after %.4f seconds\n");
	g_print("%ld(%d) rows\n",streets_list_count, rges);
    }

  if (mydebug > 50)
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
  gdk_gc_set_foreground (kontext, &colors.textback);

  streets_label_layout = gtk_widget_create_pango_layout (drawing_area, txt);
  if (local_config.guimode == GUI_PDA)
    pfd = pango_font_description_from_string ("Sans 8");
  else
    pfd = pango_font_description_from_string ("Sans 11");
  pango_layout_set_font_description (streets_label_layout, pfd);
  pango_layout_get_pixel_size (streets_label_layout, &width, &height);
  k = width + 4;
  k2 = height;

  gdk_gc_set_function (kontext, GDK_COPY);

  gdk_gc_set_function (kontext, GDK_AND);

  {				// Draw rectangle arround Text
    //gdk_gc_set_foreground (kontext, &colors.textbacknew);
    gdk_draw_rectangle (drawable, kontext, 1,
			posx + 13, posy - k2 / 2, k + 1, k2);
    gdk_gc_set_function (kontext, GDK_COPY);
    gdk_gc_set_foreground (kontext, &colors.black);
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
			       streets_label_layout, &colors.black, NULL);
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
      if (mydebug > 50)
	printf ("streets_draw is off\n");
      return;
    }

  if (mydebug > 50)
    printf ("streets_draw\n");

  if (streets_check_if_moved ())
    streets_rebuild_list ();

  gdks_streets_max = streets_list_count + 1;

  if (0 >= gdks_streets_max)
    return;

  if (mydebug > 50)
    printf ("streets_draw %ld segments\n", streets_list_count);

  gdks_streets = g_new0 (GdkSegment, gdks_streets_max);
  if (NULL == gdks_streets)
    {
      printf ("Problem reserving Memory for %d Street segments\n",
	      gdks_streets_max);
      gdks_streets_max = -1;
      return;
    }

  if (mydebug > 50)
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
      if (posxy_on_screen (posx1, posy1) || posxy_on_screen (posx2, posy2))
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
	      gdks_streets = g_renew (GdkSegment, gdks_streets, gdks_streets_max);
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

	  if (0 && mydebug > 90)
	    {
	      char beschrift[120];
	      g_snprintf (beschrift, sizeof (beschrift),
			  "(%.4f ,%.4f)\n(%.4f ,%.4f)\n%s",
			  (streets_list + i)->lat1, (streets_list + i)->lon1,
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
	      gdk_gc_set_foreground (kontext, &colors.red);
	      draw_small_plus_sign (posx1, posy1);
	      draw_small_plus_sign (posx2, posy2);

	    }

	  if (mydebug > 50)
	    {
	      //          printf("    1 %f,%f\n",  posx1,posy1);
	      //          printf("    2 %f,%f\n",  posx2,posy2);
	    }
	}

      // draw it if last Element or (streets_type_id/highlight) changes 
      if ((i == streets_list_count - 1)
	  ||
	  ((streets_list + i)->streets_type_id !=
	   (streets_list + i + 1)->streets_type_id)
	  ||
	  ((streets_list + i)->highlight !=
	   (streets_list + i + 1)->highlight))
	{
	  int streets_id = (streets_list + i)->streets_type_id;
	  if (!streets_id)
	    {
	      printf ("No Street ID\n");
	    }
	  else if (streets_id >= streets_type_list_max)
	    {
	      printf ("Street ID=%d >=  %d (streets_type_list_max)\n",
		      streets_id, streets_type_list_max);
	    }
	  else if (!
		   (streets_id ==
		    streets_type_list[streets_id].streets_type_id))
	    {
	      printf ("Undefined values in Street ID=%d\n", streets_id);
	    }
	  else
	    {
	      // --------------------------------- Background/Framing/Highlighting Line
	      gdk_gc_set_function (kontext, GDK_COPY);

	      gint width_factor=1;
	      if ( ( mapscale/zoom ) < 10000 ) {
		  width_factor=10000/( mapscale/zoom );
	      }

	      if ((streets_list + i)->highlight)
		{
		  gdk_gc_set_background (kontext, &colors.yellow);
		  gdk_gc_set_foreground (kontext, &colors.red);
		  gdk_gc_set_line_attributes (kontext, 6*width_factor,
					      GDK_LINE_DOUBLE_DASH, 0, 0);
		}
	      else
		{
		  gdk_gc_set_foreground (kontext,
					 &streets_type_list[streets_id].
					 color_bg);
		  gdk_gc_set_line_attributes (kontext,
					      streets_type_list[streets_id].
					      width_bg*width_factor, GDK_LINE_SOLID, 0, 0);
		}

	      gdk_draw_segments (drawable, kontext,
				 (GdkSegment *) gdks_streets,
				 gdks_streets_count + 1);

	      // --------------------------------- Foreground Line
	      gdk_gc_set_foreground (kontext,
				     &streets_type_list[streets_id].color);
	      gdk_gc_set_line_attributes (kontext,
					  streets_type_list[streets_id].width*width_factor,
					  GDK_LINE_SOLID, 0, 0);

	      // Colored inner part of Streets
	      gdk_draw_segments (drawable, kontext,
				 (GdkSegment *) gdks_streets,
				 gdks_streets_count + 1);
	    }
	  gdks_streets_count = -1;
	}
    }

  g_free (gdks_streets);

  if (mydebug > 50)
    printf
      ("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
}


/* *******************************************************
 * query all Info for streets in area arround lat/lon
*/
void
streets_query_point (gdouble lat, gdouble lon, gdouble distance)
{
  gint i;
  printf ("Query Streets: %f,%f ( %f )\n", lat, lon, distance);

  for (i = 0; i < streets_list_count; i++)
    {
      gint streets_type_id = (streets_list + i)->streets_type_id;

      if (distance_line_point ((streets_list + i)->lat1,
			       (streets_list + i)->lon1,
			       (streets_list + i)->lat2,
			       (streets_list + i)->lon2, lat, lon)
	  <= distance)
	{
	  printf ("Streets: %5d: %f,%f --> %f,%f	Name: %25s ",
		  i,
		  (streets_list + i)->lat1, (streets_list + i)->lon1,
		  (streets_list + i)->lat2, (streets_list + i)->lon2,
		  (streets_list + i)->name);
	  printf ("Type(%d): %s\t", streets_type_id,
		  streets_type_list[streets_type_id].name);
	  printf ("Comment: %s\n", (streets_list + i)->comment);
	  (streets_list + i)->highlight = TRUE;
	}
      else
	(streets_list + i)->highlight = FALSE;

    }

}
