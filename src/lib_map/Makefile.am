lib_map_dir = $(datadir)/gpsdrive
lib_mapdir = $(datadir)/gpsdrive
lib_map_a_SOURCES= \
	map_draw.c  map_gpsmisc.c  map_load.c  map_port.c  map_render.c  \
	map_transform.c \
	map_port_cpp.cpp  map_render_cpp.cpp \
	map.h  map_priv.h


if HAVE_GDAL
MAPDIR = ../../data/maps

lib_map_a_LIBRARIES=lib_map.a
lib_map_adir=$(datadir)/gpsdrive
lib_map_a_CXXFLAGS = \
	$(lib_map_cflags)\
	$(PREFIX) $(PLUGINPATH) \
	$(PKGDATAPATH) $(GDAL_CFLAGS) \
	$(CXXFLAGS) $(EXTRA_CXXFLAGS)
endif


EXTRA_DIST = $(lib_map_la_SOURCES) map.h CMakeLists.txt
