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
Revision 1.1  1994/06/10 02:11:00  tweety
move nmea handling to it's own file Part 1



*/


#ifndef NMEA_HANDLER_H
#define NMEA_HANDLER_H
gint get_position_data_cb (GtkWidget * widget, guint * datum);
void convertRMC (char *f);
void convertGGA (char *f);
void convertRME (char *f);
void convertGSA (char *f);

gint checksum (gchar * text);
FILE *opennmea (const char *name);
void write_nmea_line (const char *line);
void gen_nmea_coord (char *out);
gint write_nmea_cb (GtkWidget * widget, guint * datum);



#endif /* NMEA_HANDLER_H */
