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

    *********************************************************************

$Log$
Revision 1.3  1994/06/08 08:37:47  tweety
fix some ocurences of +- handling with coordinates by using coordinate_string2gdouble
instead of atof and strtod

Revision 1.2  2005/11/09 16:29:55  tweety
changed pixelfac for top_GPSWORLD
added comment to gpsd_nmea.sh

Revision 1.1  2005/11/06 17:24:26  tweety
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


 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gpsdrive.h>
#include <config.h>
#include <math.h>
#include "speech_out.h"
#include "speech_strings.h"

/* variables */
extern gint ignorechecksum, mydebug, debug, mapistopo;
extern gdouble lat2RadiusArray[201];
extern gdouble zero_long, zero_lat, target_long, target_lat, dist;
extern gint real_screen_x, real_screen_y;
extern gint real_psize, real_smallmenu, int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;
extern gdouble pixelfact, posx, posy, angle_to_destination;
extern gdouble direction, bearing;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle;
extern gdouble current_long, current_lat, old_long, old_lat;
extern gdouble groundspeed, milesconv;
extern gint nrmaps;
extern gint maploaded;
extern gint importactive;
extern gint zoom;
extern gdouble current_long, current_lat;
extern gint debug, mydebug;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;
extern gchar mapdir[500];
extern gint posmode;
extern gdouble posmode_x, posmode_y;
extern gchar targetname[40];
extern gint iszoomed;
extern gchar newmaplat[100], newmaplongi[100], newmapsc[100], oldangle[100];

extern gint showroute, routeitems;
extern gdouble routenearest;
extern gint forcenextroutepoint;
extern gint routemode;
wpstruct *routelist;
extern gint thisrouteline, routeitems, routepointer;
extern gint gcount, milesflag, downloadwindowactive;
extern GtkWidget *drawing_area, *drawing_bearing;
extern GtkWidget *drawing_sats, *drawing_miniimage;

extern gchar oldfilename[2048];

extern gint scaleprefered, scalewanted;
extern gint borderlimit;

gchar mapfilename[2048];
extern gchar activewpfile[200];
extern gint saytarget;

extern int havedefaultmap;

extern GtkWidget *destframe;
extern gint createroute;
extern gint displaymap_top, displaymap_map;
extern GdkPixbuf *image, *tempimage, *miniimage;



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


mapsstruct *maps = NULL;
gint message_wrong_maps_shown = FALSE;
time_t maptxtstamp = 0;
gint needreloadmapconfig = FALSE;
int havenasa = -1;

/* *****************************************************************************
 * map_koord.txt is in mappath! 
 */
void
test_loaded_map_names ()
{
  gint i;
  for (i = 0; i < nrmaps; i++)
    {
      if ((strncmp ((maps + i)->filename, "map_", 4)))
	if ((strncmp ((maps + i)->filename, "top_", 4)))
	  {
	    GString *error;
	    error = g_string_new (NULL);
	    g_string_printf (error, "%s%d\n%s\n",
			     _("Error in line "), i + 1,
			     _
			     ("I have found filenames in map_koord.txt which are\n"
			      "not map_* or top_* files. Please rename them and change the entries in\n"
			      "map_koord.txt.  Use map_* for street maps and top_* for topographical\n"
			      "maps.  Otherwise, the maps will not be displayed!"));
	    error_popup ((gpointer *) error->str);
	    g_string_free (error, TRUE);
	    message_wrong_maps_shown = TRUE;
	  }
    }
}

/* ******************************************************************
 * Check if map_koord.txt needs to be reloaded an do if so
 */
void
map_koord_check_and_reload ()
{
  gchar mappath[2048];
  struct stat buf;

  if (mydebug > 50)
    fprintf (stderr, "map_koord_check_and_reload()\n");

  /* Check for changed map_koord.txt and reload if changed */
  g_strlcpy (mappath, mapdir, sizeof (mappath));
  g_strlcat (mappath, "map_koord.txt", sizeof (mappath));

  stat (mappath, &buf);
  if (buf.st_mtime != maptxtstamp)
    {
      needreloadmapconfig = TRUE;
    }

  if (needreloadmapconfig)
    {
      loadmapconfig ();
      g_print ("%s reloaded\n", "map_koord.txt");
      maptxtstamp = buf.st_mtime;
    };
}


