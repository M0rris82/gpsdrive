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

#include "gui.h"


    /* External Values */

    extern gint mydebug;
extern gint debug;

extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu, int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;

extern gint borderlimit;

extern gint extrawinmenu, pdamode;
extern gdouble posx, posy;

/* Global Values */


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
    int width, height;
    uint x, y;



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

int gui_init (void) {



    return 0;
}
