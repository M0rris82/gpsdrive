# - Try to find GPS
# Once done this will define
#
#  GPS_FOUND - system has GPS
#  GPS_INCLUDE_DIRS - the GPS include directory
#  GPS_LIBRARIES - Link these to use GPS
#  GPS_DEFINITIONS - Compiler switches required for using GPS
#
#  Copyright (c) 2007 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

if (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)
  # in cache already
  set(GPS_FOUND TRUE)
else (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(libgps _GPSIncDir _GPSLinkDir _GPSLinkFlags _GPSCflags)

  set(GPS_DEFINITIONS ${_GPSCflags})

  find_path(GPS_INCLUDE_DIR
    NAMES
      gps.h
    PATHS
      ${_GPSIncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(GPS_LIBRARY
    NAMES
      gps
    PATHS
      ${_GPSLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(GPS_INCLUDE_DIRS
    ${GPS_INCLUDE_DIR}
  )
  set(GPS_LIBRARIES
    ${GPS_LIBRARY}
)

  if (GPS_INCLUDE_DIRS AND GPS_LIBRARIES)
     set(GPS_FOUND TRUE)
  endif (GPS_INCLUDE_DIRS AND GPS_LIBRARIES)

  if (GPS_FOUND)
    if (NOT GPS_FIND_QUIETLY)
      message(STATUS "Found GPS: ${GPS_LIBRARIES}")
    endif (NOT GPS_FIND_QUIETLY)
  else (GPS_FOUND)
    if (GPS_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GPS")
    endif (GPS_FIND_REQUIRED)
  endif (GPS_FOUND)

  # show the GPS_INCLUDE_DIRS and GPS_LIBRARIES variables only in the advanced view
  mark_as_advanced(GPS_INCLUDE_DIRS GPS_LIBRARIES)

endif (GPS_LIBRARIES AND GPS_INCLUDE_DIRS)

