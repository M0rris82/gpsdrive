/*  Main program */
int
main (int argc, char *argv[])
{
  GtkWidget *vbig, *vbig1, *vbox, *vbox2, *vbox3, *vbox4, *hbig, *hbox2,
    *hbox2a, *hbox2b, *vmenubig;
  GtkWidget *quit, *zoomin, *hbox3, *vboxlow, *hboxlow;
  GtkWidget *menuwin = NULL, *menuwin2 = NULL;
  GtkWidget *zoomout, *sel_target, *vtable, *wplabeltable, *alignment1;
  GtkWidget *alignment2, *alignment3, *alignment4;
  gchar maintitle[100];
/*   GdkColor farbe;   */
  GdkRectangle rectangle = {
    0, 0, SCREEN_X, SCREEN_Y
  };
  const gchar *hd, *pro;
  gchar buf[500];

/*** Mod by Arms */
  gint i, h, w;
  GtkWidget *table1, *wi;
  GtkTooltips *tooltips;
  gchar s1[100], s2[100], *p, *localestring, **lstr, lstr2[200];
  /*** Mod by Arms */
  GtkRequisition requ, *reqptr;
  GtkWidget *mainnotebook;
#ifndef NOPLUGINS
  GModule *mod1;
  void (*modulefunction) ();
  gchar *modpath;
#endif
  void *handle;
  char *error, s3[200];
  struct tm *lt;
  time_t local_time, gmt_time;
/*   GtkAccelGroup *accel_group; */
  gint nmenu_items = sizeof (main_menu) / sizeof (main_menu[0]);
  GdkPixbuf *mainwindow_icon_pixbuf;
  gdouble f;
#ifdef USETELEATLAS
  GtkWidget *navibt;
#else
  GtkWidget *helpbt;
#endif

  tzset ();
  gmt_time = time (NULL);

/*   printf("dbtypelist: %d, auxicons.name: %d\n",sizeof(dbtypelist[0]),sizeof(auxicons->name)); */
/*   exit(0); */

  lt = gmtime (&gmt_time);
  local_time = mktime (lt);
  zone = lt->tm_isdst + (gmt_time - local_time) / 3600;
/*   fprintf(stderr,"\n zeitzone: %d\n",zone); */

/*   zone = st->tm_gmtoff / 3600; */
/*  initialize variables */
/*  Hamburg */
  srand (gmt_time);
  f = 0.02 * (0.5 - rand () / (RAND_MAX + 1.0));
  current_lat = zero_lat = 53.623672 + f;
  f = 0.02 * (0.5 - rand () / (RAND_MAX + 1.0));
  current_long = zero_long = 10.055441 + f;
/*    zero_lat and zero_long are overwritten by gpsdriverc,  */
  tripreset ();

  g_strlcpy (cputempstring, "??", sizeof (cputempstring));
  g_strlcpy (dgpsserver, "dgps.wsrcc.com", sizeof (dgpsserver));
  g_strlcpy (dgpsport, "2104", sizeof (dgpsport));
  g_strlcpy (gpsdservername, "127.0.0.1", sizeof (gpsdservername));
  direction = angle_to_destination = 0;
  g_strlcpy (targetname, "     ", sizeof (targetname));
  g_strlcpy (utctime, "n/a", sizeof (utctime));
  g_strlcpy (oldangle, "none", sizeof (oldangle));
  pixelfact = MAPSCALE / PIXELFACT;
  g_strlcpy (oldfilename, "", sizeof (oldfilename));
  simmode = maploaded = FALSE;
  haveNMEA = FALSE;
  havepos = gblink = blink = FALSE;
  haveposcount = haveGARMIN = debug = 0;
  zoom = 1;
  milesflag = iszoomed = FALSE;
  sel_target = NULL;
  g_strlcpy (wplabelfont, "Sans 11", sizeof (wplabelfont));
  g_strlcpy (bigfont, "Sans bold 26", sizeof (bigfont));
  g_strlcpy (friendsserverip, "127.0.0.1", sizeof (friendsserverip));
  g_strlcpy (friendsserverfqn, "www.gpsdrive.cc", sizeof (friendsserverfqn));
  g_strlcpy (friendsidstring, "XXX", sizeof (friendsidstring));

  signal (SIGUSR2, usr2handler);
  timer = g_timer_new ();
  disttimer = g_timer_new ();
  g_timer_start (timer);
  g_timer_start (disttimer);
  memset (satlist, 0, sizeof (satlist));
  memset (satlistdisp, 0, sizeof (satlist));
  buffer = g_new (char, 2010);
  big = g_new (char, MAXBIG + 10);


  timeoutcount = lastp = bigp = bigpRME = bigpGSA = bigpGSV = bigpGGA = 0;
  lastp = lastpGGA = lastpGSV = lastpRME = lastpGSA = 0;
  downloadfilelen = gcount = xoff = yoff = 0;
  hours = minutes = 99;
  milesconv = 1.0;
/* set default color to navyblue */
  g_strlcpy (bluecolor, "navyblue", sizeof (bluecolor));
  g_strlcpy (trackcolor, "green3", sizeof (trackcolor));
  g_strlcpy (friendscolor, "orange", sizeof (friendscolor));
  g_strlcpy (messagename, "", sizeof (messagename));
  g_strlcpy (messageack, "", sizeof (messageack));
  g_strlcpy (messagesendtext, "", sizeof (messagesendtext));

  downloadwindowactive = downloadactive = importactive = FALSE;
  g_strlcpy (lastradar, "", sizeof (lastradar));
  g_strlcpy (lastradar2, "", sizeof (lastradar2));
  g_strlcpy (activewpfile, "way.txt", sizeof (activewpfile));
  g_strlcpy (dbhost, "localhost", sizeof (dbhost));
  g_strlcpy (dbuser, "gast", sizeof (dbuser));
  g_strlcpy (dbpass, "gast", sizeof (dbpass));
  g_strlcpy (dbname, "geoinfo", sizeof (dbname));
  g_strlcpy (dbtable, "waypoints", sizeof (dbtable));
  dbdistance = 2000.0;
  dbusedist = TRUE;
  g_strlcpy (loctime, "n/a", sizeof (loctime));
  voicelang = english;
  track = g_new0 (GdkSegment, 100000);
  trackshadow = g_new0 (GdkSegment, 100000);
  tracknr = 0;
  trackcoord = g_new0 (trackcoordstruct, 100000);
  trackcoordnr = 0;
  tracklimit = trackcoordlimit = 100000;
  routelist = g_new0 (wpstruct, 100);



  earthr = calcR (current_lat);
/*    homedir is the directory where the maps and other  */
/*  files are stored (~/.gpsdrive) */
  hd = g_get_home_dir ();
  g_strlcpy (homedir, hd, sizeof (homedir));
  g_strlcat (homedir, "/.gpsdrive/", sizeof (homedir));
  g_strlcpy (mapdir, homedir, sizeof (mapdir));

/*  all default values must be set BEFORE readconfig! */
  g_strlcpy (setpositionname, "", sizeof (setpositionname));
  g_strlcpy (serialdev, "/dev/ttyS3", sizeof (serialdev));

/* setup signal handler */
  signal (SIGUSR1, signalposreq);

  usesql = TRUE;
// It seems like this doesnt work on cygwin unless the dlopen comes first..-jc
  if (usesql)
    {
      handle = dlopen ("/usr/local/lib/libmysqlclient.dll", RTLD_LAZY);
      if (!handle)
	handle =
	  dlopen ("@PREFIX@/lib/mysql/libmysqlclient.10.dylib", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/opt/lib/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/opt/mysql/lib/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/lib/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle =
	  dlopen ("/usr/local/lib/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/local/mysql/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/usr/local/lib/libmysqlclient.so.10", RTLD_LAZY);
      if (!handle)
	handle = dlopen ("/sw/lib/libmysqlclient.dylib", RTLD_LAZY);

      if (handle)
	{
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

/*  I18l */

/*  Detect the language for voice output */
  localestring = setlocale (LC_ALL, "");
  if (localestring == NULL)
    localestring = setlocale (LC_MESSAGES, "");
  if (localestring != NULL)
    {
      lstr = g_strsplit (localestring, ";", 50);
      g_strlcpy (lstr2, "", 50);
      for (i = 0; i < 50; i++)
	if (lstr[i] != NULL)
	  {
	    if ((strstr (lstr[i], "LC_MESSAGES")) != NULL)
	      {
		g_strlcpy (lstr2, lstr[i], 50);
		break;
	      }
	  }
	else
	  {
	    g_strlcpy (lstr2, lstr[i - 1], 50);
	    break;
	  }
      g_strfreev (lstr);
    }
  if ((strstr (lstr2, "de_")) != NULL)
    voicelang = german;
  else if ((strstr (lstr2, "es_")) != NULL)
    voicelang = spanish;
  else
    voicelang = english;

  g_strlcpy (friendsname, "", sizeof (friendsname));



  readconfig ();

/*   we have to set expedia to false, because its not working anymore */
/*   expedia=FALSE; */
/*   defaultserver=0; */

  real_screen_x = 640;
  real_screen_y = 512;
  target_long = current_long + 0.00001;
  target_lat = current_lat + 0.00001;
  gdk_color_parse (trackcolor, &trackcolorv);
  gdk_color_parse (friendscolor, &orange);

/*  load waypoints before locale is set! */
/*  Attention! In this file the decimal point is always a '.' !! */

/*  setting defaults if setting is not yet in configuraton file */
  if ((milesflag + metricflag + nauticflag) == 0)
    metricflag = TRUE;



/*  load mapfile configurations */
/*  Attention! In this file the decimal point is that what locale says, 
i.e. '.' in english, ',' in german!! */
  loadmapconfig ();

/* PORTING */
  p = bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "utf8");
  p = textdomain (GETTEXT_PACKAGE);
  p = textdomain (NULL);
/*    needed for right decimal delimiter ('.' or ',') */
  mylocale = NULL;
  localedecimal = '.';
  mylocale = localeconv ();
  if (mylocale != NULL)
    localedecimal = (char) mylocale->decimal_point[0];

/*    Setting locale for correct Umlauts */
  gtk_set_locale ();
/*  initialization for GTK+ */


  gtk_init (&argc, &argv);
/* Needed 4 hours to find out that this is IMPORTANT!!!! */
  gdk_rgb_init ();
  h = gdk_screen_height ();
  w = gdk_screen_width ();

  /* parse cmd args */
  do
    {
/*** Mod by Arms */
      i = getopt (argc, argv, "W:ESA:ab:c:zx1qivdDFepH:hnf:l:t:s:o:r:?");
      switch (i)
	{
	case 'a':
	  disableapm = TRUE;
	  break;
	case 'S':
	  nosplash = TRUE;
	  break;
	case 'E':
	  nmeaverbose = TRUE;
	  break;
	case 'q':
	  usesql = FALSE;
	  break;
	case 'd':
	  debug = TRUE;
	  break;
	case 'D':
	  mydebug = TRUE;
	  debug = TRUE;
	  break;
	case 'e':
	  useflite = TRUE;
	  break;
	case 'i':
	  ignorechecksum = TRUE;
	  g_print ("\nWARNING: NMEA checksum test switched off!\n\n");
	  break;
	case 'x':
	  extrawinmenu = TRUE;
	  break;
	case 'p':
	  pdamode = TRUE;
	  break;
	case '1':
	  onemousebutton = TRUE;
	  break;
	case 'v':
	  printf ("\ngpsdrive (c) 2001-2004 Fritz Ganter <ganter@ganter.at>\n"
		  "\nVersion %s\n%s\n\n", VERSION, rcsid);
	  exit (0);
	  break;
	case 't':
	  g_strlcpy (serialdev, optarg, sizeof (serialdev));
	  break;
	case 'A':
	  alarm_dist = strtod (optarg, NULL);
	  break;
	case 'b':
	  g_strlcpy (gpsdservername, optarg, sizeof (gpsdservername));
	  break;
	case 'c':
	  g_strlcpy (setpositionname, optarg, sizeof (setpositionname));
	  break;
	case 'f':
	case 'n':
	  disableserialcl = TRUE;
	  break;
	case 's':
	  h = strtol (optarg, NULL, 0);
	  break;
	case 'W':
	  switch (strtol (optarg, NULL, 0))
	    {
	    case 0:
	      egnosoff = TRUE;
	      break;
	    case 1:
	      egnoson = TRUE;
	      break;
	    }
	  break;
	case 'l':
	  if (!strcmp (optarg, "english"))
	    voicelang = english;
	  else if (!strcmp (optarg, "german"))
	    voicelang = german;
	  else if (!strcmp (optarg, "spanish"))
	    voicelang = spanish;
	  else
	    {
	      usage ();
	      g_print (_
		       ("\nYou can only choose between english, spanish and german\n\n"));
	      exit (0);
	    }
	  break;
	case 'o':
	  nmeaout = opennmea (optarg);
	  break;
	case 'h':
	  usage ();
	  exit (0);
	  break;
	case 'H':
	  normalnull = strtol (optarg, NULL, 0);
	  break;
	case '?':
	  usage ();
	  exit (0);
	  break;
/*** Mod by Arms */
	case 'r':
	  w = strtol (optarg, NULL, 0);
	  break;
	case 'z':
	  zoomscale = FALSE;
	  break;
	case 'F':
	  forcehavepos = TRUE;
	  break;

	}
    }
  while (i != -1);

  if ((strlen (friendsname) == 0))
    g_strlcpy (friendsname, _("EnterYourName"), sizeof (friendsname));

  load_icons();

/*  Build array for earth radii */
  for (i = -100; i <= 100; i++)
    Ra[i + 100] = calcR (i);

/*** Mod by Arms (move) */
/*    g_print ("\nHÂŽöhe: %d\n", h); */


/* parse args moved up */

  gethostname (hostname, 256);
  proxyport = 80;
  haveproxy = FALSE;

  pro = g_getenv ("HTTP_PROXY");
  if (pro == NULL)
    pro = g_getenv ("http_proxy");

  if (debug)
    printf ("\ngpsdrive (c) 2001-2004 Fritz Ganter <ganter@ganter.at>\n"
	    "\nVersion %s\n%s\n\n", VERSION, rcsid);

  if (pro)
    {
      p = (char *) pro;
      g_strdelimit (p, ":/", ' ');

      i = sscanf (p, "%s %s %d", s1, s2, &proxyport);
      if (i == 3)
	{
	  haveproxy = TRUE;
	  g_strlcpy (proxy, s2, sizeof (proxy));
	  if (debug)
	    g_print (_("\nUsing proxy: %s on port %d"), proxy, proxyport);
	}
      else
	{
	  g_print (_("\nInvalid enviroment variable HTTP_PROXY, "
		     "must be in format: http://proxy.provider.de:3128"));
	}
    }

  if (debug)
    g_print ("\nGpsDrive version %s\n%s\n", VERSION, rcsid);

/*  show splash screen */
  if (!nosplash)
    splash ();

/* init sql support */
  if (usesql)
    usesql = sqlinit ();

  if (!usesql)
    sqlflag = FALSE;

/* loadwaypoints moved down */


/* Create toplevel window */

  PSIZE = 50;
  SMALLMENU = 0;
/*** Mod by Arms */
  PADDING = 1;
  if (h >= 1024)		/* 1280x1024 */
    {
      real_screen_x = 840;
      real_screen_y = 600;
    }
  else if (h >= 768)		/* 1024x768 */
    {
      real_screen_x = 800;
      real_screen_y = 540;
    }
  else if (h >= 600)		/* 800x600 */
    {
      real_screen_x = 690;
      real_screen_y = 455;
      PADDING = 0;
    }
  else if (h >= 480)		/* 640x480 */
    {
/*** Mod by Arms */
      if (w == 0)
	real_screen_x = 630;
      else
	real_screen_x = w - XMINUS;
      real_screen_y = h - YMINUS;
    }
  else if (h < 480)
    {
/*** Mod by Arms */
      if (w == 0)
	real_screen_x = 220;
      else
	real_screen_x = w - XMINUS;
      real_screen_y = h - YMINUS;
      PSIZE = 25;
      SMALLMENU = 1;
/*** Mod by Arms */
      PADDING = 0;
    }

  if ((extrawinmenu) && (w != 0))
    {
      real_screen_x += XMINUS - 10;
      real_screen_y += YMINUS - 30;
    }

/*   g_print ("\nx: %d, y:%d", h, w); */
  if (((w == 240) && (h == 320)) || ((h == 240) && (w == 320)))
    {
      pdamode = TRUE;
/*       SMALLMENU = 1; */
      real_screen_x = w - 8;
      real_screen_y = h - 70;
    }
  if (pdamode)
    {
      extrawinmenu = TRUE;
      PADDING = 0;
//KCFX          
      g_strlcpy (wplabelfont, "Sans 8", sizeof (wplabelfont));
      g_strlcpy (bigfont, "Sans bold 16", sizeof (bigfont));
      g_print ("\nPDA mode\n");
    }

  if (real_screen_x < real_screen_y)
    borderlimit = real_screen_x / 5;
  else
    borderlimit = real_screen_y / 5;

  if (borderlimit < 30)
    borderlimit = 30;

  SCREEN_X_2 = SCREEN_X / 2;
  SCREEN_Y_2 = SCREEN_Y / 2;
  PSIZE = 50;
  posx = SCREEN_X_2;
  posy = SCREEN_Y_2;

//KCFX
/*   if (pdamode) mod_setupcounter++;  */
// Fritz commented out above line
#ifndef NOPLUGINS
  useplugins = TRUE;
#endif
  if (pdamode)
    useplugins = FALSE;

  if (usesql)
    {
      mod_setupcounter++;
      setupfunction[mod_setupcounter] = &(sqlsetup);
      sqlplace = mod_setupcounter;
    }
  mod_setupcounter++;
  setupfunction[mod_setupcounter] = &(friendssetup);
  friendsplace = mod_setupcounter;

#ifndef NOPLUGINS
  if (useplugins)
    {
/*  fly module */
      modpath = g_module_build_path (LIBDIR, "libfly.so");
      mod1 = g_module_open (".libs/libfly.so", 0);
      if (mod1 == NULL)
	mod1 = g_module_open (modpath, 0);
      if (mod1 != NULL)
	{
	  gint *modulever;
	  mod_setupcounter++;

	  i =
	    g_module_symbol (mod1, "moduleversion", (gpointer *) & modulever);
	  g_print (" (Version %d)", *modulever);
	  i =
	    g_module_symbol (mod1, "modulesetup",
			     (gpointer *) & modulefunction);
	  setupfunction[mod_setupcounter] = modulefunction;
	}

/*  nautic module */
      modpath = g_module_build_path (LIBDIR, "libnautic.so");
      mod1 = g_module_open (".libs/libnautic.so", 0);
      if (mod1 == NULL)
	mod1 = g_module_open (modpath, 0);
      if (mod1 != NULL)
	{
	  gint *modulever;
	  mod_setupcounter++;
	  i =
	    g_module_symbol (mod1, "moduleversion", (gpointer *) & modulever);
	  g_print (" (Version %d)", *modulever);
	  i =
	    g_module_symbol (mod1, "modulesetup",
			     (gpointer *) & modulefunction);
	  setupfunction[mod_setupcounter] = modulefunction;
	}
    }
#endif
  fprintf (stderr, "\n");

  mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  if (!nosplash)
    gtk_window_set_transient_for (GTK_WINDOW (splash_window),
				  GTK_WINDOW (mainwindow));



  g_snprintf (maintitle, sizeof (maintitle),
	      "%s v%s  \xc2\xa9 2001-2004 Fritz Ganter", "GpsDrive", VERSION);

  gtk_window_set_title (GTK_WINDOW (mainwindow), maintitle);
  gtk_container_set_border_width (GTK_CONTAINER (mainwindow), 0);
  gtk_window_set_position (GTK_WINDOW (mainwindow), GTK_WIN_POS_CENTER);
  gtk_signal_connect (GTK_OBJECT (mainwindow), "delete_event",
		      GTK_SIGNAL_FUNC (quit_program), NULL);

  gtk_signal_connect (GTK_OBJECT (mainwindow), "key_press_event",
		      GTK_SIGNAL_FUNC (key_cb), NULL);

  status = gtk_statusbar_new ();
  statusid = gtk_statusbar_get_context_id (GTK_STATUSBAR (status), "main");

  gtk_statusbar_push (GTK_STATUSBAR (status), statusid,
		      _("Gpsdrive-2 (c)2001-2004 F.Ganter"));
  if (!useflite)
    havespeechout = speech_out_init ();
  else
    havespeechout = TRUE;

  if (havespeechout)
    gtk_statusbar_push (GTK_STATUSBAR (status), statusid,
			_("Using speech output"));
  if (!useflite)
    switch (voicelang)
      {
      case english:
	speech_out_speek_raw (FESTIVAL_ENGLISH_INIT);
	break;
      case spanish:
	speech_out_speek_raw (FESTIVAL_SPANISH_INIT);
	break;
      case german:
	speech_out_speek_raw (FESTIVAL_GERMAN_INIT);
	break;
      }

/*** Mod by Arms */
  vbig = gtk_vbox_new (FALSE, 0 * PADDING);
  vmenubig = gtk_vbox_new (FALSE, 0 * PADDING);
/*** Mod by Arms */
  hbig = gtk_hbox_new (FALSE, 0 * PADDING);
/*   accel_group = gtk_accel_group_new (); */
  item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", NULL);
/*  ÂŽÜbersetzen laut Bluefish Code */
  gtk_item_factory_set_translate_func (item_factory, menu_translate, "<main>",
				       NULL);
  gtk_item_factory_create_items (item_factory, nmenu_items, main_menu, NULL);
/*   gtk_accel_group_attach (accel_group, GTK_OBJECT (hauptfenster)); */
//gtk_accel_group_attach (accel_group, GTK_OBJECT (mainwindow));
//HRM: above must be new in GTK 2.0!

  menubar = gtk_item_factory_get_widget (item_factory, "<main>");

  wi =
    gtk_item_factory_get_item (item_factory,
			       N_("/Misc. Menu/Maps/Map Manager"));
  gtk_widget_set_sensitive (wi, FALSE);
  wi =
    gtk_item_factory_get_item (item_factory,
			       N_("/Misc. Menu/Waypoint Manager"));
  gtk_widget_set_sensitive (wi, FALSE);

/*  download map button */
  downloadbt = gtk_button_new_with_mnemonic (_("_Download map"));
/*   gtk_button_set_use_underline (GTK_BUTTON(downloadbt),TRUE); */
/*   gtk_label_set_text_with_mnemonic(GTK_LABEL(downloadbt),_("Karte __Download")); */
/*    gtk_label_set_mnemonic_widget(GTK_LABEL(GTK_BUTTON(downloadbt)),GTK_WIDGET (downloadbt));    */

  gtk_signal_connect (GTK_OBJECT (downloadbt),
		      "clicked", G_CALLBACK (download_cb), (gpointer) 1);
/*  Quit button */
/* PORTING */
/*   quit = gtk_button_new_with_label (_("Quit")); */
  quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_button_set_use_underline (GTK_BUTTON (quit), TRUE);

  gtk_signal_connect (GTK_OBJECT (quit),
		      "clicked", GTK_SIGNAL_FUNC (quit_program), 0);
/*    GTK_WIDGET_SET_FLAGS (quit, GTK_CAN_DEFAULT); */

  if (havespeechout)
    {
      mutebt = gtk_check_button_new_with_label (_("M_ute"));
      gtk_button_set_use_underline (GTK_BUTTON (mutebt), TRUE);
      if (muteflag)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mutebt), TRUE);

      gtk_signal_connect (GTK_OBJECT (mutebt),
			  "clicked", GTK_SIGNAL_FUNC (mute_cb), (gpointer) 1);
    }


  // Checkbox ---- POI Draw
  poi_draw_bt = gtk_check_button_new_with_label (_("draw PO_I"));
  gtk_button_set_use_underline (GTK_BUTTON (poi_draw_bt), TRUE);
  if (!poi_draw)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (poi_draw_bt), TRUE);
  gtk_signal_connect (GTK_OBJECT (poi_draw_bt),
		      "clicked", GTK_SIGNAL_FUNC (poi_draw_cb), (gpointer) 1);
  /*
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), poi_draw_bt,
    _("This will show Point of interrest located in mySQL Database"),
    NULL);
  */

  // Checkbox ---- STREETS Draw
  streets_draw_bt = gtk_check_button_new_with_label (_("draw _Streets"));
  gtk_button_set_use_underline (GTK_BUTTON (streets_draw_bt), TRUE);
  if (!streets_draw)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (streets_draw_bt), TRUE);
  gtk_signal_connect (GTK_OBJECT (streets_draw_bt),
		      "clicked", GTK_SIGNAL_FUNC (streets_draw_cb), (gpointer) 1);
  /*
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), streets_draw_bt,
    _("This will show Streets Data located in mySQL Database"),
    NULL);
  */
  



  // Checkbox ----   Use SQL
  if (usesql)
    {
      sqlbt = gtk_check_button_new_with_label (_("Use SQ_L"));
      gtk_button_set_use_underline (GTK_BUTTON (sqlbt), TRUE);
      if (sqlflag)
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sqlbt), TRUE);

      gtk_signal_connect (GTK_OBJECT (sqlbt),
			  "clicked", GTK_SIGNAL_FUNC (sql_cb), (gpointer) 1);
    }


  // Checkbox ---- Show WP
  wpbt = gtk_check_button_new_with_label (_("Show _WP"));
  gtk_button_set_use_underline (GTK_BUTTON (wpbt), TRUE);
  if (wpflag)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wpbt), TRUE);
  gtk_signal_connect (GTK_OBJECT (wpbt),
		      "clicked", GTK_SIGNAL_FUNC (wp_cb), (gpointer) 1);


  if (sqlflag)
    {
      get_sql_type_list ();
      getsqldata ();
    }
  else
    loadwaypoints ();

  for (i = 0; i < maxwp; i++)
    {
      if (strlen (setpositionname) > 0)
	{
	  if (!(strcasecmp ((wayp + i)->name, setpositionname)))
	    {
	      current_lat = (wayp + i)->lat;
	      current_long = (wayp + i)->lon;
	      target_long = current_long + 0.00001;
	      target_lat = current_lat + 0.00001;
	    }
	  if (!(strcasecmp ((wayp + i)->name, _("HomeBase"))))
	    {
	      alarm_lat = (wayp + i)->lat;
	      alarm_long = (wayp + i)->lon;
	    }
	}
    }

  posbt = gtk_check_button_new_with_label (_("Pos. _mode"));
  gtk_button_set_use_underline (GTK_BUTTON (posbt), TRUE);

  gtk_signal_connect (GTK_OBJECT (posbt),
		      "clicked", GTK_SIGNAL_FUNC (pos_cb), (gpointer) 1);

  trackbt = gtk_check_button_new_with_label (_("Show _Track"));
  gtk_button_set_use_underline (GTK_BUTTON (trackbt), TRUE);
  if (trackflag)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (trackbt), TRUE);
  gtk_signal_connect (GTK_OBJECT (trackbt),
		      "clicked", GTK_SIGNAL_FUNC (track_cb), (gpointer) 1);

