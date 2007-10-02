PROJECT(data)

SUBDIRS(
  map-icons
  maps
  pixmaps
)

INSTALL(FILES gpsdrive.desktop DESTINATION ${XDG_APPS_DIR})
INSTALL(FILES map_koord.txt DESTINATION ${DATA_INSTALL_DIR})

