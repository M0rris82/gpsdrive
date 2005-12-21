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

int metersperpixel(GDALDatasetH dataset, char *path, double lat,
		   double *xmpp, double *ympp)
{
  const char *wkt = GDALGetProjectionRef(dataset);

  if (!wkt || !strlen(wkt))
    {
      if (strstr(path, "map_"))
	{
	  double geoTransform[6];

	  if (GDALGetGeoTransform(dataset, geoTransform) != CE_None)
	    {
	      fprintf(stderr, "No valid geo transform\n");
	    }

	  *xmpp = fabs(geoTransform[1]) / PIXELFACT;
	  *ympp = fabs(geoTransform[5]) / PIXELFACT;
	  return 1;
	}
      else
	{
	  double geoTransform[6];

	  if (GDALGetGeoTransform(dataset, geoTransform) == CE_None)
	    {
#ifdef MAP_DEBUG
	      fprintf(stderr, "mpp: %lf;%lf\n",
		      fabs(geoTransform[1]) / PIXELFACT,
		      fabs(geoTransform[5]) / PIXELFACT);
#endif
	      *xmpp = fabs(geoTransform[1]) / PIXELFACT;
	      *ympp = fabs(geoTransform[5]) / PIXELFACT;
	      if (lat != 0)
		{
		  /* pseudo projection */
		  *xmpp *= fabs(cos(lat * M_PI / 180));
		}
	      return 1;
	    }
	  else
	    {
	      // No valid geo transform
	      // Treat map as world topo map
#ifdef MAP_DEBUG
	      fprintf(stderr, "Defaulting to world topo, mpp: %lf;%lf\n",
		      EQUATOR_CIRCUM / GDALGetRasterXSize(dataset),
		      POLE_CIRCUM / GDALGetRasterYSize(dataset) / 2);
#endif
	      *xmpp = EQUATOR_CIRCUM / GDALGetRasterXSize(dataset);
	      *ympp = POLE_CIRCUM / GDALGetRasterXSize(dataset);

	      if (lat != 0)
		{
		  /* pseudo projection */
		  *xmpp *= fabs(cos(lat * M_PI / 180));
		}
	      return 1;
	    }
	}
    }

  double geoTransform[6];
  OGRSpatialReferenceH srDest = OSRNewSpatialReference(wkt);

  if (GDALGetGeoTransform(dataset, geoTransform) != CE_None || !srDest)
    {
      fprintf(stderr, "No valid geo transform or spatial reference\n");
      *xmpp = EQUATOR_CIRCUM / GDALGetRasterXSize(dataset);
      *ympp = POLE_CIRCUM / GDALGetRasterYSize(dataset) / 2;
      if (lat != 0)
	{
	  /* pseudo projection */
	  *xmpp *= fabs(cos(lat * M_PI / 180));
	}
      return 0;
    }

  if (OSRIsProjected(srDest))
    {
      *xmpp = fabs(geoTransform[1]);
      *ympp = fabs(geoTransform[5]);
    }
  else
    {
      *xmpp = EQUATOR_CIRCUM * fabs(geoTransform[1]) / 360;
      *ympp = POLE_CIRCUM * fabs(geoTransform[5]) / 360;
      if (lat != 0)
	{
	  /* pseudo projection */
	  *xmpp *= fabs(cos(lat * M_PI / 180));
	}
    }

  OSRDestroySpatialReference(srDest);
  return 1;
}

int scale2zoom(GDALDatasetH dataset, char *path,
	       double scale, double pixelsize, double lat,
	       double *xZoom, double *yZoom)
{
  double xmpp;
  double ympp;

  int ret = metersperpixel(dataset, path, lat, &xmpp, &ympp);

  *xZoom = xmpp / scale / pixelsize;
  *yZoom = ympp / scale / pixelsize;

  return ret;
}

int center2pixel(GDALDatasetH dataset,
		 GInt32 *xPixel, GInt32 *yPixel)
{
  *xPixel = GDALGetRasterXSize(dataset) / 2;
  *yPixel = GDALGetRasterYSize(dataset) / 2;

  return 1;
}

