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
#include <streets.h>
#include <time.h>
#include <speech_out.h>
#include <icons.h>
#include <gpsdrive_config.h>
#include <speech_strings.h>

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

extern GtkWidget *mainwindow;
GtkWidget *splash_window;
extern gchar local_config_homedir[500], local_config_mapdir[500];
extern gint trackflag, muteflag, displaymap_top, displaymap_map;
extern gint scaleprefered_not_bestmap, milesflag, nauticflag, metricflag, wp_from_sql;
extern gint mydebug, scalewanted, savetrack, defaultserver;
extern gchar serialdev[80];
extern gdouble current_lon, current_lat, old_lon, old_lat;
extern gint setdefaultpos, shadow, etch;
extern gint do_draw_grid, streets_draw, tracks_draw, poi_draw, wlan_draw, testgarmin;
extern gint needtosave, usedgps, simfollow;
extern gchar activewpfile[200];
extern gdouble milesconv;
extern gint satposmode, printoutsats, minsecmode, nightmode, cpuload;
/* extern gint flymode, vfr,disdevwarn; */
extern gint flymode, vfr, disdevwarn;
extern gint pdamode;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu,
	int_padding, lastnotebook;
#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char dbtable[MAXDBNAME], dbname[MAXDBNAME];
extern char dbwherestring[5000];
extern double dbdistance;
extern int dbusedist, needreminder;
extern gint earthmate, havefriends, zone;
extern gchar font_s_text[100], font_s_verysmalltext[100], font_s_smalltext[100], font_s_bigtext[100], font_s_wplabel[100];
extern char friendsserverip[20], friendsname[40], friendsidstring[40],
	friendsserverfqn[255];
extern long int maxfriendssecs;
extern gchar bluecolor[40], trackcolor[40], friendscolor[40];
extern int messagenumber;
extern int sockfd, serialspeed, disableserial, showsid, storetz;
extern int sound_direction, sound_distance, sound_speed, sound_gps;

#define KM2MILES 0.62137119
#define PADDING int_padding

local_gpsdrive_config local_config;


