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
 * general module for database access
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <gmodule.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#ifdef GDA3
#include <libgda/libgda.h>
#endif

#include "config.h"
#include "gpsdrive.h"
#include "gettext.h"
#include "gpsdrive_config.h"
#include "database.h"
#include "database_sqlite.h"
#include "database_postgis.h"
#include "mapnik.h"

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

#ifdef GDA3
GdaConnection *db_conn_geo, *db_conn_osm;
#endif

	//TODO:	create table 'poi_extra_fields' in geoinfo.db
	//	filled with default values.

typedef struct
{
	glong id;
	gchar entry[8192];
} result_struct;


/* ******************************************************************
 * escape special characters in sql string.
 * returned string has to be freed after usage!
 */
gchar 
*escape_sql_string (const gchar *data)
{
	gint i, j, length;
	gchar *tdata = NULL;

	if (mydebug > 50)
		g_printf ("escape_sql_string: in  - %s\n", data);

	length = strlen (data);
	tdata = g_malloc (length*2 + 1);
	j = 0;
	for (i = 0; i <= length; i++)
	{
		if (data[i] == '\'' || data[i] == '\\' || data[i] == '\"')
			tdata[j++] = '\\';
		tdata[j++] = data[i];
	}

	if (mydebug > 50)
		g_printf ("escape_sql_string: out - %s\n", tdata);
	
	return tdata;
}


static gint
handle_cleanup_friends_cb (gpointer data, gint columns, gchar **values, gchar **names)
{
	gchar t_query[50];

	if (columns == 0)
		return 0;

	if (mydebug > 30)
		g_printf ("\nhandle_cleanup_friends_cb: %s = %s\n", names[0], values[0]);

	g_snprintf (t_query, sizeof (t_query), "DELETE FROM poi_extra WHERE poi_id='%s';", values[0]);
	db_sqlite_query (t_query, NULL, DB_SQLITE_WAYPOINTS, NULL);

	return 0;
}


static gint
handle_poi_extra_get_cb (result_struct *result, gint columns, gchar **values, gchar **names)
{
	if (columns == 0)
		return 0;

	if (mydebug > 30)
		g_printf ("\nhandle_poi_extra_get_cb: %s = %s\n", names[0], values[0]);

	(*result).id = strtol (values[0], NULL, 10);
	
	if (columns == 2)
	{
		g_strlcpy ((*result).entry, values[1], sizeof ((*result).entry));
		if (mydebug > 30)
			g_printf ("handle_poi_extra_get_cb: %s = %s\n", names[1], values[1]);
	}

	return 0;
}


/* ******************************************************************
 * delete poi data in poi table
 */
gint
db_poi_delete (gint index)
{
	gchar t_query[100];

	g_snprintf (t_query, sizeof (t_query),
		"DELETE FROM poi WHERE poi_id='%d'; DELETE FROM poi_extra WHERE poi_id='%d';",
		index, index);

	if (mydebug > 50)
		g_print ("query: %s\n", t_query);

	db_sqlite_query (t_query, NULL, DB_SQLITE_WAYPOINTS, NULL);

	return 0;
}


/* ******************************************************************
 * insert or update poi data in poi table
 */
glong
db_poi_edit
  (glong poi_id, double lat, double lon, gchar *name, gchar *typ, gchar *comment, gint src, gboolean update)
{
	gchar t_query[500], lats[20], lons[20];
	gchar *tname, *ttyp, *ttyp_l, *tcomment;
	gint pt_id;
	glong r;
	GTimeVal current_time;

	if (mydebug > 99)
	{
		g_print ("\ndb_poi_edit:\n");
		g_printf ("------- %.6f / %.6f - %d - %d\n", lat, lon, src, update);
		g_printf ("------- %s [%s]\n", name, typ);
		g_printf ("------- %s\n\n", comment);
	}

	g_snprintf (lats, sizeof (lats), "%.8f", lat);
	g_strdelimit (lats, ",", '.');
	g_snprintf (lons, sizeof (lons), "%.8f", lon);
	g_strdelimit (lons, ",", '.');

	/* convert poi_type to lower case */
	ttyp_l = g_ascii_strdown (typ, -1);

	/* escape ' */
	tname = escape_sql_string (name);
	ttyp = escape_sql_string (ttyp_l);
	tcomment = escape_sql_string (comment);

	/* set source_id at value for 'user entered data'if none is given */
	if (!src)
		src = 3;
	/* get current time for field last_modified */
	g_get_current_time (&current_time);

	if (update)
	{
		g_snprintf (t_query, sizeof (t_query),
			"UPDATE poi SET name='%s', lat='%s', lon='%s', poi_type='%s', comment='%s', source_id='%d', last_modified='%ld' WHERE poi_id='%ld';",
			tname, lats, lons, ttyp, tcomment, src, current_time.tv_sec, poi_id);
	}
	else
	{
		g_snprintf (t_query, sizeof (t_query),
			"INSERT INTO poi (name,lat,lon,poi_type,comment,source_id,last_modified) VALUES ('%s','%s','%s','%s','%s','%d','%ld');",
			tname, lats, lons, ttyp, tcomment, src, current_time.tv_sec);
	}

	if (mydebug > 50)
		g_printf ("sql query: %s\n", t_query);

	g_free (tname);
	g_free (ttyp);
	g_free (ttyp_l);
	g_free (tcomment);

	return db_sqlite_query (t_query, NULL, DB_SQLITE_WAYPOINTS, NULL);
}


