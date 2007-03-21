/*******************************************************************

Copyright (c) 2001-2004 Fritz Ganter <ganter@ganter.at>

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
#ifndef GPSDRIVE_CONFIG_H
#define GPSDRIVE_CONFIG_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <gmodule.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#include "mysql.h"
#include "gpsproto.h"

#include <string.h>
#include <stdlib.h>

void writeconfig ();
void readconfig ();

typedef struct
{
	char icon_theme[500];
	gint showwaypoints;
	gint showtooltips;
	gint showaddwpbutton;
	gint results_max;
}
local_gpsdrive_config;

extern local_gpsdrive_config local_config;

#endif /* GPSDRIVE_CONFIG_H */
