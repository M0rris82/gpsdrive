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

typedef struct
{
  GdkPixbuf *icon;
  char name[40];
}
auxiconsstruct;

#endif /* GPSDRIVE_ICONS_H */
