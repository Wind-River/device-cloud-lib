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

# Set base files
set( APP_BASE_DIR       "${CMAKE_CURRENT_LIST_DIR}" )
set( IOT_MAKE_FILE      "iot.make" )
set( IOT_LIBRARY        "${IOT_LIBRARY_NAME}" )
set( IN_MAKE_FILE       "${APP_BASE_DIR}/Makefile.gcc.in" )
set( IN_MAKE_IOT        "${APP_BASE_DIR}/${IOT_MAKE_FILE}.in" )
set( IN_README          "${APP_BASE_DIR}/README.gcc.md.in" )
if ( MSVC )
	set( IN_MAKE_FILE     "${APP_BASE_DIR}/Makefile.msvc.in" )
	set( IN_README        "${APP_BASE_DIR}/README.msvc.md.in" )
endif( MSVC )

# Build and register a sample applications with the project
#
# This function will generate a sample application and the require source files
# for sample applications
#
# APP_NAME         name of the application
# ...              source files for the application
# TARGET ...       (optional) name of executable file (default: APP_NAME)
# CFLAGS ...       (optional) additional compiler flags for the sample application
# CFLAGS_GCC ...   (optional) gcc specific compiler flags
# CFLAGS_MSVC ...  (optional) microsoft visual studio compiler flags
# C_SRCS ...       (optional) C source files
# DEPENDS ...      (optional) other sample apps this one depends on
# DESCRIPTION ...  (optional) description of the example application
# DISPLAY_NAME ... (optional) name of the application is within installer
# LDFLAGS ...      (optional) additional linker flags for the sample application
# LDFLAGS_GCC ...  (optional) gcc specific linker flags
# LDFLAGS_MSVC ... (optional) microsoft visual studio linker flags
# PY_SRCS ...      (optional) Python source files
# README ...       (optional) readme file for the application
# SCRIPTS ...      (optional) script files for the application
# SCRIPTS_UNIX  ...(optional) unix script files for the application
# SCRIPTS_WIN32 ...(optional) windows scripts for the application
# SRCS ...         (default) sourced files for the application
macro( ADD_SAMPLE_APP APP_NAME )
	cmake_parse_arguments( APP "" "DESCRIPTION;DISPLAY_NAME;TARGET"
		"CFLAGS;CFLAGS_GCC;CFLAGS_MSVC;C_SRCS;DEPENDS;LDFLAGS;LDFLAGS_GCC;LDFLAGS_MSVC;PY_SRCS;README;SCRIPTS;SCRIPTS_UNIX;SCRIPTS_WIN32;SRCS"
		${ARGN} )
	string( REPLACE " " "_" _APP_NAME "${APP_NAME}" )
	set( APP_COMPONENT "${_APP_NAME}" )
	# Component names can only be [a-z][A-Z][0-9] or '_'
	string( REGEX REPLACE "[^a-zA-Z0-9_]" "_" APP_COMPONENT "${APP_COMPONENT}" )
	string( TOUPPER "${APP_COMPONENT}" APP_COMPONENT_UPPER )
	set( OUT_DIR           "${CMAKE_CURRENT_BINARY_DIR}/out" )
	set( MAKE_CMD          "$(MAKE)" )
	set( MAKE_FLAGS        "CC=\"${CMAKE_C_COMPILER}\""
		"CFLAGS=${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}"
		"LDFLAGS=${CMAKE_EXE_LINKER_FLAGS}" )
	# Set target variables
	if ( NOT APP_TARGET )
		set(  APP_TARGET "${_APP_NAME}" )
	endif ( NOT APP_TARGET )
	set( TARGET "${APP_TARGET}${CMAKE_EXECUTABLE_SUFFIX}" )
	set( APP_SRCS ${APP_SRCS} ${APP_UNPARSED_ARGUMENTS} )
	foreach( SRC_FILE ${APP_SRCS} )
		get_filename_component( FILE_EXT "${SRC_FILE}" EXT )
		if ( "${FILE_EXT}" STREQUAL ".c" )
			set( APP_C_SRCS ${APP_C_SRCS} "${SRC_FILE}" )
		elseif ( "${FILE_EXT}" STREQUAL ".py" )
			set( APP_PY_SRCS ${APP_PY_SRCS} "${SRC_FILE}" )
		endif ( "${FILE_EXT}" STREQUAL ".c" )
	endforeach( SRC_FILE )
	set( OUT_SRCS )
	foreach ( SRC_FILE ${APP_C_SRCS} )
		add_custom_command( OUTPUT "${OUT_DIR}/${SRC_FILE}"
			COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different "${SRC_FILE}" "${OUT_DIR}/${SRC_FILE}"
			DEPENDS ${SRC_FILE}
			COMMENT "Copying ${SRC_FILE} for app ${APP_NAME}..."
			WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
		)
		set( OUT_SRCS "out/${SRC_FILE}" )
	endforeach( SRC_FILE )
	if ( APP_C_SRCS )
		string( REPLACE ";" " " SRCS ${APP_C_SRCS} )
	endif( APP_C_SRCS )

	set( CMD_PREFIX )
	if ( WIN32 )
		set( CMD_PREFIX "call" )
		set( APP_SCRIPTS ${APP_SCRIPTS} ${APP_SCRIPTS_WIN32} )
	else()
		set( APP_SCRIPTS ${APP_SCRIPTS} ${APP_SCRIPTS_UNIX} )
	endif( WIN32 )

	# Set output directories based on compiler
	set( CONFIG_DIR "" )
	if( NOT CMAKE_CFG_INTDIR STREQUAL "." )
		set( CONFIG_DIR "${CMAKE_CFG_INTDIR}" )
	endif( NOT CMAKE_CFG_INTDIR STREQUAL "." )
	set( BIN_OUT_DIR       "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG_DIR}" )
	set( IOT_LIB_DIR       "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIG_DIR}" )
	set( IOT_INCLUDE_DIR   "${CMAKE_INCLUDE_OUTPUT_DIRECTORY}" )
	if ( IOT_API_STATIC )
		set( APP_CFLAGS "-DIOT_STATIC" )
	endif ( IOT_API_STATIC )
	if ( MSVC )
		set( APP_CFLAGS       ${APP_CFLAGS} ${APP_CFLAGS_MSVC} )
		set( APP_LDFLAGS      ${APP_LDFLAGS} ${APP_LDFLAGS_MSVC} )
		set( MAKE_FLAGS       ${MAKE_FLAGS} "Configuration=${CMAKE_CFG_INTDIR}" )
		set( MAKE_CMD         "nmake" )
	else()
		set( APP_CFLAGS       ${APP_CFLAGS} ${APP_CFLAGS_GCC} )
		set( APP_LDFLAGS      ${APP_LDFLAGS} ${APP_LDFLAGS_GCC} )
		if( NOT CMAKE_SKIP_BUILD_RPATH )
			set( APP_LDFLAGS      ${APP_LDFLAGS} "-Wl,-rpath,${IOT_LIB_DIR}" )
		endif()
	endif( MSVC )
	string( REPLACE ";" " " CFLAGS "${APP_CFLAGS}" )
	string( REPLACE ";" " " LDFLAGS "${APP_LDFLAGS}" )

	get_directory_property( MAKE_CLEAN_FILES ADDITIONAL_MAKE_CLEAN_FILES )
	set( OUT_MAKE_FILE )
	set( OUT_TARGETS )
	if ( APP_C_SRCS )
		# Generate a make file
		configure_file( "${IN_MAKE_IOT}" "${CMAKE_CURRENT_BINARY_DIR}/${IOT_MAKE_FILE}" @ONLY )
		set( OUT_MAKE_FILE "${OUT_DIR}/Makefile" )
		configure_file( "${IN_MAKE_FILE}" "${OUT_MAKE_FILE}" )

		# Run make file (this tests our sample build)
		add_custom_target( "${_APP_NAME}" ALL
			COMMAND ${CMD_PREFIX} "${MAKE_CMD}" ${MAKE_FLAGS}
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${BIN_OUT_DIR}"
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${TARGET}" "${BIN_OUT_DIR}"
			SOURCES ${APP_C_SRCS} ${APP_SCRIPTS}
			DEPENDS iot ${OUT_SRCS}
			WORKING_DIRECTORY "${OUT_DIR}"
			COMMENT "Building app ${APP_NAME}..."
		)
		set( OUT_TARGETS "${OUT_DIR}/${TARGET}" )
	endif ( APP_C_SRCS )

	if ( APP_PY_SRCS )
		# Run make file (this tests our sample build)
		add_custom_target( "${_APP_NAME}" ALL
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${APP_PY_SRCS}" "${OUT_DIR}/${APP_TARGET}.py"
			SOURCES ${APP_PY_SRCS} ${APP_SCRIPTS}
			COMMENT "Building app ${APP_NAME}..."
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		)
		set( OUT_TARGETS "${OUT_DIR}/${APP_TARGET}.py" )
	endif( APP_PY_SRCS )

	set( MAKE_CLEAN_FILES ${OUT_SRCS} ${OUT_TARGETS} )
	if ( MAKE_CLEAN_FILES )
		set_directory_properties( PROPERTIES
			ADDITIONAL_MAKE_CLEAN_FILES "${MAKE_CLEAN_FILES}" )
	endif( MAKE_CLEAN_FILES )

	# Set optional attributes for the sample application component
	seT( IOT_APPS ${IOT_APPS} "${APP_COMPONENT}" )
	set( IOT_APPS ${IOT_APPS} PARENT_SCOPE )
	if ( APP_DEPENDS )
		set( IOT_APP_${APP_COMPONENT_UPPER}_DEPENDS "${APP_DEPENDS}" CACHE
			INTERNAL "Dependencies for ${APP_NAME}" )
	endif ( APP_DEPENDS )
	if ( APP_DESCRIPTION )
		set( IOT_APP_${APP_COMPONENT_UPPER}_DESCRIPTION "${APP_DESCRIPTION}" CACHE
			INTERNAL "Description for ${APP_NAME}" FORCE )
	endif ( APP_DESCRIPTION )
	if ( APP_DISPLAY_NAME )
		set( IOT_APP_${APP_COMPONENT_UPPER}_DISPLAY_NAME "${APP_DISPLAY_NAME}" CACHE
			INTERNAL "Name to use for ${APP_NAME}" FORCE )
	endif ( APP_DISPLAY_NAME )

	# Add application source files
	set( DEST_TEXT_FILES "${OUT_MAKE_FILE}" ${APP_README} ${APP_C_SRCS} )
	if ( DEST_TEXT_FILES )
		install( FILES ${DEST_TEXT_FILES}
			DESTINATION "${INSTALL_EXAMPLES_DIR}/${_APP_NAME}"
			COMPONENT "${APP_COMPONENT}"
		)
	endif( DEST_TEXT_FILES )

	# Add any required script files to sources
	set( DEST_SCRIPT_FILES ${APP_PY_SRCS} ${APP_SCRIPTS} )
	if ( DEST_SCRIPT_FILES )
		install( PROGRAMS ${DEST_SCRIPT_FILES}
			DESTINATION "${INSTALL_EXAMPLES_DIR}/${_APP_NAME}"
			COMPONENT "${APP_COMPONENT}"
		)
	endif( DEST_SCRIPT_FILES )

	# Add compiled (output) sample application to installation
	if ( OUT_TARGETS OR APP_SCRIPTS )
		install( PROGRAMS ${OUT_TARGETS} ${APP_SCRIPTS}
			DESTINATION "${INSTALL_BIN_DIR}"
			COMPONENT "${APP_COMPONENT}"
		)
		# Add example apps & script to the start menu folder
		foreach( TARGET_FILE ${OUT_TARGETS} )
			get_filename_component( TARGET_FILE "${TARGET_FILE}" NAME )
			#install_shortcut(
			#	FILES "${INSTALL_BIN_DIR}/${TARGET_FILE}"
			#	DESTINATION "examples"
			#)
		endforeach( TARGET_FILE )
	endif ( OUT_TARGETS OR APP_SCRIPTS )
