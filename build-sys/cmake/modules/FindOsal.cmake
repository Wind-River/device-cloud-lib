#
# Operating System Abstraction Library
#
# The following variables can be set to add additional find support:
# - OSAL_PREFER_STATIC, If true, tries to find static library versions
# - OSAL_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - OSAL_FOUND, If false, do not try to use osal
# - OSAL_INCLUDE_DIR, path where to find osal include files
# - OSAL_LIBRARIES, the library to link against
# - OSAL_VERSION, update osal to display version
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

set( _PROGRAMFILES     "ProgramFiles" )
set( _PROGRAMFILES_X86 "ProgramFiles(x86)" )
set( OSAL_LIBS
	${CMAKE_SHARED_LIBRARY_PREFIX}osal${CMAKE_SHARED_LIBRARY_SUFFIX}
	${CMAKE_STATIC_LIBRARY_PREFIX}osal${CMAKE_STATIC_LIBRARY_SUFFIX}
)

if ( OSAL_PREFER_STATIC )
	list( REVERSE OSAL_LIBS )
endif( OSAL_PREFER_STATIC )

# Try and find paths
set( LIB_SUFFIX "" )
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
endif()

# Allow the ability to specify a global dependency root directory
if ( NOT OSAL_ROOT_DIR )
	set( OSAL_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( OSAL_INCLUDE_DIR
	NAMES "os.h"
	DOC "osal include directory"
	PATHS "${OSAL_ROOT_DIR}/include"
	      "$ENV{${_PROGRAMFILES}}/osal/include"
	      "$ENV{${_PROGRAMFILES_X86}}/osal/include"
)

find_library( OSAL_LIBRARIES
	NAMES ${OSAL_LIBS}
	DOC "Required osal libraries"
	PATHS "${OSAL_ROOT_DIR}/lib${LIB_SUFFIX}"
	      "$ENV{${_PROGRAMFILES}}/osal/lib"
	      "$ENV{${_PROGRAMFILES_X86}}/osal/lib"
)

# determine version
if ( OSAL_INCLUDE_DIR AND EXISTS "${OSAL_INCLUDE_DIR}/os.h" )
	set( _OSAL_VERSION_REGEX
		"^[ \t]*#[ \t]*define[ \t]+OS_LIB_VERSION[ \t]+\"([0-9a-zA-Z][0-9a-zA-Z_.]*[0-9a-zA-Z])\".*$")
	file( STRINGS "${OSAL_INCLUDE_DIR}/os.h"
		_OSAL_VERSION_STRING
		LIMIT_COUNT 1
		REGEX "${_OSAL_VERSION_REGEX}"
	)
	if ( _OSAL_VERSION_STRING )
		string( REGEX REPLACE "${_OSAL_VERSION_REGEX}"
			"\\1" OSAL_VERSION
			"${_OSAL_VERSION_STRING}")
	endif ( _OSAL_VERSION_STRING )
	unset( _OSAL_VERSION_REGEX )
	unset( _OSAL_VERSION_STRING )
endif ( OSAL_INCLUDE_DIR AND EXISTS "${OSAL_INCLUDE_DIR}/os.h" )

find_package_handle_standard_args( Osal
	FOUND_VAR OSAL_FOUND
	REQUIRED_VARS OSAL_LIBRARIES OSAL_INCLUDE_DIR
	VERSION_VAR OSAL_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	OSAL_LIBRARY_DIR
	OSAL_INCLUDE_DIR
	OSAL_LIBRARIES
)

