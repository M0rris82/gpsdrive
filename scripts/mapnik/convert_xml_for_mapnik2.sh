#!/bin/sh
#  make adjustments to osm.xml to be compatible with Mapnik 2.0 changes
#   by Hamish Bowman  19 May 2012
#   gifted to the public domain
#
# see https://github.com/mapnik/mapnik/wiki/Mapnik2_Changes
#


# set environment so that sed a-zA-Z works properly in all locales
unset LC_ALL
LC_NUMERIC=C
export LC_NUMERIC


XML_FILE=osm-in.xml

#make a backup
cp "$XML_FILE" "$XML_FILE"~


### underscores changed to dashes

UNDERS="
allow_overlap
avoid_edges
buffer_size
character_spacing
clear_label_cache
face_name
fonset_name
halo_fill
halo_radius
horizontal_alignment
justify_alignment
label_position_tolerance
line_spacing
max_char_angle_delta
min_distance
minimum_distance
minimum_version
no_text
paths_from_xml
text_ratio
text_transform
unlock_image
vertical_alignment
wrap_before
wrap_character
wrap_width
"

for ITEM in $UNDERS ; do
    if [ `grep -c "$ITEM" "$XML_FILE"` -eq 0 ] ; then
       #echo "  no [$ITEM]s found"
       continue
    fi
    #echo "Patching [$ITEM] ..."
    NEW_ITEM=`echo "$ITEM" | tr '_' '-'`
    sed -i -e "s| $ITEM=\"| $NEW_ITEM=\"|" "$XML_FILE"
done


### CSSParameters --> Properties

OLDCSS="
PolygonSymbolizer
LineSymbolizer
RasterSymbolizer
BuildingSymbolizer
"

sed -i -e 's|<CssParameter name="\([^"]*\)">\([^<]*\)</CssParameter>|\1="\2"|' "$XML_FILE"

for ITEM in $OLDCSS ; do
   sed -i -n '1h
              1!H
	      $ {
	         g
		 s|\(<'$ITEM'\)>\([^<]*\)</'$ITEM'>|\1 \2 />|g
		 s|\([a-z0-9]\)"[^"/]*/>|\1" />|g
		 p
	        }' \
	"$XML_FILE"
done

### <horrible hack> still have some extra newlines in there to get rid of..
sed -i -n '1h
              1!H
	      $ {
	         g
		 s|\(<LineSymbolizer\)[^s]*|\1 |g
		 s|\(<PolygonSymbolizer\)[^f]*|\1 |g
		 s|"[^a-zA-Z0-9]*stroke\(-\+\)|" stroke\1|g
		 s|"[^a-zA-Z0-9]*fill\(-\+\)|" fill\1|g
		 p
	        }' \
	"$XML_FILE"


### renamed elements
sed -i -e 's|TextSymbolizer name="\([^"]*\)"|TextSymbolizer name="[\1]"|' \
       -e 's|\(TextSymbolizer.*\) text_convert|\1 text-transform|' \
       -e 's|\(TextSymbolizer.*\) min-distance|\1 minimum-distance|' \
     "$XML_FILE"


### bgcolor renamed
sed -i -e 's|<Map bgcolor=|<Map background-color=|' "$XML_FILE"


### removed elements
sed -i -e 's|\(PointSymbolizer .*\) type="[^"]*" width="[^"]*" height="[^"]*"|\1 |' \
       -e 's|\(PolygonPatternSymbolizer .*\) type="[^"]*" width="[^"]*" height="[^"]*"|\1 |' \
       -e 's|\(LinePatternSymbolizer .*\) type="[^"]*" width="[^"]*" height="[^"]*"|\1 |' \
   "$XML_FILE"

