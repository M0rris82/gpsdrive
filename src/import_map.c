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
  Revision 1.18  2006/10/30 12:01:49  hamish
  spelling and conversion notes in comments

  Revision 1.17  2006/08/02 12:18:36  tweety
  forgot one sed for homedir and mapdir

  Revision 1.16  2006/08/02 07:48:24  tweety
  rename variable mapdir --> local_config_mapdir

  Revision 1.15  2006/04/03 23:43:45  tweety
  rename adj --> scaler_adj
  rearrange code for some of the _cb
   streets_draw_cb
   poi_draw_cb
  move map_dir_struct definition to src/gpsdrive.h
  remove some of the history parts in the Files
  save and read settings for display_map like "display_map_<name> = 1"
  increase limit for displayed streets
  change color of de.Strassen.Allgemein to x555555
  OSM.pm make non way segments to Strassen.Allgemein
  WDB check if yountryname is valid

  Revision 1.14  2006/02/17 20:54:34  tweety
  http://bugzilla.gpsdrive.cc/show_bug.cgi?id=73
  Downloading maps doesn't allow Longitude select by mouse

  Revision 1.13  2006/02/05 16:38:06  tweety
  reading floats with scanf looks at the locale LANG=
  so if you have a locale de_DE set reading way.txt results in clearing the
  digits after the '.'
  For now I set the LC_NUMERIC always to en_US, since there we have . defined for numbers

  Revision 1.12  2006/01/03 14:24:10  tweety
  eliminate compiler Warnings
  try to change all occurences of longi -->lon, lati-->lat, ...i
  use  drawicon(posxdest,posydest,"w-lan.open") instead of using a seperate variable
  rename drawgrid --> do_draw_grid
  give the display frames usefull names frame_lat, ...
  change handling of WP-types to lowercase
  change order for directories reading icons
  always read inconfile

  Revision 1.11  2006/01/01 20:11:42  tweety
  add option -P for Posmode on start

  Revision 1.10  2005/11/05 18:30:40  tweety
  fix sigseg in import_map Code
  VS: ----------------------------------------------------------------------

  Revision 1.9  2005/10/19 07:22:21  tweety
  Its now possible to choose units for displaying coordinates also in
  Deg.decimal, "Deg Min Sec" and "Deg Min.dec"
  Author: Oddgeir Kvien <oddgeir@oddgeirkvien.com>

  Revision 1.8  2005/04/20 23:33:49  tweety
  reformatted source code with anjuta
  So now we have new indentations

  Revision 1.7  2005/04/13 19:58:31  tweety
  renew indentation to 4 spaces + tabstop=8

  Revision 1.6  2005/04/10 21:50:50  tweety
  reformatting c-sources
	
  Revision 1.5  2005/04/02 12:10:12  tweety
  2005.03.30 by Oddgeir Kvien <oddgeir@oddgeirkvien.com>
  Canges made to import a map with one point and enter the scale


  2005.03.30 by Oddgeir Kvien <oddgeir@oddgeirkvien.com>
  Canges made to import a map with one point and enter the scale


  Revision 1.4  2005/03/28 18:05:42  tweety
  Von: Darazs Attila <zumi@freestart.hu>
  added zoom correction for map import Function

  Revision 1.3  2005/03/28 17:59:38  tweety
  corrected an Error in position calculation by Darazs Attila <zumi@freestart.hu>

  Revision 1.2  2005/03/27 21:51:14  tweety
  make x/y Fields editable, to improve map import

  Revision 1.1  2005/03/27 21:25:46  tweety
  separating map_import from gpsdrive.c

*/
/*
 * poi_ support module: display
 */


