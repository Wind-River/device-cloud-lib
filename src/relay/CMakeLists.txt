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

set( TARGET "${IOT_TARGET_RELAY}" )
set( TARGET_DESCRIPTION "${IOT_RELAY_DESCRIPTION}" )

find_package( OpenSSL )

# Header files
set( IOT_HDRS_C ${IOT_HDRS_C}
	"relay_main.h"
)

# Source files
set( IOT_SRCS_C ${IOT_SRCS_C}
	"relay.c"
	"relay_main.c"
)

# System include directories
include_directories( SYSTEM
	${WEBSOCKET_INCLUDE_DIR}
	${OPENSSL_INCLUDE_DIR}
)

# Local include directories
include_directories(
	"../api"
	"../utilities"
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
	${JSON_LIBRARIES}
	${WEBSOCKET_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${OSAL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS} "m"
)

# Installation instructions
install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT core
)