double mapcontains(GDALDatasetH dataset, char *path,
		   double lat, double lon)
{
  GInt32 width = GDALGetRasterXSize(dataset);
  GInt32 height = GDALGetRasterYSize(dataset);

  GInt32 xPixel;
  GInt32 yPixel;

  double xScore;
  double yScore;

  #define CHECK_MARGIN 400.
  #define MARGIN_SCORE 0.9

  wgs2pixel(dataset, path, lat, lon, &xPixel, &yPixel);

  if (width <= 2 * CHECK_MARGIN)
    xScore = 1. - fabs((2.0 * xPixel - width) / width);
  else if (xPixel < CHECK_MARGIN)
    xScore = xPixel / CHECK_MARGIN * MARGIN_SCORE;
  else if (xPixel > width - CHECK_MARGIN)
    xScore = (width - xPixel) / CHECK_MARGIN * MARGIN_SCORE;
  else
    xScore = 1. - (1. - MARGIN_SCORE) *
      fabs((xPixel - width / 2.0) / (width / 2 - CHECK_MARGIN));

  if (height <= 2 * CHECK_MARGIN)
    yScore = 1. - fabs((2.0 * yPixel - height) / height);
  else if (yPixel < CHECK_MARGIN)
    yScore = yPixel / CHECK_MARGIN * MARGIN_SCORE;
  else if (yPixel > height - CHECK_MARGIN)
    yScore = (height - yPixel) / CHECK_MARGIN * MARGIN_SCORE;
  else
    yScore = 1. - (1. - MARGIN_SCORE) *
      fabs((yPixel - height / 2.0) / (height / 2 - CHECK_MARGIN));

  return MIN(xScore, yScore);
}

int wgs2geo(GDALDatasetH dataset,
	    double lat, double lon,
	    double *xGeo, double *yGeo)
{
  OGRSpatialReferenceH srWGS84;
  OGRSpatialReferenceH srDest;
  OGRCoordinateTransformationH trans;
  double x = lon;
  double y = lat;

  *xGeo = 0;
  *yGeo = 0;

  const char *wkt = GDALGetProjectionRef(dataset);

  if (!wkt || !strlen(wkt))
    {
      fprintf(stderr, "Unable to get projection\n");
      return 0;
    }

  // Source

  srWGS84 = OSRNewSpatialReference(NULL);
  if (!srWGS84)
    {
      fprintf(stderr, "Unable to create source spatial reference\n");
      return 0;
    }

  if (OSRSetWellKnownGeogCS(srWGS84, "WGS84") != OGRERR_NONE)
    {
      fprintf(stderr, "Unable to initialize source spatial reference\n");
      return 0;
    }

  // Destination

  srDest = OSRNewSpatialReference(wkt);
  if (!srDest)
    {
      fprintf(stderr, "Unable to create target spatial reference\n");
      return 0;
    }

#ifdef MAP_DEBUG
  if (OSRIsProjected(srDest))
    fprintf(stderr, "Projected SR\n");
  else
    fprintf(stderr, "Unprojected SR\n");
  if (OSRIsGeographic(srDest))
    fprintf(stderr, "Geographic SR\n");
  else
    fprintf(stderr, "Not geographic SR\n");
  fprintf(stderr, "Linear units: %lf\n", OSRGetLinearUnits(srDest, NULL));
#endif

  // Transform

  trans = OCTNewCoordinateTransformation(srWGS84, srDest);
  if (!trans)
    {
      fprintf(stderr, "Unable to create transform\n");
      return 0;
    }

  if (!OCTTransform(trans, 1, &x, &y, NULL))
    {
      fprintf(stderr, "Unable to transform\n");
      return 0;
    }

  *xGeo = x;
  *yGeo = y;

  // Cleanup

  OCTDestroyCoordinateTransformation(trans);
  OSRDestroySpatialReference(srWGS84);
  OSRDestroySpatialReference(srDest);

  return 1;
}

int geo2pixel(GDALDatasetH hDataset,
	      double xGeo, double yGeo,
	      GInt32 *xPixel, GInt32 *yPixel)
{
  double geoTransform[6];
  double pixelTransform[6];
  double x, y;

  if (GDALGetGeoTransform(hDataset, geoTransform) != CE_None)
    {
      fprintf(stderr, "No valid geo transform\n");

      *xPixel = xGeo;
      *yPixel = yGeo;

      return 0;
    }

  GDALInvGeoTransform(geoTransform, pixelTransform);

  GDALApplyGeoTransform(pixelTransform, xGeo, yGeo, &x, &y);

  *xPixel = x;
  *yPixel = y;

  return 1;
}

