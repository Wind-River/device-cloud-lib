/**
 * @file
 * @brief unit testing for IoT library (json encoding support)
 *
 * @copyright Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, either express or implied."
 */

#include "test_support.h"

#include "utilities/app_json.h"

#include <float.h> /* for DBL_MIN */
#include <math.h> /* for fabs */
#include <stdlib.h>
#include <string.h>

#if defined( IOT_JSON_JSONC )
#	include <json-c/json_c_version.h>
#endif /* if defined( IOT_JSON_JSONC ) */

static void test_app_json_encode_array_end_at_root( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_end_in_array( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	for ( i = 0u; i < 10u; ++i )
	{
		result = app_json_encode_integer( e, NULL, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[1,2,3,4,5,6,7,8,9,10]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_end_in_object( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_end_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_array_end( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_array_start_after_non_primative( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_bool( e, NULL, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_start_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_array_start( NULL, "test" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_array_start_key_outside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "test" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":[]}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_start_valid_base_item( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_start_valid_inside_object_blank_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":[]}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_start_valid_inside_object_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "test" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":[]}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_array_start_valid_inside_object_null_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":[]}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_bool_as_root_item( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_bool( e, NULL, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_bool_inside_array_null_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, NULL, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[true]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_bool_inside_array_valid_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "test", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[{\"test\":false}]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_bool_inside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "test", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":false}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_bool_inside_object_blank_key( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;
	const char *json_str;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, NULL, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":true}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_bool_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_bool( NULL, "test", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_bool_outside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_bool( e, "test", IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":true}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_initialize_null( void **state )
{
	app_json_encoder_t *result;
#if !defined( IOT_STACK_ONLY )
	will_return_always( __wrap_os_realloc, 1 );
#endif /* if !defined( IOT_STACK_ONLY ) */
	result = app_json_encode_initialize( NULL, 0u, 0u );
#if defined( IOT_STACK_ONLY )
	assert_null( result );
#else /* if defined( IOT_STACK_ONLY ) */
	assert_non_null( result );
	app_json_encode_terminate( result );
#endif /* else if defined( IOT_STACK_ONLY ) */
}

static void test_app_json_encode_initialize_too_small( void **state )
{
	char buf[1u];
	app_json_encoder_t *result;
	result = app_json_encode_initialize( buf, 1u, 0u );
	assert_null( result );
}

static void test_app_json_encode_initialize_valid( void **state )
{
	char buf[256u];
	app_json_encoder_t *result;
	result = app_json_encode_initialize( buf, 256u, 0u );
	assert_non_null( result );
	app_json_encode_terminate( result );
}


static void test_app_json_encode_dump_null_item( void **state )
{
	const char *result;
	result = app_json_encode_dump( NULL );
	assert_null( result );
}

static void test_app_json_encode_dump_no_items( void **state )
{
	app_json_encoder_t *e;
	const char *result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 100u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, 0u );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_dump( e );
	assert_null( result );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_dump_expand( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 256u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_EXPAND );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_EXPAND );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "array" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < 10u; ++i )
	{
		result = app_json_encode_integer( e, NULL, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}
	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "bool", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "int", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_real( e, "real", 1.0 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_string( e, "string", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );

#if defined( IOT_JSON_JSONC )
	/* json-c: contains extra space after '{' & '}' characters */
#if JSON_C_MAJOR_VERSION < 0 || \
	(JSON_C_MAJOR_VERSION == 0 && JSON_C_MINOR_VERSION < 13) || \
	(JSON_C_MAJOR_VERSION == 0 && JSON_C_MINOR_VERSION == 13) && (JSON_C_MICRO_VERSION < 1)
	assert_string_equal( json_str, "{ \"array\": [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ], \"bool\": false, \"int\": 1, \"obj\": { \"real\": 1, \"string\": \"value\" } }" );
#else /* if json-c < 0.13.1 */
	assert_string_equal( json_str, "{ \"array\": [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ], \"bool\": false, \"int\": 1, \"obj\": { \"real\": 1.0, \"string\": \"value\" } }" );
#endif /* else if json-c < 0.13.1 */
#else /* if defined( IOT_JSON_JSONC ) */
	assert_string_equal( json_str, "{\"array\": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10], \"bool\": false, \"int\": 1, \"obj\": {\"real\": 1.0, \"string\": \"value\"}}" );
#endif /* else if defined( IOT_JSON_JSONC ) */
	app_json_encode_terminate( e );
}

static void test_app_json_encode_dump_indent_0( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 256u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_INDENT(0) );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_INDENT(0) );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "array" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < 10u; ++i )
	{
		result = app_json_encode_integer( e, NULL, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}
	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "bool", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "int", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "negative", -1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_string( e, "string", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );

	assert_string_equal( json_str, "{\"array\":[1,2,3,4,5,6,7,8,9,10],\"bool\":false,\"int\":1,\"obj\":{\"negative\":-1,\"string\":\"value\"}}" );
	app_json_encode_terminate( e );
}

static void test_app_json_encode_dump_indent_1( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 256u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_INDENT(1) );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_INDENT(1) );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "array" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < 10u; ++i )
	{
		result = app_json_encode_integer( e, NULL, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}
	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "bool", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "int", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "negative", -1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_string( e, "string", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );

	assert_string_equal( json_str,
		"{\n"
		" \"array\":[\n"
		"  1,\n"
		"  2,\n"
		"  3,\n"
		"  4,\n"
		"  5,\n"
		"  6,\n"
		"  7,\n"
		"  8,\n"
		"  9,\n"
		"  10\n"
		" ],\n"
		" \"bool\":false,\n"
		" \"int\":1,\n"
		" \"obj\":{\n"
		"  \"negative\":-1,\n"
		"  \"string\":\"value\"\n"
		" }\n"
		"}" );
	app_json_encode_terminate( e );
}

static void test_app_json_encode_dump_indent_2( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 512u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_INDENT(2) );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_INDENT(2) );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "array" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < 10u; ++i )
	{
		result = app_json_encode_integer( e, NULL, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}
	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "bool", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "int", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "negative", -1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_string( e, "string", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );

	assert_string_equal( json_str,
		"{\n"
		"  \"array\":[\n"
		"    1,\n"
		"    2,\n"
		"    3,\n"
		"    4,\n"
		"    5,\n"
		"    6,\n"
		"    7,\n"
		"    8,\n"
		"    9,\n"
		"    10\n"
		"  ],\n"
		"  \"bool\":false,\n"
		"  \"int\":1,\n"
		"  \"obj\":{\n"
		"    \"negative\":-1,\n"
		"    \"string\":\"value\"\n"
		"  }\n"
		"}" );
	app_json_encode_terminate( e );
}

static void test_app_json_encode_dump_indent_5( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 512u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_INDENT(5) );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_INDENT(5) );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "array" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < 10u; ++i )
	{
		result = app_json_encode_integer( e, NULL, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}
	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "bool", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "int", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "negative", -1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_string( e, "string", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );

	assert_string_equal( json_str,
		"{\n"
		"     \"array\":[\n"
		"          1,\n"
		"          2,\n"
		"          3,\n"
		"          4,\n"
		"          5,\n"
		"          6,\n"
		"          7,\n"
		"          8,\n"
		"          9,\n"
		"          10\n"
		"     ],\n"
		"     \"bool\":false,\n"
		"     \"int\":1,\n"
		"     \"obj\":{\n"
		"          \"negative\":-1,\n"
		"          \"string\":\"value\"\n"
		"     }\n"
		"}" );
	app_json_encode_terminate( e );
}

static void test_app_json_encode_dump_indent_expand( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 256u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_EXPAND | APP_JSON_FLAG_INDENT(2) );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u,
		APP_JSON_FLAG_EXPAND | APP_JSON_FLAG_INDENT(2) );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, "array" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < 10u; ++i )
	{
		result = app_json_encode_integer( e, NULL, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}
	result = app_json_encode_array_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_bool( e, "bool", IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "int", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "negative", -1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_string( e, "string", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );

#if defined( IOT_JSON_JSONC )
	/* json-c: has space (' ') before trailing '}' character */
	assert_string_equal( json_str,
		"{\n"
		"   \"array\": [\n"
		"     1,\n"
		"     2,\n"
		"     3,\n"
		"     4,\n"
		"     5,\n"
		"     6,\n"
		"     7,\n"
		"     8,\n"
		"     9,\n"
		"     10\n"
		"   ],\n"
		"   \"bool\": false,\n"
		"   \"int\": 1,\n"
		"   \"obj\": {\n"
		"     \"negative\": -1,\n"
		"     \"string\": \"value\"\n"
		"   }\n"
		" }" );
#else /* if defined( IOT_JSON_JSONC ) */
	assert_string_equal( json_str,
		"{\n"
		"  \"array\": [\n"
		"    1,\n"
		"    2,\n"
		"    3,\n"
		"    4,\n"
		"    5,\n"
		"    6,\n"
		"    7,\n"
		"    8,\n"
		"    9,\n"
		"    10\n"
		"  ],\n"
		"  \"bool\": false,\n"
		"  \"int\": 1,\n"
		"  \"obj\": {\n"
		"    \"negative\": -1,\n"
		"    \"string\": \"value\"\n"
		"  }\n"
		"}" );
#endif /* else if defined( IOT_JSON_JSONC ) */
	app_json_encode_terminate( e );
}

static void test_app_json_encode_integer_as_root_item( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY )  */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_integer( e, NULL, 1234 );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_integer_inside_array_null_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_integer( e, NULL, -1432 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[-1432]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_integer_inside_array_valid_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_integer( e, "test", 0 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[{\"test\":0}]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_integer_inside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_integer( e, "test", 12233 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":12233}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_integer_inside_object_blank_key( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;
	const char *json_str;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_integer( e, NULL, -322342 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":-322342}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_integer_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_integer( NULL, "test", 1234 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_integer_outside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_integer( e, "test", 23423 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":23423}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_cancel_at_root( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_cancel( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_cancel_in_array( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_cancel( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_cancel_in_object( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 256u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY )*/
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_integer( e, "one", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "two", 2 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "three", 3 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	for ( i = 0u; i < 10u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_object_cancel( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_end( e );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str,
		"{\"one\":1,\"two\":2,\"three\":3}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_cancel_in_root_object( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	for ( i = 0u; i < 10u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_object_cancel( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_null( json_str );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_cancel_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_object_cancel( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_object_clear_at_root( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_clear( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_clear_in_array( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_clear( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_clear_in_object( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 256u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_INDENT(2) | APP_JSON_FLAG_EXPAND );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC |
		APP_JSON_FLAG_INDENT(2) | APP_JSON_FLAG_EXPAND );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_integer( e, "one", 1 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "two", 2 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	result = app_json_encode_integer( e, "three", 3 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	for ( i = 0u; i < 10u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_object_clear( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_end( e );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
#if defined( IOT_JSON_JSONC )
	/* json-c: has new-line '\n' inside "obj" structure */
	assert_string_equal( json_str,
	"{\n"
	"   \"one\": 1,\n"
	"   \"two\": 2,\n"
	"   \"three\": 3,\n"
	"   \"obj\": {\n"
	"   }\n"
	" }" );
#else /* if defined( IOT_JSON_JSONC ) */
	assert_string_equal( json_str,
	"{\n"
	"  \"one\": 1,\n"
	"  \"two\": 2,\n"
	"  \"three\": 3,\n"
	"  \"obj\": {}\n"
	"}" );
#endif /* else if defined( IOT_JSON_JSONC ) */
	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_clear_in_object_deep( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, "obj" );
	result = app_json_encode_object_start( e, "obj2" );
	for ( i = 0u; i < 10u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_clear( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_end( e );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"obj\":{}}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_clear_in_root_object( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	for ( i = 0u; i < 10u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_object_clear( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str,
		"{}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_clear_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_object_clear( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_object_clear_then_add( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ),
		APP_JSON_FLAG_INDENT(2) | APP_JSON_FLAG_EXPAND );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u,
		APP_JSON_FLAG_INDENT(2) | APP_JSON_FLAG_EXPAND | APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	for ( i = 0u; i < 5u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_object_clear( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	for ( ; i < 10u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
#if defined( IOT_JSON_JSONC )
	/* json-c: has space (' ') before trailing '}' character */
	assert_string_equal( json_str,
		"{\n"
		"   \"6\": 6,\n"
		"   \"7\": 7,\n"
		"   \"8\": 8,\n"
		"   \"9\": 9,\n"
		"   \"10\": 10\n"
		" }" );
#else
	assert_string_equal( json_str,
		"{\n"
		"  \"6\": 6,\n"
		"  \"7\": 7,\n"
		"  \"8\": 8,\n"
		"  \"9\": 9,\n"
		"  \"10\": 10\n"
		"}" );
#endif
	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_end_at_root( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY )*/
	assert_non_null( e );

	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_end_in_array( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_end_in_object( void **state )
{
	app_json_encoder_t *e;
	unsigned int i;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	for ( i = 0u; i < 10u; ++i )
	{
		char key[5u];
		snprintf( key, 5u, "%u", i + 1u );
		result = app_json_encode_integer( e, key, i + 1u );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
	}

	result = app_json_encode_object_end( e );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str,
		"{\"1\":1,\"2\":2,\"3\":3,\"4\":4,\"5\":5,\"6\":6,\"7\":7,\"8\":8,\"9\":9,\"10\":10}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_end_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_object_end( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_object_start_after_non_primative( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_bool( e, NULL, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_start_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_object_start( NULL, "test" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_object_start_key_outside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, "test" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":{}}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_start_valid_base_item( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_start_valid_inside_object_blank_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, "" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":{}}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_start_valid_inside_object_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, "test" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":{}}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_object_start_valid_inside_object_null_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":{}}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_real_as_root_item( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_real( e, NULL, 1.234 );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_real_inside_array_null_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_real( e, NULL, 1.50 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[1.5]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_real_inside_array_valid_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_integer( e, "test", 0 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[{\"test\":0}]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_real_inside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_real( e, "test1", -0.25 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_real( e, "test2", 1.25 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test1\":-0.25,\"test2\":1.25}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_real_inside_object_blank_key( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;
	const char *json_str;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_real( e, NULL, -2131213.25 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":-2131213.25}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_real_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_real( NULL, "test", 1.2324 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_real_outside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_real( e, "test", 2131231.0 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );

#if defined( IOT_JSON_JSONC ) && (JSON_C_MAJOR_VERSION < 0 || \
	(JSON_C_MAJOR_VERSION == 0 && JSON_C_MINOR_VERSION < 13) || \
	(JSON_C_MAJOR_VERSION == 0 && JSON_C_MINOR_VERSION == 13) && (JSON_C_MICRO_VERSION < 1))
	/* json-c: no trailing ".0" in double number */
	assert_string_equal( json_str, "{\"test\":2131231}" );
#else /* if json-c && json-c version < 1.13.1 */
	assert_string_equal( json_str, "{\"test\":2131231.0}" );
#endif /* else if json-c && json-c version < 1.13.1 */

	app_json_encode_terminate( e );
}

static void test_app_json_encode_string_as_root_item( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_string( e, NULL, "hello" );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_string_escape_chars( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_string( e, "\b\f\n\r\t\"\\", "\b\f\n\r\t\"\\" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str,
		"{\"\\b\\f\\n\\r\\t\\\"\\\\\":\"\\b\\f\\n\\r\\t\\\"\\\\\"}" );
	app_json_encode_terminate( e );
}


static void test_app_json_encode_string_inside_array_null_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_string( e, NULL, "test string" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[\"test string\"]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_string_inside_array_valid_key( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_array_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_string( e, "test", "test string" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "[{\"test\":\"test string\"}]" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_string_inside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_string( e, "test", "this is a test" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":\"this is a test\"}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_string_inside_object_blank_key( void **state )
{
	app_json_encoder_t *e;
	iot_status_t result;
	const char *json_str;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_object_start( e, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	result = app_json_encode_string( e, NULL, "test_string" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"\":\"test_string\"}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_string_null_item( void **state )
{
	iot_status_t result;

	result = app_json_encode_string( NULL, "test", "test_string" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_app_json_encode_string_outside_object( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 128u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_string( e, "test", "a string for test" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"test\":\"a string for test\"}" );

	app_json_encode_terminate( e );
}

static void test_app_json_encode_string_utf8_chars( void **state )
{
	app_json_encoder_t *e;
	const char *json_str;
	iot_status_t result;

#if defined( IOT_STACK_ONLY )
	char buffer[ 1024u ];
	e = app_json_encode_initialize( buffer, sizeof( buffer ), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
#if defined( IOT_JSON_JSONC )
	will_return( __wrap_os_malloc, 1 );
#endif /* if defined( IOT_JSON_JSONC ) */
	will_return_always( __wrap_os_realloc, 1 );
	e = app_json_encode_initialize( NULL, 0u, APP_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
	assert_non_null( e );

	result = app_json_encode_string( e, "Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ",
		"ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįʈ, șếᶑ ᶁⱺ ẽḭŭŝḿꝋď ṫĕᶆᶈṓɍ "
		"ỉñḉīḑȋᵭṵńť ṷŧ ḹẩḇőꝛế éȶ đꝍꞎôꝛȇ ᵯáꞡᶇā ąⱡîɋṹẵ" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	json_str = app_json_encode_dump( e );
	assert_non_null( json_str );
	assert_string_equal( json_str, "{\"Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ\":\""
		"ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįʈ, șếᶑ ᶁⱺ ẽḭŭŝḿꝋď ṫĕᶆᶈṓɍ "
		"ỉñḉīḑȋᵭṵńť ṷŧ ḹẩḇőꝛế éȶ đꝍꞎôꝛȇ ᵯáꞡᶇā ąⱡîɋṹẵ\"}" );
	app_json_encode_terminate( e );
}

/* main */
int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_app_json_encode_array_end_at_root ),
		cmocka_unit_test( test_app_json_encode_array_end_in_array ),
		cmocka_unit_test( test_app_json_encode_array_end_in_object ),
		cmocka_unit_test( test_app_json_encode_array_end_null_item ),
		cmocka_unit_test( test_app_json_encode_array_start_after_non_primative ),
		cmocka_unit_test( test_app_json_encode_array_start_null_item ),
		cmocka_unit_test( test_app_json_encode_array_start_key_outside_object ),
		cmocka_unit_test( test_app_json_encode_array_start_valid_base_item ),
		cmocka_unit_test( test_app_json_encode_array_start_valid_inside_object_blank_key ),
		cmocka_unit_test( test_app_json_encode_array_start_valid_inside_object_key ),
		cmocka_unit_test( test_app_json_encode_array_start_valid_inside_object_null_key ),
		cmocka_unit_test( test_app_json_encode_bool_as_root_item ),
		cmocka_unit_test( test_app_json_encode_bool_inside_array_null_key ),
		cmocka_unit_test( test_app_json_encode_bool_inside_array_valid_key ),
		cmocka_unit_test( test_app_json_encode_bool_inside_object ),
		cmocka_unit_test( test_app_json_encode_bool_inside_object_blank_key ),
		cmocka_unit_test( test_app_json_encode_bool_null_item ),
		cmocka_unit_test( test_app_json_encode_bool_outside_object ),
		cmocka_unit_test( test_app_json_encode_initialize_null ),
		cmocka_unit_test( test_app_json_encode_initialize_too_small ),
		cmocka_unit_test( test_app_json_encode_initialize_valid ),
		cmocka_unit_test( test_app_json_encode_dump_no_items ),
		cmocka_unit_test( test_app_json_encode_dump_null_item ),
		cmocka_unit_test( test_app_json_encode_dump_expand ),
		cmocka_unit_test( test_app_json_encode_dump_indent_0 ),
#if !defined( IOT_JSON_JSONC )
		cmocka_unit_test( test_app_json_encode_dump_indent_1 ),
#endif /* if !defined( IOT_JSON_JSONC ) */
		cmocka_unit_test( test_app_json_encode_dump_indent_2 ),
#if !defined( IOT_JSON_JSONC )
		cmocka_unit_test( test_app_json_encode_dump_indent_5 ),
#endif /* if !defined( IOT_JSON_JSONC ) */
		cmocka_unit_test( test_app_json_encode_dump_indent_expand ),
		cmocka_unit_test( test_app_json_encode_integer_as_root_item ),
		cmocka_unit_test( test_app_json_encode_integer_inside_array_null_key ),
		cmocka_unit_test( test_app_json_encode_integer_inside_array_valid_key ),
		cmocka_unit_test( test_app_json_encode_integer_inside_object ),
		cmocka_unit_test( test_app_json_encode_integer_inside_object_blank_key ),
		cmocka_unit_test( test_app_json_encode_integer_null_item ),
		cmocka_unit_test( test_app_json_encode_integer_outside_object ),
		cmocka_unit_test( test_app_json_encode_object_cancel_at_root ),
		cmocka_unit_test( test_app_json_encode_object_cancel_in_array ),
		cmocka_unit_test( test_app_json_encode_object_cancel_in_object ),
		cmocka_unit_test( test_app_json_encode_object_cancel_in_root_object ),
		cmocka_unit_test( test_app_json_encode_object_cancel_null_item ),
		cmocka_unit_test( test_app_json_encode_object_clear_at_root ),
		cmocka_unit_test( test_app_json_encode_object_clear_in_array ),
		cmocka_unit_test( test_app_json_encode_object_clear_in_object ),
		cmocka_unit_test( test_app_json_encode_object_clear_in_object_deep ),
		cmocka_unit_test( test_app_json_encode_object_clear_in_root_object ),
		cmocka_unit_test( test_app_json_encode_object_clear_null_item ),
		cmocka_unit_test( test_app_json_encode_object_clear_then_add ),
		cmocka_unit_test( test_app_json_encode_object_end_at_root ),
		cmocka_unit_test( test_app_json_encode_object_end_in_array ),
		cmocka_unit_test( test_app_json_encode_object_end_in_object ),
		cmocka_unit_test( test_app_json_encode_object_end_null_item ),
		cmocka_unit_test( test_app_json_encode_object_start_after_non_primative ),
		cmocka_unit_test( test_app_json_encode_object_start_null_item ),
		cmocka_unit_test( test_app_json_encode_object_start_key_outside_object ),
		cmocka_unit_test( test_app_json_encode_object_start_valid_base_item ),
		cmocka_unit_test( test_app_json_encode_object_start_valid_inside_object_blank_key ),
		cmocka_unit_test( test_app_json_encode_object_start_valid_inside_object_key ),
		cmocka_unit_test( test_app_json_encode_object_start_valid_inside_object_null_key ),
		cmocka_unit_test( test_app_json_encode_real_as_root_item ),
		cmocka_unit_test( test_app_json_encode_real_inside_array_null_key ),
		cmocka_unit_test( test_app_json_encode_real_inside_array_valid_key ),
		cmocka_unit_test( test_app_json_encode_real_inside_object ),
		cmocka_unit_test( test_app_json_encode_real_inside_object_blank_key ),
		cmocka_unit_test( test_app_json_encode_real_null_item ),
		cmocka_unit_test( test_app_json_encode_real_outside_object ),
		cmocka_unit_test( test_app_json_encode_string_as_root_item ),
		cmocka_unit_test( test_app_json_encode_string_escape_chars ),
		cmocka_unit_test( test_app_json_encode_string_inside_array_null_key ),
		cmocka_unit_test( test_app_json_encode_string_inside_array_valid_key ),
		cmocka_unit_test( test_app_json_encode_string_inside_object ),
		cmocka_unit_test( test_app_json_encode_string_inside_object_blank_key ),
		cmocka_unit_test( test_app_json_encode_string_null_item ),
		cmocka_unit_test( test_app_json_encode_string_outside_object ),
		cmocka_unit_test( test_app_json_encode_string_utf8_chars )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
