
gint
initgps ()
{
  struct sockaddr_in server;
  struct hostent *server_data;

/*  We test for gpsd serving */
  {
/*  open socket to port */
    if (sock != -1)
      {
	close (sock);
	sock = -1;
      }
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
      {
	perror (_("can't open socket for port "));
	fprintf (stderr, "error: %d\n", errno);
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
	gtk_widget_draw (drawing_sats, NULL);	/* this  calls expose handler */
	return 0;
      }
    server.sin_family = AF_INET;
/*  We retrieve the IP address of the server from its name: */
    if ((server_data = gethostbyname (SERVERNAME)) == NULL)
      {
	fprintf (stderr, _("\nCannot connect to %s: unknown host\n"),
		 SERVERNAME);
	exit (1);
      }
    memcpy (&server.sin_addr, server_data->h_addr, server_data->h_length);
    server.sin_port = htons (SERVERPORT2);

/*  We initiate the connection  */
    if (connect (sock, (struct sockaddr *) &server, sizeof server) < 0)
      {
	server.sin_port = htons (SERVERPORT);	/* We try second port */
/*  We initiate the connection  */
	if (connect (sock, (struct sockaddr *) &server, sizeof server) < 0)
	  {
	    haveNMEA = FALSE;
	    simmode = TRUE;
	  }
	else
	  {
	    haveNMEA = TRUE;
	    simmode = FALSE;
	    g_strlcpy (nmeamodeandport, _("NMEA Mode, Port 2222"),
		       sizeof (nmeamodeandport));
	    g_strlcat (nmeamodeandport, "/", sizeof (nmeamodeandport));
	    g_strlcat (nmeamodeandport, gpsdservername,
		       sizeof (nmeamodeandport));
	  }
      }
    else
      {
	g_strlcpy (nmeamodeandport, _("NMEA Mode, Port 2947"),
		   sizeof (nmeamodeandport));
	g_strlcat (nmeamodeandport, "/", sizeof (nmeamodeandport));
	g_strlcat (nmeamodeandport, gpsdservername, sizeof (nmeamodeandport));
	write (sock, "R\n", 2);
	haveNMEA = TRUE;
	simmode = FALSE;
      }


  }

  haveGARMIN = FALSE;

#ifdef NOGARMIN
  g_print (_("\nno garmin support compiled in\n"));
  testgarmin = FALSE;
#else
  if (!testgarmin)
    g_print (_("\nGarmin protocol detection disabled!\n"));
#endif

  if ((!haveNMEA) && (testgarmin))
    {
      typedef struct
      {
	gchar *a1;
	gchar *a2;
	gchar *a3;
      }
      argument;
      argument *argumente;
      gint e;

      argumente = g_new (argument, 1);
      argumente->a1 = "-p";
      argumente->a2 = "-p";
      argumente->a3 = 0;

      e = garblemain (1, (char **) argumente);

      if (e == -2)
	{
	  e = garblemain (1, (char **) argumente);
	  if (e == -2)
	    {
	      haveGARMIN = FALSE;
	      simmode = TRUE;
	    }
	  else
	    {
	      haveGARMIN = TRUE;
	      simmode = FALSE;
	    }
	}
      else
	{
	  haveGARMIN = TRUE;
	  simmode = FALSE;
	}
      g_free (argumente);

      if (haveGARMIN)		/* test it again */
	{
	  argumente = g_new (argument, 1);
	  argumente->a1 = "-p";
	  argumente->a2 = "-p";
	  argumente->a3 = 0;

	  e = garblemain (1, (char **) argumente);
	  if (e == -1)
	    {
	      haveGARMIN = FALSE;
	      simmode = TRUE;
	    }
	}
      if (haveGARMIN)
	g_print
	  ("\nAutomatic Garmin detection found GARMIN-mode receiver \n");
      else
	g_print ("\nno GARMIN-mode receiver found\n");

    }
  if (haveGARMIN || haveNMEA)
    {
      simmode = FALSE;
      if (simpos_timeout != 0)
	{
	  gtk_timeout_remove (simpos_timeout);
	  simpos_timeout = 0;
	}
    }
  return FALSE;			/* to remove timer */
}


