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

#include <gdal.h>
#include <cpl_conv.h>
#include <ogr_srs_api.h>

#ifdef HAVE_CAIRO
#include <cairo.h>
#endif

#ifdef HAVE_GTK
#include <gtk/gtk.h>
#endif

#ifdef HAVE_QUARTZ
#include <Carbon/Carbon.h>
#endif

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  int gblink;
  int blink;
  int milesflag;
  int metricflag;
  int nauticflag;
  int pdamode;
  int drawgrid;
  int zoomscale;
  int havepos;
  int posmode;
  int shadow;
  int markwaypoint;

#ifdef HAVE_GTK
  GdkColor red;
  GdkColor black;
  GdkColor white;
  GdkColor blue;
  GdkColor nightcolor;
  GdkColor lcd;
  GdkColor lcd2;
  GdkColor yellow;
  GdkColor green;
  GdkColor green2;
  GdkColor mygray;
  GdkColor textback;
  GdkColor textbacknew;
  GdkColor grey;
  GdkColor orange;
  GdkColor orange2;
  GdkColor darkgrey;
#endif
}
MapSettings;

typedef struct
{
  GInt32 maps;
  GDALDatasetH dataset[1024];
  char *path[1024];
}
MapSet;

typedef struct
{
  double req_lat;
  double req_lon;
  double req_scale;
  double req_rotation;
  double act_width;
  double act_height;
  double act_rotation;
  double act_xZoom[9];
  double act_yZoom[9];
  GInt32 act_xPixel[9];
  GInt32 act_yPixel[9];
  char *path[9];
  GDALDatasetH dataset[9];
  double refScore;
  GDALDatasetH altMaps[32];
}
MapState;

typedef struct
{
#ifdef HAVE_CAIRO
  cairo_t *cairo_cr;
#endif
#ifdef HAVE_GTK
  GtkWidget *gtk_widget;
  GdkDrawable *gtk_drawable;
  GdkGC *gtk_gc;
#endif
#ifdef HAVE_QT
  void *qt_painter;
#endif
#ifdef HAVE_QUARTZ
  CGContextRef quartz_gc;
#endif
#ifdef WIN32
  HDC win_dc;
  HPEN win_pen;
#endif
}
MapGC;

// Map loading

int mapinit(MapSettings *settings);

int addmap(char *path, MapSet *mapset);

int addmaps(char *path, MapSet *mapset);

int freemaps(MapSet *mapset);

int resetmap(MapState *mapstate);

int findbestmap(MapSet *mapset, double lat, double lon,
		double scale, int mode, double pixelsize);

int selectbestmap(MapSet *mapset, MapState *mapstate,
		  int mode, double pixelsize);

int coverifpossible(MapState *state, double width, double height);

// Map info

int metersperpixel(GDALDatasetH dataset, char *path, double lat,
		   double *xmpp, double *ympp);

int scale2zoom(GDALDatasetH dataset, char *path,
	       double scale, double pixelsize, double lat,
	       double *xZoom, double *yZoom);

int center2pixel(GDALDatasetH dataset,
		 GInt32 *xPixel, GInt32 *yPixel);

double mapcontains(GDALDatasetH dataset, char *path, double lat, double lon);

// Forward transforms

int wgs2geo(GDALDatasetH dataset,
	    double lat, double lon,
	    double *xGeo, double *yGeo);

int geo2pixel(GDALDatasetH dataset,
	      double xGeo, double yGeo,
	      GInt32 *xPixel, GInt32 *yPixel);

int wgs2pixel(GDALDatasetH dataset, char *path,
	      double lat, double lon,
	      GInt32 *xPixel, GInt32 *yPixel);

int wgs2state(double lat, double lon, MapState *state);

int wgs2screen(MapState *state, double lat, double lon,
	       int *xPos, int *yPos);

// Backward transforms

int geo2wgs(GDALDatasetH dataset,
	    double xGeo, double yGeo,
	    double *lat, double *lon);

int pixel2geo(GDALDatasetH dataset,
	      GInt32 xPixel, GInt32 yPixel,
	      double *xGeo, double *yGeo);

int pixel2wgs(GDALDatasetH dataset, char* path,
	      GInt32 xPixel, GInt32 yPixel,
	      double *lat, double *lon);

int screen2wgs(MapState *state, int xPos, int yPos,
	       double *lat, double *lon);

// Rendering

int drawmap(MapState *state, MapGC *mgc,
	    double viewWidth, double viewHeight, int mode);

int drawmarkers(MapGC *mgc, int width, int height,
		MapSettings *settings, MapState *state, double pixelsize,
		double angle_to_destination, double direction);

#ifdef __cplusplus
}
#endif
