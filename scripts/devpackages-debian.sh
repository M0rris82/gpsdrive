#!/bin/bash

# Packages required to build Gpsdrive`
# =====================================
# Notes:
# This script has been tested on Debian Stable (Etch) 
# It is now considered that the Debian Sarge packages are no longer
# sufficient to compile gpsdrive.
# 
# Minimum required versions are not checked for here. 
# ./configure will report some version problems but not all.
#
# If an individual package has no dependencies it will be installed without 
# question.  If a package had dependencies you will be asked to confirm (Y/n)
#
# ===========================================================================
# Date         Initials    Desc.
# 06/08/2006   DP          Initial Release
# 06/17/2007   d.s.e       Adaption to Debian Stable (Etch)
# 20 Aug 2007  HB          Add libfile-slurp-perl and partial mapnik depends (Etch)
#
# ===========================================================================


# Update the package information
apt-get update


apt-get install \
	libart-2.0-2 \
	libart-2.0-dev \
	libatk1.0-0 \
	libatk1.0-dev \
	libc6 \
	libc6-dev \
	libcairo2 \
	libcairo2-dev \
	libexpat1 \
	libexpat1-dev \
	libfile-slurp-perl \
	libfontconfig1 \
	libfontconfig1-dev \
	libfreetype6 \
	libfreetype6-dev \
	libgcc1	\
	libglib2.0-0 \
	libglib2.0-dev \
	libgtk2.0-0 \
	libgtk2.0-dev \
	libpango1.0-0 \
	libpango1.0-dev \
	libpcre3 \
	libpcre3-dev \
	libpng12-0 \
	libpng12-dev \
	libstdc++6 \
	libstdc++6-dev \
	libx11-6 \
	libxcursor-dev \
	libxext6 \
	libxext-dev \
	libxi6 \
	libxi-dev \
	libxinerama-dev \
	zlib1g \
	zlib1g-dev \
	libltdl3-dev \
	libgdal1-1.3.2 \
	libgdal1-1.3.2-grass \
	libgdal1-1.3.2-dev \
	gdal-bin \
	libxerces27 \
	libxerces27-dev \
	libmysqlclient15-dev \
	libtiff4-dev \
	libjasper-1.701-dev \
	libgeos-dev \
	libgrass-dev \
	libhdf4g-dev \
	libungif4-dev \
	netcdfg-dev \
	libcfitsio-dev \
	libpqxx-dev \
	libwww-mechanize-perl \
	libarchive-zip-perl \
	libdate-manip-perl \
	libhtml-parser-perl \
	libxml-parser-perl \
	libtext-query-perl \
	libwww-mechanize-perl \
	libwww-perl \
	libdbi-perl \
	perl \
	perl-base \
	perlmagick \
	perl-modules \
	imagemagick \
	automake1.9 \
	autoconf \
	libtool \
	gcc \
	g++ \
	mapnik \
	mapnik-plugins \
	mapnik-utils \
	python-mapnik
#  libdbi-perl (This one didn't like being in with the others.

if [ "$?" != "0"  ]
then 
	echo "!!! ERROR !!! "
	echo "An error occured during the apt-get install process"
	echo "Review the error message, fix the problem and rerun the script"
else
	echo "Success"
	echo "The apt-get install process completed successfully"
fi


# Other stuff I'm not sure if it is required
# ==========================================
# > Couldn't find this one in debian testing (etch)
# > aclocal 
#
# HB: If a file is not installed on the system use apt-file to find which package(s) supply it:
#
# $ apt-file search usr/bin/aclocal
# automake [...]
#
# If the file is installed, use "dpkg -S usr/bin/aclocal" to find out which package provided the file.
# Note that in Debian 'aclocal' and 'automake' are both symlinks from /etc/alternatives.
#  (ie you have to adjust both of the symlinks, not just automake)
