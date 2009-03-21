project(scripts)

if (NOT MAEMO)
  add_subdirectory(mapnik)
  add_subdirectory(osm/perl_lib)
endif (NOT MAEMO)

if (WITH_SCRIPTS)
  set(bin_SCRIPTS
    gpspoint2gpsdrive.pl
    geocache2way
    gpsreplay
    gpsfetchmap.pl
    gpsdrive-update-mapnik-poitypes.pl
    geoinfo.pl
    gpsd_nmea.sh
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

