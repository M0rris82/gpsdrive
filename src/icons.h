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
Revision 1.10  2006/05/05 22:18:08  tweety
move icons stred in memory to one array
fix size of icons drawn at poi.c
change list of default scales
don't calculate map offset if we only have vectormaps
remove some of the cvs logs in the source files. The can be retrieved from the cvs and
blow up the files so we have troubles using eclipse or something similar
move scale_min,scale_max to the streets_type and poi_type database
increase the LIMIT for the streets sql query
increase the rectangle for retreving streets from mysql for 0.01 degreees in each direction
Thieck_osm.pl more independent from gpsdrive datastructure
way we can get some of the lines where both endpoint are out of the viewing Window

Revision 1.9  2005/08/14 18:46:42  tweety
remove unnedded xpm Files; read pixmaps with read_icons,
separate more pixmaps from icons

Revision 1.8  2005/04/06 19:38:17  tweety
use disable/enable keys to improove spee in database creation
add draw_small_plus_sign, which is used if we would have too many waypoints to display
extract draw_text from draw_poi loop

Revision 1.7  2005/03/27 21:25:46  tweety
separating map_import from gpsdrive.c

Revision 1.6  2005/02/10 06:22:25  tweety
added primitive drawing of icons to POI

Revision 1.5  2005/02/08 09:01:48  tweety
move loading of usericons to icons.c

Revision 1.4  2005/02/08 08:43:46  tweety
wrong dfinition for auxicons array

Revision 1.3  2005/02/06 21:18:05  tweety
more cleanup: extracted more functionality to functions

Revision 1.2  2005/02/06 17:52:44  tweety
extract icon handling to icons.c

Revision 1.0  2005/02/02 18:11:02  tweety
*/

#ifndef GPSDRIVE_ICONS_H
#define GPSDRIVE_ICONS_H

/*
 * See icons.c for details.
 */

int drawicon (gint posxdest, gint posydest, char *ic);
void load_friends_icon (void);
void load_icons(void);
void load_user_icon(  char icon_name[200] );
void draw_plus_sign ( gdouble posxdest,   gdouble posydest );
void draw_small_plus_sign ( gdouble posxdest,   gdouble posydest );
GdkPixbuf * read_icon(char * icon_name);

typedef struct
{
  GdkPixbuf *icon;
  char name[40];
}
icons_buffer_struct;

#endif /* GPSDRIVE_ICONS_H */
