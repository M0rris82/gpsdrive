/*
**********************************************************************

Copyright (c) 2001-2007 Fritz Ganter <ganter@ganter.at>

Website: www.gpsdrive.de


Copyright (c) 2007 Ross Scanlon <ross@theinternethost.com.au>

Website: www.4x4falcon.com/gpsdrive/

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

**********************************************************************
*/

    /*
     * gui.c  
     * The purpose of this module is to do the setup
     * and resizing of user interface for gspdrive
     */

    /*
      4 Feb 2007  Version 0.1 
      Move sizing of window to this file from gpsdrive.c
    */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <gpsdrive.h>
#include <icons.h>
#include <config.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <gdk/gdktypes.h>
#include "gtk/gtk.h"
#include "gettext.h"
#include "gpsdrive.h"
#include "gpsdrive_config.h"
#include "gui.h"
#include "poi.h"
#include "routes.h"

/*  Defines for gettext I18n */
#include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif

extern gint do_unit_test;

    /* External Values */

extern gint mydebug;
extern gint debug;

extern GtkTreeStore *poi_types_tree;
extern GtkListStore *poi_result_tree;
extern GtkListStore *route_list_tree;

extern gdouble target_lat, target_lon;
extern gdouble wp_saved_target_lat;
extern gdouble wp_saved_target_lon;
extern gdouble wp_saved_posmode_lat;
extern gdouble wp_saved_posmode_lon;
extern gint posmode;
extern gdouble posmode_lon, posmode_lat;

extern GdkColor textback;

// Some of these shouldn't be necessary, when all the gui stuff is finally moved
extern GtkWidget *find_poi_bt;
extern GtkWidget *posbt;
extern GtkWidget *drawing_area;


extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu, int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;

struct pattern
{
	GtkEntry *text, *distance;
	gchar *poitype_id;
	gchar *poitype_name;
	gint typeflag;	/* 0: search all types, 1: search only selected */
	gint posflag;	/* 0: search at current position, 1: search at destination */
	guint result_count;
} criteria;

static GtkWidget *statusbar_poilist;
static gint statusbar_id;

extern gint borderlimit;

extern gint extrawinmenu, pdamode;
extern gdouble posx, posy;

extern status_struct route;

/* poi lookup window */
GtkWidget *poi_lookup_window;
GtkWidget *button_delete;
GtkWidget *button_target;
GtkWidget *button_addtoroute;

/* route window */
GtkWidget *route_window;
GtkWidget *button_startroute;
GtkWidget *button_remove;

GtkWidget *poi_types_window;


/* included from freedesktop.org source, copyright as below do not change anything in between here and function get_window_sizing */

/*

Copyright 1985, 1986, 1987,1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/lib/X11/ParseGeom.c,v 1.2 2001/10/28 03:32:30 tsi Exp $ */

/*
  #include "Xlibint.h"
  #include "Xutil.h"
*/

#include <X11/Xlibint.h>
#include <X11/Xutil.h>

#ifdef notdef
/* 
 *Returns pointer to first char ins search which is also in what, else NULL.
 */
static char *strscan (search, what)
     char *search, *what;
{
    int i, len = strlen (what);
    char c;

    while ((c = *(search++)) != NULL)
	for (i = 0; i < len; i++)
	    if (c == what [i])
		return (--search);
    return (NULL);
}
#endif

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */

static int
ReadInteger(char *string, char **NextString)
{
    register int Result = 0;
    int Sign = 1;
    
    if (*string == '+')
	string++;
    else if (*string == '-')
	{
	    string++;
	    Sign = -1;
	}
    for (; (*string >= '0') && (*string <= '9'); string++)
	{
	    Result = (Result * 10) + (*string - '0');
	}
    *NextString = string;
    if (Sign >= 0)
	return (Result);
    else
	return (-Result);
}

int XParseGeometry (
		    _Xconst char *string,
		    int *x,
		    int *y,
		    unsigned int *width,    /* RETURN */
		    unsigned int *height)    /* RETURN */
{
    int mask = NoValue;
    register char *strind;
    unsigned int tempWidth = 0, tempHeight = 0;
    int tempX = 0, tempY = 0;
    char *nextCharacter;

    if ( (string == NULL) || (*string == '\0')) return(mask);
    if (*string == '=')
	string++;  /* ignore possible '=' at beg of geometry spec */

    strind = (char *)string;
    if (*strind != '+' && *strind != '-' && *strind != 'x') {
	tempWidth = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter) 
	    return (0);
	strind = nextCharacter;
	mask |= WidthValue;
    }

    if (*strind == 'x' || *strind == 'X') {	
	strind++;
	tempHeight = ReadInteger(strind, &nextCharacter);
	if (strind == nextCharacter)
	    return (0);
	strind = nextCharacter;
	mask |= HeightValue;
    }

    if ((*strind == '+') || (*strind == '-')) {
	if (*strind == '-') {
	    strind++;
	    tempX = -ReadInteger(strind, &nextCharacter);
	    if (strind == nextCharacter)
		return (0);
	    strind = nextCharacter;
	    mask |= XNegative;

	}
	else
	    {	strind++;
	    tempX = ReadInteger(strind, &nextCharacter);
	    if (strind == nextCharacter)
		return(0);
	    strind = nextCharacter;
	    }
	mask |= XValue;
	if ((*strind == '+') || (*strind == '-')) {
	    if (*strind == '-') {
		strind++;
		tempY = -ReadInteger(strind, &nextCharacter);
		if (strind == nextCharacter)
		    return(0);
		strind = nextCharacter;
		mask |= YNegative;

	    }
	    else
		{
		    strind++;
		    tempY = ReadInteger(strind, &nextCharacter);
		    if (strind == nextCharacter)
			return(0);
		    strind = nextCharacter;
		}
	    mask |= YValue;
	}
    }
	
    /* If strind isn't at the end of the string the it's an invalid
       geometry specification. */

    if (*strind != '\0') return (0);

    if (mask & XValue)
	*x = tempX;
    if (mask & YValue)
	*y = tempY;
    if (mask & WidthValue)
	*width = tempWidth;
    if (mask & HeightValue)
	*height = tempHeight;
    return (mask);
}


/* do not change the above */

/****
     function get_window_sizing
     This creates the main window sizing from either --geometry or based on screen size if no --geometry
     as at 4 February 2007 will also use -s -r from command line but will be removed eventually
****/

