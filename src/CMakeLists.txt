project(gpsdrive_src)

subdirs(
  lib_map
  util
)

find_package(Gettext REQUIRED)
find_package(X11 REQUIRED)
find_package(GTK2 REQUIRED)
find_package(GDAL REQUIRED)
find_package(PNG REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(MySQL REQUIRED)
find_package(XML2 REQUIRED)
find_package(Boost REQUIRED)
find_package(Qt4 REQUIRED)
find_package(Mapnik REQUIRED)
find_package(Freetype2 REQUIRED)

set(GPSDRIVE_PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  CACHE INTERNAL "gpsdrive public include directories"
)

set(GPSDRIVE_PRIVATE_INCLUDE_DIRS
  ${GTK2_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
  ${MYSQL_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIRS}
  ${XML2_INCLUDE_DIRS}
  ${MAPNIK_INCLUDE_DIRS}
  ${FREETYPE2_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
  ${QT_QTGUI_INCLUDE_DIR}
)

set(GPSDRIVE_LINK_LIBRARIES
  gpsdrive
  map
  crypt
  ${GDAL_LIBRARIES}
  ${FONTCONFIG_LIBRARIES}
  ${GETTEXT_LIBRARIES}
  ${GTK2_LIBRARIES}
  ${GDAL_LIBRARIES}
  ${PNG_LIBRARIES}
  ${XML2_LIBRARIES}
  ${MAPNIK_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${BOOST_FILESYSTEM_LIBRARY}
  ${QT_QTGUI_LIBRARY}
)

SET(gpsdrive_SRCS
  battery.c
  battery.h
  download_map.c
  download_map.h
  draw_grid.c
  friends.c
  geometry.c
  gettext.h
  gpsdrive.c
  gpsdrive_config.c
  gpsdrive_config.h
  gpsdrive.h
  gps_handler.c
  gps_handler.h
  gpskismet.c
  gpsmisc.c
  gpsnasamap.c
  gpsproto.h
  gpsserial.c
  gpssql.c
  gui.c
  gui.h
  icons.c
  icons.h
  import_map.c
  import_map.h
  LatLong-UTMconversion.c
  LatLong-UTMconversion.h
  map_handler.c
  map_projection.c
  mapnik.cpp
  navigation.c
  navigation_gui.c
  nmea_handler.c
  nmea_handler.h
  poi.c
  poi_gui.c
  poi.h
  routes.c
  routes.h
  settings.c
  settings_gui.c
  speech_out.c
  speech_out.h
  speech_strings.c
  speech_strings.h
  splash.c
  splash.h
  streets.c
  streets.h
  track.c
  track.h
  unit_test.c
  waypoint.c
  waypoint.h
  wlan.c
  wlan.h
)

SET(garmin_SRCS
  garmin_application.cpp
  garmin_application.h
  garmin_command.h
  garmin_data.cpp
  garmin_data.h
  garmin_error.h
  garmin_legacy.cpp
  garmin_legacy.h
  garmin_link.cpp
  garmin_link.h
  garmin_packet.h
  garmin_phys.h
  garmin_serial.h
  garmin_serial_unix.cpp
  garmin_serial_unix.h
  garmin_types.h
  garmin_util.cpp
  garmin_util.h
  gpsdrivegarble.cpp
)

# gpsdrive i18n
MACRO_GENERATE_PO_FILES(${CMAKE_SOURCE_DIR}/po ${APPLICATION_NAME} gpsdrive_SRCS)

SET(friendsd_SRCS
  friendsd.c
  gpsdrive.h
)

SET(nautic_SRCS
  nautic.c
)

SET(fly_SRCS
  fly.c
)

include_directories(
  ${GPSDRIVE_PUBLIC_INCLUDE_DIRS}
  ${GPSDRIVE_PRIVATE_INCLUDE_DIRS}
)

add_executable(gpsdrive ${gpsdrive_SRCS} ${garmin_SRCS})
add_executable(friendsd ${friendsd_SRCS})

target_link_libraries(${GPSDRIVE_LINK_LIBRARIES})

if (DBUS_FOUND)
  target_link_libraries(gpsdrive ${DBUS_LIBRARIES})
endif (DBUS_FOUND)

target_link_libraries(friendsd
  crypt
  ${GTK2_LIBRARIES}
)

install(
  TARGETS
    gpsdrive
    friendsd
  DESTINATION
    ${BIN_INSTALL_DIR}
)

if(WITH_PLUGINS)
  add_library(nautic SHARED ${nautic_SRCS})
  add_library(fly SHARED ${fly_SRCS})

  set_target_properties(
    nautic
    PROPERTIES
      VERSION
        2.0.0
      SOVERSION
        2
  )

  set_target_properties(
    fly
    PROPERTIES
      VERSION
        2.0.0
      SOVERSION
        2
  )

  install(
    TARGETS
      nautic
      fly
    DESTINATION
      ${LIB_INSTALL_DIR}
  )
endif(WITH_PLUGINS)

