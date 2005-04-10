#!/bin/sh
# This script uses a database snapshot if you ever need to restore your
# database.
# Usage: gpssql_restore.sh /path/to/your/backup/file
# Example: gpssql_restore.sh ~/.gpsdrive/sqldump.02.12.20_00:30.gz
# Copyleft 2002 by Sven Fichtner <sven.fichtner@flugfunk.de>

if [ "$1" = "" ]; then
		echo "Usage: gpssql_restore.sh /path/to/your/backup/file"
		echo "Example: gpssql_restore.sh ~/.gpsdrive/sqldump.02.12.20_00:30.gz"
		exit 0
fi

echo "drop database geoinfo;"| mysql -ugast -pgast
echo "create database geoinfo;"| mysql -ugast -pgast
gunzip < $1 | mysql -ugast -pgast geoinfo

