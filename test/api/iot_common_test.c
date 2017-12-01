/**
 * @file
 * @brief unit testing for IoT library (common source file)
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
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

#include "api/iot_common.h"
#include "api/public/iot.h"
#include "api/shared/iot_types.h"
#include "iot_build.h"

#include <string.h>

static iot_status_t iot_common_arg_set_wrapper( struct iot_data *obj,
                                                iot_bool_t heap_alloc,
                                                iot_type_t type,
                                                ... )
{
	iot_status_t result;
	va_list args;
	va_start( args, type );
	result = iot_common_arg_set( obj, heap_alloc, type, args );
	va_end( args );
	return result;
}

static iot_status_t iot_common_arg_get_wrapper( struct iot_data *obj,
                                                iot_bool_t convert,
                                                iot_type_t type,
                                                ... )
{
	iot_status_t result;
	va_list args;
	va_start( args, type );
	result = iot_common_arg_get( obj, convert, type, args );
	va_end( args );
	return result;
}

static void test_iot_common_arg_get_fail_convert( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data = 11;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = 56;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_equal( data, 11 );
}

static void test_iot_common_arg_get_null_obj( void **state )
{
	iot_status_t result;
	iot_bool_t data;

	data = IOT_FALSE;
	result = iot_common_arg_get_wrapper( NULL, IOT_FALSE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_float32_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = 15.2f;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_float32_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = 0.0f;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_float64_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = 153.3;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_float64_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = 0.0;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_int8_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = 0;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_int8_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = 24;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_int16_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = 0;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_int16_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = 2654;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_int32_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = 0;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_int32_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = 542345;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_int64_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = 0;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_int64_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = 42396738;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_raw_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_RAW;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_raw_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_RAW;
	obj.heap_storage = test_malloc( 4u );
	obj.value.raw.ptr = obj.heap_storage;
	obj.value.raw.length = 4u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
	os_free( obj.heap_storage );
}

static void test_iot_common_arg_get_bool_from_string_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;
	const char text[] = "false";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
	os_free( obj.heap_storage );
}

static void test_iot_common_arg_get_bool_from_string_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.value.string = NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
	os_free( obj.heap_storage );
}

static void test_iot_common_arg_get_bool_from_string_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;
	const char text[] = "true";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
	os_free( obj.heap_storage );
}

static void test_iot_common_arg_get_bool_from_uint8_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = 0u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_uint8_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = 36u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_uint16_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = 0u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_uint16_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = 734u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_uint32_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = 0u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}

static void test_iot_common_arg_get_bool_from_uint32_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = 548267u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_bool_from_uint64_false( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = 0u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_FALSE );
}
static void test_iot_common_arg_get_bool_from_uint64_true( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_bool_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = 54327869u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_BOOL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, IOT_TRUE );
}

static void test_iot_common_arg_get_float32_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_float32_t data = 0.0f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = 123.45f;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_FLOAT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_memory_equal( &data, &obj.value.float32, sizeof( iot_float32_t ) );
}

static void test_iot_common_arg_get_float32_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_float32_t data = 0.0f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = 123;
	obj.has_value = IOT_TRUE;
	data = 0.0f;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_FLOAT32, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
}

static void test_iot_common_arg_get_float64_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_float64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = 12345.67;
	obj.has_value = IOT_TRUE;
	data = 0.0;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_FLOAT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_common_arg_get_float64_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_float64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = 12345;
	obj.has_value = IOT_TRUE;
	data = 0.0;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_FLOAT64, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
}

static void test_iot_common_arg_get_int8_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)IOT_TRUE );
}

static void test_iot_common_arg_get_int8_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_float32_t value = 123.4f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_float64_t value = 123.45678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = 56;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 56 );
}

static void test_iot_common_arg_get_int8_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_int16_t value = 296;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_int32_t value = 69258;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_int64_t value = 16734233;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_int8_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	const char text[] = "25";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtol, 25 );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 25 );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_int8_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_uint8_t value = 214u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_uint16_t value = 6925u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_uint32_t value = 2475542u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int8_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int8_t data;
	iot_uint64_t value = 752495024u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int8_t)value );
}

static void test_iot_common_arg_get_int16_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)IOT_TRUE );
}

static void test_iot_common_arg_get_int16_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_float32_t value = 1234.5f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_float64_t value = 1234.5678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_int8_t value = 56;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = -274;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, -274 );
}

static void test_iot_common_arg_get_int16_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_int32_t value = 69258;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_int64_t value = 16734233;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_int16_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	const char text[] = "2526";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtol, 2526 );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 2526 );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_int16_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_uint8_t value = 214u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_uint16_t value = 6925u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_uint32_t value = 2475542u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int16_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int16_t data;
	iot_uint64_t value = 752495024u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int16_t)value );
}

static void test_iot_common_arg_get_int32_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)IOT_TRUE );
}

static void test_iot_common_arg_get_int32_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_float32_t value = 1234.5f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_float64_t value = 1234.5678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_int8_t value = 56;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_int16_t value = 296;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = -624754;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, -624754 );
}

static void test_iot_common_arg_get_int32_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_int64_t value = 16734233;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_int32_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	const char text[] = "252346";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtol, 252346 );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 252346 );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_int32_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_uint8_t value = 214u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_uint16_t value = 6925u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_uint32_t value = 2475542u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int32_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_uint64_t value = 752495024u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int32_t)value );
}

static void test_iot_common_arg_get_int64_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)IOT_TRUE );
}

static void test_iot_common_arg_get_int64_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_float32_t value = 1234.5f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_float64_t value = 1234.5678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_int8_t value = 56;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_int16_t value = 296;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_int32_t value = 69258;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = -6894302;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, -6894302 );
}

static void test_iot_common_arg_get_int64_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_int64_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	const char text[] = "256582427";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtol, 256582427 );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 256582427 );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_int64_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_uint8_t value = 214u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_uint16_t value = 6925u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_uint32_t value = 2475542u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_int64_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;
	iot_uint64_t value = 752495024u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_INT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_int64_t)value );
}

static void test_iot_common_arg_get_location_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_location_t data;
	iot_int32_t value = 11;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_LOCATION, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
}

static void test_iot_common_arg_get_location_from_location( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	const iot_location_t *data = NULL;
	iot_float64_t accuracy = 123.456;
	iot_float64_t altitude = 234.567;
	iot_float64_t altitude_accuracy = 345.678;
	iot_float64_t heading = 456.789;
	iot_float64_t latitude = 567.890;
	iot_float64_t longitude = 678.901;
	iot_uint32_t source = 1;
	iot_float64_t speed = 789.012;
	const char *tag = "somelocation";
	struct iot_location *l;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_LOCATION;
	l = obj.heap_storage = test_malloc( sizeof( iot_location_t ) );
	l->accuracy = accuracy;
	l->altitude = altitude;
	l->altitude_accuracy = altitude_accuracy;
	l->heading = heading;
	l->latitude = latitude;
	l->longitude = longitude;
	l->source = source;
	l->speed = speed;
	l->tag = test_malloc( IOT_NAME_MAX_LEN );
	strncpy(l->tag, tag, IOT_NAME_MAX_LEN );
	obj.value.location = (const iot_location_t *)obj.heap_storage;
	obj.has_value = IOT_TRUE;

	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_LOCATION, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( data );
	assert_memory_equal( &data->accuracy, &accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &data->altitude, &altitude, sizeof( iot_float64_t ) );
	assert_memory_equal( &data->altitude_accuracy, &altitude_accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &data->heading, &heading, sizeof( iot_float64_t ) );
	assert_memory_equal( &data->latitude, &latitude, sizeof( iot_float64_t ) );
	assert_memory_equal( &data->longitude, &longitude, sizeof( iot_float64_t ) );
	assert_int_equal( data->source, source );
	assert_memory_equal( &data->speed, &speed, sizeof( iot_float64_t ) );
	assert_string_equal( data->tag, tag );
	test_free( obj.value.location->tag );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_null_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int32_t data;
	iot_int32_t value = 11;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	data = 0;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_NULL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_null_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_int64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	data = 0;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_NULL, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_raw_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_data_raw data;
	iot_int32_t value = 524;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	memset( &data, 0u, sizeof( struct iot_data_raw ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_RAW, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
}

static void test_iot_common_arg_get_raw_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_data_raw data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	memset( &data, 0u, sizeof( struct iot_data_raw ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_RAW, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_null( data.ptr );
	assert_int_equal( data.length, 0u );
}

static void test_iot_common_arg_get_raw_from_raw( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_data_raw data;
	const char str[] = "this is some text";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	memset( &data, 0u, sizeof( struct iot_data_raw ) );
	obj.type = IOT_TYPE_RAW;
	obj.heap_storage = test_malloc( sizeof( str ) );
	memcpy( obj.heap_storage, str, sizeof( str ) );
	obj.value.raw.ptr = obj.heap_storage;
	obj.value.raw.length = sizeof( str );
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_RAW, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_ptr_equal( data.ptr, obj.value.raw.ptr );
	assert_int_equal( data.length, sizeof( str ) );
	assert_string_equal( (const char *)data.ptr, "this is some text" );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_raw_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_data_raw data;
	const char str[] = "this is some text";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	memset( &data, 0u, sizeof( struct iot_data_raw ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( str ) );
	memcpy( obj.heap_storage, str, sizeof( str ) );
	obj.value.string = obj.heap_storage;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_RAW, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_ptr_equal( data.ptr, obj.value.string );
	assert_int_equal( data.length, sizeof( str ) - 1u );
	assert_string_equal( (const char *)data.ptr, "this is some text" );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_raw_from_string_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_data_raw data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	memset( &data, 0u, sizeof( struct iot_data_raw ) );
	obj.type = IOT_TYPE_STRING;
	obj.value.string = NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_RAW, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_null( data.ptr );
	assert_int_equal( data.length, 0u );
}

static void test_iot_common_arg_get_string_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	char *data;
	iot_int32_t value = 2346;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_STRING, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
}

static void test_iot_common_arg_get_string_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	char *data;
	const char str[] = "this is some text";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( str ) );
	memcpy( obj.heap_storage, str, sizeof( str ) );
	obj.value.string = obj.heap_storage;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_STRING, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_ptr_equal( data, obj.value.string );
	assert_string_equal( data, "this is some text" );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_uint8_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)IOT_TRUE );
}

static void test_iot_common_arg_get_uint8_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_float32_t value = 123.4f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_float64_t value = 123.45678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_int8_t value = 56;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_int16_t value = 296;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_int32_t value = 69258;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_int64_t value = 16734233;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_uint8_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	const char text[] = "25";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtoul, 25u );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 25u );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_uint8_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = 62u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 62u );
}

static void test_iot_common_arg_get_uint8_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_uint16_t value = 6925u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_uint32_t value = 2475542u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint8_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint8_t data;
	iot_uint64_t value = 752495024u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT8, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint8_t)value );
}

static void test_iot_common_arg_get_uint16_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)IOT_TRUE );
}

static void test_iot_common_arg_get_uint16_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_float32_t value = 1234.5f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_float64_t value = 1234.5678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_int8_t value = 56;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_int16_t value = 296;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_int32_t value = 69258;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_int64_t value = 16734233;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_uint16_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	const char *text = "2526";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtoul, 2526u );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 2526u );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_uint16_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_uint8_t value = 214u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = 6302u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 6302u );
}

static void test_iot_common_arg_get_uint16_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_uint32_t value = 2475542u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint16_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint16_t data;
	iot_uint64_t value = 752495024u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT16, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint16_t)value );
}

static void test_iot_common_arg_get_uint32_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)IOT_TRUE );
}

static void test_iot_common_arg_get_uint32_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_float32_t value = 1234.5f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_float64_t value = 1234.5678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_int8_t value = 56;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_int16_t value = 296;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_int32_t value = 69258;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_int64_t value = 16734233;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_uint32_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	const char *text = "252346";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtoul, 252346u );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 252346u );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_uint32_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_uint8_t value = 214u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_uint16_t value = 6925u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint32_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = 683302u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 683302u );
}

static void test_iot_common_arg_get_uint32_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint32_t data;
	iot_uint64_t value = 752495024u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint32_t)value );
}

static void test_iot_common_arg_get_uint64_from_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_BOOL;
	obj.value.boolean = IOT_TRUE;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)IOT_TRUE );
}

static void test_iot_common_arg_get_uint64_from_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_float32_t value = 1234.5f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT32;
	obj.value.float32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_float64_t value = 1234.5678;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_FLOAT64;
	obj.value.float64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_int8_t value = 56;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT8;
	obj.value.int8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_int16_t value = 296;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT16;
	obj.value.int16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_int32_t value = 69258;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT32;
	obj.value.int32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_int64_t value = 16734233;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_INT64;
	obj.value.int64 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_NULL;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 0 );
}

static void test_iot_common_arg_get_uint64_from_string( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	const char *text = "256582427";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_STRING;
	obj.heap_storage = test_malloc( sizeof( text ) );
	obj.value.string = obj.heap_storage;
	memcpy( obj.heap_storage, (const void *)text, sizeof( text ) );
	obj.has_value = IOT_TRUE;
	will_return( __wrap_os_strtoul, 256582427u );
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 256582427u );
	test_free( obj.heap_storage );
}

static void test_iot_common_arg_get_uint64_from_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_uint8_t value = 214u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT8;
	obj.value.uint8 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_uint16_t value = 6925u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT16;
	obj.value.uint16 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;
	iot_uint32_t value = 2475542u;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT32;
	obj.value.uint32 = value;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_TRUE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, (iot_uint64_t)value );
}

static void test_iot_common_arg_get_uint64_from_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_uint64_t data;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_UINT64;
	obj.value.uint64 = 6894302u;
	obj.has_value = IOT_TRUE;
	result = iot_common_arg_get_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT64, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 6894302u );
}

static void test_iot_common_arg_set_bool( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_BOOL, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_BOOL );
	assert_int_equal( obj.value.boolean, IOT_TRUE );
}

static void test_iot_common_arg_set_float32( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_float32_t data = 123.45f;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_FLOAT32, (double)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_FLOAT32 );
	assert_memory_equal( &obj.value.float32, &data, sizeof( iot_float32_t ) );
}

static void test_iot_common_arg_set_float64( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	iot_float64_t data = 12345.6789;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_FLOAT64, data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_FLOAT64 );
	assert_memory_equal( &obj.value.float64, &data, sizeof( iot_float64_t ) );
}

static void test_iot_common_arg_set_int8( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT8, 89 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_INT8 );
	assert_int_equal( obj.value.int8, 89 );
}

static void test_iot_common_arg_set_int16( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT16, -6277 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_INT16 );
	assert_int_equal( obj.value.int16, -6277 );
}

static void test_iot_common_arg_set_int32( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT32, 678267483 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_INT32 );
	assert_int_equal( obj.value.int32, 678267483 );
}

static void test_iot_common_arg_set_int64( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_INT64, -58349278265712554 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_INT64 );
	assert_int_equal( obj.value.int64, -58349278265712554 );
}

static void test_iot_common_arg_set_location( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_location location;
	iot_float64_t accuracy = 123.456;
	iot_float64_t altitude = 234.567;
	iot_float64_t altitude_accuracy = 345.678;
	iot_float64_t heading = 456.789;
	iot_float64_t latitude = 567.890;
	iot_float64_t longitude = 678.901;
	iot_uint32_t source = 1;
	iot_float64_t speed = 789.012;
	const char *tag = "somelocation";

	memset( &location, 0u, sizeof( struct iot_location ) );
	memset( &obj, 0u, sizeof( struct iot_data ) );
	location.accuracy = accuracy;
	location.altitude = altitude;
	location.altitude_accuracy = altitude_accuracy;
	location.heading = heading;
	location.latitude = latitude;
	location.longitude = longitude;
	location.source = source;
	location.speed = speed;
	location.tag = test_malloc( IOT_NAME_MAX_LEN );
	strncpy( location.tag, tag, IOT_NAME_MAX_LEN );
	will_return( __wrap_os_realloc, 1 );
	result = iot_common_arg_set_wrapper( &obj, IOT_TRUE, IOT_TYPE_LOCATION, &location );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_non_null( obj.heap_storage );
	assert_int_equal( obj.type, IOT_TYPE_LOCATION );
	assert_memory_equal( &obj.value.location->accuracy, &accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->altitude, &altitude, sizeof( iot_float64_t ) );
	assert_memory_equal(
	    &obj.value.location->altitude_accuracy, &altitude_accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->heading, &heading, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->latitude, &latitude, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->longitude, &longitude, sizeof( iot_float64_t ) );
	assert_int_equal( obj.value.location->source, source );
	assert_memory_equal( &obj.value.location->speed, &speed, sizeof( iot_float64_t ) );
	assert_string_equal( obj.value.location->tag, tag );
	os_free( obj.heap_storage );
	test_free( location.tag );
}

static void test_iot_common_arg_set_location_no_heap( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_location location;
	iot_float64_t accuracy = 123.456;
	iot_float64_t altitude = 234.567;
	iot_float64_t altitude_accuracy = 345.678;
	iot_float64_t heading = 456.789;
	iot_float64_t latitude = 567.890;
	iot_float64_t longitude = 678.901;
	iot_uint32_t source = 1;
	iot_float64_t speed = 789.012;
	const char *tag = "somelocation";

	memset( &location, 0u, sizeof( struct iot_location ) );
	memset( &obj, 0u, sizeof( struct iot_data ) );
	location.accuracy = accuracy;
	location.altitude = altitude;
	location.altitude_accuracy = altitude_accuracy;
	location.heading = heading;
	location.latitude = latitude;
	location.longitude = longitude;
	location.source = source;
	location.speed = speed;
	location.tag = test_malloc( IOT_NAME_MAX_LEN );
	strncpy( location.tag, tag, IOT_NAME_MAX_LEN );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_LOCATION, &location );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_null( obj.heap_storage );
	assert_ptr_equal( obj.value.location, &location );
	assert_int_equal( obj.type, IOT_TYPE_LOCATION );
	assert_memory_equal( &obj.value.location->accuracy, &accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->altitude, &altitude, sizeof( iot_float64_t ) );
	assert_memory_equal(
	    &obj.value.location->altitude_accuracy, &altitude_accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->heading, &heading, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->latitude, &latitude, sizeof( iot_float64_t ) );
	assert_memory_equal( &obj.value.location->longitude, &longitude, sizeof( iot_float64_t ) );
	assert_int_equal( obj.value.location->source, source );
	assert_memory_equal( &obj.value.location->speed, &speed, sizeof( iot_float64_t ) );
	assert_string_equal( obj.value.location->tag, tag );
	test_free( location.tag );
}

static void test_iot_common_arg_set_no_obj( void **state )
{
	iot_status_t result;

	result = iot_common_arg_set_wrapper( NULL, IOT_FALSE, IOT_TYPE_INT32, 255234 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_common_arg_set_null( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.has_value = IOT_TRUE;
	obj.type = IOT_TYPE_RAW;
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_NULL, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_FALSE );
	assert_int_equal( obj.type, IOT_TYPE_NULL );
	assert_null( obj.value.raw.ptr );
	assert_int_equal( obj.value.raw.length, 0u );
}

static void test_iot_common_arg_set_raw_heap( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_data_raw raw_data;
	const char data[] = "this is some text";

	memset( &raw_data, 0u, sizeof( struct iot_data_raw ) );
	memset( &obj, 0u, sizeof( struct iot_data ) );
	raw_data.ptr = (const void *)data;
	raw_data.length = sizeof( data );
	will_return( __wrap_os_realloc, 1 );
	result = iot_common_arg_set_wrapper( &obj, IOT_TRUE, IOT_TYPE_RAW, &raw_data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_RAW );
	assert_string_equal( (const char *)obj.value.raw.ptr, "this is some text" );
	assert_ptr_not_equal( obj.value.raw.ptr, (const void *)data );
	assert_int_equal( obj.value.raw.length, sizeof( data ) );
	os_free( obj.heap_storage );
}

static void test_iot_common_arg_set_raw_no_heap( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	struct iot_data_raw raw_data;
	const char data[] = "this is some text";

	memset( &raw_data, 0u, sizeof( struct iot_data_raw ) );
	memset( &obj, 0u, sizeof( struct iot_data ) );
	raw_data.ptr = (const void *)data;
	raw_data.length = sizeof( data );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_RAW, &raw_data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_RAW );
	assert_string_equal( (const char *)obj.value.raw.ptr, "this is some text" );
	assert_ptr_equal( obj.value.raw.ptr, (const void *)data );
	assert_int_equal( obj.value.raw.length, sizeof( data ) );
}

static void test_iot_common_arg_set_string_heap( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	const char data[] = "this is some text";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	will_return( __wrap_os_realloc, 1 );
	result = iot_common_arg_set_wrapper( &obj, IOT_TRUE, IOT_TYPE_STRING, data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_STRING );
	assert_string_equal( obj.value.string, "this is some text" );
	assert_ptr_not_equal( obj.value.string, data );
	os_free( obj.heap_storage );
}

static void test_iot_common_arg_set_string_no_heap( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	const char data[] = "this is some text";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_STRING, data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_STRING );
	assert_string_equal( obj.value.string, "this is some text" );
	assert_ptr_equal( obj.value.string, data );
}

static void test_iot_common_arg_set_uint8( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT8, 89u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_UINT8 );
	assert_int_equal( obj.value.uint8, 89u );
}

static void test_iot_common_arg_set_uint16( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT16, 6277u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_UINT16 );
	assert_int_equal( obj.value.uint16, 6277u );
}

static void test_iot_common_arg_set_uint32( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT32, 678267483u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_UINT32 );
	assert_int_equal( obj.value.uint32, 678267483u );
}

static void test_iot_common_arg_set_uint64( void **state )
{
	iot_status_t result;
	struct iot_data obj;

	memset( &obj, 0u, sizeof( struct iot_data ) );
	result = iot_common_arg_set_wrapper( &obj, IOT_FALSE, IOT_TYPE_UINT64, 58349278265712554u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.has_value, IOT_TRUE );
	assert_int_equal( obj.type, IOT_TYPE_UINT64 );
	assert_int_equal( obj.value.uint64, 58349278265712554u );
}

static void test_iot_common_data_copy_bad_parameters( void **state )
{
	iot_status_t result;

	result = iot_common_data_copy( NULL, NULL, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_common_data_copy_dynamic_data_false( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;
	const char data[] = "this is some text";

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_RAW;
	from.has_value = IOT_TRUE;
	from.heap_storage = test_malloc( sizeof( data ) );
	memcpy( from.heap_storage, data, sizeof( data ) );
	from.value.raw.ptr = from.heap_storage;
	from.value.raw.length = sizeof( data );
	result = iot_common_data_copy( &to, &from, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_null( to.heap_storage );
	assert_ptr_equal( to.value.raw.ptr, from.value.raw.ptr );
	assert_int_equal( to.value.raw.length, sizeof( data ) );
	assert_int_equal( to.type, IOT_TYPE_RAW );
	assert_string_equal( (const char *)to.value.raw.ptr, "this is some text" );
	test_free( from.heap_storage );
}

static void test_iot_common_data_copy_int32( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_INT32;
	from.has_value = IOT_TRUE;
	from.value.int32 = -1234;
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_null( to.heap_storage );
	assert_int_equal( to.type, IOT_TYPE_INT32 );
	assert_int_equal( to.value.int32, -1234 );
}

static void test_iot_common_data_copy_location( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;
	iot_float64_t accuracy = 123.456;
	iot_float64_t altitude = 234.567;
	iot_float64_t altitude_accuracy = 345.678;
	iot_float64_t heading = 456.789;
	iot_float64_t latitude = 567.890;
	iot_float64_t longitude = 678.901;
	iot_uint32_t source = 1;
	iot_float64_t speed = 789.012;
	const char *tag = "somelocation";
	struct iot_location *l;

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_LOCATION;
	from.has_value = IOT_TRUE;
	l = from.heap_storage = test_malloc( sizeof( iot_location_t ) );
	l->accuracy = accuracy;
	l->altitude = altitude;
	l->altitude_accuracy = altitude_accuracy;
	l->heading = heading;
	l->latitude = latitude;
	l->longitude = longitude;
	l->source = source;
	l->speed = speed;
	l->tag = test_malloc( IOT_NAME_MAX_LEN + 1u );
	strncpy( l->tag, tag, IOT_NAME_MAX_LEN );
	from.value.location = (const iot_location_t *)from.heap_storage;

	will_return( __wrap_os_malloc, 1 );
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( to.heap_storage );
	assert_ptr_not_equal( to.value.location, from.value.location );
	assert_int_equal( to.type, IOT_TYPE_LOCATION );
	assert_int_equal( to.has_value, IOT_TRUE );
	assert_memory_equal( &to.value.location->accuracy, &accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &to.value.location->altitude, &altitude, sizeof( iot_float64_t ) );
	assert_memory_equal(
	    &to.value.location->altitude_accuracy, &altitude_accuracy, sizeof( iot_float64_t ) );
	assert_memory_equal( &to.value.location->heading, &heading, sizeof( iot_float64_t ) );
	assert_memory_equal( &to.value.location->latitude, &latitude, sizeof( iot_float64_t ) );
	assert_memory_equal( &to.value.location->longitude, &longitude, sizeof( iot_float64_t ) );
	assert_int_equal( to.value.location->source, source );
	assert_memory_equal( &to.value.location->speed, &speed, sizeof( iot_float64_t ) );
	assert_string_equal( to.value.location->tag, tag );

	/* clean up */
	os_free( to.heap_storage );
	test_free( from.value.location->tag );
	test_free( from.heap_storage );
}

