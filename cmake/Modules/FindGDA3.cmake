# - Try to find GDA3
# Once done this will define
#
#  GDA3_FOUND - system has GDA3
#  GDA3_INCLUDE_DIRS - the GDA3 include directory
#  GDA3_LIBRARIES - Link these to use GDA3
#  GDA3_DEFINITIONS - Compiler switches required for using GDA3
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (GDA3_LIBRARIES AND GDA3_INCLUDE_DIRS)
  # in cache already
  set(GDA3_FOUND TRUE)
else (GDA3_LIBRARIES AND GDA3_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  if (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    include(UsePkgConfig)
    pkgconfig(libgda-3.0 _GDA3_INCLUDEDIR _GDA3_LIBDIR _GDA3_LDFLAGS _GDA3_CFLAGS)
  else (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
    find_package(PkgConfig)
    if (PKG_CONFIG_FOUND)
      pkg_check_modules(_GDA3 libgda-3.0)
    endif (PKG_CONFIG_FOUND)
  endif (${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4)
  find_path(GDA3_INCLUDE_DIR
    NAMES
      libgda/libgda.h
    PATHS
      ${_GDA3_INCLUDEDIR}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libgda-3.0
  )

  find_library(GDA_LIBRARY
    NAMES
      gda
    PATHS
      ${_GDA3_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )
  find_library(GDA-3.0_LIBRARY
    NAMES
      gda-3.0
    PATHS
      ${_GDA3_LIBDIR}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (GDA_LIBRARY)
    set(GDA_FOUND TRUE)
  endif (GDA_LIBRARY)
  if (GDA-3.0_LIBRARY)
    set(GDA-3.0_FOUND TRUE)
  endif (GDA-3.0_LIBRARY)

  set(GDA3_INCLUDE_DIRS
    ${GDA3_INCLUDE_DIR}
  )

  if (GDA_FOUND)
    set(GDA3_LIBRARIES
      ${GDA3_LIBRARIES}
      ${GDA_LIBRARY}
    )
  endif (GDA_FOUND)
  if (GDA-3.0_FOUND)
    set(GDA3_LIBRARIES
      ${GDA3_LIBRARIES}
      ${GDA-3.0_LIBRARY}
    )
  endif (GDA-3.0_FOUND)

  if (GDA3_INCLUDE_DIRS AND GDA3_LIBRARIES)
     set(GDA3_FOUND TRUE)
  endif (GDA3_INCLUDE_DIRS AND GDA3_LIBRARIES)

  if (GDA3_FOUND)
    if (NOT GDA3_FIND_QUIETLY)
      message(STATUS "Found GDA3: ${GDA3_LIBRARIES}")
    endif (NOT GDA3_FIND_QUIETLY)
  else (GDA3_FOUND)
    if (GDA3_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find GDA3")
    endif (GDA3_FIND_REQUIRED)
  endif (GDA3_FOUND)

  # show the GDA3_INCLUDE_DIRS and GDA3_LIBRARIES variables only in the advanced view
  mark_as_advanced(GDA3_INCLUDE_DIRS GDA3_LIBRARIES)

endif (GDA3_LIBRARIES AND GDA3_INCLUDE_DIRS)

