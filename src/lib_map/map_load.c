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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

int addmap(char *path, MapSet *mapset)
{
  GDALDatasetH dataset;
#ifdef MAP_DEBUG
  GDALDriverH driver;
#endif

  // Open file

  dataset = GDALOpen(path, GA_ReadOnly);

  if (!dataset)
    {
      return 0;
    }

#ifdef MAP_DEBUG
  // Driver info

  driver = GDALGetDatasetDriver(dataset);

  fprintf(stderr,
	  "driver: %s/%s\n",
	  GDALGetDriverShortName(driver),
	  GDALGetDriverLongName(driver));

  // File info

  fprintf(stderr, "image size: %d x %d x %d\n",
	 GDALGetRasterXSize(dataset),
	 GDALGetRasterYSize(dataset),
	 GDALGetRasterCount(dataset));
#endif

  mapset->dataset[mapset->maps] = dataset;

  mapset->path[mapset->maps] = (char *)malloc(strlen(path) + 1);
  strcpy(mapset->path[mapset->maps], path);

  return ++mapset->maps;
}

int addmaps(char *path, MapSet *mapset)
{
  DIR *dir;
  struct dirent *direntry;
  struct stat statbuf;

  if (stat(path, &statbuf) == -1)
    {
      fprintf(stderr, "error opening map directory '%s': %s\n",
	      path,strerror(errno));

      return 0;
    }

  if (!S_ISDIR(statbuf.st_mode))
    {
      addmap(path, mapset);
      return mapset->maps;
    }

  dir = opendir(path);

  if (!dir)
    {
      fprintf(stderr, "error opening map directory '%s': %s\n",
	      path,strerror(errno));

      return 0;
    }

  while ((direntry = readdir(dir)))
    {
      char entrypath[1024];

      if (!strcmp(direntry->d_name, ".") ||
	  !strcmp(direntry->d_name, "..") ||
	  strstr(direntry->d_name, ".wld") ||
	  strstr(direntry->d_name, ".txt"))
	continue;

      strcpy(entrypath, path);

      if (entrypath[strlen(entrypath) - 1] != '/')
	{
	  entrypath[strlen(entrypath) + 1] = 0;
	  entrypath[strlen(entrypath)] = '/';
	}

      strcat(entrypath, direntry->d_name);

      addmap(entrypath, mapset);
    }

  closedir(dir);

  return mapset->maps;
}

int freemaps(MapSet *mapset)
{
  GInt32 i;

  for (i = 0; i < mapset->maps; i++)
    {
      GDALClose(mapset->dataset[i]);
      free(mapset->path[i]);
    }

  return 1;
}

int resetmap(MapState *mapstate)
{
  int i;

  mapstate->req_lat = 0;
  mapstate->req_lon = 0;
  mapstate->req_scale = 1;
  mapstate->req_rotation = 0;
  mapstate->act_width = 0;
  mapstate->act_height = 0;
  mapstate->act_rotation = 0;

  for (i = 0; i < 9; i++)
    {
      mapstate->act_xZoom[i] = 1;
      mapstate->act_yZoom[i] = 1;
      mapstate->act_xPixel[i] = 0;
      mapstate->act_yPixel[i] = 0;
      mapstate->path[i] = NULL;
      mapstate->dataset[i] = NULL;
    }

  mapstate->refScore = 10;

  for (i = 0; i < 32; i++)
    {
      mapstate->altMaps[i] = NULL;
    }

  return 1;
}

double mapscore(GDALDatasetH dataset, char *path, double lat, double lon,
		double scale, double pixelsize)
{
  double score = mapcontains(dataset, path, lat, lon);

  double zoom;

  if (score <= 0)
    return score;

  scale2zoom(dataset, path, scale, pixelsize, 0, &zoom, &zoom);

  if (zoom < 1)
    zoom = 1 / zoom / zoom;

  return score / zoom;
}

