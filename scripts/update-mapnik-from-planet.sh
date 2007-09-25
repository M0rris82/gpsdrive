#!/bin/bash

echo "Create postgis database from planet dump"

echo "mirroring planet File"
planet-mirror.pl -v

planet_file=`planet-mirror.pl --print-filename`

echo "------- Drop Old Tables"	
for table in  planet_osm_roads planet_osm_polygon planet_osm_line \
    planet_osm_point spatial_ref_sys geometry_columns ; do \
    echo "DROP TABLE $table;" ;\
    done | psql gis 

echo "------- Drop complete Database"	
echo "DROP DATABASE gis" | pqsql

# ----------- Create Database and Grant rights

export user_name=`whoami`
createdb -Upostgres  -EUTF8 gis
createlang plpgsql gis
echo "GRANT ALL on geometry_columns TO \"$user_name\";" | psql -Upostgres gis
echo "GRANT ALL on spatial_ref_sys TO \"$user_name\";" | psql -Upostgres gis
echo "GRANT ALL ON SCHEMA PUBLIC TO \"$user_name\";" | psql -Upostgres gis

echo ""
echo "--------- Unpack and import $planet_file"
osm2pgsql $planet_file
