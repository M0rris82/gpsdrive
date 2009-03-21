
PROJECT(maps)

INSTALL(FILES map_koord.txt DESTINATION ${DATA_INSTALL_DIR})

FILE(GLOB _pngfiles ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
FILE(GLOB _jpgfiles ${CMAKE_CURRENT_SOURCE_DIR}/*.jpg)

IF (_pngfiles AND _jpgfiles)
	INSTALL(FILES ${_pngfiles} ${_jpgfiles} DESTINATION ${DATA_INSTALL_DIR}/maps)
ENDIF (_pngfiles AND _jpgfiles)

