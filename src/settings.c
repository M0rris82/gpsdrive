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
#include "poi_gui.h"

#include <sys/time.h>
#include <gpsdrive.h>
#include "gui.h"
#include "track.h"

#include "gettext.h"

#include <time.h>
#include <dirent.h>
#include <arpa/inet.h>
#include "gpsdrive_config.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint mydebug;

typedef struct
{
  gchar n[200];
}
namesstruct;

namesstruct *names;
extern GtkWidget *addwaypointwindow;
extern gint needreloadmapconfig;
extern gint iszoomed;
extern GtkWidget *miles;
extern GtkWidget *status;
extern GtkWidget *poi_types_window;
extern GtkWidget *frame_statusfriends;
extern gdouble milesconv;

GtkWidget *ipbt;
gint sqlandmode = TRUE;
extern GdkColormap *cmap;

extern GtkTooltips *main_tooltips;
extern GtkWidget *wp_draw_bt;
extern gint storetz;
static gboolean friendsiplock = FALSE;
static gboolean friendsnamelock = FALSE;
extern gchar *font_wplabel;
extern char friendserverip[20];
extern color_struct colors;
extern currentstatus_struct current;
extern GtkTreeModel *poi_types_tree;
extern GtkTreeStore *poi_types_tree_filtered;
extern GtkWidget *frame_statusbar;
GtkWidget *menuitem_sendmsg;
extern gchar *espeak_voices[];
extern GtkWidget *mute_bt;
extern gint havefestival;
extern GtkWidget *frame_battery;
extern GtkWidget *frame_temperature;
extern GHashTable *poi_types_hash;
extern guint id_timeout_autotracksave;
extern guint id_timeout_track;

GtkWidget *settings_window = NULL;

/* ****************************************************************************
 * CALLBACKS
 */


/* ************************************************************************* */
static gint
reinitgps_cb (GtkWidget *widget)
{
	g_print ("\nReinitializing GPS connection...\n");
	initgps ();
	gtk_statusbar_push (GTK_STATUSBAR (frame_statusbar),
		current.statusbar_id, _("GPS Initialization done."));
	return TRUE;
}


/* ************************************************************************* */
static gint
setdistmode_cb (GtkWidget *widget)
{
	gint selection;
	
	selection = gtk_combo_box_get_active (GTK_COMBO_BOX (widget)); 
	
	switch (selection)
	{
		case DIST_MILES:
			milesconv = KM2MILES;
			local_config.distmode = DIST_MILES;
			break;
		case DIST_METRIC:
			milesconv = 1.0;
			local_config.distmode = DIST_METRIC;
			break;
		case DIST_NAUTIC:
			milesconv = KM2NAUTIC;
			local_config.distmode = DIST_NAUTIC;
			break;
	}
	
	current.needtosave = TRUE;
	
	if (mydebug >10)
		fprintf (stderr, "Setting distance format to %d %%.\n",
			local_config.distmode);
	
	return TRUE;
}

/* ************************************************************************* */
static gint
setaltmode_cb (GtkWidget *widget)
{
	gint selection;
	
	selection = gtk_combo_box_get_active (GTK_COMBO_BOX (widget)); 

	if (selection != -1)
		local_config.altmode = selection;
	
	if (mydebug >10)
		fprintf (stderr, "Setting altitude display format to %d.\n",
			local_config.altmode);
	
	current.needtosave = TRUE;
	
	return TRUE;
}

/* ************************************************************************* */
static gint
setcolor_cb (GtkWidget *widget, GdkColor *targetcolor)
{
	gchar *tcol;
	
	gtk_color_button_get_color
		(GTK_COLOR_BUTTON (widget), targetcolor);

	tcol = get_colorstring (&colors.track);
	g_strlcpy (local_config.color_track, tcol,
		sizeof (local_config.color_track));
	g_free (tcol);
	tcol = get_colorstring (&colors.route);
	g_strlcpy (local_config.color_route, tcol,
		sizeof (local_config.color_route));
	g_free (tcol);
	tcol = get_colorstring (&colors.friends);
	g_strlcpy (local_config.color_friends, tcol,
		sizeof (local_config.color_friends));
	g_free (tcol);
	tcol = get_colorstring (&colors.wplabel);
	g_strlcpy (local_config.color_wplabel, tcol,
		sizeof (local_config.color_wplabel));
	g_free (tcol);
	tcol = get_colorstring (&colors.dashboard);
	g_strlcpy (local_config.color_dashboard, tcol,
		sizeof (local_config.color_dashboard));
	g_free (tcol);

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setlinestyle_cb (GtkWidget *widget, gint *value)
{
	gint selection;
	
	selection = gtk_combo_box_get_active (GTK_COMBO_BOX (widget)); 

	if (selection != -1)
		*value = selection;
	
	if (mydebug >10)
		g_print ("Setting line style to %d.\n", *value);
	
	current.needtosave = TRUE;
	
	return TRUE;
}

/* ************************************************************************* */
static gint
setcoordmode_cb (GtkWidget *widget)
{
	gint selection;
	
	selection = gtk_combo_box_get_active (GTK_COMBO_BOX (widget)); 

	if (selection != -1)
	{
		local_config.coordmode = selection;
		if (mydebug >10)
			fprintf (stderr, "Setting coordinate format to %d.\n",
				selection);
	}

	current.needtosave = TRUE;
	
	return TRUE;
}

/* ************************************************************************* */
static gint
setespeakvoice_cb (GtkWidget *widget)
{
	gint i;

	i = 2 * gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

	if (i > -1)
	{
		g_strlcpy (local_config.speech_voice, espeak_voices[i+1],
			sizeof (local_config.speech_voice));

		if (mydebug > 10)
			fprintf (stderr, "Setting espeak voice to %s --> %s\n",
				espeak_voices[i], espeak_voices[i+1]);
	}

	return TRUE;
}

/* ************************************************************************* */
static gint
setfont_cb (GtkWidget *widget, gchar *font)
{
	gchar *tfont;
	
	tfont = (gchar *) gtk_font_button_get_font_name
		(GTK_FONT_BUTTON (widget));
	g_strlcpy (font, tfont, 100);

	if (mydebug > 10 )
		fprintf (stderr, "setfont_cb: Setting font to: %s\n", font);

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setfriendmaxsec_cb (GtkWidget *spin, GtkWidget *combobox)
{
	gdouble value;
	gint unit;
	
	value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin));
	unit = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));
	
	switch (unit)
	{
		case 0:	/* days */
			local_config.friends_maxsecs = value * 86400;
			break;
		case 1:	/* hours */
			local_config.friends_maxsecs = value * 3600;
			break;
		case 2:	/* minutes */
			local_config.friends_maxsecs = value * 60;
			break;
	}

	/* cap max. age to seven days */
	if (local_config.friends_maxsecs > 604800)
	{
		local_config.friends_maxsecs = 604800;
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 7);
	}

	if (mydebug > 10)
		fprintf (stderr,
			"Setting max. age for friends data to %ld seconds.\n",
			local_config.friends_maxsecs);

	return TRUE;
}

/* ************************************************************************* */
static gint
setfriendmaxsecunit_cb (GtkWidget *combobox, GtkWidget *spin)
{
	return setfriendmaxsec_cb (spin, combobox);
}

/* ************************************************************************* */
static gint
setfriendname_cb (GtkWidget *widget)
{
	gchar *name;

	if (friendsnamelock)
	{
		return TRUE;
	}
	
	name = (gchar *) gtk_entry_get_text (GTK_ENTRY (widget));
	g_strlcpy (local_config.friends_name, name,
		sizeof (local_config.friends_name));

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setfriendsrv_cb (GtkWidget *widget)
{
	gchar *srv;

	srv = (gchar *) gtk_entry_get_text (GTK_ENTRY (widget));
	g_strlcpy (local_config.friends_serverfqn, srv,
		sizeof (local_config.friends_serverfqn));

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setfriendsrvip_cb (GtkWidget *widget)
{
	gchar *srvip;

	if (friendsiplock)
	{
		return TRUE;
	}

	srvip = (gchar *) gtk_entry_get_text (GTK_ENTRY (widget));
	g_strlcpy (local_config.friends_serverip, srvip,
		sizeof (local_config.friends_serverip));

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setfriendsrvip_lookup_cb (GtkWidget *entry)
{
	struct hostent *hent = NULL;

	char *quad;
	struct in_addr adr;

	hent = gethostbyname (local_config.friends_serverfqn);
	if ((NULL != hent) && (AF_INET == hent->h_addrtype))
	{
                memcpy (&adr.s_addr, hent->h_addr, hent->h_length);
		quad = inet_ntoa (adr);
		g_strlcpy (local_config.friends_serverip, quad,
			sizeof (local_config.friends_serverip));
	}
	else
	{
		g_strlcpy (local_config.friends_serverip, "0.0.0.0",
			sizeof (local_config.friends_serverip));
	}

	friendsiplock = TRUE;
	gtk_entry_set_text (GTK_ENTRY (entry), local_config.friends_serverip);
	friendsiplock = FALSE;

	if (mydebug > 10)
	{
		fprintf (stderr, "\nSetting friends server ip to %s\n",
			local_config.friends_serverip);
	}

	return FALSE;
}

/* ************************************************************************* */
static gint
setpoisearch_cb (GtkWidget *widget, gint value)
{
	switch (value)
	{
		case 1:	/* set radius preference */
			local_config.poi_searchradius = g_strtod (
				gtk_entry_get_text (GTK_ENTRY (widget)), NULL);
			break;
		case 2: /* set results limit */
			local_config.poi_results_max =
				atoi (gtk_entry_get_text (GTK_ENTRY (widget)));
			if (local_config.poi_results_max < 1)
				local_config.poi_results_max = 1;
			break;
		default:
			return FALSE;
	}
	
	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setshowfriends_cb (GtkWidget *entry)
{
	if (strlen (local_config.friends_name) == 0)
	{
		g_strlcpy (local_config.friends_name, _("EnterYourName"),
			sizeof (local_config.friends_name));
		friendsnamelock = TRUE;
		gtk_entry_set_text (GTK_ENTRY (entry),
			local_config.friends_name);
		friendsnamelock = FALSE;
	}
	
	if (0 == strcmp (local_config.friends_name, _("EnterYourName")))
	{
		popup_warning (GTK_WINDOW (settings_window),
			_("You should change your name in the first field!"));
		return TRUE;
	}

	local_config.showfriends = !local_config.showfriends;

	if (local_config.showfriends)
	{
		gtk_widget_show_all (frame_statusfriends);
		gtk_widget_set_sensitive (menuitem_sendmsg, TRUE);
	}
	else
	{
		gtk_widget_hide_all (frame_statusfriends);
		gtk_widget_set_sensitive (menuitem_sendmsg, FALSE);
	}

	if (mydebug >10)
	{
		fprintf (stderr, "Setting friend display to %d.\n",
			local_config.showfriends);
	}

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setspeechspeed_cb (GtkWidget *range)
{
	local_config.speech_speed = gtk_range_get_value (GTK_RANGE (range));
	return TRUE;
}

/* ************************************************************************* */
static gint
setspeechpitch_cb (GtkWidget *range)
{
	local_config.speech_pitch = gtk_range_get_value (GTK_RANGE (range));
	return TRUE;
}

/* ************************************************************************* */
static gint
setspeechenable_cb (GtkWidget *widget)
{
	local_config.speech = !local_config.speech;

	if (local_config.speech)
	{
		gtk_widget_show_all (mute_bt);
		havefestival = FALSE;
		speech_out_speek (_("Speech output enabled."));
		g_timeout_add (SPEECHOUTINTERVAL, (GtkFunction) speech_out_cb, 0);
	}
	else
		gtk_widget_hide_all (mute_bt);

	if (mydebug >10)
	{
		fprintf (stderr, "Setting speech to %d.\n",
			local_config.speech);
	}

	current.needtosave = TRUE;
	
	return TRUE;
}

/* ************************************************************************* */
static gint
settogglevalue_cb (GtkWidget *widget, gint *item)
{
	*item = !*item;

	if (mydebug >10)
	{
		fprintf (stderr, "Setting config value to %d.\n",
			*item);
	}

	current.needtosave = TRUE;
	
	return TRUE;
}

/* ************************************************************************* */
static gint
settoggleapm_cb (GtkWidget *widget, gint *item)
{
	*item = !*item;

	if (mydebug >10)
	{
		fprintf (stderr, "Setting apm config value to %d.\n",
			*item);
	}

	if (local_config.showbatt)
		gtk_widget_show_all (frame_battery);
	else
		gtk_widget_hide_all (frame_battery);

	if (local_config.showtemp)
		gtk_widget_show_all (frame_temperature);
	else
		gtk_widget_hide_all (frame_temperature);

	current.needtosave = TRUE;

	return TRUE;
}

/* ************************************************************************* */
static gint
settrkautoint_cb (GtkWidget *spin, gint data)
{
	gdouble value;
	gint unit;
	
	value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin));
	local_config.track_autointerval = value * 2;

	if (id_timeout_autotracksave != 0 && g_source_remove (id_timeout_autotracksave))
		id_timeout_autotracksave = 0;

	/* cap max. interval to 750 hours (about 1 month) */
	if (local_config.track_autointerval > 1500)
	{
		local_config.track_autointerval = 1500;
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 750);
	}

	if (mydebug > 10)
		g_print ("Setting interval for automatic track saving to %.1f hours.\n",
			local_config.track_autointerval/2);

	current.needtosave = TRUE;

	return TRUE;
}

/* ************************************************************************* */
static gint
settrkint_cb (GtkWidget *spin, gint data)
{
	gdouble value;
	gint unit;
	
	value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (spin));
	local_config.track_interval = value;

	if (id_timeout_track != 0 && g_source_remove (id_timeout_track))
		id_timeout_track = 0;

	/* cap max. interval to 10 minutes */
	if (local_config.track_autointerval > 600)
	{
		local_config.track_autointerval = 600;
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin), 600);
	}

	if (mydebug > 10)
		g_print ("Setting interval between saved track points to %.1f hours.\n",
			local_config.track_interval);

	current.needtosave = TRUE;

	return TRUE;
}

