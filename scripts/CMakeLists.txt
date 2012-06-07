project(scripts)

if (NOT MAEMO)
  add_subdirectory(mapnik)
  add_subdirectory(osm/perl_lib)
endif (NOT MAEMO)

if (WITH_SCRIPTS)
  set(bin_SCRIPTS
    gdal_slice.sh
    geocache2way.pl
    geoinfo.pl
    gpsd-connect-bluetooth.sh
    gpsd_nmea.sh
    gpsdrive_tile_crush.sh
    gpsfetchmap.pl
    gpspoint2gpsdrive.pl
    gpsreplay.pl
    gpstile_fetch_and_assemble.sh
    gpstiled_wms_fetch_and_assemble.sh
    nasaconv.sh
  )

  install(FILES
      ${bin_SCRIPTS}
    PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      OWNER_EXECUTE
      GROUP_READ
      GROUP_EXECUTE
      WORLD_READ
      WORLD_EXECUTE
    DESTINATION
      ${BIN_INSTALL_DIR})

  MACRO_OPTIONAL_FIND_PACKAGE(Perl)

endif (WITH_SCRIPTS)

