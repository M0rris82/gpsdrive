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


/*
 * navigation.c
 *
 * This module holds all the functions to use external modules for
 * route calculation
 *
 */

#include "config.h"
#include <stdio.h>
#include <gmodule.h>
#include "navigation.h"
#include "routes.h"
#include "gui.h"

#include "gettext.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint mydebug;

NavCalcRouteFunc calculate_route;

static GModule *nav_module;
const gchar *routing_types[] = {
	"waypoint.route",
	"waypoint.route.destination",
	"waypoint.route.left",
	"waypoint.route.left.exit",
	"waypoint.route.left.fork",
	"waypoint.route.left.hard",
	"waypoint.route.left.merge",
	"waypoint.route.left.soft",
	"waypoint.route.left.uturn",
	"waypoint.route.right",
	"waypoint.route.right.exit",
	"waypoint.route.right.fork",
	"waypoint.route.right.hard",
	"waypoint.route.right.merge",
	"waypoint.route.right.soft",
	"waypoint.route.right.uturn",
	"waypoint.route.straight"
	"waypoint.route.roundabout"
	"waypoint.route.roundabout.exit"
	"waypoint.route.roundabout.exit.1st"
	"waypoint.route.roundabout.exit.2nd"
	"waypoint.route.roundabout.exit.3rd"
	"waypoint.route.roundabout.exit.4th"
	"waypoint.route.roundabout.exit.5th"
};


/* *****************************************************************************
 * unload an external navigation module
 */
gboolean nav_close (void)
{
	if (nav_module)
	{
		if (mydebug > 10)
			g_print (_("Closing navigation module '%s'\n"), g_module_name (nav_module));

		if (!g_module_close (nav_module))
		{
			g_print (_("ERROR: %s\n"), g_module_error ());
			return FALSE;
		}
	}

	return TRUE;
}


/* *****************************************************************************
 * load and initialize and external module
 */
gboolean nav_init (const gchar *mod_path, const gchar *mod_name)
{
	gchar *t_path;

	nav_close ();

	t_path = g_module_build_path (mod_path, mod_name);
	nav_module = g_module_open (t_path, G_MODULE_BIND_LAZY);
	if (!nav_module)
    {
		g_print (_("WARNING: %s\n"), g_module_error ());
		return FALSE;
	}
	if (!g_module_symbol (nav_module, "calculate_route", (gpointer *)&calculate_route))
	{
		g_print (_("ERROR: %s\n"), g_module_error ());
		nav_close ();
		return FALSE;
    }
	if (calculate_route == NULL)
    {
		g_print (_("ERROR: Missing symbol \"calculate_route\" in navigation module '%s'\n"),
			g_module_name (nav_module));
		nav_close ();
		return FALSE;
    }

	g_print (_("Navigation module '%s' successfully loaded.\n"), g_module_name (nav_module));

	return TRUE;
}

/* *****************************************************************************
 * call the external module and insert the calculated route into gpsdrive
 */
gboolean nav_get_route (gdouble slat, gdouble slon, gdouble elat, gdouble elon, gint type)
{
	navigation_struct *data;
	gint i = 0;

	if (!nav_module)
		return FALSE;

	if (mydebug > 20)
		g_print (_("Calculating route with external module '%s'\n"), g_module_name (nav_module));

	/* calculate fast car route if type value is not valid */
	if (type > NAVTYPE_N_TYPES)
		type = NAVTYPE_CAR_FAST;

	/* trying to get route from external module */
	data = calculate_route (slat, slon, elat, elon, type, NULL);

	if (data == NULL)
	{
		g_print (_("ERROR in external navigation module...\n"));
		popup_error (NULL, _("No route found!"));
		return FALSE;
	}

	/* init new gpsdrive route list and fill it from route data */
	route_init ("Calculated route", "using external module", NULL);

	while (data[i-1].number != -1)
	{
		if (mydebug > 30)
		{
			g_print ("------ Routepoint %d ------\n", data[i].number);
			g_print ("  lat: %.6f / lon: %.6f\n", data[i].lat, data[i].lon);
			g_print ("  %d : %s\n", data[i].type, data[i].name);
			g_print ("  %s\n", data[i].instruction);
		}
		if (data[i].type >= NAV_N_ELEMENTS)
			data[i].type = NAV_ROUTEPOINT;
		add_arbitrary_point_to_route
			(data[i].name, data[i].instruction, routing_types[data[i].type],
			data[i].lat, data[i].lon);
		i++;
	};

	g_free (data);

	return TRUE;
}