/* ************************************************************************* */
static gint
settrkautoprefix_cb (GtkWidget *widget)
{
	gchar *name;

	name = (gchar *) gtk_entry_get_text (GTK_ENTRY (widget));
	g_strlcpy (local_config.track_autoprefix, name,
		sizeof (local_config.track_autoprefix));

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setmapdir_cb (GtkWidget *widget)
{
	gchar *tdir;

	tdir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));

	if (tdir && strcmp (local_config.dir_maps, tdir) != 0)
	{
		g_strlcpy (local_config.dir_maps, tdir,
			sizeof (local_config.dir_maps));
		g_strlcat (local_config.dir_maps, "/",
			sizeof (local_config.dir_maps));
		if (mydebug >3)
		{
			fprintf (stderr, "setting maps dir to: %s\n",
				local_config.dir_maps);
		}
		needreloadmapconfig = TRUE;
		current.needtosave = TRUE;
		gtk_timeout_add (2000, (GtkFunction) loadmapconfig, 0);
	}
	g_free (tdir);

	return TRUE;
}

/* ************************************************************************* */
static gint
setmaxcpuload_cb (GtkWidget *widget)
{
	local_config.maxcpuload =
		gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
	
	if (local_config.maxcpuload == 0)
		local_config.maxcpuload = 40;
		
	if (mydebug >10)
		fprintf (stderr, "Setting max. CPU-Load to %d %%.\n",
			local_config.maxcpuload);

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setnightmode_cb (GtkWidget *widget, guint value)
{
	switch (value)
	{
		case NIGHT_OFF:
			switch_nightmode (FALSE);
			break;
		case NIGHT_ON:
			switch_nightmode (TRUE);
			break;
		case NIGHT_AUTO:
			check_if_night_cb ();
			break;
		default:
			return TRUE;
	}

	local_config.nightmode = value;
	current.needtosave = TRUE;

	return TRUE;
}

/* ************************************************************************* */
static gint
setpoitheme_cb (GtkWidget *combo)
{
	gchar *theme;

	theme = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo));
	g_strlcpy (local_config.icon_theme, theme,
		sizeof (local_config.icon_theme));

	get_poitype_tree ();

	if ( mydebug > 1 )
	{
		fprintf (stderr, "POI Theme changed to: %s\n", theme);
	}

	g_free (theme);
	
	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setposmarker_cb (GtkWidget *widget)
{
	gint selection;
	
	selection = gtk_combo_box_get_active (GTK_COMBO_BOX (widget)); 

	if (selection != -1)
		local_config.posmarker = selection;
	
	if (mydebug >10)
		fprintf (stderr, "Setting posmarker style to %d.\n",
			local_config.posmarker);
	
	current.needtosave = TRUE;
	
	return TRUE;
}

/* ************************************************************************* */
static gint
setsimmode_cb (GtkWidget *widget, guint value)
{
	local_config.simmode = value;
	if (value == SIM_AUTO)
	{
		if (current.gpsfix < 2)
			current.simmode = TRUE;
		else
			current.simmode = FALSE;
	}
	else
	{
		current.simmode = value;
	}
	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
settrackdir_cb (GtkWidget *widget)
{
	gchar *tdir;

	tdir = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));

	if (tdir && strcmp (local_config.dir_tracks, tdir) != 0)
	{
		g_strlcpy (local_config.dir_tracks, tdir,
			sizeof (local_config.dir_tracks));
		g_strlcat (local_config.dir_tracks, "/",
			sizeof (local_config.dir_tracks));
		if (mydebug >3)
		{
			fprintf (stderr, "setting tracks dir to: %s\n",
				local_config.dir_tracks);
		}
		current.needtosave = TRUE;
	}
	g_free (tdir);

	return TRUE;
}

/* ************************************************************************* */
static gint
settravelmode_cb (GtkWidget *widget)
{
	gint selection;
	
	selection = gtk_combo_box_get_active (GTK_COMBO_BOX (widget)); 

	if (selection != -1)
		local_config.travelmode = selection;
	
	if (mydebug >10)
		fprintf (stderr, "Setting travelmode to %d.\n",
			local_config.travelmode);
	
	current.needtosave = TRUE;
	
	return TRUE;
}

/* ************************************************************************* */
static gint
setwpfile_cb (GtkWidget *widget)
{
	gchar *tfile;
	gchar t_tip[600];
	
	tfile = gtk_file_chooser_get_filename
		(GTK_FILE_CHOOSER (widget));
	if (tfile && strcmp (local_config.wp_file, tfile) != 0)
	{
		g_strlcpy (local_config.wp_file, tfile,
			sizeof (local_config.wp_file));
		if (mydebug >3)
			fprintf (stderr, "setting wp_file to: %s\n",
			tfile);
		g_snprintf (t_tip, sizeof (t_tip), "%s %s",
			_("Show Waypoints found in file"), tfile);
		gtk_tooltips_set_tip (GTK_TOOLTIPS (main_tooltips),
			wp_draw_bt, t_tip, NULL);
		loadwaypoints ();
	}
	g_free (tfile);

	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static gint
setwpfilequick_cb (GtkWidget *widget, guint datum)
{
	gchar *selected;
	
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
	{
		selected = g_strconcat (local_config.dir_home,
			(names + datum)->n, NULL);
		if (strcmp (local_config.wp_file, selected))
		{
			g_strlcpy (local_config.wp_file, selected,
				   sizeof (local_config.wp_file));
			if (mydebug > 3)
				fprintf (stderr, "active wp_file: %s\n",
					local_config.wp_file);
			loadwaypoints ();
			iszoomed = FALSE;
		}
		g_free (selected);
	}
	current.needtosave = TRUE;
	return TRUE;
}

/* ************************************************************************* */
static void
settings_close_cb (GtkWidget *window)
{
	if (id_timeout_autotracksave == 0 && local_config.track_autointerval > 0)
		id_timeout_autotracksave = g_timeout_add (local_config.track_autointerval * 1800 * 1000,
			(GtkFunction) track_autosave_cb, FALSE);

	gtk_widget_destroy (window);
}


/* ************************************************************************* */
static gboolean
set_poilabel_view (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gchar *data)
{
	gchar *t_buf;
	gboolean t_show;

	gtk_tree_model_get (model, iter, POITYPE_NAME, &t_buf, POITYPE_LABEL, &t_show, -1);

	if (g_str_has_prefix (t_buf, data+1))
	{
		if (mydebug > 30)
			g_print ("set_poilabel_view: setting %s to %d\n", t_buf, *data);
		if (*data)
		{
			gtk_tree_store_set (GTK_TREE_STORE (model), iter,
				POITYPE_SELECT, TRUE,
				POITYPE_LABEL, TRUE, -1);
			t_show = TRUE;
		}
		else
		{
			gtk_tree_store_set (GTK_TREE_STORE (model), iter,
				POITYPE_LABEL, FALSE, -1);
			t_show = FALSE;
		}
	}

	/* set poi label config string */
	if (g_strstr_len (t_buf, 30, ".") == NULL && !g_str_has_prefix (t_buf, "__NO_FILTER__"))
	{
		if (t_show)
		{
			g_strlcat (local_config.poi_label, t_buf, sizeof (local_config.poi_label));
			g_strlcat (local_config.poi_label, "|", sizeof (local_config.poi_label));
		}
	}

	current.needtosave = TRUE;
	g_free (t_buf);

	return FALSE;
}


/* ************************************************************************* */
static gboolean
toggle_poilabel
	(GtkCellRendererToggle *renderer, gchar *path_str, gpointer data)
{
	GtkTreeIter iter, child_iter;
	gchar *t_name;
	gchar t_buf[161];
	gboolean t_label;

	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (poi_types_tree_filtered), &iter, path_str);
	gtk_tree_model_filter_convert_iter_to_child_iter
		(GTK_TREE_MODEL_FILTER (poi_types_tree_filtered), &child_iter, &iter);
	gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree), &child_iter,
		POITYPE_NAME, &t_name, POITYPE_LABEL, &t_label, -1);
	g_snprintf (t_buf, sizeof (t_buf), "%c%s", !t_label, t_name);
	g_free (t_name);

	g_strlcpy (local_config.poi_label, "", sizeof (local_config.poi_label));
	gtk_tree_model_foreach (GTK_TREE_MODEL (poi_types_tree),
		*(GtkTreeModelForeachFunc) set_poilabel_view, t_buf);

	if (mydebug > 20)
		g_print ("Setting POI-Label filter to: %s\n", local_config.poi_label);

	return TRUE;
}


/* ************************************************************************* */
static void
toggle_poitype
	(GtkCellRendererToggle *renderer, gchar *path_str, gpointer data)
{
	GtkTreeIter iter, child_iter;
	gboolean value;

	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (poi_types_tree_filtered), &iter, path_str);
	gtk_tree_model_get (GTK_TREE_MODEL (poi_types_tree_filtered), &iter,
		POITYPE_SELECT, &value, -1);

	value = !value;

	gtk_tree_model_filter_convert_iter_to_child_iter
		(GTK_TREE_MODEL_FILTER (poi_types_tree_filtered), &child_iter, &iter);
	gtk_tree_store_set (GTK_TREE_STORE (poi_types_tree), &child_iter, POITYPE_SELECT, value, -1);

	update_poi_type_filter ();
}


