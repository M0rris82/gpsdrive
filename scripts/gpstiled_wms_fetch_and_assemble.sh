#!/bin/sh
############################################################################
#
# MODULE:       gpstiled_wms_fetch_and_assemble.sh
#
# AUTHOR(S):    M. Hamish Bowman
#               Dunedin, New Zealand
#
# PURPOSE:      Given a Tiled-WMS server, lat/lon point, and zoom level,
#		 download neighbors to get a 1280x1024 mosaic.
#
# COPYRIGHT:    (c) 2012 M.Hamish Bowman, and the GpsDrive Developement Team
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with
#               GPSDrive for details.
#
############################################################################

# Currently just set up for NASA's OnEarth global LANDSAT mosaic, 512x512
# pixel tiles, aligned to grid given by the zoom level (which is given in
# degrees per 512 pixels).
# 
# http://onearth.jpl.nasa.gov
# http://onearth.jpl.nasa.gov/wms.cgi?request=GetTileService


print_usage()
{
  echo
  echo "USAGE: `basename $0` --res=tile_resolution --latitude=lat"
  echo "                     --longitude=lon [--filename=filename.jpg]"
  echo
  echo "       `basename $0` --help  for detailed Information"
  echo
}

print_help()
{
  print_usage
cat << EOF
 (--- Work in progress ---)

 This script will create a 1280x1024 map tile suitable for GpsDrive,
 given a center latitude,longitude and one of several fixed tile
 resolutions (given in degrees). Actual lat,lon will be snapped to
 the server\'s pre-determinded tiling grid. If the filename is not
 given it will be created from input parameters.  If the --verbose
 flag is given the program will tell you more about what it\'s doing.
 The  Wget and NetPBM tools are required.

 Currently this draws the Global Mosaic Base layer from NASA/JPL\'s
 OnEarth WMS imagery archive.
EOF
}
#'

# CLI parser from OSM setup-chroot script
lat=""
lon=""
res=""
filename=""
#pseudocolor=false
verbose=false

for arg in "$@" ; do
   arg_true=true
   arg_false=false
   case $arg in
#        -p | --pseudocolor)
#	    pseudocolor=$arg_true
#	    ;;
       --res=*)
            res=${arg#*=}
            ;;
       --latitude=* | --lat=*)
            lat=${arg#*=}
            ;;
       --longitude=* | --lon=*)
            lon=${arg#*=}
            ;;
       --filename=*)
            filename=${arg#*=}
            ;;
        -h | --help | -help)
            help=$arg_true
            ;;
        --v | --verbose) #        switch on quiet Mode
            verbose=$arg_true
            ;;
        *)
            echo 1>&2
            echo "Unknown option $arg" 1>&2
            echo 1>&2
            help=true
            UNKNOWN_OPTION=true
            ;;
    esac
done # END OF OPTIONS

if [ "$help" = "true" ] ; then
  print_help
  if [ -n "$UNKNOWN_OPTION" ] ; then
     exit 1
  else
     exit 0
  fi
fi

if [ -z "$lat" ] ||  [ -z "$lon" ] ||  [ -z "$res" ] ; then
   print_usage
   exit 1
fi

if [ ! -x "`which wget`" ] ; then
    echo "ERROR: wget is required, please install it first" 1>&2
    exit 1
fi
if [ ! -x "`which pnmcat`" ] ; then
    echo "ERROR: pnmcat is required, please install the Netpbm tools" 1>&2
    exit 1
fi

if [ -e "$filename" ] ; then
   echo "ERROR: <$filename> already exists. Will not overwrite" 1>&2
   exit 1
fi


#if [ "$zoom" -gt 18 ] || [ "$zoom" -lt 0 ] ; then
#   echo "ERROR: Zoom must be between 0-18" 1>&2
#   exit 1
#fi


case $res in
   0.125 | 0.25 | 0.5 | 1 | 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256)
	value="valid"
	;;
   *)
	value="invalid"
	echo "ERROR: Resolution must be one of " 1>&2
	echo "       {0.125 0.25 0.5 1 2 4 8 16 32 64 128 256}"  1>&2
	exit 1
	;;
esac


