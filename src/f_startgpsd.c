
void
startgpsd (guint datum)
{
  gchar s[200], s2[10];
  int t[] = { 2400, 4800, 9600, 19200, 38400 };

  if (gpson == FALSE)
    {
      g_snprintf (s2, sizeof (s2), "%d", t[serialspeed]);
      if (usedgps)
	g_snprintf (s, sizeof (s), "gpsd -p %s -c -d %s -s %s -r %s",
		    serialdev, dgpsserver, s2, dgpsport);
      else
	g_snprintf (s, sizeof (s), "gpsd -p %s -s %s", serialdev, s2);
      if (earthmate)
	g_strlcat (s, " -T e -s 9600", sizeof (s));
      system (s);
      gtk_button_set_label (GTK_BUTTON (startgpsbt), _("Stop GPSD"));
      gtk_tooltips_set_tip (GTK_TOOLTIPS (temptooltips), startgpsbt,
			    _("Stop GPSD and switch to simulation mode"),
			    NULL);
      g_strlcpy (gpsdservername, "127.0.0.1", sizeof (gpsdservername));
      if (sock != -1)
	{
	  close (sock);
	  sock = -1;
	}
      gtk_timeout_add (1000, (GtkFunction) initgps, 0);
      simmode = FALSE;
      gpson = TRUE;
      if (satsimage != NULL)
	g_object_unref (satsimage);
      satsimage = NULL;

      gtk_widget_draw (drawing_sats, NULL);	/* this  calls expose handler */
    }
  else
    {
/* stop gpsd and go to simulation mode */
      system ("killall gpsd");
      gpson = FALSE;
      gtk_button_set_label (GTK_BUTTON (startgpsbt), _("Start GPSD"));
      gtk_tooltips_set_tip (GTK_TOOLTIPS (temptooltips), startgpsbt,
			    _("Starts GPSD for NMEA mode"), NULL);
      simmode = TRUE;
      haveNMEA = FALSE;
      newsatslevel = TRUE;
      if (simpos_timeout == 0)
	simpos_timeout =
	  gtk_timeout_add (300, (GtkFunction) simulated_pos, 0);
      memset (satlist, 0, sizeof (satlist));
      memset (satlistdisp, 0, sizeof (satlist));
      numsats = satsavail = 0;
      if (satsimage != NULL)
	g_object_unref (satsimage);
      satsimage = NULL;
      gtk_entry_set_text (GTK_ENTRY (satslabel1), _("n/a"));
      gtk_entry_set_text (GTK_ENTRY (satslabel2), _("n/a"));
      gtk_entry_set_text (GTK_ENTRY (satslabel3), _("n/a"));
      gtk_widget_draw (drawing_sats, NULL);	/* this  calls expose handler */
    }
}


