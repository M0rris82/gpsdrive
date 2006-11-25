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

set(GPSDRIVE_SRC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${GTK2_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIR}
  ${MYSQL_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

include_directories(${GPSDRIVE_SRC_INCLUDE_DIRS})

STRING(REGEX REPLACE "\n" "" FONTCONFIG_DEFINITIONS "${FONTCONFIG_DEFINITIONS}")

add_definitions(
  -DHAVE_CONFIG_H
  ${PNG_DEFINITIONS}
  ${FONTCONFIG_DEFINITIONS}
)


SET(gpsdrive_SRCS
  gpsdrive.c
  splash.c
  splash.h
  gpsdrive_config.c
  gpsdrive_config.h
  navigation.c
  speech_out.c
  speech_out.h
  friends.c
  battery.c
  track.c
  poi.c
  streets.c
  waypoint.c
  draw_tracks.c
  draw_grid.c
  settings.c
  battery.h
  track.h
  poi.h
  streets.h
  waypoint.h
  draw_tracks.h
  gpsdrive.h
  gpssql.c
  gpskismet.c
  icons.c
  icons.h
  gettext.h
  LatLong-UTMconversion.c
  LatLong-UTMconversion.h
  gpsserial.c
  gpsnasamap.c
  gpsmisc.c
  geometry.c
  map_handler.c
  gpsproto.h
  talogo.h
  import_map.c
  import_map.h
  routes.c
  routes.h
  download_map.c
  download_map.h
  map_projection.c
  speech_strings.c
  speech_strings.h
  gps_handler.c
  gps_handler.h
  nmea_handler.c
  nmea_handler.h
  unit_test.c
  wlan.c
  wlan.h
)

SET(garmin_SRCS
  garmin_data.cpp
  garmin_serial_unix.cpp
  garmin_application.cpp
  garmin_link.cpp
  garmin_util.cpp
  gpsdrivegarble.cpp
  garmin_legacy.cpp
  garmin_link.h
  garmin_serial_unix.h
  garmin_application.h
  garmin_packet.h
  garmin_types.h
  garmin_command.h
  garmin_phys.h
  garmin_util.h
  garmin_error.h
  garmin_serial.h
  garmin_legacy.h
  garmin_data.h
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

add_executable(gpsdrive ${gpsdrive_SRCS} ${garmin_SRCS})
add_executable(friendsd ${friendsd_SRCS})

target_link_libraries(
  gpsdrive
  map
  crypt
  ${GDAL_LIBRARIES}
  ${FONTCONFIG_LIBRARIES}
  ${GETTEXT_LIBRARIES}
  ${GTK2_LIBRARIES}
  ${GDAL_LIBRARIES}
  ${PNG_LIBRARIES}
)

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