TEMPDIR=".gpsdrive/maps/.tmp"
if [ ! -e ~/"$TEMPDIR" ] ; then
   mkdir ~/"$TEMPDIR"
   if [ $? -ne 0 ] ; then
      echo "ERROR: Unable to create temporary directory <$TEMPDIR>" 1>&2
      exit 1
   fi
fi

cd ~/"$TEMPDIR"


TEMPDIR2="twms_$$"
if [ ! -e "$TEMPDIR2" ] ; then
   mkdir "$TEMPDIR2"
   if [ $? -ne 0 ] ; then
      echo "ERROR: Unable to create temporary directory <$TEMPDIR/$TEMPDIR2>" 1>&2
      exit 1
   fi
fi

cd "$TEMPDIR2"


#### setup the download

#nice todo: wget "http://onearth.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Jan&width=480&height=480&bbox=168,-46,170,-44" -O test.jpg

LAYER=global_mosaic_base

#if [ "$pseudocolor" = "true" ] ; then
#    STYLE=default
#else
    STYLE=visual
#fi

BASE_URL="http://onearth.jpl.nasa.gov/wms.cgi"
BASE_REQUEST="request=GetMap&layers=$LAYER&srs=EPSG:4326&width=512&height=512"
BASE_META="format=image/jpeg&version=1.1.1&styles=$STYLE"
URL="$BASE_URL?$BASE_REQUEST&$BBOX&$BASE_META"

# url example:  http://
bbox=-180,-166,76,90
#minx,miny,maxx,maxy


#top row
minx=$(echo "$lon $res" | awk '{printf("%.9f", $1 - $2)}')
miny="$lat"
maxx="$lon"
maxy=$(echo "$lat $res" | awk '{printf("%.9f", $1 + $2)}')
BBOX_11="$minx,$miny,$maxx,$maxy"

minx="$lon"
miny="$lat"
maxx=$(echo "$lon $res" | awk '{printf("%.9f", $1 + $2)}')
maxy=$(echo "$lat $res" | awk '{printf("%.9f", $1 + $2)}')
BBOX_21="$minx,$miny,$maxx,$maxy"

minx=$(echo "$lon $res" | awk '{printf("%.9f", $1 + $2)}')
miny="$lat"
maxx=$(echo "$lon $res" | awk '{printf("%.9f", $1 + (2 * $2))}')
maxy=$(echo "$lat $res" | awk '{printf("%.9f", $1 + $2)}')
BBOX_31="$minx,$miny,$maxx,$maxy"


#bottom row
minx=$(echo "$lon $res" | awk '{printf("%.9f", $1 - $2)}')
miny=$(echo "$lat $res" | awk '{printf("%.9f", $1 - $2)}')
maxx="$lon"
maxy="$lat"
BBOX_12="$minx,$miny,$maxx,$maxy"

minx="$lon"
miny=$(echo "$lat $res" | awk '{printf("%.9f", $1 - $2)}')
maxx=$(echo "$lon $res" | awk '{printf("%.9f", $1 + $2)}')
maxy="$lat"
BBOX_22="$minx,$miny,$maxx,$maxy"

minx=$(echo "$lon $res" | awk '{printf("%.9f", $1 + $2)}')
miny=$(echo "$lat $res" | awk '{printf("%.9f", $1 - $2)}')
maxx=$(echo "$lon $res" | awk '{printf("%.9f", $1 + (2 * $2))}')
maxy="$lat"
BBOX_32="$minx,$miny,$maxx,$maxy"


#DL_COUNT=0
#BEGIN_TIME=`date '+%s'`
#TIMEOUT=flase

#while [ "$DL_COUNT" -lt 20 ] ; do
#    CURR_TIME=`date '+%s'`
#    if [ `expr $CURR_TIME - $BEGIN_TIME` -gt 120 ] ; then
#        TIMEOUT=true
#	echo "ERROR: timed out -- aborting." 1>&2
#	break
#    fi

# todo: some sort of error checking, maybe just filesize not equal to zero.
if [ "$verbose" = "true" ] ; then
   echo "Fetching from server ..." 1>&2
fi


#top row
URL="$BASE_URL?$BASE_REQUEST&bbox=$BBOX_11&$BASE_META"
wget -nv "$URL" -O twms_11.jpg &

