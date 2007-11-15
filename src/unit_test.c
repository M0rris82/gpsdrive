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
#include "speech_out.h"
#include "speech_strings.h"
#include "battery.h"
#include "nmea_handler.h"
#include "gui.h"

gint errors = 0;

extern gint mydebug;
extern coordinate_struct coords;
//extern gint mapistopo;
extern glong mapscale;
extern gdouble pixelfact;
extern int usesql;
extern gchar dir_proc[200];
extern gchar cputempstring[20], batstring[20];
extern int didrootcheck;
extern int newdata;
extern char serialdata[4096];
extern gint haveRMCsentence;
extern currentstatus_struct current;

/* ******************************************************************
 * This Function tests internal routines of gpsdrive
 */

void set_unittest_timer (void);

/* ******************************************************************
 * write a file with content in one function call
 */
void
write_file (gchar * filename, gchar * content)
{
  FILE *fp;
  fp = fopen (filename, "w");
  if (fp == NULL)
    {
      fprintf (stderr, "!!! ERROR: Error opening %s\n", filename);
      perror ("ERROR");
      errors++;
    }

  fprintf (fp, "%s", content);
  fclose (fp);
}


/* ******************************************************************
 * Test the nmea parser, simply check if the right position is set after parsing
 */
gint  unit_test_nmea()
{
    gint errors = 0;
    if (mydebug > 0)
	printf ("\n");
    printf ("Testing nmea handler\n");

    typedef struct
    {
	gdouble should_lat, should_lon;
	char *nmea_string;
    } test_struct;
    test_struct test_array[] = {
	/*  nothing happens (lat/lon) with these nmea sentences
	  {55.6403, 	12.6378, "$GPGSV,3,1,10,29,66,286,43,28,57,126,35,26,57,290,45,08,51,073,29*7E" },
	  {55.6403, 	12.6378, "$GPGSV,3,2,10,10,34,201,40,27,25,076,00,19,14,033,00,21,12,305,27*75" },
	  {55.6403, 	12.6378, "$GPGSV,3,3,10,15,11,329,00,18,11,325,00*79" },
	  {55.6403, 	12.6378, "$GPGSA,A,3,10,28,26,29,08,21,,,,,,,2.5,1.4,2.0*3D" },
	  {55.6403, 	12.6378, "$GPGSA,A,3,10,28,26,29,08,21,,,,,,,2.5,1.4,2.0*3D" },
	*/
	/* unsuported sentence for nmea reading
	   {55.64039333333, 	12.63781833333, "$GPGLL,5538.4236,N,01238.2691,E,122041.481,A*31" },
	   {-55.640395, 	 12.63783, "$GPGLL,5538.4237,S,01238.2698,E,122040.481,A*38" },
	*/

	{ 55.640393333,  12.637818333, "$GPRMC,122041.481,A,5538.4236,N,01238.2691,E,0.000000,214.43,010806,,*09" },
	{ 55.640395, 	-12.63783, "$GPRMC,122040.481,A,5538.4237,N,01238.2698,W,0.000000,214.43,010806,,*12" },
	{ 55.640395, 	 12.63783, "$GPRMC,122040.481,A,5538.4237,N,01238.2698,E,0.000000,214.43,010806,,*00" },
	{-55.640395, 	-12.63783, "$GPRMC,122040.481,A,5538.4237,S,01238.2698,W,0.000000,214.43,010806,,*0F" },
	{-55.640395, 	 12.63783, "$GPRMC,122040.481,A,5538.4237,S,01238.2698,E,0.000000,214.43,010806,,*1D" },

	{ 52.299051666,   9.638140, "$GPRMC,165318.993,A,5217.9431,N,00938.2884,E,000.0,000.0,010806,001.1,E*66"},

	{ 55.640393333,  12.637818333, "$GPGGA,122041.481,5538.4236,N,01238.2691,E,2,06,1.4,40.4,M,41.4,M,1.1,0000*46" },
	{ 55.640395, 	 12.63783,     "$GPGGA,122040.481,5538.4237,N,01238.2698,E,2,06,1.4,39.8,M,41.4,M,1.1,0000*4D" },
	{ 48.117500,11.595000,    "$GPGGA,125500.481,4807.0500,N,01135.7000,E,2,06,1.4,39.8,M,41.4,M,1.1,0000*40" },
	{-99,-99,""},
    };
    gint i;
    gdouble diff;
    for (i = 0; test_array[i].should_lat != -99; i++)
	{
	    haveRMCsentence=FALSE;
	    newdata=TRUE;
	    gui_status.posmode=FALSE;
	    //strncpy ( serialdata, test_array[i].nmea_string,sizeof (serialdata));
	    get_position_data_cb(NULL,NULL);

	    int ok=TRUE;

	    diff = fabs(coords.current_lat - test_array[i].should_lat);
	    if ( diff > 0.000001  )
		{
		    printf ("!!!! ERROR wrong lat diff: %f\n",diff);
		    ok=FALSE;
		}
	    diff = fabs(coords.current_lon - test_array[i].should_lon);
	    if ( diff >0.000001  )
		{
		    printf ("!!!! ERROR wrong lon diff: %f\n",diff);
		    ok=FALSE;
		}
	    if ( ! ok ) {
		printf ("!!!! ERROR is %f,%f\n"
			"       should %f,%f\n"
			"       nmea: %s\n",
			coords.current_lat,coords.current_lon,
			test_array[i].should_lat,test_array[i].should_lon,
			test_array[i].nmea_string
			);
		errors++;
	    } else {
		if ( mydebug>1 ) 
		    printf ("parsing OK; values: %f,%f\t"
			" nmea: %s\n",
			coords.current_lat,coords.current_lon,
			test_array[i].nmea_string
			);
		
	    }
	}

    return errors;
}

