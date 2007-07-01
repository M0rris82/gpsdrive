# - Try to find Mapnik
# Once done this will define
#
#  MAPNIK_FOUND - system has Mapnik
#  MAPNIK_INCLUDE_DIRS - the Mapnik include directory
#  MAPNIK_LIBRARIES - Link these to use Mapnik
#  MAPNIK_DEFINITIONS - Compiler switches required for using Mapnik
#
#  Copyright (c) 2007 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (MAPNIK_LIBRARIES AND MAPNIK_INCLUDE_DIRS)
  # in cache already
  set(MAPNIK_FOUND TRUE)
else (MAPNIK_LIBRARIES AND MAPNIK_INCLUDE_DIRS)
  find_path(MAPNIK_INCLUDE_DIR
    NAMES
      mapnik/config.hpp
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(MAPNIK_LIBRARY
    NAMES
      mapnik
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (MAPNIK_LIBRARY)
    set(MAPNIK_FOUND TRUE)
  endif (MAPNIK_LIBRARY)

  set(MAPNIK_INCLUDE_DIRS
    ${MAPNIK_INCLUDE_DIR}
  )

  if (MAPNIK_FOUND)
    set(MAPNIK_LIBRARIES
      ${MAPNIK_LIBRARIES}
      ${MAPNIK_LIBRARY}
    )
  endif (MAPNIK_FOUND)

  if (MAPNIK_INCLUDE_DIRS AND MAPNIK_LIBRARIES)
     set(MAPNIK_FOUND TRUE)
  endif (MAPNIK_INCLUDE_DIRS AND MAPNIK_LIBRARIES)

  if (MAPNIK_FOUND)
    if (NOT Mapnik_FIND_QUIETLY)
      message(STATUS "Found Mapnik: ${MAPNIK_LIBRARIES}")
    endif (NOT Mapnik_FIND_QUIETLY)
  else (MAPNIK_FOUND)
    if (Mapnik_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find Mapnik")
    endif (Mapnik_FIND_REQUIRED)
  endif (MAPNIK_FOUND)

  # show the MAPNIK_INCLUDE_DIRS and MAPNIK_LIBRARIES variables only in the advanced view
  mark_as_advanced(MAPNIK_INCLUDE_DIRS MAPNIK_LIBRARIES)

endif (MAPNIK_LIBRARIES AND MAPNIK_INCLUDE_DIRS)

