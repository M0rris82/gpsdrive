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
Revision 1.1  2005/03/27 21:25:46  tweety
separating map_import from gpsdrive.c

*/
/*
 * poi_ support module: display
 */

 
#include <math.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>

#include "gpsdrive.h"
#include "import_map.h"
#include "config.h"
#include "gettext.h"
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
extern gint debug,mydebug;
extern GtkWidget *drawing_area, *drawing_bearing, *drawing_sats, *drawing_miniimage;
extern gint pdamode;
extern gint usesql;
extern glong mapscale;
extern GtkWidget *dltext1, *dltext2, *dltext3, *dltext4, *wptext1, *wptext2;
extern gdouble zero_long, zero_lat, target_long, target_lat, dist;
extern gdouble gbreit, glang, milesconv, olddist;
extern GTimer *timer, *disttimer;
extern gint gcount, milesflag, downloadwindowactive;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle;
extern GtkWidget *mainwindow, *status, *messagestatusbar, *pixmapwidget, *gotowindow;
extern GtkWidget *messagewindow, *routewindow, *downloadbt;
extern gint SCREEN_X_2, SCREEN_Y_2;
extern GtkWidget *mylist, *myroutelist, *destframe;
extern GtkObject *adj;
extern gdouble wplat, wplon;
extern gint posmode;
extern gdouble posmode_x, posmode_y;
extern mapsstruct *maps;
extern gint simmode, zoom, iszoomed;
extern gint minsecmode, nightmode, isnight, disableisnight;
extern gchar homedir[500], mapdir[500];
extern gint nrmaps, dldiff;
extern int havenasa, nosplash, sortcolumn, sortflag;
extern gint onemousebutton;
extern gchar oldfilename[1024];
extern GtkWidget *posbt, *cover;
extern gchar targetname[40];


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
GtkWidget *dltext5, *dltext6, *dltext7;
gchar importfilename[1024];


/* *****************************************************************************
   set reference point for map calibration */
gint
setrefpoint_cb (GtkWidget * widget, guint datum)
{
  gchar b[100];
  gchar *p;
  p = b;
  gtk_clist_get_text (GTK_CLIST (mylist), datum, 1, &p);
  gtk_entry_set_text (GTK_ENTRY (dltext4), p);
  gtk_clist_get_text (GTK_CLIST (mylist), datum, 2, &p);
  gtk_entry_set_text (GTK_ENTRY (dltext1), p);

  gtk_clist_get_text (GTK_CLIST (mylist), datum, 3, &p);
  gtk_entry_set_text (GTK_ENTRY (dltext2), p);

  return TRUE;
}

/* *****************************************************************************
 */
