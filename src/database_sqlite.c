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
 * module for direct sqlite database access, used by functions in database.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmodule.h>
#include <gdk/gdktypes.h>
#include <sqlite3.h>

#include "config.h"
#include "gpsdrive.h"
#include "gettext.h"
#include "gpsdrive_config.h"
#include "database_sqlite.h"

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


static sqlite3 *waypoints_db, *geoinfo_db;


/* *****************************************************************************
 * do a query in the sqlite database
 */
glong
db_sqlite_query (gchar *query, gpointer callback, gint database, gchar *result)
{
	gint t_status = 0;
	gchar *t_error;
	glong t_last;

	switch (database)
	{
		case DB_SQLITE_GEOINFO:
			t_status = sqlite3_exec (geoinfo_db, query, callback, result, &t_error);
			t_last = 0;
			break;
		case DB_SQLITE_WAYPOINTS:
			t_status = sqlite3_exec (waypoints_db, query, callback, result, &t_error);
			t_last = sqlite3_last_insert_rowid(waypoints_db);
			break;
	}

	if (t_status != SQLITE_OK )
	{
		g_printf ("DB: SQLite error: %s\n", t_error);
		sqlite3_free(t_error);
		return -1;
	}

	return t_last;
}


/* *****************************************************************************
 * create tables in user database waypoints.db if not yet existent
 */
static gboolean
db_sqlite_create_tables (sqlite3 *db)
{
	gint t_status = 0;
	gchar *t_error;

	const gchar sql_create_poitable[] =
		"CREATE TABLE IF NOT EXISTS poi (\n"
		"poi_id        INTEGER      PRIMARY KEY,\n"
		"name          VARCHAR(80)  NOT NULL default \'not specified\',\n"
		"poi_type      VARCHAR(160) NOT NULL default \'unknown\',\n"
		"lat           DOUBLE       NOT NULL default \'0\',\n"
		"lon           DOUBLE       NOT NULL default \'0\',\n"
		"alt           DOUBLE                default \'0\',\n"
		"comment       VARCHAR(255)          default NULL,\n"
		"last_modified DATETIME     NOT NULL default \'0000-00-00\',\n"
		"source_id     INTEGER      NOT NULL default \'1\',\n"
		"private       CHAR(1)               default NULL);";

	const gchar sql_create_poiextratable[] =
		"CREATE TABLE IF NOT EXISTS poi_extra (\n"
		"poi_id         INTEGER       NOT NULL default \'0\',\n"
		"field_name     VARCHAR(160)  NOT NULL default \'0\',\n"
		"entry          VARCHAR(8192) default NULL);";

	/* creating table 'poi' in local database 'waypoints' */
	t_status = sqlite3_exec(db, sql_create_poitable, NULL, NULL, &t_error);
	if (t_status != SQLITE_OK )
	{
		g_printf ("DB: SQLite error: %s\n", t_error);
		sqlite3_free(t_error);
		return FALSE;
	}

	/* creating table 'poi_extra' in local database 'waypoints' */
	t_status = sqlite3_exec(db, sql_create_poiextratable, NULL, NULL, &t_error);
	if (t_status != SQLITE_OK )
	{
		g_printf ("DB: SQLite error: %s\n", t_error);
		sqlite3_free(t_error);
		return FALSE;
	}

	return TRUE;
}


/* *****************************************************************************
 * Init Database access
 */
gboolean
db_sqlite_init (void)
{
	gchar t_buf[200];
	gint t_status = 0;

	/* open geoinfo.db */
	t_status = sqlite3_open(local_config.geoinfo_file, &geoinfo_db);
	if (t_status != SQLITE_OK)
	{
		g_printf ("DB: Error while opening %s: %s\n",
			local_config.geoinfo_file, sqlite3_errmsg (geoinfo_db));
		sqlite3_close (geoinfo_db);
		exit(1);
	}

	/* open waypoints.db */
	g_snprintf (t_buf, sizeof (t_buf), "%s/waypoints.db", local_config.dir_home);
	t_status = sqlite3_open(t_buf, &waypoints_db);
	if (t_status == SQLITE_OK)
		g_print ("DB: Using waypoints from local user file.\n");
	else
	{
		g_printf ("DB: Error %d while opening waypoints.db: %s\n",
			t_status, sqlite3_errmsg (waypoints_db));
		sqlite3_close (waypoints_db);
		exit(1);
	}

	/* create tables in waypoints.db */
	t_status = db_sqlite_create_tables (waypoints_db);
	if (!t_status)
	{
		g_print ("DB: Error while creating local waypoints database!\n");
		return FALSE;
	}

	return TRUE;
}


/* *****************************************************************************
 * Close all SQLite Database connections
 */
gboolean
db_sqlite_close (void)
{
	sqlite3_close (geoinfo_db);
	sqlite3_close (waypoints_db);

	return TRUE;
}


/* *****************************************************************************
 * this function can be used as callback for a sqlite3_exec function
 * it dumps the results of the query to stdout (can be used it for debugging)
 */
gint
db_sqlite_dump (gpointer datum, gint columns, gchar **values, gchar **names)
{
	gint i;
	for (i=0; i < columns; i++)
		g_printf ("%s = %s\n", names[i], values[i] ? values[i] : "NULL");
	g_printf("\n");
	return 0;
}