/* ******************************************************************
 * insert or update additional poi data in poi_extra table
 */
glong
db_poi_extra_edit (glong *poi_id, gchar *field_name, gchar *field_entry, gboolean update)
{
	char t_query[9000];
	gchar *tentry, *tfield;
	int r;
	
	/* escape ' */
	tfield = escape_sql_string (field_name);
	tentry = escape_sql_string (field_entry);
	
	if (update)
	{
		g_snprintf (t_query, sizeof (t_query),
			"UPDATE poi_extra SET entry='%s' WHERE (poi_id=%ld AND field_name='%s')",
			tentry, *poi_id, field_name);
	}
	else
	{
		g_snprintf (t_query, sizeof (t_query),
			"INSERT INTO poi_extra (poi_id, field_name, entry) VALUES ('%ld','%s','%s')",
			*poi_id, field_name, tentry);
	}

	if (mydebug > 50)
		printf ("sql query: %s\n", t_query);

	g_free (tentry);
	g_free (tfield);

	return db_sqlite_query (t_query, NULL, DB_SQLITE_WAYPOINTS, NULL);
}


/* ******************************************************************
 * get additional poi data from poi_extra table
 *
 * You have to set at least two parameters; set the one you are
 * asking for to NULL.
 * The return value is the poi_id.
 */
glong
db_poi_extra_get (glong *poi_id, gchar *field_name, gchar *field_entry, gchar *result)
{
	gchar t_query[5000];
	result_struct t_result;
	
	t_result.id = 0;

	if (field_entry == NULL)
	{
		g_snprintf (t_query, sizeof (t_query),
			"SELECT poi_id,entry FROM poi_extra "
			"WHERE (poi_id='%ld' AND field_name='%s') LIMIT 1;",
			*poi_id, field_name);
	}
	else if (poi_id == NULL)
	{
		g_snprintf (t_query, sizeof (t_query),
		"SELECT poi_id FROM poi_extra "
		"WHERE (field_name='%s' AND entry='%s') LIMIT 1;",
		field_name, field_entry);
	}
	else if (field_name == NULL)
	{
		g_snprintf (t_query, sizeof (t_query),
		"SELECT poi_id,field_id FROM poi_extra "
		"WHERE (poi_id='%ld' AND entry='%s') LIMIT 1;",
		*poi_id, field_entry);
	}
	else
		return 0;
	
	if (mydebug > 20)
		g_printf ("db_poi_extra_get: sql query: %s\n", t_query);

	db_sqlite_query (t_query, handle_poi_extra_get_cb, DB_SQLITE_WAYPOINTS, (gpointer) &t_result);

	return t_result.id;
}


static gint
handle_poi_types_cb (gchar *result, gint columns, gchar **values, gchar **names)
{
	gchar t_buf[100];

	if (columns == 0)
		return 0;

	if (mydebug > 30)
		g_printf ("handle_poi_types_cb: %s = %s\n", names[0], values[0]);

	if (strlen (result) == 0)
		g_snprintf (t_buf, sizeof (t_buf), "'%s'", values[0]);
	else	
		g_snprintf (t_buf, sizeof (t_buf), ",'%s'", values[0]);
	g_strlcat (result, t_buf, 20000);

	return 0;
}


/* *******************************************************
 * update, which poi_types should be shown in the map.
 * called by update_poi_type_filter and on each change
 * of the current map scale
 */
void
db_get_visible_poi_types (gchar *filter)
{
	gchar t_query[20000];
	gchar t_result[20000];

	if (!filter)
		return;

	g_snprintf (t_query, sizeof (t_query),
		"SELECT poi_type FROM poi_type WHERE (%ld BETWEEN scale_min AND scale_max) %s;",
		current.mapscale, filter);

	if (mydebug > 20)
		g_printf ("db_get_visible_poi_types: SQL Query: %s\n", t_query);

	t_result[0] = '\0';
	db_sqlite_query (t_query, handle_poi_types_cb, DB_SQLITE_GEOINFO, t_result);
	g_strlcpy (current.poifilter, t_result, sizeof (current.poifilter));

	if (mydebug > 20)
		g_printf ("db_get_visible_poi_types: Result: %s\n", current.poifilter);
}


