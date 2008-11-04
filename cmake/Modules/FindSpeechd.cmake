# - Try to find Speechd
# Once done this will define
#
#  SPEECHD_FOUND - system has SPEECHD
#  SPEECHD_INCLUDE_DIRS - the SPEECHD include directory
#  SPEECHD_LIBRARIES - Link these to use SPEECHD
#  SPEECHD_DEFINITIONS - Compiler switches required for using SPEECHD
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if (SPEECHD_LIBRARIES AND SPEECHD_INCLUDE_DIRS)
  # in cache already
  set(SPEECHD_FOUND TRUE)
else (SPEECHD_LIBRARIES AND SPEECHD_INCLUDE_DIRS)
  find_path(SPEECHD_INCLUDE_DIR
    NAMES
      libspeechd.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(SPEECHD_LIBRARY
    NAMES
      speechd
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (SPEECHD_LIBRARY)
    set(SPEECHD_FOUND TRUE)
  endif (SPEECHD_LIBRARY)

  set(SPEECHD_INCLUDE_DIRS
    ${SPEECHD_INCLUDE_DIR}
  )

  if (SPEECHD_FOUND)
    set(SPEECHD_LIBRARIES
      ${SPEECHD_LIBRARY}
    )
  endif (SPEECHD_FOUND)

  if (SPEECHD_INCLUDE_DIRS AND SPEECHD_LIBRARIES)
     set(SPEECHD_FOUND TRUE)
  endif (SPEECHD_INCLUDE_DIRS AND SPEECHD_LIBRARIES)

  if (SPEECHD_FOUND)
    if (NOT SPEECHD_FIND_QUIETLY)
      message(STATUS "Found Speechd: ${SPEECHD_LIBRARIES}")
    endif (NOT SPEECHD_FIND_QUIETLY)
  else (SPEECHD_FOUND)
    if (SPEECHD_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find speechd")
    endif (SPEECHD_FIND_REQUIRED)
  endif (SPEECHD_FOUND)

  # show the SPEECHD_INCLUDE_DIRS and SPEECHD_LIBRARIES variables only in the advanced view
  mark_as_advanced(SPEECHD_INCLUDE_DIRS SPEECHD_LIBRARIES)

endif (SPEECHD_LIBRARIES AND SPEECHD_INCLUDE_DIRS)

