/***********************************************************************

Copyright (c) 2007 Guenther Meyer <d.s.e (at) sordidmusic.com>

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
 * module for gpx import and export
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

#include "gpsdrive.h"
#include "config.h"
#include "gettext.h"
#include <gpsdrive_config.h>
#include "gpx.h"
#include "gui.h"
#include "routes.h"
#include "track.h"

#include "gettext.h"
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint mydebug;
extern GtkWidget *main_window;
extern trackcoordstruct *trackcoord;
extern glong trackcoordnr;
extern guint id_timeout_track;

enum node_type
{
	NODE_START = 1,
	NODE_ATTR,
	NODE_TEXT,
	NODE_CDATA,
	NODE_ENTITY_REFERENCE,
	NODE_ENTITY_DECLARATION,
	NODE_PI,
	NODE_COMMENT,
	NODE_DOC,
	NODE_DTD,
	NODE_DOC_FRAGMENT,
	NODE_NOTATION,
	NODE_END = 15
};


typedef struct
{
 gchar version[255];
 gchar creator[255];
 gchar name[255];
 gchar desc[255];
 gchar author[255];
 gchar email[255];
 gchar url[255];
 gchar urlname[255];
 gchar urltype[255];
 gchar time[255];
 gchar keywords[255];
 gdouble bounds[4];
 gint wpt_count;
 gint rte_count;
 gint trk_count;
 gint rtept_count;
 gint trkpt_count;
 gint points;
} gpx_info_struct;

typedef struct
{
 xmlChar *name;
 xmlChar *cmt;
 xmlChar *type;
 xmlChar *time;
 gdouble lat;
 gdouble lon;
 gdouble ele;
} wpt_struct;

gpx_info_struct gpx_info;

void reset_gpx_info (void)
{
	gpx_info.wpt_count = gpx_info.rte_count = gpx_info.trk_count = 0;
	gpx_info.rtept_count = gpx_info.trkpt_count = gpx_info.points = 0;
	gpx_info.bounds[0] = gpx_info.bounds[1] = gpx_info.bounds[2] = gpx_info.bounds[3] = 0;
	g_strlcpy (gpx_info.version, _("n/a"), sizeof (gpx_info.version));
	g_strlcpy (gpx_info.creator, _("n/a"), sizeof (gpx_info.creator));
	g_strlcpy (gpx_info.name, _("n/a"), sizeof (gpx_info.name));
	g_strlcpy (gpx_info.desc, _("n/a"), sizeof (gpx_info.desc));
	g_strlcpy (gpx_info.author, _("n/a"), sizeof (gpx_info.author));
	g_strlcpy (gpx_info.email, _("n/a"), sizeof (gpx_info.email));
	g_strlcpy (gpx_info.url, _("n/a"), sizeof (gpx_info.url));
	g_strlcpy (gpx_info.urlname, _("n/a"), sizeof (gpx_info.urlname));
	g_strlcpy (gpx_info.urltype, _("n/a"), sizeof (gpx_info.urltype));
	g_strlcpy (gpx_info.time, _("n/a"), sizeof (gpx_info.time));
	g_strlcpy (gpx_info.keywords, _("n/a"), sizeof (gpx_info.keywords));
}


void test_gpx (gchar *filename)
{
	gpx_file_read (filename, GPX_INFO);

	fprintf (stdout, "\nTESTING GPX-FILE: %s\n", filename);
	fprintf (stderr, "==================================================\n");
	fprintf (stderr, "Version     : %s\n", gpx_info.version);
	fprintf (stderr, "Creator     : %s\n", gpx_info.creator);
	fprintf (stderr, "Name        : %s\n", gpx_info.name);
	fprintf (stderr, "Description : %s\n", gpx_info.desc);
	fprintf (stderr, "Author      : %s\n", gpx_info.author);
	fprintf (stderr, "Email       : %s\n", gpx_info.email);
	fprintf (stderr, "URL         : %s\n", gpx_info.url);
	fprintf (stderr, "URL Name    : %s\n", gpx_info.urlname);
	fprintf (stderr, "URL Type    : %s\n", gpx_info.urltype);
	fprintf (stderr, "Time        : %s\n", gpx_info.time);
	fprintf (stderr, "Keywords    : %s\n", gpx_info.keywords);
	fprintf (stderr, "Bounds      : %.6f / %.6f - %.6f / %.6f\n\n",
		gpx_info.bounds[0], gpx_info.bounds[1],
		gpx_info.bounds[2], gpx_info.bounds[3]);
	fprintf (stderr, "Waypoints: %d\nRoutes: %d (%d points)\nTracks: %d (%d Points)\n",
		gpx_info.wpt_count, gpx_info.rte_count, gpx_info.rtept_count,
		gpx_info.trk_count, gpx_info.trkpt_count);
	fprintf (stderr, "==================================================\n\n");
}


/* ******************************************************************
 * Get file info stored in gpx v1.1 format
 * Possible Nodes:
 *  name, desc, author (name, email), link (href, text, type),
 *  time, keywords, bounds
 */
