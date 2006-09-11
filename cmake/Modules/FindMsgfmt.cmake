#
# - Try to find the msgfmt executeable
#
# It will set the following variables:
#
#  MSGFMT_FOUND
#  MSGFMT_EXECUTABLE
#
# Copyright (c) 2006 Andreas Schneider
#

IF (MSGFMT_EXECUTABLE)
	# in cache alread?
	SET(MSGFMT_FOUND TRUE)

ELSE (MSGFMT_EXECUTABLE)
	IF (UNIX)
		FIND_PROGRAM(MSGFMT_EXECUTABLE
			NAMES
				msgfmt
			PATHS
				/usr/bin
				/usr/local/bin
		)
		IF(MSGFMT_EXECUTABLE)
			SET(MSGFMT_FOUND TRUE)

		ELSE(MSGFMT_EXECUTABLE)
			MESSAGE(FATAL_ERROR "msgfmt not found - po files can't be processed")

		ENDIF(MSGFMT_EXECUTABLE)

		MARK_AS_ADVANCED(MSGFMT_EXECUTABLE)
	ENDIF(UNIX)
ENDIF (MSGFMT_EXECUTABLE)