/*   importbt = gtk_button_new_with_label (_("Import")); */
/*   gtk_signal_connect (GTK_OBJECT (importbt), */
/* 		      "clicked", GTK_SIGNAL_FUNC (import1_cb), (gpointer) 1); */
/*    GTK_WIDGET_SET_FLAGS (importbt, GTK_CAN_DEFAULT); */

/*   loadtrackbt = gtk_button_new_from_stock (GTK_STOCK_OPEN); */
/*   gtk_signal_connect (GTK_OBJECT (loadtrackbt), */
/* 		      "clicked", GTK_SIGNAL_FUNC (loadtrack_cb), */
/* 		      (gpointer) 1); */
/*    GTK_WIDGET_SET_FLAGS (loadtrackbt, GTK_CAN_DEFAULT); */

#ifndef USETELEATLAS
  helpbt = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_signal_connect (GTK_OBJECT (helpbt),
		      "clicked", GTK_SIGNAL_FUNC (help_cb), (gpointer) 1);
#else
  {
    GtkWidget *image3, *hbox3, *alignment3, *label;

    ta_init ();
    navibt = gtk_button_new ();
    gtk_signal_connect (GTK_OBJECT (navibt),
			"clicked", GTK_SIGNAL_FUNC (navi_cb), (gpointer) 1);
    gtk_widget_show (navibt);
    alignment3 = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
    gtk_widget_show (alignment3);
    gtk_container_add (GTK_CONTAINER (navibt), alignment3);


    hbox3 = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (hbox3);
    gtk_container_add (GTK_CONTAINER (alignment3), hbox3);

    image3 = create_pixmap (mainwindow, "gpsiconbt.png");
    gtk_widget_show (image3);
    gtk_box_pack_start (GTK_BOX (hbox3), image3, FALSE, FALSE, 0);
    label = gtk_label_new_with_mnemonic (_("_Navigation"));
    gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);


  }
