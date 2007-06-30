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
#include "mysql/mysql.h"
#include "gpsproto.h"

#include <string.h>
#include <stdlib.h>

void writeconfig ();
void readconfig ();
void config_init ();
typedef struct
{
	gint travelmode;
	gint distmode;
	gint altmode;
	gint coordmode;
	gint nightmode;
	gint guimode;
	gint maxcpuload;
	gchar wp_file[200];
	gchar dir_home[500];
	gchar dir_maps[500];
	gchar icon_theme[500];
	guint poi_results_max;
	gdouble poi_searchradius;
	int MapnikStatusInt; /* 0 = disable, 1 = enable, 2 = active */
	gboolean simmode;
	gboolean showgrid;
	gboolean showshadow;
	gboolean showwaypoints;
	gboolean showtooltips;
	gboolean showaddwpbutton;
	gboolean showfriends;
	gboolean sound_direction;
	gboolean sound_distance;
	gboolean sound_speed;
	gboolean sound_gps;
	glong friends_maxsecs;
	gchar friends_name[40];
	gchar friends_id[40];
	gchar friends_serverfqn[255];
	gchar friends_serverip[20];
	gchar color_track[40];
	gchar color_route[40];
	gchar color_friends[40];
	gchar color_wplabel[40];
	gchar color_bigdisplay[40];
	gchar font_friends[100];
	gchar font_wplabel[100];
	gchar font_bigdisplay[100];
}
local_gpsdrive_config;

extern local_gpsdrive_config local_config;

#endif /* GPSDRIVE_CONFIG_H */