#include "config.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "gpsdrive.h"
#include "import_map.h"
#include "gettext.h"
#include "icons.h"
#include "gui.h"
#include "gpsdrive_config.h"
#include "poi_gui.h"
#include "poi.h"

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
extern gint debug, mydebug;
extern GtkWidget *dl_text_lat, *dl_text_lon, *wptext1, *wptext2;
GtkWidget *dltext4,*dltext3;
extern gdouble olddist;
extern GTimer *disttimer;
extern gint downloadwindowactive;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff;
extern GtkWidget *status;
extern GtkWidget *mylist;
extern mapsstruct *maps;
extern gint iszoomed;
extern gint nrmaps;
extern int havenasa;
extern gchar oldfilename[2048];
extern GtkWidget *posbt;
extern coordinate_struct coords;
extern currentstatus_struct current;
extern poi_struct poi_buf;

typedef struct
{
  gdouble lon;
  gdouble lat;
  gint x;
  gint y;
}
impstruct;
impstruct imports[3];
extern gdouble earthr;
#define R earthr
GtkWidget *dltext5, *dltext6, *dltext7, *scale_input;
gchar importfilename[1024];


/* *****************************************************************************
 */
gint
close_import_window_cb (GtkWidget *widget, guint datum)
{
	current.importactive = FALSE;
	gtk_widget_destroy (widget);
	g_strlcpy (oldfilename, "XXXXXXXXXXXXXXXXXX", sizeof (oldfilename));
	return FALSE;
}

/* *****************************************************************************
   set reference point for map calibration */
gint
select_refpoint_poi_cb (GtkWidget *widget, guint datum)
{
	gchar t_buf[15];

	if (mydebug > 20)
		g_print ("Setting reference point to %s: %.6f / %.6f\n",
			poi_buf.name, poi_buf.lat, poi_buf.lon);

	gtk_entry_set_text (GTK_ENTRY (dltext4), poi_buf.name);

	g_snprintf (t_buf, sizeof (t_buf), "%.6f", poi_buf.lat);
	gtk_entry_set_text (GTK_ENTRY (dl_text_lat), t_buf);

	g_snprintf (t_buf, sizeof (t_buf), "%.6f", poi_buf.lon);
	gtk_entry_set_text (GTK_ENTRY (dl_text_lon), t_buf);

	gtk_widget_hide_all (widget);

	return TRUE;
}

/* *****************************************************************************
 */
gint
nimmfile (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *buf = NULL;

  buf = gtk_file_selection_get_filename (datum);
  gtk_entry_set_text (GTK_ENTRY (dltext7), g_basename (buf));
  g_strlcpy (importfilename, g_basename (buf), sizeof (importfilename));

  gtk_widget_destroy (datum);
  loadmap ((char *) g_basename (buf));
  return (TRUE);
}

/* *****************************************************************************
 */
gint
importfb_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *fdialog;
  gchar buf[1000];
  fdialog = gtk_file_selection_new (_("Select a map file"));
  gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);

  gtk_signal_connect (GTK_OBJECT
		      (GTK_FILE_SELECTION (fdialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (nimmfile),
		      GTK_OBJECT (fdialog));
  gtk_signal_connect_object (GTK_OBJECT
			     (GTK_FILE_SELECTION (fdialog)->
			      cancel_button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (fdialog));


  g_strlcpy (buf, local_config.dir_home, sizeof (buf));

  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), buf);
  gtk_widget_show (fdialog);
  xoff = 0;
  yoff = 0;
  current.zoom = 1;
  iszoomed = FALSE;

  return TRUE;
}


/* *****************************************************************************
 */
gint
importshift_cb (GtkWidget * widget, guint datum)
{
  switch (datum)
    {
    case 1:
      yoff -= MAP_Y_2;
      break;
    case 4:
      yoff += MAP_Y_2;
      break;
    case 2:
      xoff -= MAP_X_2;
      break;
    case 3:
      xoff += MAP_X_2;
      break;
    }
  iszoomed = FALSE;
  expose_cb (NULL, 0);
  expose_mini_cb (NULL, 0);

  return TRUE;
}


/* *****************************************************************************
 */