#endif

/* PORTING */
/*   setupbt = gtk_button_new_with_label (_("Setup")); */
  setupbt = gtk_button_new_from_stock (GTK_STOCK_PREFERENCES);

  gtk_signal_connect (GTK_OBJECT (setupbt),
		      "clicked", GTK_SIGNAL_FUNC (setup_cb), (gpointer) 0);

  // Checkbox ---- Start GPSD
  startgpsbt = gtk_button_new_with_label (_("Start GPSD"));
  gtk_signal_connect (GTK_OBJECT (startgpsbt),
		      "clicked", GTK_SIGNAL_FUNC (startgpsd), (gpointer) 0);

  // Checkbox ---- Best Map
  bestmapbt = gtk_check_button_new_with_label (_("Auto _best map"));
  gtk_button_set_use_underline (GTK_BUTTON (bestmapbt), TRUE);

  if (!scaleprefered)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bestmapbt), TRUE);
  gtk_signal_connect (GTK_OBJECT (bestmapbt),
		      "clicked", GTK_SIGNAL_FUNC (bestmap_cb), (gpointer) 1);


  // Checkbox ---- Save Track
  savetrackfile (0);
  g_snprintf (s1, sizeof (s1), "%s", _("Save track"));
  savetrackbt = gtk_check_button_new_with_label (s1);
  if (savetrack)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (savetrackbt), TRUE);
  gtk_signal_connect (GTK_OBJECT (savetrackbt),
		      "clicked", GTK_SIGNAL_FUNC (savetrack_cb),
		      (gpointer) 1);

  g_snprintf (s1, sizeof (s1), "%s", savetrackfn);
  lab1 = gtk_label_new (s1);

  // Checkbox ---- Show Map
  frame_maptype = gtk_frame_new (_("Shown map type"));
