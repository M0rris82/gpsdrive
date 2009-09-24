#!/bin/sh

if echo "$@" | grep -q -e '--help' ; then
    echo "This is an example script to convert raw Nasa Maps into GeoTIFFs"
    echo "See the README.nasamaps file that comes with the GpsDrive documentation"
    exit 0
fi

# Better:
#  just create a "world file" of the same name as the .tif but with a .wld extension
#
# The 0.0083333 is cell resolution (in degrees, 0.5'/60) and N,S,E,W
#  boundaries are reduced 1/2 a cell as these refer to cell centers of
#  the outer rows, not the outer boundaries of the cell.
#
#     land_shallow_topo_east.wld 
# 0.008333333333333
# 0.000000000000000
# 0.000000000000000
# -0.008333333333333
# 0.00416666666666665
# 89.99583333333334
# 
#     land_shallow_topo_west.wld 
# 0.008333333333333
# 0.000000000000000
# 0.000000000000000
# -0.008333333333333
# -179.9958333333333
# 89.99583333333334
# 

if [ -e land_shallow_topo_8192.tif ] ; then
   gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 180 -90 -of GTiff -co "TILED=YES" \
      -co "COMPRESS=DEFLATE" land_shallow_topo_8192.tif land_shallow_topo_8192.gtiff
fi


if [ -e land_shallow_topo_21600.tif ] ; then
   gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 180 -90 -of GTiff -co "TILED=YES" \
      -co "COMPRESS=DEFLATE" land_shallow_topo_21600.tif land_shallow_topo_21600.gtiff
fi


if [ -e land_shallow_topo_west.tif ] ; then
   gdal_translate -a_srs EPSG:4326 -a_ullr -180 90 0 -90 -of GTiff -co "TILED=YES" \
      -co "COMPRESS=DEFLATE" land_shallow_topo_west.tif land_shallow_topo_west.gtiff
fi


if [ -e land_shallow_topo_east.tif ] ; then
   gdal_translate -a_srs EPSG:4326 -a_ullr 0 90 180 -90 -of GTiff -co "TILED=YES" \
      -co "COMPRESS=DEFLATE" land_shallow_topo_east.tif land_shallow_topo_east.gtiff
fi
