# - Try to find Hildon libraries
#
#  HILDON_FOUND              True if Hildon got found
#  HILDON_INCLUDE_DIR         Location of Hildon headers 
#  HILDON_LIBRARIES          List of libaries to use Hildon
#  HILDON_DEFINITIONS        Definitions to compile Hildon 
#
#  Copyright (c) 2007 Daniel Gollub <dgollub@suse.de>
#  Copyright (c) 2008 Daniel Friedrich <daniel.friedrich@opensync.org>
#  Copyright (c) 2008 Florian Boor <florian@linuxtogo.org>
#  Copyright (c) 2008 Guenther Meyer <d.s.e (at) sordidmusic (dot) com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

IF ( NOT WIN32 )
	INCLUDE( UsePkgConfig )
	pkgconfig( hildon-1 _hildon_include_DIR _hildon_link_DIR _hildon_link_FLAGS _hildon_cflags )
ENDIF ( NOT WIN32 )


# Look for Hildon include dir and libraries, and take care about pkg-config first...
find_path(HILDON_INCLUDE_DIR
  NAMES
    hildon/hildon.h
  PATHS
    ${_hildon_include_DIR}
  PATH_SUFFIXES
    hildon-1
)

find_library(HILDON_LIBRARIES
  NAMES
    hildon-1
  PATHS
    ${_hildon_link_DIR}
)


# Report results
if (HILDON_LIBRARIES AND HILDON_INCLUDE_DIR )
  set (HILDON_FOUND TRUE)
  if (NOT HILDON_FIND_QUIETLY)
    message(STATUS "Found HILDON: ${HILDON_LIBRARIES}")
  endif ( NOT HILDON_FIND_QUIETLY )
else (HILDON_LIBRARIES AND HILDON_INCLUDE_DIR)	
  if (HILDON_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Hildon")
  else (HILDON_FIND_REQUIRED)
    if (NOT HILDON_FIND_QUIETLY)
      message(STATUS "Could NOT find Hildon")
    endif (NOT HILDON_FIND_QUIETLY)
  endif (HILDON_FIND_REQUIRED)
endif (HILDON_LIBRARIES AND HILDON_INCLUDE_DIR)

# Hide advanced variables from CMake GUIs
mark_as_advanced(HILDON_LIBRARIES HILDON_INCLUDE_DIR)

