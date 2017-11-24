#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build-sys/cmake/modules" ${CMAKE_MODULE_PATH} )
set( IOT_CONFIG_FILE "build.yml" )
include ( ReadYamlFile )
read_yaml_file( ${IOT_CONFIG_FILE} )

cmake_minimum_required( VERSION 2.8.5 )
project( ${IOT_PRODUCT} )

if( WIN32 )
	set( IOT_USER "SYSTEM" )
endif()

# Generate Project Version information (from last git commit)
include( VersionSupport )
string( REGEX REPLACE "20([0-9][0-9])" "\\1" IOT_VERSION "${PROJECT_COMMIT_DATE}" )
string( REPLACE "-" "." IOT_VERSION "${IOT_VERSION}" )
version_split( IOT_VERSION "${IOT_VERSION}" )

# Project information
set( PROJECT_NAME                      "${CMAKE_PROJECT_NAME}" )
set( PROJECT_VERSION                   "${IOT_VERSION}" )
set( PROJECT_VENDOR                    "${IOT_VENDOR}" )

set( IOT_SERVICE_TARGET                "iot-service" )
set( IOT_SERVICE_NAME                  "${PROJECT_NAME} Core Service" )
set( IOT_SERVICE_DESCRIPTION           "Core aggregation service for ${PROJECT_NAME} devices." )
string( REPLACE ";" " " IOT_SERVICE_DESCRIPTION "${IOT_SERVICE_DESCRIPTION}" )

set( IOT_DEVICE_MANAGER_TARGET         "iot-device-manager" )
set( IOT_DEVICE_MANAGER_NAME           "${PROJECT_NAME} Device Manager" )
set( IOT_DEVICE_MANAGER_DESCRIPTION    "Registers and handles device level requests for ${PROJECT_NAME} devices."
                                       "Client that registers with ${IOT_SERVICE_TARGET} to handle over-the-air updates,"
                                       "device reboot/shutdown, agent reset etc." )
string( REPLACE ";" " " IOT_DEVICE_MANAGER_DESCRIPTION "${IOT_DEVICE_MANAGER_DESCRIPTION}" )

# Setup variables
# Determine if compiling in 64-bit mode
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
else()
	set( LIB_SUFFIX "" )
endif()

# Options
include( DefineCompilerFlags )
include( GetFullPath )
include( OptionSelect )
option_select( IOT_JSON_LIBRARY
	DESCRIPTION "JSON Library to use"
	DEFAULT "jsmn" "jansson" "jsmn" "json-c"
)
option_select( IOT_MQTT_LIBRARY
	DESCRIPTION "MQTT Library to use"
	DEFAULT "paho" "mosquitto" "paho"
)
option( IOT_NO_THREAD_SUPPORT "do not support the use of threads" OFF )
option( IOT_STACK_ONLY "build library without the use of the heap" OFF )