static void gpx_handle_gpxinfo_v11 (xmlTextReaderPtr xml_reader, xmlChar *node_name)
{

	gint xml_status = 0;
	gint node_type = 0;

	if (mydebug > 20)
		fprintf (stderr, "trying to read gpx v1.1 file info\n");

	xml_status = xmlTextReaderRead(xml_reader);
	node_type = xmlTextReaderNodeType(xml_reader);
	node_name = xmlTextReaderName(xml_reader);

	while (!xmlStrEqual(node_name, BAD_CAST "metadata") || node_type != NODE_END)
	{
		xml_status = xmlTextReaderRead(xml_reader);
		node_type = xmlTextReaderNodeType(xml_reader);
		node_name = xmlTextReaderName(xml_reader);
		if (xmlStrEqual(node_name, BAD_CAST "name") && node_type == NODE_START)
		{
			xml_status = xmlTextReaderRead(xml_reader);
			g_snprintf(gpx_info.name, sizeof(gpx_info.name),
				"%s", xmlTextReaderConstValue(xml_reader));
		}
		else if (xmlStrEqual(node_name, BAD_CAST "desc") && node_type == NODE_START)
		{
			xml_status = xmlTextReaderRead(xml_reader);
			g_snprintf(gpx_info.desc, sizeof(gpx_info.desc),
				"%s", xmlTextReaderConstValue(xml_reader));
		}
		else if (xmlStrEqual(node_name, BAD_CAST "author") && node_type == NODE_START)
		{
			while (!xmlStrEqual(node_name, BAD_CAST "author") || node_type != NODE_END)
			{
				xml_status = xmlTextReaderRead(xml_reader);
				node_type = xmlTextReaderNodeType(xml_reader);
				node_name = xmlTextReaderName(xml_reader);
				if (xmlStrEqual(node_name, BAD_CAST "name") && node_type == NODE_START)
				{
					xml_status = xmlTextReaderRead(xml_reader);
					g_snprintf(gpx_info.author, sizeof(gpx_info.author), "%s",
						xmlTextReaderConstValue(xml_reader));
				}
				else if (xmlStrEqual(node_name, BAD_CAST "email") && node_type == NODE_START)
				{
					g_snprintf(gpx_info.email, sizeof(gpx_info.email), "%s@%s",
						xmlTextReaderGetAttribute (xml_reader, BAD_CAST "id"),
						xmlTextReaderGetAttribute (xml_reader, BAD_CAST "domain"));
				}
			}
		}
		else if (xmlStrEqual(node_name, BAD_CAST "link") && node_type == NODE_START)
		{
			while (!xmlStrEqual(node_name, BAD_CAST "link") || node_type != NODE_END)
			{
				g_snprintf(gpx_info.url, sizeof(gpx_info.url), "%s",
						xmlTextReaderGetAttribute (xml_reader, BAD_CAST "href"));
				xml_status = xmlTextReaderRead(xml_reader);
				node_type = xmlTextReaderNodeType(xml_reader);
				node_name = xmlTextReaderName(xml_reader);
				if (xmlStrEqual(node_name, BAD_CAST "text") && node_type == NODE_START)
				{
					xml_status = xmlTextReaderRead(xml_reader);
					g_snprintf(gpx_info.urlname, sizeof(gpx_info.urlname), "%s",
						xmlTextReaderConstValue(xml_reader));
				}
				else if (xmlStrEqual(node_name, BAD_CAST "type") && node_type == NODE_START)
				{
					xml_status = xmlTextReaderRead(xml_reader);
					g_snprintf(gpx_info.urltype, sizeof(gpx_info.urltype), "%s",
						xmlTextReaderConstValue(xml_reader));
				}
			}
		}
		else if (xmlStrEqual(node_name, BAD_CAST "time") && node_type == NODE_START)
		{
			xml_status = xmlTextReaderRead(xml_reader);
			g_snprintf(gpx_info.time, sizeof(gpx_info.time),
				"%s", xmlTextReaderConstValue(xml_reader));
		}
		else if (xmlStrEqual(node_name, BAD_CAST "keywords") && node_type == NODE_START)
		{
			xml_status = xmlTextReaderRead(xml_reader);
			g_snprintf(gpx_info.keywords, sizeof(gpx_info.keywords),
				"%s", xmlTextReaderConstValue(xml_reader));
		}
		else if (xmlStrEqual(node_name, BAD_CAST "bounds") && node_type == NODE_START)
		{
			gpx_info.bounds[0] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "minlat"), NULL);
			gpx_info.bounds[1] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "minlon"), NULL);
			gpx_info.bounds[2] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "maxlat"), NULL);
			gpx_info.bounds[3] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "maxlon"), NULL);
		}
	}
}


/* ******************************************************************
 * Get file info stored in gpx file.
 * Possible Nodes:
 *  name, desc, author, email, url, urlname,
 *  time, keywords, bounds
 */