static void test_iot_common_data_copy_location_no_memory( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;
	iot_float64_t accuracy = 123.456;
	iot_float64_t altitude = 234.567;
	iot_float64_t altitude_accuracy = 345.678;
	iot_float64_t heading = 456.789;
	iot_float64_t latitude = 567.890;
	iot_float64_t longitude = 678.901;
	iot_uint32_t source = 1;
	iot_float64_t speed = 789.012;
	const char *tag = "somelocation";
	struct iot_location *l;

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_LOCATION;
	from.has_value = IOT_TRUE;
	l = from.heap_storage = test_malloc( sizeof( iot_location_t ) );
	assert_non_null( from.heap_storage );
	l->accuracy = accuracy;
	l->altitude = altitude;
	l->altitude_accuracy = altitude_accuracy;
	l->heading = heading;
	l->latitude = latitude;
	l->longitude = longitude;
	l->source = source;
	l->speed = speed;
	l->tag = test_malloc( IOT_NAME_MAX_LEN + 1u );
	assert_non_null( l->tag );
	strncpy( l->tag, tag, IOT_NAME_MAX_LEN );
	from.value.location = (const iot_location_t *)from.heap_storage;

	will_return( __wrap_os_malloc, 0 );
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_null( to.heap_storage );
	assert_null( to.value.location );
	assert_int_equal( to.type, IOT_TYPE_LOCATION );
	assert_int_equal( to.has_value, IOT_FALSE );

	/* clean up */
	test_free( from.value.location->tag );
	test_free( from.heap_storage );
}