int get_window_sizing (gchar *geom, gint usegeom, gint screen_height, gint screen_width) {

    int ret;
    uint width, height;
    int x, y;



    if ( mydebug >= 0 ) {
	fprintf (stderr, "Creating main window\n");
    }

    /*** do we have geometry from the command line ***/

    if (usegeom) {

	ret = XParseGeometry (geom, &x, &y, &width, &height);


	/* if geometry returns a width then use that otherwise use default for 640x480 */

	if (ret & WidthValue) {
	    real_screen_x = width;
	}
	else {
	    real_screen_x = 630;
	}

	/* if geometry returns a height then use that otherwise use default for 640x480 */

	if (ret & HeightValue) {
	    real_screen_y = height;
	}
	else {
	    real_screen_y = screen_height - YMINUS;
	}
    }
    else {

	/** from gpsdrive.c line 4773 to 4814 */

	PSIZE = 50;
	SMALLMENU = 0;
	PADDING = 1;
	if (screen_height >= 1024)		 /* > 1280x1024 */
	    {
		real_screen_x = min(1280,screen_width-300);
		real_screen_y = min(1024,screen_height-200);
		if ( mydebug > 0 )
		    g_print ("Set real Screen size to %d,%d\n", 
			     real_screen_x,real_screen_y);
		
	    }
	else if (screen_height >= 768)	/* 1024x768 */
	    {
		real_screen_x = 800;
		real_screen_y = 540;
	    }
	else if (screen_height >= 750)	/* 1280x800 */
	    {
		real_screen_x = 1140;
		real_screen_y = 600;
	    }
	else if (screen_height >= 480)	/* 640x480 */
	    {
		if (screen_width == 0)
		    real_screen_x = 630;
		else
		    real_screen_x = screen_width - XMINUS;
		real_screen_y = screen_height - YMINUS;
	    }
	else if (screen_height < 480)
	    {
		if (screen_width == 0)
		    real_screen_x = 220;
		else
		    real_screen_x = screen_width - XMINUS;
		real_screen_y = screen_height - YMINUS;
		PSIZE = 25;
		SMALLMENU = 1;
		PADDING = 0;
	    }
    }  /*** if usegeom  */

    /** from gpsdrive.c line 4824 to 4857 */

    if ((extrawinmenu) && (screen_width != 0))
	{
	    real_screen_x += XMINUS - 10;
	    real_screen_y += YMINUS - 30;
	}

    if (   ((screen_width  == 240) && (screen_height == 320)) 
	   || ((screen_height == 240) && (screen_width  == 320)) )
	{
	    pdamode = TRUE;
	    // SMALLMENU = 1;
	    real_screen_x = screen_width - 8;
	    real_screen_y = screen_height - 70;
	}
    if (pdamode)
	{
	    extrawinmenu = TRUE;
	    PADDING = 0;
	    g_print ("\nPDA mode\n");
	}

    if (real_screen_x < real_screen_y)
	borderlimit = real_screen_x / 5;
    else
	borderlimit = real_screen_y / 5;

    if (borderlimit < 30)
	borderlimit = 30;

    SCREEN_X_2 = SCREEN_X / 2;
    SCREEN_Y_2 = SCREEN_Y / 2;
    PSIZE = 50;
    posx = SCREEN_X_2;
    posy = SCREEN_Y_2;

    if ( mydebug > 10 ) {
	g_print ("Set real Screen size to %d,%d\n", real_screen_x,real_screen_y);
    }

    if ( mydebug > 10 ) {
	g_print ("Screen size is %d,%d\n", screen_width,screen_height);
    }

    return 0;
}



int resize_all (void) {

    return 0;
}






/* *****************************************************************************
 * CALLBACKS
 */
static void
evaluate_poi_search_cb (GtkWidget *button, struct pattern *entries)
{
	gchar search_status[80];
	
	if (mydebug>5)
	{
		fprintf (stdout, "\npoi search criteria:\n  text: %s\n  distance: %s\t  posflag: %d\n",
					gtk_entry_get_text (entries->text) ,
					gtk_entry_get_text (entries->distance),
					entries->posflag);
	}
		
	entries->result_count = poi_get_results (gtk_entry_get_text (entries->text),
						gtk_entry_get_text (entries->distance),
						entries->posflag, entries->typeflag, entries->poitype_id);

	gtk_statusbar_pop (GTK_STATUSBAR (statusbar_poilist), statusbar_id);
	if (entries->result_count == local_config.results_max)
		g_snprintf (search_status, sizeof (search_status), _(" Found %d matching entries (Limit reached)."), entries->result_count);
	else
		g_snprintf (search_status, sizeof (search_status), _(" Found %d matching entries."), entries->result_count);
	gtk_statusbar_push (GTK_STATUSBAR (statusbar_poilist), statusbar_id, search_status);
}

void
toggle_window_poi_info_cb (GtkToggleButton *togglebutton, gpointer user_data)
{

}

static void
searchpoitypemode_cb (GtkWidget *combobox, GtkToggleButton *button)
{
	if (gtk_toggle_button_get_active(button))
	{
		gtk_widget_set_sensitive (combobox, TRUE);
		/* switch to selection from selected poi-type */
		criteria.typeflag = TRUE;
	}
	else
	{
		gtk_widget_set_sensitive (combobox, FALSE);
		/* switch to selection from all poi-types */
		criteria.typeflag = FALSE;
	}
}

static void
searchdistancemode_cb (GtkToggleButton *button, gpointer user_data)
{
	if (gtk_toggle_button_get_active(button))
	{
		criteria.posflag = 0; /* search near current position */
	}
	else
	{
		criteria.posflag = 1; /* search near destination */
	}
}


static void
close_poi_lookup_window_cb (GtkWidget *window)
{
	/* restore saved values */
	target_lat = wp_saved_target_lat;
	target_lon = wp_saved_target_lon;
	if (posmode)
	{
		posmode_lat = wp_saved_posmode_lat;
		posmode_lon = wp_saved_posmode_lon;
	}

	gtk_widget_destroy (window);
	gtk_widget_set_sensitive (find_poi_bt, TRUE);
	gtk_widget_set_sensitive (posbt, TRUE);
}


static void
select_target_poi_cb (GtkWidget *window)
{
	gtk_widget_destroy (window);
	
	gtk_widget_set_sensitive (find_poi_bt, TRUE);
	gtk_widget_set_sensitive (posbt, TRUE);
}


static void
delete_poi_cb (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	gint selected_poi_id = 0;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		if (popup_yes_no(GTK_WINDOW (poi_lookup_window), NULL) == GTK_RESPONSE_YES)
		{
			gtk_tree_model_get (model, &iter,
					RESULT_ID, &selected_poi_id,
					-1);
			if (mydebug>20)
				fprintf (stderr, "deleting poi with id: %d\n", selected_poi_id);
			deletesqldata (selected_poi_id);
			gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
		}
	}
}


static void
select_poi_cb (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		if (route.edit)
		{
			add_poi_to_route (model, iter);
			if (route.available)
				gtk_widget_set_sensitive (button_startroute, TRUE);
		}
		else
		{
			if (!route.active)
			{
				gtk_tree_model_get (model, &iter,
							RESULT_LAT, &target_lat,
							RESULT_LON, &target_lon,
							-1);
				if (mydebug>50)
					fprintf (stdout, " new target -> %f / %f\n", target_lat, target_lon);
			}
		}
		
		gtk_widget_set_sensitive (button_delete, TRUE);

		/* if posmode enabled set posmode_lat/lon */
		if (posmode)
		{
			gtk_tree_model_get (model, &iter,
						RESULT_LAT, &posmode_lat,
						RESULT_LON, &posmode_lon,
						-1);
		}
	}
}


