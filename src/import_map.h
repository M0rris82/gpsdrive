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
Revision 1.1  2005/03/27 21:25:46  tweety
separating map_import from gpsdrive.c


*/


#ifndef GPSDRIVE_IMPORT_MAP_H
#define GPSDRIVE_IMPORT_MAP_H

#include <gtk/gtk.h>

/*
 * See import_map.c for details.
 */

gint importfb_cb (GtkWidget * widget, guint datum);
gint importshift_cb (GtkWidget * widget, guint datum);
gint import1_cb (GtkWidget * widget, guint datum);
gint import2_cb (GtkWidget * widget, gpointer datum);
gint import3_cb (GtkWidget * widget, gpointer datum);
gint setrefpoint_cb (GtkWidget * widget, guint datum);
gint mapclick_cb (GtkWidget * widget, GdkEventButton * event);
gint importshift_cb (GtkWidget * widget, guint datum);
gint importshift_cb (GtkWidget * widget, guint datum);

#endif /* GPSDRIVE_IMPORT_MAP_H */
