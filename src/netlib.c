/*
  $Log$
  Revision 1.4  2005/04/20 23:33:49  tweety
  reformatted source code with anjuta
  So now we have new indentations

  Revision 1.3  2005/04/13 19:58:31  tweety
  renew indentation to 4 spaces + tabstop=8

  Revision 1.2  2005/04/10 21:50:50  tweety
  reformatting c-sources
	
  Revision 1.1.1.1  2004/12/23 16:03:24  commiter
  Initial import, straight from 2.10pre2 tar.gz archive

  Revision 1.4  2003/01/23 14:05:35  ganter
  added greek translation
  added geocache scripts
  added geocache icon
  improved search for libmysqlclient.so

  Revision 1.3  2002/07/14 18:22:19  ganter
  v1.25pre1

  Revision 1.2  2002/04/06 00:36:01  ganter
  changing filelist

*/

#include "config.h"
#include <stdlib.h>
#include <string.h>

#if defined (HAVE_STRINGS_H)
#include <strings.h>
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>

#if defined (HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif

#include "gpsd.h"


#if !defined (INADDR_NONE)
#define INADDR_NONE   ((in_addr_t)-1)
#endif

static char mbuf[128];

int
passivesock (char *service, char *protocol, int qlen)
{
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;
	int s, type, i;

	bzero ((char *) &sin, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if ((pse = getservbyname (service, protocol)))
		sin.sin_port = htons (ntohs ((u_short) pse->s_port));
	else if ((sin.sin_port = htons ((u_short) atoi (service))) == 0)
	{
		sprintf (mbuf, "Can't get \"%s\" service entry.\n", service);
		errexit (mbuf);
	}
	if ((ppe = getprotobyname (protocol)) == 0)
	{
		sprintf (mbuf, "Can't get \"%s\" protocol entry.\n",
			 protocol);
		errexit (mbuf);
	}
	if (strcmp (protocol, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	s = socket (PF_INET, type, ppe->p_proto);
	if (s < 0)
		errexit ("Can't create socket:");

	i = 1;
	if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i)) < 0)
	{
		sprintf (mbuf, "Can't set SO_REUSEADDR, port %s", service);
		errexit (mbuf);
	}

	if (bind (s, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	{
		sprintf (mbuf, "Can't bind to port %s", service);
		errexit (mbuf);
	}
	if (type == SOCK_STREAM && listen (s, qlen) < 0)
	{
		sprintf (mbuf, "Can't listen on %s port:", service);
		errexit (mbuf);
	}
	return s;
}

int
passiveTCP (char *service, int qlen)
{
	return passivesock (service, "tcp", qlen);
}


int
connectsock (char *host, char *service, char *protocol)
{
	struct hostent *phe;
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in sin;
	int s, type;

	bzero ((char *) &sin, sizeof (sin));
	sin.sin_family = AF_INET;

	if ((pse = getservbyname (service, protocol)))
		sin.sin_port = htons (ntohs ((u_short) pse->s_port));
	else if ((sin.sin_port = htons ((u_short) atoi (service))) == 0)
	{
		sprintf (mbuf, "Can't get \"%s\" service entry.\n", service);
		errexit (mbuf);
	}
	if ((phe = gethostbyname (host)))
		bcopy (phe->h_addr, (char *) &sin.sin_addr, phe->h_length);
	else if ((sin.sin_addr.s_addr = inet_addr (host)) == INADDR_NONE)
	{
		sprintf (mbuf, "Can't get host entry: \"%s\".\n", host);
		errexit (mbuf);
	}
	if ((ppe = getprotobyname (protocol)) == 0)
	{
		sprintf (mbuf, "Can't get \"%s\" protocol entry.\n",
			 protocol);
		errexit (mbuf);
	}
	if (strcmp (protocol, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	s = socket (PF_INET, type, ppe->p_proto);
	if (s < 0)
		errexit ("Can't create socket:");

	if (connect (s, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	{
		sprintf (mbuf, "Can't connect to %s.%s", host, service);
		errexit (mbuf);
	}
	return s;
}

int
connectTCP (char *host, char *service)
{
	return connectsock (host, service, "tcp");
}