static void
select_poitype_cb (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	// ### temporary:
	gint ptid;
	
	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter,
					POITYPE_ID, &ptid,
					POITYPE_NAME, &criteria.poitype_name,
					-1);
		
		// ### temporary:
		g_snprintf (criteria.poitype_id, sizeof (criteria.poitype_id), "%d", ptid);
	
		
		//if (mydebug>50)
			fprintf (stdout, " selected poi-type -> %d / %s / %s\n", ptid, criteria.poitype_id, criteria.poitype_name);
		
	}

}


static void
toggle_window_cb (GtkWidget *window)
{
	if (GTK_WIDGET_VISIBLE (window))
		gtk_widget_hide_all (window);
	else
		gtk_widget_show_all (window);
}


static void
close_route_window_cb ()
{
	gtk_widget_destroy (route_window);
	route.edit = FALSE;
	
	if (GTK_IS_WIDGET (button_addtoroute))
		gtk_widget_set_sensitive (button_addtoroute, TRUE);
}


static void
route_startstop_cb ()
{
	if (route.active)
	{
		// ### stop routing
	}
	else
	{
		// ### start routing
	}
	close_route_window_cb ();	
}


static void
route_cancel_cb ()
{
	gtk_list_store_clear (route_list_tree);
	close_route_window_cb ();
	route.available = FALSE;
	route.items = 0;
}


static void
select_routepoint_cb ()
{
	gtk_widget_set_sensitive (button_remove, TRUE);
}


static void
remove_routepoint_cb (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
	}
}


/* *****************************************************************************
 * Popup: Are you sure, y/n
 */
gint popup_yes_no (GtkWindow *parent, gchar *message)
{
	GtkDialog *dialog_yesno;
	gint response_id;
	gchar *question = "Are you sure?";
	
	dialog_yesno = GTK_DIALOG (gtk_message_dialog_new (parent,
									GTK_DIALOG_MODAL,
									GTK_MESSAGE_QUESTION,
									GTK_BUTTONS_YES_NO,
									"%s", question));
	response_id = gtk_dialog_run (dialog_yesno);
	gtk_widget_destroy (GTK_WIDGET (dialog_yesno));
	return response_id;
}


/* *****************************************************************************
 * Window: POI-Info
 */
void create_poi_info_window (void)
{
  GtkWidget *poi_info_window;
  GtkWidget *vbox_poidata;
  GtkWidget *frame_poi_basicdata;
  GtkWidget *alignment_basic;
  GtkWidget *table_basic;
  GtkWidget *label_name;
  GtkWidget *label_comment;
  GtkWidget *checkbutton_private;
  GtkWidget *entry_name;
  GtkWidget *entry_comment;
  GtkWidget *entry_lat;
  GtkWidget *entry_lon;
  GtkWidget *entry_alt;
  GtkWidget *label_alt;
  GtkWidget *label_lon;
  GtkWidget *label_lat;
  GtkWidget *hseparator_basic;
  GtkWidget *label_type;
  GtkWidget *combobox_type;
  GtkWidget *entry_poitypeid;
  GtkWidget *label_basic;
  GtkWidget *expander_extra;
  GtkWidget *frame_extra;
  GtkWidget *alignment_extra;
  GtkWidget *scrolledwindow_extra;
  GtkWidget *treeview_extra;
  GtkWidget *label_extradata;
  GtkWidget *hbox_status;
  GtkWidget *statusbar_poiinfo;
  GtkWidget *button_save;
  GtkWidget *button_close;

  poi_info_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (poi_info_window), _("POI-Info"));
  gtk_window_set_icon_name (GTK_WINDOW (poi_info_window), "gtk-info");
  //gtk_window_set_decorated (GTK_WINDOW (poi_info_window), FALSE);
  gtk_window_set_focus_on_map (GTK_WINDOW (poi_info_window), FALSE);

  vbox_poidata = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_poidata);
  gtk_container_add (GTK_CONTAINER (poi_info_window), vbox_poidata);

  frame_poi_basicdata = gtk_frame_new (NULL);
  gtk_widget_show (frame_poi_basicdata);
  gtk_box_pack_start (GTK_BOX (vbox_poidata), frame_poi_basicdata, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_poi_basicdata), 5);

  alignment_basic = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment_basic);
  gtk_container_add (GTK_CONTAINER (frame_poi_basicdata), alignment_basic);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_basic), 5, 5, 5, 5);

  table_basic = gtk_table_new (6, 6, FALSE);
  gtk_widget_show (table_basic);
  gtk_container_add (GTK_CONTAINER (alignment_basic), table_basic);

  label_name = gtk_label_new (_("Name"));
  gtk_widget_show (label_name);
  gtk_table_attach (GTK_TABLE (table_basic), label_name, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_name), 0, 0.5);

  label_comment = gtk_label_new (_("Comment"));
  gtk_widget_show (label_comment);
  gtk_table_attach (GTK_TABLE (table_basic), label_comment, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_comment), 0, 0.5);

  checkbutton_private = gtk_check_button_new_with_mnemonic (_("private"));
  gtk_widget_show (checkbutton_private);
  gtk_table_attach (GTK_TABLE (table_basic), checkbutton_private, 5, 6, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);

  entry_name = gtk_entry_new ();
  gtk_widget_show (entry_name);
  gtk_table_attach (GTK_TABLE (table_basic), entry_name, 1, 5, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_max_length (GTK_ENTRY (entry_name), 80);

  entry_comment = gtk_entry_new ();
  gtk_widget_show (entry_comment);
  gtk_table_attach (GTK_TABLE (table_basic), entry_comment, 1, 6, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_max_length (GTK_ENTRY (entry_comment), 255);

  entry_lat = gtk_entry_new ();
  gtk_widget_show (entry_lat);
  gtk_table_attach (GTK_TABLE (table_basic), entry_lat, 0, 2, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_lat), 12);

  entry_lon = gtk_entry_new ();
  gtk_widget_show (entry_lon);
  gtk_table_attach (GTK_TABLE (table_basic), entry_lon, 2, 4, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_lon), 12);

  entry_alt = gtk_entry_new ();
  gtk_widget_show (entry_alt);
  gtk_table_attach (GTK_TABLE (table_basic), entry_alt, 4, 6, 5, 6,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_alt), 10);

  label_alt = gtk_label_new (_("Altitude"));
  gtk_widget_show (label_alt);
  gtk_table_attach (GTK_TABLE (table_basic), label_alt, 4, 5, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_alt), 0, 0.5);

  label_lon = gtk_label_new (_("Longitude"));
  gtk_widget_show (label_lon);
  gtk_table_attach (GTK_TABLE (table_basic), label_lon, 2, 3, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_lon), 0, 0.5);

  label_lat = gtk_label_new (_("Latitude"));
  gtk_widget_show (label_lat);
  gtk_table_attach (GTK_TABLE (table_basic), label_lat, 0, 1, 4, 5,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_lat), 0, 0.5);

  hseparator_basic = gtk_hseparator_new ();
  gtk_widget_show (hseparator_basic);
  gtk_table_attach (GTK_TABLE (table_basic), hseparator_basic, 0, 6, 3, 4,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 5);

  label_type = gtk_label_new (_("Type"));
  gtk_widget_show (label_type);
  gtk_table_attach (GTK_TABLE (table_basic), label_type, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_misc_set_alignment (GTK_MISC (label_type), 0, 0.5);

  combobox_type = gtk_combo_box_new_text ();
  gtk_widget_show (combobox_type);
  gtk_table_attach (GTK_TABLE (table_basic), combobox_type, 1, 5, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  entry_poitypeid = gtk_entry_new ();
  gtk_widget_show (entry_poitypeid);
  gtk_table_attach (GTK_TABLE (table_basic), entry_poitypeid, 5, 6, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_max_length (GTK_ENTRY (entry_poitypeid), 5);
  gtk_editable_set_editable (GTK_EDITABLE (entry_poitypeid), FALSE);
  gtk_entry_set_has_frame (GTK_ENTRY (entry_poitypeid), FALSE);
  gtk_entry_set_width_chars (GTK_ENTRY (entry_poitypeid), 5);

  label_basic = gtk_label_new (_("Basic Data"));
  gtk_widget_show (label_basic);
  gtk_frame_set_label_widget (GTK_FRAME (frame_poi_basicdata), label_basic);

  expander_extra = gtk_expander_new (NULL);
  gtk_widget_show (expander_extra);
  gtk_box_pack_start (GTK_BOX (vbox_poidata), expander_extra, TRUE, TRUE, 0);
  gtk_expander_set_expanded (GTK_EXPANDER (expander_extra), TRUE);

  frame_extra = gtk_frame_new (NULL);
  gtk_widget_show (frame_extra);
  gtk_container_add (GTK_CONTAINER (expander_extra), frame_extra);
  gtk_container_set_border_width (GTK_CONTAINER (frame_extra), 5);
  gtk_frame_set_label_align (GTK_FRAME (frame_extra), 0, 0);

  alignment_extra = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignment_extra);
  gtk_container_add (GTK_CONTAINER (frame_extra), alignment_extra);
  gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_extra), 0, 0, 12, 0);

  scrolledwindow_extra = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_extra);
  gtk_container_add (GTK_CONTAINER (alignment_extra), scrolledwindow_extra);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_extra), GTK_SHADOW_IN);

  treeview_extra = gtk_tree_view_new ();
  gtk_widget_show (treeview_extra);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_extra), treeview_extra);

  label_extradata = gtk_label_new (_("Extra Data"));
  gtk_widget_show (label_extradata);
  gtk_expander_set_label_widget (GTK_EXPANDER (expander_extra), label_extradata);

  hbox_status = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox_status);
  gtk_box_pack_start (GTK_BOX (vbox_poidata), hbox_status, FALSE, TRUE, 0);

  statusbar_poiinfo = gtk_statusbar_new ();
  gtk_widget_show (statusbar_poiinfo);
  gtk_box_pack_start (GTK_BOX (hbox_status), statusbar_poiinfo, TRUE, TRUE, 0);
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar_poiinfo), FALSE);

  button_save = gtk_button_new_from_stock ("gtk-save");
  gtk_widget_show (button_save);
  gtk_box_pack_start (GTK_BOX (hbox_status), button_save, FALSE, FALSE, 0);

  button_close = gtk_button_new_from_stock ("gtk-close");
  gtk_widget_show (button_close);
  gtk_box_pack_start (GTK_BOX (hbox_status), button_close, FALSE, FALSE, 0);

  gtk_widget_show_all (poi_info_window);
}


