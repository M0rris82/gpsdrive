#!/bin/bash
# ============================================ 
# Run some tests on Gpsdrive


# define some colors
ESC=`echo -e "\033"`
RED="${ESC}[91m"
GREEN="${ESC}[92m"
YELLOW="${ESC}[93m"
BLUE="${ESC}[94m"
MAGENTA="${ESC}[95m"
CYAN="${ESC}[96m"
WHITE="${ESC}[97m"
BG_RED="${ESC}[41m"
BG_GREEN="${ESC}[42m"
BG_YELLOW="${ESC}[43m"
BG_BLUE="${ESC}[44m"
BG_MAGENTA="${ESC}[45m"
BG_CYAN="${ESC}[46m"
BG_WHITE="${ESC}[47m"
BRIGHT="${ESC}[01m"
UNDERLINE="${ESC}[04m"
BLINK="${ESC}[05m"
REVERSE="${ESC}[07m"
NORMAL="${ESC}[0m"


mkdir -p logs

# Option to use virtual Framebuffer.
# This way no gpsdrive is shown on the real X-Display
SHORT=false
DO_SCREENSHOTS=false
USE_XVFB=""
DO_IGNORE_GTK_ERRORS=false
DO_IGNORE_KNOWN_ERRORS=true
DO_EXIT_ON_ERROR=true

for arg in "$@" ; do
    arg_true=true
    arg_false=false
    case $arg in 
	--no-*)
	    arg_true=false
	    arg_false=true
	    arg=${arg/--no-/--}
    esac
    case $arg in
	--short) #	Do only some tests. This doesn't take longer
	    SHORT=$arg_true
	    ;;
	--use-xvfb) #	Use Virtual Framebuffer
	    if $arg_true ; then
		USE_XVFB="xvfb-run "
	    else
		USE_XVFB=""
	    fi
	    ;;

	--screenshots) #Do some Screenshots
	    DO_SCREENSHOTS=$arg_true
	    ;;

	--ignore-gdk-errors) # Ignore GTK warnings and Errors
	    DO_IGNORE_GTK_ERRORS=$arg_true
	    ;;
	--ignore-known-errors) # Ignore already known but not fixed Errors
	    DO_IGNORE_KNOWN_ERRORS=$arg_true
	    ;;

	--exit-on-error) # Exits on Errors found
	    DO_EXIT_ON_ERROR=$arg_true
	    ;;

	-h)
	    help=$arg_true
	    ;;

	--help)
	    help=$arg_true
	    ;;

	-help)
	    help=$arg_true
	    ;;

	--debug) #	Switch on debugging
	    debug=$arg_true
	    verbose=$arg_true
	    quiet=""
	    ;;

	-debug)
	    debug=$arg_true
	    verbose=$arg_true
	    quiet=""
	    ;;
	
	*)
	    echo ""
	    echo "${RED}!!!!!!!!! Unknown option $arg${NORMAL}"
	    echo ""
	    help=$arg_true
	    ;;
    esac
done # END OF OPTIONS

if [ "true" = "$help" ] ; then
    # extract options from case commands above
    options=`grep -E -e esac -e '\s*--.*\).*#' $0 | sed '/esac/,$d;s/.*--/ [--/; s/=\*)/=val]/; s/)[\s ]/]/; s/#.*\s*//; s/[\n/]//g;'`
    options=`for a in $options; do echo -n " $a" ; done`
    echo "$0 $options"
    echo "

Run various Tests for Gpsdrive
    "
    # extract options + description from case commands above
    grep -E  \
	-e 'END OF OPTIONS' \
	-e '--.*\).*#' \
	-e '^[\t\s 	]+#' \
	$0 | \
	grep -v /bin/bash | sed '/END OF OPTIONS/,$d;s/.*--/  --/;s/=\*)/=val/;s/)//;s/#//;' 
    echo "  --no-<option> Switches the desired option off"
    exit;
fi


# ------------------------------------------------------------------ GpsDrive
# Test Gpsdrive -T with different Setup
PWD=`pwd`/tests
mkdir -p "$PWD/maps"
mkdir -p "$PWD/tracks"

USER_NAME=`id -u -n`

