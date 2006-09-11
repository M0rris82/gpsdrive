#
# This macro compiles the po files and sets the install dirs
#
# MACRO_GENERATE_PO_FILES($(PO_DIRECOTRY) $(APPLICATION_NAME) project_SRCS)
#
# - PO_DIRECTORY:     The path where cmake can find the PO files
# - APPLICATION_NAME: The name of our application. We need it to rename
#                     the [LANG].mo file to [APPLICATION_NAME].mo
#                     during the installation.
# - project_SRCS:     The name of the variable to add the .mo files,
#                     to get them generated
#

MACRO(MACRO_GENERATE_PO_FILES _podir _applicationname _sources)
	FIND_PACKAGE(Msgfmt REQUIRED)

	IF(MSGFMT_FOUND)
		FILE(GLOB _pofiles ${_podir}/*.po)

		FOREACH(_file ${_pofiles})

			GET_FILENAME_COMPONENT(_infile   ${_file} ABSOLUTE)
			GET_FILENAME_COMPONENT(_basename ${_file} NAME_WE)

			IF(UNIX)
				FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/po)
				GET_FILENAME_COMPONENT(_outfile
					${CMAKE_CURRENT_BINARY_DIR}/po/${_basename}.mo
					ABSOLUTE
				)
			#MESSAGE("DEBUG: ${MSGFMT_EXECUTABLE} -o ${_outfile} ${_infile}")

				ADD_CUSTOM_COMMAND(
					OUTPUT ${_outfile}
					COMMAND ${MSGFMT_EXECUTABLE}
						-o ${_outfile}
						${_infile}
					DEPENDS ${_infile}
				)
			ENDIF(UNIX)

			SET(_mofiles ${_mofiles} ${_outfile})

			INSTALL(FILES
					${_outfile}
				DESTINATION
					${LOCALE_INSTALL_DIR}/${_basename}/LC_MESSAGES/
				RENAME
					${_applicationname}.mo
			)
			#MESSAGE("DEBUG: install ${_outfile} to ${LOCALE_INSTALL_DIR}/${_basename}/LC_MESSAGES/")
		ENDFOREACH(_file ${_pofiles})

		SET(${_sources} ${${_sources}} ${_mofiles})

	ENDIF(MSGFMT_FOUND)
ENDMACRO(MACRO_GENERATE_PO_FILES _podir _applicationname)

