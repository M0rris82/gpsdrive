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


#ifndef GPSDRIVE_DATABASE_H
#define GPSDRIVE_DATABASE_H

/*
 * See database.c for details.
 */


gboolean db_init (void);
void db_close (void);

gchar *escape_sql_string (const gchar *data);

void db_get_visible_poi_data (void);
void db_get_all_poi_types (gpointer callback, guint count[]);
void db_poi_extra_get_all (glong *poi_id, gpointer data);
void db_poi_get (gchar *query, gpointer callback, gint database);
gint db_poi_delete (gint index);
void db_get_visible_poi_types (gchar *filter);
gint db_cleanup_route (void);
glong db_poi_edit (glong poi_id, double lat, double lon, gchar *name,
		   gchar *typ, gchar *comment, gint src, gboolean update);
glong db_poi_extra_edit (glong *poi_id, gchar *field_name,
			 const gchar *field_entry, gboolean update);
glong db_poi_extra_get (glong *poi_id, gchar *field_name,
			gchar *field_entry, gchar *result);


enum
{
	DB_GEOINFO,
	DB_WP_USER,
	DB_WP_OSM,
};


#endif /* GPSDRIVE_DATABASE_H */
