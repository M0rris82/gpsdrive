/***********************************************************************

Copyright (c) 2001,2002 Fritz Ganter <ganter@ganter.at>

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


#ifndef GPSDRIVE_BATTERY_H
#define GPSDRIVE_BATTERY_H

/*
 * See battery.c for details.
 */


int battery_get_values(void);
int temperature_get_values(void);
int expose_display_battery();
int expose_display_temperature();
void create_battery_widget (GtkWidget * hbox2);
void create_temperature_widget (GtkWidget * hbox2);


#endif /* GPSDRIVE_BATTERY_H */
