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
Revision 1.20  2006/02/08 08:34:04  tweety
e sql was hardecoded off

Revision 1.19  2006/02/05 16:38:05  tweety
reading floats with scanf looks at the locale LANG=
so if you have a locale de_DE set reading way.txt results in clearing the
digits after the '.'
For now I set the LC_NUMERIC always to en_US, since there we have . defined for numbers

Revision 1.18  2006/01/03 14:24:10  tweety
eliminate compiler Warnings
try to change all occurences of longi -->lon, lati-->lat, ...i
use  drawicon(posxdest,posydest,"w-lan.open") instead of using a seperate variable
rename drawgrid --> do_draw_grid
give the display frames usefull names frame_lat, ...
change handling of WP-types to lowercase
change order for directories reading icons
always read inconfile

Revision 1.17  1994/06/08 13:02:31  tweety
adjust debug levels

Revision 1.16  1994/06/08 08:37:47  tweety
fix some ocurences of +- handling with coordinates by using coordinate_string2gdouble
instead of atof and strtod

Revision 1.15  2005/07/04 04:47:03  tweety
http://bugzilla.gpsdrive.cc/show_bug.cgi?id=25
"J.D. Schmidt" <jdsmobile@gmail.com>
Fixed Wrong SQL Escapes

Revision 1.14  2005/05/24 08:35:25  tweety
move track splitting to its own function +sub track_add($)
a little bit more error handling
earth_distance somtimes had complex inumbers as result
implemented streets_check_if_moved_reset which is called when you toggle the draw streets button
this way i can re-read all currently displayed streets from the DB
fix minor array iindex counting bugs
add some content to the comment column

Revision 1.13  2005/04/20 23:33:49  tweety
reformatted source code with anjuta
So now we have new indentations

Revision 1.12  2005/04/13 19:58:31  tweety
renew indentation to 4 spaces + tabstop=8

Revision 1.11  2005/04/10 21:50:50  tweety
reformatting c-sources

Revision 1.10  2005/03/29 01:59:01  tweety
another set of minor Bugfixes

Revision 1.9  2005/03/27 00:44:42  tweety
eperated poi_type_list and streets_type_list
and therefor renaming the fields
added drop index before adding one
poi.*: a little bit more error handling
disabling poi and streets if sql is disabled
changed som print statements from \n.... to ...\n
changed some debug statements from debug to mydebug

Revision 1.8  2005/02/08 09:01:48  tweety
move loading of usericons to icons.c

Revision 1.7  2005/02/08 08:43:46  tweety
wrong dfinition for auxicons array

Revision 1.6  2005/02/08 07:58:22  tweety
load icons from system directory of .../gpsdrive/icons if exists too

Revision 1.5  2005/02/07 07:53:39  tweety
added check_if_moved inti function poi_rebuild_list

Revision 1.4  2005/02/06 21:18:05  tweety
more cleanup: extracted more functionality to functions

Revision 1.3  2005/01/20 00:12:14  tweety
don't abort gpsdrive completely on sql query errors

Revision 1.2  2005/01/11 07:13:27  tweety
added "order by \(abs(%.6f - lat)+abs(%.6f - lon)),name LIMIT 10000"
to get the nearest wp first and to have a limit on hov many are retrieved

Revision 1.1.1.1  2004/12/23 16:03:24  commiter
Initial import, straight from 2.10pre2 tar.gz archive

Revision 1.33  2004/02/08 20:37:49  ganter
handle user-defined icons for open and closed wlans
the filename should be:
for open wlan:  wlan.png
for crypted wlan: wlan-wep.png

Revision 1.32  2004/02/08 17:16:25  ganter
replacing all strcat with g_strlcat to avoid buffer overflows

Revision 1.31  2004/02/08 16:35:10  ganter
replacing all sprintf with g_snprintf to avoid buffer overflows

Revision 1.30  2004/02/07 00:02:16  ganter
added "store timezone" button in settings menu

