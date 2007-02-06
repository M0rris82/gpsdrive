/*
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
*/

/*  Include Dateien */
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include "poi.h"
#include <sys/time.h>
#include <gpsdrive.h>
#include "gettext.h"

#include <time.h>
#include <dirent.h>
#include <arpa/inet.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#define PADDING int_padding

gchar local_config_homedir[500];
gchar local_config_mapdir[500];

extern gint statusid, mydebug, havespeechout, posmode, muteflag;

typedef struct
{
  gchar n[200];
}
namesstruct;

extern namesstruct *names;
extern GtkWidget *addwaypointwindow, *setupfn[30];
extern gchar activewpfile[200], gpsdservername[200], setpositionname[80];
extern gint shadow, createroute, routemode;
extern gint needreloadmapconfig, simfollow;
extern GtkWidget *serial_bt, *mapdirbt, *addwaypoint1, *addwaypoint2,
  *frame_speed, *frame_sats;
extern gint minsecmode, nightmode, isnight, disableisnight;
extern gint nighttimer;
extern gint newsatslevel, testgarmin, needtosave;
extern gint wpsize, satfix, usedgps, earthmate, disableserial;
extern gchar serialdev[80];
extern GtkWidget *distlabel, *speedlabel, *altilabel, *miles, *startgpsbt,
  *setup_bt;
extern gint gcount, milesflag, downloadwindowactive;
extern gint metricflag, nauticflag;
extern gint defaultserver, disableapm;
extern GtkWidget *mainwindow, *status, *pixmapwidget, *gotowindow;
extern GtkWidget *routewindow, *setupentry[50], *setupentrylabel[50];
extern gdouble current_lon, current_lat, old_lon, old_lat, groundspeed;
static gdouble hour, sunrise, sunset, suntransit;
static gdouble duskastro, dawnastro, dusknaut, dawnnaut, duskciv, dawnciv;
extern gchar utctime[20], loctime[20];
static GtkWidget *utclabel;
static gint utctimer, triptimer = 0;
extern gint cpuload;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu;
extern gint int_padding;
extern gdouble tripodometer, tripavspeed, triptime, tripmaxspeed, triptmp,
  milesconv;
extern gint tripavspeedcount;
extern gint lastnotebook;
extern GtkWidget *settingsnotebook, *slowcpubt;
GtkWidget *ge12;
static gchar *slist[] = {
  "GMT-12", "GMT-11", "GMT-10", "GMT-9", "GMT-8", "GMT-7", "GMT-6", "GMT-5",
  "GMT-4", "GMT-3", "GMT-2", "GMT-1", "GMT+0", "GMT+1", "GMT+2", "GMT+3",
  "GMT+4", "GMT+5", "GMT+6", "GMT+7", "GMT+8", "GMT+9", "GMT+10", "GMT+11",
  "GMT+12"
};
gint zone;

#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char dbtable[MAXDBNAME], dbname[MAXDBNAME];
extern char dbpoifilter[5000];
extern poi_type_struct poi_type_list[poi_type_list_max];
extern int poi_type_list_count;
extern double dbdistance;
extern int dbusedist, havefriends, etch, do_draw_grid, serialspeed, disableserial;
GtkWidget *sqlfn[100], *ipbt;
gint sqlselects[MAXPOITYPES], sqlandmode = TRUE;
static int sqldontquery = FALSE;
int sqlplace, friendsplace;
extern GdkColormap *cmap;

extern gint mod_setupcounter, storetz;
static gint iplock = FALSE;
static GtkWidget *spinner1, *spinner2, *spinner3;
long int maxfriendssecs = 86400;
extern gchar *font_text, *font_verysmalltext, *font_smalltext, *font_bigtext, *font_wplabel;
extern gchar font_s_text[100], font_s_verysmalltext[100], font_s_smalltext[100], font_s_bigtext[100], font_s_wplabel[100];
extern char friendserverip[20];
extern char friendsserverip[20], friendsname[40], friendsidstring[40],
  friendsserverfqn[255];
GtkWidget *entryavspeed, *entrymaxspeed, *entrytripodometer, *entrytriptime,
  *tripunitlabel;
extern gchar bluecolor[40], trackcolor[40], friendscolor[40];
extern GdkColor trackcolorv, orange;
int showsid = TRUE;
extern int sound_direction, sound_distance, sound_speed, sound_gps;
extern int expedia_de;

/* *****************************************************************************
 */
static void
fixHour (gdouble hourToFix)
{
  // TODO - What happens if hourToFix is smaller than -24 or greater than 24?
  if (hourToFix < 0)
    {
      hourToFix += 24;
    }

  if (hourToFix > 24)
    {
      hourToFix -= 24;
    }
}

/* *****************************************************************************
 */
static void
getTimeTextHHMMSS (gdouble timeValue, gchar * timeText, int timeTextSize)
{
  if (timeValue > 24.0)
    {
      g_strlcpy (timeText, _("n/a"), timeTextSize);
    }
  else
    {
      g_snprintf (timeText, timeTextSize, "%02d:%02d:%02d",
		  (int) timeValue,
		  (int) ((timeValue - (int) timeValue) * 60),
		  (int) ((timeValue * 60 - (int) (timeValue * 60)) * 60));
    }

  if ( mydebug > 20 )
    {
      printf ("\n timeTextHHMMSS: <%s>\n", timeText);
    }
}

/* *****************************************************************************
 */
static void
baud_cb (GtkOptionMenu * button, gint data)
{
  serialspeed = gtk_option_menu_get_history (button);
}

/* *****************************************************************************
 */
static void
change_font_callback (GtkWidget * button, gint data)
{
  GtkWidget *dialog = NULL;
  gint response;
  gchar *fn;

  if (3 == data)
    {
      return;
    }

  if (1 == data)
    {
      dialog = gtk_font_selection_dialog_new (_("Setting WP label font"));
    }

  if (2 == data)
    {
      dialog = gtk_font_selection_dialog_new (_("Setting big display font"));
    }

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (mainwindow));

  if (1 == data)
    {
      gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG
					       (dialog), font_wplabel);
    }

  if (2 == data)
    {
      gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG
					       (dialog), font_bigtext);
    }

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (GTK_RESPONSE_OK == response)
    {
      fn =
	gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG
						 (dialog));
      if ( mydebug > 20 )
	{
	  printf ("\n New wp label font: %s\n", fn);
	}

      if (1 == data)
	{
	  g_strlcpy (font_wplabel, fn, sizeof (font_wplabel));
	}

      if (2 == data)
	{
	  g_strlcpy (font_bigtext, fn, sizeof (font_bigtext));
	}

      needtosave = TRUE;
    }

  gtk_widget_destroy (dialog);
}

/* *****************************************************************************
 */
gint
soundsettings_cb (GtkWidget * widget, guint datum)
{
  switch (datum)
    {
    case 1:
      sound_direction = !sound_direction;
      break;

    case 2:
      sound_distance = !sound_distance;
      break;

    case 3:
      sound_speed = !sound_speed;
      break;

    case 4:
      sound_gps = !sound_gps;
      break;
    }

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
static void
change_color_callback (GtkWidget * button, gint data)
{
  GtkWidget *dialog = NULL;
  gint response;
  GtkColorSelection *colorsel;
  GdkColor color;

  gdk_color_parse (bluecolor, &color);
  dialog = gtk_color_selection_dialog_new (_("Setting big display color"));

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (mainwindow));

  colorsel =
    GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);

  gtk_color_selection_set_previous_color (colorsel, &color);
  gtk_color_selection_set_current_color (colorsel, &color);
  gtk_color_selection_set_has_palette (colorsel, TRUE);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (GTK_RESPONSE_OK == response)
    {
      gtk_color_selection_get_current_color (colorsel, &color);
      g_snprintf (bluecolor, sizeof (bluecolor), "#%02x%02x%02x",
		  color.red >> 8, color.green >> 8, color.blue >> 8);
      if ( mydebug > 20 )
	{
	  fprintf (stderr, "\nnew color: %s\n", bluecolor);
	}
      needtosave = TRUE;
    }

  gtk_widget_destroy (dialog);
}

/* *****************************************************************************
 */
static void
change_trackcolor_callback (GtkWidget * button, gint data)
{
  GtkWidget *dialog = NULL;
  gint response;
  GtkColorSelection *colorsel;
  GdkColor color;

  gdk_color_parse (trackcolor, &color);
  dialog = gtk_color_selection_dialog_new (_("Setting track color"));

  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (mainwindow));

  colorsel =
    GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);

  gtk_color_selection_set_previous_color (colorsel, &color);
  gtk_color_selection_set_current_color (colorsel, &color);
  gtk_color_selection_set_has_palette (colorsel, TRUE);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (GTK_RESPONSE_OK == response)
    {
      gtk_color_selection_get_current_color (colorsel, &color);
      g_snprintf (trackcolor, sizeof (trackcolor), "#%02x%02x%02x",
		  color.red >> 8, color.green >> 8, color.blue >> 8);

      if ( mydebug > 20 )
	{
	  fprintf (stderr, "\nnew color: %s\n", trackcolor);
	}

      trackcolorv = color;
      gdk_color_alloc (cmap, &trackcolorv);

      needtosave = TRUE;
    }

  gtk_widget_destroy (dialog);
}

