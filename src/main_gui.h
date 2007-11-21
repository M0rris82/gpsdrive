/***********************************************************************

Copyright (c) 2001-2006 Fritz Ganter <ganter@ganter.at>

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

    *********************************************************************

***********************************************************************/

#ifndef GPSDRIVE_MAIN_GUI_H
#define GPSDRIVE_MAIN_GUI_H

/* Definitions for the dashboard */
enum
{
	DASH_DIST,
	DASH_TIMEREMAIN,
	DASH_BEARING,
	DASH_TURN,
	DASH_SPEED,
	DASH_SPEED_AVG,
	DASH_SPEED_MAX,
	DASH_HEADING,
	DASH_ALT,
	DASH_TRIP,
	DASH_GPSPRECISION,
	DASH_TIME,
	DASH_POSITION,
	DASH_MAPSCALE,
	DASH_N_ITEMS
};

gint create_main_window (void);
void create_map_drawable (void);
gint autobestmap_cb (GtkWidget *widget, guint datum);
gint update_statusdisplay (void);
gint expose_gpsfix (GtkWidget *widget, guint *datum);

#endif /* GPSDRIVE_MAIN_GUI_H */
