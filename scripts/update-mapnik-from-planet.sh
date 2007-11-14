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
sudo -u postgres dropdb -Upostgres   gis

# ----------- Create Database and Grant rights

export user_name=`whoami`
sudo -u postgres createdb -Upostgres  -EUTF8 gis
sudo -u postgres createlang plpgsql gis
sudo -u postgres osm2pgsql --create --database gis

echo "GRANT ALL on geometry_columns TO \"$user_name\";" | sudo -u postgres psql -Upostgres gis
echo "GRANT ALL on spatial_ref_sys TO \"$user_name\";" | sudo -u postgres psql -Upostgres gis
echo "GRANT ALL ON SCHEMA PUBLIC TO \"$user_name\";" | sudo -u postgres psql -Upostgres gis

echo ""
echo "--------- Unpack and import $planet_file"
sudo -u postgres osm2pgsql $planet_file