/* *************************************************************************
 *   SETTINGS WINDOW
 * ************************************************************************* */

/* ************************************************************************* */
static void
settings_general (GtkWidget *notebook)
{
	GtkWidget *simulation_lb;
	GtkWidget *simmode_auto_rb, *simmode_on_rb, *simmode_off_rb;
	GtkWidget *simmode_table;
	GtkWidget *maxcpu_label, *maxcpu_spin;
	GtkTooltips *general_tooltips;
	GtkWidget *general_vbox;
	GtkWidget *general_label;
	GtkWidget *misc_table, *misc_frame, *misc_fr_lb;
	GtkWidget *mapdir_label, *mapdir_bt, *mapnik_caching_bt;
	GtkWidget *map_table, *map_frame, *map_fr_lb;

	general_tooltips = gtk_tooltips_new ();

	/* misc settings */
	{
	simulation_lb = gtk_label_new (_("Simulation mode"));
	simmode_auto_rb = gtk_radio_button_new_with_label
		(NULL, _("Automatic"));
	g_signal_connect (simmode_auto_rb, "toggled",
		GTK_SIGNAL_FUNC (setsimmode_cb), (gpointer) SIM_AUTO);
	simmode_on_rb = gtk_radio_button_new_with_label_from_widget
		(GTK_RADIO_BUTTON (simmode_auto_rb), _("On"));
	g_signal_connect (simmode_on_rb, "toggled",
		GTK_SIGNAL_FUNC (setsimmode_cb), (gpointer) SIM_ON);
	simmode_off_rb = gtk_radio_button_new_with_label_from_widget
		(GTK_RADIO_BUTTON (simmode_auto_rb), _("Off"));
	g_signal_connect (simmode_off_rb, "toggled",
		GTK_SIGNAL_FUNC (setsimmode_cb), (gpointer) SIM_OFF);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (general_tooltips), simmode_auto_rb,
		_("If activated, the position pointer moves towards "
		"the selected target simulating a moving vehicle, when no "
		"GPS is available."), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (general_tooltips), simmode_on_rb,
		_("If activated, the position pointer moves towards "
		"the selected target simulating a moving vehicle always."),
		NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (general_tooltips), simmode_off_rb,
		_("Switches simulation mode off"), NULL);
	switch (local_config.simmode)
	{
		case SIM_OFF:
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (simmode_off_rb), TRUE);
			break;
		case SIM_ON:
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (simmode_on_rb), TRUE);
			break;
		case SIM_AUTO:
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (simmode_auto_rb), TRUE);
			break;
	}
	
	simmode_table = gtk_table_new (1, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (simmode_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (simmode_table), 5);
	gtk_table_attach_defaults (GTK_TABLE (simmode_table),
		simmode_on_rb, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (simmode_table),
		simmode_off_rb, 1, 2, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (simmode_table),
		simmode_auto_rb, 2, 3, 0, 1);

	maxcpu_label = gtk_label_new (_("Maximum CPU load (in %)"));
	maxcpu_spin = gtk_spin_button_new_with_range (0, 95, 5);
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON (maxcpu_spin),
		(gdouble) local_config.maxcpuload);
	gtk_tooltips_set_tip (general_tooltips, maxcpu_spin,
		/* xgettext:no-c-format */
		_("Select the approximate maximum CPU load.\n"
		"Use 20-30% on notebooks while on battery, to save power. "
		"This affects the refresh rate of the map screen."), NULL);
	gtk_signal_connect (GTK_OBJECT (maxcpu_spin), "changed",
		GTK_SIGNAL_FUNC (setmaxcpuload_cb), NULL);
	}

	/* misc table */
	{
	misc_table = gtk_table_new (4, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (misc_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (misc_table), 5);
	gtk_table_attach (GTK_TABLE (misc_table),
		simulation_lb, 0, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (misc_table),
		simmode_table, 0, 2, 1, 2);
	gtk_table_attach (GTK_TABLE (misc_table),
		maxcpu_label, 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (misc_table),
		maxcpu_spin, 1, 2, 2, 3);
	}

	/* map settings */
	{
	mapdir_label = gtk_label_new (_("Maps directory"));
	mapdir_bt = gtk_file_chooser_button_new (_("Select Maps Directory"),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder
		(GTK_FILE_CHOOSER (mapdir_bt), local_config.dir_maps);
	gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (mapdir_bt), TRUE);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (general_tooltips), mapdir_bt,
		_("Path to your map files. In the specified directory "
		"also the index file map_koord.txt must be present."), NULL);
	g_signal_connect (mapdir_bt, "selection-changed",
		GTK_SIGNAL_FUNC (setmapdir_cb), NULL);

	mapnik_caching_bt = gtk_check_button_new_with_label (_("Enable Map Caching"));
	if (local_config.mapnik_caching)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mapnik_caching_bt), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mapnik_caching_bt), FALSE);
	g_signal_connect (mapnik_caching_bt, "toggled", G_CALLBACK (settogglevalue_cb), &local_config.mapnik_caching);
	}
	
	/* map table */
	{
	map_table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (map_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (map_table), 5);
	gtk_table_attach (GTK_TABLE (map_table),
		mapdir_label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (map_table),
		mapdir_bt, 1, 2, 0, 1);
#ifdef MAPNIK
	gtk_table_attach_defaults (GTK_TABLE (map_table), mapnik_caching_bt, 0, 2, 1, 2);
#endif
	}
	
	general_vbox = gtk_vbox_new (FALSE, 2);

	misc_frame = gtk_frame_new (NULL);
	misc_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (misc_fr_lb), _("<b>Miscellaneous</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (misc_frame), misc_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (misc_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (misc_frame), misc_table);

	map_frame = gtk_frame_new (NULL);
	map_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (map_fr_lb), _("<b>Map Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (map_frame), map_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (map_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (map_frame), map_table);

	gtk_box_pack_start (GTK_BOX (general_vbox),
		map_frame, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (general_vbox),
		misc_frame, FALSE, FALSE, 2);
	
	general_label = gtk_label_new (_("General"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), general_vbox, general_label);
}

/* ************************************************************************* */
static void
settings_gui (GtkWidget *notebook)
{
	GtkWidget *gui_vbox, *gui_label;
	GtkWidget *gui_map_frame, *gui_map_fr_lb;
	GtkWidget *gui_other_frame, *gui_other_fr_lb;
	GtkWidget *gui_map_table, *gui_other_table;
	GtkWidget *gui_shadow_bt, *gui_gridshow_bt, *gui_compassmode_bt;
	GtkWidget *gui_scaleshow_bt, *gui_zoomshow_bt;
	GtkWidget *gui_battery_bt, *gui_temp_bt, *gui_wayshow_bt;
	GtkWidget *gui_marker_lb, *gui_marker_bt;
	GtkWidget *dist_label, *dist_combo;
	GtkWidget *alt_label, *alt_combo;
	GtkWidget *coord_label, *coord_combo;
	GtkWidget *units_table, *units_frame, *units_fr_lb;
	GtkTooltips *gui_tooltips;

	gui_vbox = gtk_vbox_new (FALSE, 2);
	gui_tooltips = gtk_tooltips_new ();	

	/* map features settings */
	{
	gui_gridshow_bt = gtk_check_button_new_with_label (_("Show grid"));
	gtk_tooltips_set_tip (gui_tooltips, gui_gridshow_bt,
		_("This will show a grid over the map"), NULL);
	if (local_config.showgrid)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_gridshow_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_gridshow_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_gridshow_bt), "clicked",
		      GTK_SIGNAL_FUNC (settogglevalue_cb),
		      &local_config.showgrid);

	gui_shadow_bt = gtk_check_button_new_with_label (_("Show Shadows"));
	gtk_tooltips_set_tip (gui_tooltips, gui_shadow_bt,
		_("Switches shadows on map on or off"), NULL);
	if (local_config.showshadow)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_shadow_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_shadow_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_shadow_bt), "clicked",
		      GTK_SIGNAL_FUNC (settogglevalue_cb),
		      &local_config.showshadow);

	gui_zoomshow_bt = gtk_check_button_new_with_label (_("Show zoom level"));
	gtk_tooltips_set_tip (gui_tooltips, gui_zoomshow_bt,
		_("This will show the current zoom level of the map"), NULL);
	if (local_config.showzoom)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_zoomshow_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_zoomshow_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_zoomshow_bt), "clicked",
		      GTK_SIGNAL_FUNC (settogglevalue_cb),
		      &local_config.showzoom);

	gui_scaleshow_bt = gtk_check_button_new_with_label (_("Show scalebar"));
	gtk_tooltips_set_tip (gui_tooltips, gui_scaleshow_bt,
		_("This will show the scalebar in the map"), NULL);
	if (local_config.showscalebar)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_scaleshow_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_scaleshow_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_scaleshow_bt), "clicked",
		      GTK_SIGNAL_FUNC (settogglevalue_cb),
		      &local_config.showscalebar);

	gui_compassmode_bt = gtk_check_button_new_with_label (_("Rotating Compass"));
	gtk_tooltips_set_tip (gui_tooltips, gui_scaleshow_bt,
		_("Rotate the compass depending on your heading"), NULL);
	if (local_config.rotating_compass)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_compassmode_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_compassmode_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_compassmode_bt), "clicked",
		      GTK_SIGNAL_FUNC (settogglevalue_cb),
		      &local_config.rotating_compass);

	gui_wayshow_bt = gtk_check_button_new_with_label (_("Show way info (EXPERIMENTAL!)"));
	gtk_tooltips_set_tip (gui_tooltips, gui_wayshow_bt,
		_("This will show information about the current way (this is an experimental feature, which may not work as expected!!!"), NULL);
#ifdef MAPNIK
	if (local_config.showway)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_wayshow_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_wayshow_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_wayshow_bt), "clicked",
		      GTK_SIGNAL_FUNC (settogglevalue_cb),
		      &local_config.showway);
#else
	gtk_widget_set_sensitive (gui_wayshow_bt, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gui_wayshow_bt), FALSE);