gint
import1_cb (GtkWidget * widget, guint datum)
{
  GtkWidget *mainbox, *window;
  GtkWidget *knopf2, *knopf, *knopf3, *knopf4, *knopf6,
    *knopf_scale_finish, *scale_txt;
  GtkWidget *table, *knopf9, *knopf10, *knopf11, *s1, *s2, *s3, *s4;
  GtkWidget *s5, *s6;
  gchar buff[1300];
  GtkWidget *text;
  GtkWidget *hbox;
  gchar *thetext1 = _("How to calibrate your own maps? "
		      "First, the map file\nmust be copied into the");
  gchar *thetext1a = _("\ndirectory as .gif, .jpg or .png file "
		       "and must have\nthe size 1280x1024. The file names must be\n"
		       "map_* for street maps or top_* for topographical maps!\n"
		       "Load the file, select coordinates "
		       "from waypoint list or\ntype them in. "
		       "Then click on the accept button.");
  gchar *thetext2 =
    _("Now do the same for your second point and click on the\n"
      "finish button. The map can be used now.");


  window = gtk_dialog_new ();

  if (datum == 1)
    gtk_window_set_title (GTK_WINDOW (window), _("Import Assistant. Step 1"));
  else
    gtk_window_set_title (GTK_WINDOW (window), _("Import Assistant. Step 2"));

  gtk_container_set_border_width (GTK_CONTAINER (window), 5);
  mainbox = gtk_vbox_new (TRUE, 2);

  if (datum == 1)
    {
      knopf = gtk_button_new_with_label (_("Accept first point"));
      gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
				 GTK_SIGNAL_FUNC (import2_cb),
				 GTK_OBJECT (window));

      knopf_scale_finish =
	gtk_button_new_with_label (_("Accept Scale and Finish"));
      gtk_signal_connect_object (GTK_OBJECT (knopf_scale_finish), "clicked",
				 GTK_SIGNAL_FUNC (import_scale_cb),
				 GTK_OBJECT (window));
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area),
			  knopf_scale_finish, TRUE, TRUE, 2);
    }
  else
    {
      knopf = gtk_button_new_with_label (_("Finish"));
      gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
				 GTK_SIGNAL_FUNC (import3_cb),
				 GTK_OBJECT (window));
    }

  knopf2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  g_signal_connect_swapped (knopf2, "clicked", G_CALLBACK (close_import_window_cb), window);
  g_signal_connect (window, "delete_event", G_CALLBACK (close_import_window_cb), NULL);

  s1 = gtk_button_new_with_label (_("Go up"));
  gtk_signal_connect (GTK_OBJECT (s1), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 1);
  s2 = gtk_button_new_with_label (_("Go left"));
  gtk_signal_connect (GTK_OBJECT (s2), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 2);
  s3 = gtk_button_new_with_label (_("Go right"));
  gtk_signal_connect (GTK_OBJECT (s3), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 3);
  s4 = gtk_button_new_with_label (_("Go down"));
  gtk_signal_connect (GTK_OBJECT (s4), "clicked",
		      GTK_SIGNAL_FUNC (importshift_cb), (gpointer) 4);
  s5 = gtk_button_new_with_label (_("Zoom in"));
  gtk_signal_connect (GTK_OBJECT (s5), "clicked", GTK_SIGNAL_FUNC (zoom_cb),
		      (gpointer) 1);
  s6 = gtk_button_new_with_label (_("Zoom out"));
  gtk_signal_connect (GTK_OBJECT (s6), "clicked", GTK_SIGNAL_FUNC (zoom_cb),
		      (gpointer) 2);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area), knopf, TRUE,
		      TRUE, 2);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->action_area), knopf2,
		      TRUE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS (knopf, GTK_CAN_DEFAULT);
  GTK_WIDGET_SET_FLAGS (knopf2, GTK_CAN_DEFAULT);
  table = gtk_table_new (7, 4, TRUE);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), table, TRUE, TRUE,
		      2);
  knopf3 = gtk_label_new (_("Latitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf3, 0, 1, 0, 1);
  knopf4 = gtk_label_new (_("Longitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf4, 0, 1, 1, 2);
  knopf9 = gtk_label_new (_("Screen X"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf9, 2, 3, 0, 1);
  knopf10 = gtk_label_new (_("Screen Y"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf10, 2, 3, 1, 2);


  if (datum == 1)
    {
      scale_txt = gtk_label_new (_("Scale"));
      gtk_table_attach_defaults (GTK_TABLE (table), scale_txt, 0, 1, 2, 3);
      scale_input = gtk_entry_new ();
      gtk_table_attach_defaults (GTK_TABLE (table), scale_input, 1, 2, 2, 3);
    }

  knopf6 = gtk_button_new_with_label (_("Browse POIs"));
  g_signal_connect (G_OBJECT (knopf6), "clicked",
		      G_CALLBACK (show_poi_lookup_cb), GINT_TO_POINTER (1));


  gtk_table_attach_defaults (GTK_TABLE (table), knopf6, 0, 1, 3, 4);
  dl_text_lat = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dl_text_lat, 1, 2, 0, 1);
  coordinate2gchar (buff, sizeof (buff), coords.current_lat, TRUE,
  	local_config.coordmode);
  gtk_entry_set_text (GTK_ENTRY (dl_text_lat), buff);
  dl_text_lon = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dl_text_lon, 1, 2, 1, 2);
  coordinate2gchar (buff, sizeof (buff), coords.current_lon, FALSE,
  	local_config.coordmode);
  gtk_entry_set_text (GTK_ENTRY (dl_text_lon), buff);


  dltext5 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext5, 3, 4, 0, 1);

  dltext6 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext6, 3, 4, 1, 2);

  dltext4 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext4, 1, 2, 3, 4);
  dltext7 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext7, 3, 4, 3, 4);

  if (datum == 1)
    {
      knopf11 = gtk_button_new_with_label (_("Browse filename"));
      gtk_signal_connect_object (GTK_OBJECT (knopf11), "clicked",
				 GTK_SIGNAL_FUNC (importfb_cb), 0);
      gtk_table_attach_defaults (GTK_TABLE (table), knopf11, 2, 3, 3, 4);
    }
  else
    gtk_entry_set_text (GTK_ENTRY (dltext7), importfilename);


  gtk_entry_set_editable (GTK_ENTRY (dltext7), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (dltext4), FALSE);
  /*
   * gtk_entry_set_editable (GTK_ENTRY (dltext5), FALSE);
   * gtk_entry_set_editable (GTK_ENTRY (dltext6), FALSE);
   */

  text = gtk_label_new ("");


  if (datum == 1)
    g_snprintf (buff, sizeof (buff),
		"<span font_family=\"Arial\" size=\"10000\">%s <span color=\"red\"> %s</span> %s</span>",
		thetext1, local_config.dir_maps, thetext1a);
  else
    g_snprintf (buff, sizeof (buff),
		"<span font_family=\"Arial\" size=\"10000\">%s</span>",
		thetext2);

  gtk_label_set_text (GTK_LABEL (text), buff);
  gtk_label_set_use_markup (GTK_LABEL (text), TRUE);

  hbox = gtk_hbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox), text, TRUE, TRUE, 0);
  /*   gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0); */

  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 4, 4, 7);
  gtk_table_attach_defaults (GTK_TABLE (table), s1, 0, 1, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), s5, 1, 2, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), s2, 0, 1, 5, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), s3, 1, 2, 5, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), s4, 0, 1, 6, 7);
  gtk_table_attach_defaults (GTK_TABLE (table), s6, 1, 2, 6, 7);

  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);
  /*    gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT); */
  /*    gtk_label_set_justify (GTK_LABEL (knopf3), GTK_JUSTIFY_RIGHT); */
  /*    gtk_label_set_justify (GTK_LABEL (knopf4), GTK_JUSTIFY_RIGHT); */
  /*    gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT); */
  gtk_window_set_default (GTK_WINDOW (window), knopf);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_widget_show_all (window);
  current.importactive = TRUE;

  return TRUE;
}


