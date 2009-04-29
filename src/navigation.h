/***********************************************************************

Copyright (c) 2009 Guenther Meyer <d.s.e (at) sordidmusic.com>

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

    *********************************************************************

*/


#ifndef GPSDRIVE_NAVIGATION_H
#define GPSDRIVE_NAVIGATION_H


gboolean nav_init (const gchar *mod_path, const gchar *mod_name);
gboolean nav_close (void);
gboolean nav_get_route (gdouble slat, gdouble slon, gdouble elat, gdouble elon, gint type);

typedef struct
{
	gint number;			/* a serial number, -1 to mark the last entry */
	gchar name[100];		/* a short name for the current point */
	gchar instruction[500];	/* a route instruction to be written or spoken */
	gdouble lat;			/* the latitude in degrees (WGS84) */
	gdouble lon;			/* the longitude in degrees (WGS84) */
	gint type;				/* the point type (see enum nav_type), used to draw an icon */
} navigation_struct;

typedef gpointer (*NavCalcRouteFunc)
	(gdouble slat, gdouble slon, gdouble elat, gdouble elon, gint navtype, gchar **errmsg);

enum nav_type
{
	NAVTYPE_CAR_FAST,
	NAVTYPE_CAR_SHORT,
	NAVTYPE_BIKE_SHORT,
	NAVTYPE_WALK_SHORT,
	NAVTYPE_N_TYPES
};


enum nav_instruction
{
	NAV_ROUTEPOINT,
	NAV_DESTINATION,
	NAV_TURN_LEFT,
	NAV_EXIT_LEFT,
	NAV_FORK_LEFT,
	NAV_TURN_HARD_LEFT,
	NAV_MERGE_LEFT,
	NAV_TURN_SOFT_LEFT,
	NAV_UTURN_LEFT,
	NAV_TURN_RIGHT,
	NAV_EXIT_RIGHT,
	NAV_FORK_RIGHT,
	NAV_TURN_HARD_RIGHT,
	NAV_MERGE_RIGHT,
	NAV_TURN_SOFT_RIGHT,
	NAV_UTURN_RIGHT,
	NAV_STRAIGHT_ON,
	NAV_ENTER_ROUNDABOUT,
	NAV_EXIT_ROUNDABOUT,
	NAV_ROUNDABOUT_FIRST_EXIT,
	NAV_ROUNDABOUT_SECOND_EXIT,
	NAV_ROUNDABOUT_THIRD_EXIT,
	NAV_ROUNDABOUT_FOURTH_EXIT,
	NAV_ROUNDABOUT_FIFTH_EXIT,
	NAV_N_ELEMENTS
};


#endif /* GPSDRIVE_NAVIGATION_H */
