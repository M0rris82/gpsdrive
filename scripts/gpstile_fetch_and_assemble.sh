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
  echo "USAGE: `basename $0` [-c] --zoom=zoom_level --xtile=x_tile"
  echo "                     --ytile=y_tile [--filename=filename.png]"
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
 is to go with OSM/Google/Bing/etc reversed order. If the --quiet flag
 is given the program will be as non-verbose as possible.
 
 Wget, NetPBM, and OptiPNG tools are required.

EOF
}


if [ ! -x "`which wget`" ] ; then
    echo "ERROR: wget is required, please install it first" 1&>2
    exit 1
fi
if [ ! -x "`which pnmcat`" ] ; then
    echo "ERROR: pnmcat is required, please install the Netpbm tools" 1&>2
    exit 1
fi
if [ ! -x "`which optipng`" ] ; then
    echo "ERROR: optipng is required, please install it first" 1&>2
    exit 1
fi

# CLI parser from OSM setup-chroot script
strict_spec=false
xtile=""
ytile=""
zoom=""
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
            echo 1&>2
            echo "Unknown option $arg" 1&>2
            echo 1&>2
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

if [ "$strict_spec" = "true" ] ; then
   echo "support for true TMS is still todo. try back later."
   exit 1
fi

if [ -e "$filename" ] ; then
   echo "ERROR: <$filename> already exists. Will not overwrite" 1&>2
   exit 1
fi

TEMPDIR=".gpsdrive/.tmp"
if [ ! -e "$TEMPDIR" ] ; then
   mkdir ~/"$TEMPDIR"
   if [ $? -ne 0 ] ; then
      echo "ERROR: Unable to create temporary directory <$TEMPDIR>" 1&>2
      exit 1
   fi
fi

cd ~/"$TEMPDIR"


TEMPDIR2="tms_$$"
if [ ! -e "$TEMPDIR2" ] ; then
   mkdir "$TEMPDIR2"
   if [ $? -ne 0 ] ; then
      echo "ERROR: Unable to create temporary directory <$TEMPDIR/$TEMPDIR2>" 1&>2
      exit 1
   fi
fi

cd "$TEMPDIR2"

# FIXME: after initial debug replace with OpenTile or someother
BASE_URL1="http://a.tile.openstreetmap.org"
BASE_URL2="http://b.tile.openstreetmap.org"
BASE_URL3="http://c.tile.openstreetmap.org"

# url example:  http://c.tile.openstreetmap.org/9/421/193.png

LOWER_CENTER_TILE="$xtile/$ytile"

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
#	echo "ERROR: timed out -- aborting." 1&>2
#	break
#    fi

# todo: some sort of error checking, maybe just filesize not equal to zero.

    wget -nv "$BASE_URL3/$TILE_11.png" -O tms_11.png &
    wget -nv "$BASE_URL3/$TILE_21.png" -O tms_21.png &
    wget -nv "$BASE_URL2/$TILE_31.png" -O tms_31.png &
    wget -nv "$BASE_URL2/$TILE_41.png" -O tms_41.png &
    wget -nv "$BASE_URL1/$TILE_51.png" -O tms_51.png
    wait

    wget -nv "$BASE_URL3/$TILE_12.png" -O tms_12.png &
    wget -nv "$BASE_URL3/$TILE_22.png" -O tms_22.png &
    wget -nv "$BASE_URL2/$TILE_32.png" -O tms_32.png &
    wget -nv "$BASE_URL2/$TILE_42.png" -O tms_42.png &
    wget -nv "$BASE_URL1/$TILE_52.png" -O tms_52.png
    wait

    wget -nv "$BASE_URL3/$TILE_13.png" -O tms_13.png &
    wget -nv "$BASE_URL3/$TILE_23.png" -O tms_23.png &
    wget -nv "$BASE_URL2/$TILE_33.png" -O tms_33.png &
    wget -nv "$BASE_URL2/$TILE_43.png" -O tms_43.png &
    wget -nv "$BASE_URL1/$TILE_53.png" -O tms_53.png
    wait

    wget -nv "$BASE_URL3/$TILE_14.png" -O tms_14.png &
    wget -nv "$BASE_URL3/$TILE_24.png" -O tms_24.png &
    wget -nv "$BASE_URL2/$TILE_34.png" -O tms_34.png &
    wget -nv "$BASE_URL2/$TILE_44.png" -O tms_44.png &
    wget -nv "$BASE_URL1/$TILE_54.png" -O tms_54.png
    wait

#done


for file in *.png ; do
    if [ `file "$file" | grep -c PNG` -eq 0 ] ; then
       echo "ERROR: <$file> appears to be bogus."
       #exit 1
    fi
    pngtopnm "$file" > `basename "$file" .png`.pnm
done

pnmcat -lr tms_11.pnm tms_21.pnm tms_31.pnm tms_41.pnm tms_51.pnm > row1.pnm
pnmcat -lr tms_12.pnm tms_22.pnm tms_32.pnm tms_42.pnm tms_52.pnm > row2.pnm
pnmcat -lr tms_13.pnm tms_23.pnm tms_33.pnm tms_43.pnm tms_53.pnm > row3.pnm
pnmcat -lr tms_14.pnm tms_24.pnm tms_34.pnm tms_44.pnm tms_54.pnm > row4.pnm
pnmcat -tb row1.pnm row2.pnm row3.pnm row4.pnm | pnmtopng > mosaic.png

rm -f tms_*.pnm tms_*.png

optipng -o5 -quiet mosaic.png

outfile=tms_`echo "$TILE_11" | tr '/' '_'`plus.png
mv mosaic.png ../"$outfile"

cd ..
rm -rf "$TEMPDIR2"


#todo: use awk to calculate bounding box in lat/lon
# see map_download.c for the formula.
