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

find_package( Doxygen )

# Helper that will generate a target for generating doxygen documenation for a
# portion of the product.  It will create a new command using the tag
# specified.  The Doxyfile input file containing the tag must exist.  The target
# generated will be of the form "doc-%TAG%".
#
# Arguments:
#   TAG - tag to use when generating the documentation
function( add_doxygen_support TAG )
	set( DOXYFILE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/build-sys/cmake/doxygen" )
	if( DOXYGEN_FOUND )
		string( TIMESTAMP DATE_YEAR "%Y" UTC )
		string( TIMESTAMP DATE_MONTH "%m" UTC )
		string( TIMESTAMP DATE_DAY "%d" UTC )
		string( TIMESTAMP DATE_TIME "%H:%M" UTC )
		string( TOLOWER "${TAG}" TAG )
		configure_file( "${DOXYFILE_SOURCE_DIR}/Doxyfile-${TAG}.in" "${CMAKE_BINARY_DIR}/Doxyfile-${TAG}" @ONLY )
		configure_file( "${DOXYFILE_SOURCE_DIR}/footer.html.in" "${CMAKE_BINARY_DIR}/footer.html" @ONLY )

		add_custom_target( doc-${TAG}
			COMMAND "${CMAKE_COMMAND}" -E echo "Generating ${TAG} documentation..."
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/doxygen"
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_BINARY_DIR}/doxygen/${TAG}"
			COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_BINARY_DIR}/Doxyfile-${TAG}"
			COMMAND "${CMAKE_COMMAND}" -E echo_append "Complete"
			DEPENDS "${CMAKE_BINARY_DIR}/Doxyfile-${TAG}"
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			COMMENT "Generating ${TAG} Documentation" VERBATIM
		)
	endif( DOXYGEN_FOUND )
endfunction( add_doxygen_support )

# Doxygen support
if( DOXYGEN_FOUND )
	add_doxygen_support( dev )
	add_custom_target( doc
		DEPENDS doc-api doc-dev
		COMMENT "Generating Documentation" VERBATIM
	)
	set_directory_properties( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
		"${CMAKE_BINARY_DIR}/doxygen" )
endif( DOXYGEN_FOUND )

add_subdirectory( "api" )
add_subdirectory( "control" )
add_subdirectory( "device-manager" )
add_subdirectory( "relay" )
add_subdirectory( "update" )
add_subdirectory( "utilities" )

