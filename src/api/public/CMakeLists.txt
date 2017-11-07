#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
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