URL="$BASE_URL?$BASE_REQUEST&bbox=$BBOX_21&$BASE_META"
wget -nv "$URL" -O twms_21.jpg &

URL="$BASE_URL?$BASE_REQUEST&bbox=$BBOX_31&$BASE_META"
wget -nv "$URL" -O twms_31.jpg
wait


#bottom row
URL="$BASE_URL?$BASE_REQUEST&bbox=$BBOX_12&$BASE_META"
wget -nv "$URL" -O twms_12.jpg &

URL="$BASE_URL?$BASE_REQUEST&bbox=$BBOX_22&$BASE_META"
wget -nv "$URL" -O twms_22.jpg &

URL="$BASE_URL?$BASE_REQUEST&bbox=$BBOX_32&$BASE_META"
wget -nv "$URL" -O twms_32.jpg
wait

#done


#### some checks before going on
if [ `ls -1 twms_*.jpg | wc -l` -lt 6 ] ; then
    echo "ERROR: Tile(s) appear to be missing." 1>&2
    if [ "$verbose" = "true" ] ; then
	ls twms_*.jpg
    fi
    # todo: look to see which one is missing and try again.
    cd ..
    rm -rf "$TEMPDIR2"
    exit 1
fi

for file in twms_*.jpg ; do
    if [ ! -s "$file" ] ; then
	echo "ERROR: <$file> appears to be empty." 1>&2
	if [ "$verbose" = "true" ] ; then
	    ls -l twms_*.jpg
	fi
	cd ..
	rm -rf "$TEMPDIR2"
	exit 1
    fi
    if [ `file "$file" | grep -c JPEG` -eq 0 ] ; then
	echo "ERROR: <$file> appears to be bogus." 1>&2
	if [ `file "$file" | grep -c XML` -eq 1 ] && [ "$verbose" = "true" ] ; then
	   cat "$file"
	fi
	cd ..
	rm -rf "$TEMPDIR2"
	exit 1
    fi
done

if [ "$verbose" = "true" ] ; then
   echo "Converting to pnm ..." 1>&2
fi

for file in twms_*.jpg ; do
    jpegtopnm "$file" > `basename "$file" .jpg`.pnm
done


if [ "$verbose" = "true" ] ; then
   echo "Patching ..." 1>&2
fi
pnmcat -lr twms_11.pnm twms_21.pnm twms_31.pnm > row1.pnm
pnmcat -lr twms_12.pnm twms_22.pnm twms_32.pnm > row2.pnm
pnmcat -tb row1.pnm row2.pnm | pnmcut 0 0 1280 1024 | \
   pnmtojpeg --quality=75 > mosaic.jpg

rm -f twms_*.pnm twms_*.jpg row[0-9].pnm


be_quiet="-quiet"
if [ "$verbose" = "true" ] ; then
   echo "Compressing ..." 1>&2
   be_quiet=""
fi


# save final version
if [ -z "$filename" ] ; then
   outfile="top_${res}_${lat}_${lon}.jpg"
else
   outfile=`basename "$filename" .jpg`.jpg
fi

mv mosaic.jpg ../"$outfile"

cd ..
rm -rf "$TEMPDIR2"


if [ "$verbose" = "false" ] ; then
   exit
fi


##########################################
# debug: use awk to calculate bounding box
n=$(echo "$lat $res" | awk '{printf("%.9f", $1 + $2)}')
s=$(echo "$lat $res" | awk '{printf("%.9f", $1 - $2)}')
e=$(echo "$lon $res" | awk '{printf("%.9f", $1 + (1.5 * $2))}')
w=$(echo "$lon $res" | awk '{printf("%.9f", $1 - $2)}')

echo "overall bbox=$w,$s,$e,$n"

center_lat=`echo "$s $n" | awk '{printf("%.8f", ($1 + $2) / 2.0)}'`
center_lon=`echo "$w $e" | awk '{printf("%.8f", ($1 + $2) / 2.0)}'`
echo "center=$center_lat,$center_lon"
echo

exit

#? mapscale=$(calc_scale $bbox)

echo "map_koord.txt line:"
echo "$outfile   $center_lat   $center_lon   $mapscale"


