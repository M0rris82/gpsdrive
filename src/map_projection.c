/***********************************************************************
 *
 * Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>
 *
 * Website: www.gpsdrive.de
 *
 * Disclaimer: Please do not use for navigation. 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 **********************************************************************
 */

#include "config.h"
#include <libintl.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "gpsdrive.h"
#include "speech_out.h"
#include "speech_strings.h"
#include "mapnik.h"
#include "gui.h"

/* variables */
extern gdouble pixelfact;
extern gdouble bearing;
extern gint haveposcount, blink, gblink, xoff, yoff;
extern gint maploaded;
extern gint mydebug;
extern glong mapscale;
extern gchar newmaplat[100], newmaplon[100], newmapsc[100], oldangle[100];

extern gint thisrouteline;
extern GtkWidget *bestmap_bt, *poi_draw_bt;
extern coordinate_struct coords;

extern GdkPixbuf *image;

#include "gettext.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


enum map_projections map_proj = proj_top;

/* ******************************************************************
 * Find the maptype for a given Filename
 */
enum map_projections
map_projection (char *filename)
{
  enum map_projections proj = proj_undef;

  if (strstr (filename, "expedia/"))
    proj = proj_map;
/*  landsat projection is based on scale, so pick from map_ or top_ in filename.
  else if (strstr (filename, "landsat/"))
    proj = proj_map; */
  else if (strstr (filename, "geoscience/"))
    proj = proj_map;
  else if (strstr (filename, "incrementp/"))
    proj = proj_map;
  else if (strstr (filename, "gov_au/"))
    proj = proj_map;
  else if (strstr (filename, "_map/"))
    proj = proj_map;
  else if (!strncmp(filename, "map_", 4)) /* For Compatibility */
    proj = proj_map;
  else if (strstr(filename, "/map_")) /* For Compatibility */
    proj = proj_map;
  else if (strstr (filename, "googlesat/"))
    proj = proj_googlesat;
#ifdef MAPNIK
  /* user may have previously downloaded mapnik tiles even if current build doesn't */
  else if (strstr (filename, "mapnik/"))
    proj = proj_map;  /* why not proj_mapnik ? */
#endif
  else if (strstr (filename, "NASAMAPS/"))
    proj = proj_top;
  else if (strstr (filename, "eniro/"))
    proj = proj_top;
  else if (strstr (filename, "_top/"))
    proj = proj_top;
  else if (!strncmp(filename, "top_", 4)) /* For Compatibility */
    proj = proj_top;
  else if (strstr(filename, "/top_")) /* For Compatibility */
    proj = proj_top;
#ifdef MAPNIK
  else if (strstr(filename, "mapnik"))
    proj = proj_mapnik;
#endif
  else
    {
      proj = proj_undef;
    }
  return proj;
}


/*  **********************************************************************
 * calculates lat and lon for the given position on the screen 
 */
