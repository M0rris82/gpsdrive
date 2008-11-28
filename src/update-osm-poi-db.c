/***********************************************************************
gpsdrive-update-osm-poi-db

    creates and fills an sqlite database file (osm.db) for storage
    of POI data from an osm xml file, matching gpsdrive poi_types
    with osm types.

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
#include <string.h>
#include <sqlite3.h>
#include <libxml/xmlreader.h>
#include <signal.h>


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


#define DB_GEOINFO "/usr/share/gpsdrive/geoinfo.db"
#define DB_OSMFILE "./osm.db"
#define MAX_TAGS_PER_NODE 10

#define PGM_VERSION "0.1"


sqlite3 *geoinfo_db, *osm_db;
gint status = 0;
gchar *error_string;
gulong count = 0;
gulong node_count = 0;
gulong poi_count = 0;
gboolean show_version = FALSE;
gboolean verbose = FALSE;
gboolean stop_on_way = FALSE;
gboolean nodes_done = FALSE;
gint spinpos = 0;
GHashTable *poitypes_hash;
xmlTextReaderPtr xml_reader;
const gchar *spinner = "|/-\\";

typedef struct
{
	gulong id;
	gdouble lat;
	gdouble lon;
	gchar key[MAX_TAGS_PER_NODE][255];
	gchar value[MAX_TAGS_PER_NODE][255];
	gchar poi_type[160];
	gchar name[80];
	guint tag_count;
}
node_struct;


/* ******************************************************************
 * escape special characters in sql string.
 * returned string has to be freed after usage!
 */
gchar 
*escape_sql_string (const gchar *data)
{
	gint i, j, length;
	gchar *tdata = NULL;

	length = strlen (data);
	tdata = g_malloc (length*2 + 1);
	j = 0;
	for (i = 0; i <= length; i++)
	{
		if (data[i] == '\'')
			tdata[j++] = '\'';
		tdata[j++] = data[i];
	}

	//g_print ("=== %s === %s ===\n", data, tdata);

	return tdata;
}


/* *****************************************************************************
 * callback for reading poi_types from geoinfo.db
 */
gint
read_poi_types_cb (gpointer datum, gint columns, gchar **values, gchar **names)
{
	gchar *t_osm, *t_poi;

	t_osm = g_strdup_printf ("%s=%s", values[0], values[1]);
	t_poi = g_strdup (values[2]);
	g_hash_table_insert (poitypes_hash, t_osm, t_poi);

	if (verbose)
		g_print ("    %s\t--->\t%s\t\n", t_osm, t_poi);

	return 0;
}


/* *****************************************************************************
 * add new POI to database
 */
void
add_new_poi (node_struct *data)
{
	gchar query[500];
	guint i = 0;
	gchar *t_name, *t_type;

	if (verbose)
	{
		g_print ("\n    |  id = %d\t%.6f / %.6f\n    |  poi_type = %s\n"
			"    |  name = %s\n", data->id, data->lat, data->lon,
			data->poi_type, data->name);
		for (i = 0; i < data->tag_count; i++)
			g_print ("    |  %s = '%s'\n", data->key[i], data->value[i]);
	}

	t_name = escape_sql_string (data->name);
	t_type = escape_sql_string (data->poi_type);

	g_snprintf (query, sizeof (query),
		"INSERT INTO poi (name,lat,lon,poi_type,source_id,last_modified)"
		" VALUES ('%s','%.6f','%.6f','%s','4',CURRENT_TIMESTAMP);",
		t_name, data->lat, data->lon, t_type);

	status = sqlite3_exec(osm_db, query, NULL, NULL, &error_string);
	if (status != SQLITE_OK )
	{
		g_print ("\n\nSQLite error: %s\n%s\n\n", error_string, query);
		sqlite3_free(error_string);
		exit (EXIT_FAILURE);
	}

	//t_last = sqlite3_last_insert_rowid(osm_db);

	g_free (t_name);
	g_free (t_type);

}


