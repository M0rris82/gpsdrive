project(gpsdrive_src)

find_package(Gettext REQUIRED)
find_package(X11 REQUIRED)
find_package(GTK2 REQUIRED)
find_package(XML2 REQUIRED)
find_package(Freetype2 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)


if (WITH_SPEECH)
  find_package(Speechd REQUIRED)
  add_definitions(-DSPEECH)
endif (WITH_SPEECH)

if (WITH_GDA3)
  find_package(GDA3 REQUIRED)
  add_definitions(-DGDA3)
endif (WITH_GDA3)

if (WITH_MAPNIK)
  find_package(Boost COMPONENTS filesystem REQUIRED)
  find_package(Mapnik REQUIRED)
  add_definitions(-DMAPNIK)
endif (WITH_MAPNIK)

if (WITH_GDAL)
  find_package(GDAL REQUIRED)
  add_subdirectory(lib_map)
endif (WITH_GDAL)

if (WITH_DBUS)
    pkg_check_modules(DBUS REQUIRED dbus-1 dbus-glib-1)        
    add_definitions(-DDBUS)
endif (WITH_DBUS)

add_subdirectory(util)

set(GPSDRIVE_PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  CACHE INTERNAL "gpsdrive public include directories"
)

set(GPSDRIVE_PRIVATE_INCLUDE_DIRS
  ${GTK2_INCLUDE_DIRS}
  ${XML2_INCLUDE_DIRS}
  ${MAPNIK_INCLUDE_DIRS}
  ${FREETYPE2_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${GDA3_INCLUDE_DIRS}
  ${SQLITE3_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${SPEECHD_INCLUDE_DIRS}
)

set(GPSDRIVE_LINK_LIBRARIES
  gpsdrive
  crypt
  ${GDAL_LIBRARIES}
  ${GETTEXT_LIBRARIES}
  ${GTK2_LIBRARIES}
  ${XML2_LIBRARIES}
  ${MAPNIK_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${GDA3_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${DBUS_LIBRARIES}
  ${CURL_LIBRARIES}
  ${SPEECHD_LIBRARIES}
)

set(gpsdrive_SRCS
  battery.c
  battery.h
  database.c
  database.h
  database_sqlite.c
  database_sqlite.h
  draw_grid.c
  friends.c
  geometry.c
  gettext.h
  gpsdrive.c
  gpsdrive_config.c
  gps_handler.c
  gps_handler.h
  gpskismet.c
  gpsmisc.c
  gpsnasamap.c
  gui.c
  gui.h
  gpx.c
  gpx.h
  icons.c
  icons.h
  import_map.c
  LatLong-UTMconversion.c
  main_gui.c
  map_download.c
  map_download.h
  map_handler.c
  map_projection.c
  navigation.c
  navigation_gui.c
  nmea_handler.c
  os_specific.c
  poi.c
  poi_gui.c
  poi.h
  routes.c
  screenshot.c
  screenshot.h
  settings.c
  settings_gui.c
  speech.c
  speech.h
  splash.c
  track.c
  track.h
  unit_test.c
  waypoint.c
)

if (WITH_GDA3)
  set(gpsdrive_SRCS
    ${gpsdrive_SRCS}
    database_postgis.c
    database_postgis.h
  )
endif (WITH_GDA3)

if (WITH_MAPNIK)
  set(gpsdrive_SRCS
    ${gpsdrive_SRCS}
    mapnik.cpp
  )
endif (WITH_MAPNIK)

# gpsdrive i18n
MACRO_GENERATE_PO_FILES(${CMAKE_SOURCE_DIR}/po ${APPLICATION_NAME} gpsdrive_SRCS)

SET(friendsd_SRCS
  friendsd.c
  gpsdrive.h
)

SET(poitypes_SRCS
  update-poitypes.c
)

SET(poiosmdb_SRCS
  update-osm-poi-db.c
)

include_directories(
  ${GPSDRIVE_PUBLIC_INCLUDE_DIRS}
  ${GPSDRIVE_PRIVATE_INCLUDE_DIRS}
)

add_executable(gpsdrive ${gpsdrive_SRCS})
add_executable(friendsd ${friendsd_SRCS})
add_executable(gpsdrive-update-osm-poi-db ${poiosmdb_SRCS})

if (WITH_GDA3)
  add_executable(gpsdrive-update-mapnik-db ${poitypes_SRCS})
endif (WITH_GDA3)

target_link_libraries(${GPSDRIVE_LINK_LIBRARIES})

target_link_libraries(friendsd
  crypt
  ${GTK2_LIBRARIES}
)

target_link_libraries(gpsdrive-update-osm-poi-db
  ${GTK2_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${XML2_LIBRARIES}
)

if (WITH_GDA3)
  target_link_libraries(gpsdrive-update-mapnik-db
    ${GTK2_LIBRARIES}
    ${GDA3_LIBRARIES}
    ${SQLITE3_LIBRARIES}
  )
endif (WITH_GDA3)

set(gpsdrive_PGMS
  gpsdrive
  friendsd
  gpsdrive-update-osm-poi-db
)

if (WITH_GDA3)
  set(gpsdrive_PGMS
    ${gpsdrive_PGMS}
    gpsdrive-update-mapnik-db
  )
endif (WITH_GDA3)


install(
  TARGETS
    ${gpsdrive_PGMS}
  DESTINATION
    ${BIN_INSTALL_DIR}
)
