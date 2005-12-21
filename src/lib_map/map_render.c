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

int mapinit(MapSettings *settings)
{
  GDALAllRegister();

  settings->gblink = 0;
  settings->blink = 0;
  settings->milesflag = FALSE;
  settings->metricflag = TRUE;
  settings->nauticflag = FALSE;
  settings->pdamode = FALSE;
  settings->drawgrid = FALSE;
  settings->zoomscale = TRUE;
  settings->havepos = TRUE;
  settings->posmode = FALSE;
  settings->shadow = TRUE;
  settings->markwaypoint = TRUE;

#ifdef HAVE_GTK
  GdkColormap *cmap;

  gdk_color_parse("Red", &settings->red);
  gdk_color_parse("Black", &settings->black);
  gdk_color_parse("White", &settings->white);
  gdk_color_parse("Blue", &settings->blue);
  gdk_color_parse("#a00000", &settings->nightcolor);
  gdk_color_parse("#8b958b", &settings->lcd);
  gdk_color_parse("#737d6a", &settings->lcd2);
  gdk_color_parse("Yellow", &settings->yellow);
  gdk_color_parse("#00b000", &settings->green);
  gdk_color_parse("Green", &settings->green2);
  gdk_color_parse("#d5d6d5", &settings->mygray);
  gdk_color_parse("#a5a6a5", &settings->textback);
  gdk_color_parse("#4076cf", &settings->textbacknew);
  gdk_color_parse("#c0c0c0", &settings->grey);
  gdk_color_parse("#f06000", &settings->orange);
  gdk_color_parse("#ff8000", &settings->orange2);
  settings->darkgrey.pixel = 0;
  settings->darkgrey.red = SHADOWGREY;
  settings->darkgrey.green = SHADOWGREY;
  settings->darkgrey.blue = SHADOWGREY;

  cmap = gdk_colormap_get_system();
  gdk_color_alloc(cmap, &settings->red);
  gdk_color_alloc(cmap, &settings->black);
  gdk_color_alloc(cmap, &settings->blue);
  gdk_color_alloc(cmap, &settings->nightcolor);
  gdk_color_alloc(cmap, &settings->lcd);
  gdk_color_alloc(cmap, &settings->lcd2);
  gdk_color_alloc(cmap, &settings->green);
  gdk_color_alloc(cmap, &settings->green2);
  gdk_color_alloc(cmap, &settings->white);
  gdk_color_alloc(cmap, &settings->mygray);
  gdk_color_alloc(cmap, &settings->yellow);
  gdk_color_alloc(cmap, &settings->darkgrey);
  gdk_color_alloc(cmap, &settings->grey);
  gdk_color_alloc(cmap, &settings->textback);
  gdk_color_alloc(cmap, &settings->textbacknew);
  gdk_color_alloc(cmap, &settings->orange2);
  gdk_color_alloc(cmap, &settings->orange);
#endif

  return 1;
}

int mapcovers(GDALDatasetH dataset,
	      double xZoom, double yZoom,
	      double xPixel, double yPixel,
	      double width, double height)
{
  GInt32 mapWidth = GDALGetRasterXSize(dataset);
  GInt32 mapHeight = GDALGetRasterYSize(dataset);

  if ((mapWidth - xPixel) * xZoom >= width / 2 &&
      (mapHeight - yPixel) * yZoom >= height / 2 &&
      xPixel * xZoom >= width / 2 &&
      yPixel * yZoom >= height / 2)
    {
#ifdef MAP_DEBUG
      fprintf(stderr, "Map covers screen\n");
#endif
      return 1;
    }
  else
    {
#ifdef MAP_DEBUG
      fprintf(stderr, "Map doesn't cover screen\n");
#endif
      return 0;
    }
}