/*  write the configurationfile ~/.gpsdrive/gpsdriverc */
void
writeconfig ()
{
	FILE *fp;
	gchar fname[220], str[40];
	gint i;

	g_strlcpy (fname, local_config_homedir, sizeof (fname));
	g_strlcat (fname, "gpsdriverc", sizeof (fname));

	if ( mydebug > 0 )
		printf ("Write config %s\n", fname);



	fp = fopen (fname, "w");
	if (fp == NULL)
	{
		perror (_("Error saving config file ~/.gpsdrive/gpsdriverc"));
		return;
	}

	fprintf (fp, "showwaypoints = ");
	if (local_config.showwaypoints)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "showtrack = ");
	if (trackflag)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "mutespeechoutput = ");
	if (muteflag)
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
	if (!scaleprefered_not_bestmap)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "units = ");
	if (milesflag)
		fprintf (fp, "miles\n");
	else
	{
		if (metricflag)
			fprintf (fp, "metric\n");
		else
			fprintf (fp, "nautic\n");
	}

	fprintf (fp, "savetrack = ");
	if (savetrack)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "scalewanted = %d\n", scalewanted);

	fprintf (fp, "serialdevice = %s\n", serialdev);

	g_snprintf (str, sizeof (str), "%.6f", current_lon);
	g_strdelimit (str, ",", '.');
	fprintf (fp, "lastlong = %s\n", str);

	g_snprintf (str, sizeof (str), "%.6f", current_lat);
	g_strdelimit (str, ",", '.');
	fprintf (fp, "lastlat = %s\n", str);

	fprintf (fp, "shadow = ");
	if (shadow)
		fprintf (fp, "1\n");
	else
		fprintf (fp, "0\n");

	fprintf (fp, "defaultserver = ");
	if (defaultserver == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "testgarminmode = ");
	if (testgarmin == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "waypointfile = ");
	fprintf (fp, "%s\n", activewpfile);

	fprintf (fp, "usedgps = ");
	if (usedgps == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "mapdir = ");
	fprintf (fp, "%s\n", local_config_mapdir);

	fprintf (fp, "simfollow = ");
	if (simfollow == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

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

	fprintf (fp, "minsecmode = %d\n",minsecmode);

	fprintf (fp, "nightmode = ");
	fprintf (fp, "%d\n", nightmode);

	fprintf (fp, "cpuload = %d\n", cpuload);

	fprintf (fp, "flymode = ");
	if (flymode == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "vfr = ");
	fprintf (fp, "%d\n", vfr);

	fprintf (fp, "disdevwarn = ");
	if (disdevwarn == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");

	fprintf (fp, "wp_from_sql = ");
	if (wp_from_sql == 0)
		fprintf (fp, "0\n");
	else
		fprintf (fp, "1\n");
	fprintf (fp, "lastnotebook = ");
	fprintf (fp, "%d\n", lastnotebook);

	fprintf (fp, "dbhostname = %s\n", dbhost);
	fprintf (fp, "dbname = %s\n", dbname);
	fprintf (fp, "dbuser = %s\n", dbuser);
	fprintf (fp, "dbpass = %s\n", dbpass);
	fprintf (fp, "dbtable = %s\n", dbtable);
	fprintf (fp, "dbdistance = %.1f\n", dbdistance);
	fprintf (fp, "dbusedist = %d\n", dbusedist);
	fprintf (fp, "dbwherestring = %s\n", dbwherestring);
	fprintf (fp, "earthmate = %d\n", earthmate);

	fprintf (fp, "font_text = %s\n", font_s_text);
	fprintf (fp, "font_verysmalltext = %s\n", font_s_verysmalltext);
	fprintf (fp, "font_smalltext = %s\n", font_s_smalltext);
	fprintf (fp, "font_bigtext = %s\n", font_s_bigtext);
	fprintf (fp, "font_wplabel = %s\n", font_s_wplabel);

	fprintf (fp, "friendsserverip = %s\n", friendsserverip);
	fprintf (fp, "friendsserverfqn = %s\n", friendsserverfqn);
	fprintf (fp, "friendsname = %s\n", friendsname);
	fprintf (fp, "friendsidstring = %s\n", friendsidstring);
	fprintf (fp, "usefriendsserver = %d\n", havefriends);
	fprintf (fp, "maxfriendssecs = %ld\n", maxfriendssecs);
	fprintf (fp, "reminder = %s\n", VERSION);
	fprintf (fp, "storetz = %d\n", storetz);
	if (storetz)
		fprintf (fp, "timezone = %d\n", zone);
	fprintf (fp, "etch = %d\n", etch);
	fprintf (fp, "bigcolor = %s\n", bluecolor);
	fprintf (fp, "trackcolor = %s\n", trackcolor);
	fprintf (fp, "friendscolor = %s\n", friendscolor);
	fprintf (fp, "messagenumber = %d\n", messagenumber);
	fprintf (fp, "serialspeed = %d\n", serialspeed);
	fprintf (fp, "disableserial = %d\n", disableserial);
	fprintf (fp, "showssid = %d\n", showsid);
	fprintf (fp, "sound_direction = %d\n", sound_direction);
	fprintf (fp, "sound_distance = %d\n", sound_distance);
	fprintf (fp, "sound_speed = %d\n", sound_speed);
	fprintf (fp, "sound_gps = %d\n", sound_gps);
	fprintf (fp, "icon_theme = %s\n", local_config.icon_theme);

	fprintf (fp, "draw_grid = %d\n", do_draw_grid);
	fprintf (fp, "draw_streets = %d\n", streets_draw);
	fprintf (fp, "draw_poi = %d\n", poi_draw);
	fprintf (fp, "draw_wlan = %d\n", wlan_draw);
	fprintf (fp, "draw_tracks = %d\n", tracks_draw);
	for ( i = 0; i < max_display_map; i++)
	    {
		fprintf (fp, "display_map_%s = %d\n",
			 display_map[i].name,
			 display_map[i].to_be_displayed);
	    }
	


	fclose (fp);
	needtosave = FALSE;
}

/*  read the configurationfile ~/.gpsdrive/gpsdriverc */
void
readconfig ()
{
    FILE *fp;
    gchar fname[220], par1[40], par2[1000], buf[1000];
    gint e;

    /* Defaults */
    local_config.showwaypoints = TRUE;
    g_strlcpy (local_config.icon_theme, "square.big", sizeof (local_config.icon_theme));

    // open Config File
    g_strlcpy (fname, local_config_homedir, sizeof (fname));
    g_strlcat (fname, "gpsdriverc", sizeof (fname));
    fp = fopen (fname, "r");
    if (fp == NULL)
	return;

    // mydebug is not set, because getopt was not run yet
    // So you won't see this Debug Output
    if ( mydebug > 0 )
	fprintf (stderr,"reading config file %s ...\n",fname);

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
			trackflag = atoi (par2);
		    else if ( (strcmp(par1, "mutespeechoutput")) == 0)
			muteflag = atoi (par2);
		    else if ( (strcmp(par1, "showtopomaps")) == 0)
			displaymap_top = atoi (par2);
		    else if ( (strcmp(par1, "showstreetmaps")) == 0)
			displaymap_map = atoi (par2);
		    /*  To set the right sensitive flags bestmap_cb is called later */
		    else if ( (strcmp(par1, "autobestmap")) == 0)
			scaleprefered_not_bestmap = !(atoi (par2));
		    else if ( (strcmp(par1, "units")) == 0)
			{
			    milesflag = metricflag = nauticflag = FALSE;
			    if ( (strcmp(par2, "miles")) == 0)
				{
				    milesflag = TRUE;
				    milesconv = KM2MILES;
				}
			    else
				{
				    if ( (strcmp(par2, "metric")) == 0)
					{
					    metricflag = TRUE;
					    milesconv = 1.0;
					}
				    else if ( (strcmp(par2, "nautic")) ==
					     0)
					{
					    nauticflag = TRUE;
					    milesconv = KM2NAUTIC;
					}
				}
			}
		    else if ( (strcmp(par1, "savetrack")) == 0)
			savetrack = atoi (par2);
		    else if ( (strcmp(par1, "scalewanted")) == 0)
			scalewanted = atoi (par2);
		    else if ( (strcmp(par1, "serialdevice")) == 0)
			g_strlcpy (serialdev, par2, sizeof (serialdev));
		    else if ( (strcmp(par1, "lastlong")) == 0)
			coordinate_string2gdouble(par2, &current_lon);
		    else if ( (strcmp(par1, "lastlat")) == 0)
			coordinate_string2gdouble(par2, &current_lat);
		    /* 
		       else if ( (strcmp(par1, "setdefaultpos")) == 0)            
		       setdefaultpos = atoi (par2); 
		    */
		    else if ( (strcmp(par1, "shadow")) == 0)
			shadow = atoi (par2);
		    else if ( (strcmp(par1, "defaultserver")) == 0)
			defaultserver = atoi (par2);
		    else if ( (strcmp(par1, "waypointfile")) == 0)
			g_strlcpy (activewpfile, par2, sizeof (activewpfile));
		    else if ( (strcmp(par1, "testgarminmode")) == 0)
			testgarmin = atoi (par2);
		    else if ( (strcmp(par1, "usedgps")) == 0)
			usedgps = atoi (par2);
		    else if ( (strcmp(par1, "mapdir")) == 0)
			g_strlcpy (local_config_mapdir, par2, sizeof (local_config_mapdir));
		    else if ( (strcmp(par1, "simfollow")) == 0)
			simfollow = atoi (par2);
		    else if ( (strcmp(par1, "satposmode")) == 0)
			satposmode = atoi (par2);
		    else if ( (strcmp(par1, "printoutsats")) == 0)
			printoutsats = atoi (par2);
		    else if ( (strcmp(par1, "minsecmode")) == 0)
			minsecmode = atoi (par2);
		    else if ( (strcmp(par1, "nightmode")) == 0)
			nightmode = atoi (par2);
		    else if ( (strcmp(par1, "cpuload")) == 0)
			cpuload = atoi (par2);
		    else if ( (strcmp(par1, "flymode")) == 0)
			flymode = atoi (par2);
		    else if ( (strcmp(par1, "vfr")) == 0)
			vfr = atoi (par2);
		    else if ( (strcmp(par1, "disdevwarn")) == 0)
			disdevwarn = atoi (par2);
		    else if ( (strcmp(par1, "wp_from_sql")) == 0)
			wp_from_sql = atoi (par2);
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
		    else if ( (strcmp(par1, "dbwherestring")) == 0)
			g_strlcpy (dbwherestring, par2, sizeof (dbwherestring));
		    else if ( (strcmp(par1, "earthmate")) == 0)
			earthmate = atoi (par2);

		    else if ( (strcmp(par1, "font_text")) == 0)
			g_strlcpy (font_s_text, par2, sizeof (font_s_text));
		    else if ( (strcmp(par1, "font_verysmalltext")) == 0)
			g_strlcpy (font_s_verysmalltext, par2, sizeof (font_s_verysmalltext));
		    else if ( (strcmp(par1, "font_smalltext")) == 0)
			g_strlcpy (font_s_smalltext, par2, sizeof (font_s_smalltext));
		    else if ( (strcmp(par1, "font_bigtext")) == 0)
			g_strlcpy (font_s_bigtext, par2, sizeof (font_s_bigtext));
		    else if ( (strcmp(par1, "font_wplabel")) == 0)
			g_strlcpy (font_s_wplabel, par2, sizeof (font_s_wplabel));

		    else if ( (strcmp(par1, "friendsserverip")) == 0)
			g_strlcpy (friendsserverip, par2, sizeof (friendsserverip));
		    else if ( (strcmp(par1, "friendsserverfqn")) == 0)
			g_strlcpy (friendsserverfqn, par2, sizeof (friendsserverfqn));
		    else if ( (strcmp(par1, "friendsname")) == 0)
			g_strlcpy (friendsname, par2, sizeof (friendsname));
		    else if ( (strcmp(par1, "friendsidstring")) == 0)
			g_strlcpy (friendsidstring, par2, sizeof (friendsidstring));
		    else if ( (strcmp(par1, "usefriendsserver")) == 0)
			havefriends = atoi (par2);
		    else if ( (strcmp(par1, "maxfriendssecs")) == 0)
			maxfriendssecs = atoi (par2);
		    else if (
			     ( (strcmp(par1, "reminder")) == 0 ) &&
			     ( (strcmp(par2, VERSION)) == 0) )
			{
			    needreminder = FALSE;
			    /*                fprintf(stderr,"needreminder false\n"); */
			}
		    else if ( (strcmp(par1, "storetz")) == 0)
			storetz = atoi (par2);
		    else if ( storetz &&
			      (strcmp(par1, "timezone")) == 0)
			zone = atoi (par2);
		    else if ( (strcmp(par1, "etch")) == 0)
			etch = atoi (par2);
		    else if ( (strcmp(par1, "bigcolor")) == 0)
			g_strlcpy (bluecolor, par2, sizeof (bluecolor));
		    else if ( (strcmp(par1, "trackcolor")) == 0)
			g_strlcpy (trackcolor, par2, sizeof (trackcolor));
		    else if ( (strcmp(par1, "friendscolor")) == 0)
			g_strlcpy (friendscolor, par2, sizeof (friendscolor));
		    else if ( (strcmp(par1, "messagenumber")) == 0)
			messagenumber = atoi (par2);
		    else if ( (strcmp(par1, "serialspeed")) == 0)
			serialspeed = atoi (par2);
		    else if ( (strcmp(par1, "disableserial")) == 0)
			disableserial = atoi (par2);
		    else if ( (strcmp(par1, "showssid")) == 0)
			showsid = atoi (par2);
		    else if ( (strcmp(par1, "sound_direction")) == 0)
			sound_direction = atoi (par2);
		    else if ( (strcmp(par1, "sound_distance")) == 0)
			sound_distance = atoi (par2);
		    else if ( (strcmp(par1, "sound_speed")) == 0)
			sound_speed = atoi (par2);
		    else if ( (strcmp(par1, "sound_gps")) == 0)
			sound_gps = atoi (par2);
		    else if ( (strcmp(par1, "icon_theme")) == 0)
			g_strlcpy (local_config.icon_theme, par2, sizeof (local_config.icon_theme));
		    else if ( (strcmp(par1, "draw_grid")) == 0)
			do_draw_grid = atoi (par2);
		    else if ( (strcmp(par1, "draw_streets")) == 0)
			streets_draw = atoi (par2);
		    else if ( (strcmp(par1, "draw_poi")) == 0)
			poi_draw = atoi (par2);
		    else if ( (strcmp(par1, "draw_wlan")) == 0)
			wlan_draw = atoi (par2);
		    else if ( (strcmp(par1, "draw_tracks")) == 0)
			tracks_draw = atoi (par2);
		    else if ( ! strncmp(par1, "display_map_",12) )
			{
			    // printf ("display_map: %s %s\n",par1,par2);
			    max_display_map += 1;
			    display_map = g_renew(map_dir_struct, display_map, max_display_map);
			    g_strlcpy (display_map[max_display_map-1].name,
				       (par1+12),
				       sizeof (display_map[max_display_map-1].name));
			    display_map[max_display_map-1].to_be_displayed = atoi (par2);
			}
		    else
			fprintf (stderr, "ERROR: Unknown Config Parameter '%s=%s'\n",par1,par2);
		}		/* if e==2 */
	}

	if ( mydebug > 1 )
		fprintf ( stderr,"\nreading config file finished\n");
	fclose (fp);
}
