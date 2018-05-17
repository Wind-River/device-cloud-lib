#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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

find_package( Threads )
if ( MQTT_SSL_SUPPORT )
	find_package( OpenSSL )
endif()
find_package( LibArchive REQUIRED )

set( API_HDRS_C "" CACHE INTERNAL "api header files" FORCE )
set( API_SRCS_C "" CACHE INTERNAL "api source files" FORCE )

set( API_HDRS_C ${API_HDRS_C}
	"iot_common.h"
	CACHE INTERNAL "" FORCE
)

set( API_SRCS_C ${API_SRCS_C}
	"iot_action.c"
	"iot_alarm.c"
	"iot_attribute.c"
	"iot_base.c"
	"iot_base64.c"
	"iot_common.c"
	"iot_event.c"
	"iot_file.c"
	"iot_location.c"
	"iot_mqtt.c"
	"iot_option.c"
	"iot_plugin.c"
	"iot_telemetry.c"
	CACHE INTERNAL "" FORCE
)

include_directories( SYSTEM
	"${JSON_INCLUDE_DIR}"
	"${MQTT_INCLUDE_DIR}"
	"${LibArchive_INCLUDE_DIRS}"
)
if( JSON_DEFINES )
	foreach( JSON_DEFINE ${JSON_DEFINES} )
		set( JSON_DEFINES_ "-D${JSON_DEFINE}=1" )
	endforeach( JSON_DEFINE )
	add_definitions( ${JSON_DEFINES_} )
endif( JSON_DEFINES )

add_subdirectory( "checksum" )
add_subdirectory( "json" )
add_subdirectory( "plugin" )
add_subdirectory( "public" )

set( IOT_API_LIBRARY_TYPE "SHARED" )
add_library( "${IOT_LIBRARY_NAME}"
	${IOT_API_LIBRARY_TYPE}
	${API_HDRS_C}
	${API_SRCS_C}
)

target_link_libraries( "${IOT_LIBRARY_NAME}"
	${CMAKE_THREAD_LIBS_INIT}
	${JSON_LIBRARIES}
	${MQTT_LIBRARIES}
	${OSAL_LIBRARIES}
	${PLUGIN_BUILTIN_LIBS}
	${LibArchive_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${CMAKE_DL_LIBS} "m"
)

set_target_properties( "${IOT_LIBRARY_NAME}" PROPERTIES
	SOVERSION ${IOT_VERSION_MAJOR}
	VERSION   ${IOT_VERSION}
)

if ( MQTT_SSL_SUPPORT )
	target_link_libraries( "${IOT_LIBRARY_NAME}" ${OPENSSL_LIBRARIES} )
endif()

# Installation instructions
# LIBRARY DESTINATION (static library for Arduino: iot.lib, iot.a);
#                     (static library for Windows: iot.lib)
#                     (shared library for Non-Windows: iot.so)
# RUNTIME DESTINATION (shared library for Windows: iot.dll)
# ARCHIVE DESTINATION (not used)
set( LIB_COMPONENT "api" )
if ( WIN32 )
	set( LIB_COMPONENT "libs" )
endif( WIN32 )
install( TARGETS ${IOT_LIBRARY_NAME}
	RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT api
	ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT libs
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT ${LIB_COMPONENT}
	PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}" COMPONENT headers
)

# Create directories for configuration and run-time files
install( DIRECTORY
	DESTINATION "${INSTALL_VAR_DIR}"
	COMPONENT api
)

