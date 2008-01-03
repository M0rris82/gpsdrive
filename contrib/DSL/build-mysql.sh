# Compile MYSQL ready to be packaged for Damn Small Linxu (DSL)
# Update Log
# ===========
# Date          Init   Description
#----------------------------------------------------------------
# 07/08/2006    DP     Initial file creation
#----------------------------------------------------------------
# Note:
# This build will probably succede on debian but the package won't be
# runnable on DSL (will run ok on debian).  
# This is Something to do with libc.so.6 versions
# I did a DSL(3.01) hard disk install and build it on there.
# You might be able to build it from a livecd but I haven't tried it.
# In short you will need to build on the platform you intend to run on.
#
# Run the command using a normal user but you will need permission 
# to use the sudo command.
# (I have been having trouble getting sudo to work on debian.  You may just
# neet to run the commands individually from the command line)

# You will need the following DSL packages from the SYSTEM group
# libcurses5.dsl
# libcurses5-dev.dsl
# gcc1.dsl
#
# On Debian probably just the folloiwng if you also managed to compiled gpsdrive
# libcurses5-dev
# 
# I used the mysql-5.0.22.tar.gz source tar ball
# (5.0.24 was released but the built in test scripts failed)
# 
# Once you have unpacked the tar file, create a sub directory
# named DSL and put this script in there along with the package
# script and cd to the new DSL folder.  As stated before if you are
# builing for debian you won't be able to package for DSL, you will
# need to compile on DSL to package for DSL.
#
# You will need to initialise the database by following the instructions here
# (or it won't work)
# http://dev.mysql.com/doc/refman/5.0/en/unix-post-installation.html
#
#
# This script creates an empty data base.  You will need to create
# the gpsdrive specific data on debin.  I couldn't get the perl
# script (gpsinfo.pl) to run on DSL.  It looked like there were
# version problems.
#
# I couldn't get geoinfo.pl to run on DSL.  The build-gpdriveDB.sh
# script will create a database and back it up for you so you can 
# restore that on DSL.  Read the notes in the top of that script too.
# ---------------------------------------------------------------
#
# *** Change these *****
#
DEST=/opt/mysql
SOURCE=..
USER=mysql
GROUP=mysql
GPSDRIVEHOME=/opt/gpsdrive

######################################################################
# Compile Notes:
# This is stuff I changed from the default build script I found.
#
# configure: error: Could not find system readline or libedit libraries
#   Use --with-readline or --with-libedit to use the bundled
#   versions of libedit or readline
#
# Removed --without-readline
#
#  Other things removed/changed from suggested configure command
#
#         --sysconfdir=/etc \
#         --libexecdir=/usr/sbin \
#         --localstatedir=/srv/mysql \
#         --with-extra-charsets=all	    (Changed to =complex)
#
#         Added
#         --without-yassl       (caused error during "make test")
#         I don't think we need SSL encryption
#
######################################################################
echo " Configure"
cd $SOURCE

CFLAGS="-O3 -DPIC -fPIC -DUNDEF_HAVE_INITGROUPS -fno-strict-aliasing" \
CXXFLAGS="-O3 -fno-strict-aliasing -felide-constructors -fno-exceptions -fno-rtti -fPIC -DPIC -DUNDEF_HAVE_INITGROUPS" \
    ./configure --prefix=$DEST \
    --enable-thread-safe-client \
    --enable-assembler \
    --enable-local-infile \
    --without-debug \
    --without-yassl \
    --without-bench \
    --with-unix-socket-path=/var/run/mysqld/mysqld.sock \
    --with-extra-charsets=complex 

if [ "$?" != "0" ]
then 
	echo "error during configure :("
	exit 1
fi

echo "Running make clean"
make clean
if [ "$?" != "0"  ]
then
	echo "error during make clean :("
	exit 1
fi
echo "Running Make"
make

if [ "$?" != "0" ]
then
	echo "error during Make :("
	exit 2
fi

echo "Running Make Test"
echo "!!! This fails if run as root !!!!!"
make test
if [ "$?" != "0" ]
then
	echo "error during Make Test :("
	exit 3
fi

echo "Running Make Install"

sudo make install
if [  "$?" != "0" ]
then
	echo "error during make install :("
	exit 4
fi

echo "Setting up"

if [ ! -d $DEST/var ]
then
	sudo mkdir $DEST/var
	if [ "$?" != "0" ]
	then
		echo "error during makedir :("
		exit 8
	fi
fi

# fix the permissions
sudo chmod -R 774 $DEST/*
if [ "$?" != "0"  ]
then
	echo "error during chmod :("
	exit 5
fi

sudo chgrp -R $GROUP $DEST/*
if [ "$?" != "0" ]
then
	echo "error during chgrp :("
	exit 6
fi

sudo chown -R $USER  $DEST/*
if [ "$?" != "0" ]
then 
	echo "error during chown :("
	exit 7
fi

#
# Create the default database
# ---------------------------
#
cd $DEST/bin
./mysql_install_db --user=$USER
if [ "$?" != "0" ]
then
	echo "error creating the default database"
	exit 9
fi

echo Now follow the steps detailed in....
echo  http://dev.mysql.com/doc/refman/5.0/en/unix-post-installation.html

# Initialise the database
# -----------------------

# start the database
./mysqld_safe --user=$user &

echo The database should have also just started

 
exit 0

