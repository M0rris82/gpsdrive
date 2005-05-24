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
Revision 1.4  2005/05/24 08:35:25  tweety
move track splitting to its own function +sub track_add($)
a little bit more error handling
earth_distance somtimes had complex inumbers as result
implemented streets_check_if_moved_reset which is called when you toggle the draw streets button
this way i can re-read all currently displayed streets from the DB
fix minor array iindex counting bugs
add some content to the comment column

Revision 1.3  2005/03/27 21:25:46  tweety
separating map_import from gpsdrive.c

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
  gchar   comment[256];
  gdouble streets_type_id;    // type of street
}
streets_struct;

void streets_init (void);
void streets_rebuild_list (void);
void streets_draw_list (void);
gint streets_draw_cb (GtkWidget * widget, guint datum);
void streets_check_if_moved_reset (void);

#endif /* GPSDRIVE_STREETS_H */
