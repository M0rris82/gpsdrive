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

#ifdef WIN32
static void win32_setpen(MapGC *mgc, int width, COLORREF color)
{
  HPEN pen = mgc->win_pen;
  mgc->win_pen = CreatePen(PS_SOLID, width, color);
  SelectObject(mgc->win_dc, mgc->win_pen);
  if (pen)
    DeleteObject(pen);
}
#endif

void map_line(MapGC *mgc, MapSettings *settings,
	      int x1, int y1, int x2, int y2, int style)
{
  if (FALSE);
#ifdef HAVE_CAIRO
  else if (mgc->cairo_cr)
    {
      if (style)
	{
	  cairo_set_source_rgb(mgc->cairo_cr, .627, .627, .627);
	  cairo_set_line_width(mgc->cairo_cr, 1);
	}
      else
	{
	  cairo_set_source_rgb(mgc->cairo_cr, 0, 0, 0);
	  cairo_set_line_width(mgc->cairo_cr, 2);
	}
      cairo_move_to(mgc->cairo_cr, x1, y1);
      cairo_line_to(mgc->cairo_cr, x2, y2);
      cairo_stroke(mgc->cairo_cr);
    }
#endif
#ifdef HAVE_GTK
  else if (mgc->gtk_drawable)
    {
      if (style)
	{
	  gdk_gc_set_foreground (mgc->gtk_gc, &settings->darkgrey);
	  gdk_gc_set_line_attributes (mgc->gtk_gc, 1, 0, 0, 0);
	}
      else
	{
	  gdk_gc_set_foreground(mgc->gtk_gc, &settings->black);
	  gdk_gc_set_line_attributes(mgc->gtk_gc, 2, 0, 0, 0);
	}
      gdk_draw_line(mgc->gtk_drawable, mgc->gtk_gc, x1, y1, x2, y2);
    }
#endif
#ifdef HAVE_QT
  else if (mgc->qt_painter)
    {
      qt_line(mgc->qt_painter, settings,
	      x1, y1, x2, y2, style);
    }
#endif
#ifdef HAVE_QUARTZ
  else if (mgc->quartz_gc)
    {
      if (style)
	{
	  CGContextSetRGBStrokeColor(mgc->quartz_gc, .627, .627, .627, 1.0);
	  CGContextSetLineWidth(mgc->quartz_gc, 1.0);
	}
      else
	{
	  CGContextSetRGBStrokeColor(mgc->quartz_gc, 0, 0, 0, 1);
	  CGContextSetLineWidth(mgc->quartz_gc, 2.0);
	}
      CGContextBeginPath(mgc->quartz_gc);
      CGContextMoveToPoint(mgc->quartz_gc, x1, y1);
      CGContextAddLineToPoint(mgc->quartz_gc, x2, y2);
      CGContextStrokePath(mgc->quartz_gc);
    }
#endif
#ifdef WIN32
  else if (mgc->win_dc)
    {
      if (style)
	{
          win32_setpen(mgc, 1, RGB(0xa0, 0xa0, 0xa0));
	}
      else
	{
          win32_setpen(mgc, 2, RGB(0x0, 0x0, 0x0));
	}
	MoveToEx(mgc->win_dc, x1, y1, NULL);
	LineTo(mgc->win_dc, x2, y2);
    }
#endif
}

void map_pos_rectangle(MapGC *mgc, MapSettings *settings,
		       int x, int y, int width, int height)
{
  if (FALSE);
#ifdef HAVE_CAIRO
  else if (mgc->cairo_cr)
    {
      cairo_set_source_rgb(mgc->cairo_cr, 0, 0, 1);
      cairo_set_line_width(mgc->cairo_cr, 4);
      cairo_rectangle(mgc->cairo_cr, x, y, width, height);
      cairo_stroke(mgc->cairo_cr);
    }
#endif
#ifdef HAVE_GTK
  else if (mgc->gtk_drawable)
    {
      gdk_gc_set_foreground(mgc->gtk_gc, &settings->blue);
      gdk_gc_set_line_attributes(mgc->gtk_gc, 4, 0, 0, 0);
      gdk_draw_rectangle(mgc->gtk_drawable, mgc->gtk_gc, FALSE,
			 x, y, width, height);
    }
#endif
#ifdef HAVE_QT
  else if (mgc->qt_painter)
    {
      qt_pos_rectangle(mgc->qt_painter, settings,
		       x, y, width, height);
    }
#endif
#ifdef HAVE_QUARTZ
  else if (mgc->quartz_gc)
    {
      CGContextSetRGBStrokeColor(mgc->quartz_gc, 0, 0, 1.0, 1.0);
      CGContextSetLineWidth(mgc->quartz_gc, 4.0);
      //gdk_gc_set_line_attributes(mgc->gtk_gc, 4, 0, 0, 0);
      //gdk_draw_rectangle(mgc->gtk_drawable, mgc->gtk_gc, FALSE,
      //		 x, y, width, height);
    }
#endif
#ifdef WIN32
  else if (mgc->win_dc)
    {
	POINT	p[5];

	p[0].x = x;
	p[0].y = y;
	p[1].x = x + width;
	p[1].y = y;
	p[2].x = x + width;
	p[2].y = y + height;
	p[3].x = x;
	p[3].y = y + height;
	p[4].x = x;
	p[4].y = y;

        win32_setpen(mgc, 4, RGB(0x0, 0x0, 0xff));
	Polyline(mgc->win_dc, p, 5);
    }
#endif
}