static void test_iot_common_data_copy_raw( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;
	const char data[] = "this is some text";

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_RAW;
	from.has_value = IOT_TRUE;
	from.heap_storage = test_malloc( sizeof( data ) );
	memcpy( from.heap_storage, data, sizeof( data ) );
	from.value.raw.ptr = from.heap_storage;
	from.value.raw.length = sizeof( data );
	will_return( __wrap_os_malloc, 1 );
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( to.heap_storage );
	assert_ptr_not_equal( to.value.raw.ptr, from.value.raw.ptr );
	assert_int_equal( to.value.raw.length, sizeof( data ) );
	assert_int_equal( to.type, IOT_TYPE_RAW );
	assert_int_equal( to.has_value, IOT_TRUE );
	assert_string_equal( (const char *)to.value.raw.ptr, "this is some text" );
	test_free( from.heap_storage );
	os_free( to.heap_storage );
}

static void test_iot_common_data_copy_raw_no_heap( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_RAW;
	from.has_value = IOT_TRUE;
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_null( to.heap_storage );
	assert_int_equal( to.value.raw.length, 0u );
	assert_int_equal( to.type, IOT_TYPE_RAW );
}

static void test_iot_common_data_copy_raw_no_memory( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;
	const char data[] = "this is some text";

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_RAW;
	from.has_value = IOT_TRUE;
	from.heap_storage = test_malloc( sizeof( data ) );
	memcpy( from.heap_storage, data, sizeof( data ) );
	from.value.raw.ptr = from.heap_storage;
	from.value.raw.length = sizeof( data );
	will_return( __wrap_os_malloc, 0 );
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_null( to.heap_storage );
	assert_null( to.value.raw.ptr );
	assert_int_equal( to.value.raw.length, 0u );
	assert_int_equal( to.type, IOT_TYPE_RAW );
	assert_int_equal( to.has_value, IOT_FALSE );
	test_free( from.heap_storage );
}