/* *****************************************************************************
 * Import map with one point and given scale
 */
gint
import_scale_cb (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *s = NULL;
  gdouble dx_pix, dy_pix, x, y, maxx, maxy, dx_m, dy_m, m_pr_pix, lat, lon;
  gdouble dlat, dlon, lat_pr_m, lon_pr_m, scale, latcenter, longcenter;
  maxx = 1280;
  maxy = 1024;

  s = gtk_entry_get_text (GTK_ENTRY (dl_text_lat));
  coordinate_string2gdouble (s, &lat);

  s = gtk_entry_get_text (GTK_ENTRY (dl_text_lon));
  coordinate_string2gdouble (s, &lon);

  s = gtk_entry_get_text (GTK_ENTRY (dltext5));
  x = strtol (s, NULL, 0);

  s = gtk_entry_get_text (GTK_ENTRY (dltext6));
  y = strtol (s, NULL, 0);

  s = gtk_entry_get_text (GTK_ENTRY (scale_input));
  coordinate_string2gdouble (s, &scale);

  if (debug)
    {
      g_print ("Import: scale: %g\n", scale);
      g_print ("Import: scale: lat: %g, lon: %g\n", lat, lon);
      g_print ("Import: scale: x: %g, y: %g\n", x, y);
    }

  gtk_widget_destroy (widget);

  /*  Calc coordinates  */
  // distance from selected point on map to center in pixels
  dx_pix = maxx / 2 - x;
  dy_pix = y - maxy / 2;

  // calculate meter pr pixel of map
  m_pr_pix = scale / PIXELFACT;
  // g_print ("dx_pix %g, dy_pix %g, m_pr_pix %g\n", dx_pix, dy_pix, m_pr_pix);  

  // distance from selected point on map to center in meters
  dx_m = dx_pix * m_pr_pix;
  dy_m = dy_pix * m_pr_pix;
  //   g_print ("dx_m %g, dy_m %g\n", dx_m, dy_m);  

  // length of 1 deg lat and lon in meters
  // lat_pr_m = 360.0/(2.0*M_PI*R); 
  /* This should be the correct length, but using this formulas gives me 
   * a nautical mile that are 1857.85 m which are wrong. Therefore I am 
   * hardcoding it to a nautical mile that are 1851.85 m 
   * HB: shouldn't this be 1852.0 ?!?
   */
  lat_pr_m = 1.0 / (1851.85 * 60.0);
  lon_pr_m = lat_pr_m / cos (M_PI * lat / 180.0);
  /*
   * g_print ("R %g, M_PI %g lat_pr_m %g, lon_pr_m %g, meter pr deg lat %g\n",
   *     R, M_PI, lat_pr_m, lon_pr_m, 1.0/lat_pr_m);  
   */

  // distance in deg from selected point on map to center
  dlat = dy_m * lat_pr_m;
  dlon = dx_m * lon_pr_m;
  //   g_print ("dlat %g, dlon %g\n", dlat, dlon);  

  // map mid point in deg
  latcenter = lat + dlat;
  longcenter = lon + dlon;
  //   g_print ("Import: scale: %g, latcenter: %g, loncenter: %g\n", scale, latcenter, longcenter);  

  if (strlen (importfilename) > 4)
    {
      maps = g_renew (mapsstruct, maps, (nrmaps + 2));
      g_strlcpy ((maps + nrmaps)->filename, importfilename, 200);
      (maps + nrmaps)->lat = latcenter;
      (maps + nrmaps)->lon = longcenter;
      (maps + nrmaps)->scale = scale;
      nrmaps++;
      havenasa = -1;

      savemapconfig ();
    }

  current.importactive = FALSE;
  g_strlcpy (oldfilename, "XXXAFHSGFAERGXXXXXX", sizeof (oldfilename));
  return TRUE;
}


