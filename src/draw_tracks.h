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
Revision 1.1  2005/08/14 10:05:24  tweety
added new Files src/draw_tracks.?

Revision 1.0  2005/05/24 08:35:25  tweety

*/


#ifndef GPSDRIVE_DRAW_TRACK_H
#define GPSDRIVE_DRAW_TRACK_H

/*
 * See track.c for details.
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
  gdouble track_type_id;    // type of track
}
tracks_struct;

void tracks_init (void);
void tracks_rebuild_list (void);
void tracks_draw_list (void);
gint tracks_draw_cb (GtkWidget * widget, guint datum);
void tracks_check_if_moved_reset (void);
void tracks_query_area ( gdouble lat1, gdouble lon1 ,gdouble lat2, gdouble lon2 );

#endif /* GPSDRIVE_DRAW_TRACKS_H */