gint
nimmfile (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *buf;

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
  gtk_window_set_transient_for (GTK_WINDOW (fdialog),
				GTK_WINDOW (mainwindow));

  gtk_signal_connect (GTK_OBJECT
		      (GTK_FILE_SELECTION (fdialog)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC (nimmfile),
		      GTK_OBJECT (fdialog));
  gtk_signal_connect_object (GTK_OBJECT
			     (GTK_FILE_SELECTION (fdialog)->
			      cancel_button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (fdialog));


  g_strlcpy (buf, homedir, sizeof (buf));

  gtk_file_selection_complete (GTK_FILE_SELECTION (fdialog), buf);
  gtk_widget_show (fdialog);
  xoff = 0;
  yoff = 0;
  zoom = 1;
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
      yoff -= SCREEN_Y_2;
      break;
    case 4:
      yoff += SCREEN_Y_2;
      break;
    case 2:
      xoff -= SCREEN_X_2;
      break;
    case 3:
      xoff += SCREEN_X_2;
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
  GtkWidget *knopf2, *knopf, *knopf3, *knopf4, *knopf6;
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
    knopf = gtk_button_new_with_label (_("Accept first point"));
  else
    knopf = gtk_button_new_with_label (_("Finish"));
  if (datum == 1)
    gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
			       GTK_SIGNAL_FUNC (import2_cb),
			       GTK_OBJECT (window));
  else
    {
      gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
				 GTK_SIGNAL_FUNC (import3_cb),
				 GTK_OBJECT (window));
    }

  knopf2 = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_signal_connect_object (GTK_OBJECT (knopf2), "clicked",
			     GTK_SIGNAL_FUNC
			     (importaway_cb), GTK_OBJECT (window));
  gtk_signal_connect_object (GTK_OBJECT (window),
			     "delete_event",
			     GTK_SIGNAL_FUNC
			     (importaway_cb), GTK_OBJECT (window));

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
  gtk_signal_connect (GTK_OBJECT (s5), "clicked",
		      GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 1);
  s6 = gtk_button_new_with_label (_("Zoom out"));
  gtk_signal_connect (GTK_OBJECT (s6), "clicked",
		      GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 2);

  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->
		       action_area), knopf, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->
		       action_area), knopf2, TRUE, TRUE, 2);
  GTK_WIDGET_SET_FLAGS (knopf, GTK_CAN_DEFAULT);
  GTK_WIDGET_SET_FLAGS (knopf2, GTK_CAN_DEFAULT);
  table = gtk_table_new (6, 4, TRUE);
  gtk_box_pack_start (GTK_BOX
		      (GTK_DIALOG (window)->vbox), table, TRUE, TRUE, 2);
  knopf3 = gtk_label_new (_("Latitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf3, 0, 1, 0, 1);
  knopf4 = gtk_label_new (_("Longitude"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf4, 0, 1, 1, 2);
  knopf9 = gtk_label_new (_("Screen X"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf9, 2, 3, 0, 1);
  knopf10 = gtk_label_new (_("Screen Y"));
  gtk_table_attach_defaults (GTK_TABLE (table), knopf10, 2, 3, 1, 2);
  knopf6 = gtk_button_new_with_label (_("Browse waypoint"));
  gtk_signal_connect (GTK_OBJECT (knopf6), "clicked",
		      GTK_SIGNAL_FUNC (sel_target_cb), (gpointer) 1);

  gtk_table_attach_defaults (GTK_TABLE (table), knopf6, 0, 1, 2, 3);
  dltext1 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext1, 1, 2, 0, 1);
  g_snprintf (buff, sizeof (buff), "%.5f", current_lat);
  if (minsecmode)
    decimaltomin (buff, 1);
  gtk_entry_set_text (GTK_ENTRY (dltext1), buff);
  dltext2 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext2, 1, 2, 1, 2);
  g_snprintf (buff, sizeof (buff), "%.5f", current_long);
  if (minsecmode)
    decimaltomin (buff, 0);
  gtk_entry_set_text (GTK_ENTRY (dltext2), buff);

  dltext5 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext5, 3, 4, 0, 1);

  dltext6 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext6, 3, 4, 1, 2);

  dltext4 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext4, 1, 2, 2, 3);
  dltext7 = gtk_entry_new ();
  gtk_table_attach_defaults (GTK_TABLE (table), dltext7, 3, 4, 2, 3);

  if (datum == 1)
    {
      knopf11 = gtk_button_new_with_label (_("Browse filename"));
      gtk_signal_connect_object (GTK_OBJECT (knopf11), "clicked",
				 GTK_SIGNAL_FUNC (importfb_cb), 0);
      gtk_table_attach_defaults (GTK_TABLE (table), knopf11, 2, 3, 2, 3);
    }
  else
    gtk_entry_set_text (GTK_ENTRY (dltext7), importfilename);


  gtk_entry_set_editable (GTK_ENTRY (dltext7), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (dltext4), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (dltext5), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (dltext6), FALSE);

  text = gtk_label_new ("");


  if (datum == 1)
    g_snprintf (buff, sizeof (buff),
		"<span font_family=\"Arial\" size=\"10000\">%s <span color=\"red\"> %s</span> %s</span>",
		thetext1, mapdir, thetext1a);
  else
    g_snprintf (buff, sizeof (buff),
		"<span font_family=\"Arial\" size=\"10000\">%s</span>",
		thetext2);

  gtk_label_set_text (GTK_LABEL (text), buff);
  gtk_label_set_use_markup (GTK_LABEL (text), TRUE);

  hbox = gtk_hbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox), text, TRUE, TRUE, 0);
/*   gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, FALSE, 0); */

  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 2, 4, 3, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), s1, 0, 1, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (table), s5, 1, 2, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (table), s2, 0, 1, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), s3, 1, 2, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), s4, 0, 1, 5, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), s6, 1, 2, 5, 6);
  gtk_table_set_row_spacings (GTK_TABLE (table), 3);
  gtk_table_set_col_spacings (GTK_TABLE (table), 3);
/*    gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT); */
/*    gtk_label_set_justify (GTK_LABEL (knopf3), GTK_JUSTIFY_RIGHT); */
/*    gtk_label_set_justify (GTK_LABEL (knopf4), GTK_JUSTIFY_RIGHT); */
/*    gtk_label_set_justify (GTK_LABEL (knopf6), GTK_JUSTIFY_RIGHT); */
  gtk_window_set_default (GTK_WINDOW (window), knopf);
  gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (mainwindow));
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_widget_show_all (window);
  importactive = TRUE;

  return TRUE;
}


