PROJECT(libmap)

set(MAP_PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "gpsdrive public include directories"
)

set(MAP_LIBRARY
  map
  CACHE INTERNAL "map library"
)

set(MAP_LINK_LIBRARIES
  ${MAP_LIBRARY}
  ${GDAL_LIBRARIES}
)

include_directories(
  ${MAP_PUBLIC_INCLUDE_DIRS}
  ${GDAL_INCLUDE_DIRS}
)

# GTK
if (GTK_FOUND)
  include_directories(${GTK_INCLUDE_DIR})
endif (GTK_FOUND)

# GDAL
if (GDAL_FOUND)
  include_directories(${GDAL_INCLUDE_DIR})
endif (GDAL_FOUND)

set(libmap_SRCS
  map_draw.c
  map_gpsmisc.c
  map_load.c
  map_port.c
  map_render.c
  map_transform.c
  map_port_cpp.cpp
  map_render_cpp.cpp
)

add_library(${MAP_LIBRARY} STATIC ${libmap_SRCS})
target_link_libraries(${MAP_LINK_LIBRARIES})

