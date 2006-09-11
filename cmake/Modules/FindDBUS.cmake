# - Try to find DBUS
# Once done this will define
#
#  DBUS_FOUND - system has DBUS
#  DBUS_INCLUDE_DIRS - the DBUS include directory
#  DBUS_LIBRARIES - Link these to use DBUS
#  DBUS_DEFINITIONS - Compiler switches required for using DBUS
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (DBUS_LIBRARIES AND DBUS_INCLUDE_DIRS)
  # in cache already
  set(DBUS_FOUND TRUE)
else (DBUS_LIBRARIES AND DBUS_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(dbus-1 _DBUSIncDir _DBUSLinkDir _DBUSLinkFlags _DBUSCflags)

  set(DBUS_DEFINITIONS ${_DBUSCflags})

  find_path(DBUS_INCLUDE_DIR
    NAMES
      dbus/dbus.h
    PATHS
      ${_DBUSIncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(DBUS-1_LIBRARY
    NAMES
      dbus-1
    PATHS
      ${_DBUSLinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(DBUS_INCLUDE_DIRS
    ${DBUS_INCLUDE_DIR}
  )
  set(DBUS_LIBRARIES
    ${DBUS-1_LIBRARY}
)

  if (DBUS_INCLUDE_DIRS AND DBUS_LIBRARIES)
     set(DBUS_FOUND TRUE)
  endif (DBUS_INCLUDE_DIRS AND DBUS_LIBRARIES)

  if (DBUS_FOUND)
    if (NOT DBUS_FIND_QUIETLY)
      message(STATUS "Found DBUS: ${DBUS_LIBRARIES}")
    endif (NOT DBUS_FIND_QUIETLY)
  else (DBUS_FOUND)
    if (DBUS_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find DBUS")
    endif (DBUS_FIND_REQUIRED)
  endif (DBUS_FOUND)

  # show the DBUS_INCLUDE_DIRS and DBUS_LIBRARIES variables only in the advanced view
  mark_as_advanced(DBUS_INCLUDE_DIRS DBUS_LIBRARIES)

endif (DBUS_LIBRARIES AND DBUS_INCLUDE_DIRS)

