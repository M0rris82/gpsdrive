# - Try to find liblocation 
#
#  LOCATION_FOUND              True if liblocation got found
#  LOCATION_INCLUDE_DIRS       Location of liblocation headers 
#  LOCATION_LIBRARIES          List of libaries to use liblocation
#
#  Copyright (c) 2009 Guenther Meyer <d.s.e (at) sordidmusic.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

INCLUDE( FindPkgConfig )

pkg_search_module( LOCATION REQUIRED liblocation )

# Hide advanced variables from CMake GUIs
MARK_AS_ADVANCED( LOCATION_LIBRARIES LOCATION_INCLUDE_DIRS )

