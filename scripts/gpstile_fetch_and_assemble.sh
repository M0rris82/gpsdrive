#!/bin/sh
############################################################################
#
# MODULE:       gpstile_fetch_and_assemble.sh
#
# AUTHOR(S):    M. Hamish Bowman
#               Dunedin, New Zealand
#
# PURPOSE:      Given a TMS tile and zoom level, download neighbors to get
#		 a 1280x1024 mosaic
#
# COPYRIGHT:    (c) 2012 M.Hamish Bowman, and the GpsDrive Developement Team
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with
#               GPSDrive for details.
#
############################################################################


print_usage()
{
  echo
  echo "USAGE: `basename $0` [-c ] --zoom=zoom_level --xtile=x_tile"
  echo "                     --ytile=y_tile [--filename=filename.png]"
  echo "                     [--datasource=...]"
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
 given the TMS zoom level and tile x,y. If filename is not given it
 will be created from input parameters. The -c flag indicates compliance
 with the OSGeo TMS spec, i.e. not reversed y-ordering. The default
 is to go with OSM/Google/Bing/etc reversed order. The --datasource
 option specifies the tile server to use. Currently supported are
 osm_mapnik, osm_cycle, osm_transport, mapquest_open, mapquest_aerial.
 The default (for now) is OSM\'s Mapnik. Please don\'t abuse other peoples\'
 servers too badly. (You won\'t get very far, this is about the least
 efficient method you could try with) If the --verbose flag is given
 the program will tell you more about what it\'s doing. The  Wget,
 NetPBM, and OptiPNG tools are required.
 Zoom levels 0-18, less for OpenCycleMap and MapQuest Aerial.
 MapQuest serves JPEGs.

EOF
}
#'

# CLI parser from OSM setup-chroot script
strict_spec=false
xtile=""
ytile=""
zoom=""
datasource="osm_mapnik"
filename=""
verbose=false

for arg in "$@" ; do
   arg_true=true
   arg_false=false
   case $arg in
        -c | --compliant)
            strict_spec=$arg_true
            ;;
       --zoom=*)
            zoom=${arg#*=}
            ;;
       --xtile=*)
            xtile=${arg#*=}
            ;;
       --ytile=*)
            ytile=${arg#*=}
            ;;
       --datasource=*)
            datasource=${arg#*=}
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

if [ -z "$xtile" ] ||  [ -z "$ytile" ] ||  [ -z "$zoom" ] ; then
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
if [ ! -x "`which optipng`" ] ; then
    echo "ERROR: optipng is required, please install it first" 1>&2
    exit 1
fi

if [ "$strict_spec" = "true" ] ; then
   echo "support for true TMS is still todo. try back later."
   exit 1
fi

if [ -e "$filename" ] ; then
   echo "ERROR: <$filename> already exists. Will not overwrite" 1>&2
   exit 1
fi


if [ "$zoom" -gt 18 ] || [ "$zoom" -lt 0 ] ; then
   echo "ERROR: Zoom must be between 0-18" 1>&2
   exit 1
fi

if [ "$datasource" = "osm_cycle" ] && [ "$zoom" -gt 16 ] ; then
   echo "ERROR: OpenCycleMap only goes to zoom level 16" 1>&2
   exit 1
elif [ "$datasource" = "mapquest_aerial" ] && [ "$zoom" -gt 11 ] ; then
   echo "ERROR: MapQuest Open Aerial only goes to zoom level 11 worldwide" 1>&2
   exit 1
fi


TEMPDIR=".gpsdrive/maps/.tmp"
if [ ! -e ~/"$TEMPDIR" ] ; then
   mkdir ~/"$TEMPDIR"
   if [ $? -ne 0 ] ; then
      echo "ERROR: Unable to create temporary directory <$TEMPDIR>" 1>&2
      exit 1
   fi
fi

cd ~/"$TEMPDIR"


TEMPDIR2="tms_$$"
if [ ! -e "$TEMPDIR2" ] ; then
   mkdir "$TEMPDIR2"
   if [ $? -ne 0 ] ; then
      echo "ERROR: Unable to create temporary directory <$TEMPDIR/$TEMPDIR2>" 1>&2
      exit 1
   fi
fi

cd "$TEMPDIR2"

# FIXME: after initial debug replace with OpenTile or someother

case "$datasource" in
   osm_mapnik)
	BASE_URL1="http://a.tile.openstreetmap.org"
	BASE_URL2="http://b.tile.openstreetmap.org"
	BASE_URL3="http://c.tile.openstreetmap.org"
	BASE_URL4="$BASE_URL1"
	FMT=png
	;;
   osm_cycle)
	BASE_URL1="http://a.tile.opencyclemap.org/cycle"
	BASE_URL2="http://b.tile.opencyclemap.org/cycle"
	BASE_URL3="http://c.tile.opencyclemap.org/cycle"
	BASE_URL4="$BASE_URL1"
	FMT=png	
	;;
   osm_transport)
	BASE_URL1="http://a.tile2.opencyclemap.org/transport"
	BASE_URL2="http://b.tile2.opencyclemap.org/transport"
	BASE_URL3="http://c.tile2.opencyclemap.org/transport"
	BASE_URL4="$BASE_URL1"
	FMT=png
	;;
   mapquest_open)
	BASE_URL1="http://otile1.mqcdn.com/tiles/1.0.0/osm"
	BASE_URL2="http://otile2.mqcdn.com/tiles/1.0.0/osm"
	BASE_URL3="http://otile3.mqcdn.com/tiles/1.0.0/osm"
	BASE_URL4="http://otile4.mqcdn.com/tiles/1.0.0/osm"
	FMT=jpg
	;;
   mapquest_aerial)
	BASE_URL1="http://otile1.mqcdn.com/naip"
	BASE_URL2="http://otile2.mqcdn.com/naip"
	BASE_URL3="http://otile3.mqcdn.com/naip"
	BASE_URL4="http://otile4.mqcdn.com/naip"
	FMT=jpg
	;;
   *)
	echo "ERROR: Unknown data source '$datasource'"
	exit 1
	;;
