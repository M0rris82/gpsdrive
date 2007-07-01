// $Id: mapnik.h 185 1994-06-08 08:37:25Z commiter $
// mapnik.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifdef MAPNIK
#ifndef _MAPNIK_DEF_H
#define _MAPNIK_DEF_H

void init_mapnik();
void set_mapnik_map(double pPosLatDbl, double pPosLonDbl, int pForceNewCenterYsn,  int pScaleLevelInt);
void render_mapnik ( );
unsigned char *get_mapnik_imagedata ( );
double get_mapnik_mapscale();
double get_mapnik_pixelfactor();
int get_mapnik_newmapysn();
void get_mapnik_center(double *pLatDbl, double *pLonDbl);
void get_mapnik_clacxytopos(double *pLatDbl, double *pLonDbl, int pX, int pY, int pXOffInt, int pYOffInt, int zoom);
void get_mapnik_clacxy(double *pXDbl, double *pYDbl, double pLatDbl, double pLonDbl, int pXOffInt, int pYOffInt, int pZoom);
void get_mapnik_minixy2latlon(int pXInt, int pYInt, double *pLatDbl, double *pLonDbl);
void get_mapnik_miniclacxy(double *pXDbl, double *pYDbl, double pLatDbl, double pLonDbl, int pZoom);

#endif // _MAPNIK_DEF_H
#endif // MAPNIK
