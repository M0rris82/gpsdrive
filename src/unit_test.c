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

/* ******************************************************************
 * This Function tests internal routines of gpsdrive
 */

void
unit_test (void)
{
  gchar test_string1[100];
  gchar test_string2[100];
  gdouble coordinate;

  // ------------------------------------------------------------------
  printf ("Testing coordinate_string2gdouble\n");

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
  g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'E", sizeof (test_string1));
  coordinate_string2gdouble (test_string1, &coordinate);
  printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	  coordinate);
  if (12.35 != coordinate)
    exit (-1);

  coordinate = -9999;
  g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'W", sizeof (test_string1));
  coordinate_string2gdouble (test_string1, &coordinate);
  printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	  coordinate);
  if (-12.35 != coordinate)
    exit (-1);

  coordinate = -9999;
  g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'N", sizeof (test_string1));
  coordinate_string2gdouble (test_string1, &coordinate);
  printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	  coordinate);
  if (12.35 != coordinate)
    exit (-1);

  coordinate = -9999;
  g_strlcpy (test_string1, "12" "\xc2" "\xb0" "21'S", sizeof (test_string1));
  coordinate_string2gdouble (test_string1, &coordinate);
  printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	  coordinate);
  if (-12.35 != coordinate)
    exit (-1);

  coordinate = -9999;
  g_strlcpy (test_string1, "12\xc2\xb021'7''E", sizeof (test_string1));
  coordinate_string2gdouble (test_string1, &coordinate);
  printf ("	coordinate_string2gdouble('%s') --> %g\n", test_string1,
	  coordinate);
  if (12 != coordinate)
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

  // ------------------------------------------------------------------
  printf ("Testing coordinate2gchar\n");
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
  coordinate2gchar (test_string1, sizeof (test_string1), coordinate, TRUE, 1);
  printf ("	%g --> '%s'\n", coordinate, test_string1);
  if (strcmp ("12" "\xc2" "\xb0" "05'60.00''N", test_string1))
    exit (-1);

  coordinate = -12.1;
  coordinate2gchar (test_string1, sizeof (test_string1), coordinate, TRUE, 1);
  printf ("	%g --> '%s'\n", coordinate, test_string1);
  if (strcmp ("12" "\xc2" "\xb0" "05'60.00''S", test_string1))
    exit (-1);

  // ------------------------------------------------------------------
  printf ("All Tests successfull\n");
  exit (0);
}
