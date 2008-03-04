# - Try to find GDA3
# Once done this will define
#
#  GDA3_FOUND - system has GDA3
#  GDA3_INCLUDE_DIRS - the GDA3 include directory
#  GDA3_LIBRARIES - Link these to use GDA3
#  GDA3_DEFINITIONS - Compiler switches required for using GDA3
#
#  Copyright (c) 2007 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>
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
  include(UsePkgConfig)

  pkgconfig(libgda-3.0 _GDA3IncDir _GDA3LinkDir _GDA3LinkFlags _GDA3Cflags)

  set(GDA3_DEFINITIONS ${_GDA3Cflags})

  find_path(GDA3_INCLUDE_DIR
    NAMES
      libgda/libgda.h
    PATHS
      ${_GDA3IncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
    PATH_SUFFIXES
      libgda-3.0
  )

  find_library(GDA3_LIBRARY
    NAMES
      gda
      gda-3.0
    PATHS
      ${_GDA3LinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(GDA3_INCLUDE_DIRS
    ${GDA3_INCLUDE_DIR}
  )
  set(GDA3_LIBRARIES
    ${GDA3_LIBRARY}
)

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