static void gpx_handle_gpxinfo_v10 (xmlTextReaderPtr xml_reader, xmlChar *node_name)
{
	gint xml_status = 0;
	gint node_type = 0;

	if (mydebug > 20)
		fprintf (stderr, "trying to read gpx v1.0 file metadata\n");

	if (xmlStrEqual(node_name, BAD_CAST "name"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.name, sizeof(gpx_info.name),
			"%s", xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "desc"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.desc, sizeof(gpx_info.desc),
			"%s", xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "author"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.author, sizeof(gpx_info.author), "%s",
			xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "email"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.email, sizeof(gpx_info.email), "%s",
			xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "url"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.url, sizeof(gpx_info.url),
			"%s", xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "urlname"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.urlname, sizeof(gpx_info.urlname), "%s",
			xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "time"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.time, sizeof(gpx_info.time),
		"%s", xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "keywords"))
	{
		xml_status = xmlTextReaderRead(xml_reader);
		g_snprintf(gpx_info.keywords, sizeof(gpx_info.keywords),
			"%s", xmlTextReaderConstValue(xml_reader));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "bounds"))
	{
		gpx_info.bounds[0] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "minlat"), NULL);
		gpx_info.bounds[1] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "minlon"), NULL);
		gpx_info.bounds[2] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "maxlat"), NULL);
		gpx_info.bounds[3] = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "maxlon"), NULL);
	}

	else return;
}


/* ******************************************************************
 * Handle information stored in rare points (wpt, rtept, trkpt)
 * the possible nodes in all these sections are the same.
 * gpx_mode chooses, where the information will be stored.
 *
 * Nodes that are used:
 *  Route:	name, cmt, sym, type
 *  Track:	ele, time
 *  Waypoint:	
 *
 * Nodes that are (currently) NOT parsed:
 *  magvar, geoidheight, desc, src, url, urlname, fix,
 *  sat, hdop, vdop, pdop, ageofdgpsdata, dgpsid, course, speed,
 *  private tags and other namespaces
 */
static void gpx_handle_point (xmlTextReaderPtr xml_reader, gchar *mode_string)
{
	gint xml_status = 0;
	gint node_type = 0;
	xmlChar *node_name = NULL;

	wpt_struct wpt;
	wpt.name = NULL;
	wpt.cmt = NULL;
	wpt.type = NULL;
	wpt.lat = 0.0;
	wpt.lon = 0.0;

	if (mydebug > 20)
		fprintf (stderr, "gpx_handle_point: %s\n", mode_string);

	wpt.lat = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "lat"), NULL);
	wpt.lon = g_strtod ((gpointer) xmlTextReaderGetAttribute(xml_reader, BAD_CAST "lon"), NULL);

	gpx_info.points++;

	while (!xmlStrEqual(node_name, BAD_CAST mode_string) || node_type != NODE_END)
	{
		xml_status = xmlTextReaderRead(xml_reader);
		node_type = xmlTextReaderNodeType(xml_reader);
		node_name = xmlTextReaderName(xml_reader);

		if (node_type == NODE_START)
		{
			if (xmlStrEqual(node_name, BAD_CAST "name"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				wpt.name = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "cmt"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				wpt.cmt = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "sym") && wpt.type == NULL)
			{
				xml_status = xmlTextReaderRead(xml_reader);
				wpt.type = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "type"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				if (wpt.type)
					xmlFree (wpt.type);
				wpt.type = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "ele"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				wpt.ele = g_strtod ((gpointer) xmlTextReaderValue(xml_reader), NULL);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "time"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				wpt.time = xmlTextReaderValue(xml_reader);
			}
		}
	}

	if (mydebug > 30)
	{
		fprintf (stderr, "(%d)\t%.6f / %.6f\n", gpx_info.points, wpt.lat, wpt.lon);
		fprintf (stderr, "\tName     : %s\n", wpt.name);
		fprintf (stderr, "\tComment  : %s\n", wpt.cmt);
		fprintf (stderr, "\tType     : %s\n", wpt.type);
		fprintf (stderr, "\tElevation: %s\n", wpt.ele);
		fprintf (stderr, "\tTime     : %s\n", wpt.time);
	}

	if (g_ascii_strcasecmp (mode_string, "wpt") == 0)
	{
		//TODO: do something...
		fprintf (stdout, "Waypoint import not implemented yet!\n");
	}
	else if (g_ascii_strcasecmp (mode_string, "rtept") == 0)
	{
		add_arbitrary_point_to_route
			((gchar*)wpt.name, (gchar*)wpt.cmt, (gchar*)wpt.type, wpt.lat, wpt.lon);
	}

	else if (g_ascii_strcasecmp (mode_string, "trkpt") == 0)
	{
		add_trackpoint (wpt.lat, wpt.lon, wpt.ele, wpt.time);
	}

	if (node_name)
		xmlFree (node_name);
	if (wpt.name)
		xmlFree (wpt.name);
	if (wpt.cmt)
		xmlFree (wpt.cmt);
	if (wpt.type)
		xmlFree (wpt.type);
	if (wpt.time)
		xmlFree (wpt.time);
}


