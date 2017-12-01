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

set( TARGET "tr50" )
set( TARGET_DESCRIPTION "${TARGET} api plugin" )

add_iot_plugin( "${TARGET}" BUILTIN ENABLED
	tr50.c
)

find_package( CURL REQUIRED )
include_directories( SYSTEM
	${CURL_INCLUDE_DIRS} )
target_link_libraries( ${TARGET}
	${CURL_LIBRARIES} )
