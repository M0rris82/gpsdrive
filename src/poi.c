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
  Revision 1.19  2005/04/20 23:33:49  tweety
  reformatted source code with anjuta
  So now we have new indentations

  Revision 1.18  2005/04/13 19:58:31  tweety
  renew indentation to 4 spaces + tabstop=8

  Revision 1.17  2005/04/12 06:14:20  tweety
  poi.c:
  Added Full Path Search for Icons
  streets.c:
  implement basic colors for streets
  added text to streets in debug mode
	
  Revision 1.16  2005/04/10 21:50:50  tweety
  reformatting c-sources
	
  Revision 1.15  2005/04/07 06:35:01  tweety
  Error handling for g_renew
  correct to extern MYSQL mysql;
  start getting color from streets_type

  Revision 1.14  2005/04/06 19:38:17  tweety
  use disable/enable keys to improove spee in database creation
  add draw_small_plus_sign, which is used if we would have too many waypoints to display
  extract draw_text from draw_poi loop

  Revision 1.13  2005/03/30 10:03:38  tweety
  corrected problem with sigSEG if sqlmode is off
  Added another bunch of comments

  Revision 1.12  2005/03/29 01:59:01  tweety
  another set of minor Bugfixes

  Revision 1.11  2005/03/27 18:22:50  tweety
  only draw cross if less than 5000 poi found

  Revision 1.10  2005/03/27 00:44:42  tweety
  eperated poi_type_list and streets_type_list
  and therefor renaming the fields
  added drop index before adding one
  poi.*: a little bit more error handling
  disabling poi and streets if sql is disabled
  changed som print statements from \n.... to ...\n
  changed some debug statements from debug to mydebug

  Revision 1.9  2005/03/15 08:40:13  tweety
  eliminate doublicate pfd description

  Revision 1.8  2005/02/22 08:18:51  tweety
  change leveling system to simpler scale marking for decission 
  what to show on display
  column_names(DBFuncs.pm get data from Database
  added functions add_index drop_index
  added language to type Database
  for some Data split unpack and mirror Directories
  for some add lat/lon min/max to get faster import for testing
  added POI::DBFuncs::segments_add; this will later be the point 
  to do some excerptions and combinations
  on the street data

  Revision 1.7  2005/02/17 09:46:34  tweety
  minor changes

  Revision 1.6  2005/02/13 22:57:00  tweety
  WDB Support

  Revision 1.5  2005/02/13 14:06:54  tweety
  start street randering functions. reading from the database streets 
  and displaying it on the screen
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
extern gint debug, mydebug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats,
	*drawing_miniimage;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;

char txt[5000];
PangoLayout *poi_label_layout;
#include "mysql.h"

extern MYSQL mysql;
extern MYSQL_RES *res;
extern MYSQL_ROW row;

// keep actual visible POIs in Memory
poi_struct *poi_list;
glong poi_nr;			// current number of poi to count
glong poi_max;			// max index of POIs actually in memory
glong poi_limit = -1;		// max allowed index (if you need more you have to alloc memory)
gint poi_draw = FALSE;

gchar poi_label_font[100];
GdkColor poi_colorv;
PangoFontDescription *pfd;
PangoLayout *poi_label_layout;

#define poi_type_list_string_length 80
typedef struct
{
	gint poi_type_id;
	gchar name[poi_type_list_string_length];
	gchar icon_name[poi_type_list_string_length];
	GdkPixbuf *icon;
} poi_type_struct;
#define poi_type_list_max 4000
poi_type_struct poi_type_list[poi_type_list_max];
int poi_type_list_count = 0;

/* ******************************************************************   */

void poi_rebuild_list (void);
void get_poi_type_list (void);

/* *******************************************************
 */
void
draw_text (char *txt, gdouble posx, gdouble posy)
{
	gint width, height;
	gint k, k2;

	gdk_gc_set_foreground (kontext, &textback);

	poi_label_layout = gtk_widget_create_pango_layout (drawing_area, txt);
	if (pdamode)
		pfd = pango_font_description_from_string ("Sans 8");
	else
		pfd = pango_font_description_from_string ("Sans 11");

	if (poi_max > 100)
		pfd = pango_font_description_from_string ("Sans 8");

	pango_layout_set_font_description (poi_label_layout, pfd);
	pango_layout_get_pixel_size (poi_label_layout, &width, &height);
	k = width + 4;
	k2 = height;

	gdk_gc_set_function (kontext, GDK_COPY);

	gdk_gc_set_function (kontext, GDK_AND);

	{			// Draw rectangle arround Text
		// gdk_gc_set_foreground (kontext, &textbacknew);
		gdk_gc_set_foreground (kontext, &grey);
		gdk_draw_rectangle (drawable, kontext, 1,
				    posx + 13, posy - k2 / 2, k + 1, k2);

	}

	poi_label_layout = gtk_widget_create_pango_layout (drawing_area, txt);
	pango_layout_set_font_description (poi_label_layout, pfd);

	gdk_draw_layout_with_colors (drawable, kontext,
				     posx + 15, posy - k2 / 2,
				     poi_label_layout, &black, NULL);
	if (poi_label_layout != NULL)
		g_object_unref (G_OBJECT (poi_label_layout));
	/* freeing PangoFontDescription, cause it has been copied by prev. call */
	pango_font_description_free (pfd);
}

/* *******************************************************
 */
void
poi_init (void)
{
	poi_limit = 40000;
	poi_list = g_new (poi_struct, poi_limit);
	if (poi_list == NULL)
	{
		g_print ("Error: Cannot allocate Memory for %ld poi\n",
			 poi_limit);
		poi_limit = -1;
		return;
	}

	get_poi_type_list ();
	//poi_rebuild_list ();
}


/* *********************************************************
 */
gdouble poi_lat_lr = 0, poi_lon_lr = 0;
gdouble poi_lat_ul = 0, poi_lon_ul = 0;

int
poi_check_if_moved (void)
{
	gdouble lat_lr, lon_lr;
	gdouble lat_ul, lon_ul;

	if (poi_lat_lr == 0 && poi_lon_lr == 0 &&
	    poi_lat_ul == 0 && poi_lon_ul == 0)
		return 1;

	calcxytopos (SCREEN_X, SCREEN_Y, &lat_lr, &lon_lr, zoom);
	calcxytopos (0, 0, &lat_ul, &lon_ul, zoom);

	if (poi_lat_lr == lat_lr && poi_lon_lr == lon_lr &&
	    poi_lat_ul == lat_ul && poi_lon_ul == lon_ul)
		return 0;
	return 1;
}

/* ****************************************************************** */
/* get a list of all possible types and load there icons */
void
get_poi_type_list (void)
{
	char sql_query[3000];


	if (!usesql)
		return;

	if (mydebug)
		printf ("get_poi_type_list()\n");

	{			// Delete poi_type_list
		int i;
		for (i = 0; i < poi_type_list_max; i++)
		{
			poi_type_list[i].icon = NULL;
		}
	}

	g_snprintf (sql_query, sizeof (sql_query),
		    "SELECT poi_type_id,name,symbol,description FROM poi_type ORDER BY poi_type_id");

	if (mydebug)
		fprintf (stderr, "get_poi_type_list: query: %s\n", sql_query);


	if (dl_mysql_query (&mysql, sql_query))
	{
		fprintf (stderr, "Error in query: %s\n",
			 dl_mysql_error (&mysql));
		return;
	}


	if (!(res = dl_mysql_store_result (&mysql)))
	{
		fprintf (stderr, "Error in store results: %s\n",
			 dl_mysql_error (&mysql));
		return;
	}

	int counter = 0;
	while ((row = dl_mysql_fetch_row (res)))
	{
		int index = (gint) g_strtod (row[0], NULL);
		if (index >= poi_type_list_max)
		{
			fprintf (stderr,
				 "Typet_list: index(%d) > poi_type_list_max(%d)\n",
				 index, poi_type_list_max);
		}
		else
		{
			if (poi_type_list_count < index)
				poi_type_list_count = index;

			poi_type_list[index].poi_type_id = index;

			if (row[1] == NULL)
			{
				poi_type_list[index].name[0] = '\0';
			}
			else
			{
				g_strlcpy (poi_type_list[index].name, row[1],
					   sizeof (poi_type_list[index].
						   name));
			}


			if (row[2] == NULL)
			{
				poi_type_list[index].icon_name[0] = '\0';
				fprintf (stderr,
					 "poi_type %d: No Icon Name returned\n",
					 index);
			}
			else
			{

				/* This results in an SigSeg later */
				/*
				 * fprintf(stderr,"get_poi_type_list:%d -> '%s'\n",index,row[2]);
				 * g_strlcpy ( poi_type_list[index].icon_name, 
				 * row[2],
				 * poi_type_list_string_length-1
				 * );
				 */
			}

			{	// load icon
				if (row[2] == NULL)
				{
					poi_type_list[index].icon = NULL;
				}
				else
				{
					gchar icon_name[80];
					gchar path[1024];
					g_strlcpy (icon_name, row[2],
						   sizeof (icon_name));

					// User config Directory
					g_snprintf (path, sizeof (path),
						    "%sicons/%s", homedir,
						    icon_name);
					poi_type_list[index].icon =
						gdk_pixbuf_new_from_file
						(path, NULL);

					if (poi_type_list[index].icon == NULL)
					{	// In System Directory
						g_snprintf (path,
							    sizeof (path),
							    "%s/gpsdrive/icons/%s",
							    DATADIR,
							    icon_name);
						poi_type_list[index].icon =
							gdk_pixbuf_new_from_file
							(path, NULL);
					}
					if (poi_type_list[index].icon == NULL)
					{	// Full Path
						poi_type_list[index].icon =
							gdk_pixbuf_new_from_file
							(icon_name, NULL);
					}

					if (poi_type_list[index].icon == NULL)
					{
						if (debug)
							printf ("get_poi_type_list: %3d:Icon '%s' \tfor '%s'\tnot found\n", index, icon_name, poi_type_list[index].name);
					}
					else
					{
						if (mydebug)
							printf ("get_poi_type_list: %3d:Icon '%s' \tfor '%s'\tloaded from %s \n", index, icon_name, poi_type_list[index].name, path);
						counter++;
					}
				}
			}
		}
	}


	if (!dl_mysql_eof (res))
	{
		fprintf (stderr, "Error in dl_mysql_eof: %s\n",
			 dl_mysql_error (&mysql));
		return;
	}


	dl_mysql_free_result (res);

	if (mydebug)
		fprintf (stderr,
			 "get_poi_type_list: Loaded %d Icons for poi_types 0 - %d\n",
			 counter, poi_type_list_count);
}

/* *******************************************************
 * if zoom, xoff, yoff or map are changed 
 * TODO: use the real datatype for reading from database
 * (don't convert string to double)
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


	if (!usesql)
		return;

	if (!poi_draw)
	{
		if (debug)
			printf ("poi_rebuild_list: POI_draw is off\n");
		return;
	}

	if (mydebug)
	{
		printf ("poi_rebuild_list: Start\t\t\t\t\t\tvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
	}


	if (!maploaded)
		return;

	if (importactive)
		return;


	// calculate the start and stop for lat/lon according to the displayed section
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

	gdouble poi_posx, poi_posy;
	gettimeofday (&t, NULL);
	ti = t.tv_sec + t.tv_usec / 1000000.0;

	{			// gernerate mysql ORDER string
		/*
		 * char sql_order_numbers[5000];
		 * g_snprintf (sql_order_numbers, sizeof (sql_order),
		 * "(abs(%.6f - lat)+abs(%.6f - lon))"
		 * ,lat_mid,lon_mid);
		 * g_strdelimit (sql_order_numbers, ",", '.'); // For different LANG
		 * 
		 * g_snprintf (sql_order, sizeof (sql_order),
		 * "order by scale_min,%s ",sql_order_numbers);
		 */
		g_snprintf (sql_order, sizeof (sql_order),
			    "order by scale_min,scale_max ");
		/*
		 * g_snprintf (sql_order, sizeof (sql_order),
		 * "order by scale ");
		 */
		if (mydebug)
			printf ("poi_rebuild_list: POI mysql order: %s\n",
				sql_order);
	}

	{			// Limit the select with WHERE min_lat<lat<max_lat AND min_lon<lon<max_lon
		g_snprintf (sql_where, sizeof (sql_where),
			    "\tWHERE ( lat BETWEEN %.6f AND %.6f ) \n"
			    "\tAND   ( lon BETWEEN %.6f AND %.6f ) \n"
			    "\tAND   ( %ld  BETWEEN scale_min AND scale_max) \n",
			    lat_min, lat_max, lon_min, lon_max, mapscale);
		g_strdelimit (sql_where, ",", '.');	// For different LANG
		if (mydebug)
		{
			//printf ("POI mysql where: %s\n", sql_where );
			printf ("poi_rebuild_list: POI mapscale: %ld\n",
				mapscale);
		}
	}


	g_snprintf (sql_query, sizeof (sql_query),
		    // "SELECT lat,lon,alt,type_id,proximity "
		    "SELECT lat,lon,name,poi_type_id " "FROM poi "
		    //            "LEFT JOIN oi_ type ON poi_type_id = type.poi_type_id "
		    "%s %s LIMIT 40000", sql_where, sql_order);
	/*    dbwherestring,sql_order,lat,lon);  */

	if (debug)
		printf ("poi_rebuild_list: POI mysql query: %s\n", sql_query);

	if (dl_mysql_query (&mysql, sql_query))
	{
		printf ("Error in query: \n");
		fprintf (stderr, "Error in query: %s\n",
			 dl_mysql_error (&mysql));
		return;
	}

	if (!(res = dl_mysql_store_result (&mysql)))
	{
		fprintf (stderr, "Error in store results: %s\n",
			 dl_mysql_error (&mysql));
		return;
	}

	rges = r = 0;
	poi_nr = 0;
	while ((row = dl_mysql_fetch_row (res)))
	{
		rges++;
		gdouble lat, lon;


		if (mydebug)
			fprintf (stderr, "Query Result: %s\t%s\t%s\t%s\n",
				 row[0], row[1], row[2], row[3]);

		lat = g_strtod (row[0], NULL);
		lon = g_strtod (row[1], NULL);
		calcxy (&poi_posx, &poi_posy, lon, lat, zoom);

		if ((poi_posx > -50) && (poi_posx < (SCREEN_X + 50)) &&
		    (poi_posy > -50) && (poi_posy < (SCREEN_Y + 50)))
		{
			// get next free mem for point
			poi_nr++;
			if (poi_nr > poi_limit)
			{
				poi_limit += 10000;
				if (debug)
					g_print ("Try to allocate Memory for %ld poi\n", poi_limit);

				poi_list =
					g_renew (poi_struct, poi_list,
						 poi_limit);
				if (poi_list == NULL)
				{
					g_print ("Error: Cannot allocate Memory for %ld poi\n", poi_limit);
					poi_limit = -1;
					return;
				}
				// TODO: check if g_renew failed
			}

			// Save retrieved poi information into structure
			(poi_list + poi_nr)->lat = lat;
			(poi_list + poi_nr)->lon = lon;
			(poi_list + poi_nr)->x = poi_posx;
			(poi_list + poi_nr)->y = poi_posy;
			g_strlcpy ((poi_list + poi_nr)->name, row[2],
				   sizeof ((poi_list + poi_nr)->name));
			(poi_list + poi_nr)->poi_type_id =
				(gint) g_strtod (row[3], NULL);
			if (debug)
			{
				g_snprintf ((poi_list + poi_nr)->name,
					    sizeof ((poi_list +
						     poi_nr)->name),
					    "%s\n(%.4f ,%.4f)",
					    //                  (poi_list + poi_nr)->poi_type_id,
					    row[2], lat, lon);
				/*
				 * `type_id` int(11) NOT NULL default \'0\',
				 * `alt` double default \'0\',
				 * `proximity` float default \'0\',
				 * `comment` varchar(255) default NULL,
				 * `scale_min` smallint(6) NOT NULL default \'0\',
				 * `scale_max` smallint(6) NOT NULL default \'0\',
				 * `last_modified` date NOT NULL default \'0000-00-00\',
				 * `url` varchar(160) NULL ,
				 * `address_id` int(11) default \'0\',
				 * `source_id` int(11) NOT NULL default \'0\',
				 */
			}
			//(poi_list + poi_nr)->wp_id      = g_strtol (row[0], NULL);
			//      (poi_list + poi_nr)->alt        = g_strtod(row[2], NULL);
			//(poi_list + poi_nr)->name[80] = row[4];
			//(poi_list + poi_nr)->poi_type_id    = g_strtod(row[5], NULL); 
			//(poi_list + poi_nr)->proximity  = g_strtod(row[6], NULL);
			//(poi_list + poi_nr)->comment[255 ] = row[7]; 
			//(poi_list + poi_nr)->scale_min     = row[8];  
			//(poi_list + poi_nr)->scale_max     = row[8];  
			//(poi_list + poi_nr)->last_modified = row[8] 
			//(poi_list + poi_nr)->url[160]      = row[10]; 
			//(poi_list + poi_nr)->address_id    = row[11];
			//(poi_list + poi_nr)->source_id     = row[12]; 

			/*
			 * if (debug) 
			 * printf ("DB: %f %f \t( x:%f, y:%f )\t%s\n",
			 * (poi_list + poi_nr)->lat, (poi_list + poi_nr)->lon, 
			 * (poi_list + poi_nr)->x, (poi_list + poi_nr)->y, 
			 * (poi_list + poi_nr)->name
			 * );
			 */
		}
	}


	poi_max = poi_nr;

	// print time for getting Data
	gettimeofday (&t, NULL);
	ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
	if (debug)
		printf (_("%ld(%d) rows read in %.2f seconds\n"), poi_max,
			rges, (gdouble) ti);


	/* remember where the data belongs to */
	poi_lat_lr = lat_lr;
	poi_lon_lr = lon_lr;
	poi_lat_ul = lat_ul;
	poi_lon_ul = lon_ul;

	if (!dl_mysql_eof (res))
		return;

	dl_mysql_free_result (res);

	if (mydebug)
	{
		printf ("poi_rebuild_list: End \t\t\t\t\t\t");
		printf ("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	}
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

	gint i;

	if (!usesql)
		return;

	if (importactive)
		return;

	if (!maploaded)
		return;

	if (!(poi_draw))
	{
		if (mydebug)
			printf ("poi_draw_list: POI_draw is off\n");
		return;
	}


	if (mydebug)
		printf ("poi_draw_list: Start\t\t\tvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");

	if (poi_check_if_moved ())
		poi_rebuild_list ();


	/* ------------------------------------------------------------------ */
	/*  draw poi_list points */
	if (mydebug)
		printf ("poi_draw_list: drawing %ld points\n", poi_max);

	for (i = 0; i < poi_max; i++)
	{
		gdouble posx, posy;

		posx = (poi_list + i)->x;
		posy = (poi_list + i)->y;

		if ((posx >= 0) && (posx < SCREEN_X) &&
		    (posy >= 0) && (posy < SCREEN_Y))
		{


			/*
			 * if ( mydebug ) 
			 * printf ("POI Draw: %f %f \t( x:%f, y:%f )\t%s\n",
			 * (poi_list + poi_nr)->lat, (poi_list + poi_nr)->lon, 
			 * (poi_list + poi_nr)->x, (poi_list + poi_nr)->y, 
			 * (poi_list + poi_nr)->name
			 * );
			 */

			gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);

			g_strlcpy (txt, (poi_list + i)->name, sizeof (txt));

			//    if ((drawicon (posx, posy, "HOTEL")) == 0)
			{
				GdkPixbuf *icon;
				int icon_index = (poi_list + i)->poi_type_id;
				icon = poi_type_list[icon_index].icon;

				if (icon != NULL && icon_index > 0)
				{
					if (poi_max < 2000)
						gdk_draw_pixbuf (drawable,
								 kontext,
								 icon, 0, 0,
								 posx - 12,
								 posy - 12,
								 24, 24,
								 GDK_RGB_DITHER_NONE,
								 0, 0);
				}
				else
				{
					gdk_gc_set_foreground (kontext, &red);
					if (poi_max < 20000)
					{	// Only draw mall + if more than ... Points 
						draw_plus_sign (posx, posy);
					}
					else
					{
						draw_small_plus_sign (posx,
								      posy);
					}
				}

				// Only draw Text if less than 1000 Points are to be displayed
				if (poi_max < 1000)
				{
					draw_text (txt, posx, posy);
				}
			}
		}


	}
	if (mydebug)
		printf ("poi_draw_list: End\t\t\t^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
}
