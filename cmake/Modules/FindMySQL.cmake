# - Find MySQL
# Find the MySQL includes and client library
# This module defines
#  MYSQL_INCLUDE_DIR, where to find mysql.h
#  MYSQL_LIBRARIES, the libraries needed to use MySQL.
#  MYSQL_FOUND, If false, do not try to use MySQL.

if (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
	# in cache
	SET(MYSQL_FOUND TRUE)
else (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

	FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
		/usr/include/mysql
		/usr/local/include/mysql
	)

	FIND_LIBRARY(MYSQL_LIBRARIES NAMES mysqlclient
		PATHS
		/usr/lib/mysql
		/usr/local/lib/mysql
	)

	if (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
		SET(MYSQL_FOUND TRUE)
		MESSAGE(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
	else (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
		SET(MYSQL_FOUND FALSE)
		MESSAGE(STATUS "MySQL not found.")
	endif (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)

	MARK_AS_ADVANCED(MYSQL_INCLUDE_DIR MYSQL_LIBRARIES)
endif (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARIES)