/* ******************************************************************
 * Handle information stored inside rte or trk section
 * besides the different childs, the possible nodes are the same.
 * gpx_mode chooses, where the information will be stored, and which
 * childs will be parsed.
 */
static void gpx_handle_rte_trk (xmlTextReaderPtr xml_reader, const gchar *mode_string)
{
	gint xml_status = 0;
	gint node_type = 0;
	xmlChar *node_name = NULL;
	xmlChar *t_name = NULL;
	xmlChar *t_cmt = NULL;
	xmlChar *t_desc = NULL;
	xmlChar *t_src = NULL;
	xmlChar *t_url = NULL;
	xmlChar *t_urlname = NULL;
	xmlChar *t_number = NULL;

	if (mydebug > 20)
		fprintf (stderr, "gpx_handle_rte_trk: Parsing <%s>\n", mode_string);

	while (!xmlStrEqual(node_name, BAD_CAST mode_string) || node_type != NODE_END)
	{
		xml_status = xmlTextReaderRead(xml_reader);
		node_type = xmlTextReaderNodeType(xml_reader);
		node_name = xmlTextReaderName(xml_reader);

		if (node_type == NODE_START)
		{
			if (xmlStrEqual(node_name, BAD_CAST "rtept"))
				gpx_handle_point (xml_reader, "rtept");
			if (xmlStrEqual(node_name, BAD_CAST "trkpt"))
				gpx_handle_point (xml_reader, "trkpt");
			else if (xmlStrEqual(node_name, BAD_CAST "trkseg"))
			{
				if (xmlTextReaderIsEmptyElement (xml_reader) != 1)
				{
					gpx_handle_rte_trk (xml_reader, "trkseg");
					add_trackpoint (1001.0, 1001.0, 1001.0, NULL);
				}
			}	
			else if (xmlStrEqual(node_name, BAD_CAST "name"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				t_name = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "cmt"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				t_cmt = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "desc"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				t_desc = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "src"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				t_src = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "url"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				t_url = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "urlname"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				t_urlname = xmlTextReaderValue(xml_reader);
			}
			else if (xmlStrEqual(node_name, BAD_CAST "number"))
			{
				xml_status = xmlTextReaderRead(xml_reader);
				t_number = xmlTextReaderValue(xml_reader);
			}
		}
	}
	
	if (node_name)
		xmlFree (node_name);
	if (t_name)
		xmlFree (t_name);
	if (t_cmt)
		xmlFree (t_cmt);
	if (t_desc)
		xmlFree (t_desc);
	if (t_src)
		xmlFree (t_src);
	if (t_url)
		xmlFree (t_url);
	if (t_urlname)
		xmlFree (t_urlname);
	if (t_number)
		xmlFree (t_number);
}


/* ******************************************************************
 * Read a given gpx file, gpx_mode sets the type of data to read.
 * Possible values: GPX_INFO, GPX_WPT, GPX_RTE, GPX_TRK
 */
gint gpx_file_read (gchar *gpx_file, gint gpx_mode)
{
	xmlTextReaderPtr xml_reader;
	gint xml_status = 0; // 0 = empty, 1 = node, -1 = error
	gint node_type = 0;
	xmlChar *node_name = NULL;

	if (mydebug > 10)
		fprintf (stdout, "gpx_file_read (%s, %d)\n", gpx_file, gpx_mode);

	xml_reader = xmlNewTextReaderFilename(gpx_file);

	if (xml_reader == NULL)
	{
		fprintf (stderr, "gpx_file_read: File %s not found!\n", gpx_file);
		return FALSE;
	}

	switch (gpx_mode)
	{
		case GPX_INFO:
			reset_gpx_info (); break;
		case GPX_RTE:
			route_init (gpx_info.name, gpx_info.desc, NULL); break;
		case GPX_WPT:
			//TODO: do something...
			break;
		case GPX_TRK:
			init_track (TRUE); break;
	}

	/* parse complete gpx file */
	xml_status = xmlTextReaderRead(xml_reader);
	while (xml_status == 1)
	{
		node_type = xmlTextReaderNodeType(xml_reader);
		node_name = xmlTextReaderName(xml_reader);
		
		if (node_type == NODE_START)
		{
			if (xmlStrEqual(node_name, BAD_CAST "wpt"))
			{
				if (gpx_mode == GPX_WPT)
					gpx_handle_point (xml_reader, "wpt");
				else
				{
					gpx_info.wpt_count++;
					while (!xmlStrEqual(node_name, BAD_CAST "wpt") || node_type != NODE_END)
					{
						xml_status = xmlTextReaderRead(xml_reader);
						node_type = xmlTextReaderNodeType(xml_reader);
						node_name = xmlTextReaderName(xml_reader);
					}
				}
			}
			else if (xmlStrEqual(node_name, BAD_CAST "rte"))
			{
				if (gpx_mode == GPX_RTE)
					gpx_handle_rte_trk (xml_reader, "rte");
				else
				{
					gpx_info.rte_count++;
					while (!xmlStrEqual(node_name, BAD_CAST "rte") || node_type != NODE_END)
					{
						xml_status = xmlTextReaderRead(xml_reader);
						node_type = xmlTextReaderNodeType(xml_reader);
						node_name = xmlTextReaderName(xml_reader);
						if (gpx_mode == GPX_INFO)
						{
							if (xmlStrEqual(node_name, BAD_CAST "rtept")
								&& node_type == NODE_START)
								gpx_info.rtept_count++;
						}
					}
				}
			}
			else if (xmlStrEqual(node_name, BAD_CAST "trk"))
			{
				if (gpx_mode == GPX_TRK)
				{
					gpx_handle_rte_trk (xml_reader, "trk");
					rebuildtracklist ();
				}
				else
				{
					gpx_info.trk_count++;
					while (!xmlStrEqual(node_name, BAD_CAST "trk") || node_type != NODE_END)
					{
						xml_status = xmlTextReaderRead(xml_reader);
						node_type = xmlTextReaderNodeType(xml_reader);
						node_name = xmlTextReaderName(xml_reader);
						if (gpx_mode == GPX_INFO)
						{
							if (xmlStrEqual(node_name, BAD_CAST "trkpt")
								&& node_type == NODE_START)
								gpx_info.trkpt_count++;
						}
					}
				}
			}
			else if (xmlStrEqual(node_name, BAD_CAST "gpx"))
			{
				g_snprintf(gpx_info.version, sizeof(gpx_info.version), "%s",
					xmlTextReaderGetAttribute(xml_reader, BAD_CAST "version"));
				g_snprintf(gpx_info.creator, sizeof(gpx_info.version), "%s",
					xmlTextReaderGetAttribute(xml_reader, BAD_CAST "creator"));
			}
			else if (xmlStrEqual(node_name, BAD_CAST "metadata"))
			{
				/* in gpx v1.1 all file metainfo should be grouped
				 * inside the element <metadata> */
				gpx_handle_gpxinfo_v11 (xml_reader, node_name);
			}
			else
			{
				/* in gpx v1.0 all file metainfo is somewhere
				 * directly under the <gpx> root element */
				gpx_handle_gpxinfo_v10 (xml_reader, node_name);
			}
		}
		xml_status = xmlTextReaderRead(xml_reader);
	}

	xmlFreeTextReader(xml_reader);
	if (node_name)
		xmlFree (node_name);

	if (xml_status != 0)
	{
		fprintf(stderr, "gpx_file_read: Failed to parse file: %s\n", gpx_file);
		return FALSE;
	}

	return TRUE;
}


/* ******************************************************************
 * Write data to a gpx file, gpx_mode sets the type of data to write.
 * Possible Values: GPX_WPT, GPX_RTE, GPX_TRK
 */
gint gpx_file_write (gchar *gpx_file, gint gpx_mode)
{
	xmlTextWriterPtr xml_writer;
	gint count = 0;
	gint i = 0;
	gboolean j = TRUE;

	/* init xml writer with given filename */
	xml_writer = xmlNewTextWriterFilename(gpx_file, FALSE);
	if (xml_writer == NULL)
	{
		fprintf(stderr, "gpx_file_write: Error creating xml writer for file %s\n", gpx_file);
		return FALSE;
	}

	/* start the document */
	xmlTextWriterStartDocument (xml_writer, NULL, "UTF-8", NULL);

	/* write root element <gpx> and add metadata */
	xmlTextWriterStartElement (xml_writer, BAD_CAST "gpx");
	xmlTextWriterWriteAttribute (xml_writer, BAD_CAST "version", BAD_CAST "1.1");
	xmlTextWriterWriteAttribute (xml_writer, BAD_CAST "creator", BAD_CAST gpx_info.creator);
	xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
	xmlTextWriterStartElement (xml_writer, BAD_CAST "metadata");
	xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
	if (strlen (gpx_info.name) > 0)
	{
		xmlTextWriterWriteElement(xml_writer, BAD_CAST "name", BAD_CAST gpx_info.name);
		xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
	}
	if (strlen (gpx_info.desc) > 0)
	{
		xmlTextWriterWriteElement(xml_writer, BAD_CAST "desc", BAD_CAST gpx_info.desc);
		xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
	}
	if (strlen (gpx_info.keywords) > 0)
	{
		xmlTextWriterWriteElement(xml_writer, BAD_CAST "keywords", BAD_CAST gpx_info.keywords);
		xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
	}
	if (strlen (gpx_info.author) > 0 || strlen (gpx_info.email) > 0)
	{
		gchar *email_at;
		
		xmlTextWriterStartElement(xml_writer, BAD_CAST "author");
		xmlTextWriterWriteElement(xml_writer, BAD_CAST "name", BAD_CAST gpx_info.author);
		email_at = g_strrstr (gpx_info.email, "@");
		if (email_at)
		{
			*email_at = '\0';
			xmlTextWriterStartElement (xml_writer, BAD_CAST "email");
			xmlTextWriterWriteAttribute (xml_writer, BAD_CAST "id", BAD_CAST gpx_info.email);
			xmlTextWriterWriteAttribute (xml_writer, BAD_CAST "domain", BAD_CAST (email_at+1));
			xmlTextWriterEndElement(xml_writer); /* email */
			*email_at = '@';
		}
		xmlTextWriterEndElement(xml_writer); /* author */
		xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
	}
	xmlTextWriterWriteElement(xml_writer, BAD_CAST "time", BAD_CAST gpx_info.time);
	xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
	xmlTextWriterEndElement(xml_writer); /* metadata */
	xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");

	/* write points data */
	switch (gpx_mode)
	{
		case GPX_RTE:
			xmlTextWriterStartElement(xml_writer, BAD_CAST "rte");
			{
				xmlTextWriterStartElement(xml_writer, BAD_CAST "rtept");
			
				xmlTextWriterEndElement(xml_writer); /* rtept */
				count++;
			}
			
			xmlTextWriterEndElement(xml_writer); /* rte */
			break;

		case GPX_TRK:
			/* remove timeout function to prevent adding new
			 * trackpoints while we are writing */
			if (g_source_remove (id_timeout_track))
				id_timeout_track = 0;

			xmlTextWriterStartElement(xml_writer, BAD_CAST "trk");
			xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
			xmlTextWriterStartElement(xml_writer, BAD_CAST "trkseg");
			xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
			for (i = 0; i < trackcoordnr; i++)
			{
				if (mydebug > 20)
				{
					fprintf (stderr,
						"%3d | lat: %.6f | lon: %.6f | alt: %.1f | time: %s\n",
						i, (trackcoord + i)->lat, (trackcoord + i)->lon,
						(trackcoord + i)->alt, (trackcoord + i)->postime);
				}
			
				if ((trackcoord + i)->lon > 1000.0)
				{
					if (j == FALSE)
					{
						xmlTextWriterEndElement(xml_writer); /* trkseg */
						xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
						xmlTextWriterStartElement(xml_writer, BAD_CAST "trkseg");
						xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
						j = TRUE;
					}
				}
				else
				{
					xmlTextWriterStartElement(xml_writer, BAD_CAST "trkpt");
					xmlTextWriterWriteFormatAttribute (xml_writer,
						BAD_CAST "lat", "%.6f", (trackcoord + i)->lat);
					xmlTextWriterWriteFormatAttribute (xml_writer,
						BAD_CAST "lon", "%.6f", (trackcoord + i)->lon);
					if ((trackcoord + i)->alt < 1000.0)
					{
						xmlTextWriterWriteFormatElement(xml_writer,
							BAD_CAST "ele", "%.1f", (trackcoord + i)->alt);
					}
					if (strlen ((trackcoord + i)->postime))
					{
						xmlTextWriterWriteElement(xml_writer, BAD_CAST "time",
							BAD_CAST (trackcoord + i)->postime);
					}
					xmlTextWriterEndElement(xml_writer); /* trkpt */
					xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
					j = FALSE;
					count++;
				}
			}
			xmlTextWriterEndElement(xml_writer); /* trkseg */
			xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
			xmlTextWriterEndElement(xml_writer); /* trk */
			xmlTextWriterWriteString (xml_writer, BAD_CAST "\n");
			
			/* add timeout function again to continue track recording */
			id_timeout_track = g_timeout_add (1000, (GtkFunction) storetrack_cb, 0);
			break;

		case GPX_WPT:
			
			{
				xmlTextWriterStartElement(xml_writer, BAD_CAST "wpt");
			
				xmlTextWriterEndElement(xml_writer); /* wpt */
				count++;
			}
			
			break;
	}

	/* end the document, this also closes all elements that are still open */
	if (xmlTextWriterEndDocument (xml_writer) < 0)
		fprintf (stderr, "gpx_file_write: Error creating file %s\n", gpx_file);
	else if (mydebug > 0)
		fprintf (stderr, "GPX-File %s with %d points written\n", gpx_file, count);

	xmlFreeTextWriter (xml_writer);
	return TRUE;
}


/* *****************************************************************************
 * Callback for toggling of gpx info display in the file dialog
 */
static void
toggle_file_info_cb (GtkWidget *button, GtkWidget *dialog)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
		gtk_file_chooser_set_preview_widget_active
			(GTK_FILE_CHOOSER (dialog), TRUE);
	else
		gtk_file_chooser_set_preview_widget_active
		(GTK_FILE_CHOOSER (dialog), FALSE);
}


