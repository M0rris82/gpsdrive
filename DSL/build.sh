##############################################################
DEST=/opt/gpsdrive
SOURCE=..
##############################################################
	cd $SOURCE
	######################################################
	echo "aclcoal"
	aclocal
	if [ "$?" != "0" ]
	then
		echo " Error During Aclocal"
		exit 1
	fi
	echo "automake"
	automake
	if [ "$?" != "0" ]
	then
		echo "Error During Automake"
		exit 1
	fi
	echo "autoconf"
	autoconf
	if [ "$?" != "0" ]
	then
		echo "Error During Autoconf"
		exit 1
	fi
	##########################################
	echo "    Configure"
	./configure --prefix=$DEST
        if [ "$?" != "0" ]
        then
           	echo "    status " $?
        	exit 2
        else
             	echo "    Success"
        fi
	###########################################
	echo "    Make Clean"
	make clean 
	if [ "$?" != "0" ]
	then
		echo "    status " $?
		exit 2
	else
	    	echo "    Success"
	fi
	###########################################
	echo "    Make "
	make 
	if [ "$?" != "0" ]
		then
	        echo "    status " $?
	        exit 2
	else
	        echo "    Success"
	fi
	#############################################
	echo "    Install"
	make install 
        if [ "$?" != "0" ]
        then
                echo "    status " $?
	        exit 2
        else
                echo "    Success"
        fi
##############################################################