/* *****************************************************************************
 *  write the definitions of the map files 
 * Attention! program  writes decimal point as set in locale
 * i.eg 4.678 is in Germany 4,678 !!! 
 */
void
savemapconfig ()
{
  gchar mappath[2048];
  FILE *st;
  gint i;

  if (mapdir[strlen (mapdir) - 1] != '/')
    g_strlcat (mapdir, "/", sizeof (mapdir));

  g_strlcpy (mappath, mapdir, sizeof (mappath));
  g_strlcat (mappath, "map_koord.txt", sizeof (mappath));
  st = fopen (mappath, "w");
  if (st == NULL)
    {
      perror (mappath);
      exit (2);
    }

  for (i = 0; i < nrmaps; i++)
    {
      fprintf (st, "%s %.5f %.5f %ld\n", (maps + i)->filename,
	       (maps + i)->lat, (maps + i)->lon, (maps + i)->scale);
    }

  fclose (st);
}

/* *****************************************************************************
 *  load the definitions of the map files 
 */
gint
loadmapconfig ()
{
  gchar mappath[2048];
  FILE *st;
  gint i;
  gint max_nrmaps = 1000;
  gchar buf[1512], s1[40], s2[40], s3[40], filename[100];
  gint p, e;

  if (mydebug > 50)
    fprintf (stderr, "loadmapconfig()\n");

  init_nasa_mapfile ();
  if (mapdir[strlen (mapdir) - 1] != '/')
    g_strlcat (mapdir, "/", sizeof (mapdir));

  g_strlcpy (mappath, mapdir, sizeof (mappath));
  g_strlcat (mappath, "map_koord.txt", sizeof (mappath));
  st = fopen (mappath, "r");
  if (st == NULL)
    {
      mkdir (homedir, 0777);
      st = fopen (mappath, "w+");
      if (st == NULL)
	{
	  perror (mappath);
	  return FALSE;
	}
      st = freopen (mappath, "r", st);
      if (st == NULL)
	{
	  perror (mappath);
	  return FALSE;
	}

    }
  if (nrmaps > 0)
    g_free (maps);

  // Here I should reserve a little bit more than 1 map
  maps = g_new (mapsstruct, max_nrmaps);
  i = nrmaps = 0;
  havenasa = -1;
  while ((p = fgets (buf, 1512, st) != 0))
    {
      e = sscanf (buf, "%s %s %s %s", filename, s1, s2, s3);
      if ((mydebug > 50) && !(nrmaps % 1000))
	{
	  fprintf (stderr, "loadmapconfig(%ld)\r", nrmaps);
	}


      if (e == 4)
	{
	  g_strdelimit (s1, ",", '.');
	  g_strdelimit (s2, ",", '.');
	  g_strdelimit (s3, ",", '.');

	  g_strlcpy ((maps + i)->filename, filename, 200);
	  coordinate_string2gdouble(s1, &((maps + i)->lat));
	  coordinate_string2gdouble(s2, &((maps + i)->lon));
	  (maps + i)->scale = strtol (s3, NULL, 0);
	  i++;
	  nrmaps = i;
	  havenasa = -1;
	  if (nrmaps >= max_nrmaps)
	    {
	      max_nrmaps += 1000;
	      maps = g_renew (mapsstruct, maps, max_nrmaps);
	    }
	}
      else
	{
	  fprintf (stderr, "Line not recognized\n");
	}
    }
  fclose (st);

  needreloadmapconfig = FALSE;
  return FALSE;
}

/* *****************************************************************************
 */