/* *****************************************************************************
 * callback for parsing node xml data
 */
gint
parse_node_cb (void)
{

//	glong rows = 0;
//	gchar query[512];

	xmlChar *name, *value;
	gint type, status;
	gchar buf[255];
	gchar *pt_pointer;
	gboolean found_poi = FALSE;
	node_struct node;

	node.tag_count = 0;
	node.id = strtol ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "id"), NULL, 10);
	node.lat = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "lat"), NULL);
	node.lon = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "lon"), NULL);
	g_strlcpy (node.name, "", sizeof (node.name));

	status = xmlTextReaderRead (xml_reader);
	type = xmlTextReaderNodeType (xml_reader);
	name = xmlTextReaderName (xml_reader);

	/* we assume, that the next 'node' xml-node is the closing node for the
	 * previously opened 'node' element, so we omit the check for that. */
	while (!xmlStrEqual(name, BAD_CAST "node") /*|| type != XML_READER_TYPE_END_ELEMENT*/)
	{
		//g_print ("parse_node_cb (): node\n");
		if (type == XML_READER_TYPE_ELEMENT
		    && xmlStrEqual (name, BAD_CAST "tag"))
		{
			/* check if type of point is known, and set poi_type */
			g_snprintf (buf, sizeof (buf), "%s=%s",
				xmlTextReaderGetAttribute (xml_reader, BAD_CAST "k"),
				xmlTextReaderGetAttribute (xml_reader, BAD_CAST "v"));
			pt_pointer = g_hash_table_lookup (poitypes_hash, buf);
			if (pt_pointer)
			{
				poi_count++;
				g_strlcpy (node.poi_type, pt_pointer, sizeof (node.poi_type));
				found_poi = TRUE;
			}
			else if (node.tag_count < MAX_TAGS_PER_NODE)
			{
				g_strlcpy (node.key[node.tag_count],
					xmlTextReaderGetAttribute (xml_reader, BAD_CAST "k"),
					sizeof (node.key[node.tag_count]));

				/* skip 'created_by' tag */
				if (strcmp ("created_by", node.key[node.tag_count]) == 0)
				{
					status = xmlTextReaderRead (xml_reader);
					type = xmlTextReaderNodeType (xml_reader);
					name = xmlTextReaderName (xml_reader);
					continue;
				}

				g_strlcpy (node.value[node.tag_count],
					xmlTextReaderGetAttribute (xml_reader, BAD_CAST "v"),
					sizeof (node.value[node.tag_count]));

				/* override poi_type if 'poi' tag is available */
				if (strcmp ("poi", node.key[node.tag_count]) == 0)
					g_strlcpy (node.poi_type, node.value[node.tag_count],
					sizeof (node.poi_type));
				/* get name of node */
				else if (strcmp ("name", node.key[node.tag_count]) == 0)
					g_strlcpy (node.name, node.value[node.tag_count],
					sizeof (node.name));
				else
					node.tag_count++;
			}
		}
		if (name)
			xmlFree (name);
		status = xmlTextReaderRead (xml_reader);
		type = xmlTextReaderNodeType (xml_reader);
		name = xmlTextReaderName (xml_reader);
	}
	if (name)
		xmlFree (name);

	if (found_poi)
		add_new_poi (&node);

/*
    name = xmlTextReaderName(xml);
    if (name == NULL)
        name = xmlStrdup(BAD_CAST "--");
    value = xmlTextReaderValue(xml);

    printf("%d %d %s %d",
            xmlTextReaderDepth(xml),
            xmlTextReaderNodeType(xml),
            name,
            xmlTextReaderIsEmptyElement(xml));
    xmlFree(name);
    if (value == NULL)
        printf("\n");
    else {
        printf(" %s\n", value);
        xmlFree(value);
    }

*/