/* *****************************************************************************
 * Window: POI-Lookup
 */
void poi_lookup_cb (GtkWidget *calling_button)
{
	GtkWidget *dialog_vbox_poisearch;
	GtkWidget *vbox_searchbox;
	GtkWidget *expander_poisearch;
	GtkWidget *frame_search_criteria;
	GtkWidget *alignment_criteria;
	GtkWidget *vbox_criteria;
	GtkWidget *hbox_text;
	GtkWidget *label_text;
	GtkWidget *entry_text;
	GtkWidget *button_search;
	GtkWidget *hbox_distance;
	GtkWidget *label_distance;
	GtkWidget *entry_distance;
	GtkWidget *label_distfrom;
	GtkWidget *radiobutton_distcurrent;
	GSList *radiobutton_distance_group = NULL;
	GtkWidget *radiobutton_distcursor;
	GtkWidget *hbox_type;
	GtkWidget *label_type;
	GtkWidget *radiobutton_typeall;
	GSList *radiobutton_type_group = NULL;
	GtkWidget *radiobutton_typesel;
	GtkWidget *comboboxentry_type;
	GtkWidget *button_types;
	GtkWidget *label_criteria;
	GtkWidget *frame_poiresults;
	GtkWidget *alignment_poiresults;
	GtkWidget *vbox_poiresults;
	GtkWidget *scrolledwindow_poilist;
	GtkWidget *treeview_poilist;
	GtkCellRenderer *renderer_poilist;
	GtkTreeViewColumn *column_poilist;
	GtkTreeSelection *poilist_select;
	GtkWidget *hbox_poistatus;
	GtkWidget *togglebutton_poiinfo;
	GtkWidget *alignment_poiinfo;
	GtkWidget *hbox11;
	GtkWidget *image_poiinfo;
	GtkWidget *label_poiinfo;
	GtkWidget *label_results;
	GtkWidget *dialog_action_area_poisearch;
	GtkWidget *alignment_addtoroute;
	GtkWidget *hbox_addtoroute;
	GtkWidget *image_addtoroute;
	GtkWidget *label_addtoroute;
	GtkWidget *alignment_target;
	GtkWidget *hbox_target;
	GtkWidget *image_target;
	GtkWidget *label_target;
	GtkWidget *button_close;
	GtkTooltips *tooltips_poilookup;
	
	criteria.posflag = 0;
	criteria.result_count = 0;

	gtk_widget_set_sensitive (find_poi_bt, FALSE);
	gtk_widget_set_sensitive (posbt, FALSE);

	/* save old target/posmode for cancel event */
	wp_saved_target_lat = target_lat;
	wp_saved_target_lon = target_lon;
	if (posmode)
	{
		wp_saved_posmode_lat = posmode_lat;
		wp_saved_posmode_lon = posmode_lon;
	}
	
	tooltips_poilookup = gtk_tooltips_new();
	gtk_tooltips_set_delay (tooltips_poilookup, TOOLTIP_DELAY);
	if (local_config.showtooltips)
		gtk_tooltips_enable (tooltips_poilookup);
	else
		gtk_tooltips_disable (tooltips_poilookup);
	  
	poi_lookup_window = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (poi_lookup_window), 2);
	gtk_window_set_title (GTK_WINDOW (poi_lookup_window), _("Lookup Point of Interest"));
	gtk_window_set_position (GTK_WINDOW (poi_lookup_window), GTK_WIN_POS_CENTER);
	gtk_window_set_type_hint (GTK_WINDOW (poi_lookup_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	if (pdamode)
		gtk_window_set_default_size (GTK_WINDOW (poi_lookup_window), real_screen_x, real_screen_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (poi_lookup_window), -1, 400);
	
	dialog_vbox_poisearch = GTK_DIALOG (poi_lookup_window)->vbox;
	
	vbox_searchbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (dialog_vbox_poisearch), vbox_searchbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_searchbox), 2);
	
	expander_poisearch = gtk_expander_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_searchbox), expander_poisearch, FALSE, TRUE, 0);
	gtk_expander_set_expanded (GTK_EXPANDER (expander_poisearch), TRUE);

	/* Frame: POI Search Criteria */
  {
	frame_search_criteria = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (expander_poisearch), frame_search_criteria);
	gtk_frame_set_label_align (GTK_FRAME (frame_search_criteria), 0, 0);
	
	alignment_criteria = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame_search_criteria), alignment_criteria);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_criteria), 5, 5, 12, 5);
	
	vbox_criteria = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (alignment_criteria), vbox_criteria);
	
	hbox_text = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_criteria), hbox_text, TRUE, TRUE, 0);
	
	label_text = gtk_label_new (_("<i>Search Text:</i>"));
	gtk_box_pack_start (GTK_BOX (hbox_text), label_text, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label_text), TRUE);
	
	entry_text = gtk_entry_new ();
	criteria.text = GTK_ENTRY (entry_text);
	gtk_box_pack_start (GTK_BOX (hbox_text), entry_text, TRUE, TRUE, 5);
	gtk_entry_set_max_length (GTK_ENTRY (entry_text), 255);
	g_signal_connect (entry_text, "activate",
				GTK_SIGNAL_FUNC (evaluate_poi_search_cb), &criteria);
	
	/* Button to activate POI search */
	button_search = gtk_button_new_from_stock ("gtk-find");
	gtk_box_pack_start (GTK_BOX (hbox_text), button_search, FALSE, FALSE, 5);
	g_signal_connect (button_search, "clicked",
				GTK_SIGNAL_FUNC (evaluate_poi_search_cb), &criteria);
	
	/* Distance selection */
	hbox_distance = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_criteria), hbox_distance, TRUE, TRUE, 0);
	
	label_distance = gtk_label_new (_("<i>max. Distance:</i>"));
	gtk_box_pack_start (GTK_BOX (hbox_distance), label_distance, FALSE, FALSE, 10);
	gtk_label_set_use_markup (GTK_LABEL (label_distance), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (label_distance), TRUE);
	
	entry_distance = gtk_entry_new ();
	criteria.distance = GTK_ENTRY (entry_distance);
	gtk_box_pack_start (GTK_BOX (hbox_distance), entry_distance, FALSE, TRUE, 0);
	gtk_entry_set_max_length (GTK_ENTRY (entry_distance), 5);
	gtk_entry_set_text (GTK_ENTRY (entry_distance), _("10"));
	gtk_entry_set_width_chars (GTK_ENTRY (entry_distance), 5);
	g_signal_connect (entry_distance, "activate",
				GTK_SIGNAL_FUNC (evaluate_poi_search_cb), &criteria);
	
	label_distfrom = gtk_label_new (_("km from"));
	gtk_box_pack_start (GTK_BOX (hbox_distance), label_distfrom, FALSE, FALSE, 0);
	
	radiobutton_distcurrent = gtk_radio_button_new_with_mnemonic (NULL, _("current position"));
	gtk_box_pack_start (GTK_BOX (hbox_distance), radiobutton_distcurrent, FALSE, FALSE, 15);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_distcurrent), radiobutton_distance_group);
	radiobutton_distance_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton_distcurrent));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_distcurrent), TRUE);
	gtk_tooltips_set_tip ( tooltips_poilookup, radiobutton_distcurrent, 
				_("Search near current Position"), NULL);
	g_signal_connect (radiobutton_distcurrent, "toggled",
				GTK_SIGNAL_FUNC (searchdistancemode_cb), NULL);
	
	radiobutton_distcursor = gtk_radio_button_new_with_mnemonic (NULL, _("Destination/Cursor"));
	gtk_box_pack_start (GTK_BOX (hbox_distance), radiobutton_distcursor, FALSE, FALSE, 10);
	gtk_container_set_border_width (GTK_CONTAINER (radiobutton_distcursor), 5);
	gtk_button_set_focus_on_click (GTK_BUTTON (radiobutton_distcursor), FALSE);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_distcursor), radiobutton_distance_group);
	radiobutton_distance_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton_distcursor));
	gtk_tooltips_set_tip ( tooltips_poilookup, radiobutton_distcursor, 
 				_("Search near selected Destination"), NULL);
	 
	/* POI-Type Selection */
	hbox_type = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_criteria), hbox_type, TRUE, TRUE, 0);

	label_type = gtk_label_new (_("<i>POI-Types:</i>"));
	gtk_box_pack_start (GTK_BOX (hbox_type), label_type, FALSE, FALSE, 10);
	gtk_label_set_use_markup (GTK_LABEL (label_type), TRUE);
	gtk_label_set_single_line_mode (GTK_LABEL (label_type), TRUE);
	
	button_types = gtk_button_new_with_mnemonic ("...");
	gtk_box_pack_end (GTK_BOX (hbox_type), button_types, FALSE, FALSE, 0);
	g_signal_connect_swapped (button_types, "clicked",
				GTK_SIGNAL_FUNC (toggle_window_cb), poi_types_window);
	
	comboboxentry_type = gtk_combo_box_entry_new ();
	gtk_box_pack_end (GTK_BOX (hbox_type), comboboxentry_type, TRUE, TRUE, 5);
	gtk_widget_set_sensitive ( comboboxentry_type, FALSE );
  
	radiobutton_typeall = gtk_radio_button_new_with_mnemonic (NULL, _("all"));
	gtk_box_pack_start (GTK_BOX (hbox_type), radiobutton_typeall, FALSE, FALSE, 10);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_typeall), radiobutton_type_group);
	radiobutton_type_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton_typeall));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radiobutton_typeall), TRUE);
	gtk_tooltips_set_tip ( tooltips_poilookup, radiobutton_typeall,
				_("Search all POI-Categories"), NULL);
	
	radiobutton_typesel = gtk_radio_button_new_with_mnemonic (NULL, _("selected:"));
	gtk_box_pack_start (GTK_BOX (hbox_type), radiobutton_typesel, FALSE, FALSE, 10);
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (radiobutton_typesel), radiobutton_type_group);
	gtk_tooltips_set_tip ( tooltips_poilookup, radiobutton_typesel,
				_("Search only in selected POI-Categories"), NULL);
	
	g_signal_connect_swapped (radiobutton_typesel, "toggled",
				GTK_SIGNAL_FUNC (searchpoitypemode_cb), comboboxentry_type);

	// ### disable POI-Type selection buttons, until the functionality is completely implemented:
	gtk_widget_set_sensitive ( radiobutton_typesel, FALSE );
	gtk_widget_set_sensitive ( button_types, FALSE );

	label_criteria = gtk_label_new (_("Search Criteria"));
	gtk_widget_show (label_criteria);
	gtk_expander_set_label_widget (GTK_EXPANDER (expander_poisearch), label_criteria);
  }
	
	/* Frame: POI-Results */
  {
	frame_poiresults = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_searchbox), frame_poiresults, TRUE, TRUE, 5);
	gtk_frame_set_label_align (GTK_FRAME (frame_poiresults), 0.02, 0.5);

	alignment_poiresults = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_container_add (GTK_CONTAINER (frame_poiresults), alignment_poiresults);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment_poiresults), 2, 2, 2, 2);

	vbox_poiresults = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (alignment_poiresults), vbox_poiresults);

	scrolledwindow_poilist = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox_poiresults), scrolledwindow_poilist, TRUE, TRUE, 0);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_poilist), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_poilist), GTK_SHADOW_IN);

	treeview_poilist = gtk_tree_view_new_with_model (GTK_TREE_MODEL (poi_result_tree));

	renderer_poilist = gtk_cell_renderer_pixbuf_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes ("_", renderer_poilist, "pixbuf", RESULT_TYPE_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist), column_poilist);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
				_("Name"), renderer_poilist,
				"text", RESULT_NAME,
				NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist, RESULT_NAME);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist), column_poilist);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
				_("Distance"), renderer_poilist,
				"text", RESULT_DISTANCE,
				NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist, RESULT_DIST_NUM);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist), column_poilist);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
				_("Type"), renderer_poilist,
				"text", RESULT_TYPE_TITLE,
			NULL);
	g_object_set (G_OBJECT (renderer_poilist),
				"foreground-gdk", &textback,
				NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist, RESULT_TYPE_TITLE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist), column_poilist);

	renderer_poilist = gtk_cell_renderer_text_new ();
	column_poilist = gtk_tree_view_column_new_with_attributes (
				_("Comment"), renderer_poilist,
				"text", RESULT_COMMENT,
				NULL);
	gtk_tree_view_column_set_sort_column_id (column_poilist, RESULT_COMMENT);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_poilist), column_poilist);

	gtk_container_add (GTK_CONTAINER (scrolledwindow_poilist), treeview_poilist);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview_poilist), TRUE);

	poilist_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_poilist));
	gtk_tree_selection_set_mode (poilist_select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (poilist_select), "changed",
					G_CALLBACK (select_poi_cb), NULL);

	hbox_poistatus = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_poiresults), hbox_poistatus, FALSE, TRUE, 0);

	statusbar_poilist = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (hbox_poistatus), statusbar_poilist, TRUE, TRUE, 0);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar_poilist), FALSE);
	statusbar_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar_poilist), "poilist");
	gtk_statusbar_push (GTK_STATUSBAR (statusbar_poilist), statusbar_id, _(" Please enter your search criteria!"));
	
	/* button "POI-Info" */
	togglebutton_poiinfo = gtk_toggle_button_new ();
	gtk_box_pack_start (GTK_BOX (hbox_poistatus), togglebutton_poiinfo, FALSE, FALSE, 0);
	alignment_poiinfo = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (togglebutton_poiinfo), alignment_poiinfo);
	hbox11 = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_poiinfo), hbox11);
	image_poiinfo = gtk_image_new_from_stock ("gtk-info", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox11), image_poiinfo, FALSE, FALSE, 0);
	label_poiinfo = gtk_label_new_with_mnemonic (_("POI-Info"));
	gtk_box_pack_start (GTK_BOX (hbox11), label_poiinfo, FALSE, FALSE, 0);
	gtk_tooltips_set_tip ( tooltips_poilookup, togglebutton_poiinfo, 
				_("Show detailed Information for selected Point of Interest"), NULL);

	// ### disable POI-Info button, until the functionality is completed:
	gtk_widget_set_sensitive (togglebutton_poiinfo, FALSE);

	label_results = gtk_label_new (_("Results"));
	gtk_frame_set_label_widget (GTK_FRAME (frame_poiresults), label_results);
	gtk_label_set_use_markup (GTK_LABEL (label_results), TRUE);
  }
  
	dialog_action_area_poisearch = GTK_DIALOG (poi_lookup_window)->action_area;

	/* button "edit route" */
	button_addtoroute = gtk_button_new ();
	gtk_dialog_add_action_widget (GTK_DIALOG (poi_lookup_window), button_addtoroute, 0);
	GTK_WIDGET_SET_FLAGS (button_addtoroute, GTK_CAN_DEFAULT);
	alignment_addtoroute = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_addtoroute), alignment_addtoroute);
	hbox_addtoroute = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_addtoroute), hbox_addtoroute);
	image_addtoroute = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox_addtoroute), image_addtoroute, FALSE, FALSE, 0);
	label_addtoroute = gtk_label_new_with_mnemonic (_("Edit _Route"));
	gtk_box_pack_start (GTK_BOX (hbox_addtoroute), label_addtoroute, FALSE, FALSE, 0);
	gtk_tooltips_set_tip ( tooltips_poilookup, button_addtoroute, 
	_("Switch to Add selected entry to Route"), NULL);
	g_signal_connect (button_addtoroute, "clicked", GTK_SIGNAL_FUNC (route_window_cb), NULL);  

	/* button "delete POI" */
	button_delete = gtk_button_new_from_stock ("gtk-delete");
	gtk_dialog_add_action_widget (GTK_DIALOG (poi_lookup_window), button_delete, 0);
	GTK_WIDGET_SET_FLAGS (button_delete, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip ( tooltips_poilookup, button_delete, 
				_("Delete selected entry"), NULL);
	g_signal_connect_swapped (button_delete, "clicked",
				GTK_SIGNAL_FUNC (delete_poi_cb), poilist_select);

	/* button "Select as Destination" */
	button_target = gtk_button_new ();
	gtk_dialog_add_action_widget (GTK_DIALOG (poi_lookup_window), button_target, 0);
	GTK_WIDGET_SET_FLAGS (button_target, GTK_CAN_DEFAULT);
	alignment_target = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_target), alignment_target);
	hbox_target = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_target), hbox_target);
	image_target = gtk_image_new_from_stock ("gtk-jump-to", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox_target), image_target, FALSE, FALSE, 0);
	if (posmode)
	{
		label_target = gtk_label_new (_("Jump to Target"));		
		gtk_tooltips_set_tip ( tooltips_poilookup, button_target, 
					_("Jump to selected entry"), NULL);
	}
	else
	{
		label_target = gtk_label_new (_("Select Target"));
		gtk_tooltips_set_tip ( tooltips_poilookup, button_target, 
					_("Use selected entry as target destination"), NULL);
	}
	gtk_box_pack_start (GTK_BOX (hbox_target), label_target, FALSE, FALSE, 0);
	g_signal_connect_swapped (button_target, "clicked",
				GTK_SIGNAL_FUNC (select_target_poi_cb), poi_lookup_window);

	/* button "close" */
	button_close = gtk_button_new_from_stock ("gtk-close");
	gtk_dialog_add_action_widget (GTK_DIALOG (poi_lookup_window), button_close, GTK_RESPONSE_CLOSE);
	GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip ( tooltips_poilookup, button_close, 
				_("Close this window"), NULL);
	g_signal_connect_swapped (button_close, "clicked",
				GTK_SIGNAL_FUNC (close_poi_lookup_window_cb), poi_lookup_window);
	g_signal_connect (GTK_OBJECT (poi_lookup_window), "delete_event",
				   GTK_SIGNAL_FUNC (close_poi_lookup_window_cb), NULL);

	/* disable delete button until POI is selected from list */
	gtk_widget_set_sensitive (button_delete, FALSE);
	
	/* disable target selection in active routemode */
	if (route.active)
		gtk_widget_set_sensitive (button_target, FALSE);

	gtk_widget_grab_focus (entry_text);
	
	gtk_widget_show_all (poi_lookup_window);
}