void
route_next_target ()
{
  gchar str[100], buf[200], mappath[2048];
  gdouble d;
  /*  test for new route point */
  if (strcmp (targetname, "     "))
    {
      if (routemode)
	d = calcdist ((routelist + routepointer)->lon,
		      (routelist + routepointer)->lat);
      else
	d = calcdist (target_long, target_lat);

      if (d < routenearest)
	{
	  routenearest = d;
	}
      /*    g_print */
      /*      ("\nroutepointer: %d d: %.1f routenearest: %.1f routereach: %0.3f", */
      /*       routepointer, d, routenearest, ROUTEREACH); */
      if ((d <= ROUTEREACH)
	  || (d > (ROUTEREACHFACT * routenearest)) || forcenextroutepoint)
	{
	  forcenextroutepoint = FALSE;
	  if ((routepointer != (routeitems - 1)) && (routemode))
	    {
	      routenearest = 9999999999.0;
	      routepointer++;

	      /* let's say the waypoint description */
	      g_strlcpy (mappath, homedir, sizeof (mappath));
	      g_strlcat (mappath, activewpfile, sizeof (mappath));
	      saytargettext (mappath, targetname);

	      setroutetarget (NULL, -1);
	    }
	  else
	    {
	      /*  route endpoint reached */
	      if (saytarget)
		{
		  g_snprintf (str, sizeof (str),
			      "%s: %s", _("To"), targetname);
		  gtk_frame_set_label (GTK_FRAME (destframe), str);
		  createroute = FALSE;
		  routemode = FALSE;
		  saytarget = FALSE;
		  routepointer = routeitems = 0;

		  g_snprintf (buf, sizeof (buf),
			      speech_target_reached[voicelang], targetname);
		  speech_out_speek (buf);

		  /* let's say the waypoint description */
		  g_strlcpy (mappath, homedir, sizeof (mappath));
		  g_strlcat (mappath, activewpfile, sizeof (mappath));
		  saytargettext (mappath, targetname);
		}
	    }
	}
    }
}


/* ******************************************************************
 *
 */
int
create_nasa ()
{
  gint i, j;
  gchar nasaname[255];
  int nasaisvalid = FALSE;

  if ((havenasa < 0) || (!nasaisvalid))
    {
      /* delete nasamaps entries from maps list */
      for (i = 0; i < nrmaps; i++)
	{
	  if ((strcmp ((maps + i)->filename, "top_NASA_IMAGE.ppm")) == 0)
	    {
	      for (j = i; j < (nrmaps - 1); j++)
		*(maps + j) = *(maps + j + 1);
	      nrmaps--;
	      continue;
	    }
	}

      /* Try creating a nasamap and add it to the map list */
      havenasa =
	create_nasa_mapfile (current_lat, current_long, TRUE, nasaname);
      if (havenasa > 0)
	{
	  i = nrmaps;
	  nrmaps++;
	  maps = g_renew (mapsstruct, maps, (nrmaps + 2));
	  havenasa =
	    create_nasa_mapfile (current_lat, current_long, FALSE, nasaname);
	  (maps + i)->lat = current_lat;
	  (maps + i)->lon = current_long;
	  (maps + i)->scale = havenasa;
	  g_strlcpy ((maps + i)->filename, nasaname, 200);
	  if ((strcmp (oldfilename, "top_NASA_IMAGE.ppm")) == 0)
	    g_strlcpy (oldfilename, "XXXOLDMAPXXX.ppm", sizeof (oldfilename));
	}
    }
  return nasaisvalid;
}

/* ******************************************************************
 * load the map with number bestmap
 */