#endif

	gui_marker_lb = gtk_label_new (_("Position Marker"));
	gui_marker_bt = gtk_combo_box_new_text ();
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (gui_marker_bt), "Blob");
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (gui_marker_bt), "Arrow");
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (gui_marker_bt), "T-Style");
	gtk_combo_box_set_active
		(GTK_COMBO_BOX (gui_marker_bt), local_config.posmarker);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (gui_tooltips), gui_marker_bt,
		_("Choose the appearance of your position marker."), NULL);
	g_signal_connect (gui_marker_bt, "changed",
		GTK_SIGNAL_FUNC (setposmarker_cb), NULL);

	gui_map_table = gtk_table_new (4, 4, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (gui_map_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (gui_map_table), 5);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_gridshow_bt, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_shadow_bt, 2, 3, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_zoomshow_bt, 0, 1, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_scaleshow_bt, 2, 3, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_compassmode_bt, 0, 1, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_wayshow_bt, 2, 3, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_marker_lb, 0, 1, 3, 4);
	gtk_table_attach_defaults (GTK_TABLE (gui_map_table),
		gui_marker_bt, 2, 3, 3, 4);
	}

	/* other features settings */
	{
	gui_battery_bt = gtk_check_button_new_with_label (_("Show battery"));
	gtk_tooltips_set_tip (gui_tooltips, gui_battery_bt,
		_("This will show the state of charge of your battery if available"), NULL);
	if (local_config.showbatt)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_battery_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_battery_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_battery_bt), "clicked",
		      GTK_SIGNAL_FUNC (settoggleapm_cb),
		      &local_config.showbatt);

	gui_temp_bt = gtk_check_button_new_with_label (_("Show temperature"));
	gtk_tooltips_set_tip (gui_tooltips, gui_temp_bt,
		_("This will show the temperature of your CPU if available"), NULL);
	if (local_config.showtemp)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_temp_bt), TRUE);
	}
	else
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (gui_temp_bt), FALSE);
	}
	g_signal_connect (GTK_OBJECT (gui_temp_bt), "clicked",
		      GTK_SIGNAL_FUNC (settoggleapm_cb),
		      &local_config.showtemp);

	gui_other_table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (gui_other_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (gui_other_table), 5);
	gtk_table_attach_defaults (GTK_TABLE (gui_other_table),
		gui_battery_bt, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (gui_other_table),
		gui_temp_bt, 1, 2, 0, 1);
	}

	/* distance format */
	{
	dist_label = gtk_label_new (_("Distance"));
	dist_combo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (dist_combo), "stat. miles");
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (dist_combo), "kilometers");
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (dist_combo), "naut. miles");
	gtk_combo_box_set_active
		(GTK_COMBO_BOX (dist_combo), local_config.distmode);
	gtk_tooltips_set_tip (gui_tooltips, dist_label,
		_("Choose here the unit for the display of distances."), NULL);
	g_signal_connect (dist_combo, "changed",
		GTK_SIGNAL_FUNC (setdistmode_cb), 0);
	}
	
	/* altitude format */
	{
	alt_label = gtk_label_new (_("Altitude"));
	alt_combo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (alt_combo), "feet");
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (alt_combo), "meters");
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (alt_combo), "yards");
	gtk_combo_box_set_active
		(GTK_COMBO_BOX (alt_combo), local_config.altmode);
	gtk_tooltips_set_tip (gui_tooltips, alt_label,
		_("Choose here the unit for the display of altitudes."), NULL);
	g_signal_connect (alt_combo, "changed",
		GTK_SIGNAL_FUNC (setaltmode_cb), 0);
	}
	
	/* coordinate format */
	{
	coord_label = gtk_label_new (_("Coordinates"));
	coord_combo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (coord_combo), "DD.ddddd");
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (coord_combo), "DD MM SS.ss");
	gtk_combo_box_append_text
			(GTK_COMBO_BOX (coord_combo), "DD MM.mmm");
	gtk_combo_box_set_active
		(GTK_COMBO_BOX (coord_combo), local_config.coordmode);
	gtk_tooltips_set_tip (gui_tooltips, coord_label,
		_("Choose here the format for the coordinates display."), NULL);
	g_signal_connect (coord_combo, "changed",
		GTK_SIGNAL_FUNC (setcoordmode_cb), 0);
	}

	/* units table */
	{
	units_table = gtk_table_new (3, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (units_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (units_table), 5);
	gtk_table_attach (GTK_TABLE (units_table),
		coord_label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (units_table),
		coord_combo, 1, 2, 0, 1);
	gtk_table_attach (GTK_TABLE (units_table),
		dist_label, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (units_table),
		dist_combo, 1, 2, 1, 2);
	gtk_table_attach (GTK_TABLE (units_table),
		alt_label, 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (units_table),
		alt_combo, 1, 2, 2, 3);
	}

	/* gui features frame */
	gui_map_frame = gtk_frame_new (NULL);
	gui_map_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (gui_map_fr_lb), _("<b>Map Features</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (gui_map_frame), gui_map_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (gui_map_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (gui_map_frame), gui_map_table);

	/* other features frame */
	gui_other_frame = gtk_frame_new (NULL);
	gui_other_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (gui_other_fr_lb), _("<b>Other Features</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (gui_other_frame), gui_other_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (gui_other_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (gui_other_frame), gui_other_table);

	/* units format frame */
	units_frame = gtk_frame_new (NULL);
	units_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (units_fr_lb), _("<b>Units</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (units_frame), units_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (units_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (units_frame), units_table);

	gtk_box_pack_start (GTK_BOX (gui_vbox), gui_map_frame, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (gui_vbox), gui_other_frame, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (gui_vbox), units_frame, FALSE, FALSE, 2);

	gui_label = gtk_label_new (_("Display"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), gui_vbox, gui_label);
}

/* ************************************************************************* */
static void
settings_trk (GtkWidget *notebook)
{
	GtkWidget *trk_vbox, *trkdir_label, *trkdir_bt;
	GtkWidget *trk_general_frame, *trk_general_fr_lb;
	GtkWidget *trk_auto_frame, *trk_auto_fr_lb, *trk_autoclear_bt;
	GtkWidget *trk_auto_table, *trk_general_table, *trk_autounit_label;
	GtkWidget *trk_label, *trk_autoint_label, *trk_autoint_spin;
	GtkWidget *trk_autoprefix_label, *trk_autoprefix_entry;
	GtkWidget *trk_showrestart_bt, *trk_showclear_bt;
	GtkWidget *trk_int_label, *trk_unit_label, *trk_int_spin;
	GtkTooltips *trk_tooltips;

	trk_vbox = gtk_vbox_new (FALSE, 2);
	trk_tooltips = gtk_tooltips_new ();

	/* track dir settings */
	trkdir_label = gtk_label_new (_("Tracks directory"));
	trkdir_bt = gtk_file_chooser_button_new (_("Select Tracks Directory"),
		GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder
		(GTK_FILE_CHOOSER (trkdir_bt), local_config.dir_tracks);
	gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (trkdir_bt), TRUE);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (trk_tooltips), trkdir_bt,
		_("Path to your track files."), NULL);
	g_signal_connect (trkdir_bt, "selection-changed",
		GTK_SIGNAL_FUNC (settrackdir_cb), NULL);

	/* track interval setings */
	trk_int_label = gtk_label_new (_("Time interval between Track Points"));
	trk_unit_label = gtk_label_new (_("seconds"));
	trk_int_spin = gtk_spin_button_new_with_range (1.0, 600.0, 1.0);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (trk_int_spin),
		local_config.track_interval);
	g_signal_connect (trk_int_spin, "changed",
		GTK_SIGNAL_FUNC (settrkint_cb), NULL);

	/* track buttons settings */
	trk_showrestart_bt = gtk_check_button_new_with_label
		(_("Show 'Restart Track' Button"));
	if (local_config.showbutton_trackrestart)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (trk_showrestart_bt), TRUE);
	}
	g_signal_connect (trk_showrestart_bt, "clicked",
		GTK_SIGNAL_FUNC (settogglevalue_cb), &local_config.showbutton_trackrestart);

	trk_showclear_bt = gtk_check_button_new_with_label
		(_("Show 'Clear Track' Button"));
	if (local_config.showbutton_trackclear)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (trk_showclear_bt), TRUE);
	}
	g_signal_connect (trk_showclear_bt, "clicked",
		GTK_SIGNAL_FUNC (settogglevalue_cb), &local_config.showbutton_trackclear);

	/* track autosave settings */
	trk_autoint_label = gtk_label_new (_("Track Export Interval (0 to disable)"));
	trk_autounit_label = gtk_label_new (_("hours"));
	trk_autoint_spin = gtk_spin_button_new_with_range (0, 750.0, 0.5);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (trk_autoint_spin),
		(gdouble) local_config.track_autointerval/2);
	g_signal_connect (trk_autoint_spin, "changed",
		GTK_SIGNAL_FUNC (settrkautoint_cb), NULL);

	trk_autoprefix_label = gtk_label_new (_("Filename Prefix"));
	trk_autoprefix_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (trk_autoprefix_entry), 50);
	gtk_entry_set_text (GTK_ENTRY (trk_autoprefix_entry), local_config.track_autoprefix);
	g_signal_connect (trk_autoprefix_entry, "changed",
		GTK_SIGNAL_FUNC (settrkautoprefix_cb), NULL);

	trk_autoclear_bt = gtk_check_button_new_with_label
		(_("Clear track after writing"));
	if (local_config.track_autoclean)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (trk_autoclear_bt), TRUE);
	}
	g_signal_connect (trk_autoclear_bt, "clicked",
		GTK_SIGNAL_FUNC (settogglevalue_cb), &local_config.track_autoclean);


	/* general settings frame */
	trk_general_frame = gtk_frame_new (NULL);
	trk_general_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (trk_general_fr_lb), _("<b>General Settings</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (trk_general_frame), trk_general_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (trk_general_frame), GTK_SHADOW_NONE);
	trk_general_table = gtk_table_new (4, 5, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (trk_general_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (trk_general_table), 5);
	gtk_table_attach (GTK_TABLE (trk_general_table),
		trkdir_label, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (trk_general_table),
		trkdir_bt, 1, 5, 1, 2);
	gtk_table_attach (GTK_TABLE (trk_general_table),
		trk_int_label, 0, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (trk_general_table),
		trk_int_spin, 3, 4, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (trk_general_table),
		trk_unit_label, 4, 5, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (trk_general_table),
		trk_showclear_bt, 0, 5, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (trk_general_table),
		trk_showrestart_bt, 0, 5, 3, 4);
	gtk_container_add (GTK_CONTAINER (trk_general_frame), trk_general_table);

	/* autosave settings frame */
	trk_auto_frame = gtk_frame_new (NULL);
	trk_auto_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (trk_auto_fr_lb), _("<b>Autosave Settings</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (trk_auto_frame), trk_auto_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (trk_auto_frame), GTK_SHADOW_NONE);
	trk_auto_table = gtk_table_new (3, 5, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (trk_auto_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (trk_auto_table), 5);
	gtk_table_attach (GTK_TABLE (trk_auto_table),
		trk_autoint_label, 0, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (trk_auto_table),
		trk_autoint_spin, 3, 4, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (trk_auto_table),
		trk_autounit_label, 4, 5, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (trk_auto_table),
		trk_autoprefix_label, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (trk_auto_table),
		trk_autoprefix_entry, 1, 5, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (trk_auto_table),
		trk_autoclear_bt, 0, 5, 2, 3);
	gtk_container_add (GTK_CONTAINER (trk_auto_frame), trk_auto_table);

	gtk_box_pack_start (GTK_BOX (trk_vbox), trk_general_frame,
		FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (trk_vbox), trk_auto_frame,
		FALSE, FALSE, 2);

	trk_label = gtk_label_new (_("Tracking"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), trk_vbox, trk_label);
}

/* ************************************************************************* */
static void
settings_col (GtkWidget *notebook)
{
	GtkWidget *col_vbox, *col_label;
	GtkWidget *col_nighton_rb, *col_nightoff_rb;
	GtkWidget *col_nightauto_rb, *col_night_table;
	GtkWidget *col_map_frame, *col_map_fr_lb;
	GtkWidget *col_night_frame, *col_night_fr_lb;
	GtkWidget *col_wpfont_bt, *col_map_table;
	GtkWidget *col_trackcol_bt, *col_routecol_bt;
	GtkWidget *col_friendscol_bt, *col_friendscol_lb;
	GtkWidget *col_wpcol_bt, *col_wpcol_lb;
	GtkWidget *col_trackcol_lb, *col_routecol_lb;
	GtkWidget *col_bigcol_lb, *col_bigcol_bt;
	GtkWidget *col_bigfont_bt, *col_friendsfont_bt;
	GtkWidget *col_trackstyle_combo, *col_routestyle_combo;
	GtkTooltips *col_tooltips;

	col_vbox = gtk_vbox_new (FALSE, 2);
	col_tooltips = gtk_tooltips_new ();	

	/* gui fonts/colors settings */
	{
	col_trackcol_lb = gtk_label_new (_("Track"));
	col_trackcol_bt = gtk_color_button_new_with_color
		(&colors.track);
	gtk_color_button_set_title
		(GTK_COLOR_BUTTON (col_trackcol_bt),
		_("Choose Track color"));
	g_signal_connect (col_trackcol_bt, "color-set",
		GTK_SIGNAL_FUNC (setcolor_cb), &colors.track);
	gtk_tooltips_set_tip (col_tooltips, col_trackcol_bt,
		_("Set here the color of the drawn track"), NULL);
	col_trackstyle_combo = gtk_combo_box_new_text ();
	gtk_tooltips_set_tip (col_tooltips, col_trackstyle_combo,
		_("Set here the line style of the drawn track"), NULL);
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_trackstyle_combo), _("________ solid ________"));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_trackstyle_combo), _("_ . _ .  dotdashed _ . _ . "));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_trackstyle_combo), _("_ _ _ _  dashed _ _ _ _ "));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_trackstyle_combo), _("........ dotted ........"));
	gtk_combo_box_set_active
		(GTK_COMBO_BOX (col_trackstyle_combo), local_config.style_track);
	g_signal_connect (col_trackstyle_combo, "changed",
		GTK_SIGNAL_FUNC (setlinestyle_cb), &local_config.style_track);

	/* Route line color & style */
	col_routecol_lb = gtk_label_new (_("Route"));
	col_routecol_bt = gtk_color_button_new_with_color
		(&colors.route);
	gtk_color_button_set_title
		(GTK_COLOR_BUTTON (col_routecol_bt),
		_("Choose Route color"));
	g_signal_connect (col_routecol_bt, "color-set",
		GTK_SIGNAL_FUNC (setcolor_cb), &colors.route);
	gtk_tooltips_set_tip (col_tooltips, col_routecol_bt,
		_("Set here the color of the drawn route"), NULL);
	col_routestyle_combo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_routestyle_combo), _("________ solid ________"));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_routestyle_combo), _("_ . _ .  dotdashed _ . _ . "));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_routestyle_combo), _("_ _ _ _ dashed _ _ _ _ "));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (col_routestyle_combo), _("........ dotted ........"));
	gtk_tooltips_set_tip (col_tooltips, col_routestyle_combo,
		_("Set here the line style of the drawn route"), NULL);
	gtk_combo_box_set_active
		(GTK_COMBO_BOX (col_routestyle_combo), local_config.style_route);
	g_signal_connect (col_routestyle_combo, "changed",
		GTK_SIGNAL_FUNC (setlinestyle_cb), &local_config.style_route);

	/* Friends label color & font */
	col_friendscol_lb = gtk_label_new (_("Friends"));
	col_friendscol_bt = gtk_color_button_new_with_color
		(&colors.friends);
	gtk_color_button_set_title
		(GTK_COLOR_BUTTON (col_friendscol_bt),
		_("Choose Friends color"));
	g_signal_connect (col_friendscol_bt, "color-set",
		GTK_SIGNAL_FUNC (setcolor_cb), &colors.friends);
	gtk_tooltips_set_tip (col_tooltips, col_friendscol_bt,
		_("Set here the text color of the drawn friends"), NULL);
	col_friendsfont_bt = gtk_font_button_new_with_font
		(local_config.font_friends);
	gtk_font_button_set_title
		(GTK_FONT_BUTTON (col_friendsfont_bt),
		_("Choose font for friends"));
	gtk_font_button_set_use_font
		(GTK_FONT_BUTTON (col_friendsfont_bt), TRUE);
	g_signal_connect (col_friendsfont_bt, "font-set",
		GTK_SIGNAL_FUNC (setfont_cb), local_config.font_friends);
	gtk_tooltips_set_tip (col_tooltips, col_friendsfont_bt,
		_("Set here the font of the drawn friends"), NULL);

	/* Waypoints label color & font */
	col_wpcol_lb = gtk_label_new (_("Waypoints"));
	col_wpcol_bt = gtk_color_button_new_with_color
		(&colors.wplabel);
	gtk_color_button_set_title
		(GTK_COLOR_BUTTON (col_wpcol_bt),
		_("Choose Waypoints label color"));
	g_signal_connect (col_wpcol_bt, "color-set",
		GTK_SIGNAL_FUNC (setcolor_cb), &colors.wplabel);
	gtk_tooltips_set_tip (col_tooltips, col_wpcol_bt,
		_("Set here the text color of the waypoint labels"), NULL);
	col_wpfont_bt = gtk_font_button_new_with_font
		(local_config.font_wplabel);
	gtk_font_button_set_title
		(GTK_FONT_BUTTON (col_wpfont_bt),
		_("Choose font for waypoint labels"));
	gtk_font_button_set_use_font
		(GTK_FONT_BUTTON (col_wpfont_bt), TRUE);
	g_signal_connect (col_wpfont_bt, "font-set",
		GTK_SIGNAL_FUNC (setfont_cb), local_config.font_wplabel);
	gtk_tooltips_set_tip (col_tooltips, col_wpfont_bt,
		_("Set here the font of waypoint labels"), NULL);

	/* Big Display color & font */
	col_bigcol_lb = gtk_label_new (_("Dashboard"));
	col_bigcol_bt = gtk_color_button_new_with_color
		(&colors.dashboard);
	gtk_color_button_set_title
		(GTK_COLOR_BUTTON (col_bigcol_bt),
		_("Choose color for dashboard"));
	g_signal_connect (col_bigcol_bt, "color-set",
		GTK_SIGNAL_FUNC (setcolor_cb), &colors.dashboard);
	gtk_tooltips_set_tip (col_tooltips, col_bigcol_bt,
		_("Set here the color of the dashboard"), NULL);
	col_bigfont_bt = gtk_font_button_new_with_font
		(local_config.font_dashboard);
	gtk_font_button_set_title
		(GTK_FONT_BUTTON (col_bigfont_bt),
		_("Choose font for dashboard"));
	gtk_font_button_set_use_font
		(GTK_FONT_BUTTON (col_bigfont_bt), TRUE);
	g_signal_connect (col_bigfont_bt, "font-set",
		GTK_SIGNAL_FUNC (setfont_cb), local_config.font_dashboard);
	gtk_tooltips_set_tip (col_tooltips, col_bigfont_bt,
		_("Set here the font of the dashboard"), NULL);

	col_map_table = gtk_table_new (5, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (col_map_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (col_map_table), 5);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_trackcol_lb, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_trackcol_bt, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (col_map_table),
		col_trackstyle_combo, 2, 3, 0, 1);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_routecol_lb, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_routecol_bt, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (col_map_table),
		col_routestyle_combo, 2, 3, 1, 2);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_friendscol_lb, 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_friendscol_bt, 1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (col_map_table),
		col_friendsfont_bt, 2, 3, 2, 3);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_wpcol_lb, 0, 1, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_wpcol_bt, 1, 2, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (col_map_table),
		col_wpfont_bt, 2, 3, 3, 4);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_bigcol_lb, 0, 1, 4, 5, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (col_map_table),
		col_bigcol_bt, 1, 2, 4, 5, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (col_map_table),
		col_bigfont_bt, 2, 3, 4, 5);
	}

	/* gui nightmode settings */
	{
	col_nightauto_rb = gtk_radio_button_new_with_label
		(NULL, _("Automatic"));
	g_signal_connect (col_nightauto_rb, "toggled",
		GTK_SIGNAL_FUNC (setnightmode_cb), (gpointer) NIGHT_AUTO);
	col_nighton_rb = gtk_radio_button_new_with_label_from_widget
		(GTK_RADIO_BUTTON (col_nightauto_rb), _("On"));
	g_signal_connect (col_nighton_rb, "toggled",
		GTK_SIGNAL_FUNC (setnightmode_cb), (gpointer) NIGHT_ON);
	col_nightoff_rb = gtk_radio_button_new_with_label_from_widget
		(GTK_RADIO_BUTTON (col_nightauto_rb), _("Off"));
	g_signal_connect (col_nightoff_rb, "toggled",
		GTK_SIGNAL_FUNC (setnightmode_cb), (gpointer) NIGHT_OFF);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (col_tooltips), col_nightauto_rb,
		_("Switches automagically to night mode if it is dark "
		"outside. Press 'N' key to turn off nightmode."), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (col_tooltips), col_nighton_rb,
		_("Switches night mode on. Press 'N' key to turn off "
		"nightmode."), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (col_tooltips), col_nightoff_rb,
		_("Switches night mode off"), NULL);
	switch (local_config.nightmode)
	{
		case NIGHT_OFF:
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (col_nightoff_rb), TRUE);
			break;
		case NIGHT_ON:
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (col_nighton_rb), TRUE);
			break;
		case NIGHT_AUTO:
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (col_nightauto_rb), TRUE);
			break;
	}

	col_night_table = gtk_table_new (1, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (col_night_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (col_night_table), 5);
	gtk_table_attach_defaults (GTK_TABLE (col_night_table),
		col_nighton_rb, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (col_night_table),
		col_nightoff_rb, 1, 2, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (col_night_table),
		col_nightauto_rb, 2, 3, 0, 1);
	}

	/* gui fonts/colors/styles frame */
	col_map_frame = gtk_frame_new (NULL);
	col_map_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (col_map_fr_lb), _("<b>Fonts, Colors, Styles</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (col_map_frame), col_map_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (col_map_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (col_map_frame), col_map_table);

	/* gui nightmode frame */
	col_night_frame = gtk_frame_new (NULL);
	col_night_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (col_night_fr_lb), _("<b>Nightmode</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (col_night_frame), col_night_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (col_night_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (col_night_frame), col_night_table);

	gtk_box_pack_start
		(GTK_BOX (col_vbox), col_map_frame, FALSE, FALSE, 2);
	gtk_box_pack_start
		(GTK_BOX (col_vbox), col_night_frame, FALSE, FALSE, 2);
	
	col_label = gtk_label_new (_("Colors"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), col_vbox, col_label);
}

/* ************************************************************************* */
static void
settings_nav (GtkWidget *notebook)
{
	GtkWidget *nav_vbox, *nav_label;
	GtkWidget *travel_label, *travel_combo;
	GtkWidget *nav_table;
	GtkWidget *nav_frame;
	GtkWidget *nav_fr_lb;
	GtkTooltips *nav_tooltips;
	
	gchar travelmodes[TRAVEL_N_MODES][20];
	gint i;

	nav_tooltips = gtk_tooltips_new ();	
	nav_vbox = gtk_vbox_new (FALSE, 2);
	
	/* travelmode */
	{
	travel_label = gtk_label_new (_("Travel Mode"));
	travel_combo = gtk_combo_box_new_text ();
	g_strlcpy (travelmodes[TRAVEL_CAR], _("Car"),
		sizeof(travelmodes[TRAVEL_CAR]));
	g_strlcpy (travelmodes[TRAVEL_BIKE], _("Bike"),
		sizeof(travelmodes[TRAVEL_BIKE]));
	g_strlcpy (travelmodes[TRAVEL_WALK], _("Walk"),
		sizeof(travelmodes[TRAVEL_WALK]));
	g_strlcpy (travelmodes[TRAVEL_BOAT], _("Boat"),
		sizeof(travelmodes[TRAVEL_BOAT]));
	g_strlcpy (travelmodes[TRAVEL_AIRPLANE], _("Airplane"),
		sizeof(travelmodes[TRAVEL_AIRPLANE]));
	for (i=0; i<TRAVEL_N_MODES; i++)
	{
		gtk_combo_box_append_text
			(GTK_COMBO_BOX (travel_combo), travelmodes[i]);
	}
	gtk_combo_box_set_active
		(GTK_COMBO_BOX (travel_combo), local_config.travelmode);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (nav_tooltips), travel_combo,
		_("Choose your travel mode. This is used to determine "
		"which icon should be used to display your position."), NULL);
	g_signal_connect (travel_combo, "changed",
		GTK_SIGNAL_FUNC (settravelmode_cb), NULL);
	}

	nav_table = gtk_table_new (3, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (nav_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (nav_table), 5);
	gtk_table_attach (GTK_TABLE (nav_table),
		travel_label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (nav_table),
		travel_combo, 1, 2, 0, 1);

	
	nav_frame = gtk_frame_new (NULL);
	nav_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (nav_fr_lb), _("<b>Navigation Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (nav_frame), nav_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (nav_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (nav_frame), nav_table);
	
	
	gtk_box_pack_start (GTK_BOX (nav_vbox), nav_frame, FALSE, FALSE, 2);

	
	nav_label = gtk_label_new (_("Navigation"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), nav_vbox, nav_label);
}

/* ************************************************************************* */
static void
settings_poi (GtkWidget *notebook)
{
	GtkWidget *poi_vbox, *poi_label;
	GtkTooltips * poi_tooltips;
	GtkWidget *wp_frame, *wp_fr_lb, *wp_table;
	GtkWidget *poisearch_frame, *poisearch_fr_lb, *poisearch_table;
	GtkWidget *poidisplay_frame, *poidisplay_fr_lb, *poidisplay_table;
	GtkWidget *wpfile_label, *wpfile_bt;
	GtkWidget *poitheme_label, *poitheme_combo;
	GtkWidget *poi_dist_label, *poi_dist_entry;
	GtkWidget *poi_max_label, *poi_max_entry;
	GtkWidget *poi_max2_label, *poi_dist2_label;
	GtkWidget *poifilter_label;
	GtkWidget *scrolledwindow_poitypes;
	GtkWidget *poitypes_treeview;
	GtkCellRenderer *renderer_poitypes;
	GtkTreeViewColumn *column_poitypes;
	GtkTreeSelection *poitypes_select;

	gchar text[50];
	
	poi_tooltips = gtk_tooltips_new ();	
	poi_vbox = gtk_vbox_new (FALSE, 2);

	/* Waypoints */
	{
	wpfile_label = gtk_label_new (_("Waypoints File"));
	wpfile_bt = gtk_file_chooser_button_new
		(_("Select Waypoints File"), GTK_FILE_CHOOSER_ACTION_OPEN);
	if (!gtk_file_chooser_set_filename
		(GTK_FILE_CHOOSER (wpfile_bt), local_config.wp_file))
	{
		gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (wpfile_bt), local_config.dir_home);
	}
	gtk_tooltips_set_tip (GTK_TOOLTIPS (poi_tooltips), wpfile_bt,
		_("Choose the waypoints file to use!\nCurrently only files in "
		"GpsDrive's way.txt format are supported."), NULL);
	g_signal_connect (wpfile_bt, "selection-changed",
		GTK_SIGNAL_FUNC (setwpfile_cb), NULL);

	wp_table = gtk_table_new (1, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (wp_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wp_table), 5);
	gtk_table_attach (GTK_TABLE (wp_table),
		wpfile_label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (wp_table),
		wpfile_bt, 1, 2, 0, 1);
	}

	/* POI Search settings */
	{
	poi_dist_label = gtk_label_new (_("Default search radius"));
	poi_dist_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (poi_dist_entry), 5);
	g_snprintf (text, sizeof (text), "%0.1f",
		local_config.poi_searchradius);
	gtk_entry_set_text
		(GTK_ENTRY (poi_dist_entry), text);
	g_signal_connect (poi_dist_entry, "changed",
		GTK_SIGNAL_FUNC (setpoisearch_cb), (gpointer) 1);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (poi_tooltips), poi_dist_entry,
		_("Choose the default search range (in km) for the POI-Lookup"
		" Window."), NULL);
	poi_dist2_label = gtk_label_new (_("km"));

	poi_max_label = gtk_label_new (_("Limit results to"));
	poi_max_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (poi_max_entry), 5);
	g_snprintf (text, sizeof (text), "%0d", local_config.poi_results_max);
	gtk_entry_set_text
		(GTK_ENTRY (poi_max_entry), text);
	g_signal_connect (poi_max_entry, "changed",
		GTK_SIGNAL_FUNC (setpoisearch_cb), (gpointer) 2);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (poi_tooltips), poi_max_entry,
		_("Choose the limit for the amount of found entries displayed "
		"in the POI-Lookup Window. Depending on your system a value "
		"set too high may slow down your system."), NULL);
	poi_max2_label = gtk_label_new (_("entries"));

	poisearch_table = gtk_table_new (2, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (poisearch_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (poisearch_table), 5);
	gtk_table_attach (GTK_TABLE (poisearch_table),
		poi_dist_label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (poisearch_table),
		poi_dist_entry, 1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (poisearch_table),
		poi_dist2_label, 2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (poisearch_table),
		poi_max_label, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (poisearch_table),
		poi_max_entry, 1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach (GTK_TABLE (poisearch_table),
		poi_max2_label, 2, 3, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	}

	/* POI Display settings */
	{
	poitheme_label = gtk_label_new (_("POI-Theme"));
	poitheme_combo = gtk_combo_box_new_text();
	gtk_combo_box_append_text
		(GTK_COMBO_BOX(poitheme_combo), "square.big");
	gtk_combo_box_append_text
		(GTK_COMBO_BOX(poitheme_combo), "square.small");
	gtk_combo_box_append_text
		(GTK_COMBO_BOX(poitheme_combo), "classic.big");
	gtk_combo_box_append_text
		(GTK_COMBO_BOX(poitheme_combo), "classic.small");
	if (!strcmp (local_config.icon_theme, "square.big"))
	{
		gtk_combo_box_set_active( GTK_COMBO_BOX (poitheme_combo), 0 );
	}
	else if (!strcmp (local_config.icon_theme, "square.small"))
	{
		gtk_combo_box_set_active( GTK_COMBO_BOX (poitheme_combo), 1 );
	}
	else if (!strcmp (local_config.icon_theme, "classic.big"))
	{
		gtk_combo_box_set_active( GTK_COMBO_BOX (poitheme_combo), 2 );
	}
	else if (!strcmp (local_config.icon_theme, "classic.small"))
	{
		gtk_combo_box_set_active( GTK_COMBO_BOX (poitheme_combo), 3 );
	}
	g_signal_connect (poitheme_combo, "changed",
		GTK_SIGNAL_FUNC (setpoitheme_cb), NULL);

	poifilter_label = gtk_label_new (_("POI-Filter"));
	scrolledwindow_poitypes = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW
		(scrolledwindow_poitypes), GTK_SHADOW_IN);

	poitypes_treeview = gtk_tree_view_new_with_model
		(GTK_TREE_MODEL (poi_types_tree_filtered));
	gtk_container_add (GTK_CONTAINER (scrolledwindow_poitypes),
		poitypes_treeview);

	renderer_poitypes = gtk_cell_renderer_toggle_new ();
	column_poitypes = gtk_tree_view_column_new_with_attributes (_("Display"),
		renderer_poitypes, "active", POITYPE_SELECT, NULL);
	g_signal_connect (renderer_poitypes, "toggled",
		G_CALLBACK (toggle_poitype), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview),
		column_poitypes);

	renderer_poitypes = gtk_cell_renderer_toggle_new ();
	column_poitypes = gtk_tree_view_column_new_with_attributes (_("Label"),
		renderer_poitypes, "active", POITYPE_LABEL, NULL);
	g_signal_connect (renderer_poitypes, "toggled",
		G_CALLBACK (toggle_poilabel), NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview),
		column_poitypes);

	renderer_poitypes = gtk_cell_renderer_pixbuf_new ();
	column_poitypes = gtk_tree_view_column_new_with_attributes (NULL,
		renderer_poitypes, "pixbuf", POITYPE_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview),
		column_poitypes);

	renderer_poitypes = gtk_cell_renderer_text_new ();
	column_poitypes = gtk_tree_view_column_new_with_attributes (NULL,
		renderer_poitypes, "text", POITYPE_TITLE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview),
		column_poitypes);

	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (poitypes_treeview),
		TRUE);
	gtk_tree_view_collapse_all (GTK_TREE_VIEW (poitypes_treeview));

	/* disable drawing of tree expanders */
	column_poitypes = gtk_tree_view_column_new ();
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview),
		column_poitypes);
	gtk_tree_view_set_expander_column
		(GTK_TREE_VIEW (poitypes_treeview), column_poitypes);
	gtk_tree_view_column_set_visible (column_poitypes, FALSE);

	poitypes_select = gtk_tree_view_get_selection
		(GTK_TREE_VIEW (poitypes_treeview));
	gtk_tree_selection_set_mode (poitypes_select, GTK_SELECTION_SINGLE);

	poidisplay_table = gtk_table_new (3, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (poidisplay_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (poidisplay_table), 5);
	gtk_table_attach (GTK_TABLE (poidisplay_table),
		poitheme_label, 0, 1, 0, 1,
		GTK_SHRINK, GTK_SHRINK, 0 ,0);
	gtk_table_attach (GTK_TABLE (poidisplay_table),
		poitheme_combo, 1, 2, 0, 1,
		GTK_EXPAND | GTK_FILL, GTK_SHRINK, 0 ,0);
	gtk_table_attach (GTK_TABLE (poidisplay_table),
		poifilter_label, 0, 1, 1, 2,
		GTK_SHRINK, GTK_SHRINK, 0 ,0);
	gtk_table_attach_defaults (GTK_TABLE (poidisplay_table),
		scrolledwindow_poitypes, 1, 2, 1, 2);	}


	wp_frame = gtk_frame_new (NULL);
	wp_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (wp_fr_lb), _("<b>Waypoints</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (wp_frame), wp_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (wp_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (wp_frame), wp_table);
	
	poisearch_frame = gtk_frame_new (NULL);
	poisearch_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (poisearch_fr_lb), _("<b>POI Search Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (poisearch_frame),
		poisearch_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (poisearch_frame),
		GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (poisearch_frame), poisearch_table);
	
	poidisplay_frame = gtk_frame_new (NULL);
	poidisplay_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (poidisplay_fr_lb), _("<b>POI Display</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (poidisplay_frame),
		poidisplay_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (poidisplay_frame),
		GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (poidisplay_frame), poidisplay_table);


	gtk_box_pack_start
		(GTK_BOX (poi_vbox), poidisplay_frame, TRUE, TRUE, 2);
	gtk_box_pack_start
		(GTK_BOX (poi_vbox), poisearch_frame, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (poi_vbox), wp_frame, FALSE, FALSE, 2);

	poi_label = gtk_label_new (_("POI"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), poi_vbox, poi_label);
}

