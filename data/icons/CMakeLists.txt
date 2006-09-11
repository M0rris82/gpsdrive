PROJECT(icons)

FILE(GLOB _classic ${CMAKE_CURRENT_SOURCE_DIR}/classic/*.png)
FILE(GLOB _squarebig ${CMAKE_CURRENT_SOURCE_DIR}/square.big/*.png)
FILE(GLOB _suqaresmall ${CMAKE_CURRENT_SOURCE_DIR}/square.small/*.png)

IF (_classic)
	INSTALL(FILES ${_classic} DESTINATION ${ICON_INSTALL_DIR}/classic)
ENDIF (_classic)

IF (_squarebig)
	INSTALL(FILES ${_squarebig} DESTINATION ${ICON_INSTALL_DIR}/square.big)
ENDIF (_squarebig)

IF (_squaresmall)
	INSTALL(FILES ${_squaresmall} DESTINATION ${ICON_INSTALL_DIR}/square.small)
ENDIF (_squaresmall)
