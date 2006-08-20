# Create the initial Mysql DB for Gpsdive
# Update Log
# ===========
# Date          Init   Description
#----------------------------------------------------------------------------
# 07/08/2006    DP     Initial file creation
# 19/08/2006	DP     Added database backup (so it can be restored on DSL
#----------------------------------------------------------------------------
# Note:
# This should be run on debian
# Gpsdrive should already be installed 
#
# You may need to create a dir named /var/mysql and grant write 
# permission to the user that starts mysql. (the user running this script)
# This is where mysql stores it's process information.
#
# Once this script finishes without error see the notes in the package script
# ---------------------------------------------------------------
#
#Change these if you feel the need.
#
DEST=/opt/mysql
#USER=dsl
USER=david
#GROUP=staff
GROUP=david
GPSDRIVEHOME=/opt/gpsdrive

######################################################################

# Initialise the Database for gpsdrive
# ------------------------------------

# Start the database running
$DEST/bin/mysqld_safe &
if [ "$?" != "0" ]
then
	echo "Couldn't start Mysql to initialise DB"
	echo "Could mean it is already running"
fi
echo $?

echo "Initialising the database for gpsdrive "
PERL5LIB=$GPSDRIVEHOME/share/perl5 
cd $GPSDRIVEHOME/bin
if [ "$?" != "0" ]
then
	echo "Couldnt cd to the gpsdrive bin dir"
	exit 10
fi
./geoinfo.pl --create-db --fill-defaults --db-user=root --db-password=password
if [ "$?" != "0" ]
then
	echo "error from geoinfo.pl  initialising the DB "
	exit 11
fi
# Add the open street map data
#./geoinfo.pl --create-db --db-user=root --db-password=password  --osm
if [ "$?" != "0" ]
then
	echo "error adding the Open Street Map data"
	exit 12
fi

# This creates a backup that can be restored onto DSL
echo  Backup the database so it can be restored on DSL
echo  ************************************************
echo  File is saved to ~/.gpsdrive dir with file name sqldump.yy.mm.dd_hh:mm.gz
echo  Use this file to restore the database on DSL

PATH=$PATH:$DEST/bin

./gpssql_backup.sh
if [ "$?" != "0" ]
then 
	echo error the backup command failed
	exit 13
fi

echo "Script Completed Successfully"
echo "Now run the package script"
exit 0