void
load_best_map (long long bestmap)
{

  if (bestmap != 9999999999LL)
    {
      g_strlcpy (mapfilename, (maps + bestmap)->filename,
		 sizeof (mapfilename));
      if ((strcmp (oldfilename, mapfilename)) != 0)
	{
	  g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
	  if (debug)
	    g_print ("New map: %s\n", mapfilename);
	  pixelfact = (maps + bestmap)->scale / PIXELFACT;
	  zero_long = (maps + bestmap)->lon;
	  zero_lat = (maps + bestmap)->lat;
	  mapscale = (maps + bestmap)->scale;
	  xoff = yoff = 0;
	  if (nrmaps > 0)
	    loadmap (mapfilename);
	}
    }
  else
    {				// No apropriate map found take worldmap
      if (((strcmp (oldfilename, mapfilename)) != 0) && (havedefaultmap))
	{
	  g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
	  g_strlcpy (mapfilename, "top_GPSWORLD.jpg", sizeof (mapfilename));
	  pixelfact = 88067900.43 / PIXELFACT;
	  zero_long = 0;
	  zero_lat = 0;
	  mapscale = 88067900.43;
	  xoff = yoff = 0;
	  loadmap (mapfilename);
	}
    }

}


/* *****************************************************************************
 *  We load the map 
 */
void
loadmap (char *filename)
{
  gchar mappath[2048];
  GdkPixbuf *limage;
  guchar *lpixels, *pixels;
  int i, j, k;


  if (mydebug > 10)
    fprintf (stderr, "loadmap(%s)\n", filename);

  if (maploaded)
    gdk_pixbuf_unref (image);

  if (strstr (filename, "/map_"))
    {
      mapistopo = FALSE;
    }
  else if (strstr (filename, "/top_"))
    {
      mapistopo = TRUE;
    }
  else if (!(strncmp (filename, "top_", 4)))
    mapistopo = TRUE;
  else
    mapistopo = FALSE;



  limage = gdk_pixbuf_new_from_file (filename, NULL);
  if (limage == NULL)
    {
      g_strlcpy (mappath, mapdir, sizeof (mappath));
      g_strlcat (mappath, filename, sizeof (mappath));
      limage = gdk_pixbuf_new_from_file (mappath, NULL);
    }
  if (limage == NULL)
    {
      g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/maps/%s",
		  DATADIR, filename);
      limage = gdk_pixbuf_new_from_file (mappath, NULL);

    }

  if (limage == NULL)
    havedefaultmap = FALSE;

  if (limage == NULL)
    {
      GString *error;
      error = g_string_new (NULL);
      g_string_sprintf (error, "%s\n%s\n",
			_(" Mapfile could not be loaded:"), mappath);
      error_popup ((gpointer *) error->str);
      g_string_free (error, TRUE);
      maploaded = FALSE;
      return;
    }


  if (!gdk_pixbuf_get_has_alpha (limage))
    image = limage;
  else
    {
      image = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8, 1280, 1024);
      if (image == NULL)
	{
	  fprintf (stderr,
		   "can't get image  gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8, 1280, 1024)\n");
	  exit (1);
	}
      lpixels = gdk_pixbuf_get_pixels (limage);
      pixels = gdk_pixbuf_get_pixels (image);
      if (pixels == NULL)
	{
	  fprintf (stderr,
		   "can't get pixels pixels = gdk_pixbuf_get_pixels (image);\n");
	  exit (1);
	}
      j = k = 0;
      for (i = 0; i < (1280 * 1024); i++)
	{
	  memcpy ((pixels + j), (lpixels + k), 3);
	  j += 3;
	  k += 4;
	}
      gdk_pixbuf_unref (limage);

    }

  expose_cb (NULL, NULL);
  iszoomed = FALSE;
  /*        zoom = 1; */
  xoff = yoff = 0;

  rebuildtracklist ();

  if (!maploaded)
    display_status (_("Map found!"));

  maploaded = TRUE;

  /*  draw minimap */
  if (miniimage)
    gdk_pixbuf_unref (miniimage);

  miniimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8, 128, 103);

  gdk_pixbuf_scale (image, miniimage, 0, 0, 128, 103,
		    0, 0, 0.1, 0.10, GDK_INTERP_TILES);
  expose_mini_cb (NULL, 0);



}



/* ******************************************************************
 * test if we need to load another map 
 */