/* *****************************************************************************
 * Callback for updating the displayed gpx info in the file dialog
 */
static void
update_file_info_cb (GtkWidget *dialog, GtkWidget *label)
{
	gchar buf[2048];
	gchar *filename;

	filename = gtk_file_chooser_get_preview_filename (GTK_FILE_CHOOSER (dialog));
	if (g_file_test (filename, G_FILE_TEST_IS_REGULAR) == FALSE)
	{
		g_snprintf (buf, sizeof (buf), _("Please select a file"));
	}
	else if (gpx_file_read (filename, GPX_INFO))
	{
		g_snprintf (buf, sizeof (buf),
			_("<b>Name:</b>\n<i>%s</i>\n<b>Description:</b>\n<i>%s</i>\n"
			"<b>Author</b>:\n<i>%s</i>\n<b>created by:</b>\n<i>%s</i>\n"
			"<b>Timestamp:</b>\n<i>%s</i>\n<b>Waypoints:</b>\n<i>%d</i>\n"
			"<b>Routes:</b>\n<i>%d (%d)</i>\n<b>Tracks:</b>\n<i>%d (%d)</i>\n"),
			gpx_info.name, gpx_info.desc, gpx_info.author,
			gpx_info.creator, gpx_info.time, gpx_info.wpt_count,
			gpx_info.rte_count, gpx_info.rtept_count,
			gpx_info.trk_count, gpx_info.trkpt_count);
	}
	else
	{
		g_snprintf (buf, sizeof (buf), "no valid GPX file");
	}
	gtk_label_set_markup (GTK_LABEL (label), buf);
	g_free (filename);
}


