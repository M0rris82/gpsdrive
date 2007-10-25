/***********************************************************************

Copyright (c) 2007 Guenther Meyer <d.s.e (at) sordidmusic.com>

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


#ifndef GPSDRIVE_GPX_H
#define GPSDRIVE_GPX_H

/*
 * See gpx.c for details.
 */

gint gpx_file_read (gchar *gpx_file, gint gpx_mode);
gint gpx_file_write (gchar *gpx_file, gint gpx_mode);
gint loadgpx_cb (gint gpx_mode);
void test_gpx (gchar *filename);

#endif /* GPSDRIVE_GPX_H */