/* *****************************************************************************
 */
static void
change_friendscolor_callback (GtkWidget * button, gint data)
{
  GtkWidget *dialog = NULL;
  gint response;
  GtkColorSelection *colorsel;
  GdkColor color;

  gdk_color_parse (friendscolor, &color);
  dialog = gtk_color_selection_dialog_new (_("Setting friends label color"));
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (mainwindow));

  colorsel =
    GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (dialog)->colorsel);

  gtk_color_selection_set_previous_color (colorsel, &color);
  gtk_color_selection_set_current_color (colorsel, &color);
  gtk_color_selection_set_has_palette (colorsel, TRUE);

  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (GTK_RESPONSE_OK == response)
    {
      gtk_color_selection_get_current_color (colorsel, &color);
      g_snprintf (friendscolor, sizeof (friendscolor), "#%02x%02x%02x",
		  color.red >> 8, color.green >> 8, color.blue >> 8);
      if ( mydebug > 20 )
	{
	  fprintf (stderr, "\nnew color: %s\n", trackcolor);
	}
      orange = color;
      gdk_color_alloc (cmap, &orange);

      needtosave = TRUE;
    }

  gtk_widget_destroy (dialog);
}

/* *****************************************************************************
 */
void
mainsetup (void)
{
  GtkTooltips *tooltips;

  GtkWidget *dgpsbt;
  GtkWidget *drawgridbt;
  GtkWidget *earthmatebt;
  GtkWidget *etchbt;
  GtkWidget *f1;
  GtkWidget *f2;
  GtkWidget *f3;
  GtkWidget *f4;
  GtkWidget *f5;
  GtkWidget *font1;
  GtkWidget *font2;
  GtkWidget *font3;
  GtkWidget *fontbox;
  GtkWidget *framefont;
  GtkWidget *framesound;
  GtkWidget *ftable;
  GtkWidget *garminbt;
  GtkWidget *gpstable;
  GtkWidget *h1;
  GtkWidget *h2;
  GtkWidget *label1;
  GtkWidget *label1a;
  GtkWidget *label2;
  GtkWidget *mainbox;
  GtkWidget *mainbox2;
  GtkWidget *menu;
  GtkWidget *menu_item;
  GtkWidget *metric;
  GtkWidget *latlon_dms;
  GtkWidget *latlon_mindec;
  GtkWidget *latlon_degdec;
  GtkWidget *misctable;
  GtkWidget *nautic;
  GtkWidget *nightModeAuto;
  GtkWidget *nightModeOff;
  GtkWidget *nightModeOn;
  GtkWidget *nighttable;
  GtkWidget *noserial_bt;
  GtkWidget *option_menu;
  GtkWidget *s1;
  GtkWidget *s2;
  GtkWidget *shadowbt;
  GtkWidget *simfollowbt;
  GtkWidget *slowcpulabel;
  GtkWidget *sounddirbt;
  GtkWidget *sounddistbt;
  GtkWidget *soundgpsbt;
  GtkWidget *soundspeedbt;
  GtkWidget *soundtable;
  GtkWidget *table2;
  GtkWidget *trackcolorbt;
  GtkWidget *trackcolorlabel;
  GtkWidget *v2;
  GtkWidget *v3;
  GtkWidget *v4;
  GtkWidget *vbox;

  DIR *d;

  char buf2[20];
  gchar path2[400];
  gchar buf[20];

  int br = 2400;
  gint dircount = 0;
  glong i;

  struct dirent *dat;
  GList *list = NULL;

  static gchar *speedlist[] = {
    "5%", "10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%"
  };

  for (i = 0; i < 10; i++)
    {
      list = g_list_append (list, speedlist[i]);
    }
  g_strlcpy (path2, local_config_homedir, sizeof (path2));
  names = g_new (namesstruct, 102);

  d = opendir (path2);
  if (NULL != d)
    {
      do
	{
	  dat = readdir (d);
	  if (NULL != dat)
	    {
	      if (0 == strncmp (dat->d_name, "way", 3))
		{
		  if (0 ==
		      strncmp ((dat->d_name + (strlen (dat->d_name) - 4)),
			       ".txt", 4))
		    {
		      g_strlcpy ((names + dircount)->n, dat->d_name, 200);
		      dircount++;
		      if (dircount >= 100)
			{
			  error_popup (_
				       ("Don't use more than\n100 waypoint(way*.txt) files!"));
			  g_free (names);
			}
		    }
		}
	    }
	}
      while (NULL != dat);
    }

  free (d);
  table2 = gtk_table_new (1 + (dircount - 1) / 2, 2, TRUE);
  setupentry[2] = ftable = gtk_frame_new (_("Waypoint files to use"));
  gtk_container_set_border_width (GTK_CONTAINER (ftable), 5 * PADDING);
  setupentrylabel[2] = gtk_label_new (_("Waypoints"));
  vbox = gtk_vbox_new (FALSE, 2 * PADDING);

  gtk_container_add (GTK_CONTAINER (ftable), vbox);
  gtk_box_pack_start (GTK_BOX (vbox), table2, FALSE, FALSE, 10 * PADDING);

  for (i = 0; i < dircount; i++)
    {
      if (0 == i)
	{
	  setupfn[i] = gtk_radio_button_new_with_label (NULL, (names + i)->n);
	}
      else
	{
	  setupfn[i] =
	    gtk_radio_button_new_with_label (gtk_radio_button_group
					     (GTK_RADIO_BUTTON (setupfn[0])),
					     (names + i)->n);
	}

      gtk_signal_connect (GTK_OBJECT (setupfn[i]), "clicked",
			  GTK_SIGNAL_FUNC (wpfileselect_cb), (gpointer) i);
      gtk_table_attach_defaults (GTK_TABLE (table2), setupfn[i], i % 2,
				 i % 2 + 1, i / 2, i / 2 + 1);

      if (!(strcmp (activewpfile, (names + i)->n)))
	{
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (setupfn[i]), TRUE);
	}
    }
  setupentrylabel[0] = gtk_label_new (_("Settings"));
  tooltips = gtk_tooltips_new ();
  setupentry[0] = mainbox = gtk_vbox_new (FALSE, 2 * PADDING);
  setupentry[1] = mainbox2 = gtk_vbox_new (FALSE, 2 * PADDING);
  setupentrylabel[1] =
    gtk_label_new (g_strdup_printf ("%s 2", _("Settings")));

  gtk_container_set_border_width (GTK_CONTAINER (mainbox), 5 * PADDING);
  gtk_container_set_border_width (GTK_CONTAINER (mainbox2), 5 * PADDING);

  /* misc area */
  f1 = gtk_frame_new (_("Misc settings"));
  gtk_box_pack_start (GTK_BOX (mainbox), f1, FALSE, TRUE, 1 * PADDING);

  shadowbt = gtk_check_button_new_with_label (_("Show Shadows"));
  if (shadow)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (shadowbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (shadowbt), "clicked",
		      GTK_SIGNAL_FUNC (shadow_cb), (gpointer) 1);

  etchbt = gtk_check_button_new_with_label (_("Etched frames"));
  if (etch)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (etchbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (etchbt), "clicked",
		      GTK_SIGNAL_FUNC (etch_cb), (gpointer) 1);

  drawgridbt = gtk_check_button_new_with_label (_("Draw grid"));
  if (do_draw_grid)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (drawgridbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (drawgridbt), "clicked",
		      GTK_SIGNAL_FUNC (drawgrid_cb), (gpointer) 1);

  simfollowbt =
    gtk_check_button_new_with_label (_("Simulation: Follow target"));
  if (simfollow)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (simfollowbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (simfollowbt), "clicked",
		      GTK_SIGNAL_FUNC (simfollow_cb), (gpointer) 1);

  slowcpulabel = gtk_label_new (_("Maximum CPU load"));

  slowcpubt = gtk_combo_new ();
  gtk_combo_set_popdown_strings (GTK_COMBO (slowcpubt), (GList *) list);
  g_snprintf (buf, sizeof (buf), "%d%%", cpuload);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (slowcpubt)->entry), buf);
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (slowcpubt)->entry), "changed",
		      GTK_SIGNAL_FUNC (slowcpu_cb), (gpointer) 0);

  trackcolorlabel = gtk_label_new (_("Track"));

  trackcolorbt = gtk_button_new_from_stock (GTK_STOCK_SELECT_COLOR);
  gtk_signal_connect (GTK_OBJECT (trackcolorbt), "clicked",
		      G_CALLBACK (change_trackcolor_callback), (gpointer) 0);

  misctable = gtk_table_new (7, 5, FALSE);
  gtk_container_add (GTK_CONTAINER (f1), misctable);
  gtk_table_set_row_spacings (GTK_TABLE (misctable), 3);
  gtk_table_set_col_spacings (GTK_TABLE (misctable), 3);

  mapdirbt = gtk_entry_new_with_max_length (490);

  gtk_widget_set_size_request (mapdirbt, 20, 26);
  gtk_entry_set_text (GTK_ENTRY (mapdirbt), local_config_mapdir);
  label2 = gtk_label_new (_("Maps directory"));

  gtk_signal_connect (GTK_OBJECT (mapdirbt), "changed",
		      GTK_SIGNAL_FUNC (mapdir_cb), (gpointer) 1);

  nightModeAuto = gtk_radio_button_new_with_label (NULL, _("Automatic"));
  gtk_signal_connect (GTK_OBJECT (nightModeAuto), "clicked",
		      GTK_SIGNAL_FUNC (night_cb), (gpointer) 2);

  nightModeOn =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (nightModeAuto)),
				     _("On"));
  gtk_signal_connect (GTK_OBJECT (nightModeOn), "clicked",
		      GTK_SIGNAL_FUNC (night_cb), (gpointer) 1);

  nightModeOff =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (nightModeAuto)),
				     _("Off"));
  gtk_signal_connect (GTK_OBJECT (nightModeOff), "clicked",
		      GTK_SIGNAL_FUNC (night_cb), (gpointer) 0);

  switch (nightmode)
    {
    case 0:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (nightModeOff), TRUE);
      break;

    case 1:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (nightModeOn), TRUE);
      break;

    case 2:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (nightModeAuto), TRUE);
      break;
    }

  gtk_table_attach_defaults (GTK_TABLE (misctable), shadowbt, 0, 2, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (misctable), simfollowbt, 0, 2, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (misctable), etchbt, 2, 4, 0, 1);

  gtk_table_attach_defaults (GTK_TABLE (misctable), drawgridbt, 2, 4, 1, 3);

  gtk_table_attach_defaults (GTK_TABLE (misctable), slowcpulabel, 0, 2, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (misctable), slowcpubt, 0, 2, 4, 5);

  gtk_table_attach_defaults (GTK_TABLE (misctable), trackcolorlabel, 2, 4, 3,
			     4);
  gtk_table_attach_defaults (GTK_TABLE (misctable), trackcolorbt, 2, 4, 4, 5);

  /* GPS settings area */
  f4 = gtk_frame_new (_("GPS settings"));

  v4 = gtk_vbox_new (FALSE, 2 * PADDING);
  gtk_container_add (GTK_CONTAINER (f4), v4);

  gpstable = gtk_table_new (4, 2, TRUE);

  //KCFX
  gtk_box_pack_end (GTK_BOX (mainbox2), f4, FALSE, TRUE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (v4), gpstable, FALSE, FALSE, 1 * PADDING);

  garminbt = gtk_check_button_new_with_label (_("Test for GARMIN"));
  if (testgarmin)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (garminbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (garminbt), "clicked",
		      GTK_SIGNAL_FUNC (testgarmin_cb), (gpointer) 1);
  gtk_table_attach_defaults (GTK_TABLE (gpstable), garminbt, 0, 1, 0, 1);

  dgpsbt = gtk_check_button_new_with_label (_("Use DGPS-IP"));
  if (usedgps)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dgpsbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (dgpsbt), "clicked",
		      GTK_SIGNAL_FUNC (usedgps_cb), (gpointer) 1);
  gtk_table_attach_defaults (GTK_TABLE (gpstable), dgpsbt, 0, 1, 1, 2);

  earthmatebt = gtk_check_button_new_with_label (_("GPS is Earthmate"));
  if (earthmate)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (earthmatebt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (earthmatebt), "clicked",
		      GTK_SIGNAL_FUNC (earthmate_cb), (gpointer) 1);
  gtk_table_attach_defaults (GTK_TABLE (gpstable), earthmatebt, 1, 2, 0, 1);

  noserial_bt = gtk_check_button_new_with_label (_("Use serial conn."));
  if (!disableserial)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (noserial_bt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (noserial_bt), "clicked",
		      GTK_SIGNAL_FUNC (noserial_cb), (gpointer) 1);
  gtk_table_attach_defaults (GTK_TABLE (gpstable), noserial_bt, 1, 2, 1, 2);

  serial_bt = gtk_entry_new_with_max_length (20);
  gtk_widget_set_size_request (serial_bt, 20, 26);

  gtk_entry_set_text (GTK_ENTRY (serial_bt), serialdev);
  gtk_entry_set_max_length (GTK_ENTRY (serial_bt), 40);
  label1 = gtk_label_new (_("Interface"));
  label1a = gtk_label_new (_("Baudrate"));

  gtk_signal_connect (GTK_OBJECT (serial_bt), "changed",
		      GTK_SIGNAL_FUNC (serialdev_cb), (gpointer) 1);

  gtk_table_attach_defaults (GTK_TABLE (gpstable), label1, 0, 1, 2, 3);
  gtk_table_attach_defaults (GTK_TABLE (gpstable), label1a, 1, 2, 2, 3);
  gtk_table_attach_defaults (GTK_TABLE (gpstable), serial_bt, 0, 1, 3, 4);

  menu = gtk_menu_new ();
  for (i = 0; i < 5; i++)
    {
      g_snprintf (buf2, sizeof (buf2), "%d", br);
      if (4 == i)
	{
	  br += br / 2;
	}
      else
	{
	  br += br;
	}

      menu_item = gtk_menu_item_new_with_label (buf2);
      gtk_widget_show (menu_item);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
    }

  option_menu = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (option_menu), menu);
  gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), serialspeed);
  gtk_signal_connect (GTK_OBJECT (option_menu), "changed",
		      GTK_SIGNAL_FUNC (baud_cb), (gpointer) 0);

  gtk_table_attach_defaults (GTK_TABLE (gpstable), option_menu, 1, 2, 3, 4);

  /*  units area */
  f2 = gtk_frame_new (_("Units"));
  v2 = gtk_table_new (4, 2, FALSE);

  gtk_box_pack_start (GTK_BOX (mainbox), f2, FALSE, TRUE, 1 * PADDING);
  gtk_container_add (GTK_CONTAINER (f2), v2);

  miles = gtk_radio_button_new_with_label (NULL, _("Miles"));
  gtk_signal_connect (GTK_OBJECT (miles), "clicked",
		      GTK_SIGNAL_FUNC (miles_cb), (gpointer) 1);

  metric =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (miles)), _("Metric"));
  gtk_signal_connect (GTK_OBJECT (metric), "clicked",
		      GTK_SIGNAL_FUNC (miles_cb), (gpointer) 2);

  nautic =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (miles)), _("Nautic"));
  gtk_signal_connect (GTK_OBJECT (nautic), "clicked",
		      GTK_SIGNAL_FUNC (miles_cb), (gpointer) 3);

  if (milesflag)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (miles), TRUE);
    }

  if (metricflag)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (metric), TRUE);
    }

  if (nauticflag)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (nautic), TRUE);
    }

  latlon_degdec = 
      gtk_radio_button_new_with_label (NULL,
				       _("Deg.decimal"));
  gtk_signal_connect (GTK_OBJECT (latlon_degdec), "clicked",
		      GTK_SIGNAL_FUNC (minsec_cb), (gpointer) LATLON_DEGDEC);

  latlon_dms = 
      gtk_radio_button_new_with_label (gtk_radio_button_group
				       (GTK_RADIO_BUTTON
					(latlon_degdec)), _("Deg Min Sec"));
  gtk_signal_connect (GTK_OBJECT (latlon_dms), "clicked",
		      GTK_SIGNAL_FUNC (minsec_cb), (gpointer) LATLON_DMS);

  latlon_mindec = 
      gtk_radio_button_new_with_label (gtk_radio_button_group
						   (GTK_RADIO_BUTTON
						    (latlon_dms)),
						   _("Deg Min.dec"));
  gtk_signal_connect (GTK_OBJECT (latlon_mindec), "clicked",
		      GTK_SIGNAL_FUNC (minsec_cb), (gpointer) LATLON_MINDEC);


  switch (minsecmode)
    {
    case LATLON_DEGDEC:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (latlon_degdec), TRUE);
      break;
    case LATLON_DMS:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (latlon_dms), TRUE);
      break;
    case LATLON_MINDEC:
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (latlon_mindec), TRUE);
      break;
    }


  gtk_table_attach_defaults (GTK_TABLE (v2), miles, 0, 1, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (v2), metric, 0, 1, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (v2), nautic, 0, 1, 2, 3);
  gtk_table_attach_defaults (GTK_TABLE (v2), latlon_degdec, 1, 2, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (v2), latlon_dms, 1, 2, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (v2), latlon_mindec, 1, 2, 2, 3);

  /*  default download server */
  f3 = gtk_frame_new (_("Default map server"));
  h1 = gtk_hbox_new (FALSE, 2 * PADDING);
  h2 = gtk_hbox_new (FALSE, 2 * PADDING);
  v3 = gtk_vbox_new (FALSE, 2 * PADDING);

  //KCFX
  gtk_box_pack_start (GTK_BOX (mainbox2), f3, FALSE, TRUE, 1 * PADDING);

  gtk_container_add (GTK_CONTAINER (f3), v3);
  gtk_box_pack_start (GTK_BOX (v3), h1, TRUE, FALSE, 2 * PADDING);

  s1 = gtk_radio_button_new_with_label (NULL, _("Expedia Germany"));
  gtk_signal_connect (GTK_OBJECT (s1), "clicked",
		      GTK_SIGNAL_FUNC (defaultserver_cb), (gpointer) 1);

  s2 =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (s1)),
				     _("Expedia USA"));
  gtk_signal_connect (GTK_OBJECT (s2), "clicked",
		      GTK_SIGNAL_FUNC (defaultserver_cb), (gpointer) 2);

  /* disable mapblast */
  if (0 == defaultserver)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (s1), TRUE);
    }
  else
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (s2), TRUE);
    }

  gtk_box_pack_start (GTK_BOX (h1), s1, TRUE, FALSE, 2 * PADDING);
  gtk_box_pack_start (GTK_BOX (h1), s2, TRUE, FALSE, 2 * PADDING);

  /* Night light mode */
  f5 = gtk_frame_new (_("Night light mode"));
  nighttable = gtk_table_new (1, 3, TRUE);
  gtk_container_add (GTK_CONTAINER (f5), nighttable);
  gtk_box_pack_start (GTK_BOX (v3), h2, TRUE, FALSE, 2 * PADDING);
  gtk_box_pack_start (GTK_BOX (h2), label2, FALSE, FALSE, 2 * PADDING);
  gtk_box_pack_start (GTK_BOX (h2), mapdirbt, TRUE, TRUE, 2 * PADDING);

  gtk_table_attach_defaults (GTK_TABLE (nighttable), nightModeAuto, 0, 1, 0,
			     1);
  gtk_table_attach_defaults (GTK_TABLE (nighttable), nightModeOn, 1, 2, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (nighttable), nightModeOff, 2, 3, 0,
			     1);

  //KCFX
  gtk_box_pack_end (GTK_BOX (mainbox2), f5, FALSE, TRUE, 1 * PADDING);

  /* Sound settings */
  framesound = gtk_frame_new (_("Speech output settings"));
  soundtable = gtk_table_new (2, 2, TRUE);
  gtk_container_add (GTK_CONTAINER (framesound), soundtable);

  /*
   * set following sounds
   * sound_direction ... say direction to target
   * sound_distance  ... say distance to target
   * sound_speed     ... say your current speed
   * sound_gps       ... say GPS status
   * 
   */
  sounddirbt = gtk_check_button_new_with_label (_("Direction"));
  if (sound_direction)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sounddirbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (sounddirbt), "clicked",
		      GTK_SIGNAL_FUNC (soundsettings_cb), (gpointer) 1);

  sounddistbt = gtk_check_button_new_with_label (_("Distance"));
  if (sound_distance)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sounddistbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (sounddistbt), "clicked",
		      GTK_SIGNAL_FUNC (soundsettings_cb), (gpointer) 2);

  soundspeedbt = gtk_check_button_new_with_label (_("Speed"));
  if (sound_speed)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (soundspeedbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (soundspeedbt), "clicked",
		      GTK_SIGNAL_FUNC (soundsettings_cb), (gpointer) 3);

  soundgpsbt = gtk_check_button_new_with_label (_("GPS Status"));
  if (sound_gps)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (soundgpsbt), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (soundgpsbt), "clicked",
		      GTK_SIGNAL_FUNC (soundsettings_cb), (gpointer) 4);

  gtk_table_attach_defaults (GTK_TABLE (soundtable), sounddirbt, 0, 1, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (soundtable), sounddistbt, 0, 1, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (soundtable), soundspeedbt, 1, 2, 0,
			     1);
  gtk_table_attach_defaults (GTK_TABLE (soundtable), soundgpsbt, 1, 2, 1, 2);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), sounddirbt,
			_
			("Switch on for speech output of the direction to the "
			 "target"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), sounddistbt,
			_
			("Switch on for speech output of the distance to the "
			 "target"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), soundspeedbt,
			_
			("Switch on for speech output of your current speed"),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), soundgpsbt,
			_("Switch on for speech output of the status of your "
			  "GPS signal"), NULL);

  /* Font settings */
  framefont = gtk_frame_new (_("Font and color settings"));
  fontbox = gtk_hbutton_box_new ();
  gtk_container_add (GTK_CONTAINER (framefont), fontbox);

  font1 = gtk_button_new_with_label (_("WP Label"));
  font2 = gtk_button_new_with_label (_("Big display"));
  font3 = gtk_button_new_with_label (_("Display color"));

  gtk_signal_connect (GTK_OBJECT (font1), "clicked",
		      G_CALLBACK (change_font_callback), (gpointer) 1);
  gtk_signal_connect (GTK_OBJECT (font2), "clicked",
		      G_CALLBACK (change_font_callback), (gpointer) 2);
  gtk_signal_connect (GTK_OBJECT (font3), "clicked",
		      G_CALLBACK (change_color_callback), (gpointer) 3);

  gtk_box_pack_start (GTK_BOX (fontbox), font1, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (fontbox), font2, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (fontbox), font3, TRUE, TRUE, 2);

  gtk_box_pack_start (GTK_BOX (mainbox), framesound, FALSE, TRUE,
		      1 * PADDING);
  gtk_box_pack_start (GTK_BOX (mainbox), framefont, FALSE, TRUE, 1 * PADDING);

  gtk_table_set_row_spacings (GTK_TABLE (table2), 5 * PADDING);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 5 * PADDING);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), miles,
			_("Switch units to statute miles"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), nautic,
			_("Switch units to nautical miles"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), metric,
			_("Switch units to metric system (Kilometers)"),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), latlon_dms,
			_("If selected display latitude and longitude in "
			  "degree, minutes and seconds notation"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), latlon_mindec,
			_("If selected display latitude and longitude in "
			  "degrees and decimal minutes notation"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), latlon_degdec,
			_("If selected display latitude and longitude in "
			  "decimal degrees notation"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), s1,
			_("Set the german expedia server(expedia.de) as "
			  "default download server. Use this if you are in "
			  "Europe"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), s2,
			_("Set Expedia as default download server"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), shadowbt,
			_("Switches shadows on map on or off"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), etchbt,
			_("Switches between different type of frame "
			  "ornaments"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), drawgridbt,
			_("This will show a grid over the map"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), GTK_COMBO (slowcpubt)->entry,
			_
			("Select the approx. maximum CPU load, use 20-30%% on "
			 "notebooks while on battery to save battery power. "
			 "This effects the refresh rate of the map screen"),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), simfollowbt,
			_("If activated, pointer moves to target in "
			  "simulation mode"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), trackcolorbt,
			_("Set here the color of the drawn track"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), mapdirbt,
			_
			("Path to your map files. In the specified directory "
			 "also the index file map_koord.txt must be "
			 "present."), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), garminbt,
			_("If selected, gpsdrive try to use GARMIN mode if "
			  "possible. Unselect if you only have a NMEA "
			  "device."), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), option_menu,
			_("Set here the baud rate of your GPS device, NMEA "
			  "devices usually have a speed of 4800 baud"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), dgpsbt,
			_("If selected, gpsdrive try to use differential GPS "
			  "over IP. You must have an internet connection and "
			  "a DGPS capable GPS receiver. Works only in NMEA "
			  "mode!"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), earthmatebt,
			_("Select this if you have a DeLorme Earthmate GPS "
			  "receiver. The StartGPSD button will provide gpsd "
			  "with the needed additional parameters"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), noserial_bt,
			_
			("Select this if you want to use of the direct serial "
			 "connection. If disabled, you can use the receiver "
			 "only through gpsd. On the other hand, the direct "
			 "serial connection needs no gpsd running and detects "
			 "the working receiver on startup"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), serial_bt,
			_("Specify the serial interface where the GPS is "
			  "connected"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), nightModeAuto,
			_
			("Switches automagically to night mode if it is dark "
			 "outside. Press 'N' key to turn off nightmode."),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), nightModeOn,
			_("Switches night mode on. Press 'N' key to turn off "
			  "nightmode."), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), nightModeOff,
			_("Switches night mode off"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), font1,
			_
			("Here you can set the font for the waypoint labels"),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), font2,
			_("Here you can set the font for the big display for "
			  "Speed and Distance"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), font3,
			_
			("Here you can set the color for the big display for "
			 "speed, distance and altitude"), NULL);

  gtk_widget_show_all (mainbox);
}