/*** Mod by Arms */
  vbox3 = gtk_vbox_new (TRUE, 1 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame_maptype), vbox3);

  frame_toogles = gtk_frame_new (NULL);
/*** Mod by Arms */
  vbox4 = gtk_vbox_new (TRUE, 1 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame_toogles), vbox4);


  // Checkbox ---- Show Map: map_
  maptogglebt = gtk_check_button_new_with_label (_("Street map"));
  if (displaymap_map)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (maptogglebt), TRUE);

  gtk_signal_connect (GTK_OBJECT (maptogglebt),
		      "clicked", GTK_SIGNAL_FUNC (maptoggle_cb),
		      (gpointer) 1);
  // Checkbox ---- Show Map: top_
  topotogglebt = gtk_check_button_new_with_label (_("Topo map"));
  if (displaymap_top)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (topotogglebt), TRUE);

  gtk_signal_connect (GTK_OBJECT (topotogglebt),
		      "clicked", GTK_SIGNAL_FUNC (topotoggle_cb),
		      (gpointer) 1);
/*** Mod by Arms */
  gtk_box_pack_start (GTK_BOX (vbox3), maptogglebt, FALSE, FALSE,
		      0 * PADDING);
/*** Mod by Arms */
  gtk_box_pack_start (GTK_BOX (vbox3), topotogglebt, FALSE, FALSE,
		      0 * PADDING);



/*  Zoom in button */
  zoomin = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
  gtk_signal_connect (GTK_OBJECT (zoomin),
		      "clicked", GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 1);
/*    GTK_WIDGET_SET_FLAGS (zoomin, GTK_CAN_DEFAULT); */
/*  Zoom out button */
  zoomout = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
  gtk_signal_connect (GTK_OBJECT (zoomout),
		      "clicked", GTK_SIGNAL_FUNC (zoom_cb), (gpointer) 2);
/*    GTK_WIDGET_SET_FLAGS (zoomout, GTK_CAN_DEFAULT); */


  scalerrbt = gtk_button_new_with_label (">>");
  gtk_signal_connect (GTK_OBJECT (scalerrbt),
		      "clicked", GTK_SIGNAL_FUNC (scalerbt_cb), (gpointer) 1);
/*    GTK_WIDGET_SET_FLAGS (scalerrbt, GTK_CAN_DEFAULT); */
  scalerlbt = gtk_button_new_with_label ("<<");
  gtk_signal_connect (GTK_OBJECT (scalerlbt),
		      "clicked", GTK_SIGNAL_FUNC (scalerbt_cb), (gpointer) 2);
/*    GTK_WIDGET_SET_FLAGS (scalerlbt, GTK_CAN_DEFAULT); */

/*  Select target button */
/*    if (maxwp > 0) */
  {
/* PORTING */
/*     sel_target = gtk_button_new_with_label (_("Select target")); */
    sel_target = gtk_button_new_from_stock (GTK_STOCK_FIND);
/*     gtk_label_set_text(GTK_BUTTON(sel_target),_("Select target")); */
    gtk_signal_connect (GTK_OBJECT (sel_target),
			"clicked",
			GTK_SIGNAL_FUNC (sel_target_cb), (gpointer) 2);
/*        GTK_WIDGET_SET_FLAGS (sel_target, GTK_CAN_DEFAULT); */
  }




/*    gtk_window_set_default (GTK_WINDOW (mainwindow), zoomin); */
/*    if we want NMEA mode, gpsd must be running and we connect to port 2222 */
/*    An alternate gpsd server may be on 2947, we try it also */

  initgps ();
  if (simmode)
    {
      if ((!disableserial) && (!disableserialcl))
	{
	  haveserial = gpsserialinit ();
	  if (haveserial)
	    {
	      simmode = FALSE;
	      haveNMEA = TRUE;
	      gtk_widget_set_sensitive (startgpsbt, FALSE);
	    }
	}
    }

  if (haveGARMIN)
    gtk_widget_set_sensitive (startgpsbt, FALSE);

  friendsinit ();


  if (usesql)
    initkismet ();
  if (havekismet)
    {
      g_print (_("\nkismet server found\n"));
      switch (voicelang)
	{
	case english:
	  g_snprintf (buf, sizeof (buf), "Found kismet. Happy wardriving");
	  break;
	case spanish:
	  g_snprintf (buf, sizeof (buf), "Found kismet. Happy wardriving");
	  break;
	case german:
	  g_snprintf (buf, sizeof (buf),
		      "Kismet gefunden. Viel Spass beim wordreifing");
	}
      speech_out_speek (buf);
    }

  load_friends_icon ();

  /*  Area for map */
  dframe = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (dframe), GTK_SHADOW_IN);
  drawing_area = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), SCREEN_X, SCREEN_Y);
/*** Mod by Arms */
  gtk_container_add (GTK_CONTAINER (dframe), drawing_area);

  gtk_box_pack_start (GTK_BOX (vbig), dframe, TRUE, TRUE, 0 * PADDING);
  gtk_widget_add_events (GTK_WIDGET (drawing_area), GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect_object (GTK_OBJECT (drawing_area),
			     "button-press-event",
			     GTK_SIGNAL_FUNC (mapclick_cb),
			     GTK_OBJECT (drawing_area));
/* Area for navigation pointer */
  drawing_bearing = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_bearing), PSIZE + 2,
			 PSIZE + 2);
  gtk_signal_connect (GTK_OBJECT (drawing_bearing),
		      "expose_event", GTK_SIGNAL_FUNC (expose_compass), NULL);
/* Area for mini map */
/*** Mod by Arms */
/* With small displays, this isn't really necessary! */
  if (SMALLMENU == 0)
    {
      drawing_miniimage = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_miniimage), 128, 103);
      gtk_signal_connect (GTK_OBJECT (drawing_miniimage),
			  "expose_event", GTK_SIGNAL_FUNC (expose_mini_cb),
			  NULL);
      gtk_widget_add_events (GTK_WIDGET (drawing_miniimage),
			     GDK_BUTTON_PRESS_MASK);
      gtk_signal_connect_object (GTK_OBJECT (drawing_miniimage),
				 "button-press-event",
				 GTK_SIGNAL_FUNC (minimapclick_cb),
				 GTK_OBJECT (drawing_miniimage));
    }