int gdal2argb(GDALDatasetH dataset, GUInt32 *imageData,
	      GInt32 imageWidth, GInt32 imageHeight,
	      GInt32 mapWest, GInt32 mapNorth,
	      GInt32 mapWidth, GInt32 mapHeight,
	      int alphaPos, int redPos, int greenPos, int bluePos, int mode)
{
  if (GDALGetRasterCount(dataset) == 1)
    {
      // Single band image

      GDALRasterBandH band = GDALGetRasterBand(dataset, 1);

#ifdef MAP_DEBUG
      int blockXSize, blockYSize;

      GDALGetBlockSize(band, &blockXSize, &blockYSize);
      fprintf(stderr, "blocksize: %d x %d type: %s, color interp: %s\n",
	      blockXSize, blockYSize,
	      GDALGetDataTypeName(GDALGetRasterDataType(band)),
	      GDALGetColorInterpretationName(
		GDALGetRasterColorInterpretation(band)));
#endif

      switch (GDALGetRasterColorInterpretation(band))
	{
	case GCI_GrayIndex:
	  {
	    GUInt32 palette[256];

	    GInt32 i;
	    for (i = 0; i < 256; i++)
	      {
		palette[i] =
		  (i << 24) |
		  (i << 16) |
		  (i << 8) |
		  i;
	      }

	    GByte *fileData = (GByte *)CPLMalloc(imageWidth * imageHeight);

	    GDALRasterIO(band, GF_Read,
			 mapWest, mapNorth, mapWidth, mapHeight,
	    		 fileData, imageWidth, imageHeight, GDT_Byte, 0, 0);

	    for (i = 0; i < imageWidth * imageHeight; i++)
	      {
		imageData[i] = palette[fileData[i]];
	      }

	    CPLFree(fileData);
	  }
	  break;

	case GCI_PaletteIndex:
	  {
	    GDALColorTableH hColorTable = GDALGetRasterColorTable(band);

	    GUInt32 *palette =
	      (GUInt32 *)CPLMalloc(GDALGetColorEntryCount(hColorTable) * 4);

	    GInt32 i;
	    for (i = 0; i < GDALGetColorEntryCount(hColorTable); i++)
	      {
		GDALColorEntry entry;

		GDALGetColorEntryAsRGB(hColorTable, i, &entry);
		palette[i] =
		  (entry.c4 << (8 * alphaPos)) |
		  (entry.c3 << (8 * redPos)) |
		  (entry.c2 << (8 * greenPos)) |
		  (entry.c1 << (8 * bluePos));
	      }

	    GByte *fileData = (GByte *)CPLMalloc(imageWidth * imageHeight);

	    GDALRasterIO(band, GF_Read,
			 mapWest, mapNorth, mapWidth, mapHeight,
			 fileData, imageWidth, imageHeight, GDT_Byte, 0, 0);

	    for (i = 0; i < imageWidth * imageHeight ; i++)
	      {
		imageData[i] = palette[fileData[i]];
	      }

	    CPLFree(palette);
	    CPLFree(fileData);
	  }
	  break;

	default:
	  fprintf(stderr, "Unsupported single band type");
	  exit(1);
	}
    }
  else
    {
      // Multi band image
      GInt32 i;
#ifdef SINGLE_RASTERIO
      int    bandMap[4];

      for (i = 0; i < imageWidth * imageHeight; i++)
	{
	  imageData[i] = 0xffffffff;
	}

      bandMap[3] = bandMap[2] = bandMap[1] = bandMap[0] = 0;

      for (i = 1; i <= GDALGetRasterCount(dataset); i++)
	{
	  GDALRasterBandH band = GDALGetRasterBand(dataset, i);

#ifdef MAP_DEBUG
	  int blockXSize, blockYSize;

	  GDALGetBlockSize(band, &blockXSize, &blockYSize);
	  fprintf(stderr, "blocksize: %d x %d type: %s, color interp: %s\n",
		  blockXSize, blockYSize,
		  GDALGetDataTypeName(GDALGetRasterDataType(band)),
		  GDALGetColorInterpretationName(
			GDALGetRasterColorInterpretation(band)));
#endif

	  switch (GDALGetRasterColorInterpretation(band))
	    {
	    case GCI_AlphaBand:
	      {
		bandMap[3] = i;
	      }
	      break;

	    case GCI_RedBand:
	      {
		bandMap[0] = i;
	      }
	      break;

	    case GCI_GreenBand:
	      {
		bandMap[1] = i;
	      }
	      break;

	    case GCI_BlueBand:
	      {
		bandMap[2] = i;
	      }
	      break;

	    default:
	      fprintf(stderr, "Skipping unsupported multi band type: %s\n",
		      GDALGetColorInterpretationName(
			GDALGetRasterColorInterpretation(band)));
	    }
	}

#ifdef MAP_DEBUG
      fprintf(stderr, "bandmap: %d,%d,%d,%d\n",
	      bandMap[0], bandMap[1], bandMap[2], bandMap[3]);
#endif

      if (!bandMap[0] || !bandMap[1] || !bandMap[2])
	{
	  fprintf(stderr, "Not an RGB image\n");
	  return 0;
	}

      if (bandMap[3])
	{
	  // Image has alpha channel

	  GByte *fileData = (GByte *)CPLMalloc(imageWidth * imageHeight * 4);

	  GDALDatasetRasterIO(dataset, GF_Read,
			      mapWest, mapNorth, mapWidth, mapHeight,
			      fileData, imageWidth, imageHeight,
			      GDT_Byte, 4, bandMap, 0, 0, 0);

	  int bi = imageWidth * imageHeight;

	  for (i = 0; i < bi; i++)
	    {
	      ((GByte *)imageData)[i * 4 + redPos] = fileData[i];
	      ((GByte *)imageData)[i * 4 + greenPos] = fileData[i + bi];
	      ((GByte *)imageData)[i * 4 + bluePos] = fileData[i + 2 * bi];
	      ((GByte *)imageData)[i * 4 + alphaPos] = fileData[i + 3 * bi];
	    }
	}
      else
	{
	  GByte *fileData = (GByte *)CPLMalloc(imageWidth * imageHeight * 3);

	  GDALDatasetRasterIO(dataset, GF_Read,
			      mapWest, mapNorth, mapWidth, mapHeight,
			      fileData, imageWidth, imageHeight,
			      GDT_Byte, 3, bandMap, 0, 0, 0);

	  int bi = imageWidth * imageHeight;

	  for (i = 0; i < bi; i++)
	    {
	      ((GByte *)imageData)[i * 3 + redPos] = fileData[i];
	      ((GByte *)imageData)[i * 3 + greenPos] = fileData[i + bi];
	      ((GByte *)imageData)[i * 3 + bluePos] = fileData[i + 2 * bi];
	    }

	  CPLFree(fileData);
	}
#else
      for (i = 0; i < imageWidth * imageHeight; i++)
	{
	  imageData[i] = 0xffffffff;
	}

      for (i = 1; i <= GDALGetRasterCount(dataset); i++)
	{
	  GDALRasterBandH band;
	  band = GDALGetRasterBand(dataset, i);

#ifdef MAP_DEBUG
	  int blockXSize, blockYSize;

	  GDALGetBlockSize(band, &blockXSize, &blockYSize);
	  fprintf(stderr, "blocksize: %d x %d type: %s, color interp: %s\n",
		  blockXSize, blockYSize,
		  GDALGetDataTypeName(GDALGetRasterDataType(band)),
		  GDALGetColorInterpretationName(
			GDALGetRasterColorInterpretation(band)));
#endif

	  switch (GDALGetRasterColorInterpretation(band))
	    {
	    case GCI_RedBand:
	      {
		GByte *start = (GByte *)imageData;
		start += 2 - redPos;
		GDALRasterIO(band, GF_Read,
			     mapWest, mapNorth, mapWidth, mapHeight,
			     start, imageWidth, imageHeight, GDT_Byte, 4, 0);
	      }
	      break;

	    case GCI_GreenBand:
	      {
		GByte *start = (GByte *)imageData;
		start += 2 - greenPos;
		GDALRasterIO(band, GF_Read,
			     mapWest, mapNorth, mapWidth, mapHeight,
			     start, imageWidth, imageHeight, GDT_Byte, 4, 0);
	      }
	      break;

	    case GCI_BlueBand:
	      {
		GByte *start = (GByte *)imageData;
		start += 2 - bluePos;
		GDALRasterIO(band, GF_Read,
			     mapWest, mapNorth, mapWidth, mapHeight,
			     start, imageWidth, imageHeight, GDT_Byte, 4, 0);
	      }
	      break;

	    case GCI_AlphaBand:
	      {
		GByte *start = (GByte *)imageData;
		start += 3;
		GDALRasterIO(band, GF_Read,
			     mapWest, mapNorth, mapWidth, mapHeight,
			     start, imageWidth, imageHeight, GDT_Byte, 4, 0);
	      }
	      break;

	    default:
	      fprintf(stderr, "Skipping unsupported multi band type: %s\n",
		      GDALGetColorInterpretationName(
			GDALGetRasterColorInterpretation(band)));
	    }
	}
#endif
    }

  if (mode)
    {
      GInt32 i;

      for (i = 0; i < imageWidth * imageHeight * 4; i++)
	{
	  ((GByte *)imageData)[i] >>= 1;
	}
    }

  return 1;
}