Revision 1.29  2004/02/06 15:13:15  ganter
...

Revision 1.28  2004/02/06 15:11:21  ganter
updated translation

Revision 1.27  2004/02/06 14:55:54  ganter
added support for user-defined icons
create the directory: $HOME/.gpsdrive/icons
place your icons (type must be png) into this directory, with the name of
the waypoint type, filename must be lowercase
i.e. for waypoint type "HOTEL" the file must have the name "hotel.png"

Revision 1.26  2004/02/02 03:38:32  ganter
code cleanup

Revision 1.25  2004/01/05 05:52:58  ganter
changed all frames to respect setting

Revision 1.24  2004/01/01 09:07:33  ganter
v2.06
trip info is now live updated
added cpu temperature display for acpi
added tooltips for battery and temperature

Revision 1.23  2003/05/07 19:27:13  ganter
replaced degree symbol with unicode string
gpsdrive should now be unicode clean

Revision 1.22  2003/05/03 18:59:47  ganter
shortcuts are now working

Revision 1.21  2003/05/02 18:27:18  ganter
porting to GTK+-2.2
GpsDrive Version 2.0pre3

Revision 1.20  2003/01/15 17:03:17  ganter
MySQL is now loaded dynamically on runtime, no mysql needed for compile.
Needs only libmysqlclient.so now.

Revision 1.19  2003/01/15 15:30:28  ganter
before dynamically loading mysql

Revision 1.18  2002/12/08 03:18:26  ganter
shortly before 1.31

Revision 1.17  2002/11/27 00:02:27  ganter
1.31pre2

Revision 1.16  2002/11/24 16:56:30  ganter
speedtrap works now with sql

Revision 1.15  2002/11/24 16:01:32  ganter
added speedtrap icon, thanks to Sven Fichtner

Revision 1.14  2002/11/13 18:20:42  ganter
fixed buffer overflow in gpssql.c

Revision 1.13  2002/11/13 17:31:57  ganter
added display of number of waypoints

Revision 1.12  2002/11/09 00:09:57  ganter
bugfix in gpssql.c

Revision 1.11  2002/11/08 23:35:20  ganter
v1.30pre3

Revision 1.10  2002/11/08 22:08:11  ganter
...

Revision 1.9  2002/11/06 05:29:15  ganter
fixed most warnings

Revision 1.8  2002/11/06 01:44:15  ganter
v1.30pre2

Revision 1.7  2002/11/02 12:38:55  ganter
changed website to www.gpsdrive.de

Revision 1.6  2002/10/27 10:51:30  ganter
1.28pre8

Revision 1.5  2002/10/24 08:44:09  ganter
...

Revision 1.4  2002/10/17 15:55:45  ganter
wp2sql added

Revision 1.3  2002/10/16 14:16:13  ganter
working on SQL gui

Revision 1.2  2002/10/15 07:44:11  ganter
...

Revision 1.1  2002/10/14 08:38:59  ganter
v1.29pre3
added SQL support

*/
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <gpsdrive.h>
#include <ctype.h>
#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>

#include <icons.h>

#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char dbtable[MAXDBNAME], dbname[MAXDBNAME];
extern gdouble current_lon, current_lat;
extern char dbwherestring[5000];
extern char dbtypelist[100][40];
extern double dbdistance;
extern int dbtypelistcount;
extern int usesql;
extern int mydebug, dbusedist;
extern gchar homedir[500], mapdir[500];
extern GtkWidget *trackbt, *wpbt;
extern gint maxauxicons, lastauxicon;
extern auxiconsstruct auxicons[];
extern GdkPixbuf *friendsimage, *friendspixbuf;

gint wptotal = 0, wpselected = 0;


/*  Defines for gettext I18n */
# include <libintl.h>
# define _(String) gettext(String)
# ifdef gettext_noop
#  define N_(String) gettext_noop(String)
# else
#  define N_(String) (String)
# endif


