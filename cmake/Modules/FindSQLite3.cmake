# - Try to find SQLite3
# Once done this will define
#
#  SQLITE3_FOUND - system has SQLite3
#  SQLITE3_INCLUDE_DIRS - the SQLite3 include directory
#  SQLITE3_LIBRARIES - Link these to use SQLite3
#  SQLITE3_DEFINITIONS - Compiler switches required for using SQLite3
#
#  Copyright (c) 2007 Andreas Schneider <mail@cynapses.org>
#  Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIRS)
  # in cache already
  set(SQLITE3_FOUND TRUE)
else (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  include(UsePkgConfig)

  pkgconfig(libsqlite3 _SQLITE3IncDir _SQLITE3LinkDir _SQLITE3LinkFlags _SQLITE3Cflags)

  set(SQLITE3_DEFINITIONS ${_SQLITE3Cflags})

  find_path(SQLITE3_INCLUDE_DIR
    NAMES
      sqlite3.h
    PATHS
      ${_SQLITE3IncDir}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(SQLITE3_LIBRARY
    NAMES
      sqlite3
    PATHS
      ${_SQLITE3LinkDir}
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(SQLITE3_INCLUDE_DIRS
    ${SQLITE3_INCLUDE_DIR}
  )
  set(SQLITE3_LIBRARIES
    ${SQLITE3_LIBRARY}
)

  if (SQLITE3_INCLUDE_DIRS AND SQLITE3_LIBRARIES)
     set(SQLITE3_FOUND TRUE)
  endif (SQLITE3_INCLUDE_DIRS AND SQLITE3_LIBRARIES)

  if (SQLITE3_FOUND)
    if (NOT SQLITE3_FIND_QUIETLY)
      message(STATUS "Found SQLite3: ${SQLITE3_LIBRARIES}")
    endif (NOT SQLITE3_FIND_QUIETLY)
  else (SQLITE3_FOUND)
    if (SQLITE3_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find SQLite3")
    endif (SQLITE3_FIND_REQUIRED)
  endif (SQLITE3_FOUND)

  # show the SQLite3_INCLUDE_DIRS and SQLite3_LIBRARIES variables only in the advanced view
  mark_as_advanced(SQLITE3_INCLUDE_DIRS SQLITE3_LIBRARIES)

endif (SQLITE3_LIBRARIES AND SQLITE3_INCLUDE_DIRS)

