#
# Civetweb support
#
# The following variables can be set to add additional find support:
# - CIVETWEB_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - CIVETWEB_DEFINITIONS, defintions to add to use the library
# - CIVETWEB_FOUND, If false, do not try to use civetweb library
# - CIVETWEB_INCLUDE_DIR, path where to find civetweb.h
# - CIVETWEB_LIBRARY_DIR, path where to find libcivetweb.a
# - CIVETWEB_LIBRARIES, the library to link against
# - CIVETWEB_VERSION, update civetweb to display version
#
# Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
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
if ( NOT CIVETWEB_ROOT_DIR )
	set( CIVETWEB_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( CIVETWEB_INCLUDE_DIR
	NAMES civetweb.h
	DOC "civetweb include directory"
	PATHS "${CIVETWEB_ROOT_DIR}/include"
)

find_library( CIVETWEB_LIBRARIES
	NAMES civetweb
	DOC "Required civetweb libraries"
	PATHS "${CIVETWEB_ROOT_DIR}/lib${LIB_SUFFIX}"
	      "${CIVETWEB_ROOT_DIR}/lib"
)

if ( CIVETWEB_LIBRARIES AND WIN32 )
	set( CIVETWEB_DEFINITIONS "-DCIVETWEB_DLL_IMPORTS=1"
		CACHE INTERNAL "" FORCE )
endif ( CIVETWEB_LIBRARIES AND WIN32 )

find_program( CIVETWEB_EXECUTABLE
	NAMES civetweb
	DOC "Civetweb server executable"
	PATHS "${CIVETWEB_ROOT_DIR}/bin"
)

get_filename_component( CIVETWEB_LIBRARY_DIR
	${CIVETWEB_LIBRARIES} PATH )

# determine version
if ( CIVETWEB_INCLUDE_DIR AND EXISTS "${CIVETWEB_INCLUDE_DIR}/civetweb.h" )
	set( _CIVETWEB_VERSION_REGEX
		"^#define[ \t]+CIVETWEB_VERSION[ \t]+\"([0-9.a-zA-Z]+)\".*$")
	file( STRINGS "${CIVETWEB_INCLUDE_DIR}/civetweb.h"
		_CIVETWEB_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_CIVETWEB_VERSION_REGEX}"
	)
	if ( _CIVETWEB_VERSION_STRING )
		string( REGEX REPLACE "${_CIVETWEB_VERSION_REGEX}"
			"\\1" CIVETWEB_VERSION
			"${_CIVETWEB_VERSION_STRING}")
	endif ( _CIVETWEB_VERSION_STRING )
	unset( _CIVETWEB_VERSION_REGEX )
	unset( _CIVETWEB_VERSION_STRING )
endif ( CIVETWEB_INCLUDE_DIR AND EXISTS "${CIVETWEB_INCLUDE_DIR}/civetweb.h" )

find_package_handle_standard_args( Civetweb
	FOUND_VAR CIVETWEB_FOUND
	REQUIRED_VARS CIVETWEB_EXECUTABLE CIVETWEB_INCLUDE_DIR CIVETWEB_LIBRARIES
	VERSION_VAR CIVETWEB_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	CIVETWEB_INCLUDE_DIR
	CIVETWEB_EXECUTABLE
	CIVETWEB_LIBRARY_DIR
	CIVETWEB_LIBRARIES
)

