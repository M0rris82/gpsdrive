/***********************************************************************

Copyright (c) 2001,2002 Fritz Ganter <ganter@ganter.at>

Website: www.kraftvoll.at/software

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
$Log$
Revision 1.4  2005/03/27 00:44:42  tweety
eperated poi_type_list and streets_type_list
and therefor renaming the fields
added drop index before adding one
poi.*: a little bit more error handling
disabling poi and streets if sql is disabled
changed som print statements from \n.... to ...\n
changed some debug statements from debug to mydebug

Revision 1.3  2005/02/22 08:18:51  tweety
change leveing system to simpler scale marking for decission what to show on display
column_names(DBFuncs.pm get data from Database
added functions add_index drop_index
added language to type Database
for some Data split unpack and mirror Directories
for some add lat/lon min/max to get faster import for testing
added POI::DBFuncs::segments_add; this will later be the point to do some excerptions and combinations
on the street data

Revision 1.2  2005/02/07 07:53:39  tweety
added check_if_moved inti function poi_rebuild_list

Revision 1.1  2005/02/02 18:11:02  tweety
Add Point Of Interrest Support with mySQL

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.3  2003/05/31 20:12:35  ganter
new UDP friendsserver build in, needs some work

*/


#ifndef GPSDRIVE_POI_H
#define GPSDRIVE_POI_H

/*
 * See poi.c for details.
 */

#include <gtk/gtk.h>



typedef struct
{
  gint    wp_id;
  gdouble lon;
  gdouble lat;
  gdouble alt;
  gchar   name[80];
  gint    poi_type_id; 
  gdouble proximity;
  gchar   comment[255]; 
  gint    scale_min;  
  gint    scale_max;  
  //date  last_modified  
  gchar   url[160]; 
  gint    address_id;
  gint    source_id; 
  gdouble    x;    // x position on screen
  gdouble    y;    // y position on screen
}
poi_struct;

void poi_init (void);
void poi_rebuild_list (void);
void poi_draw_list (void);
gint poi_draw_cb (GtkWidget * widget, guint datum);


#endif /* GPSDRIVE_POI_H */
