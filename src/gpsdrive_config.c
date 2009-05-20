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

#include "config.h"
#include <unistd.h>
#include <stdio.h>
#include <splash.h>
#include <string.h>
#include <stdlib.h>
#include <gpsdrive.h>
#include <poi.h>
#include <time.h>
#include <icons.h>
#include <gpsdrive_config.h>
#include "main_gui.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern gint max_display_map;
extern map_dir_struct *display_map;

extern gint displaymap_top;
extern gint mydebug;
extern gint setdefaultpos;
extern long int maxfriendssecs;
extern int messagenumber;
extern int storetz;
extern coordinate_struct coords;
extern currentstatus_struct current;

local_gpsdrive_config local_config;


/* write the configurationfile */
void
writeconfig ()
{
	FILE *fp;
	gchar str[40];
	gint i;

	if ( mydebug > 0 )
		printf ("Write config %s\n", local_config.config_file);



	fp = fopen (local_config.config_file, "w");
	if (fp == NULL)
	{
		fprintf (stderr,"Error saving config file %s ...\n",
			 local_config.config_file);
		return;
	}

	fprintf (fp, "showwaypoints = ");
	if (local_config.showwaypoints)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");
	
	fprintf (fp, "travelmode = %d\n", local_config.travelmode);

	fprintf (fp, "navigationtype = %d\n", local_config.nav_type);

	fprintf (fp, "showtrack = ");
	if (local_config.showtrack)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "showdestline = %d\n",
		local_config.showdestline);

	fprintf (fp, "trackpointsinterval = %d\n",
		local_config.track_interval);

	fprintf (fp, "trackautosaveinterval = %d\n",
		local_config.track_autointerval);

	fprintf (fp, "trackautosaveprefix = %s\n",
		local_config.track_autoprefix);

	fprintf (fp, "trackautoclean = %d\n",
		local_config.track_autoclean);

	fprintf (fp, "showdashboard = %d\n",
		local_config.show_dashboard);

	fprintf (fp, "showbuttonmenu = %d\n",
		local_config.show_controls);
	fprintf (fp, "showbutton_zoom = %d\n",
		local_config.showbutton_zoom);
	fprintf (fp, "showbutton_scaler = %d\n",
		local_config.showbutton_scaler);
	fprintf (fp, "showbutton_mute = %d\n",
		local_config.showbutton_mute);
	fprintf (fp, "showbutton_find = %d\n",
		local_config.showbutton_find);
	fprintf (fp, "showbutton_route = %d\n",
		local_config.showbutton_route);
	fprintf (fp, "showbutton_map = %d\n",
		local_config.showbutton_map);
	fprintf (fp, "showbutton_trackrestart = %d\n",
		local_config.showbutton_trackrestart);
	fprintf (fp, "showbutton_trackclear = %d\n",
		local_config.showbutton_trackclear);

	fprintf (fp, "mutespeechoutput = ");
	if (local_config.mute)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "showtopomaps = ");
	if (displaymap_top)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "autobestmap = ");
	if (local_config.autobestmap)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "units = ");
	if (local_config.distmode == DIST_MILES)
		fprintf (fp, "miles\n");
	else
	{
		if (local_config.distmode == DIST_NAUTIC)
			fprintf (fp, "nautic\n");
		else
			fprintf (fp, "metric\n");
	}
	fprintf (fp, "altitude = ");
	if (local_config.altmode == ALT_FEET)
		fprintf (fp, "feet\n");
	else
	{
		if (local_config.altmode == ALT_YARDS)
			fprintf (fp, "yards\n");
		else
			fprintf (fp, "meters\n");
	}

	fprintf (fp, "savetrack = ");
	if (local_config.savetrack)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "mapsource_type = %d\n", local_config.mapsource_type);
	fprintf (fp, "mapsource_scale = %d\n", local_config.mapsource_scale);
	fprintf (fp, "scalewanted = %d\n", local_config.scale_wanted);

	g_snprintf (str, sizeof (str), "%.6f", coords.current_lon);
	g_strdelimit (str, ",", '.');
	fprintf (fp, "lastlong = %s\n", str);

	g_snprintf (str, sizeof (str), "%.6f", coords.current_lat);
	g_strdelimit (str, ",", '.');
	fprintf (fp, "lastlat = %s\n", str);

	fprintf (fp, "shadow = ");
	if (local_config.showshadow)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "waypointfile = ");
	fprintf (fp, "%s\n", local_config.wp_file);

	fprintf (fp, "geoinfofile = ");
	fprintf (fp, "%s\n", local_config.geoinfo_file);

	fprintf (fp, "osmdbfile = ");
	fprintf (fp, "%s\n", local_config.osm_dbfile);

	fprintf (fp, "mapdir = ");
	fprintf (fp, "%s\n", local_config.dir_maps);
	fprintf (fp, "trackdir = ");
	fprintf (fp, "%s\n", local_config.dir_tracks);
	fprintf (fp, "routedir = ");
	fprintf (fp, "%s\n", local_config.dir_routes);
	fprintf (fp, "navigation_modulesdir = ");
	fprintf (fp, "%s\n", local_config.nav_moduledir);

	fprintf (fp, "navigation_module = ");
	fprintf (fp, "%s\n", local_config.nav_module);

	if (local_config.MapnikStatusInt > 1) { /* 2 = active, so store it as 1 */
		fprintf (fp, "mapnik = %d\n", 1);
	} else {
		fprintf (fp, "mapnik = %d\n", 0);
	}
	fprintf (fp, "mapnik_caching = ");
	if (local_config.mapnik_caching == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");
	fprintf (fp, "mapnik_xml_file = ");
	fprintf (fp, "%s\n", local_config.mapnik_xml_file);
	fprintf (fp, "mapnik_xml_template = ");
	fprintf (fp, "%s\n", local_config.mapnik_xml_template);
	fprintf (fp, "mapnik_input_path = ");
	fprintf (fp, "%s\n", local_config.mapnik_input_path);
	fprintf (fp, "mapnik_font_path = ");
	fprintf (fp, "%s\n", local_config.mapnik_font_path);

	fprintf (fp, "simfollow = %d\n", local_config.simmode);

	fprintf (fp, "minsecmode = %d\n",local_config.coordmode);

	fprintf (fp, "posmarker = %d\n", local_config.posmarker);

	fprintf (fp, "nightmode = ");
	fprintf (fp, "%d\n", local_config.nightmode);

	fprintf (fp, "cpuload = %d\n", local_config.maxcpuload);

	fprintf (fp, "use_database = ");
	fprintf (fp, "%d\n", local_config.use_database);
	fprintf (fp, "dbdistance = %.1f\n", local_config.poi_dbdistance);

	if (local_config.guimode == GUI_PDA)
	{
		fprintf (fp, "font_bigtext = %s\n", local_config.font_tmp_dashboard);
		fprintf (fp, "font_wplabel = %s\n", local_config.font_tmp_wplabel);
		fprintf (fp, "font_friends = %s\n", local_config.font_tmp_friends);
		fprintf (fp, "font_pda_bigtext = %s\n", local_config.font_dashboard);
		fprintf (fp, "font_pda_wplabel = %s\n", local_config.font_wplabel);
		fprintf (fp, "font_pda_friends = %s\n", local_config.font_friends);
	}
	else
	{
		fprintf (fp, "font_bigtext = %s\n", local_config.font_dashboard);
		fprintf (fp, "font_wplabel = %s\n", local_config.font_wplabel);
		fprintf (fp, "font_friends = %s\n", local_config.font_friends);
		fprintf (fp, "font_pda_bigtext = %s\n", local_config.font_tmp_dashboard);
		fprintf (fp, "font_pda_wplabel = %s\n", local_config.font_tmp_wplabel);
		fprintf (fp, "font_pda_friends = %s\n", local_config.font_tmp_friends);
	}

	fprintf (fp, "friendsserverip = %s\n", local_config.friends_serverip);
	fprintf (fp, "friendsserverfqn = %s\n", local_config.friends_serverfqn);
	fprintf (fp, "friendsname = %s\n", local_config.friends_name);
	fprintf (fp, "friendsidstring = %s\n", local_config.friends_id);
	fprintf (fp, "usefriendsserver = %d\n", local_config.showfriends);
	fprintf (fp, "maxfriendssecs = %ld\n", local_config.friends_maxsecs);
	fprintf (fp, "poi_results_max = %d\n", local_config.poi_results_max);
	fprintf (fp, "poi_searchradius = %0.1f\n",
		local_config.poi_searchradius);
	fprintf (fp, "storetz = %d\n", storetz);
	if (storetz)
		fprintf (fp, "timezone = %d\n", current.timezone);
	fprintf (fp, "dashboard_1 = %d\n", local_config.dashboard[1]);
	fprintf (fp, "dashboard_2 = %d\n", local_config.dashboard[2]);
	fprintf (fp, "dashboard_3 = %d\n", local_config.dashboard[3]);
	fprintf (fp, "dashboard_4 = %d\n", local_config.dashboard[4]);
	fprintf (fp, "bigcolor = %s\n", local_config.color_dashboard);
	fprintf (fp, "trackcolor = %s\n", local_config.color_track);
	fprintf (fp, "trackstyle = %d\n", local_config.style_track);
	fprintf (fp, "routecolor = %s\n", local_config.color_route);
	fprintf (fp, "routestyle = %d\n", local_config.style_route);
	fprintf (fp, "friendscolor = %s\n", local_config.color_friends);
	fprintf (fp, "wplabelcolor = %s\n", local_config.color_wplabel);
	fprintf (fp, "map_daycolor = %s\n", local_config.color_map_day);
	fprintf (fp, "map_nightcolor = %s\n", local_config.color_map_night);
	fprintf (fp, "messagenumber = %d\n", messagenumber);
	fprintf (fp, "speech = %d\n", local_config.speech);
	fprintf (fp, "speech_module = %s\n", local_config.speech_module);
	fprintf (fp, "speech_voice = %s\n", local_config.speech_voice);
	fprintf (fp, "speech_mbroladir = %s\n", local_config.dir_mbrola);
	fprintf (fp, "speech_speed = %d\n", local_config.speech_speed);
	fprintf (fp, "speech_pitch = %d\n", local_config.speech_pitch);
	fprintf (fp, "sound_direction = %d\n", local_config.sound_direction);
	fprintf (fp, "sound_distance = %d\n", local_config.sound_distance);
	fprintf (fp, "sound_speed = %d\n", local_config.sound_speed);
	fprintf (fp, "sound_gps = %d\n", local_config.sound_gps);
	fprintf (fp, "icon_theme = %s\n", local_config.icon_theme);
	fprintf (fp, "poi_filter = %s\n", local_config.poi_filter);
	fprintf (fp, "poi_label = %s\n", local_config.poi_label);
	fprintf (fp, "quickpoint_mode = %d\n", local_config.quickpoint_mode);
	fprintf (fp, "quickpoint_number = %d\n", local_config.quickpoint_num);
	fprintf (fp, "quickpoint_text = %s\n", local_config.quickpoint_text);
	fprintf (fp, "quickpoint_type = %s\n", local_config.quickpoint_type);
	fprintf (fp, "draw_grid = %d\n", local_config.showgrid);
	fprintf (fp, "show_maptype = %d\n", local_config.showmaptype);
	fprintf (fp, "show_zoom = %d\n", local_config.showzoom);
	fprintf (fp, "show_scalebar = %d\n", local_config.showscalebar);
	fprintf (fp, "show_battery = %d\n", local_config.showbatt);
	fprintf (fp, "show_temperature = %d\n", local_config.showtemp);
	fprintf (fp, "rotating_compass = %d\n", local_config.rotating_compass);
	fprintf (fp, "show_wayinfo = %d\n", local_config.showway);
	fprintf (fp, "draw_poi = %d\n", local_config.showpoi);
	fprintf (fp, "draw_wlan = %d\n", local_config.showwlan);
	for ( i = 0; i < max_display_map; i++)
	    {
		fprintf (fp, "display_map_%s = %d\n",
			 display_map[i].name,
			 display_map[i].to_be_displayed);
	    }
	


	fclose (fp);
	current.needtosave = FALSE;
}