int drawmap(MapState *state, MapGC *mgc,
	    double viewWidth, double viewHeight, int mode)
{
  int i = 0;

  while (state->dataset[i])
    i++;

  if (mapcovers(state->dataset[0],
		state->act_xZoom[0], state->act_yZoom[0],
		state->act_xPixel[0], state->act_yPixel[0],
		viewWidth, viewHeight) && !mode)
    i = 1;

  for (i--; i >= 0; i--)
    {
      double xZoom = state->act_xZoom[i];
      double yZoom = state->act_yZoom[i];

      // Image width
      GInt32 imageWidth = viewWidth;
      GInt32 imageHeight = viewHeight;

#ifdef HAVE_CAIRO
      // Only cairo supports rotation at the moment
      if (mgc->cairo_cr && state->req_rotation != 0.)
	{
	  // This is not correct, should be calculated from rotation
	  imageWidth = viewWidth * 2.0;
	  imageHeight = viewHeight * 2.0;
	}
#endif

      // Zoom
      GInt32 zoomedWidth = imageWidth / xZoom + 2;
      GInt32 zoomedHeight = imageHeight / yZoom + 2;

      GInt32 actWest = state->act_xPixel[i] - zoomedWidth / 2;
      GInt32 actEast = state->act_xPixel[i] + zoomedWidth / 2;
      GInt32 actNorth = state->act_yPixel[i] - zoomedHeight / 2;
      GInt32 actSouth = state->act_yPixel[i] + zoomedHeight / 2;

      // Correct for image size
      GInt32 limWest = MIN(MAX(actWest, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limNorth = MIN(MAX(actNorth, 0),
			    GDALGetRasterYSize(state->dataset[i]));
      GInt32 limEast = MIN(MAX(actEast, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limSouth = MIN(MAX(actSouth, 0),
			    GDALGetRasterYSize(state->dataset[i]));

      GInt32 fileWidth = limEast - limWest;
      GInt32 fileHeight = limSouth - limNorth;

      imageWidth = fileWidth * xZoom;
      imageHeight = fileHeight * yZoom;

      // Center
      GInt32 xLimCenter =
	- (actWest - limWest + (actEast - actWest) / 2) * xZoom;
      GInt32 yLimCenter =
	- (actNorth - limNorth + (actSouth - actNorth) / 2) * yZoom;

      // Position
      GInt32 xLimPos = -(actWest - limWest) * xZoom;
      GInt32 yLimPos = -(actNorth - limNorth) * yZoom;

#ifdef MAP_DEBUG
      fprintf(stderr,
	      "cairo size: %d x %d\n"
	      "center: %d;%d\n"
	      "limCenter: %d;%d\n"
	      "limPos: %d;%d\n"
	      "zoomed: %d x %d\n"
	      "actWest: %d actEast: %d\n"
	      "actNorth: %d actSouth: %d\n"
	      "limWest: %d limEast: %d\n"
	      "limNorth: %d limSouth: %d\n"
	      "file size: %d x %d\n",
	      imageWidth, imageHeight,
	      state->act_xPixel[i], state->act_yPixel[i],
	      xLimCenter, yLimCenter,
	      xLimPos, yLimPos,
	      zoomedWidth, zoomedHeight,
	      actWest, actEast,
	      actNorth, actSouth,
	      limWest, limEast,
	      limNorth, limSouth,
	      fileWidth, fileHeight);
#endif

      if (!(imageWidth && imageHeight));
#ifdef HAVE_CAIRO
      else if (mgc->cairo_cr)
	{
	  cairo_t *cr = mgc->cairo_cr;
	  cairo_surface_t *image;

	  GUInt32 *imageData = (GUInt32 *)
	    CPLMalloc(imageWidth * imageHeight * sizeof(GUInt32));

	  image =
	    cairo_image_surface_create_for_data((unsigned char*)imageData,
						CAIRO_FORMAT_ARGB32,
						imageWidth,
						imageHeight,
						imageWidth * sizeof(GUInt32));

	  // Fill image
	  gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		    limWest, limNorth, fileWidth, fileHeight, 3, 0, 1, 2, mode);

	  // Draw image
	  cairo_save(cr);

	  cairo_translate(cr, 0.5 * viewWidth, 0.5 * viewHeight);
	  cairo_rotate(cr, state->req_rotation * M_PI / 180);

	  cairo_translate(cr, xLimCenter, yLimCenter);

	  cairo_set_source_surface(cr, image, 0, 0);
	  cairo_paint(cr);
	  cairo_surface_destroy(image);

	  cairo_restore(cr);

	  // Clean up
	  CPLFree(imageData);
	}
#endif
#ifdef HAVE_GTK
      else if (mgc->gtk_drawable)
	{
	  GUInt32 *imageData = (GUInt32 *) CPLMalloc(imageWidth *
						     imageHeight *
						     sizeof(GUInt32));

	  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((guchar*)imageData,
						       GDK_COLORSPACE_RGB,
						       TRUE, 8,
						       imageWidth, imageHeight,
						       imageWidth * 4,
						       NULL, NULL);

	  // Fill image
	  gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		    limWest, limNorth, fileWidth, fileHeight,
		    3, 2, 1, 0, mode);

	  // Draw image
	  gdk_draw_pixbuf(mgc->gtk_drawable, NULL, pixbuf, 0, 0,
			  xLimPos, yLimPos, -1, -1,
			  GDK_RGB_DITHER_NORMAL, 0, 0);

	  // Clean up
	  g_object_unref(pixbuf);

	  CPLFree(imageData);
	}
#endif
#ifdef HAVE_QT
      else if (mgc->qt_painter)
	{
	  qt_drawmap(state->dataset[i], mgc->qt_painter, imageWidth, imageHeight,
		     limWest, limNorth, fileWidth, fileHeight, xLimPos, yLimPos, mode);
	}
#endif
#ifdef HAVE_QUARTZ
      else if (mgc->quartz_gc)
	{
	  GUInt32 *imageData = (GUInt32 *)
	    CPLMalloc(imageWidth * imageHeight * sizeof(GUInt32));

	  // Fill image
	  gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		    limWest, limNorth, fileWidth, fileHeight, 3, 0, 1, 2, mode);

	  CGContextSaveGState(mgc->quartz_gc);

	  // Draw image
	  {
	    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	    CGDataProviderRef provider =
	      CGDataProviderCreateWithData(NULL, imageData,
					   imageWidth * imageHeight * 4, NULL);

	    CGImageRef image =
	      CGImageCreate(
			    imageWidth,
			    imageHeight,
			    8,
			    32,
			    imageWidth * 4,
			    colorSpace,
			    kCGImageAlphaPremultipliedFirst,
			    provider,
			    NULL,
			    0,
			    kCGRenderingIntentDefault);

	    CGRect bounds = CGRectMake(xLimPos, yLimPos, imageWidth, imageHeight);

	    HIViewDrawCGImage(mgc->quartz_gc, &bounds, image);

	    CGImageRelease(image);

	    CGDataProviderRelease(provider);
	    CGColorSpaceRelease(colorSpace);
	  }

	  CGContextRestoreGState(mgc->quartz_gc);

	  // Clean up
	  CPLFree(imageData);
	}
#endif
#ifdef WIN32
      else if (mgc->win_dc)
	{
	  GUInt32 *imageData;
	  BITMAPINFO bmi;
	  HBITMAP dib;
	  HDC dib_dc = CreateCompatibleDC(NULL);

	  memset(&bmi, 0, sizeof(BITMAPINFO));

	  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	  bmi.bmiHeader.biWidth = imageWidth;
	  bmi.bmiHeader.biHeight = imageHeight;
	  bmi.bmiHeader.biPlanes = 1;
	  bmi.bmiHeader.biBitCount = 32;
	  bmi.bmiHeader.biCompression = BI_RGB;
	  bmi.bmiHeader.biClrUsed = 0;

	  dib = CreateDIBSection(dib_dc, &bmi, DIB_RGB_COLORS,
				 (void **)&imageData, NULL, 0);
	  SelectObject(dib_dc, dib);

	  // Fill image
	  gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		    limWest, limNorth, fileWidth, fileHeight,
		    3, 0, 1, 2, mode);

	  // Draw image
	  StretchBlt(mgc->win_dc, xLimPos, yLimPos, imageWidth, imageHeight,
		     dib_dc, 0, imageHeight, imageWidth, -imageHeight, SRCCOPY);

	  // Clean up
	  DeleteDC(dib_dc);
	  DeleteObject(dib);
	}
#endif
    }

  // Update state

  state->act_rotation = 0;
  state->act_width = viewWidth;
  state->act_height = viewHeight;

#ifdef HAVE_CAIRO
  // Only cairo supports rotation at the moment
  if (mgc->cairo_cr && state->req_rotation != 0.)
    {
      state->act_rotation = state->req_rotation;
    }
#endif

  return 1;
}





