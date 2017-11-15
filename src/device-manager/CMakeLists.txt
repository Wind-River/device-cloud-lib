#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "${IOT_DEVICE_MANAGER_TARGET}" )
set( TARGET_DESCRIPTION "${IOT_DEVICE_MANAGER_DESCRIPTION}" )

set( IOT_DEVICE_MANAGER_DEST_COMPONENT "core" )

#if ( WIN32 )
	#set( PROJECT_ICON ${PROJECT_BUILD_WIN_PATH}/iot.ico )

	#configure_file(
		#${PROJECT_BUILD_WIN_PATH}/version.rc.in
		#${CMAKE_CURRENT_BINARY_DIR}/version.rc
		#@ONLY )
	#list( APPEND IOT_VERSION_FILES ${CMAKE_CURRENT_BINARY_DIR}/version.rc )
#endif()

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

# System include directories
include_directories( SYSTEM
	${JSMN_INCLUDE_DIR}
)

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
	${IOT_VERSION_FILES}
)

# Required libraries
target_link_libraries( ${TARGET}
	iotutils
	iot
	${JSMN_LIBRARIES}
	${OSAL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	dl
)

# Installation instructions
install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
		COMPONENT ${IOT_DEVICE_MANAGER_DEST_COMPONENT}
)

# create directories for file-io
if ( NOT WIN32 )
	file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/var/lib/iot" )
	install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/var/lib"
		DESTINATION "${DESTDIR}/var"
		COMPONENT ${IOT_DEVICE_MANAGER_DEST_COMPONENT}
		DIRECTORY_PERMISSIONS
			OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ             GROUP_EXECUTE
			WORLD_READ             WORLD_EXECUTE
		)
	file( MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/etc/iot" )
	configure_file( "${CMAKE_SOURCE_DIR}/src/device-manager/iot.cfg.example"
		"${CMAKE_CURRENT_BINARY_DIR}/etc/iot" COPYONLY )
	install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/etc/iot"
		DESTINATION "${DESTDIR}/etc"
		COMPONENT ${IOT_DEVICE_MANAGER_DEST_COMPONENT}
		DIRECTORY_PERMISSIONS
			OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ             GROUP_EXECUTE
			WORLD_READ             WORLD_EXECUTE
		)
endif ()

