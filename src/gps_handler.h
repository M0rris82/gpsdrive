/***********************************************************************

Copyright (c) 2001,2002 Fritz Ganter <ganter@ganter.at>

Website: www.gpsdrive.de/

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
Revision 1.5  2006/08/08 08:19:32  tweety
rename callback startgpsd_cb

Revision 1.4  2006/02/05 16:38:05  tweety
reading floats with scanf looks at the locale LANG=
so if you have a locale de_DE set reading way.txt results in clearing the
digits after the '.'
For now I set the LC_NUMERIC always to en_US, since there we have . defined for numbers

Revision 1.3  1994/06/10 02:11:00  tweety
move nmea handling to it's own file Part 1

Revision 1.2  2005/10/10 13:17:52  tweety
DBUS Support for connecting to gpsd
you need to use ./configure --enable-dbus to enable it during compile
Author: "Belgabor" <belgabor@gmx.de>

Revision 1.1  2005/08/13 10:16:02  tweety
extract all/some gps_handling parts to File src/gps_handler.c


*/


#ifndef GPS_HANDLER_H
#define GPS_HANDLER_H


#include <gtk/gtk.h>

gint initgps ();
void gpsd_close();

int garblemain (int argc, char **argv);


#endif /* GPS_HANDLER_H */