/* *******************************************************
 */
void
db_poi_get (gchar *query, gpointer callback, gint database)
{
	if (mydebug > 20)
		g_printf ("db_poi_get: %s\n", query);

	switch (database)
	{
		case DB_WP_USER:
			/* get data from waypoints/sqlite database */
			db_sqlite_query (query, callback, DB_SQLITE_WAYPOINTS, NULL);
			break;
#ifdef GDA3
		case DB_WP_OSM:
			/* get data from openstreetmap/postgis database */
			if (db_conn_osm)
				db_postgis_query (query, callback);
			break;
#endif
	}
}


/* *******************************************************
 */
gint
db_streets_get (const gdouble lat, const gdouble lon, const guint distance, street_struct *street)
{
	gint t_res = 0;
#ifdef MAPNIK
	gchar sql_query[200];
	gdouble x, y;
	guint d;

	if (street == NULL)
		return;

	d = (distance) ? distance : 40;
	convert_mapnik_coords(&x, &y, lon, lat, 0);
	g_snprintf (sql_query, sizeof (sql_query),
		"SELECT name,ref,highway FROM planet_osm_line WHERE ST_DWithin(SetSRID(way,-1),"
		" 'POINT(%.8f %.8f)', %d) AND highway!='' AND (name!='' OR ref!='') LIMIT 1;",
		x, y, d);

	t_res = db_postgis_query_street (sql_query, street);

	if (mydebug > 20)
		g_print ("db_streets_get: (%s) %s [%s]\n", street->ref, street->name, street->type);
#endif
	return t_res;
}


/* *******************************************************
 */
void
db_get_all_poi_types (gpointer callback, guint count[])
{
	db_sqlite_query ("SELECT poi_type,scale_min,scale_max,title,description FROM poi_type ORDER BY poi_type;",
		callback, DB_SQLITE_GEOINFO, (gpointer) count);
}


/* ******************************************************************
 * remove friendsd data from database (on shutdown)
 */
gint
db_cleanup_friends ()
{
	if (mydebug > 20)
		g_print ("cleaning up friendsd data\n");

	db_sqlite_query ("SELECT poi_id FROM poi WHERE poi_type LIKE 'people.friendsd%';",
		handle_cleanup_friends_cb, DB_SQLITE_WAYPOINTS, NULL);
	db_sqlite_query ("DELETE FROM poi WHERE poi_type LIKE 'people.friendsd%';",
		NULL, DB_SQLITE_WAYPOINTS, NULL);

	return 0;
}


/* ******************************************************************
 * remove route data from database (on shutdown)
 */
gint
db_cleanup_route ()
{
	if (mydebug > 20)
		g_print ("cleaning up route data\n");

	db_sqlite_query ("DELETE FROM poi WHERE poi_type LIKE 'waypoint.routepoint%';",
		NULL, DB_SQLITE_WAYPOINTS, NULL);

	return 0;
}


/* *****************************************************************************
 * Init Database access
 */
gboolean
db_init (void)
{
#ifdef GDA3
	GdaClient *t_client;
#endif
	gchar t_buf[200];
	gboolean t_status = FALSE;
	gboolean t_db = FALSE;

	if (mydebug > 0)
		g_print ("Initializing Database access...\n");

	/* init local sqlite database */
	t_status = db_sqlite_init ();
	if (t_status)
	{
		t_db = TRUE;
	}

#ifdef GDA3
	/* init gda */
	gda_init ("GpsDrive", VERSION, 0, NULL);
	t_client = gda_client_new ();

	/* create connection to (old) mysql database */
/*	db_conn_geo = gda_client_open_connection_from_string
		(t_client, "MySQL", "DB_NAME=geoinfo;HOST=localhost", "gast", "gast",
		GDA_CONNECTION_OPTIONS_NONE, NULL);
	if (db_conn_geo)
	{
		g_print ("DB: Using waypoints from old MySQL database.\n");
		t_db = TRUE;
	}
*/

	/* create connection to mapnik/postgis database */
	db_conn_osm = gda_client_open_connection_from_string (t_client,
		"PostgreSQL", "DB_NAME=gis", "", "", GDA_CONNECTION_OPTIONS_NONE, NULL);
	if (db_conn_osm)
	{
		g_print ("DB: Using waypoints from Mapnik database.\n");
		t_db = TRUE;
	}
#endif

	if (!t_db)
	{
		g_print ("DB: No database sources available!\n");
		return FALSE;
	}

	return TRUE;
}


/* *****************************************************************************
 * Close all Database connections
 */
void
db_close (void)
{
	db_cleanup_route ();
	db_cleanup_friends ();

	db_sqlite_close ();
}

