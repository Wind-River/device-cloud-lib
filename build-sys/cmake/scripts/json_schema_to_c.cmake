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

if ( NOT JSON_SCHEMA_VARIABLE )
	message( FATAL_ERROR "JSON_SCHEMA_VARIABLE must be set to use this script." )
endif ( NOT JSON_SCHEMA_VARIABLE )

if ( NOT JSON_SCHEMA_FILE )
	message( FATAL_ERROR "JSON_SCHEMA_FILE must be set to use this script." )
endif( NOT JSON_SCHEMA_FILE )

if ( NOT EXISTS "${JSON_SCHEMA_FILE}" )
	message( FATAL_ERROR "${JSON_SCHEMA_FILE} doesn't exist" )
endif ( NOT EXISTS "${JSON_SCHEMA_FILE}" )

get_filename_component( JSON_SCHEMA_FILE_NAME "${JSON_SCHEMA_FILE}" NAME )

if ( NOT JSON_SCHEMA_HEADER_FILE )
	set( JSON_SCHEMA_HEADER_FILE "${JSON_SCHEMA_FILE_NAME}.h" )
endif ( NOT JSON_SCHEMA_HEADER_FILE )

if ( NOT JSON_SCHEMA_SOURCE_FILE )
	set( JSON_SCHEMA_SOURCE_FILE "${JSON_SCHEMA_FILE_NAME}.c" )
endif ( NOT JSON_SCHEMA_SOURCE_FILE )


file( READ "${JSON_SCHEMA_FILE}" SCHEMA )
file( WRITE "${JSON_SCHEMA_SOURCE_FILE}"
	"#include \"${JSON_SCHEMA_HEADER_FILE}\"\n\n"
	"const char *const ${JSON_SCHEMA_VARIABLE} =" )
string( REPLACE "\n" ";" SCHEMA "${SCHEMA}" )
foreach( SCHEMA_LINE ${SCHEMA} )
	string( REPLACE "\"" "\\\"" SCHEMA_LINE "${SCHEMA_LINE}" )
	string( REGEX MATCH "^([ \t]+)" LINE_TABS "${SCHEMA_LINE}" )
	string( REGEX REPLACE "^[ \t]+" "" SCHEMA_LINE "${SCHEMA_LINE}" )
	file( APPEND "${JSON_SCHEMA_SOURCE_FILE}"
		"\n${LINE_TABS}\"${SCHEMA_LINE}\"" )
endforeach( SCHEMA_LINE )
file( APPEND "${CMAKE_CURRENT_BINARY_DIR}/${JSON_SCHEMA_SOURCE_FILE}" ";\n" )

get_filename_component( HEADER_FILE_NAME "${JSON_SCHEMA_HEADER_FILE}" NAME )
string( REGEX REPLACE "[ -.]" "_" SCHEMA_INCLUDE_GUARD "${HEADER_FILE_NAME}" )
string( TOUPPER "${SCHEMA_INCLUDE_GUARD}" SCHEMA_INCLUDE_GUARD )
file( WRITE "${JSON_SCHEMA_HEADER_FILE}"
	"#ifndef ${SCHEMA_INCLUDE_GUARD}\n"
	"\textern const char *const ${JSON_SCHEMA_VARIABLE};\n"
	"#endif /* ifndef ${SCHEMA_INCLUDE_GUARD} */\n" )