/* *****************************************************************************
 * Callback for evaluating data from input fields in dialog_get_gpx_info
 */
static void
get_gpx_info_cb
	(GtkWidget *name, GtkWidget *desc, GtkWidget *keyw, GtkWidget *author, GtkWidget *email)
{
	g_strlcpy (gpx_info.name, gtk_entry_get_text (GTK_ENTRY (name)), sizeof (gpx_info.name));
	g_strlcpy (gpx_info.desc, gtk_entry_get_text (GTK_ENTRY (desc)), sizeof (gpx_info.desc));
	g_strlcpy (gpx_info.keywords, gtk_entry_get_text (GTK_ENTRY (keyw)), sizeof (gpx_info.keywords));
	g_strlcpy (gpx_info.author, gtk_entry_get_text (GTK_ENTRY (author)), sizeof (gpx_info.author));
	g_strlcpy (gpx_info.email, gtk_entry_get_text (GTK_ENTRY (email)), sizeof (gpx_info.email));
}


/* *****************************************************************************
 * Dialog for entering additional information when saving a gpx file
 */
static void
dialog_get_gpx_info (gint gpx_mode)
{
	GtkWidget *info_dialog, *info_table, *info_name_lb, *info_name_entry;
	GtkWidget *info_desc_lb, *info_desc_entry, *info_keyw_lb, *info_keyw_entry;
	GtkWidget *info_author_lb, *info_author_entry, *info_email_lb, *info_email_entry;
	GTimeVal current_time;

	g_get_current_time (&current_time);

	gpx_info.bounds[0] = gpx_info.bounds[1] = gpx_info.bounds[2] = gpx_info.bounds[3] = 0;
	g_strlcpy (gpx_info.version, _("1.1"), sizeof (gpx_info.version));
	g_snprintf (gpx_info.creator, sizeof (gpx_info.creator),
		"GpsDrive %s - http://www.gpsdrive.de", PACKAGE_VERSION);
	g_strlcpy (gpx_info.name, "", sizeof (gpx_info.name));
	g_strlcpy (gpx_info.desc, "", sizeof (gpx_info.desc));
	g_strlcpy (gpx_info.keywords, "", sizeof (gpx_info.keywords));
	g_strlcpy (gpx_info.time, g_time_val_to_iso8601 (&current_time), sizeof (gpx_info.time));
	g_strlcpy (gpx_info.author, "", sizeof (gpx_info.author));
	g_strlcpy (gpx_info.email, "", sizeof (gpx_info.email));

	info_dialog = gtk_dialog_new_with_buttons (
		_("Additional Info"), GTK_WINDOW (main_window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		NULL);

	info_name_lb = gtk_label_new (NULL);
	switch (gpx_mode)
	{
		case GPX_RTE:
			gtk_label_set_markup (GTK_LABEL (info_name_lb), _("<b>Route Name</b>")); break;
		case GPX_TRK:
			gtk_label_set_markup (GTK_LABEL (info_name_lb), _("<b>Track Name</b>")); break;
		default:
			gtk_label_set_markup (GTK_LABEL (info_name_lb), _("<b>Short Info</b>")); break;
	}	
	info_name_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (info_name_entry), 100);

	info_desc_lb = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (info_desc_lb), _("<b>Description</b>"));
	info_desc_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (info_desc_entry), 255);

	info_keyw_lb = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (info_keyw_lb), _("<b>Keywords</b>"));
	info_keyw_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (info_keyw_entry), 255);

	info_author_lb = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (info_author_lb), _("<b>Author Name</b>"));
	info_author_entry = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY (info_author_entry), g_get_real_name ());
	gtk_entry_set_max_length (GTK_ENTRY (info_author_entry), 100);

	info_email_lb = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (info_email_lb), _("<b>Author Email</b>"));
	info_email_entry = gtk_entry_new ();
	gtk_entry_set_max_length (GTK_ENTRY (info_email_entry), 100);

	info_table = gtk_table_new (5, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (info_table), 5);
	gtk_table_set_col_spacings (GTK_TABLE (info_table), 5);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_name_lb, 0, 1, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_name_entry, 1, 2, 0, 1);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_desc_lb, 0, 1, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_desc_entry, 1, 2, 1, 2);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_author_lb, 0, 1, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_author_entry, 1, 2, 2, 3);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_email_lb, 0, 1, 3, 4);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_email_entry, 1, 2, 3, 4);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_keyw_lb, 0, 1, 4, 5);
	gtk_table_attach_defaults (GTK_TABLE (info_table),
		info_keyw_entry, 1, 2, 4, 5);

	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(info_dialog)->vbox), info_table);
	gtk_widget_show_all (info_table);

	if (gtk_dialog_run (GTK_DIALOG (info_dialog)) == GTK_RESPONSE_ACCEPT)
	{
		get_gpx_info_cb (info_name_entry, info_desc_entry, info_keyw_entry,
			info_author_entry, info_email_entry);
	}

	gtk_widget_destroy (info_dialog);
}


