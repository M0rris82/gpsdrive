project(scripts)

set(bin_SCRIPTS
  wpcvt
  wpget
  gpspoint2gpsdrive.pl
  geo-nearest
  geocache2way
  geo-code
  gpssql_backup.sh
  gpssql_restore.sh
  gpsreplay
  gpssmswatch
  gpsfetchmap.pl
  geoinfo.pl
  wp2sql
  gpsd_nmea.sh
  nasaconv.sh
)

install(FILES
    ${bin_SCRIPTS}
  PERMISSIONS
    OWNER_EXECUTE
    GROUP_EXECUTE
    WORLD_EXECUTE
  DESTINATION
    ${BIN_INSTALL_DIR})

MACRO_OPTIONAL_FIND_PACKAGE(Perl)

if (PERL_FOUND)
  MACRO_OPTIONAL_FIND_PACKAGE(PerlLibs)
  #MESSAGE("DEBUG: > PERL_SITELIB: ${PERL_SITELIB}")

  file(GLOB_RECURSE perl_modules ${CMAKE_CURRENT_SOURCE_DIR} *.pm)

    if (perl_modules)
      foreach(perl_module ${perl_modules})
        file(RELATIVE_PATH relative_module_path ${CMAKE_CURRENT_SOURCE_DIR} ${perl_module})

        install(FILES ${perl_module} DESTINATION ${PERL_SITELIB}/${relative_module_path})
        #MESSAGE("DEBUG: install ${perl_module} to ${PERL_SITELIB}/${relative_module_path}")
      endforeach(perl_module ${perl_modules})
    endif (perl_modules)
endif (PERL_FOUND)