/* *****************************************************************************
 * Window: POI-Types
 */
GtkWidget*
create_poi_types_window (void)
{
	GtkWidget *poi_types_window;
	GtkWidget *vbox_poi_types;
	GtkWidget *scrolledwindow_poitypes;
	GtkWidget *hbox_status;
	GtkWidget *statusbar;
	GtkWidget *button_close;
	GtkWidget *poitypes_treeview;
	GtkCellRenderer *renderer_poitypes;
	GtkTreeViewColumn *column_poitypes;
	GtkTreeSelection *poitypes_select;

	poi_types_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (poi_types_window), _("POI-Type Selection"));
	if (pdamode)
		gtk_window_set_default_size (GTK_WINDOW (poi_types_window), real_screen_x, real_screen_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (poi_types_window), 250, 300);

	vbox_poi_types = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (poi_types_window), vbox_poi_types);
 
	scrolledwindow_poitypes = gtk_scrolled_window_new (NULL, NULL);
	gtk_box_pack_start (GTK_BOX (vbox_poi_types), scrolledwindow_poitypes, TRUE, TRUE, 0);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_poitypes), GTK_SHADOW_IN);

	poitypes_treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (poi_types_tree));
	gtk_container_add (GTK_CONTAINER (scrolledwindow_poitypes), poitypes_treeview);

	renderer_poitypes = gtk_cell_renderer_pixbuf_new ();
	column_poitypes = gtk_tree_view_column_new_with_attributes ("_", renderer_poitypes, "pixbuf", POITYPE_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview), column_poitypes);

	renderer_poitypes = gtk_cell_renderer_text_new ();
	column_poitypes = gtk_tree_view_column_new_with_attributes (
				_("Name"), renderer_poitypes,
				"text", POITYPE_TITLE,
				NULL);
				
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview), column_poitypes);

	renderer_poitypes = gtk_cell_renderer_text_new ();
	column_poitypes = gtk_tree_view_column_new_with_attributes (
				_("ID"), renderer_poitypes,
				"text", POITYPE_NAME,
				NULL);
	g_object_set (G_OBJECT (renderer_poitypes),
				"foreground-gdk", &textback,
				NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (poitypes_treeview), column_poitypes);

	gtk_tree_view_set_enable_search (GTK_TREE_VIEW (poitypes_treeview), TRUE);
	gtk_tree_view_set_search_column (GTK_TREE_VIEW (poitypes_treeview), POITYPE_TITLE);
	poitypes_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (poitypes_treeview));
	gtk_tree_selection_set_mode (poitypes_select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (poitypes_select), "changed",
					G_CALLBACK (select_poitype_cb), NULL);

	hbox_status = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox_poi_types), hbox_status, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox_status), 2);

	statusbar = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (hbox_status), statusbar, TRUE, TRUE, 0);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);

	button_close = gtk_button_new_from_stock ("gtk-close");
	gtk_box_pack_end (GTK_BOX (hbox_status), button_close, FALSE, FALSE, 0);
	g_signal_connect_swapped (button_close, "clicked", 
				GTK_SIGNAL_FUNC (toggle_window_cb), poi_types_window);

	return poi_types_window;
}


