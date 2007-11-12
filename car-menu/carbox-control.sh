#/bin/bash
#
# CarBox-Control v0.1
#
# (c) 2007 Guenther Meyer < d.s.e @ sordidmusic.com >
#
# This is a small script to manage applications on a car PC with touchscreen.
# Just put a call to it with the right argument into your favourite panel,
# and the wanted application will be raised and started if necessary.

# ------------------------------------------------------------
# --- Configuration Section START ----------------------------
# ---   win_* is the title of your application window      ---
# ---   app_* is the commandline to start the application  ---

win_navigation="gpsdrive"
app_navigation="/usr/bin/gpsdrive -M car"

win_media="player"
app_media="/usr/bin/rhythmbox"

win_vehicle="...title..."
app_vehicle="echo not configured"

win_logbook="...title..."
app_logbook="echo not configured"

win_settings="...title..."
app_settings="echo not configured"

# --- Configuration Section END ------------------------------
# ------------------------------------------------------------

version="CarBox-Control v0.1"

case $1 in

  "navigation" )
    if wmctrl -l |grep -i -q $win_navigation;
      then echo "Navigation already running.";
      else echo "Starting Navigation" && exec $app_navigation;
    fi
    wmctrl -a $win_navigation
  ;;

  "media" )
    if wmctrl -l |grep -i -q $win_media;
      then echo "Media Player already running.";
      else echo "Starting Media Player" && exec $app_media;
    fi
    wmctrl -a $win_media
  ;;

  "vehicle" )
    if wmctrl -l |grep -i -q $win_vehicle;
      then echo "OBD-Dashboard already running.";
      else echo "Starting OBD-Dashboard" && exec $app_vehicle;
    fi
    wmctrl -a $win_vehicle
  ;;

  "logbook" )
    if wmctrl -l |grep -i -q $win_logbook;
      then echo "Logbook already running.";
      else echo "Starting Logbook Application" && exec $app_logbook;
    fi
    wmctrl -a $win_logbook
  ;;

  "settings" )
    if wmctrl -l |grep -i -q $win_settings;
      then echo "Settings Application already running.";
      else echo "Starting Settings" && exec $app_settings;
    fi
    wmctrl -a $win_settings
  ;;

  * )
    echo $version
    echo
    echo "Possible options:"
    echo "  navigation"
    echo "  media"
    echo "  vehicle"
    echo "  logbook"
    echo "  settings"
    echo
  ;;

esac

