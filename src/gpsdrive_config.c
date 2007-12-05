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

#include <unistd.h>
#include <stdio.h>
#include <splash.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <gpsdrive.h>
#include <poi.h>
#include <wlan.h>
#include <time.h>
#include <speech_out.h>
#include <icons.h>
#include <gpsdrive_config.h>
#include <speech_strings.h>
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

extern gint displaymap_top, displaymap_map;
extern gint mydebug;
extern gint setdefaultpos;
extern gint usedgps;
extern gdouble milesconv;
extern gint satposmode, printoutsats;
extern gint lastnotebook;
#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char dbtable[MAXDBNAME], dbname[MAXDBNAME];
extern double dbdistance;
extern int dbusedist;
extern gint earthmate, zone;
extern long int maxfriendssecs;
extern int messagenumber;
extern int sockfd, showsid, storetz;
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
		fprintf (stderr,"Error saving config file %s ...\n", local_config.config_file);
		return;
	}

	fprintf (fp, "showwaypoints = ");
	if (local_config.showwaypoints)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");
	
	fprintf (fp, "travelmode = %d\n", local_config.travelmode);

	fprintf (fp, "showtrack = ");
	if (local_config.showtrack)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

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

	fprintf (fp, "showstreetmaps = ");
	if (displaymap_map)
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

	fprintf (fp, "savetrack = ");
	if (local_config.savetrack)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

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

	fprintf (fp, "usedgps = ");
	if (usedgps == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "mapdir = ");
	fprintf (fp, "%s\n", local_config.dir_maps);
	fprintf (fp, "trackdir = ");
	fprintf (fp, "%s\n", local_config.dir_tracks);
	fprintf (fp, "routedir = ");
	fprintf (fp, "%s\n", local_config.dir_routes);
	if (local_config.MapnikStatusInt > 1) { /* 2 = active, so store it as 1 */
		fprintf (fp, "mapnik = %d\n", 1);
	} else {
		fprintf (fp, "mapnik = %d\n", 0);
	}
			

	fprintf (fp, "simfollow = %d\n", local_config.simmode);

	fprintf (fp, "satposmode = ");
	if (satposmode == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "printoutsats = ");
	if (printoutsats == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "minsecmode = %d\n",local_config.coordmode);
	
	fprintf (fp, "posmarker = %d\n", local_config.posmarker);

	fprintf (fp, "nightmode = ");
	fprintf (fp, "%d\n", local_config.nightmode);

	fprintf (fp, "cpuload = %d\n", local_config.maxcpuload);

	fprintf (fp, "lastnotebook = ");
	fprintf (fp, "%d\n", lastnotebook);

	fprintf (fp, "dbhostname = %s\n", dbhost);
	fprintf (fp, "dbname = %s\n", dbname);
	fprintf (fp, "dbuser = %s\n", dbuser);
	fprintf (fp, "dbpass = %s\n", dbpass);
	fprintf (fp, "dbtable = %s\n", dbtable);
	fprintf (fp, "dbdistance = %.1f\n", dbdistance);
	fprintf (fp, "dbusedist = %d\n", dbusedist);
	fprintf (fp, "earthmate = %d\n", earthmate);

	fprintf (fp, "font_bigtext = %s\n", local_config.font_dashboard);
	fprintf (fp, "font_wplabel = %s\n", local_config.font_wplabel);
	fprintf (fp, "font_friends = %s\n", local_config.font_friends);

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
		fprintf (fp, "timezone = %d\n", zone);
	fprintf (fp, "dashboard_1 = %d\n", local_config.dashboard[1]);
	fprintf (fp, "dashboard_2 = %d\n", local_config.dashboard[2]);
	fprintf (fp, "dashboard_3 = %d\n", local_config.dashboard[3]);
	fprintf (fp, "dashboard_4 = %d\n", local_config.dashboard[4]);
	fprintf (fp, "bigcolor = %s\n", local_config.color_dashboard);
	fprintf (fp, "trackcolor = %s\n", local_config.color_track);
	fprintf (fp, "routecolor = %s\n", local_config.color_route);
	fprintf (fp, "friendscolor = %s\n", local_config.color_friends);
	fprintf (fp, "wplabelcolor = %s\n", local_config.color_wplabel);
	fprintf (fp, "messagenumber = %d\n", messagenumber);
	fprintf (fp, "showssid = %d\n", showsid);
	fprintf (fp, "speech = %d\n", local_config.speech);
	fprintf (fp, "speech_voice = %s\n", local_config.speech_voice);
	fprintf (fp, "speech_speed = %d\n", local_config.speech_speed);
	fprintf (fp, "speech_pitch = %d\n", local_config.speech_pitch);
	fprintf (fp, "sound_direction = %d\n", local_config.sound_direction);
	fprintf (fp, "sound_distance = %d\n", local_config.sound_distance);
	fprintf (fp, "sound_speed = %d\n", local_config.sound_speed);
	fprintf (fp, "sound_gps = %d\n", local_config.sound_gps);
	fprintf (fp, "icon_theme = %s\n", local_config.icon_theme);
	fprintf (fp, "poi_filter = %s\n", local_config.poi_filter);

	fprintf (fp, "draw_grid = %d\n", local_config.showgrid);
	fprintf (fp, "show_zoom = %d\n", local_config.showzoom);
	fprintf (fp, "show_scalebar = %d\n", local_config.showscalebar);
	fprintf (fp, "draw_poi = %d\n", local_config.showpoi);
	fprintf (fp, "draw_poilabel = %d\n", local_config.showpoilabel);
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

	while ((fgets (buf, 1000, fp)) > 0)
		{
		g_strlcpy (par1, "", sizeof (par1));
		g_strlcpy (par2, "", sizeof (par2));
		e = sscanf (buf, "%s = %[^\n]", par1, par2);

		if ( mydebug > 1 )
			fprintf ( stderr,"%d [%s] = [%s]\n", e, par1, par2);

		if (e == 2)
		{
			if ( (strcmp(par1, "showwaypoints")) == 0)
				local_config.showwaypoints = atoi (par2);
			else if ( (strcmp(par1, "showtrack")) == 0)
				local_config.showtrack = atoi (par2);
			else if ( (strcmp(par1, "travelmode")) == 0)
				local_config.travelmode = atoi (par2);
			else if ( (strcmp(par1, "mutespeechoutput")) == 0)
				local_config.mute = atoi (par2);
			else if ( (strcmp(par1, "showtopomaps")) == 0)
				displaymap_top = atoi (par2);
			else if ( (strcmp(par1, "showstreetmaps")) == 0)
				displaymap_map = atoi (par2);
			/*  To set the right sensitive flags bestmap_cb is called later */
			else if ( (strcmp(par1, "autobestmap")) == 0)
				local_config.autobestmap = atoi (par2);
			else if ( (strcmp(par1, "units")) == 0)
			{
				if ( (strcmp(par2, "miles")) == 0)
				{
					local_config.distmode = DIST_MILES;
					milesconv = KM2MILES;
				}
				else
				{
					if ( (strcmp(par2, "metric")) == 0)
					{
						local_config.distmode = DIST_METRIC;
						milesconv = 1.0;
					}
					else if ( (strcmp(par2, "nautic")) == 0)
					{
						local_config.distmode = DIST_NAUTIC;
						milesconv = KM2NAUTIC;
					}
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
			else if ( (strcmp(par1, "waypointfile")) == 0)
				g_strlcpy (local_config.wp_file, par2,
					sizeof (local_config.wp_file));
			else if ( (strcmp(par1, "usedgps")) == 0)
				usedgps = atoi (par2);
			else if ( (strcmp(par1, "mapdir")) == 0)
				g_strlcpy (local_config.dir_maps, par2,
					sizeof (local_config.dir_maps));
			else if ( (strcmp(par1, "trackdir")) == 0)
				g_strlcpy (local_config.dir_tracks, par2,
					sizeof (local_config.dir_tracks));
			else if ( (strcmp(par1, "routedir")) == 0)
				g_strlcpy (local_config.dir_routes, par2,
					sizeof (local_config.dir_routes));
			else if ( (strcmp(par1, "mapnik")) == 0)
				local_config.MapnikStatusInt = atoi (par2);
			else if ( (strcmp(par1, "simfollow")) == 0)
				local_config.simmode = atoi (par2);
			else if ( (strcmp(par1, "satposmode")) == 0)
				satposmode = atoi (par2);
			else if ( (strcmp(par1, "printoutsats")) == 0)
				printoutsats = atoi (par2);
			else if ( (strcmp(par1, "minsecmode")) == 0)
				local_config.coordmode = atoi (par2);
			else if ( (strcmp(par1, "posmarker")) == 0)
				local_config.posmarker = atoi (par2);
			else if ( (strcmp(par1, "nightmode")) == 0)
				local_config.nightmode = atoi (par2);
			else if ( (strcmp(par1, "cpuload")) == 0)
				local_config.maxcpuload = atoi (par2);
			else if ( (strcmp(par1, "lastnotebook")) == 0)
				lastnotebook = atoi (par2);
			else if ( (strcmp(par1, "dbhostname")) == 0)
			{
				g_strlcpy (dbhost, par2, sizeof (dbhost));
				g_strstrip (dbhost);
			}
			else if ( (strcmp(par1, "dbname")) == 0)
				g_strlcpy (dbname, par2, sizeof (dbname));
			else if ( (strcmp(par1, "dbuser")) == 0)
				g_strlcpy (dbuser, par2, sizeof (dbuser));
			else if ( (strcmp(par1, "dbpass")) == 0)
				g_strlcpy (dbpass, par2, sizeof (dbpass));
			else if ( (strcmp(par1, "dbtable")) == 0)
			{
				if ( (strcmp(par2, "waypoints")) == 0)
					g_strlcpy (dbtable, "poi", sizeof (dbtable));
				else
					g_strlcpy (dbtable, par2, sizeof (dbtable));
			}
			else if ( (strcmp(par1, "dbname")) == 0)
				g_strlcpy (dbname, par2, sizeof (dbname));
			else if ( (strcmp(par1, "dbdistance")) == 0)
				dbdistance = g_strtod (par2, 0);
			else if ( (strcmp(par1, "dbusedist")) == 0)
				dbusedist = atoi (par2);
			else if ( (strcmp(par1, "earthmate")) == 0)
				earthmate = atoi (par2);

			else if ( (strcmp(par1, "font_bigtext")) == 0)
				g_strlcpy (local_config.font_dashboard, par2,
				sizeof (local_config.font_dashboard));
			else if ( (strcmp(par1, "font_wplabel")) == 0)
				g_strlcpy (local_config.font_wplabel, par2,
				sizeof (local_config.font_wplabel));
			else if ( (strcmp(par1, "font_friends")) == 0)
				g_strlcpy (local_config.font_friends, par2,
				sizeof (local_config.font_friends));

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
				zone = atoi (par2);
			else if ( (strcmp(par1, "bigcolor")) == 0)
				g_strlcpy (local_config.color_dashboard, par2,
				sizeof (local_config.color_dashboard));
			else if ( (strcmp(par1, "trackcolor")) == 0)
				g_strlcpy (local_config.color_track, par2,
				sizeof (local_config.color_track));
			else if ( (strcmp(par1, "routecolor")) == 0)
				g_strlcpy (local_config.color_route, par2,
				sizeof (local_config.color_route));
			else if ( (strcmp(par1, "friendscolor")) == 0)
				g_strlcpy (local_config.color_friends, par2,
				sizeof (local_config.color_friends));
			else if ( (strcmp(par1, "wplabelcolor")) == 0)
				g_strlcpy (local_config.color_wplabel, par2,
				sizeof (local_config.color_wplabel));
			else if ( (strcmp(par1, "messagenumber")) == 0)
				messagenumber = atoi (par2);
			else if ( (strcmp(par1, "showssid")) == 0)
				showsid = atoi (par2);
			else if ( (strcmp(par1, "speech")) == 0)
				local_config.speech = atoi (par2);
			else if ( (strcmp(par1, "speech_voice")) == 0)
				g_strlcpy (local_config.speech_voice, par2,
				sizeof (local_config.speech_voice));
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
			else if ( (strcmp(par1, "draw_grid")) == 0)
				local_config.showgrid = atoi (par2);
			else if ( (strcmp(par1, "show_zoom")) == 0)
				local_config.showzoom = atoi (par2);
			else if ( (strcmp(par1, "show_scalebar")) == 0)
				local_config.showscalebar = atoi (par2);
			else if ( (strcmp(par1, "draw_poi")) == 0)
				local_config.showpoi = atoi (par2);
			else if ( (strcmp(par1, "draw_poilabel")) == 0)
				local_config.showpoilabel = atoi (par2);
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

	if (local_config.guimode == GUI_PDA)
	{
		g_strlcpy (local_config.font_dashboard, "Sans bold 12",
			sizeof (local_config.font_dashboard));
		g_strlcpy (local_config.font_wplabel, "Sans 8", sizeof
			(local_config.font_wplabel));
	}
}

/* init configuration with defined default values */
void
config_init ()
{
	gchar *hd;
	local_config.travelmode = TRAVEL_CAR;
	local_config.distmode = DIST_METRIC;
	local_config.altmode = ALT_METERS;
	local_config.coordmode = LATLON_DEGDEC;
	local_config.guimode = GUI_DESKTOP;
	local_config.simmode = SIM_OFF;
	local_config.enableapm = FALSE;
	local_config.dashboard[0] = 0;
	local_config.dashboard[1] = DASH_DIST;
	local_config.dashboard[2] = DASH_SPEED;
	local_config.dashboard[3] = DASH_ALT;
	local_config.dashboard[4] = DASH_TIME;
	local_config.MapnikStatusInt = 0;
	local_config.nightmode = NIGHT_OFF;
	local_config.posmarker = 0;
	local_config.maxcpuload = 40;
	local_config.showgrid = FALSE;
	local_config.showshadow = FALSE;
	local_config.showzoom = TRUE;
	local_config.showscalebar = TRUE;
	local_config.showwaypoints = TRUE;
	local_config.showpoi = TRUE;
	local_config.showpoilabel = FALSE;
	local_config.showtooltips = TRUE;
	local_config.showaddwpbutton = FALSE;
	local_config.showfriends = FALSE;
	local_config.scale_wanted = 100000;
	local_config.autobestmap = 1;

	/* set speech output */
	g_strlcpy (local_config.speech_voice,
		"default", sizeof (local_config.speech_voice));
	local_config.speech = FALSE;
	local_config.speech_speed = 120;
	local_config.speech_pitch = 50;
	local_config.sound_direction = TRUE;
	local_config.sound_distance = TRUE;
	local_config.sound_speed = TRUE;
	local_config.sound_gps = TRUE;


	/* set POI related stuff */
	local_config.poi_results_max = 200;
	local_config.poi_searchradius = 10.0;
	g_strlcpy (local_config.icon_theme,
		"square.big", sizeof (local_config.icon_theme));
	
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
		"#0000ff", sizeof (local_config.color_dashboard));
	g_strlcpy (local_config.font_dashboard,
		"Sans bold 26", sizeof (local_config.font_dashboard));
	g_strlcpy (local_config.font_wplabel,
		"Sans 11", sizeof (local_config.font_wplabel));
	g_strlcpy (local_config.font_friends,
		"Sans bold 11", sizeof (local_config.font_friends));

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
	g_snprintf(local_config.mapnik_xml_file, sizeof(local_config.mapnik_xml_file),
		"%s%s", local_config.dir_home, "osm.xml");
	g_snprintf(local_config.config_file, sizeof(local_config.config_file),
		"%s%s", local_config.dir_home, "gpsdriverc");
	
	/* kismet default values */
	g_strlcpy(local_config.kismet_servername, "localhost", sizeof(local_config.kismet_servername));
	local_config.kismet_serverport = 2501;
}
