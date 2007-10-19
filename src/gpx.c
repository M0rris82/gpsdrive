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

#include "gettext.h"
#include <libxml/xmlreader.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern gint mydebug;


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
 gchar time[255];
 gchar keywords[255];
 gchar bounds[255];
} gpx_info_struct;


void gpx_handle_gpxinfo ()
{


/*
	xmlTextReaderGetAttribute(xml_reader, BAD_CAST "k")

	if (tag_type == 3)
	{
		g_snprintf(t_scale_max, sizeof(t_scale_max), "%s", xmlTextReaderConstValue(xml_reader));
		continue;
	}
*/


}


void gpx_handle_wpt ()
{

}


void gpx_handle_rte ()
{

}


void gpx_handle_trk ()
{

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

	xml_status = xmlTextReaderRead(xml_reader);
	while (xml_status == 1) /* parse complete file */
	{
		node_type = xmlTextReaderNodeType(xml_reader);
		node_name = xmlTextReaderName(xml_reader);
		
		if (node_type == NODE_START)
		{
			if (xmlStrEqual(node_name, BAD_CAST "wpt"))
			{
				if (gpx_mode == GPX_WPT)
					gpx_handle_wpt ();
				else
				{
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
					gpx_handle_rte ();
				else
				{
					while (!xmlStrEqual(node_name, BAD_CAST "rte") || node_type != NODE_END)
					{
						xml_status = xmlTextReaderRead(xml_reader);
						node_type = xmlTextReaderNodeType(xml_reader);
						node_name = xmlTextReaderName(xml_reader);
					}
				}
			}
			else if (xmlStrEqual(node_name, BAD_CAST "trk"))
			{
				if (gpx_mode == GPX_TRK)
					gpx_handle_trk ();
				else
				{
					while (!xmlStrEqual(node_name, BAD_CAST "trk") || node_type != NODE_END)
					{
						xml_status = xmlTextReaderRead(xml_reader);
						node_type = xmlTextReaderNodeType(xml_reader);
						node_name = xmlTextReaderName(xml_reader);
					}
				}
			}
			else
				gpx_handle_gpxinfo ();
		}
		xml_status = xmlTextReaderRead(xml_reader);
	}

	if (xml_status != 0)
		fprintf(stderr, "gpx_file_read: Failed to parse file: %s\n", gpx_file);

	xmlFreeTextReader(xml_reader);
	if (node_name)
		xmlFree (node_name);

	return TRUE;
}


/* ******************************************************************
 * Write data to a gpx file, gpx_mode sets the type of data to write.
 * Possible Values: GPX_WPT, GPX_RTE, GPX_TRK
 */
gint gpx_file_write (gchar *gpx_file, gint gpx_mode)
{

	return TRUE;
}