void
testnewmap ()
{
  long long best = 1000000000LL;
  gdouble posx, posy;
  long long bestmap = 9999999999LL;
  gdouble pixelfactloc, bestscale = 1000000000.0, fact;
  gint i, skip, istopo = FALSE, ncount = 0;
  //    gchar str[100], buf[200];
  gdouble dif;
  static int nasaisvalid = FALSE;

  if (importactive)
    return;

  if (posmode)
    {
      current_long = posmode_x;
      current_lat = posmode_y;
    }
  else
    route_next_target ();

  /* search for suitable maps */
  if (displaymap_top)
    nasaisvalid = create_nasa ();
  nasaisvalid = FALSE;

  /* have a look through all the maps and decide which map 
   * is the best/apropriate
   * RESULT: bestmap [index in (maps + i) for the choosen map]
   */
  for (i = 0; i < nrmaps; i++)
    {
      /* check if map is topo or street map 
       * Result: istopo = TRUE/FALSE
       */
      skip = TRUE;
      if (displaymap_map)
	{
	  if (!(strncmp ((maps + i)->filename, "map_", 4)))
	    {
	      skip = FALSE;
	      istopo = FALSE;
	    }
	  if (strstr ((maps + i)->filename, "/map_"))
	    {
	      skip = FALSE;
	      istopo = FALSE;
	    }
	}
      if (displaymap_top)
	{
	  if (!(strncmp ((maps + i)->filename, "top_", 4)))
	    {
	      skip = FALSE;
	      istopo = TRUE;
	    }
	  if (strstr ((maps + i)->filename, "/top_"))
	    {
	      skip = FALSE;
	      istopo = TRUE;
	    }
	}

      if (skip)
	continue;


      /*  calcxy (&posx, &posy, (maps + i)->lon, (maps + i)->lat,1); */

      /*  Longitude */
      if (istopo == FALSE)
	posx = (lat2radius ((maps + i)->lat) * M_PI / 180)
	  * cos (M_PI * (maps + i)->lat / 180.0)
	  * (current_long - (maps + i)->lon);
      else
	posx = (lat2radius (0) * M_PI / 180)
	  * (current_long - (maps + i)->lon);


      /*  latitude */
      if (istopo == FALSE)
	{
	  posy = (lat2radius ((maps + i)->lat) * M_PI / 180)
	    * (current_lat - (maps + i)->lat);
	  dif = lat2radius ((maps + i)->lat)
	    * (1 - (cos ((M_PI * (current_long - (maps + i)->lon)) / 180.0)));
	  posy = posy + dif / 2.0;
	}
      else
	posy = (lat2radius (0) * M_PI / 180)
	  * (current_lat - (maps + i)->lat);


      pixelfactloc = (maps + i)->scale / PIXELFACT;
      posx = posx / pixelfactloc;
      posy = posy / pixelfactloc;

      /* */
      if (strcmp ("top_NASA_IMAGE.ppm", (maps + i)->filename) == 0)
	{
	  ncount++;
	}

      /* */
      if (((gint) posx > -(640 - borderlimit)) &&
	  ((gint) posx < (640 - borderlimit)) &&
	  ((gint) posy > -(512 - borderlimit)) &&
	  ((gint) posy < (512 - borderlimit)))
	{
	  if (displaymap_top)
	    if (strcmp ("top_NASA_IMAGE.ppm", (maps + i)->filename) == 0)
	      {
		/* nasa map is in range */
		nasaisvalid = TRUE;
	      }

	  if (scaleprefered)
	    {
	      if (scalewanted > (maps + i)->scale)
		fact = (gdouble) scalewanted / (maps + i)->scale;
	      else
		fact = (maps + i)->scale / (gdouble) scalewanted;

	      if (fact < bestscale)
		{
		  bestscale = fact;
		  bestmap = i;
		  /* bestcentereddist = centereddist; */
		}
	    }
	  else			/* autobestmap */
	    {
	      if ((maps + i)->scale < best)
		{
		  bestmap = i;
		  best = (maps + i)->scale;
		}
	    }
	}			/*  End of if posy> ... posx> ... */
    }				/* End of for ... i < nrmaps */

  // RESULT: bestmap [index in (maps + i) for the choosen map]

  load_best_map (bestmap);

}

