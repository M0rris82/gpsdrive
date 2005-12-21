//
// Copyright (c) 2005 -- Daniel Wallner
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifdef HAVE_GPS

#include <../lib_map/gps.h>

#include <stdio.h>

struct gps_data_t *gpsdata;
pthread_t handler;

void gps_callback(struct gps_data_t *sentence,
		  char *buf, size_t len, int level)
{
  printf("gps\n");
}

void init_gps()
{
  gpsdata = gps_open("localhost", DEFAULT_GPSD_PORT);

  if (!gpsdata)
    {
      fprintf(stderr, "Unable to open gps\n");
      return;
    }

  if (!gps_set_callback(gpsdata, gps_callback, &handler))
    {
      fprintf(stderr, "Unable to create gps thread\n");
      return;
    }
}

void free_gps()
{
  if (gpsdata)
    {
      gps_del_callback(gpsdata, &handler);
      gps_close(gpsdata);
    }
}

#endif