/* *****************************************************************************
 * Button: Add new Waypoint
 */
void create_button_add_wp (void)
{
	GtkWidget *add_wp_bt;
	gchar imagefile[400];
	GdkPixbuf *pixmap = NULL;
	
	local_config.showaddwpbutton = TRUE; // ###
	
	if (local_config.showaddwpbutton)
	{
		g_snprintf (imagefile, sizeof (imagefile), "%s/gpsdrive/%s", DATADIR, "pixmaps/add_waypoint.png");
		pixmap = gdk_pixbuf_new_from_file (imagefile, NULL);
		if (imagefile == NULL)
		{
			fprintf (stderr, _("\nWarning: unable to open 'add waypoint' picture\nPlease install the program as root with:\nmake install\n\n"));
			return;
		}
		
		add_wp_bt = gtk_button_new ();
		gtk_button_set_relief (GTK_BUTTON (add_wp_bt), GTK_RELIEF_NONE);
		gtk_button_set_image (GTK_BUTTON (add_wp_bt), GTK_WIDGET (pixmap));
		gtk_container_add (GTK_CONTAINER (drawing_area), add_wp_bt);
		gtk_widget_show_all (add_wp_bt);
		return;
	}
	else
	{
		return;
	}
}


/* *****************************************************************************
 * Window: Edit Route
 */
