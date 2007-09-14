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
#include <poi.h>

#define MAXDBNAME 30
extern char dbhost[MAXDBNAME], dbuser[MAXDBNAME], dbpass[MAXDBNAME];
extern char dbtable[MAXDBNAME], dbname[MAXDBNAME], poitypetable[MAXDBNAME];
extern double dbdistance;
extern int usesql;
extern int mydebug, dbusedist;
extern GtkWidget *trackbt, *wpbt;
extern GdkPixbuf *friendsimage, *friendspixbuf;
extern poi_type_struct poi_type_list[poi_type_list_max];

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


#include "mysql/mysql.h"

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
 * remove route and friendsd data at shutdown
 */
gint
cleanupsqldata ()
{
	gchar q[200];
	gint r, i;

	if (!usesql)
		return 0;

	for (i = 0; i < poi_type_list_max; i++)
	{
		if (
		  strncmp (poi_type_list[i].name,"waypoint.routepoint", 19) == 0 ||
		  strncmp (poi_type_list[i].name,"people.friendsd", 15) == 0)
		{
			g_snprintf (q, sizeof (q),
				"DELETE FROM %s WHERE poi_type_id=\"%d\";",
				dbtable, poi_type_list[i].poi_type_id);
			if (mydebug > 50)
				g_print ("query: %s\n", q);
			if (dl_mysql_query (&mysql, q))
				exiterr (3);
			r = dl_mysql_affected_rows (&mysql);
			if (mydebug > 50)
				g_print (_("rows deleted: %d\n"), r);
		}
	}

	return 0;
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
  cleanupsqldata ();
  dl_mysql_close (&mysql);
}


/* ******************************************************************
 * escape special characters in sql string.
 * returned string has to be freed after usage!
 */
gchar 
*escape_sql_string (const gchar *data)
{
	gint i, j, length;
	gchar *tdata = NULL;
	
	length = strlen (data);
	tdata = g_malloc (length*2 + 1);
	j = 0;
	for (i = 0; i <= length; i++)
	{
		if (data[i] == '\'' || data[i] == '\\' || data[i] == '\"')
			tdata[j++] = '\\';
		tdata[j++] = data[i];
	}
	
	if (mydebug > 50)
		g_print ("escape_sql_string:\tOrig. name : %s\nescape_sql_string:\tEscaped name : %s\n", data, tdata);
	
	return tdata;
}


/* ******************************************************************
 * insert poi data into poi table
 */
glong
insertsqldata (double lat, double lon, gchar *name, gchar *typ, gchar *comment, gint src)
{
	gchar q[200], lats[20], lons[20];
	gchar *tname, *ttyp, *tcomment;
	gint pt_id;
	glong r;

	if (!usesql)
		return 0;
	g_snprintf (lats, sizeof (lats), "%.6f", lat);
	g_strdelimit (lats, ",", '.');
	g_snprintf (lons, sizeof (lons), "%.6f", lon);
	g_strdelimit (lons, ",", '.');

	/* escape ' */
	tname = escape_sql_string (name);
	ttyp = escape_sql_string (typ);
	tcomment = escape_sql_string (comment);
	
	/* get poi_type_id for chosen poi_type from poi_type table */
	pt_id = poi_type_id_from_name (ttyp);
	
	/* set source_id at value for 'user entered data'if none is given */
	if (!src)
		src = 3;
	
	g_snprintf (q, sizeof (q),
				"INSERT INTO %s (name,lat,lon,poi_type_id,comment,source_id,last_modified) VALUES ('%s','%s','%s','%d','%s','%d',NOW())",
				dbtable, tname, lats, lons, pt_id, tcomment, src);
	if (mydebug > 50)
		printf ("query: %s\n", q);
	if (dl_mysql_query (&mysql, q))
		exiterr (3);
	r = dl_mysql_affected_rows (&mysql);
	if (mydebug > 50)
		printf (_("rows inserted: %ld\n"), r);

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
		printf (_("last index: %ld\n"), r);
	
	g_free (ttyp);
	g_free (tname);
	g_free (tcomment);
	
	return r;
}


/* ******************************************************************
 * update poi data in poi table
 */
