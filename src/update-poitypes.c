/***********************************************************************
gpsdrive-update-mapnik-db

    match gpsdrive poi_types with osm types and update the
    poi column inside the mapnik postgis db

Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>

Website: www.gpsdrive.de

Disclaimer: Please do not use for navigation.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*********************************************************************/


#include "config.h"
#include <stdlib.h>
#include <glib.h>
#include <locale.h>
#include <libgda/libgda.h>
#include <sqlite3.h>


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


#define DB_GEOINFO "/usr/share/gpsdrive/geoinfo.db"
#define DB_MAPNIK "DB_NAME=gis"

#define PGM_VERSION "0.2"

GdaConnection *db_conn_osm;
sqlite3 *geoinfo_db;
glong count = 0;
gboolean show_version = FALSE;
gboolean verbose = FALSE;
gint spinpos = 0;


/* *****************************************************************************
 * callback for filling the poi column with data
 */
gint
fill_poi_column_cb (gpointer datum, gint columns, gchar **values, gchar **names)
{
	glong rows = 0;
	gchar query[512];
	const gchar *spinner = "|/-\\";

	if (verbose)
		g_print ("    %s = %s\t--->\t%s\t",values[0], values[1], values[2]);

	g_snprintf (query, sizeof (query),
		"UPDATE planet_osm_point SET poi='%s' WHERE %s='%s' AND poi IS NULL;",
		values[2], values[0], values[1]);
	rows = gda_execute_sql_command (db_conn_osm, query, NULL);

	if (rows < 0 )
	{
		g_print (_("\nERROR: can't get write access to the mapnik database!\n\n"));
		exit (EXIT_FAILURE);
	}

	count += rows;

	if (verbose)
		g_print ("[%u]\n", (unsigned int) rows);
	else
		g_print ("\b%c", spinner[spinpos++%4]);

	return 0;
}


/*******************************************************************************
 *                                                                             *
 *                             Main program                                    *
 *                                                                             *
 *******************************************************************************/
int
main (int argc, char *argv[])
{
	gchar const rcsid[] = "$Id$";

	GOptionContext *opt_context;
	GdaClient *db_client;
	gint status = 0;
	gchar *db_file;
	gchar *error_string;
	GError *error = NULL;
	GTimer *timer;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	/* parse commandline options */
	opt_context = g_option_context_new (_(
		"prepare mapnik database for use with gpsdrive"));
	const gchar opt_summary[] = N_(
		"  This program looks for entries indicating \"Points of Interest\"\n"
		"  inside the mapnik database, and adds the matching gpsdrive\n"
		"  types to a separate indexed column called 'poi'.\n\n"
		"  You need to have write access to the mapnik\n"
		"  database to run this program!");
	const gchar opt_desc[] = N_("Website:\n  http://www.gpsdrive.de\n");
	GOptionEntry opt_entries[] =
	{
		{"db-file", 'f', 0, G_OPTION_ARG_FILENAME, &db_file,
			_("set alternate file for geoinfo database"), _("<FILE>")},
		{"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
			_("show some detailed output"), NULL},
		{"version", 0, 0, G_OPTION_ARG_NONE, &show_version,
			_("show version info"), NULL},
		{NULL}
	};
	g_option_context_set_summary (opt_context, opt_summary);
	g_option_context_set_description (opt_context, opt_desc);
	g_option_context_add_main_entries (opt_context, opt_entries, PACKAGE);
	if (!g_option_context_parse (opt_context, &argc, &argv, &error))
	{
		g_print (_("Parsing of commandline options failed: %s\n"), error->message);
		exit (EXIT_FAILURE);
	}
	if (show_version)
	{
		g_print (_("\ngpsdrive-update-mapnik-db\n"
			" (C) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>\n"
			"\n Version %s\n %s\n\n"), PGM_VERSION, rcsid);
		exit (EXIT_SUCCESS);
	}

	g_print ("gpsdrive-update-mapnik-db v%s\n\n", PGM_VERSION);

	/* init database connection */
	g_print (_("+ Initializing Database access\n"));
	gda_init ("gpsdrive_update", VERSION, 0, NULL);
	db_client = gda_client_new ();

	/* create connection to mapnik postgis database */
	db_conn_osm = gda_client_open_connection_from_string (db_client,
		"PostgreSQL", DB_MAPNIK, "", "",
		GDA_CONNECTION_OPTIONS_NONE, NULL);
	if (!db_conn_osm)
	{
		g_printf (_("  Error while opening mapnik database\n"));
		exit (EXIT_FAILURE);
	}

	/* create connection to gpsdrive geoinfo database */
	if (!db_file)
		db_file = g_strdup (DB_GEOINFO);
	if (verbose)
		g_print ("    Using geoinfo file: %s\n", db_file);

	status = sqlite3_open (db_file, &geoinfo_db);
	if (status != SQLITE_OK)
	{
		g_printf (_("  Error while opening %s: %s\n"),
			db_file, sqlite3_errmsg (geoinfo_db));
		sqlite3_close (geoinfo_db);
		g_free (db_file);
		exit (EXIT_FAILURE);
	}
	g_free (db_file);

	/* add new poi column to mapnik database (if necessary) */
	status = gda_execute_sql_command (db_conn_osm,
		"ALTER TABLE planet_osm_point ADD COLUMN poi text;", NULL);
	if (status == 0)
		g_print (_("+ Creating new column 'poi'\n"));
	else
	{
		g_print (_("+ Using existing column 'poi'\n"));
		status= gda_execute_sql_command (db_conn_osm,
			"DROP INDEX poi_index;", NULL);
	}

	/* fill the poi column while matching types with  geoinfo.db */
	g_print (_("+ Looking for known tags and updating poi column\n"));
	if (verbose)
		timer = g_timer_new ();
	else
		g_print ("   ");
	status = sqlite3_exec (geoinfo_db, "SELECT key,value,poi_type FROM osm;",
		fill_poi_column_cb, NULL, &error_string);
	if (status != SQLITE_OK )
	{
		g_printf (_("  SQLite error: %s\n"), error_string);
		sqlite3_free(error_string);
		exit (EXIT_FAILURE);
	}
	if (verbose)
	{
		g_print (_("  %ld rows updated in %.3f seconds\n"),
			count, g_timer_elapsed (timer, NULL));
	}
	else
		g_print (_("\b%ld rows updated\n"), count);

	/* create index on poi column */
	g_print (_("+ Creating new index on column 'poi' (this may take some time...)\n"));
	if (verbose)
		g_timer_start (timer);
	status = gda_execute_sql_command (db_conn_osm,
		"CREATE INDEX poi_index ON planet_osm_point (poi);", NULL);
	if (verbose)
	{
		g_print (_("  Index created in %.3f seconds\n"), g_timer_elapsed (timer, NULL));
		g_timer_destroy (timer);
	}

	if (status > 0)
		g_print ("  done.\n");

	/* clean up connections */
	g_print (_("+ Closing Database access\n"));
	gda_client_close_all_connections (db_client);
	sqlite3_close (geoinfo_db);

	g_print (_("\nFinished.\n"));

	return EXIT_SUCCESS;
}
