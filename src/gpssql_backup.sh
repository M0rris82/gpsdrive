#!/bin/sh
# This script takes a snapshot of the database used by GpsDrive.
# The backed up file can be found in your ~/.gpsdrive/ directory and
# contains a time stamp for better separation in its name.
# Example: ~/.gpsdrive/sqldump.02.12.20_00:30.gz was created on Dec, 20th at
# 00:30am.
# As this script requires no user interaction it might be used in your crontab.
# If you ever need to restore your database, simply run gpssql_restore.sh
# followed by the full path to your backup.
# Usage: gpssql_backup.sh
# Copyleft 2002 by Sven Fichtner <sven.fichtner@flugfunk.de>

mysqldump -ugast -pgast geoinfo | gzip \
	>~/.gpsdrive/sqldump.`date +%y.%m.%d_%k:%M`.gz
	