glong
updatesqldata (glong poi_id, double lat, double lon, gchar *name, gchar *typ, gchar *comment, gint src)
{
	gchar q[200], lats[20], lons[20];
	gchar *tname, *ttyp, *tcomment;
	gint pt_id;
	glong r;

	if (!usesql)
		return 0;
	g_snprintf (lats, sizeof (lats), "%.6f", lat);
	g_strdelimit (lats, ",", '.');
	g_snprintf (lons, sizeof (lons), "%.6f", lon);
	g_strdelimit (lons, ",", '.');

	/* escape ' */
	tname = escape_sql_string (name);
	ttyp = escape_sql_string (typ);
	tcomment = escape_sql_string (comment);
	
	/* get poi_type_id for chosen poi_type from poi_type table */
	pt_id = poi_type_id_from_name (ttyp);
	
	/* set source_id at value for 'user entered data'if none is given */
	if (!src)
		src = 3;
	
	g_snprintf (q, sizeof (q),
				"UPDATE %s SET name='%s', lat='%s', lon='%s', poi_type_id='%d', comment='%s', source_id='%d', last_modified=NOW() WHERE poi_id=%ld", dbtable, tname, lats, lons, pt_id, tcomment, src, poi_id);
	if (mydebug > 50)
		printf ("update query: %s\n", q);
	if (dl_mysql_query (&mysql, q))
		exiterr (3);
	r = dl_mysql_affected_rows (&mysql);
	if (mydebug > 50)
		printf (_("rows updated: %ld\n"), r);

	return r;
}


/* ******************************************************************
 * insert additional poi data into poi_extra table
 */
int
insertsqlextradata (glong *poi_id, gchar *field_name, gchar *field_entry)
{
	char q[9000];
	gchar *tentry;
	int r;

	/* escape ' */
	tentry = escape_sql_string (field_entry);
	
	g_snprintf (q, sizeof (q),
		"INSERT INTO poi_extra (poi_id, field_name, entry) VALUES ('%ld','%s','%s')", *poi_id, field_name, tentry);
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
		fprintf (stderr, "insertsqlextradata: Error in store results: %s\n",
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
 * update additional poi data in poi_extra table
 */
int
updatesqlextradata (glong *poi_id, gchar *field_name, gchar *field_entry)
{
	char q[9000];
	gchar *tentry, *tfield;
	int r;

	/* escape ' */
	tfield = escape_sql_string (field_name);
	tentry = escape_sql_string (field_entry);
	
	g_snprintf (q, sizeof (q),
		"UPDATE poi_extra SET entry='%s' WHERE (poi_id=%ld AND field_name='%s')", tentry, *poi_id, field_name);
	if (mydebug > 50)
		printf ("update query: %s\n", q);
	if (dl_mysql_query (&mysql, q))
		exiterr (3);
	r = dl_mysql_affected_rows (&mysql);
	if (mydebug > 50)
		printf (_("rows updated: %d\n"), r);

	return r;	
}


/* ******************************************************************
 * get additional poi data from poi_extra table
 *
 * You have to set at least two parameters; set the one you are
 * asking for to NULL.
 * The return value is the poi_id.
 */
glong
getsqlextradata (glong *poi_id, gchar *field_name, gchar *field_entry, gchar *result)
{
	gchar sql_query[5000];
	glong result_id = 0;
	
	if (field_entry == NULL)
	{
		g_snprintf (sql_query, sizeof (sql_query),
		"SELECT poi_id,entry FROM poi_extra "
     		"WHERE (poi_id='%ld' AND field_name='%s') LIMIT 1;",
     		*poi_id, field_name);
     	}
	else if (poi_id == NULL)
	{
		g_snprintf (sql_query, sizeof (sql_query),
		"SELECT poi_id FROM poi_extra "
     		"WHERE (field_name='%s' AND entry='%s') LIMIT 1;",
     		field_name, field_entry);
     	}
	else if (field_name == NULL)
	{
		g_snprintf (sql_query, sizeof (sql_query),
		"SELECT poi_id,field_id FROM poi_extra "
     		"WHERE (poi_id='%ld' AND entry='%s') LIMIT 1;",
     		*poi_id, field_entry);
     	}
	else
		return 0;
		
	if (mydebug > 20)
		printf ("getsqlextradata: mysql query: %s\n", sql_query);

	if (dl_mysql_query (&mysql, sql_query))
	{
		fprintf (stderr, "getsqlextradata: Error in query: %s\n",
			dl_mysql_error (&mysql));
		return 0;
	}
	res = dl_mysql_store_result (&mysql);
	row = dl_mysql_fetch_row (res);
	if (!row)
		return 0;
	else
	{
		result_id = strtol (row[0], NULL, 10);
		if (row[1])
			result = g_strdup (row[1]);
	}
	dl_mysql_free_result (res);
	res = NULL;
	return result_id;
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
  g_snprintf (q, sizeof (q), "DELETE FROM %s  WHERE poi_id='%d'", dbtable, index);
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
getsqldata ()
{
   if (!usesql)
    return FALSE;
  
  poi_rebuild_list();
  
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
