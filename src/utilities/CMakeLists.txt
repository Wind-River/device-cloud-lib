#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
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

set( TARGET "iotutils" )

set( IOT_HDRS_C ${IOT_HDRS_C}
	"app_arg.h"
	"app_config.h"
	"app_json.h"
	"app_json_base.h"
	"app_json_schema.h"
	"app_log.h"
	"app_path.h"
)

set( IOT_SRCS_C ${IOT_SRCS_C}
	"app_arg.c"
	"app_config.c"
	"app_json_base.c"
	"app_json_decode.c"
	"app_json_encode.c"
	"app_json_schema.c"
	"app_log.c"
	"app_path.c"
)

add_definitions( ${IOT_DEFS} )

if( JSON_DEFINES )
	foreach( JSON_DEFINE ${JSON_DEFINES} )
		set( JSON_DEFINES_ "${JSON_DEFINES_} -D${JSON_DEFINE}=1" )
	endforeach( JSON_DEFINE )
	add_definitions( ${JSON_DEFINES_} )
endif( JSON_DEFINES )

option( IOT_LOG_TIMESTAMP "Option to add time stamps in the log" OFF )
if( IOT_LOG_TIMESTAMP )
	add_definitions( "-DIOT_LOG_TIMESTAMP" )
endif( IOT_LOG_TIMESTAMP )

add_library( ${TARGET} STATIC
	${IOT_HDRS_C}
	${IOT_HDRS_CXX}
	${IOT_SRCS_C}
	${IOT_SRCS_CXX}
)

if( MSVC )
	# Compile MSVC runtime libraries into the output binary
	target_compile_options( "${TARGET}"
		PRIVATE "/MT$<$<CONFIG:Debug>:d>" )
endif( MSVC )

include_directories( SYSTEM
	${JSMN_INCLUDE_DIR}
	${IOT_INCS}
	"${CMAKE_SOURCE_DIR}/src/api"
	"${CMAKE_SOURCE_DIR}/src/app-common"
)