/* read the configurationfile */
void
readconfig ()
{
	FILE *fp;
	gchar par1[40], par2[1000], buf[1000];
	gint e;

	// open Config File
	fp = fopen (local_config.config_file, "r");
	if (fp == NULL)
		return;

	// mydebug is not set, because getopt was not run yet
	// So you won't see this Debug Output
	if ( mydebug > 0 )
		fprintf (stderr,"reading config file %s ...\n", local_config.config_file);

	while (fgets (buf, 1000, fp))
		{
		g_strlcpy (par1, "", sizeof (par1));
		g_strlcpy (par2, "", sizeof (par2));
		e = sscanf (buf, "%s = %[^\n]", par1, par2);

		if ( mydebug > 1 )
			fprintf ( stderr,"readconfig(): %d [%s] = [%s]\n", e, par1, par2);

		if (e == 2)
		{
			if ( (strcmp(par1, "showwaypoints")) == 0)
				local_config.showwaypoints = atoi (par2);
			else if ( (strcmp(par1, "showtrack")) == 0)
				local_config.showtrack = atoi (par2);
			else if ( (strcmp(par1, "trackpointsinterval")) == 0)
				local_config.track_interval = atoi (par2);
			else if ( (strcmp(par1, "trackautosaveinterval")) == 0)
				local_config.track_autointerval = atoi (par2);
			else if ( (strcmp(par1, "trackautosaveprefix")) == 0)
				g_strlcpy (local_config.track_autoprefix, par2,
				sizeof (local_config.track_autoprefix));
			else if ( (strcmp(par1, "trackautoclean")) == 0)
				local_config.track_autoclean = atoi (par2);
			else if ( (strcmp(par1, "showdashboard")) == 0)
				local_config.show_dashboard = atoi (par2);
			else if ( (strcmp(par1, "showbuttonmenu")) == 0)
				local_config.show_controls = atoi (par2);
			else if ( (strcmp(par1, "showbutton_zoom")) == 0)
				local_config.showbutton_zoom = atoi (par2);
			else if ( (strcmp(par1, "showbutton_scaler")) == 0)
				local_config.showbutton_scaler = atoi (par2);
			else if ( (strcmp(par1, "showbutton_mute")) == 0)
				local_config.showbutton_mute = atoi (par2);
			else if ( (strcmp(par1, "showbutton_find")) == 0)
				local_config.showbutton_find = atoi (par2);
			else if ( (strcmp(par1, "showbutton_route")) == 0)
				local_config.showbutton_route = atoi (par2);
			else if ( (strcmp(par1, "showbutton_map")) == 0)
				local_config.showbutton_map = atoi (par2);
			else if ( (strcmp(par1, "showbutton_trackrestart")) == 0)
				local_config.showbutton_trackrestart = atoi (par2);
			else if ( (strcmp(par1, "showbutton_trackclear")) == 0)
				local_config.showbutton_trackclear = atoi (par2);
			else if ( (strcmp(par1, "travelmode")) == 0)
				local_config.travelmode = atoi (par2);
			else if ( (strcmp(par1, "navigationtype")) == 0)
				local_config.nav_type = atoi (par2);
			else if ( (strcmp(par1, "mutespeechoutput")) == 0)
				local_config.mute = atoi (par2);
			else if ( (strcmp(par1, "showtopomaps")) == 0)
				displaymap_top = atoi (par2);
			/*  To set the right sensitive flags bestmap_cb is called later */
			else if ( (strcmp(par1, "autobestmap")) == 0)
				local_config.autobestmap = atoi (par2);
			else if ( (strcmp(par1, "units")) == 0)
			{
				if ( (strcmp(par2, "miles")) == 0)
				{
					local_config.distmode = DIST_MILES;
					local_config.distfactor = KM2MILES;
				}
				else
				{
					if ( (strcmp(par2, "metric")) == 0)
					{
						local_config.distmode = DIST_METRIC;
						local_config.distfactor = 1.0;
					}
					else if ( (strcmp(par2, "nautic")) == 0)
					{
						local_config.distmode = DIST_NAUTIC;
						local_config.distfactor = KM2NAUTIC;
					}
				}
			}
			else if ( (strcmp(par1, "altitude")) == 0)
			{
				if ( (strcmp(par2, "feet")) == 0)
				{
					local_config.altmode = ALT_FEET;
				}
				else if ( (strcmp(par2, "meters")) == 0)
				{
					local_config.altmode = ALT_METERS;
				}
				else if ( (strcmp(par2, "yards")) == 0)
				{
					local_config.altmode = ALT_YARDS;
				}
			}
			else if ( (strcmp(par1, "dashboard_1")) == 0)
				local_config.dashboard[1] = atoi (par2);
			else if ( (strcmp(par1, "dashboard_2")) == 0)
				local_config.dashboard[2] = atoi (par2);
			else if ( (strcmp(par1, "dashboard_3")) == 0)
				local_config.dashboard[3] = atoi (par2);
			else if ( (strcmp(par1, "dashboard_4")) == 0)
				local_config.dashboard[4] = atoi (par2);
			else if ( (strcmp(par1, "savetrack")) == 0)
				local_config.savetrack = atoi (par2);
			else if ( (strcmp(par1, "mapsource_type")) == 0)
				local_config.mapsource_type = atoi (par2);
			else if ( (strcmp(par1, "mapsource_scale")) == 0)
				local_config.mapsource_scale = atoi (par2);
			else if ( (strcmp(par1, "scalewanted")) == 0)
				local_config.scale_wanted = atoi (par2);
			else if ( (strcmp(par1, "lastlong")) == 0)
				coordinate_string2gdouble(par2,
					&coords.current_lon);
			else if ( (strcmp(par1, "lastlat")) == 0)
				coordinate_string2gdouble(par2,
					&coords.current_lat);
			/*
			else if ( (strcmp(par1, "setdefaultpos")) == 0)            
		       setdefaultpos = atoi (par2); 
			*/
			else if ( (strcmp(par1, "shadow")) == 0)
				local_config.showshadow = atoi (par2);
			else if ( (strcmp(par1, "showdestline")) == 0)
				local_config.showdestline = atoi (par2);
			else if ( (strcmp(par1, "rotating_compass")) == 0)
				local_config.rotating_compass = atoi (par2);
			else if ( (strcmp(par1, "waypointfile")) == 0)
				g_strlcpy (local_config.wp_file, par2,
					sizeof (local_config.wp_file));
			else if ( (strcmp(par1, "geoinfofile")) == 0)
				g_strlcpy (local_config.geoinfo_file, par2,
					sizeof (local_config.geoinfo_file));
			else if ( (strcmp(par1, "osmdbfile")) == 0)
				g_strlcpy (local_config.osm_dbfile, par2,
					sizeof (local_config.osm_dbfile));
			else if ( (strcmp(par1, "mapdir")) == 0)
				g_strlcpy (local_config.dir_maps, par2,
					sizeof (local_config.dir_maps));
			else if ( (strcmp(par1, "trackdir")) == 0)
				g_strlcpy (local_config.dir_tracks, par2,
					sizeof (local_config.dir_tracks));
			else if ( (strcmp(par1, "routedir")) == 0)
				g_strlcpy (local_config.dir_routes, par2,
					sizeof (local_config.dir_routes));
			else if ( (strcmp(par1, "navigation_modulesdir")) == 0)
				g_strlcpy (local_config.nav_moduledir, par2,
					sizeof (local_config.nav_moduledir));
			else if ( (strcmp(par1, "navigation_module")) == 0)
				g_strlcpy (local_config.nav_module, par2,
					sizeof (local_config.nav_module));

			else if ( (strcmp(par1, "mapnik")) == 0)
				local_config.MapnikStatusInt = atoi (par2);
			else if ( (strcmp(par1, "mapnik_caching")) == 0)
				local_config.mapnik_caching = atoi (par2);
			else if ( (strcmp(par1, "mapnik_xml_file")) == 0)
				g_strlcpy (local_config.mapnik_xml_file, par2,
					sizeof (local_config.mapnik_xml_file));
			else if ( (strcmp(par1, "mapnik_xml_template")) == 0)
				g_strlcpy (local_config.mapnik_xml_template, par2,
					sizeof (local_config.mapnik_xml_template));
			else if ( (strcmp(par1, "mapnik_input_path")) == 0)
				g_strlcpy (local_config.mapnik_input_path, par2,
					sizeof (local_config.mapnik_input_path));
			else if ( (strcmp(par1, "mapnik_font_path")) == 0)
				g_strlcpy (local_config.mapnik_font_path, par2,
					sizeof (local_config.mapnik_font_path));
			else if ( (strcmp(par1, "simfollow")) == 0)
				local_config.simmode = atoi (par2);
			else if ( (strcmp(par1, "minsecmode")) == 0)
				local_config.coordmode = atoi (par2);
			else if ( (strcmp(par1, "posmarker")) == 0)
				local_config.posmarker = atoi (par2);
			else if ( (strcmp(par1, "nightmode")) == 0)
				local_config.nightmode = atoi (par2);
			else if ( (strcmp(par1, "cpuload")) == 0)
				local_config.maxcpuload = atoi (par2);
			else if ( (strcmp(par1, "use_database")) == 0)
				local_config.use_database = atoi (par2);
			else if ( (strcmp(par1, "dbdistance")) == 0)
				local_config.poi_dbdistance = g_strtod (par2, 0);
			else if ( (strcmp(par1, "show_apm")) == 0)
			     {
				local_config.showbatt = atoi (par2);
				local_config.showtemp = atoi (par2);
			     }
			else if ( (strcmp(par1, "show_battery")) == 0)
				local_config.showbatt = atoi (par2);
			else if ( (strcmp(par1, "show_temperature")) == 0)
				local_config.showtemp = atoi (par2);
			else if ( (strcmp(par1, "show_wayinfo")) == 0)
				local_config.showway = atoi (par2);

			else if ( (strcmp(par1, "font_bigtext")) == 0)
			{
				if (local_config.guimode != GUI_PDA)
					g_strlcpy (local_config.font_dashboard, par2,
					sizeof (local_config.font_dashboard));
				else
					g_strlcpy (local_config.font_tmp_dashboard, par2,
					sizeof (local_config.font_tmp_dashboard));
			}
			else if ( (strcmp(par1, "font_wplabel")) == 0)
			{
				if (local_config.guimode != GUI_PDA)
					g_strlcpy (local_config.font_wplabel, par2,
					sizeof (local_config.font_wplabel));
				else
					g_strlcpy (local_config.font_tmp_wplabel, par2,
					sizeof (local_config.font_tmp_wplabel));
			}
			else if ( (strcmp(par1, "font_friends")) == 0)
			{
				if (local_config.guimode != GUI_PDA)
					g_strlcpy (local_config.font_friends, par2,
					sizeof (local_config.font_friends));
				else
					g_strlcpy (local_config.font_tmp_friends, par2,
					sizeof (local_config.font_tmp_friends));
			}
			else if ( (strcmp(par1, "font_pda_bigtext")) == 0)
			{
				if (local_config.guimode == GUI_PDA)
					g_strlcpy (local_config.font_dashboard, par2,
					sizeof (local_config.font_dashboard));
				else
					g_strlcpy (local_config.font_tmp_dashboard, par2,
					sizeof (local_config.font_tmp_dashboard));
			}
			else if ( (strcmp(par1, "font_pda_wplabel")) == 0)
			{
				if (local_config.guimode == GUI_PDA)
					g_strlcpy (local_config.font_wplabel, par2,
					sizeof (local_config.font_wplabel));
				else
					g_strlcpy (local_config.font_tmp_wplabel, par2,
					sizeof (local_config.font_tmp_wplabel));
			}
			else if ( (strcmp(par1, "font_pda_friends")) == 0)
			{
				if (local_config.guimode == GUI_PDA)
					g_strlcpy (local_config.font_friends, par2,
					sizeof (local_config.font_friends));
				else
					g_strlcpy (local_config.font_tmp_friends, par2,
					sizeof (local_config.font_tmp_friends));
			}
			else if ( (strcmp(par1, "friendsserverip")) == 0)
				g_strlcpy (local_config.friends_serverip, par2,
				sizeof (local_config.friends_serverip));
			else if ( (strcmp(par1, "friendsserverfqn")) == 0)
				g_strlcpy (local_config.friends_serverfqn, par2,
				sizeof (local_config.friends_serverfqn));
			else if ( (strcmp(par1, "friendsname")) == 0)
				g_strlcpy (local_config.friends_name, par2,
				sizeof (local_config.friends_name));
			else if ( (strcmp(par1, "friendsidstring")) == 0)
				g_strlcpy (local_config.friends_id, par2, sizeof
				(local_config.friends_id));
			else if ( (strcmp(par1, "usefriendsserver")) == 0)
				local_config.showfriends = atoi (par2);
			else if ( (strcmp(par1, "maxfriendssecs")) == 0)
				local_config.friends_maxsecs = atoi (par2);
			else if ( (strcmp(par1, "poi_results_max")) == 0)
				local_config.poi_results_max = atoi (par2);
			else if ( (strcmp(par1, "poi_searchradius")) == 0)
				local_config.poi_searchradius =
					g_strtod (par2, NULL);
			else if ( (strcmp(par1, "storetz")) == 0)
				storetz = atoi (par2);
			else if ( storetz && (strcmp(par1, "timezone")) == 0)
				current.timezone = atoi (par2);
			else if ( (strcmp(par1, "bigcolor")) == 0)
				g_strlcpy (local_config.color_dashboard, par2,
				sizeof (local_config.color_dashboard));
			else if ( (strcmp(par1, "trackcolor")) == 0)
				g_strlcpy (local_config.color_track, par2,
				sizeof (local_config.color_track));
			else if ( (strcmp(par1, "trackstyle")) == 0)
				local_config.style_track = atoi (par2);
			else if ( (strcmp(par1, "routecolor")) == 0)
				g_strlcpy (local_config.color_route, par2,
				sizeof (local_config.color_route));
			else if ( (strcmp(par1, "routestyle")) == 0)
				local_config.style_route = atoi (par2);
			else if ( (strcmp(par1, "friendscolor")) == 0)
				g_strlcpy (local_config.color_friends, par2,
				sizeof (local_config.color_friends));
			else if ( (strcmp(par1, "wplabelcolor")) == 0)
				g_strlcpy (local_config.color_wplabel, par2,
				sizeof (local_config.color_wplabel));
			else if ( (strcmp(par1, "map_daycolor")) == 0)
				g_strlcpy (local_config.color_map_day, par2,
				sizeof (local_config.color_map_day));
			else if ( (strcmp(par1, "map_nightcolor")) == 0)
				g_strlcpy (local_config.color_map_night, par2,
				sizeof (local_config.color_map_night));
			else if ( (strcmp(par1, "messagenumber")) == 0)
				messagenumber = atoi (par2);
			else if ( (strcmp(par1, "speech")) == 0)
				local_config.speech = atoi (par2);
			else if ( (strcmp(par1, "speech_module")) == 0)
				g_strlcpy (local_config.speech_module, par2,
				sizeof (local_config.speech_module));
			else if ( (strcmp(par1, "speech_voice")) == 0)
				g_strlcpy (local_config.speech_voice, par2,
				sizeof (local_config.speech_voice));
			else if ( (strcmp(par1, "speech_mbroladir")) == 0)
				g_strlcpy (local_config.dir_mbrola, par2,
					sizeof (local_config.dir_mbrola));
			else if ( (strcmp(par1, "speech_speed")) == 0)
				local_config.speech_speed = atoi (par2);
			else if ( (strcmp(par1, "speech_pitch")) == 0)
				local_config.speech_pitch = atoi (par2);
			else if ( (strcmp(par1, "sound_direction")) == 0)
				local_config.sound_direction = atoi (par2);
			else if ( (strcmp(par1, "sound_distance")) == 0)
				local_config.sound_distance = atoi (par2);
			else if ( (strcmp(par1, "sound_speed")) == 0)
				local_config.sound_speed = atoi (par2);
			else if ( (strcmp(par1, "sound_gps")) == 0)
				local_config.sound_gps = atoi (par2);
			else if ( (strcmp(par1, "icon_theme")) == 0)
				g_strlcpy (local_config.icon_theme, par2,
					sizeof (local_config.icon_theme));
			else if ( (strcmp(par1, "poi_filter")) == 0)
				g_strlcpy (local_config.poi_filter, par2,
					sizeof (local_config.poi_filter));
			else if ( (strcmp(par1, "poi_label")) == 0)
				g_strlcpy (local_config.poi_label, par2,
					sizeof (local_config.poi_label));
			else if ( (strcmp(par1, "quickpoint_mode")) == 0)
				local_config.quickpoint_mode = atoi (par2);
			else if ( (strcmp(par1, "quickpoint_number")) == 0)
				local_config.quickpoint_num = atoi (par2);
			else if ( (strcmp(par1, "quickpoint_text")) == 0)
				g_strlcpy (local_config.quickpoint_text, par2,
					sizeof (local_config.quickpoint_text));
			else if ( (strcmp(par1, "quickpoint_type")) == 0)
				g_strlcpy (local_config.quickpoint_type, par2,
					sizeof (local_config.quickpoint_type));
			else if ( (strcmp(par1, "draw_grid")) == 0)
				local_config.showgrid = atoi (par2);
			else if ( (strcmp(par1, "show_maptype")) == 0)
				local_config.showmaptype = atoi (par2);
			else if ( (strcmp(par1, "show_zoom")) == 0)
				local_config.showzoom = atoi (par2);
			else if ( (strcmp(par1, "show_scalebar")) == 0)
				local_config.showscalebar = atoi (par2);
			else if ( (strcmp(par1, "draw_poi")) == 0)
				local_config.showpoi = atoi (par2);
			else if ( (strcmp(par1, "draw_wlan")) == 0)
				local_config.showwlan = atoi (par2);
			else if ( ! strncmp(par1, "display_map_",12) )
			{
				// printf ("display_map: %s %s\n",par1,par2);
				max_display_map += 1;
				display_map = g_renew(map_dir_struct, display_map, max_display_map);
				g_strlcpy (display_map[max_display_map-1].name,
				(par1+12),sizeof (display_map[max_display_map-1].name));
				display_map[max_display_map-1].to_be_displayed = atoi (par2);
			}
			else
				fprintf (stderr, "ERROR: Unknown Config Parameter '%s=%s'\n",par1,par2);
		}	/* if e==2 */
	}

	if ( mydebug > 1 )
		fprintf ( stderr,"\nreading config file finished\n");
	fclose (fp);
}

