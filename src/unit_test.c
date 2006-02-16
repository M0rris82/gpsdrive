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
Revision 1.5  2006/02/16 09:52:44  tweety
rearrange acpi handling and displaying of battery and temperature display

Revision 1.4  2006/02/10 22:33:26  tweety
fix more in the ACPI/APM handling. write unti tests for this part of Code

Revision 1.3  2006/02/10 17:36:04  tweety
rearrange ACPI handling

Revision 1.2  2006/01/04 19:19:31  tweety
more unit tests
search for icons in the local directory data/icons and data/pixmaps first

Revision 1.1  2006/01/04 16:32:30  tweety
Add the first unit test

Revision 1.0  2006/01/03 14:24:10  tweety

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

extern gint zoom;
extern gdouble current_lon, current_lat;
extern gint mapistopo;
extern glong mapscale;
extern gdouble pixelfact;
extern int usesql;
extern gchar dir_proc[200];
extern gchar cputempstring[20], batstring[20];

/* ******************************************************************
 * This Function tests internal routines of gpsdrive
 */

/* ******************************************************************
 * write a file with content in one function call
 */
void write_file(gchar *filename, gchar *content)
{
    FILE *fp;
    fp = fopen (filename, "w");
    if (fp == NULL)
	{
	    fprintf (stderr,"Error opening %s\n",filename);
	    perror("ERROR");
	    exit(-1);
	}

    fprintf (fp, "%s",content);
    fclose ( fp );
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


  // ------------------------------------------------------------------
  {
    gchar test_string1[100];
    gchar test_string2[100];
    gdouble coordinate;
    gdouble delta;
    printf ("Testing coordinate_string2gdouble()\n");

    coordinate = -9999;
    g_strlcpy (test_string1, "0", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (0 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "0.0", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (0 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "1.0", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (1 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "121.7654789", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (121.7654789 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "121.7654789", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (121.7654789 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "  121,7654789   ", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (121.7654789 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'E",
	       sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (12.35 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'W",
	       sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (-12.35 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'N",
	       sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (12.35 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'S",
	       sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (-12.35 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'7''E",
	       sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    delta = coordinate - 12.35194444444444;
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (delta > 1e-10)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "12.0", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (12 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "-12.0", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (-12 != coordinate)
      exit (-1);

    coordinate = -9999;
    g_strlcpy (test_string1, "12", sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	    coordinate);
    if (12 != coordinate)
      exit (-1);

    // Will the input string be modified ?
    coordinate = -9999;
    g_strlcpy (test_string1, "  121,7654789   ", sizeof (test_string1));
    g_strlcpy (test_string2, test_string1, sizeof (test_string1));
    coordinate_string2gdouble (test_string1, &coordinate);
    printf ("	coordinate_string2gdouble('%s') --> '%s',%g\n", test_string1,
	    test_string2, coordinate);
    if (strcmp (test_string1, test_string2))
      exit (-1);
    if (121.7654789 != coordinate)
      exit (-1);
  }

  // ------------------------------------------------------------------
  {
    gchar test_string1[100];
    gdouble coordinate;

    printf ("Testing coordinate2gchar()\n");
    // coordinate2gchar (gchar * buff, gint buff_size, gdouble pos, gint islat, gint mode)

    coordinate = 12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, FALSE,
		      1);
    printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("12" "\xc2" "\xb0" "05'60.00''E", test_string1))
      exit (-1);

    coordinate = -12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, FALSE,
		      1);
    printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("12" "\xc2" "\xb0" "05'60.00''W", test_string1))
      exit (-1);

    coordinate = 12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, TRUE,
		      1);
    printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("12" "\xc2" "\xb0" "05'60.00''N", test_string1))
      exit (-1);

    coordinate = -12.1;
    coordinate2gchar (test_string1, sizeof (test_string1), coordinate, TRUE,
		      1);
    printf ("	%g --> '%s'\n", coordinate, test_string1);
    if (strcmp ("12" "\xc2" "\xb0" "05'60.00''S", test_string1))
      exit (-1);
  }

  // ------------------------------------------------------------------
  {
    gdouble lat, lon;
    gint x, y;
    gdouble gx, gy;
    printf ("Testing calcxytopos() and calcxy()\n");

    // void calcxytopos (int posx, int posy, gdouble *mylat, gdouble *mylon, gint zoom);
    // void calcxy (gdouble *posx, gdouble *posy, gdouble lon, gdouble lat, gint zoom);

    zoom = 1;
    current_lon = -48;
    current_lat = -12;
    mapistopo = FALSE;
    // mapscale =3000000000;
    // pixelfact = 1;

    printf ("pixelfact: %g\n", pixelfact);
    printf ("mapscale: %ld\n", mapscale);

    x = 0;
    y = 0;
    calcxytopos (x, y, &lat, &lon, zoom);
    printf ("calcxytopos(%d,%d)	--> (%g,%g)\n", x, y, lat, lon);
    calcxy (&gx, &gy, lon, lat, zoom);
    printf ("calcxy(%g,%g)	--> (%.1f,%.1f)\n", lat, lon, gx, gy);
    if (abs (gx - x) >= 1 || abs (gy - y) >= 1)
      exit (-1);

    x = SCREEN_X;
    y = 0;
    calcxytopos (x, y, &lat, &lon, zoom);
    printf ("calcxytopos(%d,%d)	--> (%g,%g)\n", x, y, lat, lon);
    calcxy (&gx, &gy, lon, lat, zoom);
    printf ("calcxy(%g,%g)	--> (%.1f,%.1f)\n", lat, lon, gx, gy);
    if (abs (gx - x) >= 1 || abs (gy - y) >= 1)
      exit (-1);

    x = 0;
    y = SCREEN_Y;
    calcxytopos (x, y, &lat, &lon, zoom);
    printf ("calcxytopos(%d,%d)	--> (%g,%g)\n", x, y, lat, lon);
    calcxy (&gx, &gy, lon, lat, zoom);
    printf ("calcxy(%g,%g)	--> (%.1f,%.1f)\n", lat, lon, gx, gy);
    if (abs (gx - x) >= 1 || abs (gy - y) >= 1)
      exit (-1);

    x = SCREEN_X;
    y = SCREEN_Y;
    calcxytopos (x, y, &lat, &lon, zoom);
    printf ("calcxytopos(%d,%d)	--> (%g,%g)\n", x, y, lat, lon);
    calcxy (&gx, &gy, lon, lat, zoom);
    printf ("calcxy(%g,%g)	--> (%.1f,%.1f)\n", lat, lon, gx, gy);
    if (abs (gx - x) >= 1 || abs (gy - y) >= 1)
      exit (-1);
  }

  // ------------------------------------------------------------------
  {
      printf("Testing if SQL Support is on\n");
      if ( ! usesql )
	  {
	      printf("Problem with SQL Support\n");
	      exit(-1);
	  }
  }

  // ------------------------------------------------------------------
  {
      gchar fn[500];
      gint response;

      printf("Testing if acpi/apm is parsing correct\n");

      g_snprintf( dir_proc, sizeof(dir_proc) , "/tmp/gpsdrive-unit-test");
      mkdir (dir_proc, 0777);
      g_snprintf( dir_proc, sizeof(dir_proc) , "/tmp/gpsdrive-unit-test/proc");
      mkdir (dir_proc, 0777);

      printf("	--------> remove maybe old Files\n");
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/state",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/state",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/info",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/info",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/apm",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone/ATF00/temperature",dir_proc);
      unlink(fn);


      printf("	-------> Check if we get positive answers even if no files should be there\n");
      if ( battery_get_values () ) {
	  printf("battery reporting Problem: battery status without file\n");
	  exit(-1);
      }

      if ( temperature_get_values () ) {
	  printf("temperature reporting Problemtem: temperature without file\n");
	  exit(-1);
      }

      printf("	-------> Create Dummy Files/dirs for ACPI\n");
      g_snprintf (fn, sizeof (fn), dir_proc);
      g_strlcat (fn, "/acpi", sizeof (fn));
      mkdir (fn, 0777);
      g_strlcat (fn, "/battery", sizeof (fn));
      mkdir (fn, 0777);

      printf("	-------> one Battery\n");
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1",dir_proc);
      mkdir (fn, 0777);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/state",dir_proc);
      write_file(fn,"present:                 yes\n"
		 "capacity state:          ok\n"
		 "charging state:          charging\n"
		 "present rate:            5000 mW\n"
		 "remaining capacity:      20000 mWh\n"
		 "present voltage:         16764 mV\n");
      
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/info",dir_proc);
      write_file(fn,"present:                 yes\n"
		 "design capacity:         59200 mWh\n"
		 "last full capacity:      40000 mWh\n"
		 "battery technology:      non-rechargeable\n"
		 "design voltage:          14800 mV\n"
		 "design capacity warning: 0 mWh\n"
		 "design capacity low:     120 mWh\n"
		 "capacity granularity 1:  0 mWh\n"
		 "capacity granularity 2:  10 mWh\n"
		 "model number:            \n");
      if ( ! battery_get_values () ) {
	  printf("battery reporting Problem: no battery status for 1 Bat\n");
	  exit(-1);
      }
      printf("batstring: %s\n",batstring);
      if ( strcmp (batstring, "Batt 50%, 240 min")) 
	  exit (-1);


      printf("	-------> and another Battery\n");
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2",dir_proc);
      mkdir (fn, 0777);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/state",dir_proc);
      write_file(fn,"present:                 yes\n"
		 "capacity state:          ok\n"
		 "charging state:          charging\n"
		 "present rate:            500 mW\n"
		 "remaining capacity:      1000 mWh\n"
		 "present voltage:         16764 mV\n");
      
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/info",dir_proc);
      write_file(fn,"present:                 yes\n"
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
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/state",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT1/info",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/state",dir_proc);
      unlink(fn);
      g_snprintf (fn, sizeof (fn), "%s/acpi/battery/BAT2/info",dir_proc);
      unlink(fn);
      if ( ! response  ) {
	  printf("battery reporting Problem: no battery status for 1 Bat\n");
	  exit(-1);
      }
      printf("batstring: %s\n",batstring);
      if ( strcmp (batstring, "Batt 42%, 229 min")) 
	  exit (-1);

      printf("	-------> Check with apm\n");
      g_snprintf (fn, sizeof (fn), "%s/apm", dir_proc);
      //      write_file(fn,"test test test 0x03 test 0x01 99% test test\n");

      gchar battery_string[200];
      sprintf (battery_string, "%s %s %s %x %s %x %d%% %s %s\ntest1\n",
	       "test1","test2","test3",3,"test4",3,99,"test5","test6");
      write_file(fn,battery_string);
      write_file(fn,"1 2 3 04 4 06 71% 7 8\n");
      response = battery_get_values ();
      printf("batstring: %s\n",batstring);
      unlink(fn);
      if ( ! response  ) {
	  printf("battery reporting Problem: no bat reported\n");
	  exit(-1);
      }
      if ( !strcmp (batstring, "Batt 71%")) 
	  exit (-1);




      printf("	-------> Check acpi thermal zone\n");
      g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone",dir_proc);
      mkdir (fn, 0777);
      g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone/ATF00",dir_proc);
      mkdir (fn, 0777);
      g_snprintf (fn, sizeof (fn), "%s/acpi/thermal_zone/ATF00/temperature",dir_proc);
      write_file(fn,"temperature:             59 C\n");

      response = temperature_get_values ();
      printf("tempstring: %s\n",cputempstring);
      //      unlink(fn);
      if ( ! response  ) {
	  printf("temperature reporting Problem: no temp reported\n");
	  exit(-1);
      }

  }

  // ------------------------------------------------------------------
  printf ("All Tests successfull\n");
  exit (0);
}
