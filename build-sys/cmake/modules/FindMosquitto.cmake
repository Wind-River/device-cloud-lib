#
# Mosquitto support
#
# The following variables can be set to add additional find support:
# - MOSQUITTO_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - MOSQUITTO_FOUND - System has mosquitto
# - MOSQUITTO_INCLUDE_DIRS/MOSQUITTO_INCLUDES - Include directories for mosquitto
# - MOSQUITTO_LIBRARIES/MOSQUITTO_LIBS - Libraries needed for mosquitto
# - MOSQUITTO_DEFINITIONS - Compiler switches required for mosquitto
# - MOSQUITTO_VERSION - Library version
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

find_package( PkgConfig )
include( FindPackageHandleStandardArgs )

# Try and find paths
set( LIB_SUFFIX "" )
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
endif()

# Allow the ability to specify a global dependency root directory
if ( NOT MOSQUITTO_ROOT_DIR )
	set( MOSQUITTO_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( MOSQUITTO_INCLUDE_DIR
	NAMES mosquitto.h
	DOC "Mosquitto include directory"
	PATHS "${MOSQUITTO_ROOT_DIR}/include"
)

find_library( MOSQUITTO_LIBRARY
	NAMES mosquitto
	DOC "Required mosquitto libraries"
	PATHS "${MOSQUITTO_ROOT_DIR}/lib${LIB_SUFFIX}"
)

# determine version
if ( MOSQUITTO_INCLUDE_DIR AND EXISTS "${MOSQUITTO_INCLUDE_DIR}/mosquitto.h" )
	set( _MOSQUITTO_VERSION_REGEX
		"^#define[ \t]+LIBMOSQUITTO_[A-Z]+[ \t]+([0-9]+).*$" )
	file( STRINGS "${MOSQUITTO_INCLUDE_DIR}/mosquitto.h"
		_MOSQUITTO_VERSION_STRINGS
		REGEX "${_MOSQUITTO_VERSION_REGEX}"
	)
	set( MOSQUITTO_VERSION )
	foreach( _MOSQUITTO_VERSION_STRING ${_MOSQUITTO_VERSION_STRINGS} )
		string( REGEX REPLACE "${_MOSQUITTO_VERSION_REGEX}"
			"${MOSQUITTO_VERSION}.\\1" MOSQUITTO_VERSION
			"${_MOSQUITTO_VERSION_STRING}")
		string( REGEX REPLACE "^\\." "" MOSQUITTO_VERSION
			"${MOSQUITTO_VERSION}" )
	endforeach( _MOSQUITTO_VERSION_STRING )
	unset( _MOSQUITTO_VERSION_REGEX )
	unset( _MOSQUITTO_VERSION_STRING )
	unset( _MOSQUITTO_VERSION_STRINGS )
endif ( MOSQUITTO_INCLUDE_DIR AND EXISTS "${MOSQUITTO_INCLUDE_DIR}/mosquitto.h" )

find_package_handle_standard_args( Mosquitto
	FOUND_VAR MOSQUITTO_FOUND
	REQUIRED_VARS MOSQUITTO_INCLUDE_DIR MOSQUITTO_LIBRARY
	VERSION_VAR MOSQUITTO_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

set( MOSQUITTO_LIBRARIES ${MOSQUITTO_LIBRARY} )
set( MOSQUITTO_INCLUDE_DIRS ${MOSQUITTO_INCLUDE_DIR} )
set( MOSQUITTO_DEFINITIONS "" )

set( MOSQUITTO_LIBS ${MOSQUITTO_LIBRARIES} )
set( MOSQUITTO_INCLUDES ${MOSQUITTO_INCLUDE_DIRS} )

mark_as_advanced( MOSQUITTO_INCLUDE_DIR MOSQUITTO_LIBRARY )

