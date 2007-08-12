project(mapnik_symbols)

file(GLOB mapnik_symbols *.png)

install(
  FILES
    ${mapnik_symbols}
  DESTINATION
    ${DATA_INSTALL_DIR}/mapnik/symbols
)

