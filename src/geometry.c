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
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <gpsdrive.h>
#include <config.h>
#include <math.h>


/* variables */
extern gint ignorechecksum, mydebug;
extern gdouble zero_lon, zero_lat, target_lon, target_lat, dist;
extern gint real_screen_x, real_screen_y, real_psize, real_smallmenu,
  int_padding;
extern gint SCREEN_X_2, SCREEN_Y_2;
extern gdouble pixelfact, posx, posy, angle_to_destination, direction,
  bearing;
extern gint havepos, haveposcount, blink, gblink, xoff, yoff, crosstoogle;


/* ******************************************************************
 * liang-barsky line clipping
 */
int clipt(gdouble d, gdouble n, gdouble *te, gdouble *tl)
{
    gdouble t;
    int ac = 1;

    if (d > 0.0) {
	t = n / d;
	if (t > *tl) {
	    ac = 0;
	} else if (t > *te) {
	    *te = t;
	}
    } else if (d < 0.0) {
	t = n / d;
	if (t < *te) {
	    ac = 0;
	} else if (t < *tl) {
	    *tl = t;
	}
    } else {
	if (n > 0.0) {
	    ac = 0;
	}
    }
    return ac;
}

static int clip_pointxy(gdouble x,   gdouble y,
		       gdouble xg1, gdouble yg1,
		       gdouble xg2, gdouble yg2)
{
    return ((x >= xg1) && (x <= xg2) && (y >= yg1) && (y <= yg2));
}


/* ******************************************************************
 */
gint
line_crosses_rectangle(gdouble x0, gdouble y0, gdouble x1, gdouble y1,
		       gdouble xg1, gdouble yg1,
		       gdouble xg2, gdouble yg2	  )
{
    gdouble te = 0.0;
    gdouble tl = 1.0;
    gdouble dx = (x1 - x0);
    gdouble dy = (y1 - y0);

    if (dx == 0.0 && 
	dy == 0.0 && 
	clip_pointxy(x1, y1,
		     xg1, yg1,
		     xg2, yg2)
	) {
	return 1;
    }
    if (clipt(dx, xg1 - x0, &te, &tl)) {
	if (clipt(-dx, x0 - xg2, &te, &tl)) {
	    if (clipt(dy, yg1 - y0, &te, &tl)) {
		if (clipt(-dy, y0 - yg2, &te, &tl)) {
		    return 1;
		}
	    }
	}
    }
    return 0;
}

/* ******************************************************************
 * Find the shortest distance from a point to a line-segment
 */
gdouble
distance_line_point(gdouble x1, gdouble y1, gdouble x2, gdouble y2,
		    gdouble xp, gdouble yp)
{

    //if ( mydebug >0 ) 	
    // fprintf( stderr, "distance_line_point(%g,%g, %g,%g,   %g,%g)\n", x1, y1, x2, y2,  xp, yp);
    
    gdouble dx1p = x1 - xp;
    gdouble dx21 = x2 - x1;
    gdouble dy1p = y1 - yp;
    gdouble dy21 = y2 - y1;
    gdouble frac = dx21*dx21 + dy21*dy21;

    gdouble lambda = -(dx1p*dx21 + dy1p*dy21) / frac;
    //if ( mydebug > 10 ) printf ("distance_line_point(): lambda_1: %g\n",lambda);
    lambda = min(max(lambda,0.0),1.0);
   
    //if ( mydebug > 10 ) printf ("distance_line_point(): lambda: %g\n",lambda);

    gdouble xsep = dx1p + lambda*dx21;
    gdouble ysep = dy1p + lambda*dy21;
    return sqrt(xsep*xsep + ysep*ysep);
}
