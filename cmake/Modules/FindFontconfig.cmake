# - Try to find Fontconfig
# Once done this will define
#
#  FONTCONFIG_FOUND - system has Fontconfig
#  FONTCONFIG_INCLUDE_DIRS - the Fontconfig include directory
#  FONTCONFIG_LIBRARIES - Link these to use Fontconfig
#  FONTCONFIG_DEFINITIONS - Compiler switches required for using Fontconfig
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIRS)
  # in cache already
  set(FONTCONFIG_FOUND TRUE)
else (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(fontconfig _FONTCONFIG_INCLUDEDIR _FONTCONFIG_LIBDIR _FONTCONFIG_LDFLAGS _FONTCONFIG_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_FONTCONFIG fontconfig)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(FONTCONFIG_INCLUDE_DIR
    NAMES
      fontconfig/fontconfig.h
    PATHS
      ${_FONTCONFIG_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(FONTCONFIG_LIBRARY
    NAMES
      fontconfig
    PATHS
      ${_FONTCONFIG_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (FONTCONFIG_LIBRARY)
    set(FONTCONFIG_FOUND TRUE)
  endif (FONTCONFIG_LIBRARY)

  set(FONTCONFIG_INCLUDE_DIRS
    ${FONTCONFIG_INCLUDE_DIR}
  )

  if (FONTCONFIG_FOUND)
    set(FONTCONFIG_LIBRARIES
      ${FONTCONFIG_LIBRARIES}
      ${FONTCONFIG_LIBRARY}
    )
  endif (FONTCONFIG_FOUND)

  if (FONTCONFIG_INCLUDE_DIRS AND FONTCONFIG_LIBRARIES)
     set(FONTCONFIG_FOUND TRUE)
  endif (FONTCONFIG_INCLUDE_DIRS AND FONTCONFIG_LIBRARIES)

  if (FONTCONFIG_FOUND)
    if (NOT Fontconfig_FIND_QUIETLY)
      message(STATUS "Found Fontconfig: ${FONTCONFIG_LIBRARIES}")
    endif (NOT Fontconfig_FIND_QUIETLY)
  else (FONTCONFIG_FOUND)
    if (Fontconfig_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Fontconfig")
    endif (Fontconfig_FIND_REQUIRED)
  endif (FONTCONFIG_FOUND)

  # show the FONTCONFIG_INCLUDE_DIRS and FONTCONFIG_LIBRARIES variables only in the advanced view
  mark_as_advanced(FONTCONFIG_INCLUDE_DIRS FONTCONFIG_LIBRARIES)

endif (FONTCONFIG_LIBRARIES AND FONTCONFIG_INCLUDE_DIRS)

