#
# - MACRO_INSTALL_MANPAGES
#        Find manpages in the given directory and install them
#

MACRO(MACRO_INSTALL_MANPAGES _mandir)
	FILE(GLOB_RECURSE _manpages ${_mandir} *.[1-9])

	IF (_manpages)
		FOREACH(_man ${_manpages})
			FILE(RELATIVE_PATH _lang ${_mandir} ${_man})
			GET_FILENAME_COMPONENT(_lang ${_lang} PATH)
			GET_FILENAME_COMPONENT(_manext ${_man} EXT)
			STRING(REGEX MATCH "[1-9]" _manext ${_manext})
			INSTALL(FILES ${_man} DESTINATION ${MAN_INSTALL_DIR}/${_lang}/man${_manext})
			#MESSAGE("DEBUG: install ${_man} to ${MAN_INSTALL_DIR}/${_lang}/man${_manext}")
		ENDFOREACH(_man ${_manpages})
	ENDIF (_manpages)
ENDMACRO(MACRO_INSTALL_MANPAGES _mandir)

