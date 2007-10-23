project(gpsdrive_src)

add_subdirectory(lib_map)
add_subdirectory(util)

find_package(Gettext REQUIRED)
find_package(X11 REQUIRED)
find_package(GTK2 REQUIRED)
find_package(GDAL REQUIRED)
find_package(PNG REQUIRED)
find_package(Fontconfig REQUIRED)
find_package(MySQL REQUIRED)
find_package(XML2 REQUIRED)
find_package(Boost REQUIRED)
find_package(Freetype2 REQUIRED)

if (WITH_MAPNIK)
  find_package(Mapnik REQUIRED)
endif (WITH_MAPNIK)


set(GPSDRIVE_PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  CACHE INTERNAL "gpsdrive public include directories"
)

set(GPSDRIVE_PRIVATE_INCLUDE_DIRS
  ${LIBMAP_PUBLIC_INCLUDE_DIRS}
  ${GTK2_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
  ${MYSQL_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIRS}
  ${XML2_INCLUDE_DIRS}
  ${MAPNIK_INCLUDE_DIRS}
  ${FREETYPE2_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
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
)

set(gpsdrive_SRCS
  battery.c
  battery.h
  download_map.c
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
  gpssql.c
  gui.c
  gui.h
  gpx.c
  gpx.h
  icons.c
  icons.h
  import_map.c
  LatLong-UTMconversion.c
  main_gui.c
  map_handler.c
  map_projection.c
  navigation.c
  navigation_gui.c
  nmea_handler.c
  poi.c
  poi_gui.c
  poi.h
  routes.c
  screenshot.c
  screenshot.h
  settings.c
  settings_gui.c
  speech_out.c
  speech_out.h
  speech_strings.c
  splash.c
  track.c
  track.h
  unit_test.c
  waypoint.c
  wlan.c
)

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

include_directories(
  ${GPSDRIVE_PUBLIC_INCLUDE_DIRS}
  ${GPSDRIVE_PRIVATE_INCLUDE_DIRS}
)

add_executable(gpsdrive ${gpsdrive_SRCS})
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

