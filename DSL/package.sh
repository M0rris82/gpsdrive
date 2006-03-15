###########################################################################################
# Create the Package for DSL
###########################################################################################
# Note:
# This script should be run from the current directory (./package.sh)
# Don't forget to create the package directory two levels up from DSL
#
############################################################################################
# Update Log
# Date		Name	Description
# 07/03/06	DP	Made the pagkage directory a relative path
#                       Added some error checking. (you gota love rm -rf in a scipt. ;)
############################################################################################
	LOGFILE=package.log
	LIBDIR=opt/gpsdrive/lib
	ICONDIR=`pwd`
	
	cd ../../package
	if [ "$?" != "0" ]
	then
	        echo
		echo "--------------------------------------------------------"
		echo "You will need to create the package staging area"
		echo "The default is a folder called package up two levels"
		echo "Next to the gpsdrive cvs folder"
		echo "i.e.  Up two levels from DSL"
		echo "--------------------------------------------------------"
		exit 1
	else
		echo "Current Working Directory"
		pwd
	fi

	# Remove old package staging area
	   rm -rf opt
	   rm -rf home
	   rm -rf etc
	   
	# Make the install staging dir
	   mkdir opt

	# Move the program into the pagkage staging area
	   cp -Pr /opt/gpsdrive ./opt

	# Make the Icon item
	   mkdir home
	   mkdir home/dsl
	   mkdir home/dsl/.xtdesktop
	  
	# Copy the Icon and the .lnk desktop icon file
	# 
		cp $ICONDIR/Gpsdrive.lnk home/dsl/.xtdesktop/Gpsdrive.lnk
	 	cp $ICONDIR/Gpsdrive.gif home/dsl/.xtdesktop/Gpsdrive.gif
	  
	# Add other missing files required to run the program
		# Files for libpcre3
		cp /usr/lib/libpcre.a $LIBDIR
		cp /usr/lib/libpcreposix.a $LIBDIR
		cp /usr/lib/libpcreposix.so.3.10.0 $LIBDIR/libpcreposix.so
		# ln -s $LIBDIR/libpcreposix.so $LIBDIR/libpcreposix.so.3.10.0
		# ln -s $LIBDIR/libpcreposix.so.$LIBDIR/libpcreposix.so.3.10.0
		cp /usr/lib/libpcre.so.3.10.0 $LIBDIR/libpcre.so
		# ln -s $LIBDIR/libpcre.so $LIBDIR/libpcre.so.3.10.0
		# ln -s $LIBDIR/libpcre.so.3 $LIBDIR/libpcre.so.3.10.0

	# Files for Pixbuf
		cp /usr/lib/gtk-2.0/2.4.0/engines/libpixmap.la $LIBDIR
		cp /usr/lib/gtk-2.0/2.4.0/engines/libpixmap.a $LIBDIR
		cp /usr/lib/gtk-2.0/2.4.0/engines/libpixmap.so $LIBDIR

	# Add other missing files required to run the program
	# that don't live in the gpsdrive/lib directory
	  mkdir etc
	  mkdir etc/gtk-2.0
	  mkdir etc/pango
	  cp /etc/gtk-2.0/gdk-pixbuf.loaders etc/gtk-2.0
	  cp /etc/gtk-2.0/gtk.immodules etc/gtk-2.0
	  cp /etc/pango/pango.modules etc/pango
	  cp /etc/pango/pangox.aliases etc/pango
	
	# Fix file ownership
	  chown -R 0.0 ./{opt/,etc}
	  chown -R 1001.50 ./home/dsl/

	# Remove the old archive if any
	rm -f gpsdrive.tar.gz

	# Create the file list
	 find . > files.txt

	# Remove unwanted files from the list
	# -v sends all non matching lines to the output file
	# -x matches the whole line only
	  echo "Building the files list "
	  cat files.txt \
	  | grep -vx . \
	  | grep -vx ./opt \
	  | grep -v  /include \
	  | grep -v  /doc \
	  | grep -v  /gtk-doc \
	  | grep -vx ./home \
	  | grep -vx ./home/dsl \
	  | grep -vx ./var \
	  | grep -vx ./var/tmp \
	  | grep -vx .var/tmp/mydsk.menu \
	  | grep -vx ./files.txt \
	  | grep -v  .gpsdrive.tar \
	  > includedfiles.txt
	  echo "Build file list status " $?
	
	# Create the archive

	   echo " Creating the Archive "
	   tar -cvf gpsdrive.tar --no-recursion --numeric-owner -T includedfiles.txt >> $LOGFILE
	   echo " Create Archive Status " $?

	   echo " Compressing the Archive....  please wait "
	   gzip -9 gpsdrive.tar
	   echo " Compress Archive Status " $?

	# Create an info file
	  # TODO
	
	# Create an md5sum
	  # TODO
	  # md6sum gpsdrive.tar.gz > gpsdrive.tar.gz.md5.txt
	#
	# Finished
	echo Finished
	echo The built files can be found in..... 
	pwd
################################################################