/* init configuration with defined default values */
void
config_init ()
{
	gchar *hd;
	local_config.travelmode = TRAVEL_CAR;
	local_config.distmode = DIST_METRIC;
	local_config.distfactor = 1.0;
	local_config.altmode = ALT_METERS;
	local_config.coordmode = LATLON_DEGDEC;
	local_config.guimode = GUI_DESKTOP;
	local_config.simmode = SIM_OFF;
	local_config.showbatt = FALSE;
	local_config.showtemp = FALSE;
	local_config.showway = FALSE;
	local_config.dashboard[0] = 0;
	local_config.dashboard[1] = DASH_DIST;
	local_config.dashboard[2] = DASH_SPEED;
	local_config.dashboard[3] = DASH_ALT;
	local_config.dashboard[4] = DASH_TIME;
	local_config.MapnikStatusInt = 0;
	local_config.mapnik_caching = TRUE;
	local_config.nightmode = NIGHT_OFF;
	local_config.posmarker = 0;
	local_config.maxcpuload = 40;
	local_config.use_database = TRUE;
	local_config.showdestline = FALSE;
	local_config.showmaptype = FALSE;
	local_config.showgrid = FALSE;
	local_config.showshadow = FALSE;
	local_config.showzoom = TRUE;
	local_config.showscalebar = TRUE;
	local_config.showwaypoints = TRUE;
	local_config.showpoi = POIDRAW_ALL;
	local_config.showtooltips = TRUE;
	local_config.showaddwpbutton = FALSE;
	local_config.showfriends = FALSE;
	local_config.scale_wanted = 100000;
	local_config.track_autointerval = 1;
	local_config.track_interval = 1;
	local_config.mapsource_type = 0;
	local_config.mapsource_scale = 0;
	local_config.autobestmap = 1;
	local_config.rotating_compass = TRUE;
	local_config.track_autoclean = FALSE;
	local_config.style_track = 3;
	local_config.style_route = 1;

	/* set speech output */
	g_strlcpy (local_config.speech_module,
		"espeak", sizeof (local_config.speech_module));
	g_strlcpy (local_config.speech_voice,
		"default", sizeof (local_config.speech_voice));
	g_strlcpy (local_config.dir_mbrola,
		"/usr/share/mbrola/", sizeof (local_config.dir_mbrola));
	local_config.speech = FALSE;
	local_config.speech_speed = 0;
	local_config.speech_pitch = 0;
	local_config.sound_direction = TRUE;
	local_config.sound_distance = TRUE;
	local_config.sound_speed = TRUE;
	local_config.sound_gps = TRUE;

	/* set POI related stuff */
	local_config.poi_results_max = 200;
	local_config.poi_searchradius = 10.0;
	local_config.poi_dbdistance = 2000.0;
	g_strlcpy (local_config.icon_theme,
		"square.big", sizeof (local_config.icon_theme));
	g_snprintf (local_config.geoinfo_file, sizeof (local_config.geoinfo_file),
		"%s%s", DATADIR, "/icons/map-icons/geoinfo.db");
	g_snprintf (local_config.osm_dbfile, sizeof (local_config.osm_dbfile),
		"%s%s", DATADIR, "/gpsdrive/osm.db");
	g_strlcpy (local_config.quickpoint_type, "waypoint.pin.green", sizeof (local_config.quickpoint_type));
	g_strlcpy (local_config.quickpoint_text, "WP_", sizeof (local_config.quickpoint_text));
	local_config.quickpoint_mode = 1;
	local_config.quickpoint_num = 0;

	/* set friends stuff */
	local_config.friends_maxsecs = 1209600;
	g_strlcpy (local_config.friends_name, "",
		sizeof (local_config.friends_name));
	g_strlcpy (local_config.friends_id,
		"XXX", sizeof (local_config.friends_id));
	g_strlcpy (local_config.friends_serverfqn,
		"friendsd.gpsdrive.de", sizeof (local_config.friends_serverfqn));
	g_strlcpy (local_config.friends_serverip,
		"127.0.0.1", sizeof (local_config.friends_serverip));

	/* set colors and fonts */
	g_strlcpy (local_config.color_track,
		"#0000ff", sizeof (local_config.color_track));
	g_strlcpy (local_config.color_route,
		"#00ff00", sizeof (local_config.color_route));
	g_strlcpy (local_config.color_friends,
		"#ffa500", sizeof (local_config.color_friends));
	g_strlcpy (local_config.color_wplabel,
		"#00ffff", sizeof (local_config.color_wplabel));
	g_strlcpy (local_config.color_dashboard,
		"#000080", sizeof (local_config.color_dashboard));
	g_strlcpy (local_config.color_map_day,
		"#ffebcc", sizeof (local_config.color_map_day));
	g_strlcpy (local_config.color_map_night,
		"#330033", sizeof (local_config.color_map_night));
	g_strlcpy (local_config.font_dashboard,
		"Sans bold 16", sizeof (local_config.font_tmp_dashboard));
	g_strlcpy (local_config.font_wplabel,
		"Sans 8", sizeof (local_config.font_tmp_wplabel));
	g_strlcpy (local_config.font_friends,
		"Sans bold 10", sizeof (local_config.font_tmp_friends));
	g_strlcpy (local_config.font_dashboard,
		"Sans bold 26", sizeof (local_config.font_dashboard));
	g_strlcpy (local_config.font_wplabel,
		"Sans 11", sizeof (local_config.font_wplabel));
	g_strlcpy (local_config.font_friends,
		"Sans bold 11", sizeof (local_config.font_friends));
	g_strlcpy (local_config.font_tmp_dashboard,
		"Sans bold 16", sizeof (local_config.font_tmp_dashboard));
	g_strlcpy (local_config.font_tmp_wplabel,
		"Sans 8", sizeof (local_config.font_tmp_wplabel));
	g_strlcpy (local_config.font_tmp_friends,
		"Sans bold 10", sizeof (local_config.font_tmp_friends));

	/* buttons displayed */
	local_config.show_controls = TRUE;
	local_config.show_dashboard = TRUE;
	local_config.showbutton_trackclear = TRUE;
	local_config.showbutton_trackrestart = TRUE;
	local_config.showbutton_zoom = TRUE;
	local_config.showbutton_scaler = TRUE;
	local_config.showbutton_mute = TRUE;
	local_config.showbutton_find = TRUE;
	local_config.showbutton_route = TRUE;
	local_config.showbutton_map = TRUE;

	/* set files and directories (~/.gpsdrive) */
	hd = (gchar *) g_get_home_dir ();
	g_strlcpy (local_config.dir_home, hd, sizeof (local_config.dir_home));
	g_strlcat (local_config.dir_home, "/.gpsdrive/",
		sizeof (local_config.dir_home));
	g_snprintf (local_config.dir_maps, sizeof (local_config.dir_maps),
		"%s%s",local_config.dir_home,"maps/");
	g_snprintf (local_config.dir_routes, sizeof (local_config.dir_routes),
		"%s%s",local_config.dir_home,"routes/");
	g_snprintf (local_config.dir_tracks, sizeof (local_config.dir_tracks),
		"%s%s",local_config.dir_home,"tracks/");
	g_snprintf (local_config.wp_file, sizeof (local_config.wp_file),
		"%s%s", local_config.dir_home, "way.txt");
	g_snprintf(local_config.config_file, sizeof(local_config.config_file),
		"%s%s", local_config.dir_home, "gpsdriverc");
	g_snprintf(local_config.track_autoprefix, sizeof(local_config.track_autoprefix),
		"%s", "autosave");

	/* mapnik default values */
	g_snprintf(local_config.mapnik_xml_file, sizeof(local_config.mapnik_xml_file),
		"%s%s", local_config.dir_home, "osm.xml");
	g_snprintf(local_config.mapnik_xml_template, sizeof(local_config.mapnik_xml_template),
		"%s%s", DATADIR, "/gpsdrive/osm-template.xml");
	g_snprintf(local_config.mapnik_input_path, sizeof(local_config.mapnik_input_path),
		"%s", "/usr/lib/mapnik/0.5/input/");
	g_snprintf(local_config.mapnik_font_path, sizeof(local_config.mapnik_font_path),
		"%s", "/usr/share/fonts/truetype/ttf-dejavu/");
	
	/* kismet default values */
	g_strlcpy(local_config.kismet_servername, "127.0.0.1", sizeof(local_config.kismet_servername));
	local_config.kismet_serverport = 2501;

	/* navigation settings */
	local_config.nav_enabled = FALSE;
	local_config.nav_type = 0;
	g_snprintf (local_config.nav_moduledir, sizeof (local_config.nav_moduledir),
		"%s%s", LIBDIR, "/gpsdrive");
	g_strlcpy(local_config.nav_module, "nav_dummy", sizeof(local_config.nav_module));


	/* run a normal GUI by default */
	local_config.embeddable_gui = FALSE;

	/* set gpsd default values */
	g_strlcpy (local_config.gpsd_server, "127.0.0.1", sizeof (local_config.gpsd_server));
	g_strlcpy (local_config.gpsd_port, "2947", sizeof (local_config.gpsd_port));
}
