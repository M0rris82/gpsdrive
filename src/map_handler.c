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
#include "gpsdrive.h"
#include "speech_out.h"
#include "speech_strings.h"

/* variables */
extern gint ignorechecksum, mydebug, debug;
extern gdouble zero_lon, zero_lat, target_lon, target_lat, dist;
extern gint real_screen_x, real_screen_y;
extern gint real_psize, real_smallmenu, int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;
extern gdouble pixelfact, posx, posy, angle_to_destination;
extern gdouble direction, bearing;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle;
extern gdouble current_lon, current_lat, old_lon, old_lat;
extern gdouble trip_lat, trip_lon;
extern gdouble groundspeed, milesconv;
extern gint nrmaps;
extern gint maploaded;
extern gint importactive;
extern gint zoom;
extern gdouble current_lon, current_lat;
extern gint debug, mydebug;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;
extern gchar local_config_mapdir[500];
extern gint posmode;
extern gint selected_wp_mode;
extern gdouble posmode_lon, posmode_lat;
extern gchar targetname[40];
extern gint iszoomed;
extern gchar newmaplat[100], newmaplon[100], newmapsc[100], oldangle[100];
extern gint needtosave;

extern gint showroute, routeitems;
extern gdouble routenearest;
extern gint forcenextroutepoint;
extern gint routemode;
wpstruct *routelist;
extern gint thisrouteline, routeitems, routepointer;
extern gint gcount, milesflag, downloadwindowactive;
extern GtkWidget *drawing_area, *drawing_bearing;
extern GtkWidget *drawing_sats, *drawing_miniimage;
extern GtkWidget *bestmap_bt, *poi_draw_bt, *streets_draw_bt;
extern GtkWidget *posbt;
extern gint streets_draw;

extern gchar oldfilename[2048];

extern gint scaleprefered_not_bestmap, scalewanted;
extern gint borderlimit;

gchar mapfilename[2048];
extern gchar activewpfile[200];
extern gint saytarget;

extern int havedefaultmap;

extern GtkWidget *destframe;
extern gint createroute;
extern GdkPixbuf *image, *tempimage, *miniimage;
extern GtkWidget *scaler_widget;
extern GtkWidget *scaler_left_bt, *scaler_right_bt;
extern GtkObject *scaler_adj;

#include "gettext.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#define Deg2Rad(x) (x*M_PI/180.0)

enum map_projections map_projection (char *filename);


mapsstruct *maps = NULL;
gint message_wrong_maps_shown = FALSE;
time_t maptxtstamp = 0;
gint needreloadmapconfig = FALSE;
int havenasa = -1;

GtkWidget *maptogglebt, *topotogglebt;


gint max_display_map = 0;
map_dir_struct *display_map;
gint displaymap_top = TRUE;
gint displaymap_map = TRUE;

/* *****************************************************************************
 */
gint
bestmap_cb (GtkWidget * widget, guint datum)
{
    if (datum == 1)
	scaleprefered_not_bestmap = !scaleprefered_not_bestmap;
    if (!scaleprefered_not_bestmap)
	{
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmap_bt),
					  TRUE);
	    gtk_widget_set_sensitive (scaler_right_bt, FALSE);
	    gtk_widget_set_sensitive (scaler_left_bt, FALSE);
	    if (scaler_widget)
		gtk_widget_set_sensitive (scaler_widget, FALSE);
	}
    else
	{
	    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmap_bt),
					  FALSE);
	    gtk_widget_set_sensitive (scaler_right_bt, TRUE);
	    gtk_widget_set_sensitive (scaler_left_bt, TRUE);
	    if (scaler_widget)
		gtk_widget_set_sensitive (scaler_widget, TRUE);
	}
    needtosave = TRUE;
    return TRUE;
}

/* *****************************************************************************
 */
gint
maptoggle_cb (GtkWidget * widget, guint datum)
{
  displaymap_map = !displaymap_map;
  if (displaymap_map)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (maptogglebt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (maptogglebt), FALSE);
  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
topotoggle_cb (GtkWidget * widget, guint datum)
{
  displaymap_top = !displaymap_top;
  if (displaymap_top)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (topotogglebt), TRUE);
  else
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (topotogglebt), FALSE);
  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
display_maps_cb (GtkWidget * widget, guint datum)
{
  if (gtk_toggle_button_get_active
      (GTK_TOGGLE_BUTTON (display_map[datum].checkbox)))
    display_map[datum].to_be_displayed = TRUE;
  else
    display_map[datum].to_be_displayed = FALSE;

  int i;
  for (i = 0; i < max_display_map; i++)
    {
      char tbd = display_map[i].to_be_displayed ? 'D' : '_';
      printf ("Found %s,%c\n", display_map[i].name, tbd);
    }

  needtosave = TRUE;
  return TRUE;
}