/* *****************************************************************************
 * Robins hacking 
 * Show (in yellow) any downloaded maps with in +/-20% of the currently
 * requested map download also show bounds of map with a black border
 * This is currently hooked in to the drawdownloadrectangle() function 
 * but may be better else where as a seperate function that can be
 * turned on and off as requried.
 * Due to RGB bit masks the map to be downloaded will now be green 
 * so that the new download area will be visible over the top of the 
 * previous downloaded maps.
 */
void
drawloadedmaps ()
{
  int i;
  gdouble x, y, la, lo;
  gint scale, xo, yo;
  gchar sc[20];

  if (mydebug > 50)
    fprintf (stderr, "drawloadedmaps()\n");

  for (i = 0; i < nrmaps; i++)
    {
      g_strlcpy (sc, newmapsc, sizeof (sc));
      g_strdelimit (sc, ",", '.');
      scale = g_strtod (sc, NULL);

      if (maps[i].scale <= scale * 1.2 && maps[i].scale >= scale * 0.8)
	{
	  //printf("Selected map at long %lf lat %lf\n",maps[i].lat,maps[i].lon);
	  la = maps[i].lat;
	  lo = maps[i].lon;
	  //              scale=maps[i].scale;
	  calcxy (&x, &y, lo, la, zoom);
	  xo = 1280.0 * zoom * scale / mapscale;
	  yo = 1024.0 * zoom * scale / mapscale;
	  // yellow background
	  gdk_gc_set_foreground (kontext, &yellow);
	  gdk_gc_set_function (kontext, GDK_AND);
	  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	  gdk_draw_rectangle (drawable, kontext, 1, x - xo / 2,
			      y - yo / 2, xo, yo);
	  // solid border
	  gdk_gc_set_foreground (kontext, &black);
	  gdk_gc_set_function (kontext, GDK_SOLID);
	  gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
	  gdk_draw_rectangle (drawable, kontext, 0, x - xo / 2,
			      y - yo / 2, xo, yo);

	}
    }
}


/* *****************************************************************************
 * draw downloadrectangle 
 */
void
drawdownloadrectangle (gint big)
{

  if (mydebug > 50)
    fprintf (stderr, "drawdownloadrectangle()\n");

  drawloadedmaps ();

  if (downloadwindowactive)
    {
      gdouble x, y, la, lo;
      gchar longi[100], lat[100], sc[20];
      gint scale, xo, yo;

      coordinate_string2gdouble(newmaplat, &la);
      coordinate_string2gdouble(newmaplongi, &lo);

      g_strlcpy (sc, newmapsc, sizeof (sc));
      g_strdelimit (sc, ",", '.');
      scale = g_strtod (sc, NULL);

      gdk_gc_set_foreground (kontext, &green2);
      gdk_gc_set_function (kontext, GDK_AND);
      gdk_gc_set_line_attributes (kontext, 2, 0, 0, 0);
      if (big)
	{
	  calcxy (&x, &y, lo, la, zoom);
	  xo = 1280.0 * zoom * scale / mapscale;
	  yo = 1024.0 * zoom * scale / mapscale;
	  gdk_draw_rectangle (drawable, kontext, 1, x - xo / 2,
			      y - yo / 2, xo, yo);
	}
      else
	{
	  calcxymini (&x, &y, lo, la, 1);
	  xo = 128.0 * scale / mapscale;
	  yo = 102.0 * scale / mapscale;
	  gdk_draw_rectangle (drawing_miniimage->window,
			      kontext, 1, x - xo / 2, y - yo / 2, xo, yo);
	}

      gdk_gc_set_function (kontext, GDK_COPY);
    }

}
