
/*  load the waypoint from way.txt, if success we display the sel target window */
void
loadwaypoints ()
{
  gchar mappath[400];
  FILE *st;
  gint i, e, p, wlan, action, sqlnr;
  gchar buf[512], slat[80], slong[80], typ[40];
  struct stat buf2;

  if (waytxtstamp == 0)
    wayp = g_new (wpstruct, wpsize);

  g_strlcpy (mappath, homedir, sizeof (mappath));
  if (!sqlflag)
    g_strlcat (mappath, activewpfile, sizeof (mappath));
  else
    g_strlcat (mappath, "way-SQLRESULT.txt", sizeof (mappath));

  maxwp = 0;
  sqlnr = -1;
  st = fopen (mappath, "r");
  if (st == NULL)
    {
      perror (mappath);
    }
  else
    {
      if (debug)
	g_print ("\nsqlflag: %d, load waypoint file %s\n", sqlflag, mappath);

      i = 0;
      while ((p = fgets (buf, 512, st) != 0))
	{
	  e =
	    sscanf (buf, "%s %s %s %s %d %d %d\n", (wayp + i)->name, slat,
		    slong, typ, &wlan, &action, &sqlnr);
	  (wayp + i)->lat = g_strtod (slat, NULL);
	  (wayp + i)->longitude = g_strtod (slong, NULL);
/*  limit waypoint name to 20 chars */
	  (wayp + i)->name[20] = 0;
	  g_strlcpy ((wayp + i)->typ, "", 40);
	  (wayp + i)->wlan = 0;
	  (wayp + i)->action = 0;
	  (wayp + i)->sqlnr = -1;

	  if (e >= 3)
	    {
	      (wayp + i)->dist = 0;

	      if (e >= 4)
		g_strlcpy ((wayp + i)->typ, typ, 40);

	      if (e >= 5)
		(wayp + i)->wlan = wlan;
	      if (e >= 6)
		(wayp + i)->action = action;
	      if (e >= 7)
		(wayp + i)->sqlnr = sqlnr;


	      if (!sqlflag)
		{
		  if ((strncmp ((wayp + i)->name, "R-", 2)) == 0)
		    (wayp + i)->action = 1;
		}

	      i++;
	      maxwp = i;
/*
	      if (!sqlflag)
		{
		  g_snprintf (s, sizeof(s), "%s %d", _("Show _WP"), maxwp);
		  if (GTK_IS_BUTTON (GTK_BUTTON (wpbt)))
		    gtk_button_set_label (GTK_BUTTON (wpbt), s);
		  else
		    g_print ("kein button!\n");
		}
*/
	      if (maxwp >= wpsize)
		{
		  wpsize += 1000;
		  wayp = g_renew (wpstruct, wayp, wpsize);
		}

	    }
	}
      fclose (st);

/* Check for changed way.txt file */
      stat (mappath, &buf2);
      waytxtstamp = buf2.st_mtime;

    }

}