void map_bkg_rectangle(MapGC *mgc, MapSettings *settings,
		       int x, int y, int width, int height)
{
  if (FALSE);
#ifdef HAVE_CAIRO
  else if (mgc->cairo_cr)
    {
      cairo_set_operator(mgc->cairo_cr, CAIRO_OPERATOR_OVER);
      cairo_set_source_rgba(mgc->cairo_cr, .937, .937, .937, .8);
      cairo_rectangle(mgc->cairo_cr, x, y, width, height);
      cairo_fill(mgc->cairo_cr);
      cairo_set_operator(mgc->cairo_cr, CAIRO_OPERATOR_SOURCE);
    }
#endif
#ifdef HAVE_GTK
  else if (mgc->gtk_drawable)
    {
      gdk_gc_set_function(mgc->gtk_gc, GDK_OR);
      gdk_gc_set_foreground(mgc->gtk_gc, &settings->textback);
      gdk_draw_rectangle(mgc->gtk_drawable, mgc->gtk_gc, TRUE,
			 x, y, width, height);
      gdk_gc_set_function(mgc->gtk_gc, GDK_COPY);
    }
#endif
#ifdef HAVE_QT
  else if (mgc->qt_painter)
    {
      qt_bkg_rectangle(mgc->qt_painter, settings,
		       x, y, width, height);
    }
#endif
#ifdef HAVE_QUARTZ
  else if (mgc->quartz_gc)
    {
      CGContextSetRGBFillColor(mgc->quartz_gc, .937, .937, .937, 1.);
      //gdk_gc_set_function(mgc->gtk_gc, GDK_OR);
      //gdk_gc_set_foreground(mgc->gtk_gc, &settings->textback);
      //gdk_draw_rectangle(mgc->gtk_drawable, mgc->gtk_gc, TRUE,
      //		 x, y, width, height);
      //gdk_gc_set_function(mgc->gtk_gc, GDK_COPY);
    }
#endif
#ifdef WIN32
  else if (mgc->win_dc)
    {
      COLORREF color = RGB(0xa5, 0xa6, 0xa5);
      HBRUSH brush = CreateSolidBrush(color);
      HBRUSH oldBrush = SelectObject(mgc->win_dc, brush);
      SetROP2(mgc->win_dc, R2_MERGEPEN);
      win32_setpen(mgc, 4, color);
      Rectangle(mgc->win_dc, x, y, x + width, y + height);
      SetROP2(mgc->win_dc, R2_COPYPEN);
      SelectObject(mgc->win_dc, oldBrush);
      DeleteObject(brush);
    }
#endif
}