/* ******************************************************************
 */
GtkWidget *
make_display_map_controlls ()
{
  GtkWidget *frame_maptype;
  GtkWidget *vbox_map_controlls;
  GtkTooltips *tooltips;
  tooltips = gtk_tooltips_new ();

  // Frame
  frame_maptype = gtk_frame_new (_("Map Controls"));
  vbox_map_controlls = gtk_vbox_new (TRUE, 1 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame_maptype), vbox_map_controlls);

  // Checkbox ---- STREETS Draw
  streets_draw_bt = gtk_check_button_new_with_label (_("draw _Streets"));
  gtk_button_set_use_underline (GTK_BUTTON (streets_draw_bt), TRUE);
  if (!streets_draw)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				    (streets_draw_bt), TRUE);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), streets_draw_bt,
			_("Draw Streets found in mySQL"), NULL);
  gtk_signal_connect (GTK_OBJECT (streets_draw_bt), "clicked",
		      GTK_SIGNAL_FUNC (streets_draw_cb), (gpointer) 1);
  gtk_box_pack_start (GTK_BOX (vbox_map_controlls), streets_draw_bt, FALSE,FALSE, 0 * PADDING);
  

  // Checkbox ---- Best Map
  bestmap_bt = gtk_check_button_new_with_label (_("Auto _best map"));
  gtk_button_set_use_underline (GTK_BUTTON (bestmap_bt), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox_map_controlls), bestmap_bt, FALSE, FALSE,0 * PADDING);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), bestmap_bt,
			_("Always select the most detailed map available"),
			NULL);
  
  if (!scaleprefered_not_bestmap)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmap_bt),  TRUE);
  gtk_signal_connect (GTK_OBJECT (bestmap_bt), "clicked",
		      GTK_SIGNAL_FUNC (bestmap_cb), (gpointer) 1);
  
  // Checkbox ---- Pos Mode
  posbt = gtk_check_button_new_with_label (_("Pos. _mode"));
  gtk_button_set_use_underline (GTK_BUTTON (posbt), TRUE);
  gtk_signal_connect (GTK_OBJECT (posbt),     "clicked", GTK_SIGNAL_FUNC (pos_cb),    (gpointer) 1);
  gtk_box_pack_start (GTK_BOX (vbox_map_controlls), posbt, FALSE, FALSE,0 * PADDING);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), posbt,
			_("Turn position mode on. "
			  "You can move on the map with the left mouse button click."
			  " Clicking near the border switches to the proximate map."),
			NULL);
  
  
  return frame_maptype;
}
  
/* ******************************************************************
 */
GtkWidget *
make_display_map_checkboxes()
{
  GtkWidget *frame_maptype;
  GtkWidget *vbox3;
  GtkTooltips *tooltips;

  // Frame
  frame_maptype = gtk_frame_new (_("Shown map type"));
  vbox3 = gtk_vbox_new (TRUE, 1 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame_maptype), vbox3);

  if (0)
    {
      // Checkbox ---- Show Map: map_
      maptogglebt = gtk_check_button_new_with_label (_("Street map"));
      if (displaymap_map)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (maptogglebt), TRUE);
      gtk_signal_connect (GTK_OBJECT (maptogglebt),
			  "clicked", GTK_SIGNAL_FUNC (maptoggle_cb),
			  (gpointer) 1);
      gtk_box_pack_start (GTK_BOX (vbox3), maptogglebt, FALSE, FALSE,
			  0 * PADDING);

      // Checkbox ---- Show Map: top_
      topotogglebt = gtk_check_button_new_with_label (_("Topo map"));
      if (displaymap_top)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (topotogglebt), TRUE);

      gtk_signal_connect (GTK_OBJECT (topotogglebt),
			  "clicked", GTK_SIGNAL_FUNC (topotoggle_cb),
			  (gpointer) 1);

      gtk_box_pack_start (GTK_BOX (vbox3), topotogglebt, FALSE, FALSE,
			  0 * PADDING);

    }

  tooltips = gtk_tooltips_new ();
  glong i;
  for (i = 0; i < max_display_map; i++)
    {
      // Checkbox ---- Show Map: name xy
      gchar display_name[100];


      if (mydebug > 1)
	g_snprintf (display_name, sizeof (display_name), "%s (%d)",
		    _(display_map[i].name), display_map[i].count);
      else
	g_snprintf (display_name, sizeof (display_name), "%s",
		    _(display_map[i].name));


      display_map[i].count++;

      display_map[i].checkbox
	= gtk_check_button_new_with_label (display_name);
      
      if (display_map[i].to_be_displayed)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON
				      (display_map[i].checkbox), TRUE);

      gtk_signal_connect (GTK_OBJECT (display_map[i].checkbox),
			  "clicked", GTK_SIGNAL_FUNC (display_maps_cb),
			  (gpointer) i);

      gtk_box_pack_start (GTK_BOX (vbox3), display_map[i].checkbox, FALSE,
			  FALSE, 0 * PADDING);
    }

  return frame_maptype;
}

