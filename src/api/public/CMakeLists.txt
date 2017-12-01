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

set( C_HDRS_PUBLIC ${C_HDRS_PUBLIC}
	"iot.h"
)

set( C_HDRS ${C_HDRS}
	${C_HDRS_PUBLIC}
	"iot_checksum.h"
	"iot_json.h"
	"iot_mqtt.h"
	"iot_plugin.h"
)

get_full_path( C_HDRS ${C_HDRS} )
set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} CACHE INTERNAL "" FORCE )

# Doxygen API support
set( DOXYGEN_API_FILES )
foreach( API_HEADER ${C_HDRS_PUBLIC} )
	file( TO_NATIVE_PATH
		"${CMAKE_CURRENT_SOURCE_DIR}/${API_HEADER}" API_PATH )
	set( DOXYGEN_API_FILES "${DOXYGEN_API_FILES} ${API_PATH}" )
endforeach( API_HEADER )
add_doxygen_support( api )

# Installation instructions
install( FILES ${C_HDRS}
	DESTINATION "${INSTALL_INCLUDE_DIR}"
	COMPONENT headers )

