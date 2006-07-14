LIB_MAP=../lib_map/map_gpsdrive.c  ../lib_map/map_load.c \
	../lib_map/map_transform.c ../lib_map/map_gpsmisc.c \
	../lib_map/map_render.c \
	../lib_map/map.h  ../lib_map/map_priv.h


LIB_MAP_O=../lib_map/map_gpsdrive.o  ../lib_map/map_load.o \
	../lib_map/map_transform.o ../lib_map/map_gpsmisc.o \
	../lib_map/map_render.o

LIB_MAP_L= map_gpsdrive.c  map_load.c \
	map_transform.c map_gpsmisc.c \
	map_render.c

if HAVE_GDAL
worldgendir=$(bindir)
worldgen_PROGRAMS = worldgen
worldgen_SOURCES= worldgen.c ../lib_map/lib_map.a
worldgen_LDFLAGS = $(GDAL_LIBS)

if HAVE_GTK
#CFLAGS += -DHAVE_CAIRO
#CFLAGS += -DHAVE_GTK

# the $(GDAL_LIBS) part is a quick hack. So please could 
# someone knowing autoconf reconsidder if there is a better solution
lib_map_lflags = \
	$(GDAL_LIBS) \
	`pkg-config cairo --libs` \
	`pkg-config gtk+-2.0 --libs`


gmapviewdir=$(bindir)
gmapview_PROGRAMS = gmapview
gmapview_SOURCES= gmapview.c 
gmapview_LDFLAGS = \
	  ../lib_map/lib_map.a \
	$(lib_map_lflags) \
	 $(LDADD) $(GDAL_LDADD) $(LFLAGS) -lm
gmapview_CXXFLAGS = $(CFLAGS) $(PREFIX) $(PLUGINPATH) \
	$(PKGDATAPATH) $(GDAL_CFLAGS) $(CXXFLAGS) $(EXTRA_CXXFLAGS) \
	-DHAVE_CAIRO -DHAVE_GTK
endif

endif

EXTRA_DIST = $(gmapview_SOURCES) $(worldgen_SOURCES) \
	gps.c mmapview.c qmapview.cpp wmapview.c