/* ******************************************************************
 * extract the directory part of the File and then 
 * add it to the  display_map[] structure
 * returns the index of the display_map[] structure
 */
int
add_map_dir (gchar * filename)
{

  gint i;

  /* memorize map dir names */
  if (mydebug > 99)
    fprintf (stderr, "add_map_dir(%s)\n", filename);

  gchar map_dir[200];

  g_strlcpy (map_dir, filename, sizeof (map_dir));
  char *slash_pos = strstr (map_dir, "/");
  if (slash_pos)
    slash_pos[0] = '\0';
  else
    g_strlcpy (map_dir, "no_dir", sizeof (map_dir));

  for (i = 0; i < max_display_map; i++)
    {
      if (!strcmp (display_map[i].name, map_dir))
	{
	  display_map[i].count++;
	  //printf("Found %s,%s\n",display_map[i].name,map_dir);
	  return i;
	}
    }
  if (i >= max_display_map)
    {
      max_display_map += 1;
      display_map = g_renew (map_dir_struct, display_map, max_display_map);
    }
  g_strlcpy (display_map[i].name, map_dir, sizeof (display_map[i].name));
  display_map[i].to_be_displayed = TRUE;
  display_map[i].count++;
  return i;
}

/* *****************************************************************************
 * map_koord.txt is in mappath! 
 */