gint
import2_cb (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *s;

  s = gtk_entry_get_text (GTK_ENTRY (dltext1));
  checkinput ((gchar *) s);
  imports[0].lat = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext2));
  checkinput ((gchar *) s);
  imports[0].lon = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext5));
  imports[0].x = strtol (s, NULL, 0);
  s = gtk_entry_get_text (GTK_ENTRY (dltext6));
  imports[0].y = strtol (s, NULL, 0);

  gtk_widget_destroy (widget);
  import1_cb (NULL, 2);

  return TRUE;
}

gint
import3_cb (GtkWidget * widget, gpointer datum)
{
  G_CONST_RETURN gchar *s;
  gdouble tx, ty, scale, latmax, latmin, latcenter, longmax, longmin;
  gdouble longcenter;
  gdouble px, py;

  s = gtk_entry_get_text (GTK_ENTRY (dltext1));
  checkinput ((gchar *) s);
  imports[1].lat = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext2));
  checkinput ((gchar *) s);
  imports[1].lon = g_strtod (s, NULL);
  s = gtk_entry_get_text (GTK_ENTRY (dltext5));
  imports[1].x = strtol (s, NULL, 0);
  s = gtk_entry_get_text (GTK_ENTRY (dltext6));
  imports[1].y = strtol (s, NULL, 0);
  gtk_widget_destroy (widget);

/*  Calc coordinates and scale */

  tx =
    (2 * R * M_PI / 360) * cos (M_PI * imports[0].lat / 180.0) *
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

  latmin =
    imports[0].lat - (imports[0].lat - imports[1].lat) * (1024 -
							  imports[0].y) / py;
  latmax = latmin + (imports[0].lat - imports[1].lat) * 1024.0 / py;
  latcenter = (latmax + latmin) / 2.0;

  longmin =
    imports[0].lon - imports[0].x * (imports[0].lon - imports[1].lon) / px;

  longmax = longmin + 1280.0 * (imports[0].lon - imports[1].lon) / px;
  longcenter = (longmax + longmin) / 2.0;

  if (debug)
    g_print
      ("Import: scale: %g, latmitte: %g, latmin: %g, "
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

  importactive = FALSE;
  g_strlcpy (oldfilename, "XXXAFHSGFAERGXXXXXX", sizeof (oldfilename));

  return TRUE;
}

gint
mapclick_cb (GtkWidget * widget, GdkEventButton * event)
{
  gint x, y;
  gdouble lon, lat, vali;
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
  calcxytopos (x, y, &lat, &lon, zoom);

  if (downloadwindowactive || importactive)
    {
      if (downloadwindowactive)
	{
	  g_snprintf (s, sizeof (s), "%.5f", lat);
	  if (minsecmode)
	    decimaltomin (s, 1);
	  gtk_entry_set_text (GTK_ENTRY (dltext1), s);
	  g_snprintf (s, sizeof (s), "%.5f", lon);
	  if (minsecmode)
	    decimaltomin (s, 0);
	  gtk_entry_set_text (GTK_ENTRY (dltext2), s);
	  downloadsetparm (NULL, 0);
	}
      else
	{
	  g_snprintf (s, sizeof (s), "%d", x + SCREEN_X_2 + xoff);
	  gtk_entry_set_text (GTK_ENTRY (dltext5), s);
	  g_snprintf (s, sizeof (s), "%d", y + SCREEN_Y_2 + yoff);
	  gtk_entry_set_text (GTK_ENTRY (dltext6), s);

	}
    }
  else
    {
/*        g_print("\nstate: %x x:%d y:%d", state, x, y); */
      vali = (GTK_ADJUSTMENT (adj)->value);
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
	  wplat = lat;
	  wplon = lon;
	  addwaypoint_cb (NULL, 0);
	  return TRUE;
	}
/*  Add current position as waypoint */
/*  Right mouse button + control key */
      if ((state & (GDK_BUTTON3_MASK | GDK_CONTROL_MASK)) ==
	  (GDK_BUTTON3_MASK | GDK_CONTROL_MASK))
	{
	  wplat = current_lat;
	  wplon = current_long;

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
	  if (posmode)
	    {
	      posmode_x = lon;
	      posmode_y = lat;
	      rebuildtracklist ();
	      if (onemousebutton)
		gtk_timeout_add (10000, (GtkFunction) posmodeoff_cb, 0);
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
	  g_strlcpy (targetname, _("SELECTED"), sizeof (targetname));
	  g_snprintf (s, sizeof (s), "%s: %s", _("To"), targetname);
	  gtk_frame_set_label (GTK_FRAME (destframe), s);
	  target_lat = lat;
	  target_long = lon;
	  g_timer_stop (disttimer);
	  g_timer_start (disttimer);
	  olddist = dist;
	}
    }

/*    g_print("\nx: %d, y: %d", x, y); */
  return TRUE;
}