int wgs2pixel(GDALDatasetH dataset, char *path,
	      double lat, double lon,
	      GInt32 *xPixel, GInt32 *yPixel)
{
  const char *wkt = GDALGetProjectionRef(dataset);

  if (!wkt || !strlen(wkt))
    {
      if (strstr(path, "map_"))
	{
	  double geoTransform[6];

	  if (GDALGetGeoTransform(dataset, geoTransform) != CE_None)
	    {
	      fprintf(stderr, "No valid geo transform\n");
	      return 0;
	    }

	  double map_lat = geoTransform[3] + 0.5 * geoTransform[5];
	  double map_lon = geoTransform[0] + 0.5 * geoTransform[1];
	  double map_scale = geoTransform[1];

#ifdef MAP_DEBUG
	  printf("map lat lon scale %lf;%lf;%lf\n",
		 map_lat, map_lon, map_scale);
#endif

	  double x, y;

	  calcxy2(&x, &y, lat, lon,
		  map_lat, map_lon, map_scale);

	  x += 0.5 * GDALGetRasterXSize(dataset);
	  y += 0.5 * GDALGetRasterYSize(dataset);

	  *xPixel = x;
	  *yPixel = y;

#ifdef MAP_DEBUG
	  printf("map coord: %lf;%lf\n", x, y);
#endif
	}
      else
	{
	  double geoTransform[6];

	  if (GDALGetGeoTransform(dataset, geoTransform) == CE_None)
	    {
	      double map_lat = geoTransform[3] + 0.5 * geoTransform[5];
	      double map_lon = geoTransform[0] + 0.5 * geoTransform[1];
	      double map_scale = geoTransform[1];

	      *xPixel = 0.5 * GDALGetRasterXSize(dataset) +
		(lon - map_lon) / map_scale * 313687320;
	      *yPixel = 0.5 * GDALGetRasterXSize(dataset) -
		(lat - map_lat) / map_scale * 313687320;
	    }
	  else
	    {
	      // No valid geo transform
	      // Treat map as world topo map
	      *xPixel = 0.5 * GDALGetRasterXSize(dataset) +
		(lon / 360) * GDALGetRasterXSize(dataset);
	      *yPixel = 0.5 * GDALGetRasterYSize(dataset) -
		(lat / 360) * GDALGetRasterXSize(dataset);
	    }
	}
    }
  else
    {
      double xGeo;
      double yGeo;

      wgs2geo(dataset, lat, lon, &xGeo, &yGeo);
      geo2pixel(dataset, xGeo, yGeo, xPixel, yPixel);
    }

  return 1;
}

int wgs2state(double lat, double lon, MapState *state)
{
  int i;

  for (i = 0; state->dataset[i]; i++)
    wgs2pixel(state->dataset[i], state->path[i], lat, lon,
	      &state->act_xPixel[i], &state->act_yPixel[i]);

  return 1;
}

int wgs2screen(MapState *state, double lat, double lon,
	       int *xPos, int *yPos)
{
  // This is incorrect for rotated maps

  GInt32 xPixel;
  GInt32 yPixel;

  wgs2pixel(state->dataset[0], state->path[0], lat, lon, &xPixel, &yPixel);

  double xDistance = state->act_xZoom[0] * (xPixel - state->act_xPixel[0]);
  double yDistance = state->act_yZoom[0] * (yPixel - state->act_yPixel[0]);

  *xPos = 0.5 * state->act_width + xDistance;
  *yPos = 0.5 * state->act_height + yDistance;

  return 1;
}

int geo2wgs(GDALDatasetH dataset,
	    double xGeo, double yGeo,
	    double *lat, double *lon)
{
  OGRSpatialReferenceH srSrc;
  OGRSpatialReferenceH srWGS84;
  OGRCoordinateTransformationH trans;
  double x = xGeo;
  double y = yGeo;

  *lon = 0;
  *lat = 0;

  const char *wkt = GDALGetProjectionRef(dataset);

  if (!wkt || !strlen(wkt))
    {
      fprintf(stderr, "Unable to get projection\n");
      return 0;
    }

  // Source

  srSrc = OSRNewSpatialReference(wkt);
  if (!srSrc)
    {
      fprintf(stderr, "Unable to create target spatial reference\n");
      return 0;
    }

  // Destination

  srWGS84 = OSRNewSpatialReference(NULL);
  if (!srWGS84)
    {
      fprintf(stderr, "Unable to create source spatial reference\n");
      return 0;
    }

  if (OSRSetWellKnownGeogCS(srWGS84, "WGS84") != OGRERR_NONE)
    {
      fprintf(stderr, "Unable to initialize source spatial reference\n");
      return 0;
    }

  // Transform

  trans = OCTNewCoordinateTransformation(srSrc, srWGS84);
  if (!trans)
    {
      fprintf(stderr, "Unable to create transform\n");
      return 0;
    }

  if (!OCTTransform(trans, 1, &x, &y, NULL))
    {
      fprintf(stderr, "Unable to transform\n");
      return 0;
    }

  *lon = x;
  *lat = y;

  // Cleanup

  OCTDestroyCoordinateTransformation(trans);
  OSRDestroySpatialReference(srSrc);
  OSRDestroySpatialReference(srWGS84);

  return 1;
}

