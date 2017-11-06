#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

set( TARGET "${IOT_TARGET_RELAY}" )
set( TARGET_DESCRIPTION "${IOT_RELAY_DESCRIPTION}" )

find_package( LibWebSockets REQUIRED )
find_package( OpenSSL )
find_package( Threads )

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
	${LIBWEBSOCKETS_INCLUDE_DIR}
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
	iot
	${LIBWEBSOCKETS_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${OSAL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	dl
)

# Installation instructions
install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT core
)
