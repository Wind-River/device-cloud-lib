#
# json-c support
#
# The following variables can be set to add additional find support:
# - JSONC_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - JSONC_FOUND - System has json-c
# - JSONC_INCLUDE_DIRS/JSONC_INCLUDES - Include directories for json-c
# - JSONC_LIBRARIES/JSONC_LIBS - Libraries needed for json-c
# - JSONC_DEFINITIONS - Compiler switches required for json-c
# - JSONC_VERSION - Library version
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#

include( FindPackageHandleStandardArgs )

# Try and find paths
set( LIB_SUFFIX "" )
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
endif()

# Allow the ability to specify a global dependency root directory
if ( NOT JSONC_ROOT_DIR )
	set( JSONC_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( JSONC_INCLUDE_DIR
	NAMES json-c/json.h
	DOC "json-c include directory"
	PATHS "${JSONC_ROOT_DIR}/include"
)

find_library( JSONC_LIBRARY
	NAMES json-c
	DOC "Required json-c libraries"
	PATHS "${JSONC_ROOT_DIR}/lib${LIB_SUFFIX}"
)

# determine version
if ( JSONC_INCLUDE_DIR AND EXISTS "${JSONC_INCLUDE_DIR}/json-c/json_c_version.h" )
	set( _JSONC_VERSION_REGEX
		"^[ \t]*#define[ \t]+JSON_C_VERSION[ \t]+\"([0-9]+)\\.([0-9]+)\\.?([0-9]*)\".*$")
	file( STRINGS "${JSONC_INCLUDE_DIR}/json-c/json_c_version.h"
		_JSONC_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_JSONC_VERSION_REGEX}"
	)
	if ( _JSONC_VERSION_STRING )
		string( REGEX REPLACE "${_JSONC_VERSION_REGEX}"
			"\\1.\\2.\\3" JSONC_VERSION
			"${_JSONC_VERSION_STRING}")
		string( REGEX REPLACE "\\.$" ".0" JSONC_VERSION
			"${JSONC_VERSION}" )
	endif ( _JSONC_VERSION_STRING )
	unset( _JSONC_VERSION_REGEX )
	unset( _JSONC_VERSION_STRING )
endif ( JSONC_INCLUDE_DIR AND EXISTS "${JSONC_INCLUDE_DIR}/json-c/json_c_version.h" )

find_package_handle_standard_args( JsonC
	FOUND_VAR JSONC_FOUND
	REQUIRED_VARS JSONC_INCLUDE_DIR JSONC_LIBRARY
	VERSION_VAR JSONC_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

set( JSONC_LIBRARIES ${JSONC_LIBRARY} )
set( JSONC_INCLUDE_DIRS ${JSONC_INCLUDE_DIR} )
set( JSONC_DEFINITIONS "" )

set( JSONC_LIBS ${JSONC_LIBRARIES} )
set( JSONC_INCLUDES ${JSONC_INCLUDE_DIRS} )

mark_as_advanced( JSONC_INCLUDE_DIR JSONC_LIBRARY )
