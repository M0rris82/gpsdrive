/***********************************************************************

Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>

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


/*
 * module for accessing the mapnik/postgis database with gda, used by functions in database.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmodule.h>
#include <gdk/gdktypes.h>
#include <libgda/libgda.h>

#include "config.h"
#include "gpsdrive.h"
#include "gettext.h"
#include "gpsdrive_config.h"
#include "database_postgis.h"
#include "database.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint mydebug;
extern currentstatus_struct current;

extern GdaConnection *db_conn_osm;


/* *****************************************************************************
 * do a query in the postgis database
 */
glong
db_postgis_query (gchar *query, gint (*callback)(const char*, ...))
{
	GdaDataModel *t_model;
	GValue *t_value;
	gint t_rows = 0;
	gint i;

	if (!db_conn_osm)
		return -1;

	t_model = gda_execute_select_command (db_conn_osm, query, NULL);

	if (t_model == NULL)
	{
		g_print ("db_postgis_query: an error occured while trying to read from the database!\n");
		return -1;
	}

	t_rows = gda_data_model_get_n_rows (t_model);

	if (mydebug > 30)
		g_printf ("db_postgis_query: read %d rows from database.\n", t_rows);
	if (mydebug > 60)
		gda_data_model_dump (t_model, stderr);

	for (i = 0; i < t_rows; i++)
	{
		gchar *t_name, *t_type, *t_geom, *t_id;

		t_value = (GValue *) gda_data_model_get_value_at (t_model, 0, i);
		t_name = gda_value_stringify (t_value);

		t_value = (GValue *) gda_data_model_get_value_at (t_model, 1, i);
		t_type = gda_value_stringify (t_value);

		t_value = (GValue *) gda_data_model_get_value_at (t_model, 2, i);
		t_geom = gda_value_stringify (t_value);

		if (gda_data_model_get_n_columns (t_model) > 3)
		{
			t_value = (GValue *) gda_data_model_get_value_at (t_model, 3, i);
			t_id = gda_value_stringify (t_value);
		}
		else
			t_id = NULL;

		callback (t_name, t_type, t_geom, t_id);
	}

	g_object_unref (t_model);
	return t_rows;
}


/* *****************************************************************************
 * do a streets query in the postgis database
 */
glong
db_postgis_query_street (gchar *query, street_struct *street)
{
	GdaDataModel *t_model;
	GValue *t_value;
	gchar *t_buf;
	gint t_rows = 0;

	if (!db_conn_osm)
		return -1;

	if (mydebug > 20)
		g_printf ("db_postgis_query_street: %s\n", query);

	t_model = gda_execute_select_command (db_conn_osm, query, NULL);

	if (t_model == NULL)
	{
		g_print ("db_postgis_query: an error occured while trying to read from the database!\n");
		return -1;
	}

	t_rows = gda_data_model_get_n_rows (t_model);

	if (mydebug > 30)
		g_printf ("db_postgis_query_street: read %d rows from database.\n", t_rows);
	if (mydebug > 60)
		gda_data_model_dump (t_model, stderr);

	if (t_rows == 1)
	{
		t_value = (GValue *) gda_data_model_get_value_at (t_model, 0, 0);
		g_strlcpy (street->name, gda_value_stringify (t_value), sizeof (street->name));

		t_value = (GValue *) gda_data_model_get_value_at (t_model, 1, 0);
		g_strlcpy (street->ref, gda_value_stringify (t_value), sizeof (street->ref));

		t_value = (GValue *) gda_data_model_get_value_at (t_model, 2, 0);
		g_strlcpy (street->type, gda_value_stringify (t_value), sizeof (street->type));
	}

	g_object_unref (t_model);
	return t_rows;
}