//	count += rows;
//	if (verbose)
//		g_print ("[%d]\n", rows);
//	else
//		g_print ("\b%c", spinner[spinpos++%4]);
}


/* *****************************************************************************
 * on a INT signal, do a clean shutdown
 */
void signalhandler_int (int sig)
{
	g_print ("\nCatched SIGINT - shutting down !\n\n");

	xmlFreeTextReader (xml_reader);
	sqlite3_close (osm_db);

	exit (EXIT_SUCCESS);
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
	gchar *db_file = NULL;
	gchar *osm_file = NULL;
	GError *error = NULL;
	GTimer *timer;
	gint xml_type = 0;
	xmlChar *xml_name = NULL;
	gulong count = 0;
	gint parsing_time = 0;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

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


	/* parse commandline options */
	opt_context = g_option_context_new (_(
		"source.osm - create and fill GpsDrive POI Database from OSM data"));
	const gchar opt_summary[] = N_(
		"  This program looks for entries indicating \"Points of Interest\"\n"
		"  inside a given OSM XML file, and adds the data to an sqlite database\n"
		"  file used by gpsdrive to access those data.\n");
	const gchar opt_desc[] = N_("Website:\n  http://www.gpsdrive.de\n");
	GOptionEntry opt_entries[] =
	{
		{"db-file", 'f', 0, G_OPTION_ARG_FILENAME, &db_file,
			_("set alternate file for geoinfo database"), _("<FILE>")},
		{"osm-file", 'o', 0, G_OPTION_ARG_FILENAME, &osm_file,
			_("set alternate file for created database"), _("<FILE>")},
		{"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
			_("show some detailed output"), NULL},
		{"stop-on-way", 'w', 0, G_OPTION_ARG_NONE, &stop_on_way,
			_("stop parsing when a way or relation is found"), NULL},
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
	g_print ("gpsdrive-update-osm-poi-db v%s\n\n", PGM_VERSION);

	/* setup signal handler to gracefully handle a CTRL-C command */
	signal (SIGINT, signalhandler_int);


	/* create connection to gpsdrive geoinfo database */
	g_print (_("+ Initializing Database access\n"));
	if (db_file == NULL)
		db_file = g_strdup (DB_GEOINFO);
	if (verbose)
		g_print ("    Using geoinfo database file: %s\n", db_file);
	status = sqlite3_open (db_file, &geoinfo_db);
	if (status != SQLITE_OK)
	{
		g_print (_("  Error while opening %s: %s\n"),
			db_file, sqlite3_errmsg (geoinfo_db));
		sqlite3_close (geoinfo_db);
		g_free (db_file);
		exit (EXIT_FAILURE);
	}
	g_free (db_file);


	/* create/open osm database file */
	if (osm_file == NULL)
		osm_file = g_strdup (DB_OSMFILE);
	g_print ("+ Creating osm database file: %s\n", osm_file);
	status = sqlite3_open (osm_file, &osm_db);
	if (status != SQLITE_OK)
	{
		g_print (_("  Error while opening %s: %s\n"),
			osm_file, sqlite3_errmsg (osm_db));
		sqlite3_close (geoinfo_db);
		g_free (osm_file);
		exit (EXIT_FAILURE);
	}
	g_free (osm_file);


	/* create table 'poi' in osm database file */
	status = sqlite3_exec(osm_db, sql_create_poitable, NULL, NULL, &error_string);
	if (status != SQLITE_OK )
	{
		g_print ("SQLite error: %s\n", error_string);
		sqlite3_free(error_string);
	}


	/* create table 'poi_extra' in osm database file */
	status = sqlite3_exec(osm_db, sql_create_poiextratable, NULL, NULL, &error_string);
	if (status != SQLITE_OK )
	{
		g_print ("SQLite error: %s\n", error_string);
		sqlite3_free(error_string);
	}


	/* read poi_types for matching osm types from gpsdrive geoinfo.db */
	g_print (_("+ Reading POI types from gpsdrive geoinfo database\n"));
	poitypes_hash = g_hash_table_new (g_str_hash, g_str_equal);
	status = sqlite3_exec (geoinfo_db, "SELECT key,value,poi_type FROM osm;",
		read_poi_types_cb, NULL, &error_string);
	if (status != SQLITE_OK )
	{
		g_print (_("  SQLite error: %s\n"), error_string);
		sqlite3_free(error_string);
		exit (EXIT_FAILURE);
	}
	g_print ("  %d known POI types found.\n", g_hash_table_size (poitypes_hash));
	sqlite3_close (geoinfo_db);

	/* start timer to show duration of parsing process */
		timer = g_timer_new ();


	/* parse xml file and write data into database */
	xml_reader = xmlNewTextReaderFilename (argv[1]);
	if (xml_reader != NULL)
	{
		g_print ("+ Parsing OSM file '%s'\n", argv[1]);
		status = xmlTextReaderRead (xml_reader);
		while (status == 1)
		{
			//if (!verbose)
			//	g_print ("\b%c", spinner[spinpos++%4]);
			g_print ("\r  %ld/%ld/%ld", poi_count, node_count, count);

			xml_name = xmlTextReaderName(xml_reader);
			if (xmlStrEqual(xml_name, BAD_CAST "node"))
			{
				if (xmlTextReaderNodeType(xml_reader) == XML_READER_TYPE_ELEMENT)
				{
					node_count++;
					parse_node_cb ();
				}
			}
			else
				count++;
			/* Usually the *.osm dump files are sorted in the
			 * order node/way/relation. So we can assume, that
			 * there will appear no more nodes after a 'way' or
			 * 'relation' element is found, and stop reading to
			 * speed up the processing */
			if (!nodes_done)
			{
				if (xmlStrEqual(xml_name, BAD_CAST "way")
				    || xmlStrEqual(xml_name, BAD_CAST "relation"))
				{
					nodes_done = TRUE;
					if (stop_on_way)
					{
						g_print ("\r  Reached end of nodes, terminating...\n");
						status = 0;
						break;
					}
					else
						g_print ("\n\n\nFOUND WAY OR RELATION TAG!\n"
							"END OF NODES SECTION REACHED???\n\n\n");
				}
			}
			if (xml_name)
				xmlFree (xml_name);
			status = xmlTextReaderRead (xml_reader);
        	}
        	xmlFreeTextReader (xml_reader);
        	if (status != 0)
        		g_print (_("  Failed to parse '%s'\n"), argv[1]);
	}
	else
	{
		g_print (_("\nERROR: Unable to open %s\n"), argv[1]);
		g_print (_("Please specify a valid OpenStreetMap XML file!\n"));
		exit (EXIT_FAILURE);
	}
	parsing_time = g_timer_elapsed (timer, NULL);
	if (parsing_time < 60)
		g_print (_("\r  %ld of %ld nodes identified as POI in %d seconds\n"),
			poi_count, node_count, parsing_time);
	else
		g_print (_("\r  %ld of %ld nodes identified as POI in %d:%2d minutes\n"),
			poi_count, node_count, parsing_time/60, parsing_time%60);


	/* create index on poi column */
//	g_print (_("+ Creating new index on column 'poi' (this may take some time...)\n"));
//	if (verbose)
//		g_timer_start (timer);

//	status = gda_execute_sql_command (db_conn_osm,
//		"CREATE INDEX poi_index ON planet_osm_point (poi);", NULL);
//	if (verbose)
//	{
//		g_print (_("  Index created in %.3f seconds\n"), g_timer_elapsed (timer, NULL));
//		g_timer_destroy (timer);
//	}

//	if (status > 0)
//		g_print ("  done.\n");

	/* clean up connections */
	g_print (_("+ Closing Database access\n"));
	sqlite3_close (osm_db);

	g_print (_("\nFinished.\n"));

	return EXIT_SUCCESS;
}
