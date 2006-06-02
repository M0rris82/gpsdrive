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
GdkPixbuf *read_themed_icon (gchar * icon_name);
GdkPixbuf * read_icon(char * icon_name);

typedef struct
{
  GdkPixbuf *icon;
  char name[40];
}
icons_buffer_struct;

#endif /* GPSDRIVE_ICONS_H */