/* ******************************************************************
 * Unit Tests
 * in this function some of the internal functions of gpsdrive are called
 * with known parameters. The results then are checked to what is expected.
 * If a non expected/wrong result is detected we exit with -1
 *
 * This way I hope we can reduce the number of errors comming back after
 * already having been fixed.
 */
void
unit_test (void)
{

    printf ("\n");
    printf ("#############################################################################\n");
    printf ("#############################################################################\n");
    printf ("#############################################################################\n");
    printf ("\n");
    printf ("GpsDrive Started to do UNIT Tests\n");
    printf ("\n");
    printf ("#############################################################################\n");
    printf ("#############################################################################\n");
    printf ("#############################################################################\n");
    printf ("\n");


  // ------------------------------------------------------------------
  {
    if (mydebug > 0)
      printf ("\n");
    printf ("Testing line_crosses_rectangle(lx/y,lx/y, rx/y,rx/y)\n");

    typedef struct
    {
      gint result;
      gdouble x1, y1, x2, y2;
      gdouble xr1, ry1, rx2, ry2;
    } test_struct;
    test_struct test_array[] = {
      {1, 2, 2, 15, 6, 4, 3, 10, 5},
      {1, 2, 2, 15, 6, 4, 3, 10, 5},
      {1, 6, 4, 5, 6, 4, 3, 10, 5},
      {1, 8, 2, 12, 6, 4, 3, 10, 5},
      {1, 6, 2, 7, 7, 4, 3, 10, 5},
      {0, 8, 6, 12, 8, 4, 3, 10, 5},
      {0, 12, 4, 15, 4, 4, 3, 10, 5},
      {0, 5, 6, 5, 8, 4, 3, 10, 5},

      {-99, 0, 0, 0, 0, 0, 0, 0, 0}
    };

    gint i;
    for (i = 0; test_array[i].result != -99; i++)
      {
	gint erg = line_crosses_rectangle (test_array[i].x1, test_array[i].y1,
					   test_array[i].x2, test_array[i].y2,
					   test_array[i].xr1,
					   test_array[i].ry1,
					   test_array[i].rx2,
					   test_array[i].ry2);
	if (erg == test_array[i].result)
	  {
	    if (mydebug > 0)
	      fprintf (stderr,
		       "	%d: line_crosses_rectangle(%g,%g %g,%g,   %g,%g %g,%g)	==> %d\n",
		       i,
		       test_array[i].x1, test_array[i].y1,
		       test_array[i].x2, test_array[i].y2,
		       test_array[i].xr1, test_array[i].ry1,
		       test_array[i].rx2, test_array[i].ry2, erg);
	  }
	else
	  {
	    printf ("!!!! ERROR\n");
	    fprintf (stderr,
		     "	%d: line_crosses_rectangle(%g,%g %g,%g,   %g,%g %g,%g)"
		     " ==> %d "
		     "should be %d\n",
		     i,
		     test_array[i].x1, test_array[i].y1,
		     test_array[i].x2, test_array[i].y2,
		     test_array[i].xr1, test_array[i].ry1,
		     test_array[i].rx2, test_array[i].ry2,
		     erg, test_array[i].result);
	    errors++;
	  }
      }

  }

  // ------------------------------------------------------------------
  {
    if (mydebug > 0)
      printf ("\n");
    printf ("Testing distance_line_point()\n");

    typedef struct
    {
      gdouble dist, x1, y1, x2, y2, xp, yp;
    } test_struct;
    test_struct test_array[] = {
      // dist, line                   Point
      {0.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0},
      {1.0, 0.0, 0.0, 2.0, 0.0, 1.0, 1.0},
      {1.0, 0.0, 0.0, 2.0, 0.0, 1.0, -1.0},
      {1.0, 2.0, 0.0, 0.0, 0.0, 1.0, 1.0},
      {1.0, 2.0, 0.0, 0.0, 0.0, 1.0, -1.0},

      {sqrt (2 * 2 + 1 * 1), 2.0, 2.0, 6.0, 4.0, 8.0, 5.0},
      {sqrt (2 * 2 + 1 * 1), 2.0, 2.0, 6.0, 4.0, 0.0, 1.0},

      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 8.0, 5.0},
      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 0.0, 1.0},

      {sqrt (2 * 2 + 1 * 1), 2.0, 2.0, 6.0, 4.0, 3.0, 5.0},
      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 3.0, 5.0},

      {sqrt (2 * 2 + 1 * 1), 2.0, 2.0, 6.0, 4.0, 5.0, 1.0},
      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 5.0, 1.0},

      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 7.0, 6.0},
      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 7.0, 2.0},
      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 5.0, 6.0},
      {2, 6.0, 4.0, 2.0, 2.0, 6.0, 6.0},

      {2, 6.0, 4.0, 2.0, 2.0, 2.0, 0.0},
      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 3.0, 0.0},
      {sqrt (2 * 2 + 1 * 1), 6.0, 4.0, 2.0, 2.0, 1.0, 0.0},

      {-99.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    };

    gint i;
    for (i = 0; test_array[i].dist != -99; i++)
      {
	gdouble d;
	d = distance_line_point (test_array[i].x1, test_array[i].y1,
				 test_array[i].x2, test_array[i].y2,
				 test_array[i].xp, test_array[i].yp);
	if ((d - test_array[i].dist) < 0.00000001)
	  {
	    if (mydebug > 0)
	      fprintf (stderr,
		       "	%d: distance_line_point(%g,%g, %g,%g,   %g,%g)	==> %g\n",
		       i, test_array[i].x1, test_array[i].y1,
		       test_array[i].x2, test_array[i].y2,
		       test_array[i].xp, test_array[i].yp, d);
	  }
	else
	  {
	    printf ("!!!! ERROR\n");
	    printf
	      ("distance_line_point(%g,%g, %g,%g,   %g,%g) = %g should be %g\n",
	       test_array[i].x1, test_array[i].y1, test_array[i].x2,
	       test_array[i].y2, test_array[i].xp, test_array[i].yp, d,
	       test_array[i].dist);
	    errors++;
	  }
      }
  }

  // ------------------------------------------------------------------
  {
    if (mydebug > 0)
      printf ("\n");
    printf ("Testing coordinate_string2gdouble()\n");

    gchar test_string1[100];
    gchar test_string2[100];
    gdouble coordinate;
    gdouble delta;

    typedef struct
    {
      gdouble number;
      gchar string[100];
    } test_struct;
    test_struct test_array[] = {
      {0.0, "0"},
      {0.0, "0.0"},
      {0.0, "0,0"},

      {1.0, "1"},
      {1.0, "1.0"},
      {1.0, "1,0"},

      {12.0, "12"},
      {12.0, "12.0"},
      {12.0, "12,0"},

      {-12.0, "-12"},
      {-12.0, "-12.0"},
      {-12.0, "-12,0"},

      {-12.0, "12W"},
      {-12.0, "12.0W"},
      {-12.0, "12,0W"},

      {121.7654789, "121.7654789"},
      {121.7654789, "121,7654789"},
      {121.7654789, " 121.7654789 "},
      {121.7654789, " 121,7654789 "},
      {121.7654789, "\t121,7654789 "},
      {121.7654789, " 121,7654789\t"},
      {121.7654789, "\n121,7654789\t"},
      {121.7654789, " 121,7654789\t"},

      {-121.7654789, "-121.7654789"},
      {-121.7654789, "-121,7654789"},
      {-121.7654789, " -121.7654789 "},
      {-121.7654789, " -121,7654789 "},
      {-121.7654789, "\t-121,7654789 "},
      {-121.7654789, " -121,7654789\t"},
      {-121.7654789, "\n-121,7654789\t"},
      {-121.7654789, " -121,7654789\t"},

      {-121.7654789, "121.7654789E"},
      {-121.7654789, "121,7654789E"},
      {-121.7654789, " 121.7654789E "},
      {-121.7654789, " 121,7654789E "},
      {-121.7654789, "\t121,7654789E "},
      {-121.7654789, " 121,7654789E\t"},
      {-121.7654789, "\n121,7654789E\t"},
      {-121.7654789, " 121,7654789E\t"},

      {12.35, "12" "\xc2" "\xb0" "21'E"},
      {-12.35, "12" "\xc2" "\xb0" "21'W"},
      {12.35, "12" "\xc2" "\xb0" "21'N"},
      {-12.35, "12" "\xc2" "\xb0" "21'S"},

      {12.35194444444444, "12" "\xc2" "\xb0" "21'7''E"},


      {0.0, "END"}
    };

    gint i;
    for (i = 0;
	 strncmp (test_array[i].string, "END", sizeof (test_array[i].string));
	 i++)
      {

	coordinate = -9999;
	g_strlcpy (test_string1, test_array[i].string, sizeof (test_string1));
	g_strlcpy (test_string2, test_string1, sizeof (test_string1));
	coordinate_string2gdouble (test_string1, &coordinate);
	if (mydebug > 0)
	  fprintf (stderr,
		   "	%d: coordinate_string2gdouble('%s')	--> %g\n", i,
		   test_string1, coordinate);
	delta = test_array[i].number - coordinate;
	if (delta > 1e-10)
	  {
	    printf ("!!!! ERROR\n");
	    printf ("	coordinate_string2gdouble('%s')	--> %g\n",
		    test_string1, coordinate);
	    errors++;
	  }
	// Will the input string be modified ?
	if (strcmp (test_string1, test_string2))
	  {
	    printf ("!!!! ERROR:	Input String changed\n");
	    errors++;
	    printf ("	coordinate_string2gdouble('%s')\n", test_string1);
	    printf ("	                     --> ('%s')\n", test_string2);
	  }

      }



    if (mydebug > 0)
      printf ("\n");
    printf ("Testing coordinate_string2gdouble()\n");


  }

  // ------------------------------------------------------------------
  {
    if (mydebug > 0)
      printf ("\n");
    printf ("Testing coordinate2gchar()\n");

    gchar test_string1[100];
    gdouble coordinate;

    // coordinate2gchar (gchar * buff, gint buff_size, gdouble pos, gint islat, gint mode)

    coordinate = 12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, FALSE,
		      1);
    if (mydebug > 1)
      printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("E 12" "\xc2" "\xb0" " 05' 60.00\"", test_string1))
      {
	printf ("!!!! ERROR\n");
	errors++;
      }

    coordinate = -12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, FALSE,
		      1);
    if (mydebug > 1)
      printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("W 12" "\xc2" "\xb0" " 05' 60.00\"", test_string1))
      {
	printf ("!!!! ERROR\n");
	errors++;
      }

    coordinate = 12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, TRUE,
		      1);
    if (mydebug > 1)
      printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("N 12" "\xc2" "\xb0" " 05' 60.00\"", test_string1))
      {
	printf ("!!!! ERROR\n");
	errors++;
      }

    coordinate = -12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, TRUE,
		      1);
    if (mydebug > 1)
      printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("S 12" "\xc2" "\xb0" " 05' 60.00\"", test_string1))
      {
	printf ("!!!! ERROR\n");
	errors++;
      }
  }

  // ------------------------------------------------------------------
  {
    if (mydebug > 0)
      printf ("\n");
    printf ("Testing calcxytopos() and calcxy()\n");

    // void calcxytopos (int posx, int posy, gdouble *mylat, gdouble *mylon, gint zoom);
    // void calcxy (gdouble *posx, gdouble *posy, gdouble lon, gdouble lat, gint zoom);

    map_proj = proj_map;
    current.zoom = 1;
    current.mapscale = 10000;
    pixelfact = current.mapscale / PIXELFACT;

    if (mydebug > 1)
      {
	printf ("	pixelfact: %g\n", pixelfact);
	printf ("	mapscale: %ld\n", current.mapscale);
	printf ("	SCREEN X/Y: %d/%d\n", SCREEN_X, SCREEN_Y);
	printf ("\n");
      }

    typedef struct
    {
      gdouble cur_lat, cur_lon;
      gdouble x, y;
      gdouble lat, lon;
    } test_struct;
    test_struct test_array[] = {
      {12, 48, SCREEN_X / 2, SCREEN_Y / 2, 12, 48},
      {12, 48, SCREEN_X, SCREEN_Y, 11.9848, 48.021},
      {12, 48, 0, SCREEN_Y, 11.9848, 47.979},
      {12, 48, SCREEN_X, 0, 12.0152, 48.021},
      {12, 48, 0, 0, 12.0152, 47.979},

      {-12, 48, SCREEN_X / 2, SCREEN_Y / 2, -12, 48},
      {-12, 48, SCREEN_X, SCREEN_Y, -11.9848, 48.021},
      {-12, 48, 0, SCREEN_Y, -11.9848, 47.979},
      {-12, 48, SCREEN_X, 0, -12.0152, 48.021},
      {-12, 48, 0, 0, -12.0152, 47.979},

      {12, -48, SCREEN_X / 2, SCREEN_Y / 2, 12, -48},
      {12, -48, SCREEN_X, SCREEN_Y, 11.9848, -48.021},
      {12, -48, 0, SCREEN_Y, 11.9848, -47.979},
      {12, -48, SCREEN_X, 0, 12.0152, -48.021},
      {12, -48, 0, 0, 12.0152, -47.979},

      {-12, -48, SCREEN_X / 2, SCREEN_Y / 2, -12, -48},
      {-12, -48, SCREEN_X, SCREEN_Y, -11.9848, -48.021},
      {-12, -48, 0, SCREEN_Y, -11.9848, -47.979},
      {-12, -48, SCREEN_X, 0, -12.0152, -48.021},
      {-12, -48, 0, 0, -12.0152, -47.979},

      {0, 0, -99.0, 0.0, 0.0, 0.0},
    };

    gint i;
    for (i = 0; test_array[i].x != -99; i++)
      {
	gdouble lat, lon;
	gint x, y;
	gdouble gx, gy;

	coords.current_lat = coords.zero_lat = test_array[i].cur_lat;
	coords.current_lon = coords.zero_lon = test_array[i].cur_lon;

	x = test_array[i].x;
	y = test_array[i].y;
	calcxytopos (x, y, &lat, &lon, current.zoom);
	if (mydebug > 0)
	  {
	    printf ("	%d: current_pos: %g,%g\n", i, coords.current_lat,
		    coords.current_lon);
	    fprintf (stderr,
		     "	%d: calcxytopos(%-7d,%-7d)	-->       (%g,%g)\n",
		     i, x, y, lat, lon);
	  }

	gdouble delta_lat = test_array[i].lat - lat;
	if (abs (delta_lat) > 1e-10)
	  {
	    printf ("!!!! ERROR:	Delta-lat(%g)\n", delta_lat);
	    errors++;
	    fprintf (stderr,
		     "!!ERROR:	calcxytopos(%-7.4d,%-7.4d)	-->       (%g,%g) should be  (%g,%g)\n",
		     x, y, lat, lon, test_array[i].lat, test_array[i].lon);
	  }

	// Backward transformation
	calcxy (&gx, &gy, lon, lat, current.zoom);
	if (mydebug > 0)
	  fprintf (stderr,
		   "	%d:            (%-7.4g,%-7.4g)	<-- calcxy(%g,%g)\n",
		   i, gx, gy, lat, lon);
	gint delta_x = gx - x;
	if (abs (delta_x) > 1)
	  {
	    printf ("!!!! ERROR:	Delta-x(%d)>1\n", delta_x);
	    errors++;
	  }
	gint delta_y = gy - y;
	if (abs (delta_y) > 1)
	  {
	    printf ("!!!! ERROR:	Delta-y(%d)>1\n", delta_y);
	    errors++;
	  }
	if (mydebug > 0)
	  printf ("\n");
      }
  }

  // ------------------------------------------------------------------
  {
    int res;

    if (mydebug > 0)
      printf ("\n");
    printf ("Testing if SQL Support is on\n");

    if (!usesql)
      {
	printf ("ERROR !!!!!!!!\n");
	printf ("Problem with SQL Support\n");
	errors++;
      }
    
    res = insertsqldata(999.9,999.9,"Testpoint","test","Test Description", 1);
    if (1>res)
      {
	printf ("ERROR !!!!!!!!\n");
	printf ("Problem inserting waypoint into mySQL Table waypoints\n");
	errors++;
      }
    
  }

  // ------------------------------------------------------------------
  {
    if (mydebug > 0)
      printf ("\n");
    printf ("Testing if acpi/apm is parsing correct\n");

    gchar fn[500];
    gint response;


    g_snprintf (dir_proc, sizeof (dir_proc), "/tmp/gpsdrive-unit-test");
    mkdir (dir_proc, 0777);
    g_snprintf (dir_proc, sizeof (dir_proc), "/tmp/gpsdrive-unit-test/proc");
    mkdir (dir_proc, 0777);

    if (mydebug > 0)
      printf ("	--------> remove maybe old Files\n");
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/state", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/state", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/info", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/info", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/apm", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone/ATF00/temperature",
		dir_proc);
    unlink (fn);


    if (mydebug > 0)
      printf
	("	-------> Check if we get positive answers even if no files should be there\n");
    if (battery_get_values ())
      {
	printf ("battery reporting Problem: battery status without file\n");
	errors++;
      }

    if (temperature_get_values ())
      {
	printf
	  ("temperature reporting Problemtem: temperature without file\n");
	errors++;
      }

    if (mydebug > 0)
      printf ("	-------> Create Dummy Files/dirs for ACPI\n");
    g_snprintf (fn, sizeof (fn), dir_proc);
    g_strlcat (fn, "/acpi", sizeof (fn));
    mkdir (fn, 0777);
    g_strlcat (fn, "/battery", sizeof (fn));
    mkdir (fn, 0777);

    if (mydebug > 0)
      printf ("	-------> one Battery\n");
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1", dir_proc);
    mkdir (fn, 0777);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/state", dir_proc);
    write_file (fn, "present:                 yes\n"
		"capacity state:          ok\n"
		"charging state:          charging\n"
		"present rate:            5000 mW\n"
		"remaining capacity:      20000 mWh\n"
		"present voltage:         16764 mV\n");

    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/info", dir_proc);
    write_file (fn, "present:                 yes\n"
		"design capacity:         59200 mWh\n"
		"last full capacity:      40000 mWh\n"
		"battery technology:      non-rechargeable\n"
		"design voltage:          14800 mV\n"
		"design capacity warning: 0 mWh\n"
		"design capacity low:     120 mWh\n"
		"capacity granularity 1:  0 mWh\n"
		"capacity granularity 2:  10 mWh\n"
		"model number:            \n");
    if (!battery_get_values ())
      {
	printf ("battery_get_values() reporting Problem: no battery status for 1 Bat\n");
	errors++;
      }
    if (mydebug > 1)
      printf ("batstring: %s\n", batstring);
    if (strcmp (batstring, "Batt 50%, 240 min"))
      {
	printf ("!!!! ERROR\n");
	errors++;
      }


    if (mydebug > 0)
      printf ("	-------> and another Battery\n");
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2", dir_proc);
    mkdir (fn, 0777);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/state", dir_proc);
    write_file (fn, "present:                 yes\n"
		"capacity state:          ok\n"
		"charging state:          charging\n"
		"present rate:            500 mW\n"
		"remaining capacity:      1000 mWh\n"
		"present voltage:         16764 mV\n");

    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/info", dir_proc);
    write_file (fn, "present:                 yes\n"
		"design capacity:         59200 mWh\n"
		"last full capacity:      10000 mWh\n"
		"battery technology:      non-rechargeable\n"
		"design voltage:          14800 mV\n"
		"design capacity warning: 0 mWh\n"
		"design capacity low:     120 mWh\n"
		"capacity granularity 1:  0 mWh\n"
		"capacity granularity 2:  10 mWh\n"
		"model number:            \n");
    response = battery_get_values ();
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/state", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/info", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/state", dir_proc);
    unlink (fn);
    g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/info", dir_proc);
    unlink (fn);
    if (!response)
      {
	printf ("battery reporting Problem: no battery status for 1 Bat\n");
	errors++;
      }
    if (mydebug > 1)
      printf ("batstring: %s\n", batstring);
    if (strcmp (batstring, "Batt 42%, 229 min"))
      {
	printf ("!!!! ERROR\n");
	errors++;
      }

    if (mydebug > 0)
      printf ("	-------> Check with apm\n");
    g_snprintf (fn, sizeof (fn), "%s/apm", dir_proc);
    //      write_file(fn,"test test test 0x03 test 0x01 99% test test\n");

    gchar battery_string[200];
    sprintf (battery_string, "%s %s %s %x %s %x %d%% %s %s\ntest1\n",
	     "test1", "test2", "test3", 3, "test4", 3, 99, "test5", "test6");
    write_file (fn, battery_string);
    write_file (fn, "1 2 3 04 4 06 71% 7 8\n");
    response = battery_get_values ();
    if (mydebug > 1)
      printf ("batstring: %s\n", batstring);
    unlink (fn);
    if (!response)
      {
	printf ("battery reporting Problem: no bat reported\n");
	errors++;
      }
    if (!strcmp (batstring, "Batt 71%"))
      {
	printf ("!!!! ERROR\n");
	errors++;
      }




    if (mydebug > 0)
      printf ("	-------> Check acpi thermal zone\n");
    g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone", dir_proc);
    mkdir (fn, 0777);
    g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone/ATF00", dir_proc);
    mkdir (fn, 0777);
    g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone/ATF00/temperature",
		dir_proc);
    write_file (fn, "temperature:             59 C\n");

    response = temperature_get_values ();
    if (mydebug > 1)
      printf ("tempstring: %s\n", cputempstring);
    //      unlink(fn);
    if (!response)
      {
	printf ("ERROR:	temperature reporting Problem: no temp reported\n");
	errors++;
      }

  }


  // ------------------------------------------------------------------
  //errors += unit_test_nmea();

  set_unittest_timer();
  if (errors > 0)
    {
      printf ("\n\n");
      printf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      printf ("!!!                                                   !!!\n");
      printf ("!!! FOUND %2d ERRORS                                   !!!\n",
	      errors);
      printf ("!!!                                                   !!!\n");
      printf
	("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
      exit (-1);
    }

  // ------------------------------------------------------------------
  printf ("\n\nUnit Tests ( Part 1 )  successfull\n\n");
}




/* *****************************************************************************
 */
gint
unittest_settings_open (GtkWidget * widget, guint * datum)
{
    
    if (mydebug > 0)
	    printf ("	-------> Open Settings\n");

    settings_main_cb(0,1);

    return FALSE; // Only once
}
gint
unittest_settings_close (GtkWidget * widget, guint * datum)
{
    
    if (mydebug > 0)
	    printf ("	-------> Close Settings\n");
    
    return FALSE; // Only once
}

/* *****************************************************************************
 */
void
set_unittest_timer (void)
{
    printf ("Testing Open and Close Settings\n");
    gtk_timeout_add (1000, (GtkFunction) unittest_settings_open,NULL ); 
    gtk_timeout_add (2000, (GtkFunction) quit_program_cb,NULL);
}
