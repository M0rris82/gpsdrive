PROJECT(data)

if (WITH_BASEMAPS)
  SUBDIRS(
    maps
    )
endif (WITH_BASEMAPS)

SUBDIRS(
  pixmaps
)

if (NOT MAEMO)
  INSTALL(FILES gpsdrive.desktop DESTINATION ${XDG_APPS_DIR})
endif (NOT MAEMO)

# This is now in the map-icons Package
# INSTALL(FILES geoinfo.db DESTINATION ${DATA_INSTALL_DIR})