/*** Mod by Arms */
  hbox2 = gtk_hbox_new (FALSE, 1 * PADDING);
  hbox2a = gtk_hbox_new (FALSE, 1 * PADDING);
  hbox2b = gtk_vbox_new (FALSE, 1 * PADDING);
/*** Mod by Arms */
  hbox3 = gtk_hbox_new (FALSE, 1 * PADDING);
  frame_bearing = gtk_frame_new (_("Bearing"));
/*   gtk_container_add (GTK_CONTAINER (frame_bearing), drawing_bearing); */
  compasseventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (compasseventbox), drawing_bearing);
  alignment1 = gtk_alignment_new (0.6, 0.5, 0, 0);
  gtk_container_add (GTK_CONTAINER (alignment1), compasseventbox);
  gtk_container_add (GTK_CONTAINER (frame_bearing), alignment1);

/*** Mod by Arms */
  gtk_box_pack_start (GTK_BOX (hbox2), frame_bearing, FALSE, FALSE,
		      1 * PADDING);
/* Area for field strength, we have data only in NMEA mode */
  drawing_sats = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_sats), PSIZE + 10,
			 PSIZE + 6);
  gtk_signal_connect (GTK_OBJECT (drawing_sats), "expose_event",
		      GTK_SIGNAL_FUNC (expose_sats_cb), NULL);
  gtk_widget_add_events (GTK_WIDGET (drawing_sats), GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect (GTK_OBJECT (drawing_sats),
		      "button-press-event",
		      GTK_SIGNAL_FUNC (satpos_cb), NULL);
  frame_sats = gtk_frame_new (_("GPS Info"));
  sateventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (sateventbox), drawing_sats);
  alignment2 = gtk_alignment_new (0.5, 0.5, 0, 0);
  gtk_container_add (GTK_CONTAINER (alignment2), sateventbox);
  satsvbox = gtk_vbox_new (FALSE, 1 * PADDING);
  satshbox = gtk_hbox_new (FALSE, 1 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame_sats), satshbox);
  gtk_box_pack_start (GTK_BOX (satshbox), alignment2, FALSE, FALSE,
		      1 * PADDING);
  gtk_box_pack_start (GTK_BOX (satshbox), satsvbox, FALSE, FALSE,
		      1 * PADDING);

  satslabel1 = gtk_entry_new ();
  satslabel1eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (satslabel1eventbox), satslabel1);
  gtk_entry_set_text (GTK_ENTRY (satslabel1), _("n/a"));
  gtk_box_pack_start (GTK_BOX (satsvbox), satslabel1eventbox, TRUE, FALSE,
		      0 * PADDING);

  satslabel2 = gtk_entry_new ();
  satslabel2eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (satslabel2eventbox), satslabel2);
  gtk_entry_set_text (GTK_ENTRY (satslabel2), _("n/a"));
  gtk_box_pack_start (GTK_BOX (satsvbox), satslabel2eventbox, TRUE, FALSE,
		      0 * PADDING);

  satslabel3 = gtk_entry_new ();
  satslabel3eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (satslabel3eventbox), satslabel3);
  gtk_entry_set_text (GTK_ENTRY (satslabel3), _("n/a"));
  gtk_box_pack_start (GTK_BOX (satsvbox), satslabel3eventbox, TRUE, FALSE,
		      0 * PADDING);

  gtk_entry_set_editable (GTK_ENTRY (satslabel1), FALSE);
  gtk_widget_set_usize (satslabel1, 38, 20);
  gtk_entry_set_editable (GTK_ENTRY (satslabel2), FALSE);
  gtk_widget_set_usize (satslabel2, 38, 20);
  gtk_entry_set_editable (GTK_ENTRY (satslabel3), FALSE);
  gtk_widget_set_usize (satslabel3, 38, 20);

/*** Mod by Arms */
  gtk_box_pack_start (GTK_BOX (hbox2), frame_sats, FALSE, FALSE, 1 * PADDING);

  havebattery = battery_get_values ();
  if (debug)
    fprintf (stderr, "batt: %d, temp: %d", havebattery, havetemperature);
  if (havebattery)
    {
      drawing_battery = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_battery), 27, 52);
      fbat = gtk_frame_new (_("Bat."));
      batteventbox = gtk_event_box_new ();
      gtk_container_add (GTK_CONTAINER (batteventbox), drawing_battery);
      alignment3 = gtk_alignment_new (0.5, 0.5, 0, 0);
      gtk_container_add (GTK_CONTAINER (alignment3), batteventbox);
      gtk_container_add (GTK_CONTAINER (fbat), alignment3);
      gtk_box_pack_start (GTK_BOX (hbox2), fbat, FALSE, FALSE, 1 * PADDING);
    }

/* JH added drawing area for cpu temp meter  */

  if (havetemperature)
    {
      drawing_temp = gtk_drawing_area_new ();
      gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_temp), 15, 52);
      ftem = gtk_frame_new (_("TC"));
      tempeventbox = gtk_event_box_new ();
      gtk_container_add (GTK_CONTAINER (tempeventbox), drawing_temp);
      alignment4 = gtk_alignment_new (0.5, 0.5, 0, 0);
      gtk_container_add (GTK_CONTAINER (alignment4), tempeventbox);
      gtk_container_add (GTK_CONTAINER (ftem), alignment4);
      gtk_box_pack_start (GTK_BOX (hbox2), ftem, FALSE, FALSE, 1 * PADDING);
    }


  if (pdamode)
    gtk_box_pack_start (GTK_BOX (hbox2), hbox2b, TRUE, TRUE, 1 * PADDING);

/*  displays distance to destination */
  distlabel = gtk_label_new ("---");
  gtk_label_set_use_markup (GTK_LABEL (distlabel), TRUE);
  gtk_label_set_justify (GTK_LABEL (distlabel), GTK_JUSTIFY_RIGHT);
/*  displays speed over ground */
  speedlabel = gtk_label_new (_("---"));
  gtk_label_set_justify (GTK_LABEL (speedlabel), GTK_JUSTIFY_RIGHT);
/*  displays zoom factor of map */
  altilabel = gtk_label_new (_("n/a"));
  if (pdamode)
    {
      g_snprintf (s3, sizeof (s3),
		  "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"5000\">%s</span>",
		  bluecolor, _("n/a"));
    }
  else
    {
      g_snprintf (s3, sizeof (s3),
		  "<span color=\"%s\" font_family=\"Arial\" weight=\"bold\" size=\"10000\">%s</span>",
		  bluecolor, _("n/a"));
    }
  gtk_label_set_markup (GTK_LABEL (altilabel), s3);
  gtk_label_set_justify (GTK_LABEL (altilabel), GTK_JUSTIFY_RIGHT);

/*  displays waypoints number */
  wplabeltable = gtk_table_new (2, 6, TRUE);

/* selected waypoints */
  wplabel1 = gtk_entry_new ();
  wp1eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (wp1eventbox), wplabel1);
  gtk_entry_set_text (GTK_ENTRY (wplabel1), "--");
  gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp1eventbox, 0, 2, 0,
			     1);

/* waypoints in range */
  wplabel2 = gtk_entry_new ();
  wp2eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (wp2eventbox), wplabel2);
  gtk_entry_set_text (GTK_ENTRY (wplabel2), "--");
  gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp2eventbox, 2, 4, 0,
			     1);

/* range in km */
  wplabel3 = gtk_entry_new ();
  wp3eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (wp3eventbox), wplabel3);
  gtk_entry_set_text (GTK_ENTRY (wplabel3), "--");
  gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp3eventbox, 0, 3, 1,
			     2);

