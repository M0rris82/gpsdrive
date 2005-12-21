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

void qt_line(void *painter, MapSettings *settings,
	     int x1, int y1, int x2, int y2, int style)
{
  QPainter *qpainter = (QPainter *)painter;

  if (style)
    {
      qpainter->setPen(QColor(0xa0, 0xa0, 0xa0));
      //gdk_gc_set_line_attributes (mgc->gtk_gc, 1, 0, 0, 0);
    }
  else
    {
      qpainter->setPen(QColor(0, 0, 0));
      //gdk_gc_set_line_attributes(mgc->gtk_gc, 2, 0, 0, 0);
    }
  qpainter->drawLine(x1, y1, x2, y2);
}

void qt_pos_rectangle(void *painter, MapSettings *settings,
		      int x, int y, int width, int height)
{
  QPainter *qpainter = (QPainter *)painter;

  qpainter->setPen(QColor(0x00, 0x00, 0xff));
  //gdk_gc_set_line_attributes(mgc->gtk_gc, 4, 0, 0, 0);
  qpainter->drawRect(x, y, width, height);
}

void qt_bkg_rectangle(void *painter, MapSettings *settings,
		      int x, int y, int width, int height)
{
  QPainter *qpainter = (QPainter *)painter;

  //qpainter->setCompositionMode(QPainter::CompositionMode_SourceAtop));
  qpainter->fillRect(x, y, width, height,
		     QBrush(QColor(0xa5, 0xa6, 0xa5)));
  //qpainter->setCompositionMode(QPainter::CompositionMode_SourceOver));
}

void qt_text(void *painter, MapSettings *settings,
	     int x, int y, const char *text, int style)
{
  QPainter *qpainter = (QPainter *)painter;

  int xpos = x;
  int ypos = y;
  int width;
  int height;

  QFont font;

  if (style & 0x800)
    {
      if (settings->pdamode)
	font.setPointSize(9);
      else
	font.setPointSize(14);
    }
  else
    {
      if (settings->pdamode)
	font.setPointSize(8);
      else
	font.setPointSize(11);
    }

  qpainter->setFont(font);

  width = strlen(text) * 10;
  height = 20;

  if ((style & 0x300) == 0x100)
    {
      xpos -= width;
    }
  else if ((style & 0x300) == 0x200)
    {
      xpos -= width / 2;
    }
  else if ((style & 0x300) == 0x300)
    {
      xpos -= width / 2;
      ypos -= height / 2;
    }

  if (style & 0x400)
    qt_bkg_rectangle(painter, settings,
		     xpos, ypos, width, height);

  ypos += 15;

  if (style & 1)
    qpainter->setPen(QColor(0x00, 0x00, 0xff));
  else
    qpainter->setPen(QColor(0x00, 0x00, 0x00));

  qpainter->drawText(xpos, ypos, text);
}

#endif