/* ************************************************************************* */
static void
settings_wp (GtkWidget *notebook)
{
	GtkWidget *wp_vbox, *wp_label;
	GtkWidget *wpfile_label, *wpfile_bt;
	GtkWidget *wp_table, *wp_frame, *wp_fr_lb;
	GtkWidget *wpqs_table, *wpqs_frame, *wpqs_fr_lb;
	GtkTooltips *wp_tooltips;

	GtkWidget *wpfile_rb[30];
	GDir *d;
	const gchar *dat_name;
	gchar path[400];
	gchar *current_wpfile;
	gint dircount = 0;
	gint i;

	wp_vbox = gtk_vbox_new (FALSE, 2);
	wp_tooltips = gtk_tooltips_new ();

	/* waypoints file dialog */
	{
	wpfile_label = gtk_label_new (_("Waypoints File"));
	wpfile_bt = gtk_file_chooser_button_new
		(_("Select Waypoints File"), GTK_FILE_CHOOSER_ACTION_OPEN);
	if (!gtk_file_chooser_set_filename
		(GTK_FILE_CHOOSER (wpfile_bt), local_config.wp_file))
	{
		gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (wpfile_bt), local_config.dir_home);
	}
	gtk_tooltips_set_tip (GTK_TOOLTIPS (wp_tooltips), wpfile_bt,
		_("Choose the waypoints file to use!\nCurrently only files in "
		"GpsDrive's way.txt format are supported."), NULL);
	g_signal_connect (wpfile_bt, "selection-changed",
		GTK_SIGNAL_FUNC (setwpfile_cb), NULL);

	wp_table = gtk_table_new (1, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (wp_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wp_table), 5);
	gtk_table_attach_defaults (GTK_TABLE (wp_table),
		wpfile_label, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (wp_table),
		wpfile_bt, 1, 2, 0, 1);
	}

	/* waypoints quick select */
	{
	g_strlcpy (path, local_config.dir_home, sizeof (path));
	current_wpfile = g_strrstr (local_config.wp_file, "/") + 1;
	names = g_new (namesstruct, 102);
	d = g_dir_open(path, 0, NULL);
	if (NULL != d)
	{
		do
		{
			dat_name = g_dir_read_name(d);
			if (NULL != dat_name)
			{
				if (	0 == strncmp (dat_name, "way", 3)
					&& 0 == strncmp ((dat_name +
					(strlen (dat_name) - 4)),".txt", 4)
				   )
				{
					g_strlcpy ((names + dircount)->n,
						dat_name, 200);
					dircount++;
					if (dircount >= 100)
					{
						popup_warning (NULL,
						_("Don't use more than\n100"
						"waypoint(way*.txt) files!"));
						g_free (names);
					}
				}
			}
		}
		while (dat_name != NULL);

        g_dir_close(d);
	}

	wpqs_table = gtk_table_new (1 + (dircount - 1) / 2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (wpqs_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (wpqs_table), 5);

	for (i = 0; i < dircount; i++)
	{
		if (0 == i)
		{
			wpfile_rb[i] = gtk_radio_button_new_with_label
				(NULL, (names + i)->n);
		}
		else
		{
			wpfile_rb[i] = gtk_radio_button_new_with_label (
				gtk_radio_button_group (
				GTK_RADIO_BUTTON (wpfile_rb[0])),
				(names + i)->n);
		}
		g_signal_connect (wpfile_rb[i], "clicked",
			GTK_SIGNAL_FUNC (setwpfilequick_cb), (gpointer) i);
		gtk_table_attach_defaults (GTK_TABLE (wpqs_table),
			wpfile_rb[i], i % 2, i % 2 + 1, i / 2, i / 2 + 1);

		if (!(strcmp (current_wpfile, (names + i)->n)))
		{
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (wpfile_rb[i]), TRUE);
		}
	}
	
	}

	wp_frame = gtk_frame_new (NULL);
	wp_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (wp_fr_lb), _("<b>File Dialog Selection</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (wp_frame), wp_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (wp_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (wp_frame), wp_table);

	wpqs_frame = gtk_frame_new (NULL);
	wpqs_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (wpqs_fr_lb), _("<b>Quick Select File</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (wpqs_frame), wpqs_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (wpqs_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (wpqs_frame), wpqs_table);

	gtk_box_pack_start (GTK_BOX (wp_vbox), wpqs_frame, FALSE, FALSE, 2);
	//gtk_box_pack_start (GTK_BOX (wp_vbox), wp_frame, TRUE, FALSE, 2);

	wp_label = gtk_label_new (_("Waypoints"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), wp_vbox, wp_label);
}

/* ************************************************************************* */
static void
settings_friends (GtkWidget *notebook)
{
	GtkWidget *friends_vbox, *friends_label;
	GtkWidget *friendgen_table, *friendsrv_table;
	GtkWidget *friendgen_frame, *friendsrv_frame;
	GtkWidget *friendgen_fr_lb, *friendsrv_fr_lb;
	GtkTooltips *friends_tooltips;
	GtkWidget *friendenable_bt, *friendwarning_lb;
	GtkWidget *friendname_label, *friendname_entry;
	GtkWidget *friendmaxsec_label, *friendmaxsec_spin;
	GtkWidget *friendmaxsec_combo;
	GtkWidget *friendsrv_label, *friendsrv_entry;
	GtkWidget *friendsrvip_label, *friendsrvip_entry;
	GtkWidget *friendsrvip_bt;
	
	friends_tooltips = gtk_tooltips_new ();	
	friends_vbox = gtk_vbox_new (FALSE, 2);
	
	/* friends general settings */
	{
	friendgen_table = gtk_table_new (4, 4, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (friendgen_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (friendgen_table), 5);
	
	friendwarning_lb = gtk_label_new (_("If you enable this "
		"service, <span color=\"red\">everyone</span> using\n"
		"the same server can see your position!"));
	gtk_label_set_use_markup (GTK_LABEL (friendwarning_lb), TRUE);

	friendname_label = gtk_label_new (_("Your name"));
	friendname_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (friendname_entry), 40);
	gtk_entry_set_text
		(GTK_ENTRY (friendname_entry), local_config.friends_name);
	g_signal_connect (friendname_entry, "changed",
		GTK_SIGNAL_FUNC (setfriendname_cb), NULL);

	friendenable_bt = gtk_check_button_new_with_label
		(_("Enable friends service"));
	if (local_config.showfriends)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (friendenable_bt), TRUE);
	}
	g_signal_connect_swapped (friendenable_bt, "clicked",
		GTK_SIGNAL_FUNC (setshowfriends_cb), friendname_entry);

	friendmaxsec_label = gtk_label_new
		(_("Show only positions newer than"));
	friendmaxsec_spin = gtk_spin_button_new_with_range (0, 120, 0.5);
	friendmaxsec_combo = gtk_combo_box_new_text ();
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (friendmaxsec_combo), _("Days"));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (friendmaxsec_combo), _("Hours"));
	gtk_combo_box_append_text
		(GTK_COMBO_BOX (friendmaxsec_combo), _("Minutes"));
	if (local_config.friends_maxsecs > 120 * 3600)
	{
		gtk_combo_box_set_active
			(GTK_COMBO_BOX (friendmaxsec_combo), 0);
		gtk_spin_button_set_value
			(GTK_SPIN_BUTTON (friendmaxsec_spin),
			local_config.friends_maxsecs / 86400);
	}
	else if (local_config.friends_maxsecs > 120 * 60)
	{
		gtk_combo_box_set_active
			(GTK_COMBO_BOX (friendmaxsec_combo), 1);
		gtk_spin_button_set_value
			(GTK_SPIN_BUTTON (friendmaxsec_spin),
			local_config.friends_maxsecs / 3600);
	}
	else
	{
		gtk_combo_box_set_active
			(GTK_COMBO_BOX (friendmaxsec_combo), 2);
		gtk_spin_button_set_value
			(GTK_SPIN_BUTTON (friendmaxsec_spin),
			local_config.friends_maxsecs / 60);
	}
	g_signal_connect (friendmaxsec_spin, "changed",
		GTK_SIGNAL_FUNC (setfriendmaxsec_cb), friendmaxsec_combo);
	g_signal_connect (friendmaxsec_combo, "changed",
		GTK_SIGNAL_FUNC (setfriendmaxsecunit_cb), friendmaxsec_spin);

	gtk_table_attach_defaults (GTK_TABLE (friendgen_table),
		friendenable_bt, 0, 4, 0, 1);
	gtk_table_attach (GTK_TABLE (friendgen_table),
		friendname_label, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (friendgen_table),
		friendname_entry, 1, 4, 1, 2);
	gtk_table_attach (GTK_TABLE (friendgen_table),
		friendmaxsec_label, 0, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (friendgen_table),
		friendmaxsec_spin, 2, 3, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (friendgen_table),
		friendmaxsec_combo, 3, 4, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (friendgen_table),
		friendwarning_lb, 0, 4, 3, 4);
	
	gtk_tooltips_set_tip (friends_tooltips, friendenable_bt,
			_("Enable/disable use of friends service. You have to "
			"enter a username, don't use the default name!"),
			NULL);
	gtk_tooltips_set_tip (friends_tooltips, friendname_entry,
		_("Set here the name which will be shown near your position."),
		NULL);
	gtk_tooltips_set_tip (friends_tooltips, friendmaxsec_spin,
		_("Set here the max. age of friends positions that are "
		"displayed. Older positions are not shown."),
		NULL);
	}
	
	/* friends server settings */
	{
	friendsrv_table = gtk_table_new (2, 3, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (friendsrv_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (friendsrv_table), 5);
	
	friendsrv_label = gtk_label_new (_("Name"));
	friendsrv_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (friendsrv_entry), 255);
	gtk_entry_set_text
		(GTK_ENTRY (friendsrv_entry), local_config.friends_serverfqn);
	g_signal_connect (friendsrv_entry, "changed",
		GTK_SIGNAL_FUNC (setfriendsrv_cb), NULL);

	friendsrvip_label = gtk_label_new (_("IP"));
	friendsrvip_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (friendsrvip_entry), 20);
	gtk_entry_set_text
		(GTK_ENTRY (friendsrvip_entry), local_config.friends_serverip);
	g_signal_connect (friendsrvip_entry, "changed",
		GTK_SIGNAL_FUNC (setfriendsrvip_cb), NULL);

	friendsrvip_bt = gtk_button_new_with_label (_("Lookup"));
	g_signal_connect_swapped (friendsrvip_bt, "clicked",
		GTK_SIGNAL_FUNC (setfriendsrvip_lookup_cb), friendsrvip_entry);

	gtk_table_attach (GTK_TABLE (friendsrv_table),
		friendsrv_label, 0, 1, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (friendsrv_table),
		friendsrv_entry, 1, 3, 0, 1);
	gtk_table_attach (GTK_TABLE (friendsrv_table),
		friendsrvip_label, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (friendsrv_table),
		friendsrvip_entry, 1, 2, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (friendsrv_table),
		friendsrvip_bt, 2, 3, 1, 2);

	gtk_tooltips_set_tip (friends_tooltips, friendsrv_entry,
		_("Set here the fully qualified host name (i.e. friends."
		"gpsdrive.de) of the friends server to use, then press "
		"the \"Lookup\" button."), NULL);
	gtk_tooltips_set_tip (friends_tooltips, friendsrvip_bt,
		_("Press this button to resolve the friends server name."),
		NULL);
	gtk_tooltips_set_tip (friends_tooltips, friendsrvip_entry,
		_("Set here the IP adress (i.e. 127.0.0.1) if you don't set "
		"the hostname above"), NULL);
	}

	/* friends general frame */
	friendgen_frame = gtk_frame_new (NULL);
	friendgen_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (friendgen_fr_lb), _("<b>General</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (friendgen_frame), friendgen_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (friendgen_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (friendgen_frame), friendgen_table);
	
	/* friends server frame */
	friendsrv_frame = gtk_frame_new (NULL);
	friendsrv_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (friendsrv_fr_lb), _("<b>Server</b>"));
	gtk_frame_set_label_widget
		(GTK_FRAME (friendsrv_frame), friendsrv_fr_lb);
	gtk_frame_set_shadow_type
		(GTK_FRAME (friendsrv_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (friendsrv_frame), friendsrv_table);
	
	gtk_box_pack_start
		(GTK_BOX (friends_vbox), friendgen_frame, FALSE, FALSE, 2);
	gtk_box_pack_start
		(GTK_BOX (friends_vbox), friendsrv_frame, FALSE, FALSE, 2);
	
	friends_label = gtk_label_new (_("Friends"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), friends_vbox, friends_label);
}