/* gps time */
  wplabel4 = gtk_entry_new ();
  wp4eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (wp4eventbox), wplabel4);
  gtk_entry_set_text (GTK_ENTRY (wplabel4), _("n/a"));
  gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp4eventbox, 3, 6, 1,
			     2);

  wplabel5 = gtk_entry_new ();
  wp5eventbox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (wp5eventbox), wplabel5);
  gtk_entry_set_text (GTK_ENTRY (wplabel5), _("n/a"));
  gtk_table_attach_defaults (GTK_TABLE (wplabeltable), wp5eventbox, 4, 6, 0,
			     1);


  gtk_entry_set_editable (GTK_ENTRY (wplabel1), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (wplabel2), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (wplabel3), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (wplabel4), FALSE);
  gtk_entry_set_editable (GTK_ENTRY (wplabel5), FALSE);
  gtk_widget_set_usize (wplabel1, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (wplabel2, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (wplabel3, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (wplabel4, USIZE_X, USIZE_Y);
  gtk_widget_set_usize (wplabel5, USIZE_X, USIZE_Y);



#ifdef AFDAFDA
  if (pdamode)
    {
      g_snprintf (s3, sizeof (s3),
		  "<span color=\"%s\" font_family=\"Sans\"  size=\"7000\">%s %d\n%d %s %.1fkm</span>",
		  bluecolor, _("Selected:"), wptotal, wpselected, _("within"),
		  dbdistance);
    }
  else
    {
      g_snprintf (s3, sizeof (s3),
		  "<span color=\"%s\" font_family=\"Sans\"  size=\"10000\">%s %d\n%d %s %.1fkm</span>",
		  bluecolor, _("Selected:"), wptotal, wpselected, _("within"),
		  dbdistance);
    }
  gtk_label_set_markup (GTK_LABEL (wplabel), s3);
  gtk_label_set_justify (GTK_LABEL (wplabel), GTK_JUSTIFY_LEFT);
#endif

/*  create frames for labels */
  frame_target = gtk_frame_new (_("Distance to target"));
  destframe = frame_target;
  gtk_container_add (GTK_CONTAINER (frame_target), distlabel);
/*** Mod by Arms */
/*   if (!pdamode) */
/*     gtk_box_pack_start (GTK_BOX (hbox2a), frame_target, TRUE, TRUE, */
/* 			2 * PADDING); */
/*     ; */
/*   else */
/*     gtk_box_pack_start (GTK_BOX (hbox2a), frame_target, TRUE, TRUE, */
/* 			2 * PADDING); */
  if (pdamode)
    {
      if (milesflag)
	g_snprintf (s1, sizeof (s1), "[%s]", _("mi/h"));
      else if (nauticflag)
	g_snprintf (s1, sizeof (s1), "[%s]", _("knots"));
      else
	g_snprintf (s1, sizeof (s1), "[%s]", _("km/h"));
    }
  else
    {
      if (milesflag)
	g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"), _("mi/h"));
      else if (nauticflag)
	g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"), _("knots"));
      else
	g_snprintf (s1, sizeof (s1), "%s [%s]", _("Speed"), _("km/h"));
    }
  frame_speed = gtk_frame_new (s1);
  gtk_container_add (GTK_CONTAINER (frame_speed), speedlabel);
/*** Mod by Arms */
/*   if (!pdamode) */
/*     gtk_box_pack_start (GTK_BOX (hbox2), frame_speed, TRUE, TRUE, */
/* 			1 * PADDING); */

  frame_altitude = gtk_frame_new (_("Altitude"));
  gtk_container_add (GTK_CONTAINER (frame_altitude), altilabel);
/*** Mod by Arms */
/*   if (!pdamode) */
/*     gtk_box_pack_start (GTK_BOX (hbox2), frame_altitude, FALSE, TRUE, */
/* 			1 * PADDING); */

  frame_wp = gtk_frame_new (_("Waypoints"));
/*   gtk_container_add (GTK_CONTAINER (frame_wp), wplabel); */
/*** Mod by Arms */
/*   if (!pdamode) */
/*     gtk_box_pack_start (GTK_BOX (hbox2), frame_wp, FALSE, TRUE, 1 * PADDING); */

  vtable = gtk_table_new (1, 20, TRUE);
  gtk_table_attach_defaults (GTK_TABLE (vtable), frame_target, 0, 6, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (vtable), frame_speed, 6, 12, 0, 1);
  gtk_table_attach_defaults (GTK_TABLE (vtable), frame_altitude, 12, 15, 0,
			     1);
  gtk_table_attach_defaults (GTK_TABLE (vtable), frame_wp, 15, 20, 0, 1);
  gtk_box_pack_start (GTK_BOX (hbox2), vtable, TRUE, TRUE, 2 * PADDING);
  gtk_container_add (GTK_CONTAINER (frame_wp), wplabeltable);




/*    gtk_box_pack_start (GTK_BOX (vbig), hbox2, TRUE, TRUE, 2); */
/*** Lots of Mods by Arms */
  vbox = gtk_vbox_new (TRUE, 3 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox), menubar, FALSE, FALSE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox), zoomin, FALSE, FALSE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox), zoomout, FALSE, FALSE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox), hbox3, FALSE, FALSE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (hbox3), scalerlbt, TRUE, TRUE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (hbox3), scalerrbt, TRUE, TRUE, 1 * PADDING);
/*  only if we have read in waypoints we have the select target button */
/*    if (maxwp > 0) */
  gtk_box_pack_start (GTK_BOX (vbox), sel_target, FALSE, FALSE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox), downloadbt, FALSE, FALSE, 1 * PADDING);
/*   gtk_box_pack_start (GTK_BOX (vbox), importbt, FALSE, FALSE, 1 * PADDING);  */
/*   gtk_box_pack_start (GTK_BOX (vbox), loadtrackbt, FALSE, FALSE, 1 * PADDING); */
  gtk_box_pack_start (GTK_BOX (vbox), startgpsbt, FALSE, FALSE, 1 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox), setupbt, FALSE, FALSE, 1 * PADDING);
#ifndef USETELEATLAS
  gtk_box_pack_start (GTK_BOX (vbox), helpbt, FALSE, FALSE, 1 * PADDING);
#else
  gtk_box_pack_start (GTK_BOX (vbox), navibt, FALSE, FALSE, 1 * PADDING);
#endif
  gtk_box_pack_start (GTK_BOX (vbox), quit, FALSE, FALSE, 1 * PADDING);
  hboxlow = vbox2 = NULL;
  if (!extrawinmenu)
    {
      vbox2 = gtk_vbox_new (FALSE, 0 * PADDING);
      gtk_box_pack_start (GTK_BOX (vbox2), vbox, TRUE, TRUE, 1 * PADDING);
      gtk_box_pack_start (GTK_BOX (vbox2), frame_toogles, TRUE, TRUE,
			  1 * PADDING);
      gtk_box_pack_start (GTK_BOX (vbox2), frame_maptype, TRUE, TRUE,
			  1 * PADDING);
      if (SMALLMENU == 0)
	gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET (drawing_miniimage),
			    TRUE, FALSE, 0 * PADDING);
    }
  else
    {
      vboxlow = gtk_vbox_new (FALSE, 0 * PADDING);
      hboxlow = gtk_hbox_new (FALSE, 0 * PADDING);
      gtk_box_pack_start (GTK_BOX (vboxlow), frame_toogles, TRUE, TRUE,
			  1 * PADDING);
      gtk_box_pack_start (GTK_BOX (vboxlow), frame_maptype, TRUE, TRUE,
			  1 * PADDING);
      if (SMALLMENU == 0)
	gtk_box_pack_start (GTK_BOX (vboxlow), GTK_WIDGET (drawing_miniimage),
			    TRUE, FALSE, 0 * PADDING);
      gtk_box_pack_start (GTK_BOX (hboxlow), vbox, TRUE, TRUE, 1 * PADDING);
      gtk_box_pack_start (GTK_BOX (hboxlow), vboxlow, TRUE, TRUE,
			  1 * PADDING);

    }


  if (havespeechout)
    gtk_box_pack_start (GTK_BOX (vbox4), mutebt, FALSE, FALSE, 0 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox4), poi_draw_bt, FALSE, FALSE, 0 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox4), streets_draw_bt, FALSE, FALSE, 0 * PADDING);
  if (usesql)
    gtk_box_pack_start (GTK_BOX (vbox4), sqlbt, FALSE, FALSE, 0 * PADDING);
  /*    if (maxwp > 0) */
  gtk_box_pack_start (GTK_BOX (vbox4), wpbt, FALSE, FALSE, 0 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox4), posbt, FALSE, FALSE, 0 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox4), trackbt, FALSE, FALSE, 0 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox4), bestmapbt, FALSE, FALSE, 0 * PADDING);
  gtk_box_pack_start (GTK_BOX (vbox4), savetrackbt, FALSE, FALSE,
		      0 * PADDING);
//KCFX
/*   if (!pdamode) */
  {
    adj = gtk_adjustment_new (12, 0, 29, 1, 4, 0.034);
    scaler = gtk_hscale_new (GTK_ADJUSTMENT (adj));
    gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
			GTK_SIGNAL_FUNC (scaler_cb), NULL);
    gtk_scale_set_draw_value (GTK_SCALE (scaler), FALSE);
  }

  if (pdamode)
    table1 = gtk_table_new (5, 3, FALSE);
  else
    {
      if (SMALLMENU)
	table1 = gtk_table_new (4, 3, FALSE);
      else
	table1 = gtk_table_new (8, 2, FALSE);
    }
  lf1 = gtk_frame_new (_("Latitude"));
  lf2 = gtk_frame_new (_("Longitude"));
  lf3 = gtk_frame_new (_("Map file"));
  lf4 = gtk_frame_new (_("Map scale"));
  lf5 = gtk_frame_new (_("Heading"));
  lf6 = gtk_frame_new (_("Bearing"));
  lf7 = gtk_frame_new (_("Time at Dest."));
  lf8 = gtk_frame_new (_("Pref. scale"));

  etch = !etch;
  etch_cb (NULL, 0);
  l1 = gtk_label_new (_("000,00000N"));
  gtk_container_add (GTK_CONTAINER (lf1), l1);
  l2 = gtk_label_new (_("000,00000E"));
  gtk_container_add (GTK_CONTAINER (lf2), l2);
  l3 = gtk_label_new (_("---"));
  gtk_container_add (GTK_CONTAINER (lf3), l3);
  l4 = gtk_label_new (_("---"));
  gtk_container_add (GTK_CONTAINER (lf4), l4);
  l5 = gtk_label_new (_("0000"));
  gtk_container_add (GTK_CONTAINER (lf5), l5);
  l6 = gtk_label_new (_("0000"));
  gtk_container_add (GTK_CONTAINER (lf6), l6);
  l7 = gtk_label_new (_("---"));
  gtk_container_add (GTK_CONTAINER (lf7), l7);
  l8 = gtk_label_new (_("---"));
  gtk_container_add (GTK_CONTAINER (lf8), l8);

  if (pdamode)
    {
      gtk_table_attach_defaults (GTK_TABLE (table1), lf6, 0, 1, 0, 1);
      gtk_table_attach_defaults (GTK_TABLE (table1), lf5, 1, 2, 0, 1);
      gtk_table_attach_defaults (GTK_TABLE (table1), lf4, 2, 3, 0, 1);

      gtk_table_attach_defaults (GTK_TABLE (table1), lf1, 0, 1, 1, 2);
      gtk_table_attach_defaults (GTK_TABLE (table1), lf2, 1, 2, 1, 2);
      gtk_table_attach_defaults (GTK_TABLE (table1), lf8, 2, 3, 1, 2);

      gtk_table_attach_defaults (GTK_TABLE (table1), lf7, 0, 1, 2, 3);
      gtk_table_attach_defaults (GTK_TABLE (table1), lf3, 1, 3, 2, 3);
//KCFX
      gtk_table_attach_defaults (GTK_TABLE (table1), scaler, 0, 3, 3, 4);
      gtk_table_attach_defaults (GTK_TABLE (table1), status, 0, 3, 4, 5);
    }
  else
    {
      if (SMALLMENU)
	{
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf6, 0, 1, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf5, 1, 2, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf1, 2, 3, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf2, 3, 4, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf7, 0, 1, 1, 2);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf3, 1, 2, 1, 2);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf4, 2, 3, 1, 2);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf8, 3, 4, 1, 2);
	  gtk_table_attach_defaults (GTK_TABLE (table1), status, 0, 4, 3, 4);
	  gtk_table_attach_defaults (GTK_TABLE (table1), scaler, 0, 4, 2, 3);
	}
      else
	{
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf6, 0, 1, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf5, 1, 2, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf1, 2, 3, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf2, 3, 4, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf7, 4, 5, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf3, 5, 6, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf4, 6, 7, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), lf8, 7, 8, 0, 1);
	  gtk_table_attach_defaults (GTK_TABLE (table1), status, 0, 4, 1, 2);
	  gtk_table_attach_defaults (GTK_TABLE (table1), scaler, 4, 8, 1, 2);
	}
    }