/* *****************************************************************************
 * Dialog: Load GPX File, including preview for gpx information
 */
gint loadgpx_cb (gint gpx_mode)
{
	GtkWidget *fdialog;
	GtkFileFilter *filter_gpx;
	GtkWidget *info_bt, *info_lb;

	/* create filedialog with gpx filter */
	fdialog = gtk_file_chooser_dialog_new (_("Select a GPX file"),
		GTK_WINDOW (main_window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
	gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (fdialog),
		local_config.dir_home, NULL);
	filter_gpx = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter_gpx, "*.gpx");
	gtk_file_filter_add_pattern (filter_gpx, "*.GPX");
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fdialog), filter_gpx);

	/* create widget for additional file info */
	info_lb = gtk_label_new (NULL);
	gtk_label_set_width_chars (GTK_LABEL (info_lb), 30);
	gtk_label_set_line_wrap (GTK_LABEL (info_lb), TRUE);
	gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (fdialog), info_lb);
	gtk_file_chooser_set_preview_widget_active (GTK_FILE_CHOOSER (fdialog), FALSE);
	gtk_file_chooser_set_use_preview_label (GTK_FILE_CHOOSER (fdialog), FALSE);
	g_signal_connect (fdialog, "update-preview",
		G_CALLBACK (update_file_info_cb), info_lb);
	info_bt = gtk_check_button_new_with_label (_("Show Info"));
	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (fdialog), info_bt);
	g_signal_connect (info_bt, "toggled",
		G_CALLBACK (toggle_file_info_cb), fdialog);

	/* preset directory depending on wanted data */
	switch (gpx_mode)
	{
		case GPX_RTE:
			gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (fdialog), local_config.dir_routes);
			break;
		case GPX_TRK:
			gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (fdialog), local_config.dir_tracks);
			break;
		default:
			gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (fdialog), local_config.dir_home);
			break;
	}

	if (gtk_dialog_run (GTK_DIALOG (fdialog)) == GTK_RESPONSE_ACCEPT)
	{
		if (mydebug >10)
			test_gpx (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fdialog)));

		gpx_file_read (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fdialog)), gpx_mode);
	}

	gtk_widget_destroy (fdialog);
	return TRUE;
}


