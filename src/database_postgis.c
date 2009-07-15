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
 * module for accessing the mapnik/postgis database,
 * used by functions in database.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmodule.h>
#include <gdk/gdktypes.h>
#include <libpq-fe.h>

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

extern PGconn *db_conn_postgis;


/* *****************************************************************************
 * do a query in the postgis database
 */
glong
db_postgis_query (gchar *query, gint (*callback)(const char*, ...))
{
	gint t_rows = 0;

	if (!db_conn_postgis)
		return -1;

	return t_rows;
}


/* *****************************************************************************
 * do a streets query in the postgis database
 */
gboolean
db_postgis_query_street (gchar *query, street_struct *street)
{
	PGresult *t_result;
	gchar *t_value;

	if (!db_conn_postgis)
		return -1;

	if (mydebug > 20)
		g_print ("db_postgis_query_street: %s\n", query);

	t_result = PQexec (db_conn_postgis, query);

	if (t_result == NULL)
	{
		g_print ("db_postgis_query_street: an error occured while trying to read from the database!\n");
		g_print ("query: %s\n",query);
		return -1;
	}

	if (PQresultStatus (t_result) == PGRES_TUPLES_OK && PQntuples (t_result) == 1) 
	{
		if (mydebug > 30)
			g_print ("db_postgis_query_street: read entry in database.\n");

		t_value = PQgetvalue (t_result, 0, 0);
		g_strlcpy (street->name, t_value, sizeof (street->name));

		t_value = PQgetvalue (t_result, 0, 1);
		g_strlcpy (street->ref, t_value, sizeof (street->ref));

		t_value = PQgetvalue (t_result, 0, 2);
		g_strlcpy (street->type, t_value, sizeof (street->type));

		PQclear (t_result);
		return TRUE;
	}

	PQclear (t_result);
	return FALSE;
}

