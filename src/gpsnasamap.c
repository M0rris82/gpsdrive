/***********************************************************************

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
Revision 1.6  2006/08/02 12:18:36  tweety
forgot one sed for homedir and mapdir

Revision 1.5  2006/08/02 07:48:24  tweety
rename variable mapdir --> local_config_mapdir

Revision 1.4  2005/04/20 23:33:49  tweety
reformatted source code with anjuta
So now we have new indentations

Revision 1.3  2005/04/13 19:58:31  tweety
renew indentation to 4 spaces + tabstop=8

Revision 1.2  2005/04/10 21:50:50  tweety
reformatting c-sources

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.15  2004/02/08 17:16:25  ganter
replacing all strcat with g_strlcat to avoid buffer overflows

Revision 1.14  2004/02/08 16:35:10  ganter
replacing all sprintf with g_snprintf to avoid buffer overflows

Revision 1.13  2004/02/06 22:29:24  ganter
updated README and man page

Revision 1.12  2004/02/04 14:47:10  ganter
added GPGSA sentence for PDOP (Position Dilution Of Precision).

Revision 1.11  2004/02/03 07:11:21  ganter
working on problems if gpsdrive is not installed

Revision 1.10  2004/02/01 22:48:01  ganter
added output to gpsnasamap.c

Revision 1.9  2004/02/01 05:24:59  ganter
missing nasamaps should now really work!
upload again 2.08pre9!

Revision 1.8  2004/02/01 05:10:12  ganter
fixed bug if 1 nasamap is missing

Revision 1.7  2004/02/01 01:57:03  ganter
it seems that nasamaps now working fine

Revision 1.6  2004/01/31 14:48:03  ganter
pre8

Revision 1.5  2004/01/31 13:43:57  ganter
nasamaps are working better, but still bugs

Revision 1.4  2004/01/31 08:27:22  ganter
i hope the nasa maps work all over the world
I expect it works not in australia, will see after i get a little bit sleep

Revision 1.3  2004/01/31 06:24:21  ganter
nasa maps at lon=0 works now

Revision 1.2  2004/01/31 04:16:49  ganter
...

Revision 1.1  2004/01/31 04:11:01  ganter
oh, forgot to add to CVS

Revision 1.1  2004/01/30 22:20:44  ganter
convnasamap creates mapfiles from the big nasa map files


*/

#include "os_specific.h"
#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gpsdrive.h>
#include "gpsdrive_config.h"

/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

static char mybuffer[10000];
static GtkWidget *nasawindow = NULL;
extern int debug;
static int fdin_w, fdin_e;
static char outfilename[100], inputfilename_e[255], inputfilename_w[255];
static int havenasamaps;
int
init_nasa_mapfile ()
{

	havenasamaps = FALSE;

	if (local_config.dir_maps[strlen (local_config.dir_maps) - 1] != '/')
		g_strlcat (local_config.dir_maps, "/", sizeof (local_config.dir_maps));


	g_snprintf (outfilename, sizeof (outfilename), "%stop_NASA_IMAGE.ppm",
		    local_config.dir_maps);

	g_snprintf (inputfilename_e, sizeof (inputfilename_e),
		    "%snasamaps/top_nasamap_east.raw", local_config.dir_home);

	g_snprintf (inputfilename_w, sizeof (inputfilename_w),
		    "%snasamaps/top_nasamap_west.raw", local_config.dir_home);

	fdin_e = open (inputfilename_e, O_RDONLY | O_BINARY);
	if (fdin_e >= 0)
		havenasamaps = TRUE;
	fdin_w = open (inputfilename_w, O_RDONLY | O_BINARY);
	if (fdin_w >= 0)
		havenasamaps = TRUE;

	return 0;
}

void
cleanup_nasa_mapfile ()
{
	if (fdin_w >= 0)
		close (fdin_w);
	if (fdin_e >= 0)
		close (fdin_e);
}