void
calcxytopos (int posx, int posy, gdouble * mylat, gdouble * mylon, gint zoom)
{
  int px, py;
  gdouble dif, lat, lon;
	
  // hack to avoid some strange errors caused by commas that shouldn't be there	
  setlocale(LC_NUMERIC,"C");
	
  if (mydebug > 99)
    fprintf (stderr, "calcxytopos(%d,%d,__,%d)\n", posx, posy, zoom);

  // Screen xy --> pixmap xy
  px = (MAP_X_2 - posx - xoff) * pixelfact / zoom;
  py = (-MAP_Y_2 + posy + yoff) * pixelfact / zoom;

  if (proj_map == map_proj)
    {
      lat = coords.zero_lat - py / lat2radius_pi_180 (coords.current_lat);
      lat = coords.zero_lat - py / lat2radius_pi_180 (lat);
      lon = coords.zero_lon - px / (lat2radius_pi_180 (lat) * cos (DEG2RAD(lat)));

      dif = lat * (1 - (cos (DEG2RAD(fabs (lon - coords.zero_lon)))));
      lat = lat - dif / 1.5;

      lon = coords.zero_lon - px / (lat2radius_pi_180 (lat) * cos (DEG2RAD(lat)));
    }
  else if (proj_top == map_proj)
    {
      lat = coords.zero_lat - py / lat2radius_pi_180 (0);
      lon = coords.zero_lon - px / lat2radius_pi_180 (0);
    }
  else if (proj_googlesat == map_proj)
    {
      lat = coords.zero_lat - (py/1.5) / lat2radius_pi_180 (0);
      lon = coords.zero_lon - (px*1.0) / lat2radius_pi_180 (0);
    }
#ifdef MAPNIK
  else if (proj_mapnik == map_proj)
    {
	  // only use the offset
	  get_mapnik_calcxytopos(&lat, &lon, posx, posy, xoff, yoff, zoom, MAP_X_2, MAP_Y_2);
    }
#endif
  else
    {
      fprintf (stderr, "ERROR: calcxytopos: unknown map Projection\n");
      lat = 0.0; /* dummy value */
      lon = 0.0;
    }

  // Error check
  if (mydebug > 20) 
      {
	  if (lat > 360)
	      fprintf (stderr, "ERROR: calcxytopos(lat %f) out of bound\n", lat);
	  if (lat < -360)
	      fprintf (stderr, "ERROR: calcxytopos(lat %f) out of bound\n", lat);
	  if (lon > 180)
	      fprintf (stderr, "ERROR: calcxytopos(lon %f) out of bound\n", lon);
	  if (lon < -180)
	      fprintf (stderr, "ERROR: calcxytopos(lon %f) out of bound\n", lon);
      };

  *mylat = lat;
  *mylon = lon;

  if (mydebug > 90)
    fprintf (stderr, "calcxytopos(%d,%d,_,_,%d) ---> %g,%g\n", posx, posy, zoom, lat, lon);
}



/* ******************************************************************
 * calculate xy pos of given lon/lat 
 */
void calcxy (gint * posx, gint * posy, gdouble lon, gdouble lat, gint zoom)
{
  gdouble dif;
  if (mydebug > 99)
    fprintf (stderr, "calcxy(_,_,%d,%d,%d)\n", *posx, *posy, zoom);
  // Error check
  if (mydebug > 20)
      {
	  if (lat > 360)
	      fprintf (stderr, "WARNING: calcxy(lat %f) out of bound\n", lat);
	  if (lat < -360)
	      fprintf (stderr, "WARNING: calcxy(lat %f) out of bound\n", lat);
	  if (lon > 180)
	      fprintf (stderr, "WARNING: calcxy(lon %f) out of bound\n", lon);
	  if (lon < -180)
	      fprintf (stderr, "WARNING: calcxy(lon %f) out of bound\n", lon);
    };

  if (proj_map == map_proj)
      {
	  dif = lat2radius (lat) * (1 - (cos (DEG2RAD((lon - coords.zero_lon)))));
	  *posx = 0.50 + lat2radius_pi_180 (lat) * cos (DEG2RAD(lat)) * (lon - coords.zero_lon);
	  *posy = 0.50 + lat2radius_pi_180 (lat) * (lat - coords.zero_lat) + dif / 1.85;
      }
  else if (proj_top == map_proj)
      {
	  *posx = 0.50 + lat2radius_pi_180 (0.0) * (lon - coords.zero_lon);
	  *posy = 0.50 + lat2radius_pi_180 (lat) * (lat - coords.zero_lat);
      }
  else if (proj_googlesat == map_proj)
      {
	  *posx = 0.50 + 1.0 * lat2radius_pi_180 (0.0) * (lon - coords.zero_lon);
	  *posy = 0.50 + 1.5 * lat2radius_pi_180 (lat) * (lat - coords.zero_lat);
      }
#ifdef MAPNIK
  else if (proj_mapnik == map_proj)
    {
	  // only use the offset
	  get_mapnik_calcxy(posx, posy, lat, lon, xoff, yoff, zoom, MAP_X_2, MAP_Y_2);
    }
#endif
  else
	fprintf (stderr, "ERROR: calcxy: unknown map Projection\n");

  if (proj_mapnik != map_proj) {
	  // pixmap xy --> Screen xy
	  *posx = (MAP_X_2 + *posx * zoom / pixelfact) - xoff;
	  *posy = (MAP_Y_2 - *posy * zoom / pixelfact) - yoff;
  }
  if (mydebug > 90)
    fprintf (stderr, "calcxy(_,_,%d,%d,%d) ---> %g,%g\n", *posx, *posy, zoom, lat, lon);
}

