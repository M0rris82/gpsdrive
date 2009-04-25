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
 * mod_nav_dummy.c
 *
 * This is a dummy module delivering a route to gpsdrive.
 * It's purpose is to demonstrate how external route calculation
 * libraries could be implemented to be used with gpsdrive.
 *
 */

#include "config.h"
#include <stdio.h>
#include <gmodule.h>
#include <navigation.h>

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


/* This is the main function that does the actual work.
 * the parameters:
 * 	slat, slon: starting position (usually current position)
 * 	elat, elon: end position (destination)
 * 	navtype: type of wanted route (see navigation.h - nav_type)
 * 	errmsg: a string containing the error message in case of failure, or NULL
 * 	return value: a pointer to a navigation_struct to contain the actual route
 * 		data calculated, or NULL if something fails.
 */
navigation_struct *calculate_route
	(gdouble slat, gdouble slon, gdouble elat, gdouble elon,
	 gint navtype, gchar **errmsg)
{
	navigation_struct *data;

	/* create a dummy route */
	{
		data = g_new (navigation_struct, 3);

		data[0].number = 0;
		g_strlcpy (data[0].name, "Foo Road", sizeof (data[0].name));
		g_strlcpy (data[0].instruction, _("Turn right into Foo Road."),
			sizeof (data[0].instruction));
		data[0].lat = slat;
		data[0].lon = elon;
		data[0].type = NAV_TURN_RIGHT;

		data[1].number = 1;
		g_strlcpy (data[1].name, "Bar Avenue", sizeof (data[1].name));
		g_strlcpy (data[1].instruction, _("Turn left into Bar Avenue."),
			sizeof (data[1].instruction));
		data[1].lat = elat;
		data[1].lon = slon;
		data[1].type = NAV_TURN_LEFT;

		data[2].number = -1;
		g_strlcpy (data[2].name, "Destination", sizeof (data[2].name));
		g_strlcpy (data[2].instruction, _("You have reached your destination."),
			sizeof (data[2].instruction));
		data[2].lat = elat;
		data[2].lon = elon;
		data[2].type = NAV_DESTINATION;
	}

	return data;
}


/* This function is called when this module is loaded.
 * This happens on gpsdrive startup or when this module is
 * selected in the gpsdrive settings.
 * Use it to initialize your external software.
 * It should return NULL n success or a string describing the error
 */
const gchar *g_module_check_init (GModule *module)
{
	if (mydebug > 30)
		g_print (_("Navigation module 'dummy' initialized\n"));

	return NULL;
}


/* This function is called when this module is unloaded.
 * This happens on gpsdrive shutdown or when another module is
 * selected in the gpsdrive settings.
 * Use it to clean up your external software.
 */
void g_module_unload (GModule *module)
{
	if (mydebug > 30)
		g_print (_("Navigation module 'dummy' unloaded\n"));
}
