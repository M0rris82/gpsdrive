#!/bin/sh
gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 180 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_8192.tif land_shallow_topo_8192.gtiff
gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 180 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_21600.tif land_shallow_topo_21600.gtiff
gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 0 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_west.tif land_shallow_topo_west.gtiff
gdal_translate -a_srs EPSG:4326 -a_ullr 0 90 180 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_east.tif land_shallow_topo_east.gtiff
