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

cmake_minimum_required( VERSION 2.8.5 )

# Determine lib output directory suffix
set( LIB_SUFFIX "" )
get_property( LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS )
if( LIB64 )
	set( LIB_SUFFIX 64 )
endif()

include( ExternalProject )
set( GIT_PATCH_FLAGS "--ignore-space-change" "--ignore-whitespace" "-p1"
	"--whitespace=nowarn" "--unsafe-paths"
)
build_command( CMAKE_BUILD_COMMAND )
separate_arguments( CMAKE_BUILD_COMMAND )

# jsmn
set( JSMN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/jsmn" )
set( JSMN_INSTALL_DIR "${JSMN_BUILD_DIR}/out" )
ExternalProject_Add( "jsmn"
	GIT_REPOSITORY         "https://github.com/zserge/jsmn.git"
	GIT_TAG                "master"
	PREFIX                 "${JSMN_BUILD_DIR}"
	CONFIGURE_COMMAND      "make" "clean"
	BUILD_COMMAND          "make" "CFLAGS=-DJSMN_PARENT_LINKS=1 -DJSMN_STRICT=1 -fPIC"
	BUILD_IN_SOURCE        1
	INSTALL_DIR            "${JSMN_INSTALL_DIR}" # creates empty directory
	INSTALL_COMMAND        "${CMAKE_COMMAND}" "-E" "make_directory" "${JSMN_INSTALL_DIR}/lib${LIB_SUFFIX}"
	        COMMAND        "${CMAKE_COMMAND}" "-E" "copy" "${JSMN_BUILD_DIR}/src/jsmn/libjsmn.a" "${JSMN_INSTALL_DIR}/lib${LIB_SUFFIX}"
	        COMMAND        "${CMAKE_COMMAND}" "-E" "make_directory" "${JSMN_INSTALL_DIR}/include"
	        COMMAND        "${CMAKE_COMMAND}" "-E" "copy" "${JSMN_BUILD_DIR}/src/jsmn/jsmn.h" "${JSMN_INSTALL_DIR}/include"
	TEST_BEFORE_INSTALL    1
	TEST_COMMAND           "make" "test"
)

# paho
set( PAHO_BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/paho" )
set( PAHO_BINARY_DIR "${PAHO_BASE_DIR}/src/paho-build" )
set( PAHO_INSTALL_DIR  "${PAHO_BASE_DIR}/out" )
set( PAHO_SOURCE_DIR "${PAHO_BASE_DIR}/src/paho" )
string( REPLACE ";.;" ";${PAHO_SOURCE_DIR};" PAHO_BUILD_COMMAND "${CMAKE_BUILD_COMMAND}" )
ExternalProject_Add( "paho"
	GIT_REPOSITORY         "https://github.com/eclipse/paho.mqtt.c.git"
	GIT_TAG                "master" #"v1.1.0"
	PREFIX                 "${PAHO_BASE_DIR}"
	CMAKE_ARGS             "-DCMAKE_INSTALL_PREFIX:PATH=${PAHO_INSTALL_DIR}"
	                       "-DCMAKE_INSTALL_LIBDIR:PATH=lib${LIB_SUFFIX}"
	                       "-DPAHO_WITH_SSL:BOOL=TRUE"
	                       "-DPAHO_BUILD_STATIC:BOOL=TRUE"
	                       "-DCMAKE_C_FLAGS:STRING=-fPIC"
	#BINARY_DIR             "${PAHO_BINARY_DIR}"
	BUILD_COMMAND          ${PAHO_BUILD_COMMAND}
	BUILD_IN_SOURCE        1
	INSTALL_DIR            "${PAHO_INSTALL_DIR}" # creates empty directory
	INSTALL_COMMAND        ${CMAKE_MAKE_PROGRAM} "install"
	#TEST_BEFORE_INSTALL    1
	#TEST_COMMAND            ${CMAKE_MAKE_PROGRAM} "test"
)