#ifdef bajshora

#ifdef HAVE_CAIRO
int gdal2cairo(MapState *state, cairo_t *cr,
	       double crWidth, double crHeight, int mode)
{
  int i = 0;

  while (state->dataset[i])
    i++;

  if (mapcovers(state->dataset[0],
		state->act_xZoom[0], state->act_yZoom[0],
		state->act_xPixel[0], state->act_yPixel[0],
		crWidth, crHeight) && !mode)
    i = 1;

  for (i--; i >= 0; i--)
    {
      double xZoom = state->act_xZoom[i];
      double yZoom = state->act_yZoom[i];

      // Image width
      GInt32 imageWidth = crWidth;
      GInt32 imageHeight = crHeight;

      if (state->req_rotation != 0.)
	{
	  // This is not correct, should be calculated from rotation
	  imageWidth = crWidth * 2.0;
	  imageHeight = crHeight * 2.0;
	}

      // Zoom
      GInt32 zoomedWidth = imageWidth / xZoom + 2;
      GInt32 zoomedHeight = imageHeight / yZoom + 2;

      GInt32 actWest = state->act_xPixel[i] - zoomedWidth / 2;
      GInt32 actEast = state->act_xPixel[i] + zoomedWidth / 2;
      GInt32 actNorth = state->act_yPixel[i] - zoomedHeight / 2;
      GInt32 actSouth = state->act_yPixel[i] + zoomedHeight / 2;

      // Correct for image size
      GInt32 limWest = MIN(MAX(actWest, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limNorth = MIN(MAX(actNorth, 0),
			    GDALGetRasterYSize(state->dataset[i]));
      GInt32 limEast = MIN(MAX(actEast, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limSouth = MIN(MAX(actSouth, 0),
			    GDALGetRasterYSize(state->dataset[i]));

      GInt32 fileWidth = limEast - limWest;
      GInt32 fileHeight = limSouth - limNorth;

      imageWidth = fileWidth * xZoom;
      imageHeight = fileHeight * yZoom;

      // Center
      GInt32 xLimCenter =
	- (actWest - limWest + (actEast - actWest) / 2) * xZoom;
      GInt32 yLimCenter =
	- (actNorth - limNorth + (actSouth - actNorth) / 2) * yZoom;

#ifdef MAP_DEBUG
      fprintf(stderr,
	      "cairo size: %d x %d\n"
	      "center: %d;%d\n"
	      "limCenter: %d;%d\n"
	      "zoomed: %d x %d\n"
	      "actWest: %d actEast: %d\n"
	      "actNorth: %d actSouth: %d\n"
	      "limWest: %d limEast: %d\n"
	      "limNorth: %d limSouth: %d\n"
	      "file size: %d x %d\n",
	      imageWidth, imageHeight,
	      state->act_xPixel[i], state->act_yPixel[i],
	      xLimCenter, yLimCenter,
	      zoomedWidth, zoomedHeight,
	      actWest, actEast,
	      actNorth, actSouth,
	      limWest, limEast,
	      limNorth, limSouth,
	      fileWidth, fileHeight);
#endif

      cairo_surface_t *image;

      GUInt32 *imageData = (GUInt32 *)
	CPLMalloc(imageWidth * imageHeight * sizeof(GUInt32));

      image =
	cairo_image_surface_create_for_data((unsigned char*)imageData,
					    CAIRO_FORMAT_ARGB32,
					    imageWidth,
					    imageHeight,
					    imageWidth * sizeof(GUInt32));

      // Fill image
      gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		limWest, limNorth, fileWidth, fileHeight, 3, 0, 1, 2, mode);

      // Draw image
      cairo_save(cr);

      cairo_translate(cr, 0.5 * crWidth, 0.5 * crHeight);
      cairo_rotate(cr, state->req_rotation * M_PI / 180);

      cairo_translate(cr, xLimCenter, yLimCenter);

      cairo_set_source_surface(cr, image, 0, 0);
      cairo_paint(cr);
      cairo_surface_destroy(image);

      cairo_restore(cr);

      // Clean up
      CPLFree(imageData);
    }

  // Update state
  state->act_rotation = state->req_rotation;
  state->act_width = crWidth;
  state->act_height = crHeight;

  return 1;
}
#endif

#ifdef HAVE_GTK
int gdal2gtk(MapState *state, GdkDrawable *drawable,
	     gint gtkWidth, gint gtkHeight, int mode)
{
  int i = 0;

  while (state->dataset[i])
    i++;

  if (mapcovers(state->dataset[0],
		state->act_xZoom[0], state->act_yZoom[0],
		state->act_xPixel[0], state->act_yPixel[0],
		gtkWidth, gtkHeight) && !mode)
    i = 1;

  for (i--; i >= 0; i--)
    {
      double xZoom = state->act_xZoom[i];
      double yZoom = state->act_yZoom[i];

      // Image width
      GInt32 imageWidth = gtkWidth;
      GInt32 imageHeight = gtkHeight;

#ifdef MAP_DEBUG
      fprintf(stderr,"zoom: %lf;%lf x y %d;%d\n",
	      xZoom, yZoom, state->act_xPixel[i], state->act_yPixel[i]);
#endif

      // Zoom
      GInt32 zoomedWidth = imageWidth / xZoom + 2;
      GInt32 zoomedHeight = imageHeight / yZoom + 2;

      GInt32 actWest = state->act_xPixel[i] - zoomedWidth / 2;
      GInt32 actEast = state->act_xPixel[i] + zoomedWidth / 2;
      GInt32 actNorth = state->act_yPixel[i] - zoomedHeight / 2;
      GInt32 actSouth = state->act_yPixel[i] + zoomedHeight / 2;

      // Correct for image size
      GInt32 limWest = MIN(MAX(actWest, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limNorth = MIN(MAX(actNorth, 0),
			    GDALGetRasterYSize(state->dataset[i]));
      GInt32 limEast = MIN(MAX(actEast, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limSouth = MIN(MAX(actSouth, 0),
			    GDALGetRasterYSize(state->dataset[i]));

      GInt32 fileWidth = limEast - limWest;
      GInt32 fileHeight = limSouth - limNorth;

      imageWidth = fileWidth * xZoom;
      imageHeight = fileHeight * yZoom;

      // Position
      GInt32 xLimPos = -(actWest - limWest) * xZoom;
      GInt32 yLimPos = -(actNorth - limNorth) * yZoom;

#ifdef MAP_DEBUG
      fprintf(stderr,
	      "gtk size: %d x %d\n"
	      "center: %d;%d\n"
	      "limPos: %d;%d\n"
	      "zoomed: %d x %d\n"
	      "actWest: %d actEast: %d\n"
	      "actNorth: %d actSouth: %d\n"
	      "limWest: %d limEast: %d\n"
	      "limNorth: %d limSouth: %d\n"
	      "file size: %d x %d\n",
	      imageWidth, imageHeight,
	      state->act_xPixel[i], state->act_yPixel[i],
	      xLimPos, yLimPos,
	      zoomedWidth, zoomedHeight,
	      actWest, actEast,
	      actNorth, actSouth,
	      limWest, limEast,
	      limNorth, limSouth,
	      fileWidth, fileHeight);
#endif

      if (imageWidth && imageHeight)
	{
	  GUInt32 *imageData = (GUInt32 *) CPLMalloc(imageWidth *
						     imageHeight *
						     sizeof(GUInt32));

	  GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data((guchar*)imageData,
						       GDK_COLORSPACE_RGB,
						       TRUE, 8,
						       imageWidth, imageHeight,
						       imageWidth * 4,
						       NULL, NULL);

	  // Fill image
	  gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		    limWest, limNorth, fileWidth, fileHeight,
		    3, 2, 1, 0, mode);

	  // Draw image
	  gdk_draw_pixbuf(drawable, NULL, pixbuf, 0, 0,
			  xLimPos, yLimPos, -1, -1,
			  GDK_RGB_DITHER_NORMAL, 0, 0);

	  // Clean up
	  g_object_unref(pixbuf);

	  CPLFree(imageData);
	}
    }

  // Update state
  state->act_rotation = 0.;
  state->act_width = gtkWidth;
  state->act_height = gtkHeight;

  return 1;
}
#endif

#ifdef HAVE_QUARTZ
int gdal2quartz(MapState *state, CGContextRef gc,
	        double qWidth, double qHeight, int mode)
{
  int i = 0;

  while (state->dataset[i])
    i++;

  if (mapcovers(state->dataset[0],
		state->act_xZoom[0], state->act_yZoom[0],
		state->act_xPixel[0], state->act_yPixel[0],
		qWidth, qHeight) && !mode)
    i = 1;

  for (i--; i >= 0; i--)
    {
      double xZoom = state->act_xZoom[i];
      double yZoom = state->act_yZoom[i];

      // Image width
      GInt32 imageWidth = qWidth;
      GInt32 imageHeight = qHeight;

      if (state->req_rotation != 0.)
	{
	  // This is not correct, should be calculated from rotation
	  imageWidth = qWidth * 2.0;
	  imageHeight = qHeight * 2.0;
	}

      // Zoom
      GInt32 zoomedWidth = imageWidth / xZoom + 2;
      GInt32 zoomedHeight = imageHeight / yZoom + 2;

      GInt32 actWest = state->act_xPixel[i] - zoomedWidth / 2;
      GInt32 actEast = state->act_xPixel[i] + zoomedWidth / 2;
      GInt32 actNorth = state->act_yPixel[i] - zoomedHeight / 2;
      GInt32 actSouth = state->act_yPixel[i] + zoomedHeight / 2;

      // Correct for image size
      GInt32 limWest = MIN(MAX(actWest, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limNorth = MIN(MAX(actNorth, 0),
			    GDALGetRasterYSize(state->dataset[i]));
      GInt32 limEast = MIN(MAX(actEast, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limSouth = MIN(MAX(actSouth, 0),
			    GDALGetRasterYSize(state->dataset[i]));

      GInt32 fileWidth = limEast - limWest;
      GInt32 fileHeight = limSouth - limNorth;

      imageWidth = fileWidth * xZoom;
      imageHeight = fileHeight * yZoom;

      // Center
      GInt32 xLimCenter =
	- (actWest - limWest + (actEast - actWest) / 2) * xZoom;
      GInt32 yLimCenter =
	- (actNorth - limNorth + (actSouth - actNorth) / 2) * yZoom;

      // Position
      GInt32 xLimPos = -(actWest - limWest) * xZoom;
      GInt32 yLimPos = -(actNorth - limNorth) * yZoom;

#ifdef MAP_DEBUG
      fprintf(stderr,
	      "cairo size: %d x %d\n"
	      "center: %d;%d\n"
	      "limCenter: %d;%d\n"
	      "zoomed: %d x %d\n"
	      "actWest: %d actEast: %d\n"
	      "actNorth: %d actSouth: %d\n"
	      "limWest: %d limEast: %d\n"
	      "limNorth: %d limSouth: %d\n"
	      "file size: %d x %d\n",
	      imageWidth, imageHeight,
	      state->act_xPixel[i], state->act_yPixel[i],
	      xLimCenter, yLimCenter,
	      zoomedWidth, zoomedHeight,
	      actWest, actEast,
	      actNorth, actSouth,
	      limWest, limEast,
	      limNorth, limSouth,
	      fileWidth, fileHeight);
#endif

      GUInt32 *imageData = (GUInt32 *)
	CPLMalloc(imageWidth * imageHeight * sizeof(GUInt32));

      // Fill image
      gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		limWest, limNorth, fileWidth, fileHeight, 3, 0, 1, 2, mode);

      CGContextSaveGState(gc);

      //cairo_translate(cr, 0.5 * crWidth, 0.5 * crHeight);
      //cairo_rotate(cr, state->req_rotation * M_PI / 180);

      //cairo_translate(cr, xLimCenter, yLimCenter);

      //cairo_set_source_surface(cr, image, 0, 0);
      //cairo_paint(cr);

      // Draw image
{
      CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef provider =
    CGDataProviderCreateWithData(
      NULL, imageData, imageWidth * imageHeight * 4, NULL);

  CGImageRef image =
    CGImageCreate(
      imageWidth,
      imageHeight,
      8,
      32,
      imageWidth * 4,
      colorSpace,
			       kCGImageAlphaPremultipliedFirst,
      provider,
      NULL,
      0,
      kCGRenderingIntentDefault);

      CGRect bounds = CGRectMake(xLimPos, yLimPos, imageWidth, imageHeight);

  HIViewDrawCGImage(gc, &bounds, image);

  CGImageRelease(image);

  CGDataProviderRelease(provider);
      CGColorSpaceRelease(colorSpace);
}

      CGContextRestoreGState(gc);

      // Clean up
      CPLFree(imageData);
    }

  // Update state
  //state->act_rotation = state->req_rotation;
  state->act_rotation = 0;
  state->act_width = qWidth;
  state->act_height = qHeight;

  return 1;
}
#endif

#ifdef WIN32
int gdal2win32(MapState *state, HDC dc,
	       int gtkWidth, int gtkHeight, int mode)
{
  int i = 0;

  while (state->dataset[i])
    i++;

  if (mapcovers(state->dataset[0],
		state->act_xZoom[0], state->act_yZoom[0],
		state->act_xPixel[0], state->act_yPixel[0],
		gtkWidth, gtkHeight) && !mode)
    i = 1;

  for (i--; i >= 0; i--)
    {
      double xZoom = state->act_xZoom[i];
      double yZoom = state->act_yZoom[i];

      // Image width
      GInt32 imageWidth = gtkWidth;
      GInt32 imageHeight = gtkHeight;

#ifdef MAP_DEBUG
      fprintf(stderr,"zoom: %lf;%lf x y %d;%d\n",
	      xZoom, yZoom, state->act_xPixel[i], state->act_yPixel[i]);
#endif

      // Zoom
      GInt32 zoomedWidth = imageWidth / xZoom + 2;
      GInt32 zoomedHeight = imageHeight / yZoom + 2;

      GInt32 actWest = state->act_xPixel[i] - zoomedWidth / 2;
      GInt32 actEast = state->act_xPixel[i] + zoomedWidth / 2;
      GInt32 actNorth = state->act_yPixel[i] - zoomedHeight / 2;
      GInt32 actSouth = state->act_yPixel[i] + zoomedHeight / 2;

      // Correct for image size
      GInt32 limWest = MIN(MAX(actWest, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limNorth = MIN(MAX(actNorth, 0),
			    GDALGetRasterYSize(state->dataset[i]));
      GInt32 limEast = MIN(MAX(actEast, 0),
			   GDALGetRasterXSize(state->dataset[i]));
      GInt32 limSouth = MIN(MAX(actSouth, 0),
			    GDALGetRasterYSize(state->dataset[i]));

      GInt32 fileWidth = limEast - limWest;
      GInt32 fileHeight = limSouth - limNorth;

      imageWidth = fileWidth * xZoom;
      imageHeight = fileHeight * yZoom;

      // Position
      GInt32 xLimPos = -(actWest - limWest) * xZoom;
      GInt32 yLimPos = -(actNorth - limNorth) * yZoom;

#ifdef MAP_DEBUG
      fprintf(stderr,
	      "gtk size: %d x %d\n"
	      "center: %d;%d\n"
	      "limPos: %d;%d\n"
	      "zoomed: %d x %d\n"
	      "actWest: %d actEast: %d\n"
	      "actNorth: %d actSouth: %d\n"
	      "limWest: %d limEast: %d\n"
	      "limNorth: %d limSouth: %d\n"
	      "file size: %d x %d\n",
	      imageWidth, imageHeight,
	      state->act_xPixel[i], state->act_yPixel[i],
	      xLimPos, yLimPos,
	      zoomedWidth, zoomedHeight,
	      actWest, actEast,
	      actNorth, actSouth,
	      limWest, limEast,
	      limNorth, limSouth,
	      fileWidth, fileHeight);
#endif

      if (imageWidth && imageHeight)
	{
	  GUInt32 *imageData;
	  BITMAPINFO bmi;
	  HBITMAP dib;
	  HDC dib_dc = CreateCompatibleDC(NULL);

	  memset(&bmi, 0, sizeof(BITMAPINFO));

	  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	  bmi.bmiHeader.biWidth = imageWidth;
	  bmi.bmiHeader.biHeight = imageHeight;
	  bmi.bmiHeader.biPlanes = 1;
	  bmi.bmiHeader.biBitCount = 32;
	  bmi.bmiHeader.biCompression = BI_RGB;
	  bmi.bmiHeader.biClrUsed = 0;

	  dib = CreateDIBSection(dib_dc, &bmi, DIB_RGB_COLORS,
				 (void **)&imageData, NULL, 0);
	  SelectObject(dib_dc, dib);

	  // Fill image
	  gdal2argb(state->dataset[i], imageData, imageWidth, imageHeight,
		    limWest, limNorth, fileWidth, fileHeight,
		    3, 0, 1, 2, mode);

	  // Draw image
	  StretchBlt(dc, xLimPos, yLimPos, imageWidth, imageHeight,
		     dib_dc, 0, imageHeight, imageWidth, -imageHeight, SRCCOPY);

	  // Clean up
	  DeleteDC(dib_dc);
	  DeleteObject(dib);
	}
    }

  // Update state
  state->act_rotation = 0.;
  state->act_width = gtkWidth;
  state->act_height = gtkHeight;

  return 1;
}
#endif

#endif