/* ******************************************************************
 */
void
minimap_xy2latlon (gint px, gint py, gdouble * lon, gdouble * lat, gdouble * dif)
{
  *lat = coords.zero_lat - py / lat2radius_pi_180 (coords.current_lat);
  *lat = coords.zero_lat - py / lat2radius_pi_180 (*lat);
  *lon = coords.zero_lon - px / (lat2radius (*lat) * cos (DEG2RAD(*lat)));

  if (proj_top == map_proj)
    {
      *dif = (*lat) * (1 - (cos (DEG2RAD(fabs (*lon - coords.zero_lon)))));
      *lat = (*lat) - (*dif) / 1.5;
    }
  else if (proj_map == map_proj)
    *dif = 0;
  else if (proj_googlesat == map_proj)
    {
      *dif = (*lat) * (1 - (cos (DEG2RAD(fabs (*lon - coords.zero_lon)))));
      *lat = (*lat) - (*dif) / 1.5;
    }
#ifdef MAPNIK
  else if (proj_mapnik == map_proj) {
	  *dif = 0;
	  get_mapnik_minixy2latlon(px, py, lat, lon);
  }
#endif
  else {
    printf ("ERROR: minimap_xy2latlon: unknown map Projection\n");
  *lon = coords.zero_lon - px / (lat2radius_pi_180 (*lat) * cos (DEG2RAD(*lat)));
  }
}

/* ******************************************************************
 * calculate xy position in mini map window from given lat/lon
 */
void calcxymini (gint * posx, gint * posy, gdouble lon, gdouble lat, gint zoom)
{
  
#ifdef MAPNIK
  if (proj_mapnik == map_proj) {
	  get_mapnik_minicalcxy(posx, posy, lat, lon, zoom);
	  return;
  }
#endif
  gdouble dif;
  if (proj_map == map_proj)
    *posx = lat2radius_pi_180 (lat) * cos (DEG2RAD(lat)) * (lon - coords.zero_lon);
  else if (proj_top == map_proj)
    *posx = lat2radius_pi_180 (0) * (lon - coords.zero_lon);
  else if (proj_googlesat == map_proj)
    *posx = lat2radius_pi_180 (0) * (lon - coords.zero_lon);
  else
    printf ("Eroor: calcxymini: unknown Projection\n");

  *posx = 64 + *posx * zoom / (10 * pixelfact);
  *posx = *posx;
  if (proj_map == map_proj)
    {
      dif = lat2radius (lat) * (1 - (cos (DEG2RAD(lon - coords.zero_lon))));
      *posy = lat2radius_pi_180 (lat) * (lat - coords.zero_lat);
      *posy = *posy + dif / 1.85;
    }
  else if (proj_top == map_proj)
    *posy = lat2radius_pi_180 (lat) * (lat - coords.zero_lat);
  else if (proj_googlesat == map_proj)
    *posy = lat2radius_pi_180 (lat) * (lat - coords.zero_lat);
  else
    printf ("Eroor: calcxymini: unknown Projection\n");

  *posy = 51 - *posy * zoom / (10 * pixelfact);
  *posy = *posy;
}