/* *****************************************************************************
 */
void
testifnight (void)
{
  daylights ();

  if ((hour > sunset) || (hour < sunrise))
    {
      isnight = TRUE;
    }
  else
    {
      isnight = FALSE;
    }

  if (mydebug > 20)
    {
      if (isnight)
	{
	  g_print ("\nIt is night\n");
	}
      else
	{
	  g_print ("\nIt is day\n");
	}
    }
}

/* *****************************************************************************
 */
void
daylights (void)
{
  gint Rank, D, TZ;
  gdouble MA, C, RR, ET, L, Dec, Ho, lat, lon;
  time_t now = time (NULL);
  struct tm *st = localtime (&now);

  lat = M_PI * current_lat / 180.0;
  lon = M_PI * current_lon / 180.0;
  Rank = st->tm_yday + 1;
  D = st->tm_mday;
  MA = 357 + 0.9856 * Rank;
  C = 1.914 * sin (M_PI * MA / 180.0) + 0.02 * sin (2 * M_PI * MA / 180.0);
  L = 280.0 + C + 0.9856 * Rank;
  RR = -2.466 * sin (2.0 * M_PI * L / 180.0) +
    0.053 * sin (4 * M_PI * L / 180.0);
  ET = (C + RR) * 4;
  Dec = asin (0.3978 * sin (M_PI * L / 180.0));
  Ho = acos (((-0.01454 - sin (Dec) * sin (lat)) / (cos (Dec) * cos (lat))));
  Ho = 180.0 * Ho / M_PI;
  Ho = Ho / 15;

  TZ = zone;

  sunrise = 12.0 - Ho + ET / 60.0 - current_lon / 15 + TZ;
  suntransit = 12 + ET / 60.0 - current_lon / 15 + TZ;
  sunset = 12.0 + Ho + ET / 60.0 - current_lon / 15 + TZ;

  fixHour (sunrise);
  fixHour (suntransit);
  fixHour (sunset);

  Ho = acos (((-0.309 - sin (Dec) * sin (lat)) / (cos (Dec) * cos (lat))));
  Ho = 180.0 * Ho / M_PI;
  Ho = Ho / 15;
  dawnastro = 12.0 - Ho + ET / 60.0 - current_lon / 15 + TZ;
  duskastro = 12.0 + Ho + ET / 60.0 - current_lon / 15 + TZ;

  Ho = acos (((-0.208 - sin (Dec) * sin (lat)) / (cos (Dec) * cos (lat))));
  Ho = 180.0 * Ho / M_PI;
  Ho = Ho / 15;
  dawnnaut = 12.0 - Ho + ET / 60.0 - current_lon / 15 + TZ;
  dusknaut = 12.0 + Ho + ET / 60.0 - current_lon / 15 + TZ;

  Ho = acos (((-0.105 - sin (Dec) * sin (lat)) / (cos (Dec) * cos (lat))));
  Ho = 180.0 * Ho / M_PI;
  Ho = Ho / 15;
  dawnciv = 12.0 - Ho + ET / 60.0 - current_lon / 15 + TZ;
  duskciv = 12.0 + Ho + ET / 60.0 - current_lon / 15 + TZ;

  fixHour (dawnastro);
  fixHour (dawnnaut);
  fixHour (dawnciv);
  fixHour (duskastro);
  fixHour (dusknaut);
  fixHour (duskciv);

  hour = st->tm_hour;
  hour = hour + st->tm_min / 60.0;

  if ( mydebug > 20 )
    {
      g_print ("\nhour: %d:%02d",
	       (int) hour, (int) ((hour - (int) hour) * 60));

      g_print ("\nsunrise: %d:%02d",
	       (int) sunrise, (int) ((sunrise - (int) sunrise) * 60));

      g_print ("\ntransit: %d:%02d",
	       (int) suntransit,
	       (int) ((suntransit - (int) suntransit) * 60));

      g_print ("\nsunset: %d:%02d",
	       (int) sunset, (int) ((sunset - (int) sunset) * 60));
    }

  /*
   * Formulas for dawn and dusk are almost the same as sunrise and sunset.
   * In the formula for Cos Ho -0.01454 must be replaced by:
   * astronomical: -0.309
   * nautical    : -0.208
   * civil       : -0.105
   */
}

