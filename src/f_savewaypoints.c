
/*  load the waypoint from way.txt, if success we display the sel target window */
void
savewaypoints ()
{
  gchar mappath[400], la[20], lo[20];
  FILE *st;
  gint i, e;


  g_strlcpy (mappath, homedir, sizeof (mappath));
  g_strlcat (mappath, activewpfile, sizeof (mappath));

  st = fopen (mappath, "w+");
  if (st == NULL)
    {
      perror (mappath);
    }
  else
    {
      for (i = 0; i < maxwp; i++)
	{
	  g_snprintf (la, sizeof (la), "%10.6f", (wayp + i)->lat);
	  g_snprintf (lo, sizeof (lo), "%10.6f", (wayp + i)->longitude);
	  g_strdelimit (la, ",", '.');
	  g_strdelimit (lo, ",", '.');

	  e =
	    fprintf (st, "%-22s %10s %11s %s\n", (wayp + i)->name, la, lo,
		     (wayp + i)->typ);
	}
      fclose (st);
    }

}

