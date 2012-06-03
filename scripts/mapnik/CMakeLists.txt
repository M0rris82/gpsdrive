project(mapnik)

message("mapnik datadir: ${DATA_INSTALL_DIR}/mapnik")

set(USER "@USER@")

configure_file(gpsdrive_mapnik_gentiles-in.py ${CMAKE_CURRENT_BINARY_DIR}/gpsdrive_mapnik_gentiles.py)

if(IS_DIRECTORY /usr/lib/mapnik/2.0)
  configure_file(osm_mapnik2.0-in.xml ${CMAKE_CURRENT_BINARY_DIR}/osm-template.xml)
else()
  configure_file(osm-in.xml ${CMAKE_CURRENT_BINARY_DIR}/osm-template.xml)
endif (IS_DIRECTORY)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/osm-template.xml
  DESTINATION
    ${DATA_INSTALL_DIR}
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/gpsdrive_mapnik_gentiles.py
  DESTINATION
    ${BIN_INSTALL_DIR}
)