int findbestmap(MapSet *mapset, double lat, double lon,
		double scale, int mode, double pixelsize)
{
  int ret = 0;
  double bestscore = -1e20;
  int i;

#ifdef MAP_DEBUG
  fprintf(stderr, "find: %lf;%lf %lf\n", lat, lon, scale);
#endif

  for (i = 0; i < mapset->maps; i++)
    {
      if ((mode & 0x10) && strstr(mapset->path[i], "top_"))
	continue;
      if ((mode & 0x20) && strstr(mapset->path[i], "map_"))
	continue;

      double score = mapscore(mapset->dataset[i], mapset->path[i],
			      lat, lon, scale, pixelsize);

      if (bestscore < score)
	{
	  bestscore = score;
	  ret = i;
	}

#ifdef MAP_DEBUG
      fprintf(stderr, "score: %lf best: %lf\n",
	      score, bestscore);
#endif
    }

  return ret;
}

int setaltmaps(MapSet *mapset, MapState *mapstate, int mode, double pixelsize)
{
  int altIndex = 1;
  int i;

  #define ALT_ACCEPT 0.5

  for (i = 0; i < mapset->maps && altIndex < 32; i++)
    {
      if (mapstate->altMaps[0] == mapset->dataset[i])
	continue;

      if ((mode & 0x10) && strstr(mapset->path[i], "top_"))
	continue;
      if ((mode & 0x20) && strstr(mapset->path[i], "map_"))
	continue;

      double score = mapscore(mapset->dataset[i], mapset->path[i],
			      mapstate->req_lat, mapstate->req_lon,
			      mapstate->req_scale, pixelsize);

      if (score > mapstate->refScore * ALT_ACCEPT)
	{
	  mapstate->altMaps[altIndex] = mapset->dataset[i];
	  altIndex++;
	}
    }

  fprintf(stderr, "alt maps found: %d\n", altIndex - 1);

  return altIndex;
}