/*****************************************************************
change this functions if you want to implement another database
******************************************************************/


#include "mysql.h"

MYSQL mysql;
MYSQL_RES *res;
MYSQL_ROW row;
//res = NULL; // Set res as default NULL

/* ******************************************************************
 */
void
exiterr (int exitcode)
{

  fprintf (stderr, "Error while using mysql\n");
  fprintf (stderr, "%s\n", dl_mysql_error (&mysql));
  exit (exitcode);
}

/* ******************************************************************
 */
int
sqlinit (void)
{
  if (!usesql)
    return 0;

  if (!(dl_mysql_init (&mysql)))
    exiterr (1);

  if (!
      (dl_mysql_real_connect
       (&mysql, dbhost, dbuser, dbpass, dbname, 0, NULL, 0)))
    {
      fprintf (stderr, "%s\n", dl_mysql_error (&mysql));
      return FALSE;
    }
  /*   if ( mydebug > 50 ) */
  printf (_("SQL: connected to %s as %s using %s\n"), dbhost, dbuser, dbname);
  return TRUE;
}

/* ******************************************************************
 */
void
sqlend (void)
{
  if (!usesql)
    return;
  dl_mysql_close (&mysql);
}


/* ******************************************************************
 */
int
insertsqldata (double lat, double lon, char *name, char *typ)
{
  char q[200], lats[20], lons[20], tname[500], ttyp[50];
  int r, j, i;

  if (!usesql)
    return 0;
  g_snprintf (lats, sizeof (lats), "%.6f", lat);
  g_strdelimit (lats, ",", '.');
  g_snprintf (lons, sizeof (lons), "%.6f", lon);
  g_strdelimit (lons, ",", '.');
  g_strdelimit (name, " ", '_');
  g_strdelimit (typ, " ", '_');

  /* escape ' */
  j = 0;
  for (i = 0; i <= (int) strlen (name); i++)
    {
      if (name[i] != '\'' && name[i] != '\\' && name[i] != '\"')
	tname[j++] = name[i];
      else
	{
	  tname[j++] = '\\';
	  tname[j++] = name[i];
	  if (mydebug > 50)
	    g_print ("Orig. name : %s\nEscaped name : %s\n", name, tname);
	}
    }

  j = 0;
  for (i = 0; i <= (int) strlen (typ); i++)
    {
      if (typ[i] != '\'' && typ[i] != '\\' && typ[i] != '\"')
	ttyp[j++] = typ[i];
      else
	{
	  ttyp[j++] = '\\';
	  ttyp[j++] = typ[i];
	  if (mydebug > 50)
	    g_print ("\n Orig. typ : %s\nEscaped typ : %s\n", typ, ttyp);
	}
    }

  g_snprintf (q, sizeof (q),
	      "INSERT INTO %s (name,lat,lon,type) VALUES ('%s','%s','%s','%s')",
	      dbtable, tname, lats, lons, ttyp);
  if (mydebug > 50)
    printf ("query: %s\n", q);
  if (dl_mysql_query (&mysql, q))
    exiterr (3);
  r = dl_mysql_affected_rows (&mysql);
  if (mydebug > 50)
    printf (_("rows inserted: %d\n"), r);

  g_snprintf (q, sizeof (q), "SELECT LAST_INSERT_ID()");
  if (mydebug > 50)
    printf ("insertsqldata: query: %s\n", q);
  if (dl_mysql_query (&mysql, q))
    exiterr (3);
  if (!(res = dl_mysql_store_result (&mysql)))
    {
      dl_mysql_free_result (res);
      res = NULL;
      fprintf (stderr, "insert_sql_data: Error in store results: %s\n",
	       dl_mysql_error (&mysql));
      return -1;
    }
  r = 0;
  while ((row = dl_mysql_fetch_row (res)))
    {
      r = strtol (row[0], NULL, 10);	/* last index */
    }

  if (mydebug > 50)
    printf (_("last index: %d\n"), r);
  return r;
}