/* *****************************************************************************
 */
gint
infosettz (GtkWidget * widget, guint datum)
{
  gchar *sc;

  sc = (char *) gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (ge12)->entry));
  sscanf (sc, "GMT%d", &zone);

  if ( mydebug > 20 )
    {
      g_print ("\nTimezone: %d", zone);
    }

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
storetz_cb (GtkWidget * widget, guint datum)
{
  storetz = !storetz;
  fprintf (stderr, "storetz: %d\n", storetz);

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
void
infos (void)
{
  gchar text[80];
  gint i;

  GList *list = NULL;
  GtkWidget *mainbox, *frame, *table;
  GtkWidget *l1, *l2, *l3, *l4, *l5, *l6, *l7, *l8, *l9, *l10;
  GtkWidget *e1, *e2, *e3, *e4, *e5, *e6, *e7, *e8, *e9, *e10, *e11;
  GtkTooltips *tooltips;

  for (i = 0; i < 25; i++)
    {
      list = g_list_append (list, slist[i]);
    }

  setupentry[3] = frame = gtk_frame_new (_("Geo information"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5 * PADDING);
  setupentrylabel[3] = gtk_label_new (_("Geo info"));

  testifnight ();

  mainbox = gtk_vbox_new (FALSE, 15 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame), mainbox);

  table = gtk_table_new (10, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, FALSE, 10 * PADDING);

  l1 = gtk_label_new (_("Sunrise"));
  gtk_table_attach_defaults (GTK_TABLE (table), l1, 1, 2, 0, 1);

  l2 = gtk_label_new (_("Sunset"));
  gtk_table_attach_defaults (GTK_TABLE (table), l2, 2, 3, 0, 1);

  l3 = gtk_label_new (_("Standard"));
  gtk_table_attach_defaults (GTK_TABLE (table), l3, 0, 1, 1, 2);

  l4 = gtk_label_new (_("Transit"));
  gtk_table_attach_defaults (GTK_TABLE (table), l4, 0, 1, 6, 7);

  l5 = gtk_label_new (_("GPS-Time"));
  gtk_table_attach_defaults (GTK_TABLE (table), l5, 0, 1, 7, 8);

  l6 = gtk_label_new (_("Astro."));
  gtk_table_attach_defaults (GTK_TABLE (table), l6, 0, 1, 2, 3);

  l7 = gtk_label_new (_("Naut."));
  gtk_table_attach_defaults (GTK_TABLE (table), l7, 0, 1, 3, 4);

  l8 = gtk_label_new (_("Civil"));
  gtk_table_attach_defaults (GTK_TABLE (table), l8, 0, 1, 4, 5);

  l9 = gtk_label_new (_("Timezone"));
  gtk_table_attach_defaults (GTK_TABLE (table), l9, 0, 1, 9, 10);

  l10 = gtk_check_button_new_with_label (_("Store TZ"));
  gtk_table_attach_defaults (GTK_TABLE (table), l10, 2, 3, 9, 10);

  if (storetz)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (l10), TRUE);
    }

  gtk_signal_connect (GTK_OBJECT (l10), "clicked",
		      GTK_SIGNAL_FUNC (storetz_cb), (gpointer) 0);

  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), l10,
			_
			("If selected, the timezone is stored, otherwise your "
			 "actual timezone will automatically used"), NULL);

  e1 = gtk_entry_new ();
  getTimeTextHHMMSS (sunrise, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e1), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e1, 1, 2, 1, 2);

  e2 = gtk_entry_new ();
  getTimeTextHHMMSS (sunset, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e2), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e2, 2, 3, 1, 2);

  e3 = gtk_entry_new ();
  getTimeTextHHMMSS (suntransit, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e3), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e3, 1, 2, 6, 7);

  e4 = gtk_entry_new ();
  if (isnight)
    {
      g_snprintf (text, sizeof (text), "%s", _("Night"));
    }
  else
    {
      g_snprintf (text, sizeof (text), "%s", _("Day"));
    }
  gtk_entry_set_text (GTK_ENTRY (e4), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e4, 2, 3, 7, 8);

  utclabel = e5 = gtk_entry_new ();
  g_snprintf (text, sizeof (text), "%s", loctime);
  gtk_entry_set_text (GTK_ENTRY (e5), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e5, 1, 2, 7, 8);

  e6 = gtk_entry_new ();
  getTimeTextHHMMSS (dawnastro, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e6), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e6, 1, 2, 2, 3);

  e7 = gtk_entry_new ();
  getTimeTextHHMMSS (dawnnaut, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e7), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e7, 1, 2, 3, 4);

  e8 = gtk_entry_new ();
  getTimeTextHHMMSS (dawnciv, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e8), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e8, 1, 2, 4, 5);

  e9 = gtk_entry_new ();
  getTimeTextHHMMSS (duskastro, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e9), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e9, 2, 3, 2, 3);

  e10 = gtk_entry_new ();
  getTimeTextHHMMSS (dusknaut, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e10), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e10, 2, 3, 3, 4);

  e11 = gtk_entry_new ();
  getTimeTextHHMMSS (duskciv, text, sizeof (text));
  gtk_entry_set_text (GTK_ENTRY (e11), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e11, 2, 3, 4, 5);

  ge12 = gtk_combo_new ();
  gtk_combo_set_popdown_strings (GTK_COMBO (ge12), (GList *) list);
  gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (ge12)->entry), slist[zone + 12]);
  gtk_signal_connect (GTK_OBJECT (GTK_COMBO (ge12)->entry), "changed",
		      GTK_SIGNAL_FUNC (infosettz), (gpointer) 0);
  gtk_table_attach_defaults (GTK_TABLE (table), ge12, 1, 2, 9, 10);

  gtk_entry_set_editable (GTK_ENTRY (e1), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e2), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e3), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e4), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e5), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e6), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e7), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e8), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e9), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e10), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (e11), FALSE);

  gtk_widget_set_usize (e1, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e2, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e3, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e4, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e5, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e6, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e7, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e8, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e9, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e10, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (e11, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (ge12, USIZE_X, USIZE_Y);

  gtk_table_set_row_spacings (GTK_TABLE (table), 5 * PADDING);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5 * PADDING);

  utctimer = gtk_timeout_add (300, (GtkFunction) setutc, 0);

  gtk_widget_show_all (frame);
}

