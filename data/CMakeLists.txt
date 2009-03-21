PROJECT(data)

SUBDIRS(
  maps
  pixmaps
)

if (MAEMO)
  INSTALL(FILES gpsdrive.desktop.maemo RENAME gpsdrive.dektop DESTINATION "/usr/share/applications/hildon")
else (MAEMO)
  INSTALL(FILES gpsdrive.desktop DESTINATION ${XDG_APPS_DIR})
endif (MAEMO)

INSTALL(FILES map_koord.txt DESTINATION ${DATA_INSTALL_DIR})
INSTALL(FILES geoinfo.db DESTINATION ${DATA_INSTALL_DIR})

