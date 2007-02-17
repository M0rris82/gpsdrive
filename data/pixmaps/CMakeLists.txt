PROJECT(pixmaps)

FILE(GLOB _pixmaps ${CMAKE_CURRENT_SOURCE_DIR}/*.png  ${CMAKE_CURRENT_SOURCE_DIR}/*.gif)

IF (_pixmaps)
	INSTALL(FILES ${_pixmaps} DESTINATION ${DATA_INSTALL_DIR}/pixmaps)
ENDIF (_pixmaps)

