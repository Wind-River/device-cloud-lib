#
# Jansson support
#
# The following variables can be set to add additional find support:
# - JANSSON_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - JANSSON_FOUND - System has jansson
# - JANSSON_INCLUDE_DIRS/JANSSON_INCLUDES - Include directories for jansson
# - JANSSON_LIBRARIES/JANSSON_LIBS - Libraries needed for jansson
# - JANSSON_DEFINITIONS - Compiler switches required for jansson
# - JANSSON_VERSION - Library version
#
# Copyright (C) 2014-2017 Wind River Systems, Inc. All Rights Reserved.
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
if ( NOT JANSSON_ROOT_DIR )
	set( JANSSON_ROOT_DIR ${JANSSON_ROOT_DIR} )
endif()


find_path( JANSSON_INCLUDE_DIR
	NAMES jansson.h
	DOC "Jansson include directory"
	PATHS "${JANSSON_ROOT_DIR}/include"
)

find_library( JANSSON_LIBRARY
	NAMES jansson
	DOC "Required jansson libraries"
	PATHS "${JANSSON_ROOT_DIR}/lib${LIB_SUFFIX}"
)

# determine version
if ( JANSSON_INCLUDE_DIR AND EXISTS "${JANSSON_INCLUDE_DIR}/jansson.h" )
	set( _JANSSON_VERSION_REGEX
		"^#define[ \t]+JANSSON_VERSION[ \t]+\"([0-9]+)\\.([0-9]+)\\.?([0-9]*)\".*$")
	file( STRINGS "${JANSSON_INCLUDE_DIR}/jansson.h"
		_JANSSON_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_JANSSON_VERSION_REGEX}"
	)
	if ( _JANSSON_VERSION_STRING )
		string( REGEX REPLACE "${_JANSSON_VERSION_REGEX}"
			"\\1.\\2.\\3" JANSSON_VERSION
			"${_JANSSON_VERSION_STRING}")
		string( REGEX REPLACE "\\.$" ".0" JANSSON_VERSION
			"${JANSSON_VERSION}" )
	endif ( _JANSSON_VERSION_STRING )
	unset( _JANSSON_VERSION_REGEX )
	unset( _JANSSON_VERSION_STRING )
endif ( JANSSON_INCLUDE_DIR AND EXISTS "${JANSSON_INCLUDE_DIR}/jansson.h" )

find_package_handle_standard_args( Jansson
	FOUND_VAR JANSSON_FOUND
	REQUIRED_VARS JANSSON_INCLUDE_DIR JANSSON_LIBRARY
	VERSION_VAR JANSSON_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

set( JANSSON_LIBRARIES ${JANSSON_LIBRARY} )
set( JANSSON_INCLUDE_DIRS ${JANSSON_INCLUDE_DIR} )
set( JANSSON_DEFINITIONS "" )

set( JANSSON_LIBS ${JANSSON_LIBRARIES} )
set( JANSSON_INCLUDES ${JANSSON_INCLUDE_DIRS} )

mark_as_advanced( JANSSON_INCLUDE_DIR JANSSON_LIBRARY )

