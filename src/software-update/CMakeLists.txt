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
set( TARGET "${IOT_TARGET_UPDATE}" )

set( IOT_HDRS_C ${IOT_HDRS_C}
	"iot_update_main.h"
)

set( IOT_SRCS_C ${IOT_SRCS_C}
	"iot_update.c"
	"iot_update_main.c"
)

if ( WIN32 )
	set( PROJECT_ICON ${PROJECT_BUILD_WIN_PATH}/iot.ico )

	configure_file(
		${PROJECT_BUILD_WIN_PATH}/version.rc.in
		${CMAKE_CURRENT_BINARY_DIR}/version.rc
		@ONLY )
	list( APPEND IOT_VERSION_FILES ${CMAKE_CURRENT_BINARY_DIR}/version.rc )
endif()

add_executable( ${TARGET}
	${IOT_HDRS_C}
	${IOT_HDRS_CXX}
	${IOT_SRCS_C}
	${IOT_SRCS_CXX}
	${IOT_VERSION_FILES}
)

target_link_libraries( ${TARGET}
	iotutils
	iot
)

if( MSVC )
	# Compile MSVC runtime libraries into the output binary
	target_compile_options( "${TARGET}"
		PRIVATE "/MT$<$<CONFIG:Debug>:d>" )
endif( MSVC )

# Installation instructions
install( TARGETS ${TARGET}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT core
)
