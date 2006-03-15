DEST=/opt/gpsdrive
SOURCE=..
##############################################################
	echo "    Configure"
	cd $SOURCE
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
