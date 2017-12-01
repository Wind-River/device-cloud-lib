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

set( C_HDRS ${C_HDRS}
	"iot_json_base.h"
	"iot_json_schema.h"
)

set( C_SRCS ${C_SRCS}
	"iot_json_base.c"
	"iot_json_decode.c"
	"iot_json_encode.c"
	"iot_json_schema.c"
)

get_full_path( C_HDRS ${C_HDRS} )
set( API_HDRS_C ${API_HDRS_C} ${C_HDRS} CACHE INTERNAL "" FORCE )

get_full_path( C_SRCS ${C_SRCS} )
set( API_SRCS_C ${API_SRCS_C} ${C_SRCS} CACHE INTERNAL "" FORCE )
