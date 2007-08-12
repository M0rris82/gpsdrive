PROJECT(libmap)

set(LIBMAP_PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "gpsdrive public include directories"
)

include_directories(
  ${LIBMAP_PUBLIC_INCLUDE_DIRS}
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
  map.h
  map_priv.h
)

ADD_LIBRARY(map STATIC ${libmap_SRCS})

