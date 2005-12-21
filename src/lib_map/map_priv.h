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

#include "map.h"

/*  defines offset and color of the shadows */
#define SHADOWOFFSET 7

/* #define SHADOWGREY 0xD000  */
#define SHADOWGREY 0xA000

#define PIXELFACT 2817.947378

/* WGS84 parameters */

#define EQUATOR_CIRCUM 40075017.
#define POLE_CIRCUM 40007863.

#ifdef __cplusplus
extern "C" {
#endif

// Private functions

void calcxytopos2(int posx, int posy, double *mylat, double *mylon,
		  double zero_lat, double zero_long, double mapscale);
void calcxy2(double *posx, double *posy, double lat, double lon,
	     double zero_lat, double zero_long, double mapscale);

int mapcovers(GDALDatasetH dataset,
	      double xZoom, double yZoom,
	      double xPixel, double yPixel,
	      double width, double height);

int gdal2argb(GDALDatasetH dataset, GUInt32 *imageData,
	      GInt32 imageWidth, GInt32 imageHeight,
	      GInt32 mapWest, GInt32 mapNorth,
	      GInt32 mapWidth, GInt32 mapHeight,
	      int alphaPos, int redPos, int greenPos, int bluePos, int mode);

// Portable drawing functions

void map_line(MapGC *mgc, MapSettings *settings,
	      int x1, int y1, int x2, int y2, int style);

void map_pos_rectangle(MapGC *mgc, MapSettings *settings,
		       int x, int y, int width, int height);

void map_bkg_rectangle(MapGC *mgc, MapSettings *settings,
		       int x, int y, int width, int height);

void map_text(MapGC *mgc, MapSettings *settings,
	      int x, int y, const char *text, int style);

#ifdef HAVE_QT
void qt_line(void *painter, MapSettings *settings,
	     int x1, int y1, int x2, int y2, int style);

void qt_pos_rectangle(void *painter, MapSettings *settings,
		      int x, int y, int width, int height);

void qt_bkg_rectangle(void *painter, MapSettings *settings,
		      int x, int y, int width, int height);

void qt_text(void *painter, MapSettings *settings,
	     int x, int y, const char *text, int style);

int qt_drawmap(GDALDatasetH dataset, void *painter,
	       GInt32 imageWidth, GInt32 imageHeight,
	       GInt32 mapWest, GInt32 mapNorth,
	       GInt32 mapWidth, GInt32 mapHeight,
	       GInt32 xPos, GInt32 yPos, int mode);
#endif

#ifdef __cplusplus
}
#endif