for LANG in en_US de_DE ; do 
    echo "${BLUE}-------------> check LANG=$LANG${NORMAL}"
    for ICON_THEME in square.big square.small classic.big classic.small; do 
	echo "${BLUE}-------------> check icon_theme=$ICON_THEME${NORMAL}"
	for USER_INTERFACE in desktop car pda ; do 
	    for MAPNIK in 0 1  ; do 
		echo "${BLUE}------------------> check './build/src/gpsdrive -s -C tests/gpsdriverc -M $USER_INTERFACE -D 1 -T '  mapnik = $MAPNIK${NORMAL}"

		perl -p \
		    -e "s,PWD,$PWD,g;" \
		    -e "s/icon_theme = .*/icon_theme = $ICON_THEME/;" \
		    -e "s/mapnik = .*/mapnik = $MAPNIK/" <tests/gpsdriverc-in >tests/gpsdriverc
		cp tests/gpsdriverc tests/gpsdriverc-pre

		# --server-args="-screen 0 1280x1024x16" 
		$USE_XVFB \
		    ./build/src/gpsdrive \
		    -s \
		    -C tests/gpsdriverc \
		    -M $USER_INTERFACE \
		    -D 1 \
		    -T >logs/gpsdrive_test_$LANG.txt 2>&1 
		rc=$?

		if [ $rc != 0 ] ; then
		    cat logs/gpsdrive_test_$LANG.txt
		    echo "${RED}Error starting gpsdrive -T (rc=$rc)${NORMAL}"
		    $DO_EXIT_ON_ERROR && exit 1;
		fi
		if $DO_IGNORE_GTK_ERRORS ; then 
		    grep -v \
			-e 'Gtk-CRITICAL \*\*: gtk_widget_set_sensitive: assertion .GTK_IS_WIDGET .widget.. failed' \
			-e 'GdkPixbuf-CRITICAL \*\*: gdk_pixbuf_copy: assertion .pixbuf != NULL. failed' \
		    logs/gpsdrive_test_$LANG.txt >logs/gpsdrive_test_$LANG.err
		    mv logs/gpsdrive_test_$LANG.err  logs/gpsdrive_test_$LANG.txt
		fi
		if $DO_IGNORE_KNOWN_ERRORS ; then 
		    grep -v \
			-e 'db_postgis_query: an error occured while trying to read from the database!' \
			-e 'gda_connection_add_event: assertion .GDA_IS_CONNECTION .cnc.. failed' \
			-e 'Could not find provider PostgreSQL in the current setup' \
			-e 'cannot open image.*rendering' \
			-e 'Error: Could not find provider PostgreSQL in the current setup' \
			-e 'Unknown Config Parameter .*reminder' \
		        logs/gpsdrive_test_$LANG.txt >logs/gpsdrive_test_$LANG.err
		    mv logs/gpsdrive_test_$LANG.err  logs/gpsdrive_test_$LANG.txt
		fi

		if  grep -q -i -e 'Failed' -e 'ERROR' -e 'CRITICAL' -e 'exception' logs/gpsdrive_test_$LANG.txt; then
		    echo "Error String Check: -----------------------"
		    grep --color -i -B 2  -A 1 -e 'Failed' -e 'ERROR'  -e 'CRITICAL' -e 'exception' logs/gpsdrive_test_$LANG.txt
		    echo "${RED}Found (Error/Failed/CRITICAL) in 'gpsdrive -T output ( LANG=$LANG icon_theme=$ICON_THEME Userinterface=$USER_INTERFACE)'${NORMAL}"
		    $DO_EXIT_ON_ERROR && exit 1;
		fi

		if ! diff -u tests/gpsdriverc-pre tests/gpsdriverc ; then
		    echo "${RED}!!!!! gpsdriverc was modified by test${NORMAL}"
		    $DO_EXIT_ON_ERROR && exit 1;
		fi
	    done
	    $SHORT && continue
	done
    done
done

# -------------------------------------------- Screenshots
if $DO_SCREENSHOTS; then
    echo "------------> Screenshots...."
    for LANG in en_US de_DE ; do 
	echo "-------------> check LANG=$LANG"
	for ICON_THEME in square.big square.small classic.big classic.small; do 
	    echo "-------------> check icon_theme=$ICON_THEME"
	    for USER_INTERFACE in  desktop car pda ; do 
		for MAPNIK in 0 1  ; do 
		    
		    perl -p \
			-e "s,PWD,$PWD,g;
                        s/USER/$USER_NAME/g;
                        s/icon_theme = .*/icon_theme = $ICON_THEME/g;
                        s/mapnik = .*/mapnik = $MAPNIK/g;
                        " <tests/gpsdriverc-in >tests/gpsdriverc
		    ./build/src/gpsdrive --geometry 800x600 -S tests/ -S ./tests -C tests/gpsdriverc  -M $USER_INTERFACE >logs/gpsdrive_test_$LANG.txt 2>&1 
		    
		done || exit 1
	    done || exit 1
	done || exit 1
    done || exit 1
fi

