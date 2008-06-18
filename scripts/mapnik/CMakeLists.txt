project(mapnik)

message("mapnik datadir: ${DATA_INSTALL_DIR}/mapnik")

set(USER "@USER@")

configure_file(gpsdrive_mapnik_gentiles-in.py ${CMAKE_CURRENT_BINARY_DIR}/gpsdrive_mapnik_gentiles.py)
configure_file(osm-in.xml ${CMAKE_CURRENT_BINARY_DIR}/osm-template.xml)

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