gint
import2_cb (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *s = NULL;

  s = gtk_entry_get_text (GTK_ENTRY (dl_text_lat));
  coordinate_string2gdouble (s, &imports[0].lat);
  s = gtk_entry_get_text (GTK_ENTRY (dl_text_lon));
  coordinate_string2gdouble (s, &imports[0].lon);
  s = gtk_entry_get_text (GTK_ENTRY (dltext5));
  imports[0].x = strtol (s, NULL, 0);
  s = gtk_entry_get_text (GTK_ENTRY (dltext6));
  imports[0].y = strtol (s, NULL, 0);


  if (debug)
    {
      fprintf (stderr, "Import: lat:%g,lon:%g x:%d,y:%d\n",
	       imports[0].lat, imports[0].lon, imports[0].x, imports[0].y);
    }

  gtk_widget_destroy (widget);

  import1_cb (NULL, 2);

  return TRUE;
}

gint
import3_cb (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *s = NULL;
  gdouble tx, ty, scale, latmax, latmin, latcenter, longmax, longmin;
  gdouble longcenter;
  gdouble px, py;

  s = gtk_entry_get_text (GTK_ENTRY (dl_text_lat));
  coordinate_string2gdouble (s, &imports[1].lat);

  s = gtk_entry_get_text (GTK_ENTRY (dl_text_lon));
  coordinate_string2gdouble (s, &imports[1].lon);

  s = gtk_entry_get_text (GTK_ENTRY (dltext5));
  imports[1].x = strtol (s, NULL, 0);

  s = gtk_entry_get_text (GTK_ENTRY (dltext6));
  imports[1].y = strtol (s, NULL, 0);

  gtk_widget_destroy (widget);

  /*  Calc coordinates and scale */

  tx = (2 * R * M_PI / 360) * cos (M_PI * imports[0].lat / 180.0) *
    (imports[0].lon - imports[1].lon);
  ty = (2 * R * M_PI / 360) * (imports[0].lat - imports[1].lat);
  /*  ty is meter */
  px = abs (imports[0].x - imports[1].x);
  py = abs (imports[0].y - imports[1].y);
  if (px > py)
    scale = fabs (tx) * PIXELFACT / px;
  else
    scale = fabs (ty) * PIXELFACT / py;
  px = imports[0].x - imports[1].x;
  py = imports[0].y - imports[1].y;
  py = -py;

  latmin = imports[0].lat - (imports[0].lat - imports[1].lat) * (1024 -
								 imports
								 [0].y) / py;
  latmax = latmin + (imports[0].lat - imports[1].lat) * 1024.0 / py;
  latcenter = (latmax + latmin) / 2.0;

  longmin =
    imports[0].lon - imports[0].x * (imports[0].lon - imports[1].lon) / px;

  longmax = longmin + 1280.0 * (imports[0].lon - imports[1].lon) / px;
  longcenter = (longmax + longmin) / 2.0;

  if (debug)
    g_print ("Import: scale: %g, latmitte: %g, latmin: %g, "
	     "latmax: %g\n longmin: %g, longmax: %g, longmitte: %g\n",
	     scale, latcenter, latmin, latmax, longmin, longmax, longcenter);

  if (strlen (importfilename) > 4)
    {
      maps = g_renew (mapsstruct, maps, (nrmaps + 2));
      g_strlcpy ((maps + nrmaps)->filename, importfilename, 200);
      (maps + nrmaps)->lat = latcenter;
      (maps + nrmaps)->lon = longcenter;
      (maps + nrmaps)->scale = scale;
      nrmaps++;
      havenasa = -1;

      savemapconfig ();
    }

  current.importactive = FALSE;
  g_strlcpy (oldfilename, "XXXAFHSGFAERGXXXXXX", sizeof (oldfilename));

  return TRUE;
}

