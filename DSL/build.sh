LOGDIR=/build
PKGNAME=gpsdrive
DEST=/opt/gpsdrive
SOURCE=/build/source/gpsdrive
##############################################################
	echo "    Configure"
	cd $SOURCE
	./configure --prefix=$DEST >> $LOGDIR/$PKGNAME.configure.log
        if [ "$?" != "0" ]
        then
           	echo "    status " $?
        	exit 2
        else
             	echo "    Success"
        fi
	###########################################
	echo "    Make Clean"
	make clean >> $LOGDIR/$PKGNAME.makeclean.log
	if [ "$?" != "0" ]
	then
		echo "    status " $?
		exit 2
	else
	    	echo "    Success"
	fi
	###########################################
	echo "    Make "
	make >> $LOGDIR/$PKGNAME.make.log
	if [ "$?" != "0" ]
		then
	        echo "    status " $?
	        exit 2
	else
	        echo "    Success"
	fi
	#############################################
	echo "    Install"
	make install >> $LOGDIR/$PKGNAME.makeinstall.log
        if [ "$?" != "0" ]
        then
                echo "    status " $?
	        exit 2
        else
                echo "    Success"
        fi
##############################################################
