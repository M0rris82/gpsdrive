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
Revision 1.1  2004/12/23 16:03:24  commiter
Initial revision

Revision 1.18  2004/02/08 17:16:25  ganter
replacing all strcat with g_strlcat to avoid buffer overflows

Revision 1.17  2004/02/02 03:38:31  ganter
code cleanup

Revision 1.16  2004/01/01 09:07:31  ganter
v2.06
trip info is now live updated
added cpu temperature display for acpi
added tooltips for battery and temperature

Revision 1.15  2003/09/17 20:18:01  ganter
...

Revision 1.14  2003/05/11 21:15:46  ganter
v2.0pre7
added script convgiftopng
This script converts .gif into .png files, which reduces CPU load
run this script in your maps directory, you need "convert" from ImageMagick

Friends mode runs fine now
Added parameter -H to correct the alitude

Revision 1.13  2003/01/15 15:30:28  ganter
before dynamically loading mysql

Revision 1.12  2002/11/02 12:38:55  ganter
changed website to www.gpsdrive.de

Revision 1.11  2002/06/23 17:09:34  ganter
v1.23pre9
now PDA mode looks good.

Revision 1.10  2002/06/16 17:50:34  ganter
working on PDA screen

Revision 1.9  2002/06/02 20:54:09  ganter
added navigation.c and copyrights

Revision 1.8  2002/06/01 15:42:31  ganter
fixed bug for little screens
added new wpget from Miguel Angelo Rozsas <miguel@rozsas.xx.nom.br>

Revision 1.7  2002/05/20 20:40:23  ganter
v1.22

Revision 1.6  2002/05/20 10:02:48  ganter
v1.22pre7

Revision 1.5  2002/05/19 19:25:16  ganter
fly and nautic loading works

Revision 1.4  2002/04/14 15:25:56  ganter
v1.17pre3
added simulaton follow switch in setup menu

Revision 1.3  2002/04/13 17:33:29  ganter
...

Revision 1.2  2002/04/13 17:30:35  ganter
v1.17pre2

Revision 1.1  2002/04/07 00:47:42  ganter
added fly.c
added more GPS-receivers


fly.c -- routines specific for aeronautical usage

at the moment, there is no functionality except creating a settings
menu entry

*****************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gpsdrive.h>
#include <config.h>


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

#define PADDING int_padding

extern GtkWidget *hauptfenster;
GtkWidget *splash_window;
extern gchar homedir[500], mapdir[500];
extern gint wpflag, trackflag, muteflag, displaymap_top, displaymap_map;
extern gint scaleprefered, milesflag, nauticflag, metricflag;
extern gint debug, scalewanted, savetrack, defaultserver;
extern gchar serialdev[80];
extern gdouble current_long, current_lat, old_long, old_lat, groundspeed;
extern gint setdefaultpos, shadow, testgarmin, needtosave, usedgps;
extern gchar activewpfile[200];
extern gint mod_setupcounter;
extern GtkWidget *setupentry[50], *setupentrylabel[50];
void (*setupfunction[50]) ();

static gint myplace;
gint moduleversion = 2;
gint flymode = FALSE, vfr = 1,disdevwarn=TRUE;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu, int_padding;


/*  this inits the additional functions for fly */
gint
modulesetup ()
{
  GtkWidget *mainbox, *table, *flymodebt,*disdevbt;
  GtkWidget *l3, *l4, *vfr1, *vfr2,*e1,*e2;
  gchar text[100];

  setupentry[myplace] = gtk_frame_new (_("Aeronautical settings"));
  gtk_container_set_border_width (GTK_CONTAINER (setupentry[myplace]), 5* PADDING);
  setupentrylabel[myplace] = gtk_label_new (_("Fly"));
  mainbox = gtk_vbox_new (FALSE, 15* PADDING);
  gtk_container_add (GTK_CONTAINER (setupentry[myplace]), mainbox);

  table = gtk_table_new (7, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (mainbox), table, FALSE, FALSE, 10* PADDING);

  flymodebt = gtk_check_button_new_with_label (_("Plane mode"));
  if (flymode)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (flymodebt), TRUE);

  gtk_signal_connect (GTK_OBJECT (flymodebt),
		      "clicked", GTK_SIGNAL_FUNC (flymode_cb), (gpointer) 1);

  vfr1 = gtk_radio_button_new_with_label (NULL, _("Use VFR"));
  gtk_signal_connect (GTK_OBJECT (vfr1),
		      "clicked", GTK_SIGNAL_FUNC (vfr_cb), (gpointer) 1);

  vfr2 =
    gtk_radio_button_new_with_label (gtk_radio_button_group
				     (GTK_RADIO_BUTTON (vfr1)), _("Use IFR"));
  gtk_signal_connect (GTK_OBJECT (vfr2), "clicked",
		      GTK_SIGNAL_FUNC (vfr_cb), (gpointer) 2);


  gtk_table_attach_defaults (GTK_TABLE (table), flymodebt, 0, 2, 0, 1);

  gtk_table_attach_defaults (GTK_TABLE (table), vfr1, 0, 1, 2, 3);
  gtk_table_attach_defaults (GTK_TABLE (table), vfr2, 1, 2, 2, 3);

  l3 = gtk_label_new (_("max. horizontal deviation "));
  gtk_table_attach_defaults (GTK_TABLE (table), l3, 0, 1, 4, 5);
  l4 = gtk_label_new (_("max. vertical deviation "));
  gtk_table_attach_defaults (GTK_TABLE (table), l4, 0, 1, 5, 6);

  e1 = gtk_entry_new ();
  g_snprintf (text, sizeof(text),"%d", (int) 500);
  gtk_entry_set_text (GTK_ENTRY (e1), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e1, 1, 2, 4, 5);
  e2 = gtk_entry_new ();
  g_snprintf (text, sizeof(text), "%d", (int) 1000);
  gtk_entry_set_text (GTK_ENTRY (e2), text);
  gtk_table_attach_defaults (GTK_TABLE (table), e2, 1, 2, 5, 6);

  gtk_widget_set_usize (e1, 20, 22);
  gtk_widget_set_usize (e2, 20, 22);

  disdevbt = gtk_check_button_new_with_label (_("disable vert. deviation warning above 5000ft MSL"));
  if (disdevwarn)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (disdevbt), TRUE);

  gtk_signal_connect (GTK_OBJECT (disdevbt),
		      "clicked", GTK_SIGNAL_FUNC (disdev_cb), (gpointer) 1);
  gtk_table_attach_defaults (GTK_TABLE (table), disdevbt, 0, 2, 6, 7);


  if (debug)
    g_print ("\nCreated fly setup window. %d\n", myplace);
  return 0;
}

gint
g_module_check_init ()
{
  myplace = mod_setupcounter + 1;
  g_print ("\nModule fly loaded");
  return 0;
}

/*  switching nightmode */
gint
vfr_cb (GtkWidget *widget, guint datum)
{

  switch (datum)
    {
    case 1:
      vfr = 1;
      break;
    case 2:
      vfr = 2;
      break;

    }
  needtosave = TRUE;
  return TRUE;
}

/*  switching flymode on/off */
gint
flymode_cb (GtkWidget *widget, guint datum)
{
  flymode = !flymode;
  needtosave = TRUE;
  return TRUE;
}

/*  switching flymode on/off */
gint
disdev_cb (GtkWidget *widget, guint datum)
{
  disdevwarn = !disdevwarn;
  needtosave = TRUE;
  return TRUE;
}