void map_text(MapGC *mgc, MapSettings *settings,
	      int x, int y, const char *text, int style)
{
  if (FALSE);
#ifdef HAVE_CAIRO
  else if (mgc->cairo_cr)
    {
      int xpos = x;
      int ypos = y;
      cairo_text_extents_t extents;

      cairo_select_font_face(mgc->cairo_cr, "Sans",
			     CAIRO_FONT_SLANT_NORMAL,
			     CAIRO_FONT_WEIGHT_NORMAL);

      if (style & 0x800)
	{
	  if (settings->pdamode)
	    cairo_set_font_size(mgc->cairo_cr, 9);
	  else
	    cairo_set_font_size(mgc->cairo_cr, 14);
	}
      else
	{
	  if (settings->pdamode)
	    cairo_set_font_size(mgc->cairo_cr, 8);
	  else
	    cairo_set_font_size(mgc->cairo_cr, 11);
	}

      cairo_text_extents(mgc->cairo_cr, text, &extents);

      if ((style & 0x300) == 0x100)
	{
	  xpos -= extents.width;
	}
      else if ((style & 0x300) == 0x200)
	{
	  xpos -= extents.width / 2;
	}
      else if ((style & 0x300) == 0x300)
	{
	  xpos -= extents.width / 2;
	  ypos -= extents.height / 2;
	}

      if (style & 0x400)
	map_bkg_rectangle(mgc, settings,
			  xpos, ypos, extents.width, extents.height);

      if (style & 1)
	cairo_set_source_rgb(mgc->cairo_cr, 0, 0, 1);
      else
	cairo_set_source_rgb(mgc->cairo_cr, 0, 0, 0);

      cairo_move_to(mgc->cairo_cr, xpos, ypos + extents.height);
      cairo_show_text(mgc->cairo_cr, text);
    }
#endif
#ifdef HAVE_GTK
  else if (mgc->gtk_drawable)
    {
      int xpos = x;
      int ypos = y;
      int width;
      int height;

      PangoFontDescription *fd;
      PangoLayout *layout =
	gtk_widget_create_pango_layout(mgc->gtk_widget, text);

      if (style & 0x800)
	{
	  if (settings->pdamode)
	    fd = pango_font_description_from_string("Sans 9");
	  else
	    fd = pango_font_description_from_string("Sans 14");
	}
      else
	{
	  if (settings->pdamode)
	    fd = pango_font_description_from_string("Sans 8");
	  else
	    fd = pango_font_description_from_string("Sans 11");
	}

      pango_layout_set_font_description(layout, fd);

      pango_layout_get_pixel_size(layout, &width, &height);

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
	map_bkg_rectangle(mgc, settings,
			  xpos, ypos, width, height);

      if (style & 1)
	gdk_draw_layout_with_colors(mgc->gtk_drawable, mgc->gtk_gc, xpos, ypos,
				    layout, &settings->blue, NULL);
      else
	gdk_draw_layout_with_colors(mgc->gtk_drawable, mgc->gtk_gc, xpos, ypos,
				    layout, &settings->black, NULL);

      if (layout != NULL)
	g_object_unref(G_OBJECT(layout));

      pango_font_description_free(fd);
    }
#endif
#ifdef HAVE_QT
  else if (mgc->qt_painter)
    {
      qt_text(mgc->qt_painter, settings,
	      x, y, text, style);
    }
#endif
#ifdef HAVE_QUARTZ
  else if (mgc->quartz_gc)
    {
      int xpos = x;
      int ypos = y;
      int width;
      int height;

      CGContextSelectFont(mgc->quartz_gc, "Verdana",
			  8, kCGEncodingMacRoman);

      if (style & 0x800)
	{
	  if (settings->pdamode)
	    CGContextSetFontSize(mgc->quartz_gc, 9);
	  else
	    CGContextSetFontSize(mgc->quartz_gc, 14);
	}
      else
	{
	  if (settings->pdamode)
	    CGContextSetFontSize(mgc->quartz_gc, 8);
	  else
	    CGContextSetFontSize(mgc->quartz_gc, 11);
	}

      {
	CGPoint oldPos = CGContextGetTextPosition(mgc->quartz_gc);
	CGContextSetTextDrawingMode(mgc->quartz_gc, kCGTextInvisible);
	CGContextShowText(mgc->quartz_gc, text, strlen(text));
	CGPoint newPos = CGContextGetTextPosition(mgc->quartz_gc);
	CGContextSetTextDrawingMode(mgc->quartz_gc, kCGTextFill);
	CGContextSetTextPosition(mgc->quartz_gc, oldPos.x, oldPos.y);
	width = newPos.x - oldPos.x;
      }

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
	map_bkg_rectangle(mgc, settings,
			  xpos, ypos, width, height);

      if (style & 1)
	CGContextSetRGBStrokeColor(mgc->quartz_gc, 0, 0, 1, 1);
      else
	CGContextSetRGBStrokeColor(mgc->quartz_gc, 0, 0, 0, 1);

      CGContextSetTextPosition(mgc->quartz_gc, xpos, ypos);
      CGContextShowText(mgc->quartz_gc, text, strlen(text));
    }
#endif
#ifdef WIN32
  else if (mgc->win_dc)
    {
      int xpos = x;
      int ypos = y;
      int fontSize;
      HFONT oldFont;
      HFONT font;
      SIZE textSize;

      if (style & 0x800)
	{
	  if (settings->pdamode)
	    fontSize = 9;
	  else
	    fontSize = 14;
	}
      else
	{
	  if (settings->pdamode)
	    fontSize = 8;
	  else
	    fontSize = 11;
	}

      fontSize = fontSize * 3 / 2;

      font = CreateFont(fontSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "MSSansSerif");
      oldFont = SelectObject(mgc->win_dc, font);

      GetTextExtentPoint32(mgc->win_dc, text, strlen(text), &textSize);

      if ((style & 0x300) == 0x100)
	{
	  xpos -= textSize.cx;
	}
      else if ((style & 0x300) == 0x200)
	{
	  xpos -= textSize.cx / 2;
	}
      else if ((style & 0x300) == 0x300)
	{
	  xpos -= textSize.cx / 2;
	  ypos -= textSize.cy / 2;
	}

      if (style & 0x400)
	map_bkg_rectangle(mgc, settings,
			  xpos, ypos, textSize.cx, textSize.cy);

      if (style & 1)
	SetTextColor(mgc->win_dc, RGB(0x00, 0x00, 0xff));
      else
	SetTextColor(mgc->win_dc, RGB(0x00, 0x00, 0x00));

      SetBkMode(mgc->win_dc, TRANSPARENT);
      TextOut(mgc->win_dc, xpos, ypos, text, strlen(text));
      SelectObject(mgc->win_dc, oldFont);
      DeleteObject(font);
    }
#endif
}
