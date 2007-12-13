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

#ifndef GPSDRIVE_OS_SPECIFIC_H_XXX
#define GPSDRIVE_OS_SPECIFIC_H


// The open() call on Win32 needs O_BINARY flag that's not even defined on UNIX
#ifndef O_BINARY
#define O_BINARY 0
#endif


// network socket specific things
#ifdef _WIN32
typedef SOCKET SOCKET_TYPE;
#else
typedef int SOCKET_TYPE;
#endif

extern void socket_close(SOCKET_TYPE sock);
extern void socket_nonblocking(SOCKET_TYPE sock);

#endif
