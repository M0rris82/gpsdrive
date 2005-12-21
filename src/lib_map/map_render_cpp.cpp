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

#include "map_priv.h"

#ifdef HAVE_QT
#include <qpainter.h>
#include <qimage.h>

int qt_drawmap(GDALDatasetH dataset, void *painter,
	       GInt32 imageWidth, GInt32 imageHeight,
	       GInt32 mapWest, GInt32 mapNorth,
	       GInt32 mapWidth, GInt32 mapHeight,
	       GInt32 xPos, GInt32 yPos, int mode)
{
  QPainter *qpainter = (QPainter *)painter;
  QImage qimage(imageWidth, imageHeight, 32);
  GUInt32 *imageData = (GUInt32 *)qimage.bits();

  // Fill image
  gdal2argb(dataset, imageData, imageWidth, imageHeight,
	    mapWest, mapNorth, mapWidth, mapHeight,
	    3, 0, 1, 2, mode);

  // Draw image
  qpainter->drawImage(xPos, yPos, qimage);

  return 1;
}
#endif
