#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#

set( TARGET "${IOT_DEVICE_MANAGER_TARGET}" )
set( TARGET_DESCRIPTION "${IOT_DEVICE_MANAGER_DESCRIPTION}" )

# Header files
set( IOT_HDRS_C ${IOT_HDRS_C}
	"device_manager_main.h"
	"device_manager_file.h"
	"device_manager_md5.h"
	"device_manager_sha256.h"
	"device_manager_ota.h"
)

# Source files
set( IOT_SRCS_C ${IOT_SRCS_C}
	"device_manager.c"
	"device_manager_main.c"
	"device_manager_ota.c"
)

# Resource files
if ( WIN32 )
	set( PROJECT_ICON "${PROJECT_RESOURCE_PATH_WIN32}/iot.ico" )

	configure_file(
		"${PROJECT_RESOURCE_PATH_WIN32}/version.rc.in"
		"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
		@ONLY )
	set( IOT_RESOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/version.rc" )
endif ( WIN32 )

# Local include directories
include_directories(
	"../src/api"
	"../src/utilities"
)

# Executable files
add_executable( ${TARGET}
	${IOT_HDRS_C}
	${IOT_HDRS_CXX}
	${IOT_SRCS_C}
	${IOT_SRCS_CXX}
	${IOT_RESOURCE_FILES}
)

# Required libraries
target_link_libraries( ${TARGET}
	${IOT_LIBRARY_NAME}
	iotutils
)
if ( NOT WIN32 )
	target_link_libraries( ${TARGET} dl )
endif( NOT WIN32 )

# Installation instructions
install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
		COMPONENT core
)

# create directories for file-io
if( NOT WIN32 )
	#install( FILES "${CMAKE_CURRENT_SOURCE_DIR}/iot.cfg.example"
	#	DESTINATION "${INSTALL_CONFIG_DIR}"
	#	COMPONENT core
	#	PERMISSIONS
	#		OWNER_READ OWNER_WRITE
	#		GROUP_READ
	#		WORLD_READ
	#)
endif( NOT WIN32 )