int selectbestmap(MapSet *mapset, MapState *mapstate,
		  int mode, double pixelsize)
{
  double pseudoProj = (mode & 0x100) ? mapstate->req_lat : 0;
  int map = -1;
  int altIndex = -1;
  int i;

  #define NORMAL_DISCARD 0.8
  #define ALT_DISCARD 0.4

  if (mapstate->altMaps[1])
    {
      // Find current alt map index

      for (i = 0; i < 32 && mapstate->altMaps[i]; i++)
	{
	  if (mapstate->altMaps[i] == mapstate->dataset[0])
	    {
	      altIndex = i;
	      break;
	    }
	}
    }

  if (mapstate->dataset[0])
    {
      // Check if old maps are good enough

      if (altIndex != -1)
	{
	  if (mode & 0x200)
	    {
	      altIndex++;
	      if (altIndex == 32 || !mapstate->altMaps[altIndex])
		altIndex = 0;
	    }

	  for (i = 0; i < mapset->maps; i++)
	    {
	      if (mapset->dataset[i] == mapstate->altMaps[altIndex])
		{
		  map = i;
		  break;
		}
	    }

	  mapstate->dataset[0] = mapset->dataset[i];
	  mapstate->path[0] = mapset->path[i];

	  double score = mapscore(mapstate->dataset[0], mapstate->path[0],
				  mapstate->req_lat, mapstate->req_lon,
				  mapstate->req_scale, pixelsize);

	  if (score < mapstate->refScore * ALT_DISCARD)
	    map = -1;
	}
      else if (!(mode & 0x200))
	{
	  double score = mapscore(mapstate->dataset[0], mapstate->path[0],
				  mapstate->req_lat, mapstate->req_lon,
				  mapstate->req_scale, pixelsize);

	  if (score > mapstate->refScore * NORMAL_DISCARD)
	    {
#ifndef MAP_DEBUG
	      fprintf(stderr, "keeping old map: %s %lf %lf\n",
		      mapstate->path[0], score, mapstate->refScore);
#endif
	      for (i = 0; i < mapset->maps; i++)
		{
		  if (mapset->dataset[i] == mapstate->dataset[0])
		    {
		      map = i;
		      break;
		    }
		}
	    }
	}
    }

  fprintf(stderr, "map: %d alt: %d\n", map, altIndex);

  if (map != -1)
    {
      wgs2pixel(mapstate->dataset[0], mapstate->path[0],
		mapstate->req_lat, mapstate->req_lon,
		&mapstate->act_xPixel[0], &mapstate->act_yPixel[0]);

      scale2zoom(mapstate->dataset[0], mapstate->path[0],
		 mapstate->req_scale, pixelsize, pseudoProj,
		 &mapstate->act_xZoom[0], &mapstate->act_yZoom[0]);
    }
  else
    {
      map = findbestmap(mapset, mapstate->req_lat, mapstate->req_lon,
			mapstate->req_scale, mode, pixelsize);

      mapstate->dataset[0] = mapset->dataset[map];
      mapstate->path[0] = mapset->path[map];

      wgs2pixel(mapstate->dataset[0], mapstate->path[0],
		mapstate->req_lat, mapstate->req_lon,
		&mapstate->act_xPixel[0], &mapstate->act_yPixel[0]);

      scale2zoom(mapstate->dataset[0], mapstate->path[0],
		 mapstate->req_scale, pixelsize, pseudoProj,
		 &mapstate->act_xZoom[0], &mapstate->act_yZoom[0]);

      mapstate->refScore = mapscore(mapstate->dataset[0], mapstate->path[0],
				    mapstate->req_lat, mapstate->req_lon,
				    mapstate->req_scale, pixelsize);

      mapstate->altMaps[0] = mapstate->dataset[0];

      for (i = 1; i < 32; i++)
	mapstate->altMaps[i] = NULL;

      if (mode & 0x200)
	setaltmaps(mapset, mapstate, mode, pixelsize);

      if ((mode & 0xf) == 1)
	{
	  // Add background map 
	  int best = map;
	  double bestScale = 1e20;
	  double refScale;

	  scale2zoom(mapstate->dataset[0], mapstate->path[0],
		     1, pixelsize, pseudoProj, &refScale, &refScale);

	  for (i = 0; i < mapset->maps; i++)
	    {
	      double scale;
	      scale2zoom(mapset->dataset[i], mapset->path[i],
			 1, pixelsize, pseudoProj, &scale, &scale);

	      // We should check projection too!
	      if (scale > refScale && scale < bestScale &&
		  mapcontains(mapset->dataset[i], mapset->path[i],
			      mapstate->req_lat, mapstate->req_lon) > 0)
		{
		  bestScale = scale;
		  best = i;
		}
	    }

	  if (best != map)
	    {
	      mapstate->dataset[1] = mapset->dataset[best];
	      mapstate->path[1] = mapset->path[best];

	      wgs2pixel(mapstate->dataset[1], mapstate->path[1],
			mapstate->req_lat, mapstate->req_lon,
			&mapstate->act_xPixel[1], &mapstate->act_yPixel[1]);

	      scale2zoom(mapstate->dataset[1], mapstate->path[1],
			 mapstate->req_scale, pixelsize, pseudoProj,
			 &mapstate->act_xZoom[1], &mapstate->act_yZoom[1]);
	    }
	  else
	    {
	      mapstate->dataset[1] = NULL;
	      mapstate->path[1] = NULL;
	    }

	  mapstate->dataset[2] = NULL;
	  mapstate->path[2] = NULL;
	}
      else if ((mode & 0xf) == 2)
	{
	  // Add map tiles
	  int maps = 1;
	  double refScale;

	  GInt32 width = GDALGetRasterXSize(mapstate->dataset[0]);
	  GInt32 height = GDALGetRasterYSize(mapstate->dataset[0]);

	  double minLat;
	  double maxLat;
	  double minLon;
	  double maxLon;

	  scale2zoom(mapstate->dataset[0], mapstate->path[0],
		     1, pixelsize, pseudoProj, &refScale, &refScale);

	  pixel2wgs(mapstate->dataset[0], mapstate->path[0],
		    -10, -10, &maxLat, &minLon);
	  pixel2wgs(mapstate->dataset[0], mapstate->path[0],
		    width + 10, height + 10, &minLat, &maxLon);

	  for (i = 0; i < mapset->maps && maps < 8; i++)
	    {
	      double scale;

	      scale2zoom(mapset->dataset[i], mapset->path[i],
			 1, pixelsize, pseudoProj, &scale, &scale);

	      // We should check projection too!
	      if (i != map && scale == refScale &&
		  (mapcontains(mapset->dataset[i], mapset->path[i],
			       minLat, minLon) > 0 ||
		   mapcontains(mapset->dataset[i], mapset->path[i],
			       maxLat, minLon) > 0 ||
		   mapcontains(mapset->dataset[i], mapset->path[i],
			       minLat, maxLon) > 0 ||
		   mapcontains(mapset->dataset[i], mapset->path[i],
			       maxLat, maxLon) > 0 ||
		   mapcontains(mapset->dataset[i], mapset->path[i],
			       (minLat + maxLat) / 2, minLon) > 0 ||
		   mapcontains(mapset->dataset[i], mapset->path[i],
			       minLat, (minLon + maxLon) / 2) > 0 ||
		   mapcontains(mapset->dataset[i], mapset->path[i],
			       (minLat + maxLat) / 2, maxLon) > 0 ||
		   mapcontains(mapset->dataset[i], mapset->path[i],
			       maxLat, (minLon + maxLon) / 2) > 0))
		{
		  mapstate->dataset[maps] = mapset->dataset[i];
		  mapstate->path[maps] = mapset->path[i];

		  wgs2pixel(mapstate->dataset[maps], mapstate->path[1],
			    mapstate->req_lat, mapstate->req_lon,
			    &mapstate->act_xPixel[maps],
			    &mapstate->act_yPixel[1]);

		  scale2zoom(mapstate->dataset[maps], mapstate->path[maps],
			     mapstate->req_scale, pixelsize, pseudoProj,
			     &mapstate->act_xZoom[maps],
			     &mapstate->act_yZoom[maps]);

		  maps++;
		}
	    }

	  mapstate->dataset[maps] = NULL;
	  mapstate->path[maps] = NULL;
	}
      else
	{
	  // No secondary maps
	  mapstate->dataset[1] = NULL;
	  mapstate->path[1] = NULL;
	}
    }

#ifdef MAP_DEBUG
  if (mapstate->path[1])
    fprintf(stderr, "selected: %s, %s\n",
	    mapstate->path[0], mapstate->path[1]);
  else
    fprintf(stderr, "selected: %s\n", mapstate->path[0]);
#endif

  return 1;
}

