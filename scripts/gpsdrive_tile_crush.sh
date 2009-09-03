#!/bin/sh
############################################################################
#
# MODULE:       gpstile_crush.sh
#
# AUTHOR(S):    M. Hamish Bowman
#               Dunedin, New Zealand
#
# PURPOSE:      Look through the GpsDrive map archive and try to make images smaller
#
# COPYRIGHT:    (c) 2009 M.Hamish Bowman, and the GpsDrive Developement Team
#
#               This program is free software under the GNU General Public
#               License (>=v2). Read the file COPYING that comes with
#               GPSDrive for details.
#
############################################################################


if [ $# -lt 1 ] ; then
  echo "Usage:  $0 . [-a] [-j] [-c] [-l] [map_path]"
  echo "use $0 --help for detailed Information"
  exit 1
fi

if echo "$@" | grep -q -e "--help" ; then
  echo "Usage:  gpstile_crush.sh [-a] [-j] [-c] [-l] [map_path]"
cat << EOF

 (--- Work in progress ---)

 This script will loop through your ~/.gpsdrive/maps/ directory
 and try to reduce the filesize of any image it finds. By default
 it just works on PNG images using the lossless pngcrush utility.
 Optionally you can use the -j flag to convert PNG to JPEG, you
 can use the -a flag to look in all subdirectories, not just the
 OSM_tah one, and also you can specify a map path to only process
 a single subdirectory. The -c flag cleans up after you removing
 all the empty .crushed tag files which are kept so that if you
 run this script again you do not have to waste time recrushing
 a tile which has already been processed. The -l flag makes it
 run in the background at low priority.
 
 Initially it just tries OpenStreetMap tiles, which are particularly
 bloated (saves them cycles on the server).
 
 You may wish to make backups before running this, and go out for
 a beer while it is running.

EOF
  exit 1
fi

# if [ -l flag ] ; then
   renice +18 -p $$ 
#fi


#if [ ! -j flag ]
if [ ! -x "`which pngcrush`" ] ; then
   echo "ERROR: pngcrush not available. Please install it first"
   exit 1
fi
#else jpeg mode
# if [ ! -x `which pngtopnm` ] ; then
#   echo "ERROR: NetPBM tools not available. Please install them first"
#   exit 1
# fi
#fi


if [ ! -d ~/.gpsdrive/maps/ ] ; then
   echo "Map repository not found. Nothing to do."
   exit 1
fi



cd ~/.gpsdrive/maps/
PROCDIRS="openstreetmap_tah mapnik mapnik_cache"

# TODO : command line specification of dir to process
#if [ -d "$2" ] ; then 
#   PROCDIRS="$2"
#fi

for OSMER in $PROCDIRS ; do
   if [ ! -d "$OSMER" ] ; then
      continue
   fi
   echo
   echo "Processing $OSMER ..."
   echo "Size before: `du -sh $OSMER`"
   cd "$OSMER"

   TOTALTILES=`find . | grep '\.png$' | wc -l`
   i=0
   for MAP in `find . | grep '\.png$'` ; do
      i=`expr $i + 1`
      if [ -e "$MAP.crushed" ] ; then
         continue
      fi
      echo "Crushing --|$MAP|--  ($i/$TOTALTILES) ..."

      pngcrush -brute -reduce -c 2 -q "$MAP" "$MAP.crush"

      if [ $? -eq 0 ] && [ -s "$MAP.crush" ] ; then
         BEFORE=`wc --bytes < "$MAP"`
         AFTER=`wc --bytes < "$MAP.crush"`
         \mv "$MAP.crush" "$MAP"
         touch "$MAP.crushed"  # so we don't repeat
         echo "$BEFORE $AFTER" | \
	    awk '{printf("   reduced %.1f%%\n", ($1-$2)*100.0/$1)}'
          # for JPEG we'll need to update map_koord.txt too
      else
         echo "Error crushing [$MAP]."
         \rm "$MAP.crush"
      fi
   done
   cd ..
   echo "Size after: `du -sh $OSMER`"
done


if [ "$TODO_flag" = "-c" ] ; then
  for OSMER in $PROCDIRS ; do
    if [ ! -d "$OSMER" ] ; then
       continue
    fi
    cd "$OSMER"
    for CRUFT in `find . | grep '\.png.crushed$'` ; do
       if [ -e "$MAP.crushed" ] ; then
          \rm "$MAP.crushed"
       fi
    done
    cd ..
  done
fi


echo
echo "Done."
