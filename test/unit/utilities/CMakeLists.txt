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

set( TARGET "utilities" )
set( TESTS
	"app_arg"
	"app_log"
	"app_path"
	"app_json_encode"
	"app_json_decode"
)

include( "mock_api" )
include( "mock_osal" )

set( TEST_APP_ARG_MOCK ${MOCK_OSAL_FUNC} )
set( TEST_APP_ARG_SRCS ${MOCK_OSAL_SRCS} "app_arg_test.c" )
set( TEST_APP_ARG_LIBS ${MOCK_OSAL_LIBS} )
set( TEST_APP_ARG_UNIT "app_arg.c" )

set( TEST_APP_LOG_MOCK ${MOCK_API_FUNC} ${MOCK_OSAL_FUNC} )
set( TEST_APP_LOG_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "app_log_test.c" )
set( TEST_APP_LOG_LIBS ${MOCK_API_LIBS} ${MOCK_OSAL_LIBS} )
set( TEST_APP_LOG_UNIT "app_log.c" )

set( TEST_APP_PATH_MOCK ${MOCK_OSAL_FUNC} )
set( TEST_APP_PATH_SRCS ${MOCK_OSAL_SRCS} "app_path_test.c" )
set( TEST_APP_PATH_LIBS ${MOCK_OSAL_LIBS} )
set( TEST_APP_PATH_UNIT "app_path.c" )

# app_json_decode.c

#require to set PARENT_LINKS for JSMN
if( JSON_DEFINES )
	foreach( JSON_DEFINE ${JSON_DEFINES} )
		set( JSON_DEFINES_ "-D${JSON_DEFINE}=1" )
	endforeach( JSON_DEFINE )
	add_definitions( ${JSON_DEFINES_} )
endif( JSON_DEFINES )

set( MOCK_API_PART ${MOCK_API_FUNC} )
list( REMOVE_ITEM MOCK_API_PART
	"app_json_decode_array_at"
	"app_json_decode_array_iterator"
	"app_json_decode_array_iterator_next"
	"app_json_decode_array_iterator_value"
	"app_json_decode_array_size"
	"app_json_decode_bool"
	"app_json_decode_initialize"
	"app_json_decode_integer"
	"app_json_decode_number"
	"app_json_decode_object_find"
	"app_json_decode_object_find_len"
	"app_json_decode_object_iterator"
	"app_json_decode_object_iterator_key"
	"app_json_decode_object_iterator_next"
	"app_json_decode_object_iterator_value"
	"app_json_decode_object_size"
	"app_json_decode_parse"
	"app_json_decode_real"
	"app_json_decode_string"
	"app_json_decode_terminate"
	"app_json_decode_type"
)
set( TEST_APP_JSON_DECODE_MOCK ${MOCK_API_PART} ${MOCK_OSAL_FUNC} )
set( TEST_APP_JSON_DECODE_DEFS "${JSON_DEFINES_}" )
set( TEST_APP_JSON_DECODE_INCS "${JSON_INCLUDE_DIR}" )
set( TEST_APP_JSON_DECODE_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "app_json_decode_test.c" )
set( TEST_APP_JSON_DECODE_LIBS ${MOCK_API_LIBS} iotutils ${MOCK_OSAL_LIBS} ${JSON_LIBRARIES} )
set( TEST_APP_JSON_DECODE_UNIT "app_json_decode.c" "app_json_base.c" )

# app_json_encode.c
set( MOCK_API_PART ${MOCK_API_FUNC} )
list( REMOVE_ITEM MOCK_API_PART
	"app_json_encode_array_end"
	"app_json_encode_array_start"
	"app_json_encode_bool"
	"app_json_encode_dump"
	"app_json_encode_initialize"
	"app_json_encode_integer"
	"app_json_encode_object_end"
	"app_json_encode_object_start"
	"app_json_encode_object_cancel"
	"app_json_encode_object_clear"
	"app_json_encode_real"
	"app_json_encode_string"
	"app_json_encode_terminate"
)
set( TEST_APP_JSON_ENCODE_MOCK ${MOCK_API_PART} ${MOCK_OSAL_FUNC} )
set( TEST_APP_JSON_ENCODE_DEFS "${JSON_DEFINES_}" )
set( TEST_APP_JSON_ENCODE_INCS "${JSON_INCLUDE_DIR}" )
set( TEST_APP_JSON_ENCODE_SRCS ${MOCK_API_SRCS} ${MOCK_OSAL_SRCS} "app_json_encode_test.c" )
set( TEST_APP_JSON_ENCODE_LIBS ${MOCK_API_LIBS} ${IOT_UTILITIES} ${MOCK_OSAL_LIBS} ${JSON_LIBRARIES} )
set( TEST_APP_JSON_ENCODE_UNIT "app_json_encode.c" "app_json_base.c" )

include( TestSupport )
add_tests( ${TARGET} ${TESTS} )