/* *****************************************************************************
 */
gint
settripvalues (GtkWidget * widget, guint datum)
{
  gchar text[80], s1[80];
  gdouble f;

  /* NEVER do this kind of explicit comparison, FIX! */
  if (FALSE == GTK_IS_WIDGET (entrytripodometer))
    {
      return FALSE;
    }

  g_snprintf (text, sizeof (text), "%.3f", tripodometer * milesconv);
  gtk_entry_set_text (GTK_ENTRY (entrytripodometer), text);
  f = (time (NULL) - triptime) / 3600.0;
  g_snprintf (text, sizeof (text), "%02d:%02d:%02d",
	      (int) f,
	      (int) ((f - (int) f) * 60),
	      (int) ((f * 60 - (int) (f * 60)) * 60));
  gtk_entry_set_text (GTK_ENTRY (entrytriptime), text);

  if (tripavspeedcount > 0)
    {
      g_snprintf (text, sizeof (text), "%.1f",
		  tripavspeed * milesconv / tripavspeedcount);
    }
  else
    {
      g_snprintf (text, sizeof (text), "---");
    }
  gtk_entry_set_text (GTK_ENTRY (entryavspeed), text);

  g_snprintf (text, sizeof (text), "%.1f", tripmaxspeed * milesconv);
  gtk_entry_set_text (GTK_ENTRY (entrymaxspeed), text);

  if (milesflag)
    {
      g_snprintf (s1, sizeof (s1), "%s [%s]", _("Unit:"), _("miles"));
    }
  else if (nauticflag)
    {
      g_snprintf (s1, sizeof (s1), "%s [%s]",
		  _("Unit:"), _("nautic miles/knots"));
    }
  else
    {
      g_snprintf (s1, sizeof (s1), "%s [%s]", _("Unit:"), _("kilometers"));
    }
  gtk_label_set_text (GTK_LABEL (tripunitlabel), s1);

  return TRUE;
}

