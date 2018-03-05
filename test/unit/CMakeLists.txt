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

# Add directory for find cmake configuration files for mocking libraries
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/mock" )

# Remove some compiler warnings for clang
set_compiler_flag_if_supported(
	"-Wno-conversion"
	"-Wno-reserved-id-macro"
	"-Wno-shorten-64-to-32"
	"-Wno-sign-conversion"
	"-Wno-undef"
	"-Wno-unused-macros" )

# setup include directories
string( REPLACE "/unit/test" "/src" CURRENT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )
string( REPLACE "/unit/test" "/src" CURRENT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" )
include_directories( "${CURRENT_SOURCE_DIR}" )
include_directories( "${CURRENT_BINARY_DIR}/api/protocol-client/out" )
include_directories( "${CURRENT_BINARY_DIR}/build-info/out" )

# Supporting directories
add_subdirectory( "mock" )

# Directories containing tests
set( TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" )
set( TEST_BINARY_DIR "${CMAKE_SOURCE_DIR}/src" )
add_subdirectory( "api" )
add_subdirectory( "utilities" )