/* *****************************************************************************
 * Dialog: Save GPX File, including editing of file properties
 */
gint savegpx_cb (gint gpx_mode)
{
	GtkWidget *fdialog;
	GtkFileFilter *filter_gpx;

	/* create filedialog with gpx filter */
	fdialog = gtk_file_chooser_dialog_new (_("Enter a GPX filename"),
		GTK_WINDOW (main_window),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
	gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (fdialog),
		local_config.dir_home, NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (fdialog), TRUE);
	filter_gpx = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter_gpx, "*.gpx");
	gtk_file_filter_add_pattern (filter_gpx, "*.GPX");
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fdialog), filter_gpx);

	/* preset directory depending on wanted data */
	switch (gpx_mode)
	{
		case GPX_RTE:
			gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (fdialog), local_config.dir_routes);
			break;
		case GPX_TRK:
			gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (fdialog), local_config.dir_tracks);
			break;
		default:
			gtk_file_chooser_set_current_folder
			(GTK_FILE_CHOOSER (fdialog), local_config.dir_home);
			break;
	}

	if (gtk_dialog_run (GTK_DIALOG (fdialog)) == GTK_RESPONSE_ACCEPT)
	{
		dialog_get_gpx_info (gpx_mode);
		gpx_file_write (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fdialog)), gpx_mode);
	}

	gtk_widget_destroy (fdialog);
	return TRUE;
}