/* *****************************************************************************
 */
void
trip (void)
{
  GtkWidget *mainbox, *frame, *table, *knopf;
  GtkWidget *l1, *l2, *l3, *l4;
  GtkTooltips *tooltips;

  setupentry[4] = frame = gtk_frame_new (_("Trip information"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5 * PADDING);
  setupentrylabel[4] = gtk_label_new (_("Trip info"));

  mainbox = gtk_vbox_new (FALSE, 15 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame), mainbox);

  table = gtk_table_new (6, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, FALSE, 10 * PADDING);

  tripunitlabel = gtk_label_new ("---");
  gtk_table_attach_defaults (GTK_TABLE (table), tripunitlabel, 0, 2, 0, 1);

  l1 = gtk_label_new (_("Odometer"));
  gtk_table_attach_defaults (GTK_TABLE (table), l1, 0, 1, 1, 2);
  l2 = gtk_label_new (_("Total time"));
  gtk_table_attach_defaults (GTK_TABLE (table), l2, 0, 1, 2, 3);
  l3 = gtk_label_new (_("Av. speed"));
  gtk_table_attach_defaults (GTK_TABLE (table), l3, 0, 1, 3, 4);
  l4 = gtk_label_new (_("Max. speed"));
  gtk_table_attach_defaults (GTK_TABLE (table), l4, 0, 1, 4, 5);

  entrytripodometer = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entrytripodometer), "---");
  gtk_table_attach_defaults (GTK_TABLE (table), entrytripodometer, 1, 2, 1,
			     2);

  entrytriptime = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entrytriptime), "---");
  gtk_table_attach_defaults (GTK_TABLE (table), entrytriptime, 1, 2, 2, 3);

  entryavspeed = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entryavspeed), "---");
  gtk_table_attach_defaults (GTK_TABLE (table), entryavspeed, 1, 2, 3, 4);

  entrymaxspeed = gtk_entry_new ();
  gtk_entry_set_text (GTK_ENTRY (entrymaxspeed), "---");
  gtk_table_attach_defaults (GTK_TABLE (table), entrymaxspeed, 1, 2, 4, 5);

  knopf = gtk_button_new_with_label (_("Reset"));
  gtk_signal_connect_object (GTK_OBJECT (knopf), "clicked",
			     GTK_SIGNAL_FUNC (tripreset), NULL);

  tooltips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), knopf,
			_("Resets the trip values to zero"), NULL);
  gtk_table_attach_defaults (GTK_TABLE (table), knopf, 1, 2, 5, 6);

  gtk_entry_set_editable (GTK_ENTRY (entrytripodometer), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (entryavspeed), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (entrytriptime), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (entrymaxspeed), FALSE);

  gtk_widget_set_usize (entrytripodometer, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (entryavspeed, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (entrytriptime, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (entrymaxspeed, USIZE_X, USIZE_Y);

  gtk_table_set_row_spacings (GTK_TABLE (table), 5 * PADDING);
  gtk_table_set_col_spacings (GTK_TABLE (table), 5 * PADDING);

  gtk_widget_show_all (frame);
  triptimer = gtk_timeout_add (1000, (GtkFunction) settripvalues, 0);
}

/* *****************************************************************************
 */
gint
dbdistance_cb (GtkWidget * widget, guint datum)
{
  gchar *s;

  s = g_strstrip ((char *) gtk_entry_get_text (GTK_ENTRY (widget)));
  dbdistance = g_strtod (s, 0);
  needtosave = TRUE;
  gtk_timeout_add (2000, (GtkFunction) callsqlupdateonce_cb, 0);

  return TRUE;
}

/* *****************************************************************************
 */
gint
callsqlupdateonce_cb (GtkWidget * widget, guint datum)
{
  getsqldata ();

  return FALSE;
}

/* *****************************************************************************
 */
gint
dbusedist_cb (GtkWidget * widget, guint datum)
{
  dbusedist = !dbusedist;
  needtosave = TRUE;
  getsqldata ();

  return TRUE;
}

/* *****************************************************************************
 */
gint
showsid_cb (GtkWidget * widget, guint datum)
{
  showsid = !showsid;
  needtosave = TRUE;
  dbbuildquery_cb (NULL, 999999);
  getsqldata ();

  return TRUE;
}

/* *****************************************************************************
 */
gint
noserial_cb (GtkWidget * widget, guint datum)
{
  disableserial = !disableserial;

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
usefriends_cb (GtkWidget * widget, guint datum)
{
  if (0 == strcmp (friendsname, _("EnterYourName")))
    {
      error_popup (_("You should change your name in the first field!"));
      return TRUE;
    }
  havefriends = !havefriends;

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
friendsname_cb (GtkWidget * widget, guint datum)
{
  gchar *s;

  s = (char *) gtk_entry_get_text (GTK_ENTRY (widget));
  g_strlcpy (friendsname, s, sizeof (friendsname));

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
friendsserverfqn_cb (GtkWidget * widget, guint datum)
{
  gchar *s;

  s = (char *) gtk_entry_get_text (GTK_ENTRY (widget));
  g_strlcpy (friendsserverfqn, s, sizeof (friendsserverfqn));

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
friendslookup_cb (GtkWidget * widget, guint datum)
{
  struct hostent *hent = NULL;

  char *quad;
  struct in_addr adr;

  hent = gethostbyname (friendsserverfqn);
  if ((NULL != hent) && (AF_INET == hent->h_addrtype))
    {
      bcopy (hent->h_addr, &adr.s_addr, hent->h_length);
      quad = inet_ntoa (adr);
      g_strlcpy (friendsserverip, quad, sizeof (friendsserverip));
    }
  else
    {
      g_strlcpy (friendsserverip, "0.0.0.0", sizeof (friendsserverip));
    }

  iplock = TRUE;
  gtk_entry_set_text (GTK_ENTRY (ipbt), friendsserverip);
  iplock = FALSE;

  if ( mydebug > 20 )
    {
      printf ("\nserver ip: %s\n", friendsserverip);
    }

  return FALSE;
}

/* *****************************************************************************
 */
gint
friendsserverip_cb (GtkWidget * widget, guint datum)
{
  gchar *s;

  // NEVER do this! FIX this to be a non-explicit check.
  if (TRUE == iplock)
    {
      return TRUE;
    }

  s = (char *) gtk_entry_get_text (GTK_ENTRY (widget));
  g_strlcpy (friendsserverip, s, sizeof (friendsserverip));

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
dbbuildquery_cb (GtkWidget * widget, guint datum)
{
  gint i, sel;
  gchar s[50];
  gint flag = 0;

  if (999999 != datum)
    {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{
	  sqlselects[datum] = TRUE;
	}
      else
	{
	  sqlselects[datum] = FALSE;
	}
    }

  g_strlcpy (dbpoifilter, "AND (", sizeof (dbpoifilter));
  for (i = 0; i < poi_type_list_count; i++)
    {
      sel = sqlselects[i];
      if (sel)
	{
	  flag = TRUE;
	  if (sqlandmode)
	    {
	      g_snprintf (s, sizeof (s), "poi_type.name  LIKE '%s%%' OR  ", poi_type_list[i].name);
	      g_strlcat (dbpoifilter, s, sizeof (dbpoifilter));
	    }
	  else
	    {
	      g_snprintf (s, sizeof (s), "poi_type.name NOT LIKE '%s%%' AND ", poi_type_list[i].name);
	      g_strlcat (dbpoifilter, s, sizeof (dbpoifilter));
	    }
	}
    }
  dbpoifilter[strlen (dbpoifilter) - 4] = 0;
  g_strlcat (dbpoifilter, ")", sizeof (dbpoifilter));
  //~ if (!showsid)
    //~ {
      //~ g_strlcat (dbpoifilter, " AND name != 'no_ssid'",
		 //~ sizeof (dbpoifilter));
    //~ }

  if (!flag)
    {
      g_strlcpy (dbpoifilter, "", sizeof (dbpoifilter));
    }

  if ( mydebug > 20 )
    {
      printf ("\n%s\n", dbpoifilter);
    }

  if (!sqldontquery)
    {
      getsqldata ();
    }

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
gint
sqlselectmode_cb (GtkWidget * widget, guint datum)
{
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
      sqlandmode = TRUE;
    }
  else
    {
      sqlandmode = FALSE;
    }
  dbbuildquery_cb (NULL, 999999);

  return TRUE;
}

/* *****************************************************************************
 */
gint
grab_spinner_value (GtkSpinButton * a_spinner, gpointer user_data)
{
  long int secs;

  secs = 60 * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinner3));
  secs +=
    3600 * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinner2));
  secs +=
    24 * 3600 * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinner1));
  maxfriendssecs = secs;

  needtosave = TRUE;
  return TRUE;
}

/* *****************************************************************************
 */
void
friendssetup (void)
{
  GtkWidget *mainbox, *frame, *friendscolorbt, *hbbox;
  GtkWidget *table, *spintable, *sl1, *sl2, *sl3;
  GtkWidget *d1, *d2, *d3, *d4, *d5, *d6, *d7, *d8, *look, *label, *d9;
  GtkTooltips *tooltips;
  GtkAdjustment *spinner1_adj, *spinner2_adj, *spinner3_adj;
  long int d, h, m;

  d = maxfriendssecs / 86400;
  spinner1_adj =
    (GtkAdjustment *) gtk_adjustment_new ((float) d, 0.0, 90.0, 1.0, 5.0,
					  5.0);
  spinner1 = gtk_spin_button_new (spinner1_adj, 1.0, 0);

  h = (maxfriendssecs - d * 86400) / 3600;
  spinner2_adj =
    (GtkAdjustment *) gtk_adjustment_new ((float) h, 0.0, 23.0, 1.0, 5.0,
					  5.0);
  spinner2 = gtk_spin_button_new (spinner2_adj, 1.0, 0);

  m = (maxfriendssecs - d * 86400 - h * 3600) / 60;
  spinner3_adj =
    (GtkAdjustment *) gtk_adjustment_new ((float) m, 0.0, 59.0, 1.0, 5.0,
					  5.0);
  spinner3 = gtk_spin_button_new (spinner3_adj, 1.0, 0);

  gtk_signal_connect (GTK_OBJECT (spinner1), "changed",
		      GTK_SIGNAL_FUNC (grab_spinner_value), NULL);
  gtk_signal_connect (GTK_OBJECT (spinner2), "changed",
		      GTK_SIGNAL_FUNC (grab_spinner_value), NULL);
  gtk_signal_connect (GTK_OBJECT (spinner3), "changed",
		      GTK_SIGNAL_FUNC (grab_spinner_value), NULL);

  d9 = gtk_label_new (_("Show position newer as"));
  setupentry[friendsplace] = frame =
    gtk_frame_new (_("Friends server setup"));
  setupentrylabel[friendsplace] = gtk_label_new (_("Friends"));

  gtk_container_set_border_width (GTK_CONTAINER (frame), 5 * PADDING);

  tooltips = gtk_tooltips_new ();

  mainbox = gtk_vbox_new (FALSE, 15 * PADDING);

  table = gtk_table_new (9, 2, FALSE);
  spintable = gtk_table_new (2, 3, TRUE);

  sl1 = gtk_label_new (_("Days"));
  sl2 = gtk_label_new (_("Hours"));
  sl3 = gtk_label_new (_("Minutes"));
  gtk_container_add (GTK_CONTAINER (frame), mainbox);

  gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, TRUE, 3 * PADDING);

  d1 = gtk_label_new (_("Your name"));

  d2 = gtk_entry_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), d2,
			_
			("Set here your name which should be shown near your "
			 "vehicle. You may use spaces here!"), NULL);
  gtk_entry_set_text (GTK_ENTRY (d2), friendsname);
  gtk_widget_set_usize (d2, USIZE_X, USIZE_Y);
  gtk_signal_connect (GTK_OBJECT (d2), "changed",
		      GTK_SIGNAL_FUNC (friendsname_cb), d2);

  d3 = gtk_label_new (_("Server name"));

  d4 = gtk_entry_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), d4,
			_("Set here the full qualified host name (i.e. "
			  "www.gpsdrive.cc) of your friends server, then you "
			  "have to press the \"Lookup\" button!"), NULL);
  gtk_entry_set_text (GTK_ENTRY (d4), friendsserverfqn);
  gtk_widget_set_usize (d4, USIZE_X, USIZE_Y);
  gtk_signal_connect (GTK_OBJECT (d4), "changed",
		      GTK_SIGNAL_FUNC (friendsserverfqn_cb), d2);

  look = gtk_button_new_with_label (_("Lookup"));
  gtk_signal_connect (GTK_OBJECT (look), "clicked",
		      GTK_SIGNAL_FUNC (friendslookup_cb), d2);
  GTK_WIDGET_SET_FLAGS (look, GTK_CAN_DEFAULT);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), look,
			_
			("You have to press the \"Lookup\" button to resolve "
			 "the friends server name!"), NULL);

  friendscolorbt = gtk_button_new_from_stock (GTK_STOCK_SELECT_COLOR);
  gtk_signal_connect (GTK_OBJECT (friendscolorbt), "clicked",
		      G_CALLBACK (change_friendscolor_callback),
		      (gpointer) 0);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), friendscolorbt,
			_("Set here the color of the label displayed at "
			  "friends position"), NULL);

  hbbox = gtk_hbutton_box_new ();
  gtk_box_pack_start (GTK_BOX (hbbox), friendscolorbt, TRUE, TRUE, 3);
  gtk_box_pack_start (GTK_BOX (hbbox), look, TRUE, TRUE, 3);

  d5 = gtk_label_new (_("Friends server IP"));
  ipbt = d6 = gtk_entry_new ();

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), d6,
			_("Set here the IP adress (i.e. 127.0.0.1) if you "
			  "don't set the hostname above"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), spinner1,
			_("Set here the time limit in which the friends "
			  "position is shown. Older positions are not shown."),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), spinner2,
			_("Set here the time limit in which the friends "
			  "position is shown. Older positions are not shown."),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), spinner3,
			_("Set here the time limit in which the friends "
			  "position is shown. Older positions are not shown."),
			NULL);

  gtk_entry_set_text (GTK_ENTRY (d6), friendsserverip);
  gtk_widget_set_usize (d6, USIZE_X, USIZE_Y);
  gtk_signal_connect (GTK_OBJECT (d6), "changed",
		      GTK_SIGNAL_FUNC (friendsserverip_cb), d2);

  d7 = gtk_label_new (_("Use friends server"));
  d8 = gtk_check_button_new ();
  if (havefriends)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d8), TRUE);
    }

  gtk_signal_connect (GTK_OBJECT (d8), "clicked",
		      GTK_SIGNAL_FUNC (usefriends_cb), d8);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), d8,
			_("Enable/disable use of friends server. You must "
			  "enter a Username, don't use the default name!"),
			NULL);

  label = gtk_label_new (_("If you enable the friends server mode,\n"
			   "<span color=\"red\">everyone</span> using the "
			   "same server\ncan see your position!"));
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

  gtk_table_attach_defaults (GTK_TABLE (spintable), sl1, 0, 1, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (spintable), sl2, 1, 2, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (spintable), sl3, 2, 3, 0, 1);

  gtk_table_attach_defaults (GTK_TABLE (spintable), spinner1, 0, 1, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (spintable), spinner2, 1, 2, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (spintable), spinner3, 2, 3, 1, 2);

  gtk_table_attach_defaults (GTK_TABLE (table), d1, 0, 1, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table), d2, 1, 2, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table), d3, 0, 1, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (table), d4, 1, 2, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (table), hbbox, 0, 2, 2, 3);

  gtk_table_attach_defaults (GTK_TABLE (table), d5, 0, 1, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (table), d6, 1, 2, 3, 4);
  gtk_table_attach_defaults (GTK_TABLE (table), d7, 0, 1, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), d8, 1, 2, 4, 5);
  gtk_table_attach_defaults (GTK_TABLE (table), d9, 0, 1, 5, 6);
  gtk_table_attach_defaults (GTK_TABLE (table), spintable, 1, 2, 5, 6);

  gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 2, 6, 9);