/* ************************************************************************* */
static void
settings_speech (GtkWidget *notebook)
{
	GtkWidget *speech_vbox, *speech_label;
	GtkWidget *speech_table, *speech_frame, *speech_fr_lb;
	GtkWidget *sounddir_bt, *sounddist_bt;
	GtkWidget *soundspeed_bt, *soundgps_bt;
	GtkWidget *speechgen_table, *speechgen_frame, *speechgen_fr_lb;
	GtkWidget *speechenable_bt;
	GtkWidget *espeakspeed_label, *espeakspeed_scale;
	GtkWidget *espeakpitch_label, *espeakpitch_scale;
	GtkWidget *espeakvoice_label, *espeakvoice_combo;
	GtkTooltips *speech_tooltips;
	gint i=0, j=0;

	speech_tooltips = gtk_tooltips_new ();
	speech_vbox = gtk_vbox_new (FALSE, 2);

	/* speech general settings */
	{
	speechgen_table = gtk_table_new (4, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (speechgen_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (speechgen_table), 5);

	speechenable_bt = gtk_check_button_new_with_label (_("Enable speech output using espeak"));
	if (local_config.speech)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (speechenable_bt), TRUE);
	g_signal_connect (speechenable_bt, "clicked",
		G_CALLBACK (setspeechenable_cb), NULL);

	espeakspeed_label = gtk_label_new (_("Speed"));
	espeakspeed_scale = gtk_hscale_new_with_range (50, 300, 1.0);
	gtk_range_set_value (GTK_RANGE (espeakspeed_scale), local_config.speech_speed);
	g_signal_connect (espeakspeed_scale, "value-changed", G_CALLBACK (setspeechspeed_cb), NULL);

	espeakpitch_label = gtk_label_new (_("Pitch"));
	espeakpitch_scale = gtk_hscale_new_with_range (0, 99, 1.0);
	gtk_range_set_value (GTK_RANGE (espeakpitch_scale), local_config.speech_pitch);
	g_signal_connect (espeakpitch_scale, "value-changed", G_CALLBACK (setspeechpitch_cb), NULL);

	espeakvoice_label = gtk_label_new (_("Voice"));
	espeakvoice_combo = gtk_combo_box_new_text ();
	while (strcmp (espeak_voices[i], "-1"))
	{
		gtk_combo_box_append_text (GTK_COMBO_BOX (espeakvoice_combo), espeak_voices[i]);
		if (strcmp (espeak_voices[i+1], local_config.speech_voice) == 0)
			j = i/2;
		i += 2;
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (espeakvoice_combo), j);
	g_signal_connect (espeakvoice_combo, "changed", G_CALLBACK (setespeakvoice_cb), NULL);

	gtk_table_attach_defaults (GTK_TABLE (speechgen_table),
		speechenable_bt, 0, 4, 0, 1);
	gtk_table_attach (GTK_TABLE (speechgen_table),
		espeakvoice_label, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (speechgen_table),
		espeakvoice_combo, 1, 2, 1, 2);
	gtk_table_attach (GTK_TABLE (speechgen_table),
		espeakspeed_label, 0, 1, 2, 3, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (speechgen_table),
		espeakspeed_scale, 1, 2, 2, 3);
	gtk_table_attach (GTK_TABLE (speechgen_table),
		espeakpitch_label, 0, 1, 3, 4, GTK_SHRINK, GTK_SHRINK, 0, 0);
	gtk_table_attach_defaults (GTK_TABLE (speechgen_table),
		espeakpitch_scale, 1, 2, 3, 4);
	}


	/* Speech output settings */
	{
	speech_table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (speech_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (speech_table), 5);

	/* speech output settings
	 * set following sounds
	 * sound_direction ... say direction to target
	 * sound_distance  ... say distance to target
	 * sound_speed     ... say your current speed
	 * sound_gps       ... say GPS status
	 */
	sounddir_bt = gtk_check_button_new_with_label (_("Direction"));
	sounddist_bt = gtk_check_button_new_with_label (_("Distance"));
	soundspeed_bt = gtk_check_button_new_with_label (_("Speed"));
	soundgps_bt = gtk_check_button_new_with_label (_("GPS Status"));
	if (local_config.sound_direction)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (sounddir_bt), TRUE);
	}
	if (local_config.sound_distance)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (sounddist_bt), TRUE);
	}
	if (local_config.sound_speed)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (soundspeed_bt), TRUE);
	}
	if (local_config.sound_gps)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON (soundgps_bt), TRUE);
	}
	g_signal_connect (sounddir_bt, "clicked",
		GTK_SIGNAL_FUNC (settogglevalue_cb),
		&local_config.sound_direction);
	g_signal_connect (GTK_OBJECT (sounddist_bt), "clicked",
		GTK_SIGNAL_FUNC (settogglevalue_cb),
		&local_config.sound_distance);
	g_signal_connect (GTK_OBJECT (soundspeed_bt), "clicked",
		GTK_SIGNAL_FUNC (settogglevalue_cb),
		&local_config.sound_speed);
	g_signal_connect (GTK_OBJECT (soundgps_bt), "clicked",
		GTK_SIGNAL_FUNC (settogglevalue_cb),
		&local_config.sound_gps);
	gtk_table_attach_defaults
		(GTK_TABLE (speech_table), sounddir_bt, 0, 1, 0, 1);
	gtk_table_attach_defaults
		(GTK_TABLE (speech_table), sounddist_bt, 0, 1, 1, 2);
	gtk_table_attach_defaults
		(GTK_TABLE (speech_table), soundspeed_bt, 1, 2, 0, 1);
	gtk_table_attach_defaults
		(GTK_TABLE (speech_table), soundgps_bt, 1, 2, 1, 2);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (speech_tooltips), sounddir_bt,
		_("Switch on for speech output of the direction to the "
		"target"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (speech_tooltips), sounddist_bt,
		_("Switch on for speech output of the distance to the "
		"target"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (speech_tooltips), soundspeed_bt,
		_("Switch on for speech output of your current speed"), NULL);
	gtk_tooltips_set_tip (GTK_TOOLTIPS (speech_tooltips), soundgps_bt,
		_("Switch on for speech output of the status of your "
		"GPS signal"), NULL);
	}

	speechgen_frame = gtk_frame_new (NULL);
	speechgen_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (speechgen_fr_lb), _("<b>General</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (speechgen_frame), speechgen_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (speechgen_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (speechgen_frame), speechgen_table);

	speech_frame = gtk_frame_new (NULL);
	speech_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (speech_fr_lb), _("<b>Output Settings</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (speech_frame), speech_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (speech_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (speech_frame), speech_table);

	gtk_box_pack_start (GTK_BOX (speech_vbox), speechgen_frame, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (speech_vbox), speech_frame, FALSE, FALSE, 2);

	speech_label = gtk_label_new (_("Speech"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), speech_vbox, speech_label);
}

/* ************************************************************************* */
static void
settings_gps (GtkWidget *notebook)
{
	GtkWidget *gps_vbox, *gps_label;
	GtkWidget *general_frame, *general_fr_lb;
	GtkWidget *gps_reinit_bt, *gps_reinit_img;

	gps_vbox = gtk_vbox_new (FALSE, 2);

	/* button: reinit gps connection */
	gps_reinit_bt = gtk_button_new_with_label (_("Reinit GPS"));
	gps_reinit_img = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_BUTTON);
	gtk_button_set_image (GTK_BUTTON (gps_reinit_bt), gps_reinit_img);
	g_signal_connect (gps_reinit_bt, "clicked", G_CALLBACK (reinitgps_cb), NULL);

	general_frame = gtk_frame_new (NULL);
	general_fr_lb = gtk_label_new (NULL);
	gtk_label_set_markup
		(GTK_LABEL (general_fr_lb), _("<b>General</b>"));
	gtk_frame_set_label_widget (GTK_FRAME (general_frame), general_fr_lb);
	gtk_frame_set_shadow_type (GTK_FRAME (general_frame), GTK_SHADOW_NONE);
	gtk_container_add (GTK_CONTAINER (general_frame), gps_reinit_bt);

	gtk_box_pack_start (GTK_BOX (gps_vbox), general_frame, FALSE, FALSE, 2);

	gps_label = gtk_label_new (_("GPS"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), gps_vbox, gps_label);

}


