project(util)

if (WITH_GDAL)
  find_package(GDAL REQUIRED)
endif (WITH_GDAL)

set(UTIL_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GTK2_INCLUDE_DIRS}
  ${GDAL_INCLUDE_DIRS}
)

#includes
set(util_INCLUDE_DIR
  ${util_SOURCE_DIR}/
  CACHE INTERNAL ""
)

add_definitions(-DHAVE_GTK -DHAVE_CAIRO)

include_directories(${UTIL_INCLUDE_DIRS})

set(worldgen_SRCS
  worldgen.c
)

set(gmapview_SRCS
  gmapview.c
)

if (GDAL_FOUND)
  add_executable(worldgen ${worldgen_SRCS})
  target_link_libraries(worldgen ${GDAL_LIBRARIES})
  install(TARGETS worldgen DESTINATION ${BIN_INSTALL_DIR})
endif (GDAL_FOUND)

if (GDAL_FOUND AND GTK2_FOUND)
  add_executable(gmapview ${gmapview_SRCS})
  target_link_libraries(gmapview
    map
    ${GDAL_LIBRARIES}
    ${GTK2_LIBRARIES}
  )
  install(TARGETS gmapview DESTINATION ${BIN_INSTALL_DIR})
endif (GDAL_FOUND AND GTK2_FOUND)

