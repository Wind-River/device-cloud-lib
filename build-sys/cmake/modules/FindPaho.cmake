#
# Paho support
#
# The following variables can be set to add additional find support:
# - PAHO_ASYNC, If true, tries to find asynchronous C library
# - PAHO_PREFER_STATIC, If true, only finds static library version
# - PAHO_USE_SSL, If true, only finds ssl version
# - PAHO_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - PAHO_FOUND, If false, paho was not found on the system
# - PAHO_SSL_SUPPORT, If true, SSL supported version of library found
# - PAHO_INCLUDE_DIR, path to directory containing paho include files (based on PAHO_ASYNC)
# - PAHO_LIBRARIES, the library to link against (based on PAHO_ASYNC)
# - PAHO_VERSION, version of paho that was found
# - PAHO_ASYNC_INCLUDE_DIR, path to directory containing paho async include files
# - PAHO_ASYNC_LIBRARIES, path to directory containing paho async libraries
# - PAHO_SYNC_INCLUDE_DIR, path to directory containing paho async include files
# - PAHO_SYNC_LIBRARIES, path to directory containing paho async libraries
#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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

# Define variables for paho synchronous & asynchronous version of the library
set( PAHO_LIB_TYPES          "ASYNC" "SYNC" )
set( PAHO_ASYNC_TYPE         "MQTTAsync" )
set( PAHO_ASYNC_LIB_NAME     "paho-mqtt3a" )
set( PAHO_SYNC_TYPE          "MQTTClient" )
set( PAHO_SYNC_LIB_NAME      "paho-mqtt3c" )

# Set the type of paho library the user would like to use
set( PAHO_LIB "SYNC" )
if ( PAHO_ASYNC )
	set( PAHO_LIB "ASYNC" )
endif( PAHO_ASYNC )

# List of libraries to search for
set( PAHO_LIBS )
foreach( _LIB_TYPE ${PAHO_LIB_TYPES} )
	# Support for SSL library
	if( PAHO_USE_SSL )
		set( PAHO_${_LIB_TYPE}_LIB_NAME "${PAHO_${_LIB_TYPE}_LIB_NAME}s" )
	endif( PAHO_USE_SSL )

	# List of SSL libraries to find
	set( PAHO_${_LIB_TYPE}_LIBS
		"${CMAKE_SHARED_LIBRARY_PREFIX}${PAHO_${_LIB_TYPE}_LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}"
		"${CMAKE_STATIC_LIBRARY_PREFIX}${PAHO_${_LIB_TYPE}_LIB_NAME}-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
	)

	# Reverse list if static libraries are preferred
	if ( PAHO_PREFER_STATIC )
		list( REVERSE PAHO_${_LIB_TYPE}_LIBS )
	endif()
endforeach( _LIB_TYPE )

# Allow the ability to specify a global dependency root directory
if ( NOT PAHO_ROOT_DIR )
	set( PAHO_ROOT_DIR "${DEPENDS_ROOT_DIR}" )
endif()

foreach( _LIB_TYPE ${PAHO_LIB_TYPES} )
	string( TOLOWER "${_LIB_TYPE}" _LIB_TYPE_LC )
	find_path( PAHO_${_LIB_TYPE}_INCLUDE_DIR NAMES
		"${PAHO_${_LIB_TYPE}_TYPE}.h"
		DOC "paho ${_LIB_TYPE_LC} include directory"
		PATHS "${PAHO_ROOT_DIR}/include" )
	find_library( PAHO_${_LIB_TYPE}_LIBRARIES NAMES
		${PAHO_${_LIB_TYPE}_LIBS}
		DOC "Required paho ${_LIB_TYPE_LC} libraries"
		PATHS "${PAHO_ROOT_DIR}/lib" )
endforeach( _LIB_TYPE )

set( PAHO_TYPE "${PAHO_${PAHO_LIB}_TYPE}" )
set( PAHO_INCLUDE_DIR "${PAHO_${PAHO_LIB}_INCLUDE_DIR}" CACHE
	PATH "paho include directory" FORCE )
set( PAHO_LIBRARIES "${PAHO_${PAHO_LIB}_LIBRARIES}" CACHE
	FILEPATH "Required paho libraries" FORCE )

# If SSL version of paho then set applicable variable
if ( PAHO_LIBRARIES AND PAHO_USE_SSL )
	set( PAHO_SSL_SUPPORT ON )
else()
	set( PAHO_SSL_SUPPORT OFF )
endif()

# Determine version
if ( PAHO_INCLUDE_DIR AND PAHO_LIBRARIES )
	set( TEST_LIBS ${PAHO_LIBRARIES} )
	if ( PAHO_SSL_SUPPORT )
		find_package( OpenSSL )
		set( TEST_LIBS ${TEST_LIBS} ${OPENSSL_LIBRARIES} )
	endif( PAHO_SSL_SUPPORT )
	find_package( Threads )
	set( PAHO_TEST_OUT_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/paho_test.c" )

	file( WRITE "${PAHO_TEST_OUT_FILE}"
		"#include <${PAHO_TYPE}.h>\n"
		"#include <stdio.h>\n"
		"int main( void ) {\n"
		"  int i;\n"
		"  const ${PAHO_TYPE}_nameValue *nv = ${PAHO_TYPE}_getVersionInfo();\n"
		"  for ( i = 0; nv && nv->name; ++nv, ++i ) {\n"
		"    if ( i == 1 ) { printf( \"%s\\n\", nv->value ); return 0; }\n"
		"  }\n"
		"  return 1;\n"
		"}\n"
	)
	try_run( PAHO_VERSION_RUN_RESULT PAHO_VERSION_COMPILE_RESULT
		"${CMAKE_BINARY_DIR}" "${PAHO_TEST_OUT_FILE}"
		CMAKE_FLAGS
			"-DINCLUDE_DIRECTORIES:STRING=${PAHO_INCLUDE_DIR}"
			"-DLINK_LIBRARIES:STRING=${TEST_LIBS};${CMAKE_THREAD_LIBS_INIT}"
		RUN_OUTPUT_VARIABLE PAHO_VERSION )
	string( STRIP "${PAHO_VERSION}" PAHO_VERSION )
endif ( PAHO_INCLUDE_DIR AND PAHO_LIBRARIES )

find_package_handle_standard_args( Paho
	FOUND_VAR PAHO_FOUND
	REQUIRED_VARS PAHO_INCLUDE_DIR PAHO_LIBRARIES
	VERSION_VAR PAHO_VERSION
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	PAHO_INCLUDE_DIR
	PAHO_LIBRARIES
)