esac



# url example:  http://c.tile.openstreetmap.org/9/421/193.png

LOWER_CENTER_TILE="$xtile/$ytile"
if [ "$verbose" = "true" ] ; then
   echo "Lower center tile is $xtile,$ytile, zoom level $zoom" 1>&2
fi
TILE_11="$zoom/`expr $xtile - 2`/`expr $ytile - 2`"
TILE_21="$zoom/`expr $xtile - 1`/`expr $ytile - 2`"
TILE_31="$zoom/$xtile/`expr $ytile - 2`"
TILE_41="$zoom/`expr $xtile + 1`/`expr $ytile - 2`"
TILE_51="$zoom/`expr $xtile + 2`/`expr $ytile - 2`"

TILE_12="$zoom/`expr $xtile - 2`/`expr $ytile - 1`"
TILE_22="$zoom/`expr $xtile - 1`/`expr $ytile - 1`"
TILE_32="$zoom/$xtile/`expr $ytile - 1`"
TILE_42="$zoom/`expr $xtile + 1`/`expr $ytile - 1`"
TILE_52="$zoom/`expr $xtile + 2`/`expr $ytile - 1`"

TILE_13="$zoom/`expr $xtile - 2`/$ytile"
TILE_23="$zoom/`expr $xtile - 1`/$ytile"
TILE_33="$zoom/$LOWER_CENTER_TILE"
TILE_43="$zoom/`expr $xtile + 1`/$ytile"
TILE_53="$zoom/`expr $xtile + 2`/$ytile"

TILE_14="$zoom/`expr $xtile - 2`/`expr $ytile + 1`"
TILE_24="$zoom/`expr $xtile - 1`/`expr $ytile + 1`"
TILE_34="$zoom/$xtile/`expr $ytile + 1`"
TILE_44="$zoom/`expr $xtile + 1`/`expr $ytile + 1`"
TILE_54="$zoom/`expr $xtile + 2`/`expr $ytile + 1`"

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
    wget -nv "$BASE_URL4/$TILE_11.$FMT" -O tms_11.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_21.$FMT" -O tms_21.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_31.$FMT" -O tms_31.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL2/$TILE_41.$FMT" -O tms_41.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL1/$TILE_51.$FMT" -O tms_51.$FMT
    wait

    wget -nv "$BASE_URL4/$TILE_12.$FMT" -O tms_12.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_22.$FMT" -O tms_22.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_32.$FMT" -O tms_32.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL2/$TILE_42.$FMT" -O tms_42.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL1/$TILE_52.$FMT" -O tms_52.$FMT
    wait

    wget -nv "$BASE_URL4/$TILE_13.$FMT" -O tms_13.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_23.$FMT" -O tms_23.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_33.$FMT" -O tms_33.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL2/$TILE_43.$FMT" -O tms_43.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL1/$TILE_53.$FMT" -O tms_53.$FMT
    wait

    wget -nv "$BASE_URL4/$TILE_14.$FMT" -O tms_14.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_24.$FMT" -O tms_24.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL3/$TILE_34.$FMT" -O tms_34.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL2/$TILE_44.$FMT" -O tms_44.$FMT &
    sleep 0.08
    wget -nv "$BASE_URL1/$TILE_54.$FMT" -O tms_54.$FMT
    wait
