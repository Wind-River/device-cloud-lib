#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# Save variable values for all variables for the current installer
get_cmake_property( CMAKE_VARIABLES VARIABLES )
foreach( CMAKE_VARIABLE ${CMAKE_VARIABLES} )
	if ( CMAKE_VARIABLE MATCHES "CPACK_${CPACK_GENERATOR}_.*" )
		set( "${CMAKE_VARIABLE}" "${${CMAKE_VARIABLE}}" PARENT_SCOPE )
	endif ( CMAKE_VARIABLE MATCHES "CPACK_${CPACK_GENERATOR}_.*" )
endforeach( CMAKE_VARIABLE )

# Generate installer scripts
set( INSTALL_SCRIPTS
	"linux_pre_install.sh\;preinst"
	"linux_post_install.sh\;postinst"
	"linux_pre_uninstall.sh\;prerm"
	"linux_post_uninstall.sh\;postrm"
)

foreach( SCRIPT_PAIR ${INSTALL_SCRIPTS} )
	string( REGEX REPLACE ";.*" "" SCRIPT_IN "${SCRIPT_PAIR}" )
	string( REPLACE "${SCRIPT_IN};" "" SCRIPT_OUT "${SCRIPT_PAIR}" )
	if ( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_IN}" )
		configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_IN}"
			"${CMAKE_BINARY_DIR}/${SCRIPT_OUT}" @ONLY )
	endif ( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_IN}" )
endforeach( SCRIPT_PAIR )

configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/CPackOptions.cmake.in"
	"${CMAKE_BINARY_DIR}/CPackOptions.cmake" @ONLY )
set( CPACK_PROJECT_CONFIG_FILE "${CMAKE_BINARY_DIR}/CPackOptions.cmake"
	PARENT_SCOPE )

# Add options for Packaging
# Packaging options
if ( WIN32 )
	set( GENERATOR_DEFAULT "WIX" )
	set( GENERATOR_LIST "NSIS" "WIX" )
else()
	set( GENERATOR_DEFAULT "DEB" )
	find_program( RPMBUILD_EXECUTABLE rpmbuild
		DOC "path to rpmbuild executable" )
	mark_as_advanced( RPMBUILD_EXECUTABLE )

	if ( RPMBUILD_EXECUTABLE )
		set( GENERATOR_DEFAULT "RPM" )
	endif ( RPMBUILD_EXECUTABLE )

	set( GENERATOR_LIST "DEB" "RPM" "STGZ" "TGZ" "TZ" )
endif()
option_select( CPACK_GENERATOR
	DESCRIPTION "Package generator"
	DEFAULT ${GENERATOR_DEFAULT}
		${GENERATOR_LIST}
)
