/*  on a USR2 signal, re-start the GPS connection  */

void
usr2handler (int sig)
{
  g_print ("\ngot SIGUSR2\n");
  initgps ();
}