/*    gtk_box_pack_start (GTK_BOX (vbig), table1, FALSE, FALSE, 1); */
/*  all position calculations are made in the expose callback */
/*   if (!pdamode) */
  gtk_signal_connect (GTK_OBJECT (drawing_area),
		      "expose_event", GTK_SIGNAL_FUNC (expose_cb), NULL);
  if (havebattery)
    gtk_signal_connect (GTK_OBJECT (drawing_battery),
			"expose_event",
			GTK_SIGNAL_FUNC (expose_display_battery), NULL);
  if (havetemperature)
    gtk_signal_connect (GTK_OBJECT (drawing_temp),
			"expose_event",
			GTK_SIGNAL_FUNC (expose_display_battery), NULL);

  if (!pdamode)
    {
      if (extrawinmenu)
	{
	  menuwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
/*       gdk_window_lower((GdkWindow *)menuwin); */

	  gtk_window_set_title (GTK_WINDOW (menuwin), _("Menu"));
/*** Mod by Arms */
	  gtk_container_set_border_width (GTK_CONTAINER (menuwin),
					  2 * PADDING);
	  gtk_container_add (GTK_CONTAINER (menuwin), hboxlow);
	  gtk_signal_connect (GTK_OBJECT (menuwin), "delete_event",
			      GTK_SIGNAL_FUNC (quit_program), NULL);
	  menuwin2 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
/*       gdk_window_lower((GdkWindow *)menuwin2); */
	  gtk_window_set_title (GTK_WINDOW (menuwin2), _("Status"));
/*** Mod by Arms */
	  gtk_container_set_border_width (GTK_CONTAINER (menuwin2),
					  2 * PADDING);
	  vbig1 = gtk_vbox_new (FALSE, 2);
	  gtk_container_add (GTK_CONTAINER (menuwin2), vbig1);
/*** Mod by Arms */
	  gtk_box_pack_start (GTK_BOX (vbig1), hbox2, TRUE, TRUE,
			      2 * PADDING);
	  if (pdamode)
	    gtk_box_pack_start (GTK_BOX (vbig1), hbox2a, TRUE, TRUE,
				2 * PADDING);
/*** Mod by Arms */
	  gtk_box_pack_start (GTK_BOX (vbig1), table1, TRUE, TRUE,
			      2 * PADDING);
	  gtk_signal_connect (GTK_OBJECT (menuwin2), "delete_event",
			      GTK_SIGNAL_FUNC (quit_program), NULL);
	}
      else
	{
/*** Mod by Arms */
//        gtk_box_pack_start (GTK_BOX (vmenubig), menubar, TRUE, TRUE,
//                            2 * PADDING);
//        gtk_box_pack_start (GTK_BOX (vmenubig), hbig, TRUE, TRUE,
//                            2 * PADDING);

	  gtk_box_pack_start (GTK_BOX (hbig), vbox2, TRUE, TRUE, 1 * PADDING);
/*** Mod by Arms */
	  gtk_box_pack_start (GTK_BOX (vbig), hbox2, TRUE, TRUE, 2 * PADDING);
/*** Mod by Arms */
	  gtk_box_pack_start (GTK_BOX (vbig), table1, FALSE, FALSE,
			      1 * PADDING);
	}
    }

/*   if pdamode is set, we use gtk-notebook add arrange the elements */
/*   in pdamode, extrawinmenu is also set! */
  mainnotebook = NULL;
  if (pdamode)
    {
      GtkWidget *l1, *l2, *l3;
      l1 = gtk_label_new (_("Map"));
      l2 = gtk_label_new (_("Menu"));
      l3 = gtk_label_new (_("Status"));
//KCFX
      vbig1 = gtk_vbox_new (FALSE, 2);
//      gtk_container_add (GTK_CONTAINER (menuwin2), vbig1);
      gtk_box_pack_start (GTK_BOX (vbig1), hbox2, TRUE, TRUE, 2 * PADDING);
      gtk_box_pack_start (GTK_BOX (vbig1), hbox2a, TRUE, TRUE, 2 * PADDING);
      gtk_box_pack_start (GTK_BOX (vbig1), hbox2b, TRUE, TRUE, 2 * PADDING);
      gtk_box_pack_start (GTK_BOX (hbox2b), frame_speed, TRUE, TRUE,
			  1 * PADDING);
      gtk_box_pack_start (GTK_BOX (hbox2b), frame_altitude, TRUE, TRUE,
			  1 * PADDING);
//KCFX          
      gtk_box_pack_start (GTK_BOX (hbox2a), frame_wp, TRUE, TRUE,
			  1 * PADDING);

      gtk_box_pack_start (GTK_BOX (vbig1), table1, TRUE, TRUE, 2 * PADDING);

      mainnotebook = gtk_notebook_new ();
      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (mainnotebook), GTK_POS_TOP);
      gtk_box_pack_start (GTK_BOX (hbig), vbig, TRUE, TRUE, 1 * PADDING);
      gtk_container_add (GTK_CONTAINER (mainwindow), mainnotebook);
      gtk_widget_show_all (hboxlow);
      gtk_widget_show_all (vbig1);
      gtk_widget_show_all (hbig);
      gtk_widget_show_all (vbig);
      gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), hbig, l1);
      gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), hboxlow, l2);
      gtk_notebook_append_page (GTK_NOTEBOOK (mainnotebook), vbig1, l3);
      gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);
      gtk_widget_show_all (mainnotebook);
    }
  else
    {
      gtk_box_pack_start (GTK_BOX (hbig), vbig, TRUE, TRUE, 1 * PADDING);
      //      gtk_container_add (GTK_CONTAINER (mainwindow), vmenubig);
      gtk_container_add (GTK_CONTAINER (mainwindow), hbig);
    }

/*** Mod by Arms */
/* This one should position the windows in the corners, */
/* so that gpsdrive can be run w/o a xwm (stand-alone mode) */
/* With a xwm, you should be able to reposition the */
/* windows afterwards... */
  if (extrawinmenu && SMALLMENU && !pdamode)
    {
      reqptr = &requ;
      gtk_widget_size_request (GTK_WIDGET (menuwin2), reqptr);
      gtk_widget_set_uposition (GTK_WIDGET (menuwin2), gdk_screen_width () - requ.width, gdk_screen_height () - requ.height);	/* rechts unten */
      gtk_widget_size_request (GTK_WIDGET (mainwindow), reqptr);
      gtk_widget_set_uposition (GTK_WIDGET (mainwindow), gdk_screen_width () - requ.width, 0);	/* rechts oben */
      gtk_widget_size_request (GTK_WIDGET (menuwin), reqptr);
      gtk_widget_set_uposition (GTK_WIDGET (menuwin), 0, 0);	/* links oben */
    }

/*  Now show all Widgets */
//KCFX
  if ((extrawinmenu) && (!pdamode))
    {
      gtk_widget_show_all (menuwin);
      gtk_widget_show_all (menuwin2);

    }
  gtk_widget_show_all (mainwindow);

  mainwindow_icon_pixbuf = create_pixbuf ("gpsicon.png");
  if (mainwindow_icon_pixbuf)
    {
      gtk_window_set_icon (GTK_WINDOW (mainwindow), mainwindow_icon_pixbuf);
      gdk_pixbuf_unref (mainwindow_icon_pixbuf);
    }


/*  now we know the drawables */
  if (pdamode)
    gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);

  drawable = gdk_pixmap_new (mainwindow->window, SCREEN_X, SCREEN_Y, -1);