static void test_iot_common_data_copy_same_pointer_heap( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	const char data[] = "this is some text";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_RAW;
	obj.has_value = IOT_TRUE;
	obj.heap_storage = test_malloc( sizeof( data ) );
	memcpy( obj.heap_storage, data, sizeof( data ) );
	obj.value.raw.ptr = obj.heap_storage;
	obj.value.raw.length = sizeof( data );
	result = iot_common_data_copy( &obj, &obj, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.value.raw.length, sizeof( data ) );
	assert_int_equal( obj.type, IOT_TYPE_RAW );
	assert_string_equal( (const char *)obj.value.raw.ptr, "this is some text" );
	test_free( obj.heap_storage );
}

static void test_iot_common_data_copy_same_pointer_stack( void **state )
{
	iot_status_t result;
	struct iot_data obj;
	const char data[] = "this is some text";

	memset( &obj, 0u, sizeof( struct iot_data ) );
	obj.type = IOT_TYPE_RAW;
	obj.has_value = IOT_TRUE;
	obj.value.raw.ptr = (const void *)data;
	obj.value.raw.length = sizeof( data );
	will_return( __wrap_os_malloc, 1 );
	result = iot_common_data_copy( &obj, &obj, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( obj.value.raw.length, sizeof( data ) );
	assert_int_equal( obj.type, IOT_TYPE_RAW );
	assert_ptr_equal( obj.value.raw.ptr, obj.heap_storage );
	assert_string_equal( (const char *)obj.value.raw.ptr, "this is some text" );
	os_free( obj.heap_storage );
}

static void test_iot_common_data_copy_string( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;
	const char data[] = "this is some text";

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_STRING;
	from.has_value = IOT_TRUE;
	from.heap_storage = test_malloc( sizeof( data ) );
	memcpy( from.heap_storage, data, sizeof( data ) );
	from.value.string = from.heap_storage;
	will_return( __wrap_os_malloc, 1 );
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( to.heap_storage );
	assert_ptr_not_equal( to.value.string, from.value.string );
	assert_int_equal( to.type, IOT_TYPE_STRING );
	assert_int_equal( to.has_value, IOT_TRUE );
	assert_string_equal( to.value.string, "this is some text" );
	test_free( from.heap_storage );
	os_free( to.heap_storage );
}

static void test_iot_common_data_copy_string_no_memory( void **state )
{
	iot_status_t result;
	struct iot_data to;
	struct iot_data from;
	const char data[] = "this is some text";

	memset( &to, 0u, sizeof( struct iot_data ) );
	memset( &from, 0u, sizeof( struct iot_data ) );
	from.type = IOT_TYPE_STRING;
	from.has_value = IOT_TRUE;
	from.heap_storage = test_malloc( sizeof( data ) );
	memcpy( from.heap_storage, data, sizeof( data ) );
	from.value.string = from.heap_storage;
	will_return( __wrap_os_malloc, 0 );
	result = iot_common_data_copy( &to, &from, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_null( to.heap_storage );
	assert_null( to.value.string );
	assert_int_equal( to.has_value, IOT_FALSE );
	assert_int_equal( to.type, IOT_TYPE_STRING );
	test_free( from.heap_storage );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_common_arg_get_fail_convert ),
		cmocka_unit_test( test_iot_common_arg_get_null_obj ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_float32_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_float32_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_float64_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_float64_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int8_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int8_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int16_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int16_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int32_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int32_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int64_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_int64_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_raw_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_raw_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_string_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_string_null ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_string_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint8_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint8_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint16_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint16_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint32_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint32_true ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint64_false ),
		cmocka_unit_test( test_iot_common_arg_get_bool_from_uint64_true ),
		cmocka_unit_test( test_iot_common_arg_get_float32_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_float32_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_float64_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_float64_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_int8_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_int16_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_int32_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_int64_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_get_location_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_location_from_location ),
		cmocka_unit_test( test_iot_common_arg_get_null_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_null_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_raw_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_raw_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_raw_from_raw ),
		cmocka_unit_test( test_iot_common_arg_get_raw_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_raw_from_string_null ),
		cmocka_unit_test( test_iot_common_arg_get_string_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_string_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint8_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint16_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint32_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_bool ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_float32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_float64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_int8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_int16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_int32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_int64 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_null ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_string ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_uint8 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_uint16 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_uint32 ),
		cmocka_unit_test( test_iot_common_arg_get_uint64_from_uint64 ),
		cmocka_unit_test( test_iot_common_arg_set_bool ),
		cmocka_unit_test( test_iot_common_arg_set_float32 ),
		cmocka_unit_test( test_iot_common_arg_set_float64 ),
		cmocka_unit_test( test_iot_common_arg_set_int8 ),
		cmocka_unit_test( test_iot_common_arg_set_int16 ),
		cmocka_unit_test( test_iot_common_arg_set_int32 ),
		cmocka_unit_test( test_iot_common_arg_set_int64 ),
		cmocka_unit_test( test_iot_common_arg_set_location ),
		cmocka_unit_test( test_iot_common_arg_set_location_no_heap ),
		cmocka_unit_test( test_iot_common_arg_set_no_obj ),
		cmocka_unit_test( test_iot_common_arg_set_null ),
		cmocka_unit_test( test_iot_common_arg_set_raw_heap ),
		cmocka_unit_test( test_iot_common_arg_set_raw_no_heap ),
		cmocka_unit_test( test_iot_common_arg_set_string_heap ),
		cmocka_unit_test( test_iot_common_arg_set_string_no_heap ),
		cmocka_unit_test( test_iot_common_arg_set_uint8 ),
		cmocka_unit_test( test_iot_common_arg_set_uint16 ),
		cmocka_unit_test( test_iot_common_arg_set_uint32 ),
		cmocka_unit_test( test_iot_common_arg_set_uint64 ),
		cmocka_unit_test( test_iot_common_data_copy_bad_parameters ),
		cmocka_unit_test( test_iot_common_data_copy_dynamic_data_false ),
		cmocka_unit_test( test_iot_common_data_copy_int32 ),
		cmocka_unit_test( test_iot_common_data_copy_location ),
		cmocka_unit_test( test_iot_common_data_copy_location_no_memory ),
		cmocka_unit_test( test_iot_common_data_copy_raw ),
		cmocka_unit_test( test_iot_common_data_copy_raw_no_heap ),
		cmocka_unit_test( test_iot_common_data_copy_raw_no_memory ),
		cmocka_unit_test( test_iot_common_data_copy_same_pointer_heap ),
		cmocka_unit_test( test_iot_common_data_copy_same_pointer_stack ),
		cmocka_unit_test( test_iot_common_data_copy_string ),
		cmocka_unit_test( test_iot_common_data_copy_string_no_memory )
	};
	MOCK_SYSTEM_ENABLED = 1;
	result = cmocka_run_group_tests( tests, NULL, NULL );
	MOCK_SYSTEM_ENABLED = 0;
	return result;
}
