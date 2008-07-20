#!/bin/sh

if echo "$@" | grep -q -e '--help' ; then
    echo "This is an example script to convert Nasa Maps"
    exit -1
fi



gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 180 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_8192.tif land_shallow_topo_8192.gtiff
gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 180 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_21600.tif land_shallow_topo_21600.gtiff
gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 0 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_west.tif land_shallow_topo_west.gtiff
gdal_translate -a_srs EPSG:4326 -a_ullr 0 90 180 -90 -of GTiff -co "TILED=YES" -co "COMPRESS=DEFLATE" land_shallow_topo_east.tif land_shallow_topo_east.gtiff
