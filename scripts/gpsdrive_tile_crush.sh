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



print_usage()
{
  echo
  echo "USAGE: $0 . [-a] [-j] [-c] [-n] [--paths=/map/path/]"
  echo "       use $0 --help for detailed Information"
  echo
}

print_help()
{
  print_usage
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
 a tile which has already been processed. The -n flag makes it
 run at normal priority (niceness), otherwise it tries to go low.
 
 Initially it just tries OpenStreetMap tiles, which are particularly
 bloated (saves them cycles on the server).
 
 You may wish to make backups before running this, and go out for
 a beer while it is running. It may take some time.

EOF
}


# CLI parser from OSM setup-chroot script
all_subdirs=false
do_jpeg=false
do_cleanup=false
low_priority=true
proc_path=""
verbose=false
quiet=false


for arg in "$@" ; do
   arg_true=true
   arg_false=false
   case $arg in
	-a | --all)
	    all_subdirs=$arg_true
	    ;;
	-j | --jpeg)
	    do_jpeg=$arg_true
	    ;;
	-c | --cleanup)
	    do_cleanup=$arg_true
	    ;;
	-n | --normal-priority)
	    low_priority=$arg_false
	    ;;
       --paths=*) # Specify specific path to process
	    proc_path=${arg#*=}
	    ;;
	-h | --help | -help)
	    help=$arg_true
	    ;;
	--v | --verbose) #	switch on verbose output
	    verbose=$arg_true
	    quiet=$arg_false
	    ;;
	--q | --quiet) #	switch on quiet Mode
	    debug=$arg_false
	    verbose=$arg_false
	    quiet=$arg_true
	    ;;
	*)
	    echo ""
	    echo "Unknown option $arg"
	    echo ""
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


if [ "$low_priority" = "true" ] ; then
   renice +18 -p $$ 
fi

if [ "$do_jpeg" = "false" ] ; then
   if [ ! -x "`which pngcrush`" ] ; then
      echo "ERROR: pngcrush not available. Please install it first"
      exit 1
   fi
else
   if [ ! -x `which pngtopnm` ] ; then
      echo "ERROR: NetPBM tools not available. Please install them first"
      exit 1
   fi
   if [ ! -e ~/.gpsdrive/maps/map_koord.txt ] ; then
      echo "ERROR: map_koord.txt not found"
      exit 1
   fi
fi


if [ "$verbose" = "true" ] ; then
   quiet_opt=""
else
   quiet_opt="-q"
fi


if [ -z "$proc_path" ] ; then
   proc_path=~/.gpsdrive/maps/
   PROCDIRS="openstreetmap_tah mapnik mapnik_cache"

   if [ ! -d "$proc_path" ] ; then
      echo "Map repository not found at <$proc_path>. Nothing to do."
      exit 1
   fi
else
   #todo: make safe for spaces in path name
   echo "multi path= is still todo"
   PROCDIRS=`echo "$proc_path" | tr ':' ' '`
   # todo
   # for DIR in $PROCDIRS ; do 
   #   if [ ! -d "$proc_path" ] ; then
   #      echo "Map repository not found. Nothing to do."
   #      exit 1
   #   fi
   # done
fi


cd "$proc_path"


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

      if [ "$do_jpeg" = "true" ] ; then
          echo "JPEG is TODO. Haven't touched <$MAP>"

	  continue

	  JPEG_MAP=`basename $MAP .png`.jpg

	  echo "pngtopnm \"$MAP\" | pnmtojpeg --quality=85 > $JPEG_MAP"

	  if [ $? -eq 0 ] && [ -s "$JPEG_MAP" ] ; then
             BEFORE=`wc --bytes < "$MAP"`
             AFTER=`wc --bytes < "$JPEG_MAP"`
             echo "$BEFORE $AFTER" | \
	       awk '{printf("   reduced %.1f%%\n", ($1-$2)*100.0/$1)}'

	     echo \rm "$MAP"
	     # for JPEG we'll need to update map_koord.txt too
	     # FIXME
	     echo sed -i -e "s/\($MAP\)/\1.jpg/" ~/.gpsdrive/maps/map_koord.txt

          else
             echo "Error converting <$MAP>."
             \rm "$JPEG_MAP"
          fi
      else
         pngcrush -brute -reduce -c 2 $quiet_opt "$MAP" "$MAP.crush"

         if [ $? -eq 0 ] && [ -s "$MAP.crush" ] ; then
            BEFORE=`wc --bytes < "$MAP"`
            AFTER=`wc --bytes < "$MAP.crush"`
            \mv "$MAP.crush" "$MAP"
            touch "$MAP.crushed"  # so we don't repeat
            echo "$BEFORE $AFTER" | \
	       awk '{printf("   reduced %.1f%%\n", ($1-$2)*100.0/$1)}'
         else
            echo "Error crushing <$MAP>."
            \rm "$MAP.crush"
         fi
      fi
   done
   cd ..
   echo "Size after: `du -sh $OSMER`"
done


if [ "$do_cleanup" = "true" ] ; then
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
