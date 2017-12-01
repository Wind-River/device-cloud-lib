#
# Jsmn support
#
# The following variables can be set to add additional find support:
# - JSMN_ROOT_DIR, specified an explicit root path to test
#
# If found the following will be defined:
# - JSMN_DEFINES, definitions JSMN library was built with
# - JSMN_FOUND, If false, do not try to use jsmn
# - JSMN_INCLUDE_DIR, path where to find jsmn include files
# - JSMN_LIBRARIES, the library to link against
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
if ( NOT JSMN_ROOT_DIR )
	set( JSMN_ROOT_DIR ${DEPENDS_ROOT_DIR} )
endif()

find_path( JSMN_INCLUDE_DIR
	NAMES "jsmn.h"
	DOC "jsmn include directory"
	PATHS "${JSMN_ROOT_DIR}/include"
)

find_library( JSMN_LIBRARIES
	NAMES jsmn
	DOC "Required jsmn libraries"
	PATHS "${JSMN_ROOT_DIR}/lib${LIB_SUFFIX}"
)

if( JSMN_INCLUDE_DIR AND JSMN_LIBRARIES )
	# Determine flags JSMN was compiled with
	set( JSMN_TEST_OUT_FILE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/jsmn_test.c" )
	file( WRITE "${JSMN_TEST_OUT_FILE}"
		"#define JSMN_PARENT_LINKS\n"
		"#ifdef __clang__\n"
		"#pragma clang diagnostic push\n"
		"#pragma clang diagnostic ignored \"-Wreserved-id-macro\"\n"
		"#endif /* ifdef __clang__ */\n"
		"#include <jsmn.h>\n"
		"#ifdef __clang__\n"
		"#pragma clang diagnostic pop\n"
		"#endif /* ifdef __clang__ */\n"
		"#include <stdio.h>\n"
		"int main( void ) {\n"
		"  const char *const js = \"{\\\"key\\\":bad_value}\";\n"
		"  jsmn_parser p;\n"
		"  jsmntok_t t[5];\n;"
		"  jsmn_init( &p );\n"
		"  if(jsmn_parse( &p, js, 15, &t[0], 5) == JSMN_ERROR_INVAL)\n"
		"    printf(\"JSMN_STRICT\\n\");\n"
		"  if(t[0].parent == -1)\n"
		"    printf(\"JSMN_PARENT_LINKS\\n\");\n"
		"  return 0;\n"
		"}\n"
	)
	try_run( JSMN_RUN_RESULT JSMN_COMPILE_RESULT
		"${CMAKE_BINARY_DIR}" "${JSMN_TEST_OUT_FILE}"
		CMAKE_FLAGS
			"-DINCLUDE_DIRECTORIES:STRING=${JSMN_INCLUDE_DIR}"
			"-DLINK_LIBRARIES:STRING=${JSMN_LIBRARIES}"
		COMPILE_OUTPUT_VARIABLE JSMN_COMPILE
		RUN_OUTPUT_VARIABLE JSMN_DEFINES )
	string( STRIP "${JSMN_DEFINES}" JSMN_DEFINES )
	string( REPLACE "\n" ";" JSMN_DEFINES "${JSMN_DEFINES}" )
	if( NOT JSMN_COMPILE_RESULT )
		message( FATAL_ERROR "Failed to compile simple JSMN application:\n${JSMN_COMPILE}" )
	endif( NOT JSMN_COMPILE_RESULT )
endif( JSMN_INCLUDE_DIR AND JSMN_LIBRARIES )

find_package_handle_standard_args( Jsmn
	FOUND_VAR JSMN_FOUND
	REQUIRED_VARS JSMN_INCLUDE_DIR JSMN_LIBRARIES
	FAIL_MESSAGE DEFAULT_MSG
)

mark_as_advanced(
	JSMN_INCLUDE_DIR
	JSMN_LIBRARIES
)