int coverifpossible(MapState *state, double width, double height)
{
  int i;

  GInt32 westLeft = state->act_xPixel[0];
  GInt32 northLeft = state->act_yPixel[0];
  GInt32 eastLeft =
    GDALGetRasterXSize(state->dataset[0]) - state->act_xPixel[0];
  GInt32 southLeft =
    GDALGetRasterYSize(state->dataset[0]) - state->act_yPixel[0];

  double imageMinX = width / 2 / state->act_xZoom[0];
  double imageMinY = height / 2 / state->act_yZoom[0];

  if ((westLeft < -2 * imageMinX || eastLeft < -2 * imageMinX) ||
      (northLeft < -2 * imageMinY || southLeft < -2 * imageMinY))
    {
      return 0;
    }

  if (westLeft < imageMinX && eastLeft > imageMinX)
    {
      state->act_xPixel[0] += MIN(eastLeft - imageMinX,
				  imageMinX - westLeft);
    }
  else if (westLeft > imageMinX && eastLeft < imageMinX)
    {
      state->act_xPixel[0] -= MIN(westLeft - imageMinX,
				  imageMinX - eastLeft);
    }

  if (northLeft < imageMinY && southLeft > imageMinY)
    {
      state->act_yPixel[0] += MIN(southLeft - imageMinY,
				  imageMinY - northLeft);
    }
  else if (northLeft > imageMinY && southLeft < imageMinY)
    {
      state->act_yPixel[0] -= MIN(northLeft - imageMinY,
				  imageMinY - southLeft);
    }

  double lat;
  double lon;

  if (state->dataset[1])
    pixel2wgs(state->dataset[0], state->path[0],
	      state->act_xPixel[0], state->act_yPixel[0],
	      &lat, &lon);

  for (i = 1; state->dataset[i]; i++)
    {
      wgs2pixel(state->dataset[i], state->path[i], lat, lon,
		&state->act_xPixel[i], &state->act_yPixel[i]);
    }

  state->act_rotation = 0;

  return 1;
}
