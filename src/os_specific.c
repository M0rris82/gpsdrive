/***********************************************************************

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

***********************************************************************/

#include "config.h"
#include "os_specific.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#endif

void socket_close(SOCKET_TYPE sock)
{
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

void socket_nonblocking(SOCKET_TYPE sock)
{
#ifdef _WIN32
  u_long iMode = 1;
  ioctlsocket(sock, FIONBIO, &iMode);
#else
  int opts;

  opts = fcntl (sock, F_GETFL);
  if (opts < 0)
    {
      perror ("fcntl(F_GETFL)");
      exit (EXIT_FAILURE);
    }
  opts = (opts | O_NONBLOCK);
  if (fcntl (sock, F_SETFL, opts) < 0)
    {
      perror ("fcntl(F_SETFL)");
      exit (EXIT_FAILURE);
    }
  return;
#endif
}
