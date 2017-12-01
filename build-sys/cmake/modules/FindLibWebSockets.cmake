#
# LibWebSockets support
#
# The following variables can be set to add additional find support:
# - LIBWEBSOCKETS_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - LIBWEBSOCKETS_FOUND, If false, do not try to use libWebSockets
# - LIBWEBSOCKETS_INCLUDE_DIR, path where to find libwebsockets.h
# - LIBWEBSOCKETS_LIBRARY_DIR, path where to find libwebsockets.so
# - LIBWEBSOCKETS_LIBRARIES, the library to link against
# - LIBWEBSOCKETS_VERSION, update libwebsockets to display version
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
if ( NOT LIBWEBSOCKETS_ROOT_DIR )
	set( LIBWEBSOCKETS_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( LIBWEBSOCKETS_INCLUDE_DIR
	NAMES libwebsockets.h
	DOC "libwebsockets include directory"
	PATHS "${LIBWEBSOCKETS_ROOT_DIR}/include"
)

find_library( LIBWEBSOCKETS_LIBRARIES
	NAMES websockets
	DOC "Required libwebsockets libraries"
	PATHS "${LIBWEBSOCKETS_ROOT_DIR}/lib${LIB_SUFFIX}"
)

get_filename_component( LIBWEBSOCKETS_LIBRARY_DIR
	${LIBWEBSOCKETS_LIBRARIES} PATH )

# determine version
if ( LIBWEBSOCKETS_INCLUDE_DIR AND EXISTS "${LIBWEBSOCKETS_INCLUDE_DIR}/lws_config.h" )
	set( _LIBWEBSOCKETS_VERSION_REGEX
		"^#define[ \t]+LWS_LIBRARY_VERSION[ \t]+\"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*$")
	file( STRINGS "${LIBWEBSOCKETS_INCLUDE_DIR}/lws_config.h"
		_LIBWEBSOCKETS_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_LIBWEBSOCKETS_VERSION_REGEX}"
	)
	if ( _LIBWEBSOCKETS_VERSION_STRING )
		string( REGEX REPLACE "${_LIBWEBSOCKETS_VERSION_REGEX}"
			"\\1.\\2.\\3" LIBWEBSOCKETS_VERSION
			"${_LIBWEBSOCKETS_VERSION_STRING}")
	endif ( _LIBWEBSOCKETS_VERSION_STRING )
	unset( _LIBWEBSOCKETS_VERSION_REGEX )
	unset( _LIBWEBSOCKETS_VERSION_STRING )
endif ( LIBWEBSOCKETS_INCLUDE_DIR AND EXISTS "${LIBWEBSOCKETS_INCLUDE_DIR}/lws_config.h" )

find_package_handle_standard_args( LibWebSockets
	FOUND_VAR LIBWEBSOCKETS_FOUND
	REQUIRED_VARS LIBWEBSOCKETS_INCLUDE_DIR LIBWEBSOCKETS_LIBRARIES
	VERSION_VAR LIBWEBSOCKETS_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	LIBWEBSOCKETS_LIBRARY_DIR
	LIBWEBSOCKETS_INCLUDE_DIR
	LIBWEBSOCKETS_LIBRARIES
)