#define XALIGN 0.1
#define YALIGN 0.5

  gtk_misc_set_alignment (GTK_MISC (d1), XALIGN, YALIGN);
  gtk_misc_set_alignment (GTK_MISC (d3), XALIGN, YALIGN);
  gtk_misc_set_alignment (GTK_MISC (d5), XALIGN, YALIGN);
  gtk_misc_set_alignment (GTK_MISC (d7), XALIGN, YALIGN);
  gtk_misc_set_alignment (GTK_MISC (d9), XALIGN, YALIGN);

  gtk_table_set_row_spacings (GTK_TABLE (table), 6 * PADDING);
  gtk_table_set_col_spacings (GTK_TABLE (table), 6 * PADDING);

  gtk_widget_show_all (frame);
}

/* *****************************************************************************
 */
void
setup_poi (void)
{
  GtkTooltips *tooltips;

  GtkWidget *d1;
  GtkWidget *d2;
  GtkWidget *d4;
  GtkWidget *d5;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *l[MAXPOITYPES];
  GtkWidget *mainbox;
  GtkWidget *scroll;
  GtkWidget *t0;
  GtkWidget *t1;
  GtkWidget *t2;
  GtkWidget *table;
  GtkWidget *table2;

  gchar temp[80];
  gchar text[50];
  gchar filtertemp[5000];
  glong i;
  
  gint poi_type_entries = 30; // only show base types

  setupentry[sqlplace] = frame = gtk_frame_new (_("POI selection criterias"));
  setupentrylabel[sqlplace] = gtk_label_new (_("POI"));

  gtk_container_set_border_width (GTK_CONTAINER (frame), 5 * PADDING);

  g_strlcpy (filtertemp, dbpoifilter, sizeof (filtertemp));

  for (i = 1; i < poi_type_entries; i++)
    {
      sqlselects[i] = 0;
    }

  tooltips = gtk_tooltips_new ();

  mainbox = gtk_vbox_new (FALSE, 15 * PADDING);
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy ((GtkScrolledWindow *) scroll,
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  table = gtk_table_new (4, 2, FALSE);
  table2 = gtk_table_new (poi_type_entries, 2, FALSE);

  gtk_container_add (GTK_CONTAINER (frame), mainbox);

  gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, TRUE, 3 * PADDING);

  d1 = gtk_label_new (_("Dist. limit[km] "));
  d2 = gtk_entry_new ();
  g_snprintf (text, sizeof (text), "%0.1f", dbdistance);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), d2,
			_("If enabled, show POIs only within this "
			  "distance"), NULL);

  gtk_entry_set_text (GTK_ENTRY (d2), text);
  gtk_widget_set_usize (d2, USIZE_X, USIZE_Y);
  gtk_signal_connect (GTK_OBJECT (d2), "changed",
		      GTK_SIGNAL_FUNC (dbdistance_cb), d2);

  d4 = gtk_check_button_new ();
  hbox = gtk_hbox_new (FALSE, 2);
  if (dbusedist)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d4), TRUE);
    }
  gtk_signal_connect (GTK_OBJECT (d4), "clicked",
		      GTK_SIGNAL_FUNC (dbusedist_cb), d4);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), d4,
			_("Enable/disable distance selection"), NULL);
  gtk_box_pack_start (GTK_BOX (hbox), d1, FALSE, FALSE, 0 * PADDING);
  gtk_box_pack_start (GTK_BOX (hbox), d4, FALSE, FALSE, 0 * PADDING);

  gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (table), d2, 1, 2, 0, 1);

  d5 = gtk_check_button_new_with_label (_("Show no_ssid "));

  if (showsid)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (d5), TRUE);
    }

  gtk_signal_connect (GTK_OBJECT (d5), "clicked",
		      GTK_SIGNAL_FUNC (showsid_cb), d4);

  gtk_table_attach_defaults (GTK_TABLE (table), d5, 0, 1, 3, 4);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), d5,
			_("If enabled, WLANs with no SSID are shown, because "
			  "this is perhaps useless, you can disable it here"),
			NULL);

  sqldontquery = TRUE;

  t0 = gtk_label_new (_("Selection mode"));

  t1 = gtk_radio_button_new_with_label (NULL, _("include"));
  t2 =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (t1)), _("exclude"));

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), t1,
			_("Show only POIs where the type field contains "
			  "one of the selected words"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), t2,
			_("Show only POIs where the type field doesn't "
			  "contain any the selected words"), NULL);

  gtk_signal_connect (GTK_OBJECT (t1), "clicked",
		      GTK_SIGNAL_FUNC (sqlselectmode_cb), (gpointer) 1);

  if (NULL != (strstr (filtertemp, "type != ")))
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (t2), TRUE);
    }

  gtk_table_attach_defaults (GTK_TABLE (table), t0, 0, 2, 1, 2);
  gtk_table_attach_defaults (GTK_TABLE (table), t1, 0, 1, 2, 3);
  gtk_table_attach_defaults (GTK_TABLE (table), t2, 1, 2, 2, 3);

  gtk_scrolled_window_add_with_viewport ((GtkScrolledWindow *) scroll,
					 table2);

  gtk_box_pack_start (GTK_BOX (mainbox), scroll, TRUE, TRUE, 3 * PADDING);

  for (i = 1; i < poi_type_entries; i++)
    {
      if (g_ascii_strcasecmp(poi_type_list[i].name,"\0") != 0)
	  {
		  l[i] = gtk_entry_new ();
		  gtk_entry_set_text (GTK_ENTRY (l[i]), poi_type_list[i].name);
		  gtk_entry_set_editable (GTK_ENTRY (l[i]), FALSE);
		  gtk_widget_set_usize (l[i], USIZE_X, USIZE_Y);
		  gtk_table_attach_defaults (GTK_TABLE (table2), l[i], 0, 1, i, i + 1);
		  
		  sqlfn[i] = gtk_check_button_new ();
		  gtk_signal_connect (GTK_OBJECT (sqlfn[i]), "clicked",
			  GTK_SIGNAL_FUNC (dbbuildquery_cb), (gpointer) i);
		  g_snprintf (temp, sizeof (temp), "= '%s'", poi_type_list[i].name);
		  
		  if (NULL != (strstr (filtertemp, temp)))
		  {
			  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sqlfn[i]), TRUE);
		  }
		  
		  gtk_table_attach_defaults (GTK_TABLE (table2), sqlfn[i], 1, 2, i,i + 1);
	  }
    }

  sqldontquery = FALSE;
  gtk_table_set_row_spacings (GTK_TABLE (table), 2 * PADDING);
  gtk_table_set_col_spacings (GTK_TABLE (table), 2 * PADDING);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 2 * PADDING);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 2 * PADDING);
  gtk_widget_show_all (frame);
}

/* *****************************************************************************
 */
gint
setutc (GtkWidget * widget, guint datum)
{
  gchar text[20];

  g_snprintf (text, sizeof (text), "%s", loctime);

  if (GTK_IS_WIDGET (utclabel))
    {
      gtk_entry_set_text (GTK_ENTRY (utclabel), text);
    }

  return TRUE;
}

/* *****************************************************************************
 */
gint
removesetutc (GtkWidget * widget, guint datum)
{
  lastnotebook =
    gtk_notebook_get_current_page (GTK_NOTEBOOK (settingsnotebook));

  gtk_timeout_remove (utctimer);

  if (0 != triptimer)
    {
      gtk_timeout_remove (triptimer);
      triptimer = 0;
    }

  gtk_widget_set_sensitive (setup_bt, TRUE);

  return TRUE;
}