int
create_nasa_mapfile (double lat, double lon, int test, char *fn)
{
	/*     lat,lon= koordinates */
	/* test= test if maps are present */
	/* fn = filename of the generated file */
	int fdout, uc = 0;
	int scale, e, xsize_e, xsize_w;
	int xstart, ystart, y, x_w, x_e;
	double mylon;
	GtkWidget *myprogress, *text, *vbox;
	char textbuf[40];

	if (!havenasamaps)
		return -1;

	scale = 2614061;
	mylon = lon;

	g_strlcpy (fn, "nofile.sorry", 255);

	/* return if no map found */
	if (lon > 0)
	{
		xstart = (int) (21600.0 * (lon / 180.0));
		if (((xstart < 1280) || (xstart > 20320)) && (fdin_w < 0))
			return -1;
		if (fdin_e < 0)
			return -1;
	}
	else
	{
		lon = 180.0 + lon;
		xstart = (int) (21600.0 * (lon / 180.0));
		if (((xstart < 1280) || (xstart > 20320)) && (fdin_e < 0))
			return -1;
		if (fdin_w < 0)
			return -1;
	}

	if (!test)
	{
		fdout = open (outfilename, O_RDWR | O_TRUNC | O_CREAT | O_BINARY, 0644);
		if (fdout < 0)
		{
			fprintf (stderr,
				 _("could not create output map file %s!\n"),
				 outfilename);
			return -1;
		}

		nasawindow = gtk_window_new (GTK_WINDOW_POPUP);
		vbox = gtk_vbox_new (FALSE, 6);
		gtk_container_add (GTK_CONTAINER (nasawindow), vbox);

		gtk_window_set_position (GTK_WINDOW (nasawindow),
					 GTK_WIN_POS_CENTER);

		/*   g_signal_connect (window, "destroy", */
		/*              G_CALLBACK (gtk_widget_destroyed), &window); */

		gtk_window_set_title (GTK_WINDOW (nasawindow),
				      _("Creating map..."));
		gtk_container_set_border_width (GTK_CONTAINER (nasawindow),
						20);

		myprogress = gtk_progress_bar_new ();
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (myprogress),
					       0.0);
		gtk_box_pack_start (GTK_BOX (vbox), myprogress, TRUE, TRUE,
				    2);
		text = gtk_label_new (_
				      ("Creating a temporary map from NASA satellite images"));
		gtk_box_pack_start (GTK_BOX (vbox), text, TRUE, TRUE, 2);

		/*       gtk_widget_show_all (nasawindow); */
		gtk_widget_show_all (nasawindow);
		if (debug)
			fprintf (stdout,
				 _
				 ("converting map for latitude: %f and longitude: %f ...\n"),
				 lat, lon);

		/*       if (lon < 0.0) */
		/*  lon = 180.0 + lon; */
		g_strlcpy (fn, "top_NASA_IMAGE.ppm", 255);


		g_snprintf (mybuffer, sizeof (mybuffer),
			    "P6\n# CREATOR: GpsDrive\n1280 1024\n255\n");

		e = write (fdout, mybuffer, strlen (mybuffer));
		uc = e;
		lon = mylon;
		xstart = (int) (21600.0 * (lon / 180.0));

		ystart = 3 * 21600 * (int) (10800 - 10800.0 * (lat / 90.0));

		/*    fprintf (stdout, "xstart: %d, ystart: %d\n", xstart, ystart);  */
		xstart -= 640;
		ystart = ystart - 512 * 21600 * 3;

		x_w = x_e = -1;
		xsize_w = xsize_e = 1280;

		if (xstart < 0)
		{
			x_w = 21600 + xstart;
			x_e = 1280 - x_w;

			if (x_e < -20320)
				x_e = -1;
			else if (x_e < 0)
				x_e = 0;

			if (x_w < -20320)
				x_w = -1;
			else if (x_w < 0)
				x_w = 0;
			xsize_w = 21600 - x_w;
			xsize_e = 1280 - xsize_w;
		}
		else if (xstart > 20320)
		{
			x_w = (xstart + 1280) - 21600;
			x_e = 1280 - x_w;
			if (x_e < 20320)
				x_e = -1;
			if (x_e < 0)
				x_e = 0;
			xsize_w = x_w;
			xsize_e = 1280 - xsize_w;
		}
		else
		{
			if (mylon >= 0.0)
				x_e = xstart;
			else
				x_w = xstart;
		}
		if (xsize_w > 1280)
			xsize_w = 1280;
		if (xsize_e > 1280)
			xsize_e = 1280;
		x_w *= 3;
		x_e *= 3;

		for (y = 0; y < 1024; y++)
		{
			if ((y % 32) == 0)
			{
				gtk_progress_bar_set_fraction
					(GTK_PROGRESS_BAR (myprogress),
					 y / 1024.0);
				g_snprintf (textbuf, sizeof (textbuf), "%d%%",
					    (int) (100.0 * y / 1024));
				gtk_progress_bar_set_text (GTK_PROGRESS_BAR
							   (myprogress),
							   textbuf);
				while (gtk_events_pending ())
					gtk_main_iteration ();

			}

			if (x_w != -3)
			{
				e = lseek (fdin_w,
					   x_w + ystart + y * 21600 * 3,
					   SEEK_SET);
				e = read (fdin_w, mybuffer, xsize_w * 3);
				e = write (fdout, mybuffer, xsize_w * 3);
				uc += e;
			}
			if (x_e != -3)
			{
				e = lseek (fdin_e,
					   x_e + ystart + y * 21600 * 3,
					   SEEK_SET);
				e = read (fdin_e, mybuffer, xsize_e * 3);
				e = write (fdout, mybuffer, xsize_e * 3);
				uc += e;
			}
		}
		/*       fprintf (stderr, "wrote %d bytes (%.1f MB) to mapfile\n", uc, */
		/*         uc / (1024.0 * 1024.0)); */

		gtk_widget_destroy (GTK_WIDGET (nasawindow));
		close (fdout);
		g_strlcpy (mybuffer, g_basename (outfilename),
			   sizeof (mybuffer));
		fprintf (stdout,
			 _
			 ("\nYou can permanently add this map file with following line in your\nmap_koord.txt (rename the file!):\n"));
		fprintf (stdout, "\n%s %f %f %d\n", mybuffer, lat, lon,
			 scale);

	}			/* End of if !test */


	return scale;
}
