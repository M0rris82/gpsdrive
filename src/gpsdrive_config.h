/*******************************************************************

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
#ifndef GPSDRIVE_CONFIG_H
#define GPSDRIVE_CONFIG_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <gmodule.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#include "gpsproto.h"

#include <string.h>
#include <stdlib.h>

void writeconfig ();
void readconfig ();
void config_init ();
typedef struct
{
	gchar config_file[500];
	gint travelmode;
	gint distmode;
	gdouble distfactor;
	gint altmode;
	gint coordmode;
	gint nightmode;
	gint guimode;
	gboolean showbatt;
	gboolean showtemp;
	gboolean showway;
	gint maxcpuload;
	gint posmarker;
	gchar wp_file[500];
	gchar mapnik_postgis_dbname[100];
	gchar mapnik_xml_file[500];
	gchar mapnik_xml_template[500];
	gchar mapnik_input_path[500];
	gchar mapnik_font_path[500];
	gchar geoinfo_file[500];
	gchar osm_dbfile[500];
	gchar dir_home[500];
	gchar dir_maps[500];
	gchar dir_routes[500];
	gchar dir_tracks[500];
	gchar dir_mbrola[500];
	gchar icon_theme[500];
	gchar poi_filter[2000];
	gchar poi_label[1000];
	gchar quickpoint_type[160];
	gchar quickpoint_text[200];
	guint quickpoint_num;
	guint quickpoint_mode;
	guint poi_results_max;
	gdouble poi_searchradius;
	gdouble poi_dbdistance;
	int MapnikStatusInt; /* 0 = disable, 1 = enable, 2 = active */
	gboolean nav_enabled;
	gint nav_type;
	gchar nav_moduledir[500];
	gchar nav_module[100];
	gboolean mapnik_caching;
	gint simmode;
	gboolean use_database;
	gboolean rotating_compass;
	gboolean showdestline;
	gboolean showmaptype;
	gboolean showgrid;
	gboolean showshadow;
	gboolean showzoom;
	gboolean showscalebar;
	gboolean showwaypoints;
	gint showpoi;
	gboolean showwlan;
	gboolean showtooltips;
	gboolean showaddwpbutton;
	gboolean showfriends;
	gboolean showminimap;
	gboolean showtrack;
	gboolean savetrack;
	gboolean show_controls;
	gboolean show_dashboard;
	gboolean show_compass;
	gboolean showbutton_trackrestart;
	gboolean showbutton_trackclear;
	gboolean showbutton_explore;
	gboolean showbutton_addwpt;
	gboolean showbutton_zoom;
	gboolean showbutton_scaler;
	gboolean showbutton_mute;
	gboolean showbutton_find;
	gboolean showbutton_route;
	gboolean showbutton_map;
	gboolean track_autoclean;
	gboolean track_autointerval;
	gboolean track_interval;
	gboolean gpsfix_style;
	gint buttonsmode;
	gchar track_autoprefix[50];
	gboolean autobestmap;
	gint scale_wanted;
	gint mapsource_type;
	gint mapsource_scale;
	gboolean speech;
	gchar speech_module[20];
	gchar speech_voice[20];
	gint speech_speed;
	gint speech_pitch;
	gboolean sound_direction;
	gboolean sound_distance;
	gboolean sound_speed;
	gboolean sound_gps;
	gboolean mute;
	glong friends_maxsecs;
	gchar friends_name[40];
	gchar friends_id[40];
	gchar friends_serverfqn[255];
	gchar friends_serverip[20];
	gint style_track;
	gint style_route;
	gchar color_track[40];
	gchar color_route[40];
	gchar color_friends[40];
	gchar color_wplabel[40];
	gchar color_dashboard[40];
	gchar color_map_day[40];
	gchar color_map_night[40];
	gchar font_friends[100];
	gchar font_wplabel[100];
	gchar font_dashboard[100];
	gchar font_tmp_friends[100];
	gchar font_tmp_wplabel[100];
	gchar font_tmp_dashboard[100];
	gint dashboard[5];
	gdouble normalnull;
	gchar kismet_servername[500];
	gint kismet_serverport;
	gchar screenshot_dir[1000];
	gboolean embeddable_gui;	/* use GtkPlug instead of GtkWindow for main_window */
	gchar gpsd_server[200];
	gchar gpsd_port[6];
}
local_gpsdrive_config;

extern local_gpsdrive_config local_config;

#endif /* GPSDRIVE_CONFIG_H */
