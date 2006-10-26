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
extern char wp_typelist[100][40];
extern int  wp_typelistcount;
extern double dbdistance;
extern int usesql;
extern int mydebug, dbusedist;
extern gchar local_config_homedir[500];
extern GtkWidget *trackbt, *wpbt;
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
      g_strlcpy (wp_typelist[r++], temp, sizeof (wp_typelist[0]));
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

  wp_typelistcount = r;
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
  gchar file_path[400];
  FILE *st;
  double lat, lon, l, ti;
  struct timeval t;

  if (!usesql)
    return FALSE;
  gettimeofday (&t, NULL);
  ti = t.tv_sec + t.tv_usec / 1000000.0;
  g_strlcpy (file_path, local_config_homedir, sizeof (file_path));
  g_strlcat (file_path, "way-SQLRESULT.txt", sizeof (file_path));
  st = fopen (file_path, "w+");
  if (st == NULL)
    {
      perror (file_path);
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
	handle = dlopen ("libmysqlclient.so.17", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so.16", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so.15", RTLD_LAZY);
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