/* 
 * handle map scrolling event
 * ie zoom in / out
 */
gint
mapscroll_cb (GtkWidget *widget, GdkEventScroll *event) {
	switch(event->direction) {
		case(GDK_SCROLL_UP): /* zoom in */
			scalerbt_cb (NULL, 2);
			break;
		case(GDK_SCROLL_DOWN): /* zoom out */
			scalerbt_cb (NULL, 1);
			break;
		case(GDK_SCROLL_LEFT): /* blank */
			break;
		case(GDK_SCROLL_RIGHT): /* blank */
			break;
	}
	return TRUE;
}

gint
mapclick_cb (GtkWidget * widget, GdkEventButton * event)
{
  gint x, y;
  gdouble lon, lat;
  GdkModifierType state;
  gchar s[200];

  /*   printf("bin in mapclick\n"); */

  if (event->button)
    gdk_window_get_pointer (event->window, &x, &y, &state);
  else
    {
      x = event->x;
      y = event->y;
      state = event->state;
    }
  if (state == 0)
    return 0;
  calcxytopos (x, y, &lat, &lon, current.zoom);
  if (mydebug)
      {
	  fprintf (stderr, "Mouse click at x:%d,y:%d -->lat:%f,lon:%f  \n", x, y,lat,lon);
      }

  if (downloadwindowactive || current.importactive)
    {
      if (downloadwindowactive)
	{
	  coordinate2gchar (s, sizeof (s), lat, TRUE,
	  	local_config.coordmode);
	  gtk_entry_set_text (GTK_ENTRY (dl_text_lat), s);
	  coordinate2gchar (s, sizeof (s), lon, FALSE,
	  	local_config.coordmode);
	  gtk_entry_set_text (GTK_ENTRY (dl_text_lon), s);
	  downloadsetparm (NULL, 0);
	}
      else
	{
	  g_snprintf (s, sizeof (s), "%d",
		x / current.zoom + (640 - MAP_X_2 / current.zoom) + xoff
		/ current.zoom);
	  gtk_entry_set_text (GTK_ENTRY (dltext5), s);
	  g_snprintf (s, sizeof (s), "%d",
		y / current.zoom + (512 - MAP_Y_2 / current.zoom) + yoff
		/ current.zoom);
	  gtk_entry_set_text (GTK_ENTRY (dltext6), s);

	}

    }
  else
    {
      /*        g_print("\nstate: %x x:%d y:%d", state, x, y); */
      //vali = (GTK_ADJUSTMENT (scaler_adj)->value);
      /*  Left mouse button + shift key */
      if ((state & (GDK_BUTTON1_MASK | GDK_SHIFT_MASK)) ==
	  (GDK_BUTTON1_MASK | GDK_SHIFT_MASK))
	{
	  scalerbt_cb (NULL, 2);
	  return TRUE;
	}
      /*  Add mouse position as waypoint */
      /*  Left mouse button + control key */
      if ((state & (GDK_BUTTON1_MASK | GDK_CONTROL_MASK)) ==
	  (GDK_BUTTON1_MASK | GDK_CONTROL_MASK))
	{
	  coords.wp_lat = lat;
	  coords.wp_lon = lon;
	  addwaypoint_cb (NULL, 0);
	  return TRUE;
	}
      /*  Add current position as waypoint */
      /*  Right mouse button + control key */
      if ((state & (GDK_BUTTON3_MASK | GDK_CONTROL_MASK)) ==
	  (GDK_BUTTON3_MASK | GDK_CONTROL_MASK))
	{
	  coords.wp_lat = coords.current_lat;
	  coords.wp_lon = coords.current_lon;

	  addwaypoint_cb (NULL, 0);
	  return TRUE;
	}
      /*  Right mouse button + shift key */
      if ((state & (GDK_BUTTON3_MASK | GDK_SHIFT_MASK)) ==
	  (GDK_BUTTON3_MASK | GDK_SHIFT_MASK))
	{
	  scalerbt_cb (NULL, 1);
	  return TRUE;
	}

      /*  Left mouse button */
      if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
	{
	  if (gui_status.posmode)
	    {
	      coords.posmode_lon = lon;
	      coords.posmode_lat = lat;
	      rebuildtracklist ();
	    }
	}
      /*  Middle mouse button */
      if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
	{
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);

	  rebuildtracklist ();
	}
      /*  Right mouse button */
      if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
	{
	  /* set  as target */
	  /* only if RIGHT mouse button clicked */
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (posbt), FALSE);
	  rebuildtracklist ();
	  g_strlcpy (current.target, _("SELECTED"), sizeof (current.target));
	  coords.target_lat = lat;
	  coords.target_lon = lon;
	  g_timer_stop (disttimer);
	  g_timer_start (disttimer);
	  olddist = current.dist;
	}
    }

  /*    g_print("\nx: %d, y: %d", x, y); */
  return TRUE;
}
