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
Revision 1.2  2005/03/27 00:44:42  tweety
eperated poi_type_list and streets_type_list
and therefor renaming the fields
added drop index before adding one
poi.*: a little bit more error handling
disabling poi and streets if sql is disabled
changed som print statements from \n.... to ...\n
changed some debug statements from debug to mydebug

Revision 1.1  2005/02/13 14:06:54  tweety
start street randering functions. reading from the database streets and displayi
ng it on the screen
improve a little bit in the sql-queries
fixed linewidth settings in draw_cross

*/


#ifndef GPSDRIVE_STREETS_H
#define GPSDRIVE_STREETS_H

/*
 * See streets.c for details.
 */

#include <gtk/gtk.h>



typedef struct
{
  gint    wp_id;
  gdouble lon1;
  gdouble lat1;
  gdouble lon2;
  gdouble lat2;
  gdouble x1;    // x position on screen start
  gdouble y1;    // y position on screen
  gdouble x2;    // x position on screen end
  gdouble y2;    // y position on screen
  gchar   name[80];
  gdouble streets_type_id;    // type of street
}
streets_struct;

void streets_init (void);
void streets_rebuild_list (void);
void streets_draw_list (void);
gint streets_draw_cb (GtkWidget * widget, guint datum);


#endif /* GPSDRIVE_STREETS_H */
