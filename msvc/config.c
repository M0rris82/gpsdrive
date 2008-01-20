


#include "config.h"

char gpsdr_datadir[100] = "C:\\Dokumente und Einstellungen\\ulfl\\Eigene Dateien\\gpsdrive\\svn.gpsdrive.cc\\msvc\\datadir";
//char gpsdr_datadir[100] = "C:\\Dokumente und Einstellungen\\ulfl\\Eigene Dateien\\gpsdrive\\svn.gpsdrive.cc\\data";
char gpsdr_localedir[100] = "./";
char gpsdr_gettext_package[] = "gnometext";

#if defined(_WIN32) && !defined(__MINGW32__)
/* copied from GPL'ed Wireshark file gtk/sctp_graph.c */
/* replacement of Unix rint() for Windows */
int rint (double x)
{
	char *buf;
	int i,dec,sig;

	buf = _fcvt(x, 0, &dec, &sig);
	i = atoi(buf);
	if(sig == 1) {
		i = i * -1;
	}
	return(i);
}
#endif
