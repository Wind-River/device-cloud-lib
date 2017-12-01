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

include( CMakeParseArguments )

set( PLUGIN_BUILTIN_LIBS "" CACHE INTERNAL "list of built-in plug-ins" FORCE )
set( PLUGIN_BUILTIN_ENABLED "" CACHE INTERNAL "list of built-in enabled plug-ins" FORCE )

set( PLUGIN_BUILTIN_LIBS "${IOT_PLUGIN_BUILTIN}" CACHE INTERNAL "" FORCE )
set( PLUGIN_ENABLED )
foreach( PLUGIN ${IOT_PLUGIN_BUILTIN} )
	if ( IOT_PLUGIN_BUILTIN_${PLUGIN} )
		set( PLUGIN_ENABLED ${PLUGIN_ENABLED} "${PLUGIN}" )
	endif()
endforeach( PLUGIN )
set( PLUGIN_BUILTIN_ENABLED "${PLUGIN_ENABLED}" CACHE INTERNAL "" FORCE )

# Helper function for building an iot plugin either built-in into the library
# or as a run-time external loadable plug-in
function( ADD_IOT_PLUGIN PLUGIN_NAME )
	cmake_parse_arguments( PLUGIN "BUILTIN;ENABLED" "" "LIBS;SRCS" ${ARGN} )
	set( PLUGIN_SRCS ${PLUGIN_SRCS} ${PLUGIN_UNPARSED_ARGUMENTS} )

	if( PLUGIN_BUILTIN )
		set( PLUGIN_LIBRARY_TYPE "STATIC" )
	else()
		set( PLUGIN_LIBRARY_TYPE "MODULE" )
	endif()
	add_library( "${PLUGIN_NAME}" ${PLUGIN_LIBRARY_TYPE} ${PLUGIN_SRCS} )
	target_link_libraries( "${PLUGIN_NAME}" ${PLUGIN_LIBS} )

	if( PLUGIN_BUILTIN )
		set( PLUGIN_BUILTIN_LIBS ${PLUGIN_BUILTIN_LIBS}
			"${PLUGIN_NAME}" )
		set( PLUGIN_BUILTIN_ENABLED ${PLUGIN_BUILTIN_ENABLED}
			"${PLUGIN_NAME}" )
		list( REMOVE_DUPLICATES PLUGIN_BUILTIN_LIBS )
		list( REMOVE_DUPLICATES PLUGIN_BUILTIN_ENABLED )
		set( PLUGIN_BUILTIN_LIBS "${PLUGIN_BUILTIN_LIBS}"
			CACHE INTERNAL "" FORCE )
		set( PLUGIN_BUILTIN_ENABLED "${PLUGIN_BUILTIN_ENABLED}"
			CACHE INTERNAL "" FORCE )
		set_target_properties( "${PLUGIN_NAME}" PROPERTIES
			COMPILE_FLAGS "-DIOT_PLUGIN_BUILTIN" )
	else()
		install( TARGETS "${PLUGIN_NAME}"
			LIBRARY DESTINATION "${INSTALL_PLUGIN_DIR}"
			COMPONENT plugins
		)
	endif()
	add_definitions( "-fPIC" )
endfunction( ADD_IOT_PLUGIN )

#add_subdirectory( "device_manager" )
#add_subdirectory( "saf" )
add_subdirectory( "tr50" )
#add_subdirectory( "udmp" )

set( IOT_PLUGIN_BUILTIN_ENABLE )
set( IOT_PLUGIN_BUILTIN_INCS )
set( IOT_PLUGIN_BUILTIN_IMPL )
foreach( PLUGIN_BUILTIN_NAME ${PLUGIN_BUILTIN_LIBS} )
	set( IOT_PLUGIN_BUILTIN_INCS "${PLUGIN_BUILTIN_INCS}/**
 @brief internal function to load the ${PLUGIN_BUILTIN_NAME} plug-in
 @param[out]       p                   location to load plug-in to
 @retval           IOT_TRUE            successfully loaded plug-in
 @retval           IOT_FALSE           failed to load plug-in
 */
iot_bool_t ${PLUGIN_BUILTIN_NAME}_load( iot_plugin_t *p );
"
	)
	set( IOT_PLUGIN_BUILTIN_IMPL "${PLUGIN_BUILTIN_IMPL}/* ${PLUGIN_BUILTIN_NAME} */
		if ( (lib->plugin_count + result < max) && ${PLUGIN_BUILTIN_NAME}_load( lib->plugin_ptr[lib->plugin_count + result] ) ) { ++result; }"
	)
	list( FIND "${PLUGIN_BUILTIN_ENABLED}" "${PLUGIN_NAME}" PLUGIN_ENABLE )
	if( PLUGIN_ENABLE )
		set( IOT_PLUGIN_BUILTIN_ENABLE "${IOT_PLUGIN_BUILTIN_ENABLE}/* ${PLUGIN_BUILTIN_NAME} */
		if ( iot_plugin_enable( lib, \"${PLUGIN_BUILTIN_NAME}\" ) != IOT_STATUS_SUCCESS ) result = IOT_FALSE;"
		)
	endif()
endforeach( PLUGIN_BUILTIN_NAME )
set( IOT_PLUGIN_BUILTIN_FILE_IN  "iot_plugin_builtin.c.in" )
set( IOT_PLUGIN_BUILTIN_FILE_OUT "${CMAKE_CURRENT_BINARY_DIR}/iot_plugin_builtin.c" )

configure_file( "${IOT_PLUGIN_BUILTIN_FILE_IN}"
	"${IOT_PLUGIN_BUILTIN_FILE_OUT}" @ONLY )
set( API_SRCS_C ${API_SRCS_C} "${IOT_PLUGIN_BUILTIN_FILE_OUT}"
	CACHE INTERNAL "" FORCE )

