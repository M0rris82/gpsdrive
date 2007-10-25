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
extern GtkWidget *main_window;

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
 gdouble bounds[4];
 gint wpt_count;
 gint rte_count;
 gint trk_count;
} gpx_info_struct;


gpx_info_struct gpx_info;


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
	fprintf (stderr, "Time        : %s\n", gpx_info.time);
	fprintf (stderr, "Keywords    : %s\n", gpx_info.keywords);
	fprintf (stderr, "Bounds      : %.6f / %.6f - %.6f / %.6f\n\n",
		gpx_info.bounds[0], gpx_info.bounds[1],
		gpx_info.bounds[2], gpx_info.bounds[3]);
	fprintf (stderr, "The file contains %d waypoints, %d routes, and %d tracks.\n",
		gpx_info.wpt_count, gpx_info.rte_count, gpx_info.trk_count);
	fprintf (stderr, "==================================================\n\n");
}


/* ******************************************************************
 * Get file info stored in gpx file.
 * Possible Nodes:
 *  version, creator, name, desc, author, email, url, urlname,
 *  time, keywords, bounds
 */
void gpx_handle_gpxinfo (xmlTextReaderPtr xml_reader, xmlChar *node_name)
{
	gint xml_status = 0;

	if (xmlStrEqual(node_name, BAD_CAST "gpx"))
	{
		g_snprintf(gpx_info.version, sizeof(gpx_info.version), "%s",
			xmlTextReaderGetAttribute(xml_reader, BAD_CAST "version"));
		g_snprintf(gpx_info.creator, sizeof(gpx_info.version), "%s",
			xmlTextReaderGetAttribute(xml_reader, BAD_CAST "creator"));
	}
	else if (xmlStrEqual(node_name, BAD_CAST "name"))
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

	/* reset gpx info structure */
	gpx_info.wpt_count = gpx_info.rte_count = gpx_info.trk_count = 0;
	gpx_info.bounds[0] = gpx_info.bounds[1] = gpx_info.bounds[2] = gpx_info.bounds[3] = 0;
	g_strlcpy (gpx_info.version, _("n/a"), sizeof (gpx_info.version));
	g_strlcpy (gpx_info.creator, _("n/a"), sizeof (gpx_info.creator));
	g_strlcpy (gpx_info.name, _("n/a"), sizeof (gpx_info.name));
	g_strlcpy (gpx_info.desc, _("n/a"), sizeof (gpx_info.desc));
	g_strlcpy (gpx_info.author, _("n/a"), sizeof (gpx_info.author));
	g_strlcpy (gpx_info.email, _("n/a"), sizeof (gpx_info.email));
	g_strlcpy (gpx_info.url, _("n/a"), sizeof (gpx_info.url));
	g_strlcpy (gpx_info.urlname, _("n/a"), sizeof (gpx_info.urlname));
	g_strlcpy (gpx_info.time, _("n/a"), sizeof (gpx_info.time));
	g_strlcpy (gpx_info.keywords, _("n/a"), sizeof (gpx_info.keywords));

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
					gpx_handle_wpt ();
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
					gpx_handle_rte ();
				else
				{
					gpx_info.rte_count++;
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
					gpx_info.trk_count++;
					while (!xmlStrEqual(node_name, BAD_CAST "trk") || node_type != NODE_END)
					{
						xml_status = xmlTextReaderRead(xml_reader);
						node_type = xmlTextReaderNodeType(xml_reader);
						node_name = xmlTextReaderName(xml_reader);
					}
				}
			}
			else
				gpx_handle_gpxinfo (xml_reader, node_name);
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


/* *****************************************************************************
 * Dialog: Load GPX File, including preview for gpx information
 */
gint loadgpx_cb (gint gpx_mode)
{
	GtkWidget *fdialog;
	GtkFileFilter *filter_gpx;

	fdialog = gtk_file_chooser_dialog_new (_("Select a GPX file"),
		GTK_WINDOW (main_window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);
	gtk_window_set_modal (GTK_WINDOW (fdialog), TRUE);
	filter_gpx = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (filter_gpx, "*.gpx");
	gtk_file_filter_add_pattern (filter_gpx, "*.GPX");
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fdialog), filter_gpx);

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
		test_gpx (gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (fdialog)));
	}

	gtk_widget_destroy (fdialog);
	return TRUE;
}