#done


if [ `ls -1 *.$FMT | wc -l` -lt 20 ] ; then
    echo "ERROR: Tile(s) appear to be missing."
    ls *.$FMT
    # todo: look to see which one is missing and try again.
    #exit 1
fi

if [ "$verbose" = "true" ] ; then
   echo "Converting to pnm ..." 1>&2
fi

for file in *.$FMT ; do
    if [ ! -s "$file" ] ; then
       echo "ERROR: <$file> appears to be empty."
       #exit 1
    fi
done

for file in *.png ; do
    if [ `file "$file" | grep -c PNG` -eq 0 ] ; then
       echo "ERROR: <$file> appears to be bogus."
       #exit 1
    fi
    pngtopnm "$file" > `basename "$file" .png`.pnm
done
for file in *.jpg ; do
    if [ `file "$file" | grep -c JPEG` -eq 0 ] ; then
       echo "ERROR: <$file> appears to be bogus."
       #exit 1
    fi
    jpegtopnm "$file" > `basename "$file" .png`.pnm
done


if [ "$verbose" = "true" ] ; then
   echo "Patching ..." 1>&2
fi
pnmcat -lr tms_11.pnm tms_21.pnm tms_31.pnm tms_41.pnm tms_51.pnm > row1.pnm
pnmcat -lr tms_12.pnm tms_22.pnm tms_32.pnm tms_42.pnm tms_52.pnm > row2.pnm
pnmcat -lr tms_13.pnm tms_23.pnm tms_33.pnm tms_43.pnm tms_53.pnm > row3.pnm
pnmcat -lr tms_14.pnm tms_24.pnm tms_34.pnm tms_44.pnm tms_54.pnm > row4.pnm
pnmcat -tb row1.pnm row2.pnm row3.pnm row4.pnm | pnmtopng > mosaic.png

rm -f tms_*.pnm tms_*.png


be_quiet="-quiet"
if [ "$verbose" = "true" ] ; then
   echo "Compressing ..." 1>&2
   be_quiet=""
fi

if [ $FMT = "png" ] ; then
    optipng -o5 $be_quiet mosaic.png
else
    pngtopnm mosaic.png | pnmtojpeg > mosaic.jpg
fi


# save final version
if [ -z "$filename" ] ; then
   outfile=map_`echo "$TILE_33" | tr '/' '_'`.$FMT
else
   outfile=`basename "$filename" .png`.$FMT
fi

mv mosaic.$FMT ../"$outfile"

cd ..
rm -rf "$TEMPDIR2"


if [ "$verbose" = "false" ] ; then
   exit
fi



#########################################################################
# debug: use awk to calculate bounding box in lat/lon
xtile2lat()
{
   xtile=$1
   zoom=$2
   echo "$xtile $zoom" | awk '{printf("%.9f", $1 / 2.0^$2 * 360.0 - 180)}'
}

ytile2lon()
{
   ytile=$1
   zoom=$2
   
   echo "$ytile $zoom" | awk -v PI=3.14159265358979323846 '{
      num_tiles = PI - 2.0 * PI * $1 / 2.0^$2;
      printf("%.9f", 180.0 / PI * atan2(0.5 * (exp(num_tiles) - exp(-num_tiles)),1)); }'
}


n=$(ytile2lon `expr $ytile - 2` $zoom)
s=$(ytile2lon `expr $ytile + 1 + 1` $zoom)
e=$(xtile2lat `expr $xtile + 2 + 1` $zoom)
w=$(xtile2lat `expr $xtile - 2` $zoom)

echo "bbox=$w,$s,$e,$n"

center_lat=`echo "$s $n" | awk '{printf("%.8f", ($1 + $2) / 2.0)}'`
center_lon=`echo "$w $e" | awk '{printf("%.8f", ($1 + $2) / 2.0)}'`
echo "center=$center_lat,$center_lon"
echo

calc_webtile_scale()
{
    lat=$1
    scale=$2
    echo "$lat $scale" | awk -v PI=3.14159265358979323846 \
      -v PIXELFACT=2817.947378 '{
       a = 6378137.0;
       printf("%.0f", (a * 2*PI * cos($1 * PI/180) * PIXELFACT) / (256 * 2^$2))
    
    }'
}

mapscale=$(calc_webtile_scale $center_lat $zoom)

echo "map_koord.txt line:"
echo "$outfile   $center_lat   $center_lon   $mapscale"