/* gtk_widget_set_double_buffered(drawable, FALSE);  */
/*    drawable = drawing_area->window; */
  if (pdamode)
    gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 2);

  drawable_sats = drawing_sats->window;
  drawable_bearing = drawing_bearing->window;
/*  gtk_widget_set_double_buffered(GTK_WIDGET(drawable_sats), TRUE);   */
/*  gtk_widget_set_double_buffered(GTK_WIDGET(drawable_bearing), TRUE);   */

//KCFX
  // if (!pdamode) 
  kontext = gdk_gc_new (mainwindow->window);
  //  else 
  //    kontext = gdk_gc_new (mainnotebook->window); 

  gdk_gc_set_clip_origin (kontext, 0, 0);
  rectangle.width = SCREEN_X;
  rectangle.height = SCREEN_Y;

  gdk_gc_set_clip_rectangle (kontext, &rectangle);
  cmap = gdk_colormap_get_system ();
  gdk_color_alloc (cmap, &red);
  gdk_color_alloc (cmap, &black);
  gdk_color_alloc (cmap, &blue);
  gdk_color_alloc (cmap, &nightcolor);
  gdk_color_alloc (cmap, &lcd);
  gdk_color_alloc (cmap, &lcd2);
  gdk_color_alloc (cmap, &green);
  gdk_color_alloc (cmap, &green2);
  gdk_color_alloc (cmap, &white);
  gdk_color_alloc (cmap, &mygray);
  gdk_color_alloc (cmap, &yellow);
  gdk_color_alloc (cmap, &darkgrey);
  gdk_color_alloc (cmap, &grey);
  gdk_color_alloc (cmap, &textback);
  gdk_color_alloc (cmap, &textbacknew);
  gdk_color_alloc (cmap, &orange2);
  gdk_color_alloc (cmap, &orange);
  gdk_color_alloc (cmap, &trackcolorv);

/* fill window with color */
  gdk_gc_set_function (kontext, GDK_COPY);
  gdk_gc_set_foreground (kontext, &lcd2);
  gdk_draw_rectangle (drawing_area->window, kontext, 1, 0, 0, SCREEN_X,
		      SCREEN_Y);
  {
    GtkStyle *style;
    style = gtk_rc_get_style (mainwindow);
    defaultcolor = style->bg[GTK_STATE_NORMAL];
  }

  if (pdamode)
    gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 1);
  if (SMALLMENU == 0)
    gdk_window_set_cursor (drawing_miniimage->window, cursor);

  cursor = gdk_cursor_new (GDK_CROSS);
  if (pdamode)
    gtk_notebook_set_page (GTK_NOTEBOOK (mainnotebook), 0);
  gdk_window_set_cursor (drawing_area->window, cursor);

/*  Tooltips */
  temptooltips = tooltips = gtk_tooltips_new ();

  if (havetemperature)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), tempeventbox,
			  cputempstring, NULL);
  if (havebattery)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), batteventbox,
			  batstring, NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), sateventbox,
			_
			("Click here to switch betwen satetellite level and satellite position display. A rotating globe is shown in simulation mode"),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), satslabel1eventbox,
			_("Number of used satellites/satellites in view"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), satslabel2eventbox,
			_("EPE (Estimated Precision Error), if available"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), satslabel3eventbox,
			_
			("PDOP (Position Dilution Of Precision). PDOP less than 4 gives the best accuracy, between 4 and 8 gives acceptable accuracy and greater than 8 gives unacceptable poor accuracy. "),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), compasseventbox,
			_
			("On top of the compass you see the direction to which you move. The pointer shows the target direction on the compass."),
			NULL);
  wi = NULL;
  wi = gtk_item_factory_get_item (item_factory, N_("/Misc. Menu"));
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wi,
			_
			("Here you find extra functions for maps, tracks and messages"),
			NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), downloadbt,
			_("Download map from Internet"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), quit,
			_("Leave the program"), NULL);
  if (havespeechout)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), mutebt,
			  _("Disable output of speech"), NULL);
  if (usesql)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), sqlbt,
			  _("Use SQL server for waypoints"), NULL);
/*    if (maxwp > 0) */
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wpbt,
			_("Show waypoints on the map"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), posbt,
			_
			("Turn position mode on. You can move on the map with the left mouse button click. Clicking near the border switches to the proximate map."),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), trackbt,
			_("Show tracking on the map"), NULL);
/*   gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), importbt, */
/* 			_("Let you import and calibrate your own map"), NULL); */
#ifndef USETELEATLAS
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), helpbt,
			_("Opens the help window"), NULL);
#else
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), navibt,
			_("Navigation menu. Enter here your destination."),
			NULL);
#endif
  if (haveNMEA)
    {
      gtk_button_set_label (GTK_BUTTON (startgpsbt), _("Stop GPSD"));
      gtk_tooltips_set_tip (GTK_TOOLTIPS (temptooltips), startgpsbt,
			    _("Stop GPSD and switch to simulation mode"),
			    NULL);
      gpson = TRUE;
    }
  else
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), startgpsbt,
			  _("Starts GPSD for NMEA mode"), NULL);

  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), setupbt,
			_("Settings for GpsDrive"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), zoomin,
			_("Zoom into the current map"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), zoomout,
			_("Zooms out off the current map"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scalerlbt,
			_("Select the next more detailed map"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scalerrbt,
			_("Select the next less detailed map"), NULL);
/*    if (maxwp > 0) */
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips),
			sel_target,
			_
			("Select here a destination from the waypoint list"),
			NULL);
  if (scaler)
    gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), scaler,
			  _("Select the map scale of avail. maps."), NULL);
/*   gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), loadtrackbt, */
/* 			_("Load and display a previous stored track file"), */
/* 			NULL); */
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), bestmapbt,
			_("Always select the most detailed map available"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), poi_draw_bt,
			_("Draw Point Of Interrests found in mySQL"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), streets_draw_bt,
			_("Draw Streets found in mySQL"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), savetrackbt,
			_("Save the track to given filename at program exit"),
			NULL);


  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp1eventbox,
			_
			("Number of waypoints selected from SQL server"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp2eventbox,
			_
			("Number of selected waypoints, which are in range"),
			NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp3eventbox,
			_
			("Range for waypoint selection in kilometers"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp4eventbox,
			_
			("This shows the time from your GPS receiver"), NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tooltips), wp5eventbox,
			_
			("Number of mobile targets within timeframe/total received from friendsserver"),
			NULL);


/*    gtk_tooltips_set_tip(GTK_TOOLTIPS(tooltips),,_(""),NULL); */
  gtk_tooltips_set_delay (GTK_TOOLTIPS (tooltips), 1000);
  g_strlcpy (mapfilename, "***", sizeof (mapfilename));
/*  set the timers */
  if (haveserial)
    timerto =
      gtk_timeout_add (TIMERSERIAL, (GtkFunction) get_position_data_cb, NULL);
  else
    timerto =
      gtk_timeout_add (TIMER, (GtkFunction) get_position_data_cb, NULL);
  gtk_timeout_add (WATCHWPTIMER, (GtkFunction) watchwp_cb, NULL);


  redrawtimeout =
    gtk_timeout_add (100, (GtkFunction) calldrawmarker_cb, NULL);

/*  if we started in simulator mode we have a little move roboter */

  if (simmode)
    simpos_timeout = gtk_timeout_add (300, (GtkFunction) simulated_pos, 0);
  if (nmeaout)
    gtk_timeout_add (1000, (GtkFunction) write_nmea_cb, NULL);
  gtk_timeout_add (10000, (GtkFunction) testconfig_cb, 0);
  gtk_timeout_add (600000, (GtkFunction) speech_saytime_cb, 0);
  gtk_timeout_add (1000, (GtkFunction) storetrack_cb, 0);
  gtk_timeout_add (10000, (GtkFunction) masteragent_cb, 0);
  gtk_timeout_add (15000, (GtkFunction) getsqldata, 0);
  gtk_timeout_add (2000, (GtkFunction) nav_doit, NULL);
  if (havebattery)
    gtk_timeout_add (5000, (GtkFunction) expose_display_battery, NULL);

  gtk_timeout_add (15000, (GtkFunction) friendsagent_cb, 0);

  if (havespeechout)
    {
      speech_saytime_cb (NULL, 1);
      gtk_timeout_add (SPEECHOUTINTERVAL, (GtkFunction) speech_out_cb, 0);
    }

  /*  To set the checkboxes to the right values */
  bestmap_cb (NULL, 0);
  drawgrid_cb (NULL, 0);
  poi_draw_cb (NULL, 0);
  streets_draw_cb (NULL, 0);
  needtosave = FALSE;

  poi_init ();
  streets_init ();


  /*  Mainloop */

  gtk_main ();


  g_timer_destroy (timer);
  writeconfig ();
  gdk_pixbuf_unref (friendspixbuf);


  gpsserialquit ();
  unlink ("/tmp/cammain.pid");
  unlink ("/tmp/gpsdrivetext.out");
  unlink ("/tmp/gpsdrivepos");
  if (savetrack)
    savetrackfile (2);
  sqlend ();
  free (friends);
  free (fserver);
  g_free (routelist);
  if (kismetsock != -1)
    close (kismetsock);
  if (sock != -1)
    close (sock);
  if (sockfd != -1)
    close (sockfd);
  speech_out_close ();
  cleanup_nasa_mapfile ();
  fprintf (stderr, _("\n\nThank you for using GpsDrive!\n\n"));
  return 0;
}