/* ************************************************************************* */
/* TODO: fill with content
static void
settings_nautic (GtkWidget *notebook)
{
	GtkWidget *nautic_vbox, *nautic_label;
	
	
	nautic_vbox = gtk_vbox_new (FALSE, 2);
	
	nautic_label = gtk_label_new (_("Nautic"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK (notebook), nautic_vbox, nautic_label);

}
*/

/* *************************************************************************
 * main setup:
 * creates the settings window and calls all the other setup functions
 */
gint
settings_main_cb (GtkWidget *widget, guint datum)
{
	GtkWidget *settings_nb, *close_bt;

	settings_window = gtk_dialog_new ();
	gtk_window_set_title
		(GTK_WINDOW (settings_window), _("GpsDrive Settings"));
	gtk_window_set_position
		(GTK_WINDOW (settings_window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal
		(GTK_WINDOW (settings_window), TRUE);

	/* settings close button */
	close_bt = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	GTK_WIDGET_SET_FLAGS (close_bt, GTK_CAN_DEFAULT);
	gtk_window_set_default (GTK_WINDOW (settings_window), close_bt);
	g_signal_connect_swapped (close_bt, "clicked",
		GTK_SIGNAL_FUNC (settings_close_cb), settings_window);
	g_signal_connect (GTK_OBJECT (settings_window), "delete_event",
		GTK_SIGNAL_FUNC  (settings_close_cb), NULL);
	
	gtk_container_set_border_width
		(GTK_CONTAINER (settings_window), 2 * PADDING);

	/* Create a new notebook, place the position of the tabs */
	settings_nb = gtk_notebook_new ();
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (settings_nb), TRUE);
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (settings_nb), GTK_POS_TOP);
	gtk_notebook_set_show_border (GTK_NOTEBOOK (settings_nb), TRUE);
	gtk_notebook_popup_enable (GTK_NOTEBOOK (settings_nb));
	
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (settings_window)->vbox),
		settings_nb, TRUE, TRUE, 2 * PADDING);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (settings_window)->action_area),
		close_bt, TRUE, TRUE, 2);

	/* fill the tabs with the necessary dialogs */
	settings_general (settings_nb);
	if (local_config.use_database)
		settings_poi (settings_nb);
	else
		settings_wp (settings_nb);
	settings_friends (settings_nb);
	settings_nav (settings_nb);	
	settings_gui (settings_nb);
	settings_col (settings_nb);
	settings_trk (settings_nb);
	if (!havefestival)
		settings_speech (settings_nb);
	settings_gps (settings_nb);
	//settings_nautic (settings_nb);

	gtk_widget_show_all (settings_window);

	return TRUE;
}

