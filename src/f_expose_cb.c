
/* Copy Image from loaded map */
gint
expose_cb (GtkWidget * widget, guint * datum)
{
  gint x, y, i, oldxoff, oldyoff, xoffmax, yoffmax, ok, okcount;
  gdouble tx, ty, lastangle;
  gchar name[40], s1[40], *tn;
/*    g_print("\nexpose_cb %d",exposecounter++);   */

/*   fprintf (stderr, "lat: %f long: %f\n", current_lat, current_long); */
  if (exposed && pdamode)
    return TRUE;



  errortextmode = FALSE;
  if (!importactive)
    {
/*  We don't need to draw anything if there is no map yet */
      if (!maploaded)
	{
	  display_status (_("No map available for this position!"));
/*        return TRUE; */
	}

      if (posmode)
	{
	  current_long = posmode_x;
	  current_lat = posmode_y;
	}


/*  get pos for current position */
      calcxy (&posx, &posy, current_long, current_lat, zoom);
/*  do this because calcxy already substracted xoff and yoff */
      posx = posx + xoff;
      posy = posy + yoff;
/*  Calculate Angle to destination */
      tx =
	(2 * R * M_PI / 360) * cos (M_PI * current_lat / 180.0) *
	(target_long - current_long);
      ty = (2 * R * M_PI / 360) * (target_lat - current_lat);
      lastangle = angle_to_destination;
      angle_to_destination = atan (tx / ty);
/*        g_print ("\ntx: %f, ty:%f angle_to_dest: %f", tx, ty, */
/*  	       angle_to_destination); */
      if (!finite (angle_to_destination))
	angle_to_destination = lastangle;
      else
	{
/*  correct the value to be < 2*PI */
	  if (ty < 0)
	    angle_to_destination = M_PI + angle_to_destination;
	  if (angle_to_destination >= (2 * M_PI))
	    angle_to_destination -= 2 * M_PI;
	  if (angle_to_destination < 0)
	    angle_to_destination += 2 * M_PI;
	}
      if (mydebug)
	g_print ("\nAngle_To_Destination: %.0fÂŽ°",
		 angle_to_destination * 180 / M_PI);

      if (havefriends && targetname[0] == '*')
	for (i = 0; i < maxfriends; i++)
	  {
	    g_strlcpy (name, "*", sizeof (name));

	    g_strlcat (name, (friends + i)->name, sizeof (name));
	    tn = g_strdelimit (name, "_", ' ');
	    if ((strcmp (targetname, tn)) == 0)
	      {
		target_lat = g_strtod ((friends + i)->lat, NULL);
		target_long = g_strtod ((friends + i)->longi, NULL);
	      }
	  }

/*  Calculate distance to destination */
      dist = calcdist (target_long, target_lat);
/*  correct the shift of the map */
      oldxoff = xoff;
      oldyoff = yoff;
/*  now we test if the marker fits into the map and set the shift of the 
little SCREEN_XxSCREEN_Y region in relation to the real 1280x1024 map */
      okcount = 0;
      do
	{
	  ok = TRUE;
	  okcount++;
	  x = posx - xoff;
	  y = posy - yoff;

	  if (x < borderlimit)
	    xoff -= 2 * borderlimit;
	  if (x > (SCREEN_X - borderlimit))
	    xoff += 2 * borderlimit;
	  if (y < borderlimit)
	    yoff -= 2 * borderlimit;
	  if (y > (SCREEN_Y - borderlimit))
	    yoff += 2 * borderlimit;

	  if (x < borderlimit)
	    ok = FALSE;
	  if (x > (SCREEN_X - borderlimit))
	    ok = FALSE;
	  if (y < borderlimit)
	    ok = FALSE;
	  if (y > (SCREEN_Y - borderlimit))
	    ok = FALSE;
	  if (okcount > 20000)
	    {
	      g_print ("\nloop detected, please report!\n");
	      ok = TRUE;
	    }
	}
      while (!ok);

      xoffmax = (640 * zoom) - SCREEN_X_2;
      yoffmax = (512 * zoom) - SCREEN_Y_2;
      if (xoff > xoffmax)
	xoff = xoffmax;
      if (xoff < -xoffmax)
	xoff = -xoffmax;
      if (yoff > yoffmax)
	yoff = yoffmax;
      if (yoff < -yoffmax)
	yoff = -yoffmax;

/*       if ((xoff - 640) / zoom < -1280) */
/* 	xoff = -1280 * zoom + 640; */
/*       if ((xoff + 640) / zoom > 1280) */
/* 	xoff = 1280 * zoom - 640; */

/*       if ((yoff - 512) / zoom < -1024) */
/* 	yoff = -1024 * zoom + 512; */
/*       if ((yoff + 512) / zoom > 1024) */
/* 	yoff = 1024 * zoom - 512; */

/*  we only need to create a new region if the shift is not changed */
      if ((oldxoff != xoff) || (oldyoff != yoff))
	iszoomed = FALSE;

      if (mydebug)
	g_print ("\nx: %d  xoff: %d oldxoff: %d Zoom: %d xoffmax: %d\n", x,
		 xoff, oldxoff, zoom, xoffmax);
      if (mydebug)
	g_print ("\ny: %d  yoff: %d oldyoff: %d Zoom: %d yoffmax: %d\n", y,
		 yoff, oldyoff, zoom, yoffmax);
      posx = posx - xoff;
      posy = posy - yoff;
    }

/*       if (scroll) */
/*       { */
/* 	  xoff=posx+640; */
/* 	  posx=320; */
/* 	  yoff=posy+512; */
/* 	  posx=256; */
/*       } */


/*  zoom from to 1280x1024 map to the SCREEN_XxSCREEN_Y region */
  if (!iszoomed)
    {
      rebuildtracklist ();

      if (tempimage == NULL)
	tempimage = gdk_pixbuf_new (GDK_COLORSPACE_RGB, 0, 8, 1280, 1024);

      if (maploaded)
	{
/* 	  g_print ("\nmap loaded, do gdk_pixbuf_scale\n");  */
	  gdk_pixbuf_scale (image, tempimage, 0, 0, 1280, 1024,
			    640 - xoff - 640 * zoom,
			    512 - yoff - 512 * zoom, zoom, zoom,
			    GDK_INTERP_BILINEAR);

/*       image=gdk_pixbuf_scale_simple(tempimage,640 - xoff - 640 * zoom,
 *   			  512 - yoff - 512 * zoom, 
 *   			  GDK_INTERP_BILINEAR);
 */

	}

      if (debug)
	g_print ("\nmap zoomed!\n");
      iszoomed = TRUE;
      expose_mini_cb (NULL, 0);

    }

  gdk_draw_pixbuf (drawable, kontext, tempimage,
		   640 - SCREEN_X_2,
		   512 - SCREEN_Y_2, 0, 0,
		   SCREEN_X, SCREEN_Y, GDK_RGB_DITHER_NONE, 0, 0);

  if ((!disableisnight) && (!downloadwindowactive))
    {
      if ((nightmode == 1) || ((nightmode == 2) && isnight))
	{
	  gdk_gc_set_function (kontext, GDK_AND);
	  gdk_gc_set_foreground (kontext, &nightcolor);
	  gdk_draw_rectangle (drawable, kontext, 1, 0, 0, SCREEN_X, SCREEN_Y);
	  gdk_gc_set_function (kontext, GDK_COPY);
	}
    }

  if (sqlflag)
    {
      g_snprintf (s1, sizeof (s1), "%d", wptotal);
      gtk_entry_set_text (GTK_ENTRY (wplabel1), s1);
      g_snprintf (s1, sizeof (s1), "%d", wpselected);
      gtk_entry_set_text (GTK_ENTRY (wplabel2), s1);
      if (dbusedist)
	g_snprintf (s1, sizeof (s1), "%.1f km", dbdistance);
      else
	g_snprintf (s1, sizeof (s1), _("unused"));
      gtk_entry_set_text (GTK_ENTRY (wplabel3), s1);
      gtk_entry_set_text (GTK_ENTRY (wplabel4), loctime);
    }
  else
    {
      g_strlcpy (s1, _("n/a"), sizeof (s1));
      gtk_entry_set_text (GTK_ENTRY (wplabel1), s1);
      gtk_entry_set_text (GTK_ENTRY (wplabel2), s1);
      gtk_entry_set_text (GTK_ENTRY (wplabel3), s1);
      gtk_entry_set_text (GTK_ENTRY (wplabel4), loctime);
    }
  if (havefriends)
    g_snprintf (s1, sizeof (s1), "%d/%d", actualfriends, maxfriends);
  else
    g_strlcpy (s1, _("n/a"), sizeof (s1));
  gtk_entry_set_text (GTK_ENTRY (wplabel5), s1);

  drawmarker (0, 0);

  gdk_draw_pixmap (drawing_area->window, kontext, drawable, 0,
		   0, 0, 0, SCREEN_X, SCREEN_Y);
  exposed = TRUE;
  return TRUE;
}