/* ******************************************************************
 */
int
deletesqldata (int index)
{
  char q[200];
  int r;

  if (!usesql)
    return 0;
  g_snprintf (q, sizeof (q), "DELETE FROM %s  WHERE id='%d'", dbtable, index);
  if (mydebug > 50)
    g_print ("query: %s\n", q);

  if (dl_mysql_query (&mysql, q))
    exiterr (3);
  r = dl_mysql_affected_rows (&mysql);
  if (mydebug > 50)
    g_print (_("rows deleted: %d\n"), r);
  return 0;
}

/* ******************************************************************
 */
int
get_sql_type_list (void)
{
  char q[200], temp[200];
  int r, i;
  static int usericonsloaded = FALSE;

  if (!usesql)
    return FALSE;


  /* make list of possible type entries */
  g_snprintf (q, sizeof (q), "SELECT DISTINCT upper(type) FROM %s", dbtable);
  if (mydebug > 50)
    printf ("get_sql_type_list: query: %s\n", q);

  if (dl_mysql_query (&mysql, q))
    exiterr (3);
  if (!(res = dl_mysql_store_result (&mysql)))
    {
      dl_mysql_free_result (res);
      res = NULL;
      fprintf (stderr, "get_sql_type_list: Error in store results: %s\n",
	       dl_mysql_error (&mysql));
      return -1;
    }
  r = 0;
  while ((row = dl_mysql_fetch_row (res)))
    {
      g_strlcpy (temp, row[0], sizeof (temp));
      for (i = 0; i < (int) strlen (temp); i++)
	temp[i] = tolower (temp[i]);
      g_strlcpy (dbtypelist[r++], temp, sizeof (dbtypelist[0]));
      if (r >= MAXWPTYPES)
	{
	  printf ("\nSQL: too many waypoint types!\n");
	  break;
	}
      /* load user defined icons */
      if (FALSE == usericonsloaded)
	load_user_icon (temp);
    }

  dl_mysql_free_result (res);
  res = NULL;

  dbtypelistcount = r;
  usericonsloaded = TRUE;

  if (mydebug > 50)
    printf ("%d External Icons loaded\n", r);
  return r;
}

/* ******************************************************************
 */