void route_window_cb (GtkWidget *calling_button)
{
	GtkWidget *dialog_vbox_route;
	GtkWidget *vbox_routedetails;
	GtkWidget *frame_routedetails;
	GtkWidget *scrolledwindow_routedetails;
	GtkWidget *alignment_startroute;
	GtkWidget *hbox_startroute;
	GtkWidget *image_startroute;
	GtkWidget *label_startroute;
	GtkWidget *label_routedetails;
	GtkWidget *treeview_routelist;
	GtkCellRenderer *renderer_routelist;
	GtkTreeViewColumn *column_routelist;
	GtkTreeSelection *routelist_select;
	GtkWidget *dialog_action_area_routedetails;
	GtkWidget *button_close;
	GtkWidget *button_cancel;
	GtkWidget *alignment_cancel;
	GtkWidget *hbox_cancel;
	GtkWidget *image_cancel;
	GtkWidget *label_cancel;
	GtkTooltips *tooltips_routewindow;
	
	gtk_widget_set_sensitive (button_addtoroute, FALSE);
	
	route.edit = TRUE;
	
	if (!route.available)
		route.items = 0;

	tooltips_routewindow = gtk_tooltips_new();
	gtk_tooltips_set_delay (tooltips_routewindow, TOOLTIP_DELAY);
	if (local_config.showtooltips)
		gtk_tooltips_enable (tooltips_routewindow);
	else
		gtk_tooltips_disable (tooltips_routewindow);
	  
	route_window = gtk_dialog_new ();
	gtk_container_set_border_width (GTK_CONTAINER (route_window), 2);
	gtk_window_set_title (GTK_WINDOW (route_window), _("Edit Route"));
	gtk_window_set_position (GTK_WINDOW (route_window), GTK_WIN_POS_NONE);
	gtk_window_set_type_hint (GTK_WINDOW (route_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	if (pdamode)
		gtk_window_set_default_size (GTK_WINDOW (route_window), real_screen_x, real_screen_y);
	else
		gtk_window_set_default_size (GTK_WINDOW (route_window), -1, 250);
	
	dialog_vbox_route = GTK_DIALOG (route_window)->vbox;
	
	vbox_routedetails = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (dialog_vbox_route), vbox_routedetails, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox_routedetails), 2);
	
	/* Frame: Route-Details */
  {
	frame_routedetails = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (vbox_routedetails), frame_routedetails, TRUE, TRUE, 5);
	gtk_frame_set_label_align (GTK_FRAME (frame_routedetails), 0.02, 0.5);

	scrolledwindow_routedetails = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_routedetails), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_routedetails), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame_routedetails), scrolledwindow_routedetails);

	treeview_routelist = gtk_tree_view_new_with_model (GTK_TREE_MODEL (route_list_tree));

	  	// ### show number of point in route list...
	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
				"#", renderer_routelist,
				"text", ROUTE_NUMBER,
				NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist), column_routelist);
	  
	renderer_routelist = gtk_cell_renderer_pixbuf_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes ("_", renderer_routelist, "pixbuf", ROUTE_ICON, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist), column_routelist);

	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
				_("Name"), renderer_routelist,
				"text", ROUTE_NAME,
				NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist), column_routelist);

	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
				_("Distance"), renderer_routelist,
				"text", ROUTE_DISTANCE,
				NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist), column_routelist);
	
	renderer_routelist = gtk_cell_renderer_text_new ();
	column_routelist = gtk_tree_view_column_new_with_attributes (
				_("Trip"), renderer_routelist,
				"text", ROUTE_TRIP,
				NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_routelist), column_routelist);

	gtk_container_add (GTK_CONTAINER (scrolledwindow_routedetails), treeview_routelist);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview_routelist), TRUE);

	routelist_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_routelist));
	gtk_tree_selection_set_mode (routelist_select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (routelist_select), "changed",
					G_CALLBACK (select_routepoint_cb), NULL);

	label_routedetails = gtk_label_new (_("Route List"));
	gtk_frame_set_label_widget (GTK_FRAME (frame_routedetails), label_routedetails);
	gtk_label_set_use_markup (GTK_LABEL (label_routedetails), TRUE);
  }
  
	dialog_action_area_routedetails = GTK_DIALOG (route_window)->action_area;

	/* button "start route" */
	button_startroute = gtk_button_new ();
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_startroute, 0);
	GTK_WIDGET_SET_FLAGS (button_startroute, GTK_CAN_DEFAULT);
	alignment_startroute = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_startroute), alignment_startroute);
	hbox_startroute = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_startroute), hbox_startroute);
	if (route.active)
	{
		image_startroute = gtk_image_new_from_stock ("gtk-media-stop", GTK_ICON_SIZE_BUTTON);
		label_startroute = gtk_label_new_with_mnemonic (_("Stop Route"));
		gtk_tooltips_set_tip ( tooltips_routewindow, button_startroute,
					_("Stop the Route Mode"), NULL);
	}
	else
	{
		image_startroute = gtk_image_new_from_stock ("gtk-media-play", GTK_ICON_SIZE_BUTTON);
		label_startroute = gtk_label_new_with_mnemonic (_("Start Route"));
		gtk_tooltips_set_tip ( tooltips_routewindow, button_startroute,
					_("Start the Route Mode"), NULL);
	}
	if (!route.available)
		gtk_widget_set_sensitive (button_startroute, FALSE);
	gtk_box_pack_start (GTK_BOX (hbox_startroute), image_startroute, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox_startroute), label_startroute, FALSE, FALSE, 0);
	g_signal_connect (button_startroute, "clicked", GTK_SIGNAL_FUNC (route_startstop_cb), NULL);

	/* button "remove point from route" */
	button_remove = gtk_button_new_from_stock ("gtk-remove");
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_remove, 0);
	GTK_WIDGET_SET_FLAGS (button_remove, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip ( tooltips_routewindow, button_remove, 
				_("Remove selected Entry from Route"), NULL);
	gtk_widget_set_sensitive (button_remove, FALSE);
	g_signal_connect_swapped (button_remove, "clicked",
				GTK_SIGNAL_FUNC (remove_routepoint_cb), routelist_select);

	/* button "cancel route" */
	button_cancel = gtk_button_new ();
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_cancel, 0);
	GTK_WIDGET_SET_FLAGS (button_cancel, GTK_CAN_DEFAULT);
	alignment_cancel = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_container_add (GTK_CONTAINER (button_cancel), alignment_cancel);
	hbox_cancel = gtk_hbox_new (FALSE, 2);
	gtk_container_add (GTK_CONTAINER (alignment_cancel), hbox_cancel);
	image_cancel = gtk_image_new_from_stock ("gtk-cancel", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start (GTK_BOX (hbox_cancel), image_cancel, FALSE, FALSE, 0);
	label_cancel = gtk_label_new (_("Cancel Route"));		
	gtk_tooltips_set_tip ( tooltips_routewindow, button_cancel,
				_("Discard Route"), NULL);
	gtk_box_pack_start (GTK_BOX (hbox_cancel), label_cancel, FALSE, FALSE, 0);
	g_signal_connect (button_cancel, "clicked",
				GTK_SIGNAL_FUNC (route_cancel_cb), NULL);

	/* button "close" */
	button_close = gtk_button_new_from_stock ("gtk-close");
	gtk_dialog_add_action_widget (GTK_DIALOG (route_window), button_close, GTK_RESPONSE_CLOSE);
	GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
	gtk_tooltips_set_tip (tooltips_routewindow, button_close, 
				_("Close this window"), NULL);
	g_signal_connect_swapped (button_close, "clicked",
				GTK_SIGNAL_FUNC (close_route_window_cb), route_window);
	g_signal_connect (GTK_OBJECT (route_window), "delete_event",
				   GTK_SIGNAL_FUNC (close_route_window_cb), NULL);

	gtk_widget_show_all (route_window);
}


/* *****************************************************************************
 * GUI Init:
 * This will call all the necessary functions to init the graphical interface
 */
int gui_init (void)
{

	//create_mainwindow();

	//create_button_add_wp();

	poi_types_window = create_poi_types_window ();

	return 0;
}