void
test_loaded_map_names ()
{
  gint i;
  for (i = 0; i < nrmaps; i++)
    {
      if (proj_undef == map_projection ((maps + i)->filename))
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
  g_strlcpy (mappath, local_config_mapdir, sizeof (mappath)); 
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

  if (local_config_mapdir[strlen (local_config_mapdir) - 1] != '/')
    g_strlcat (local_config_mapdir, "/", sizeof (local_config_mapdir));

  g_strlcpy (mappath, local_config_mapdir, sizeof (mappath));
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
  if (local_config_mapdir[strlen (local_config_mapdir) - 1] != '/')
    g_strlcat (local_config_mapdir, "/", sizeof (local_config_mapdir));

  g_strlcpy (mappath, local_config_mapdir, sizeof (mappath));
  g_strlcat (mappath, "map_koord.txt", sizeof (mappath));
  st = fopen (mappath, "r");
  if (st == NULL)
    {
      mkdir (local_config_homedir, 0777);
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
	  fprintf (stderr, "loadmapconfig(%d)\r", nrmaps);
	}


      if (e == 4)
	{
	  g_strdelimit (s1, ",", '.');
	  g_strdelimit (s2, ",", '.');
	  g_strdelimit (s3, ",", '.');

	  g_strlcpy ((maps + i)->filename, filename, 200);
	  (maps + i)->map_dir = add_map_dir (filename);
	  coordinate_string2gdouble (s1, &((maps + i)->lat));
	  coordinate_string2gdouble (s2, &((maps + i)->lon));
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
	d = calcdist (target_lon, target_lat);

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
	      g_strlcpy (mappath, local_config_homedir, sizeof (mappath));
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
		  g_strlcpy (mappath, local_config_homedir, sizeof (mappath));
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
	create_nasa_mapfile (current_lat, current_lon, TRUE, nasaname);
      if (havenasa > 0)
	{
	  i = nrmaps;
	  nrmaps++;
	  maps = g_renew (mapsstruct, maps, (nrmaps + 2));
	  havenasa =
	    create_nasa_mapfile (current_lat, current_lon, FALSE, nasaname);
	  (maps + i)->lat = current_lat;
	  (maps + i)->lon = current_lon;
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
	  zero_lon = (maps + bestmap)->lon;
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
	  zero_lon = 0;
	  zero_lat = 0;
	  mapscale = 88067900.43;
	  xoff = yoff = 0;
	  loadmap (mapfilename);
	}
    }

}

/* *****************************************************************************
 * We load the map 
 * return TRUE on success
 */
int
loadmap (char *filename)
{
  gchar mappath[2048];
  GdkPixbuf *limage;
  guchar *lpixels, *pixels;
  int i, j, k;
  static int print_loadmap_error = FALSE;

  if (mydebug > 10)
    fprintf (stderr, "loadmap(%s)\n", filename);

  if (maploaded)
    gdk_pixbuf_unref (image);


  limage = gdk_pixbuf_new_from_file (filename, NULL);
  if (limage == NULL)
    {
      g_snprintf (mappath, sizeof (mappath), "data/maps/%s", filename);
      limage = gdk_pixbuf_new_from_file (mappath, NULL);
    }
  if (limage == NULL)
    {
      g_snprintf (mappath, sizeof (mappath), "%s%s", local_config_mapdir, filename);
      limage = gdk_pixbuf_new_from_file (mappath, NULL);
    }
  if (limage == NULL)
    {
      g_snprintf (mappath, sizeof (mappath), "%s/gpsdrive/maps/%s", DATADIR,
		  filename);
      limage = gdk_pixbuf_new_from_file (mappath, NULL);
    }
  if (limage == NULL)
    {
      g_snprintf (mappath, sizeof (mappath), "/usr/share/gpsdrive/maps/%s",  filename);
      limage = gdk_pixbuf_new_from_file (mappath, NULL);
    }

  map_proj = map_projection (filename);

  if (limage == NULL)
    havedefaultmap = FALSE;

  if (limage == NULL) {
  	if (!print_loadmap_error) {
      GString *error;
      error = g_string_new (NULL);
      g_string_sprintf (error, "%s\n%s\n",
			_(" Mapfile could not be loaded:"), filename);
      error_popup ((gpointer *) error->str);
      g_string_free (error, TRUE);
      maploaded = FALSE;
      print_loadmap_error = TRUE;
  	}
    return FALSE;
  } else {
  	print_loadmap_error = FALSE;
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

  return TRUE;
}


/* ******************************************************************
 * test if any of the "Display_backgroud_map"-Checkboxes are on
 */
int
display_background_map ()
{
  gint i;
  gint show__background_map = FALSE;

  for (i = 0; i < max_display_map; i++)
    {
      if (display_map[i].to_be_displayed)
	show__background_map = TRUE;
    }
  return show__background_map;
}


/* ******************************************************************
 * test if we need to load another map 
 */
void
test_and_load_newmap ()
{
  long long best = 1000000000LL;
  gdouble posx = 0, posy =0; 
  long long bestmap = 9999999999LL;
  gdouble pixelfactloc;
  gdouble bestscale = 1000000000.0;
  gdouble fact;
  gint i, ncount = 0;

  //    gchar str[100], buf[200];
  gdouble dif;
  static int nasaisvalid = FALSE;

  if (importactive)
    return;

  if (posmode)
    {
  		trip_lat = current_lon = posmode_lon;
  		trip_lon = current_lat = posmode_lat;
  	} else
  	route_next_target ();


  // Test if we want Background image as Map
  if (!display_background_map ())
    {
      mapscale = (glong) scalewanted;
      pixelfact = mapscale / PIXELFACT;
      zero_lat = current_lat;
      zero_lon = current_lon;
      xoff = yoff = 0;
      map_proj = proj_map;

      // extra variable; so we can later make it configurable
      gchar bg_mapfilename[2048];
      g_strlcpy (bg_mapfilename, "map_LightYellow.png", sizeof (bg_mapfilename));

      g_strlcpy (oldfilename, mapfilename, sizeof (oldfilename));
      g_strlcpy (mapfilename, bg_mapfilename, sizeof (mapfilename));
      loadmap (mapfilename);
      return;
    }


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
      if (!display_map[(maps + i)->map_dir].to_be_displayed)
	{
	  continue;
	}

      enum map_projections proj = map_projection ((maps + i)->filename);

      /*  Longitude */
      if (proj_map == proj)
	posx = (lat2radius ((maps + i)->lat) * M_PI / 180)
	    * cos (Deg2Rad( (maps + i)->lat))
	    * (current_lon - (maps + i)->lon);
      else if (proj_top == proj)
	posx = (lat2radius (0) * M_PI / 180) * (current_lon - (maps + i)->lon);
      else if (proj_googlesat == proj)
	posx = (lat2radius (0) * M_PI / 180) * (current_lon - (maps + i)->lon);
      else 
	  printf("Error: unknown Projection\n");

      /*  latitude */
      if (proj_map == proj)
	{
	  posy = (lat2radius ((maps + i)->lat) * M_PI / 180)
	    * (current_lat - (maps + i)->lat);
	  dif = lat2radius ((maps + i)->lat)
	    * (1 - (cos (Deg2Rad((current_lon - (maps + i)->lon)) )));
	  posy = posy + dif / 2.0;
	}
      else if (proj_top == proj)
	  {
	      posy = (lat2radius (0) * M_PI / 180) * (current_lat - (maps + i)->lat);
	  }
      else if (proj_googlesat == proj)
	  {
	      posy = 1.5* (lat2radius (0) * M_PI / 180)  * (current_lat - (maps + i)->lat);
	  }
      else 
	  printf("Error: unknown Projection\n");

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

	  if (scaleprefered_not_bestmap)
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
	  //printf("Selected map at lon %lf lat %lf\n",maps[i].lat,maps[i].lon);
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
      gchar sc[20];
      gint scale, xo, yo;
      coordinate_string2gdouble (newmaplat, &la);
      coordinate_string2gdouble (newmaplon, &lo);
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
