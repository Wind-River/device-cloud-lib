#
# SQLite3 support
#
# The following variables can be set to add additional find support:
# - SQLITE3_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - SQLITE3_FOUND - System has SQLite3
# - SQLITE3_INCLUDE_DIRS/SQLITE3_INCLUDES - Include directories for SQLite3
# - SQLITE3_LIBRARIES/SQLITE3_LIBS - Libraries needed for SQLite3
# - SQLITE3_DEFINITIONS - Compiler switches required for SQLite3
#
# Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

find_package( PkgConfig )
include( FindPackageHandleStandardArgs )

# Try and find paths
set( LIB_SUFFIX "" )
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
endif()

# Allow the ability to specify a global dependency root directory
if ( NOT SQLITE3_ROOT_DIR )
	set( SQLITE3_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( SQLITE3_INCLUDE_DIR
	NAMES sqlite3.h
	DOC "SQLite3 include directory"
	PATHS "${SQLITE3_ROOT_DIR}/include"
)

find_library( SQLITE3_LIBRARY
	NAMES sqlite3
	DOC "Required SQLite3 libraries"
	PATHS "${SQLITE3_ROOT_DIR}/lib${LIB_SUFFIX}"
)

find_package_handle_standard_args( SQLite3
	FOUND_VAR SQLITE3_FOUND
	REQUIRED_VARS SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY
	FAIL_MESSAGE DEFAULT_MSG
)

set( SQLITE3_LIBRARIES ${SQLITE3_LIBRARY} )
set( SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR} )
set( SQLITE3_DEFINITIONS "" )

set( SQLITE3_LIBS ${SQLITE3_LIBRARIES} )
set( SQLITE3_INCLUDES ${SQLITE3_INCLUDE_DIRS} )

mark_as_advanced( SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY )

