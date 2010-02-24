project(gpsdrive_src)

find_package(Gettext REQUIRED)
find_package(X11 REQUIRED)
find_package(GTK2 REQUIRED)
find_package(XML2 REQUIRED)
find_package(Freetype2 REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Gps REQUIRED)

if (MAEMO)
  find_package(Hildon REQUIRED)
  find_package(Libosso REQUIRED)
  find_package(Location REQUIRED)
  add_definitions(-DMAEMO)
  SET (WITH_POSTGIS FALSE)
  SET (WITH_MAPNIK FALSE)
  SET (WITH_SPEECH FALSE)
  SET (WITH_FRIENDSD FALSE)
  SET (WITH_SCRIPTS FALSE)
  SET (WITH_DBUS TRUE)
endif (MAEMO)

if (WITH_POSTGIS)
  find_package(PostgreSQL REQUIRED)
  add_definitions(-DPOSTGIS)
endif (WITH_POSTGIS)

if (WITH_MAPNIK)
  find_package(Boost COMPONENTS filesystem REQUIRED)
  find_package(Mapnik REQUIRED)
  add_definitions(-DMAPNIK)
endif (WITH_MAPNIK)

if (WITH_SPEECH)
  find_package(Speechd REQUIRED)
  add_definitions(-DSPEECH)
endif (WITH_SPEECH)

if (WITH_GDAL)
  find_package(GDAL REQUIRED)
  add_subdirectory(lib_map)
endif (WITH_GDAL)

if (WITH_DBUS)
    INCLUDE(FindPkgConfig)
    pkg_check_modules(DBUS REQUIRED dbus-1 dbus-glib-1)        
    add_definitions(-DDBUS)
endif (WITH_DBUS)

if (WITH_KISMET)
  add_definitions(-DWITH_KISMET)
endif (WITH_KISMET)

if (WITH_NAVIGATION)
  add_definitions(-DNAVIGATION)
endif (WITH_NAVIGATION)

if (LIBGPS_OLD)
  add_definitions(-DLIBGPS_OLD)
endif (LIBGPS_OLD)

if (DEBUG)
  add_definitions(-DDEBUG)
endif (DEBUG)

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
  ${POSTGRESQL_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS}
  ${SPEECHD_INCLUDE_DIRS}
  ${HILDON_INCLUDE_DIR}
  ${LIBOSSO_INCLUDE_DIR}
  ${LOCATION_INCLUDE_DIRS}
  ${GPS_INCLUDE_DIRS}
)

set(GPSDRIVE_LINK_LIBRARIES
  gpsdrive
  crypt
  ${GETTEXT_LIBRARIES}
  ${GTK2_LIBRARIES}
  ${XML2_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${SQLITE3_LIBRARIES}
  ${CURL_LIBRARIES}
  ${GPS_LIBRARIES}
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

if (WITH_GDAL)
  set(GPSDRIVE_LINK_LIBRARIES
    ${GPSDRIVE_LINK_LIBRARIES}
    ${GDAL_LIBRARIES}
  )
endif (WITH_GDAL)

if (WITH_POSTGIS)
  set(gpsdrive_SRCS
    ${gpsdrive_SRCS}
    database_postgis.c
    database_postgis.h
  )
  set(GPSDRIVE_LINK_LIBRARIES
    ${GPSDRIVE_LINK_LIBRARIES}
    ${POSTGRESQL_LIBRARIES}
  )
endif (WITH_POSTGIS)

if (WITH_MAPNIK)
  set(gpsdrive_SRCS
    ${gpsdrive_SRCS}
    mapnik.cpp
  )
  set(GPSDRIVE_LINK_LIBRARIES
    ${GPSDRIVE_LINK_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARY}
    ${MAPNIK_LIBRARIES}
  )
endif (WITH_MAPNIK)

if (MAEMO)
  set(GPSDRIVE_LINK_LIBRARIES
    ${GPSDRIVE_LINK_LIBRARIES}
    ${HILDON_LIBRARIES}
    ${LIBOSSO_LIBRARIES}
    ${LOCATION_LIBRARIES}
  )
endif (MAEMO)

if (WITH_KISMET)
  set(gpsdrive_SRCS
    ${gpsdrive_SRCS}
    gpskismet.c
  )
endif (WITH_KISMET)

if (WITH_SPEECH)
  set(GPSDRIVE_LINK_LIBRARIES
    ${GPSDRIVE_LINK_LIBRARIES}
    ${SPEECHD_LIBRARIES}
  )
endif (WITH_SPEECH)

if (WITH_DBUS)
  set(GPSDRIVE_LINK_LIBRARIES
    ${GPSDRIVE_LINK_LIBRARIES}
    ${DBUS_LIBRARIES}
  )
endif (WITH_DBUS)

if (WITH_NAVIGATION)
  set(gpsdrive_SRCS
    ${gpsdrive_SRCS}
  navigation.c
  navigation.h
  navigation_gui.c
  )
  add_library (nav_dummy MODULE
  mod_nav_dummy.c
  )
endif (WITH_NAVIGATION)


# gpsdrive i18n
MACRO_GENERATE_PO_FILES(${CMAKE_SOURCE_DIR}/po ${APPLICATION_NAME} gpsdrive_SRCS)

if (WITH_FRIENDSD)
  if (NOT MAEMO)
    SET(friendsd_SRCS
      friendsd.c
      gpsdrive.h
      )
  endif (NOT MAEMO)
endif (WITH_FRIENDSD)

include_directories(
  ${GPSDRIVE_PUBLIC_INCLUDE_DIRS}
  ${GPSDRIVE_PRIVATE_INCLUDE_DIRS}
)

add_executable(gpsdrive ${gpsdrive_SRCS})
if (WITH_FRIENDSD)
  if (NOT MAEMO)
    add_executable(friendsd ${friendsd_SRCS})
  endif (NOT MAEMO)
endif (WITH_FRIENDSD)

target_link_libraries(${GPSDRIVE_LINK_LIBRARIES})

if (WITH_FRIENDSD AND NOT MAEMO)
 target_link_libraries(friendsd
   crypt
   ${GTK2_LIBRARIES}
 )
endif (WITH_FRIENDSD AND NOT MAEMO)

set(gpsdrive_PGMS
  gpsdrive
)
if (WITH_FRIENDSD AND NOT MAEMO)
  set(gpsdrive_PGMS
    ${gpsdrive_PGMS}
    friendsd
  )
endif (WITH_FRIENDSD AND NOT MAEMO)


install(
  TARGETS
    ${gpsdrive_PGMS}
  DESTINATION
    ${BIN_INSTALL_DIR}
)

if (WITH_NAVIGATION)
  install(
    TARGETS
      nav_dummy
    LIBRARY DESTINATION
      ${LIB_INSTALL_DIR}/gpsdrive/
)
endif (WITH_NAVIGATION)
