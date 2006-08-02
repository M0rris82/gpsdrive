/****************************************************************

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

    *********************************************************************


$Log$
Revision 1.7  2006/08/02 07:48:24  tweety
rename variable mapdir --> local_config_mapdir

Revision 1.6  2006/07/27 06:40:38  tweety
delete unused extern declaration

Revision 1.5  2005/04/20 23:33:49  tweety
reformatted source code with anjuta
So now we have new indentations

Revision 1.4  2005/04/13 19:58:31  tweety
renew indentation to 4 spaces + tabstop=8

Revision 1.3  2005/04/10 21:50:50  tweety
reformatting c-sources

Revision 1.2  2005/03/27 21:25:46  tweety
separating map_import from gpsdrive.c

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.8  2004/02/02 03:38:32  ganter
code cleanup

Revision 1.7  2004/01/01 09:07:33  ganter
v2.06
trip info is now live updated
added cpu temperature display for acpi
added tooltips for battery and temperature

Revision 1.6  2003/05/11 21:15:46  ganter
v2.0pre7
added script convgiftopng
This script converts .gif into .png files, which reduces CPU load
run this script in your maps directory, you need "convert" from ImageMagick

Friends mode runs fine now
Added parameter -H to correct the alitude

Revision 1.5  2003/01/15 15:30:28  ganter
before dynamically loading mysql

Revision 1.4  2002/11/02 12:38:55  ganter
changed website to www.gpsdrive.de

Revision 1.3  2002/06/02 20:54:10  ganter
added navigation.c and copyrights

Revision 1.2  2002/05/20 20:40:23  ganter
v1.22

Revision 1.1  2002/05/19 19:25:32  ganter
added nautic.c

nautic.c -- routines specific for nautic usage
*****************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <config.h>
#include <gpsdrive.h>

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


extern GtkWidget *hauptfenster;
extern GtkWidget *splash_window;
extern gint wpflag, trackflag, muteflag, displaymap_top, displaymap_map;
extern gint scaleprefered, milesflag, nauticflag, metricflag;
extern gint debug, scalewanted, savetrack, defaultserver;
extern gchar serialdev[80];
extern gint setdefaultpos, shadow, testgarmin, needtosave, usedgps;
extern gchar activewpfile[200];
extern gint mod_setupcounter;
extern GtkWidget *setupentry[50], *setupentrylabel[50];
void (*setupfunction[50]) ();
static gint myplace;
gint moduleversion = 2;

/*  this inits the additional functions for fly */
gint
modulesetup ()
{
	GtkWidget *mainbox;


	setupentry[myplace] = gtk_frame_new (_("Nautic settings"));
	gtk_container_set_border_width (GTK_CONTAINER
					(setupentry[myplace]), 5);
	setupentrylabel[myplace] = gtk_label_new (_("Nautic"));
	mainbox = gtk_vbox_new (FALSE, 15);
	gtk_container_add (GTK_CONTAINER (setupentry[myplace]), mainbox);

	if (debug)
		g_print ("\nCreated nautic setup window.%d\n", myplace);
	return 0;
}

gint
g_module_check_init ()
{
	myplace = mod_setupcounter + 1;
	g_print ("\nModule nautic loaded");
	return 0;
}
