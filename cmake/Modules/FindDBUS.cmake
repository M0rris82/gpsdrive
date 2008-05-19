# - Try to find DBUS
# Once done this will define
#
#  DBUS_FOUND - system has DBUS
#  DBUS_INCLUDE_DIRS - the DBUS include directory
#  DBUS_LIBRARIES - Link these to use DBUS
#  DBUS_DEFINITIONS - Compiler switches required for using DBUS
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
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
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(dbus-1 _DBUS_INCLUDEDIR _DBUS_LIBDIR _DBUS_LDFLAGS _DBUS_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_DBUS dbus-1)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(DBUS_INCLUDE_DIR
    NAMES
      dbus/dbus.h
    PATHS
      ${_DBUS_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(DBUS-1_LIBRARY
    NAMES
      dbus-1
    PATHS
      ${_DBUS_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (DBUS-1_LIBRARY)
    set(DBUS-1_FOUND TRUE)
  endif (DBUS-1_LIBRARY)

  set(DBUS_INCLUDE_DIRS
    ${DBUS_INCLUDE_DIR}
  )

  if (DBUS-1_FOUND)
    set(DBUS_LIBRARIES
      ${DBUS_LIBRARIES}
      ${DBUS-1_LIBRARY}
    )
  endif (DBUS-1_FOUND)

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