int
getsqldata ()
{
  char q[5000];
  char sql_order[5000];
  int r, rges, wlan, action, sqlnr;
  gchar mappath[400];
  FILE *st;
  double lat, lon, l, ti;
  struct timeval t;

  if (!usesql)
    return FALSE;
  gettimeofday (&t, NULL);
  ti = t.tv_sec + t.tv_usec / 1000000.0;
  g_strlcpy (mappath, homedir, sizeof (mappath));
  g_strlcat (mappath, "way-SQLRESULT.txt", sizeof (mappath));
  st = fopen (mappath, "w+");
  if (st == NULL)
    {
      perror (mappath);
      return 1;
    }

  g_snprintf (sql_order, sizeof (sql_order),
	      "order by \(abs(%.6f - lat)+abs(%.6f - lon))",
	      current_lat, current_lon);
  g_strdelimit (sql_order, ",", '.');
  if (mydebug > 50)
    printf ("mysql order: %s\n", sql_order);

  g_snprintf (q, sizeof (q),
	      "SELECT name,lat,lon,upper(type),id FROM %s %s %s,name LIMIT 10000",
	      dbtable, dbwherestring, sql_order);
  if (mydebug > 50)
    printf ("waypoints: mysql query: %s\n", q);

  if (dl_mysql_query (&mysql, q))
    {
      fprintf (stderr, "get_sql_type_list: Error in query: %s\n",
	       dl_mysql_error (&mysql));
      return (1);
    }

  if (!(res = dl_mysql_store_result (&mysql)))
    {
      fprintf (stderr, "Error in store results: %s\n",
	       dl_mysql_error (&mysql));
      dl_mysql_free_result (res);
      res = NULL;
      return (1);
    }

  rges = r = wlan = 0;
  while ((row = dl_mysql_fetch_row (res)))
    {
      rges++;
      /*  wlan=0: no wlan, 1:open wlan, 2:WEP crypted wlan */
      if ((strcmp (row[3], "WLAN")) == 0)
	wlan = 1;
      else if ((strcmp (row[3], "WLAN-WEP")) == 0)
	wlan = 2;
      else
	wlan = 0;
      action = 0;
      if ((strcmp (row[3], "SPEEDTRAP")) == 0)
	action = 1;
      sqlnr = atol (row[4]);
      if (dbusedist)
	{
	  coordinate_string2gdouble (row[1], &lat);
	  coordinate_string2gdouble (row[2], &lon);
	  l = calcdist (lon, lat);
	  if (l < dbdistance)
	    {
	      fprintf (st,
		       "%-22s %10s %11s %20s %d %d %d\n",
		       row[0], row[1], row[2], row[3], wlan, action, sqlnr);
	      r++;
	    }
	}
      else
	{
	  fprintf (st, "%-22s %10s %11s %20s %d %d %d\n",
		   row[0], row[1], row[2], row[3], wlan, action, sqlnr);
	  r++;
	}
    }
  fclose (st);

  gettimeofday (&t, NULL);
  ti = (t.tv_sec + t.tv_usec / 1000000.0) - ti;
  if (mydebug > 50)
    printf (_("%d(%d) rows read in %.2f seconds\n"), r, rges, ti);
  wptotal = rges;
  wpselected = r;
  loadwaypoints ();
  if (!dl_mysql_eof (res))
    {
      dl_mysql_free_result (res);
      res = NULL;
      return FALSE;
    }
  dl_mysql_free_result (res);
  res = NULL;
  return TRUE;
}

/* *****************************************************************************
 * Load mysql libraries and connect to function calls
 */
void
sql_load_lib ()
{
  void *handle;
  char *error;

  usesql = TRUE;
  // It seems like this doesn't work on cygwin unless the dlopen comes first..-jc
  if (usesql)
    {
      handle = dlopen ("/usr/local/lib/libmysqlclient.dll", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so.14", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so.12", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/opt/lib/mysql/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/opt/lib/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/opt/mysql/lib/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/opt/mysql/lib/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/sw/lib/libmysqlclient.dylib", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/libmysqlclient.so.12", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/mysql/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/mysql/libmysqlclient.so.12", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/local/lib/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/local/lib/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/local/lib/mysql/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen
	  ("/usr/local/lib/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/local/mysql/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/local/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen
	  ("@PREFIX@/lib/mysql/libmysqlclient.10.dylib", RTLD_LAZY);

      if (handle)
	{
	  // Clear previous errors
	  dlerror ();

	  dl_mysql_error = dlsym (handle, "mysql_error");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }

	  dl_mysql_init = dlsym (handle, "mysql_init");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_real_connect = dlsym (handle, "mysql_real_connect");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_close = dlsym (handle, "mysql_close");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_query = dlsym (handle, "mysql_query");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_affected_rows = dlsym (handle, "mysql_affected_rows");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_store_result = dlsym (handle, "mysql_store_result");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_fetch_row = dlsym (handle, "mysql_fetch_row");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_free_result = dlsym (handle, "mysql_free_result");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }


	  dl_mysql_eof = dlsym (handle, "mysql_eof");
	  if ((error = dlerror ()) != NULL)
	    {
	      fprintf (stderr, "%s\n", error);
	      usesql = FALSE;
	    }
	}
      else if ((error = dlerror ()) != NULL)
	{
	  fprintf (stderr, _("\nlibmysqlclient.so not found.\n"));
	  usesql = FALSE;
	}
      /*       dlclose(handle); */
    }
  if (!usesql)
    fprintf (stderr, _("\nMySQL support disabled.\n"));

}