int pixel2geo(GDALDatasetH dataset,
	      GInt32 xPixel, GInt32 yPixel,
	      double *xGeo, double *yGeo)
{
  double geoTransform[6];
  double x, y;

  if (GDALGetGeoTransform(dataset, geoTransform) != CE_None)
    {
      fprintf(stderr, "No valid geo transform\n");

      *xGeo = xPixel;
      *yGeo = yPixel;

      return 0;
    }

  GDALApplyGeoTransform(geoTransform, xPixel, yPixel, &x, &y);

  *xGeo = x;
  *yGeo = y;

  return 1;
}

int pixel2wgs(GDALDatasetH dataset, char *path,
	      GInt32 xPixel, GInt32 yPixel,
	      double *lat, double *lon)
{
  const char *wkt = GDALGetProjectionRef(dataset);

  if (!wkt || !strlen(wkt))
    {
      if (strstr(path, "map_"))
	{
	  double geoTransform[6];

	  if (GDALGetGeoTransform(dataset, geoTransform) != CE_None)
	    {
	      fprintf(stderr, "No valid geo transform\n");
	      return 0;
	    }

	  double map_lat = geoTransform[3] + 0.5 * geoTransform[5];
	  double map_lon = geoTransform[0] + 0.5 * geoTransform[1];
	  double map_scale = geoTransform[1];

#ifdef MAP_DEBUG
	  printf("map lat lon scale w h %lf;%lf;%lf %d;%d\n",
		 map_lat, map_lon, map_scale,
		 GDALGetRasterXSize(dataset),
		 GDALGetRasterYSize(dataset));
#endif

	  calcxytopos2(xPixel - GDALGetRasterXSize(dataset) / 2,
		       yPixel - GDALGetRasterYSize(dataset) / 2,
		       lat, lon, map_lat, map_lon, map_scale);

#ifdef MAP_DEBUG
	  printf("res lat lon: %lf;%lf\n", *lat, *lon);
#endif
	}
      else
	{
	  double geoTransform[6];

	  if (GDALGetGeoTransform(dataset, geoTransform) == CE_None)
	    {
	      double map_lat = geoTransform[3] + 0.5 * geoTransform[5];
	      double map_lon = geoTransform[0] + 0.5 * geoTransform[1];
	      double map_scale = geoTransform[1];

	      *lon = (xPixel - 0.5 * GDALGetRasterXSize(dataset))
		* map_scale / 313687320 + map_lon;
 	      *lat = -(yPixel - 0.5 * GDALGetRasterXSize(dataset))
		* map_scale / 313687320 + map_lon + map_lat;
	    }
	  else
	    {
	      // No valid geo transform
	      // Treat map as world topo map
	      *lon = (xPixel - 0.5 * GDALGetRasterXSize(dataset)) * 360.0
		/ GDALGetRasterXSize(dataset);
	      *lat = (yPixel - 0.5 * GDALGetRasterYSize(dataset)) * -360.0
		/ GDALGetRasterXSize(dataset);
	    }
	}
    }
  else
    {
      double xGeo;
      double yGeo;

      pixel2geo(dataset, xPixel, yPixel, &xGeo, &yGeo);
      geo2wgs(dataset, xGeo, yGeo, lat, lon);
    }

  return 1;
}

int screen2wgs(MapState *state, int xPos, int yPos,
	       double *lat, double *lon)
{
  // This is incorrect for rotated maps

  double xDistance = xPos - 0.5 * state->act_width;
  double yDistance = yPos - 0.5 * state->act_height;

  GInt32 xPixel = state->act_xPixel[0] + xDistance / state->act_xZoom[0];
  GInt32 yPixel = state->act_yPixel[0] + yDistance / state->act_yZoom[0];

  pixel2wgs(state->dataset[0], state->path[0], xPixel, yPixel, lat, lon);

  return 1;
}
