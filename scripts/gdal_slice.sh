#!/bin/sh
############################################################################
#
# MODULE:       gdal_slice.sh
#
# AUTHOR(S):    M. Hamish Bowman
#               Dept. Marine Science, University of Otago, New Zealand
#
# PURPOSE:      Slice a big GeoTIFF into lots of 1280x1024 chunks for GPSDrive
#
# COPYRIGHT:    (c) 2006-8 M.Hamish Bowman, and the GPSDrive Developement Team
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with
#               GPSDrive for details.
#

if [ $# -lt 1 ] ; then
  echo "Usage:  $0 <FILENAME.tif> [top|Map] [Png|jpg]"
  echo "use $0 --help for detailed Information"
  exit 1
fi

if echo "$@" | grep -q -e "--help" ; then
  echo "Usage:  gdal_slice.sh <FILENAME.tif> [top|Map] [Png|jpg]"
cat <<EOF

 This script will slice a big GeoTIFF (or other raster format
 known to GDAL) into many 1280x1024 tiles for use with GpsDrive.


 It assumes that the input file is:
   - A format known to GDAL
   - Is georegistered (ie it knows bounding box if only GCP are
     given the map must be run through gdalwarp first to make a
     GeoTiff first)
   - Has minimal convergence angle (true North is directly up in
     the map's native map projection).

  Requires awk, GDAL tools (from gdal.org), and NetPBM tools.

 Important! The maps must be named "map_*" for UTM-like
  projections (lat:lon = 1:cos(lat)) and "top_*" for lat/lon
  Plate carree projection (lat:lon = 1:1). The prefix is given
  so that gpsdrive knows how to scale the maps correctly.
  Alternatively the maps can be stored without prefix in
  subdirectories of $HOME/.gpsdrive/ which end in "_map" or
  "_top".  To avoid distortion, anything more than 1:150k to
  1:500k should use "top_*".

 Beware  if you are using an image originating from a map
  projection with a significant deviation between true  north
  and the map projection's local +y direction (known as the
  Convergence Angle). GpsDrive assumes that true north is always
  directly up! i.e. the Central Meridian of  the map projection
  is located at the map tile's center. For some map projections
  and areas of the country this can be a really bad  assumption
  and your map will be significantly rotated. This effect is
  most visible at map scales covering a large area. This does
  not affect lat/lon maps. The pnmrotate program may help, or
  gdalwarp to a custom, map-centered, tmerc.

 To make an overview map, open your GeoTIFF in GIMP, Image->
  Mode->RGB, Image->Scale, 25%, cubic, and save as PNG. Next use
  gdal_translate to re-ref the GeoTIFF. You can also shrink with
  gdal_translate and gdalwarp, but the output isn't as nice. i.e.
  gdalwarp lets you use cubic spline resapmpling (-rcs), which
  is great for imagery but no good for a paletted image like a
  chart. If using that multiply the original resolution (see
  gdalinfo report) by 4 and use with the gdalwarp -tr option.

  To reapply georeferencing to the GIMP image:

    gdal_translate -a_ullr w n e s  -a_srs EPSG:code \\
       \$FILE.png \$FILE.tif

EOF
  exit 1
fi

INPUT_IMG_NAME="$1"
TOPMAP="$2"
IMGFMT="$3"


# percent tiles will overlap. 33% is nice; 25% gets jumpy; 20% is minimum
OVERLAP=33

#defaults
if [ -z "$TOPMAP" ] ; then
    TOPMAP=map
fi
if [ -z "$IMGFMT" ] ; then
    IMGFMT=png
fi


if [ ! -r "$INPUT_IMG_NAME" ] ; then
   echo "Can't find file <$INPUT_IMG_NAME>"
   exit 1
fi

OUTPUT_BASE="`basename "$INPUT_IMG_NAME" .tif`"
OUTDIR="${OUTPUT_BASE}_$TOPMAP"


mkdir "$OUTDIR"
cd "$OUTDIR"
INPUT_IMG="../$INPUT_IMG_NAME"


# get image size in pixels
SIZE_STR=`gdalinfo "$INPUT_IMG" | \
   grep '^Size is' | sed -e 's/.*is //' -e 's/,//'`
SIZE_X=`echo "$SIZE_STR" | cut -f1 -d' '`
SIZE_Y=`echo "$SIZE_STR" | cut -f2 -d' '`


# build the sequence of tiles
DELTA_X=`echo $OVERLAP | awk '{print 1280 - int($1 * 1280/100.0) }'`
DELTA_Y=`echo $OVERLAP | awk '{print 1024 - int($1 * 1024/100.0) }'`
XOFF_SEQ=`seq 0 $DELTA_X $SIZE_X | head -n-1`
YOFF_SEQ=`seq 0 $DELTA_Y $SIZE_Y | head -n-1`

#left overs
LAST_X_OFF=`echo "$SIZE_X - 1280" | bc`
LAST_Y_OFF=`echo "$SIZE_Y - 1024" | bc`

#put it all together
XOFF_ALL="$XOFF_SEQ $LAST_X_OFF"
YOFF_ALL="$YOFF_SEQ $LAST_Y_OFF"


for XOFF in $XOFF_ALL ; do
  for YOFF in $YOFF_ALL ; do
    echo
    echo "Processing chunk [$XOFF $YOFF] ..."
    OUTFILE="${OUTPUT_BASE}_${XOFF}_${YOFF}"

    # create world file: -co "TFW=YES"
    gdal_translate -co "COMPRESS=PACKBITS" -quiet \
      -srcwin $XOFF $YOFF 1280 1024 "$INPUT_IMG" "${OUTFILE}.tif"

    if [ $? -ne 0 ] ; then
       echo "Quietly moving on ..."
#       if [ ! -f "${OUTFILE}.tif" ] ; then
          continue
#       fi
    fi

    # check if image is empty
    CNT=`gdalinfo -mm -noct -nomd "${OUTFILE}.tif" | grep 'Min/Max' | \
       cut -f2 -d= | tr ',' '\n' | uniq | wc -l`
    if [ "$CNT" -eq 1 ] ; then
       echo "Skipping blank image ..."
       \rm "${OUTFILE}.tif"
       continue
    fi

    # convert to PNG   (or "-of PNG" above)
    # gpsdrive with read GeoTIFFs with packbit encoding, so this is unneeded.
    #  but the png files are slightly smaller than tiffs.
    OUTNAME="${OUTFILE}.$IMGFMT"
    if [ "$IMGFMT" = "jpg" ] ; then
	tifftopnm "${OUTFILE}.tif" 2> /dev/null | pnmtojpeg > "$OUTNAME"
    else
	tifftopnm "${OUTFILE}.tif" 2> /dev/null | pnmtopng > "$OUTNAME"
    fi

    # find center (lat/lon)
    unset CENTER_STR
    unset SCALE
    CENTER_STR=`gdalinfo "${OUTFILE}.tif" | grep '^Center' | \
      cut -f3 -d'(' | cut -f1 -d')'`

    # parse and build lat/lon as decimal degree
    LON_D=`echo "$CENTER_STR" | cut -f1 -d'd' | awk '{print $1}'`
    LON_M=`echo "$CENTER_STR" | cut -f2 -d'd' | cut -f1 -d"'" | awk '{print $1}'`
    LON_S=`echo "$CENTER_STR" | cut -f2 -d"'" | cut -f1 -d'"' | awk '{print $1}'`
    LON_H=`echo "$CENTER_STR" | cut -f2 -d'"' | cut -f1 -d',' | awk '{print $1}'`
    LAT_D=`echo "$CENTER_STR" | cut -f2 -d',' | cut -f1 -d'd' | awk '{print $1}'`
    LAT_M=`echo "$CENTER_STR" | cut -f3 -d'd' | cut -f1 -d"'" | awk '{print $1}'`
    LAT_S=`echo "$CENTER_STR" | cut -f3 -d"'" | cut -f1 -d'"' | awk '{print $1}'`
    LAT_H=`echo "$CENTER_STR" | cut -f3 -d'"' | cut -f1 -d',' | awk '{print $1}'`
    LON=`echo $LON_D $LON_M  $LON_S | awk '{printf("%.9f", $1 + $2/60. + $3/3600.)}'`
    LAT=`echo $LAT_D $LAT_M  $LAT_S | awk '{printf("%.9f", $1 + $2/60. + $3/3600.)}'`
    if [ "$LON_H" = "W" ] ; then
       LON_FULL="-$LON"
    else
       LON_FULL="$LON"
    fi
    if [ "$LAT_H" = "S" ] ; then
       LAT_FULL="-$LAT"
    else
       LAT_FULL="$LAT"
    fi

    # find Pixel Size  (we need meters and same value in X and Y !)
    PIXEL_SIZES=`gdalinfo "${OUTFILE}.tif" | grep '^Pixel Size'`

    M_PER_PIXEL=`echo "$PIXEL_SIZES" | cut -f2 -d'(' | cut -f1 -d','`
    M_PER_PIXEL_Y=`echo "$PIXEL_SIZES" | cut -f2 -d',' | cut -f1 -d')' | sed -e 's/^-//'`

    if [ "$M_PER_PIXEL" != "$M_PER_PIXEL_Y" ] ; then
       echo "!!! DANGER DANGER. Pixel size differs in X and Y axes."
    fi

    PIXEL_UNITS=`gdalinfo "${OUTFILE}.tif" | grep 'UNIT' | cut -f2 -d'"'`
    if [ "$PIXEL_UNITS" = "degree" ] ; then
       # convert to meters
       M_PER_PIXEL=`echo $M_PER_PIXEL | awk '{printf("%.16g", $1 * 1852*60)}'`
    elif [ "$PIXEL_UNITS" = "feet" ] ; then
       # convert to meters  ## assume NOT US survey feet
       M_PER_PIXEL=`echo $M_PER_PIXEL | awk '{printf("%.16g", $1 * 0.3048)}'`
    fi


    #  You can figure out the "scale" factor by meters/pixel * 2817.947378
    #    2817 is maybe someones monitor dpi converted to pixels/meter??

    SCALE=`echo $M_PER_PIXEL | awk '{printf("%d", 0.5 + ($1 * 2817.947378) )}'`

    echo "$OUTDIR/$OUTNAME $LAT_FULL $LON_FULL $SCALE" >> map_koord_draft.txt

  done
done