# Enforce Build Type
# set a default build type if none was specified
set( DEFAULT_BUILD_TYPE "Debug" )
if( NOT CMAKE_CONFIGURATION_TYPES )
	if ( NOT CMAKE_BUILD_TYPE )
		message( STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified." )
	endif ( NOT CMAKE_BUILD_TYPE )
	option_select( CMAKE_BUILD_TYPE DEFAULT ${DEFAULT_BUILD_TYPE} DESCRIPTION "Type of build"
		Coverage Debug MinSizeRel Release RelWithDebInfo )
	message( STATUS "Selected build type is '${CMAKE_BUILD_TYPE}'" )
endif( NOT CMAKE_CONFIGURATION_TYPES )

# Base build configuration file
set( IOT_IN_FILE  "iot_build.h.in" )
set( IOT_OUT_FILE "iot_build.h" )
set( IOT_GIT_SHA  "${PROJECT_GIT_SHA}" )
configure_file( "${IOT_IN_FILE}" "${CMAKE_BINARY_DIR}/${IOT_OUT_FILE}" @ONLY )

# Setup Project output directories
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/bin" )
# Location to find public include files when building sample applications
set( CMAKE_INCLUDE_OUTPUT_DIRECTORY    "${CMAKE_SOURCE_DIR}/src/api/public" )

# Installation directories
set( INSTALL_BIN_DIR      "bin" )
set( INSTALL_EXAMPLES_DIR "share/iot" )
set( INSTALL_INCLUDE_DIR  "include" )
set( INSTALL_LIB_DIR      "lib" )
set( INSTALL_PLUGIN_DIR   "lib" )

# 3rd party libraries
set( PAHO_PREFER_STATIC   ON )
set( PAHO_USE_SSL         ON )
if ( IOT_JSON_LIBRARY STREQUAL "jansson" )
	find_package( Jansson REQUIRED )
	add_definitions( "-DIOT_JSON_JANSSON" )
	set( JSON_INCLUDE_DIR "${JANSSON_INCLUDE_DIR}" )
	set( JSON_LIBRARIES   "${JANSSON_LIBRARIES}" )
elseif ( IOT_JSON_LIBRARY STREQUAL "json-c" )
	find_package( JsonC REQUIRED )
	add_definitions( "-DIOT_JSON_JSONC" )
	set( JSON_INCLUDE_DIR "${JSONC_INCLUDE_DIR}" )
	set( JSON_LIBRARIES   "${JSONC_LIBRARIES}" )
else()
	find_package( Jsmn )
	set( JSON_INCLUDE_DIR "${JSMN_INCLUDE_DIR}" )
	set( JSON_LIBRARIES   "${JSMN_LIBRARIES}" )
	set( JSON_DEFINES     "${JSMN_DEFINES}" )
endif()

if ( IOT_MQTT_LIBRARY STREQUAL "mosquitto" )
	find_package( Mosquitto REQUIRED )
	add_definitions( "-DIOT_MQTT_MOSQUITTO" )
	set( MQTT_INCLUDE_DIR "${MOSQUITTO_INCLUDE_DIR}" )
	set( MQTT_LIBRARIES "${MOSQUITTO_LIBRARIES}" )
	set( MQTT_SSL_SUPPORT ON )
else()
	find_package( Paho )
	set( MQTT_INCLUDE_DIR "${PAHO_INCLUDE_DIR}" )
	set( MQTT_LIBRARIES "${PAHO_LIBRARIES}" )
	set( MQTT_SSL_SUPPORT ${PAHO_SSL_SUPPORT} )
endif()

set( OSAL_PREFER_STATIC ON )
find_package( Osal REQUIRED )
include_directories( SYSTEM
	"${OSAL_INCLUDE_DIR}"
)

if( NOT MQTT_LIBRARIES OR NOT JSON_LIBRARIES )
	build_command( CMAKE_BUILD_COMMAND )
	separate_arguments( CMAKE_BUILD_COMMAND )
	set( 3RDPARTY_DIR "${CMAKE_CURRENT_BINARY_DIR}/3rdparty" )
	file( MAKE_DIRECTORY "${3RDPARTY_DIR}" )
	execute_process( COMMAND "${CMAKE_COMMAND}" "-G${CMAKE_GENERATOR}"
			"${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
		WORKING_DIRECTORY "${3RDPARTY_DIR}"
		RESULT_VARIABLE CMD_RESULT
		ERROR_VARIABLE CMD_ERROR )
	if( NOT CMD_RESULT STREQUAL "0" )
		message( FATAL_ERROR "Failed to obtain 3rd party libraries "
			"(error code: ${CMD_RESULT}): ${CMD_ERROR}" )
	endif( NOT CMD_RESULT STREQUAL "0" )
	execute_process( COMMAND ${CMAKE_BUILD_COMMAND}
		WORKING_DIRECTORY "${3RDPARTY_DIR}"
		RESULT_VARIABLE CMD_RESULT
		ERROR_VARIABLE CMD_ERROR )
	if( NOT CMD_RESULT STREQUAL "0" )
		message( FATAL_ERROR "Failed to build 3rd party libraries "
			"(error code: ${CMD_RESULT}): ${CMD_ERROR}" )
	endif( NOT CMD_RESULT STREQUAL "0" )

	# jsmn
	set( JSMN_ROOT_DIR "${3RDPARTY_DIR}/jsmn/out" )
	find_package( Jsmn REQUIRED )
	set( JSON_INCLUDE_DIR "${JSMN_INCLUDE_DIR}" )
	set( JSON_LIBRARIES   "${JSMN_LIBRARIES}" )
	set( JSON_DEFINES     "${JSMN_DEFINES}" )

	# paho
	set( PAHO_ROOT_DIR "${3RDPARTY_DIR}/paho/out" )
	find_package( Paho REQUIRED )
	set( MQTT_INCLUDE_DIR "${PAHO_INCLUDE_DIR}" )
	set( MQTT_LIBRARIES "${PAHO_LIBRARIES}" )
	set( MQTT_SSL_SUPPORT ${PAHO_SSL_SUPPORT} )
endif( NOT MQTT_LIBRARIES OR NOT JSON_LIBRARIES )

set( CMAKE_POSITION_INDEPENDENT_CODE ON )
include_directories( "src/api/public" "src" "${CMAKE_BINARY_DIR}" )
add_subdirectory( "src" )
add_subdirectory( "share" )

include( TestSupport )