endmacro( ADD_SAMPLE_APP )

# Subdirectories containing example applications
add_subdirectory( "actions" )
add_subdirectory( "complete" )
add_subdirectory( "location" )
add_subdirectory( "telemetry" )
add_subdirectory( "validate" )

# Files required by all applications for building from source
# Add base iot.make file with correct path for install
set( IOT_INCLUDE_DIR "${INSTALL_INCLUDE_DIR}" )
if ( NOT IS_ABSOLUTE "${INSTALL_INCLUDE_DIR}" )
	set( IOT_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_INCLUDE_DIR}" )
endif ( NOT IS_ABSOLUTE "${INSTALL_INCLUDE_DIR}" )
set( IOT_LIB_DIR "${INSTALL_LIB_DIR}" )
if ( NOT IS_ABSOLUTE "${INSTALL_LIB_DIR}" )
	set( IOT_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${INSTALL_LIB_DIR}" )
endif ( NOT IS_ABSOLUTE "${INSTALL_LIB_DIR}" )
file( TO_NATIVE_PATH "${IOT_INCLUDE_DIR}" IOT_INCLUDE_DIR )
file( TO_NATIVE_PATH "${IOT_LIB_DIR}" IOT_LIB_DIR )
configure_file( "${IN_MAKE_IOT}" "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${IOT_MAKE_FILE}" @ONLY )
configure_file( "${IN_README}" "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/README.md" @ONLY )
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${IOT_MAKE_FILE}"
		"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/README.md"
	DESTINATION "${INSTALL_EXAMPLES_DIR}"
	COMPONENT source
)

# Update the path to the install header files and libraries within the base
# Makefile for building example applications, this is nessecessary if the
# installation directory different from the default directory
# (CMAKE_INSTALL_PREFIX)
#file( TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}" NATIVE_INSTALL_PREFIX )
#install_replace_in_file( "${INSTALL_EXAMPLES_DIR}/${PROJECT_NAME_SHORT}.make" "${NATIVE_INSTALL_PREFIX}" "${CMAKE_INSTALL_PREFIX}" "source" )

# Update the list of example applications to the parent CMakeLists.txt file
# (this is used for packaging)
#set( IOT_APPS ${IOT_APPS} PARENT_SCOPE )
#install_set_variables( PARENT_SCOPE )

