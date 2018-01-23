/**
 * @file
 * @brief unit testing for IoT library (json decoding support)
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

#include "api/public/iot.h"
#include "api/public/iot_json.h"

#include <float.h> /* for DBL_MIN */
#include <math.h> /* for fabs */
#include <stdlib.h>
#include <string.h>

static void test_iot_json_decode_array_at_invalid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *arr;
	const iot_json_item_t *obj;
	const iot_json_item_t *root = NULL;

	snprintf( json, 256u, "{\"array\":{\"not\":\"an_array\"}}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	arr = iot_json_decode_object_find( decoder, root, "array" );
	assert_non_null( arr );

	result = iot_json_decode_array_at( decoder, arr, 0u, &obj );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_null( obj );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_at_null_array( void **state )
{
	iot_json_decoder_t *decoder = (iot_json_decoder_t*)0x1;
	const iot_json_item_t *obj = (const iot_json_item_t*)0x2;
	iot_status_t result;

	result = iot_json_decode_array_at( decoder, NULL, 0u, &obj );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( obj );
}

static void test_iot_json_decode_array_at_null_item( void **state )
{
	const iot_json_item_t *arr = (const iot_json_item_t*)0x1;
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x2;
	iot_status_t result;

	result = iot_json_decode_array_at( json, arr, 0u, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_json_decode_array_at_null_json( void **state )
{
	const iot_json_item_t *arr = (const iot_json_item_t*)0x1;
	const iot_json_item_t *item = (const iot_json_item_t*)0x2;
	iot_status_t result;

	result = iot_json_decode_array_at( NULL, arr, 0u, &item );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( item );
}


static void test_iot_json_decode_array_at_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *arr;
	const iot_json_item_t *obj;
	const iot_json_item_t *root = NULL;
	iot_int64_t value;

	snprintf( json, 256u, "{\"array\":[1,2,3,5,8,13,21,34,55,89,144]}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	arr = iot_json_decode_object_find( decoder, root, "array" );
	assert_non_null( arr );

	/* first item */
	result = iot_json_decode_array_at( decoder, arr, 0u, &obj );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( obj );
	iot_json_decode_integer( decoder, obj, &value );
	assert_int_equal( value, 1 );

	/* middle item */
	result = iot_json_decode_array_at( decoder, arr, 5u, &obj );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( obj );
	iot_json_decode_integer( decoder, obj, &value );
	assert_int_equal( value, 13 );

	/* last item */
	result = iot_json_decode_array_at( decoder, arr, 10u, &obj );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( obj );
	iot_json_decode_integer( decoder, obj, &value );
	assert_int_equal( value, 144 );

	/* not found */
	result = iot_json_decode_array_at( decoder, arr, 11u, &obj );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_null( obj );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_null_item( void **state )
{
	const iot_json_object_iterator_t *i;
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x1;
	i = iot_json_decode_array_iterator( json, NULL );
	assert_null( i );
}

static void test_iot_json_decode_array_iterator_null_json( void **state )
{
	const iot_json_object_iterator_t *i;
	const iot_json_item_t *item = (const iot_json_item_t*)0x1;
	i = iot_json_decode_array_iterator( NULL, item );
	assert_null( i );
}


static void test_iot_json_decode_array_iterator_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( i );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_next_null_item( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( i );

	i = iot_json_decode_array_iterator_next( decoder, NULL, i );
	assert_null( i );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_next_null_iterator( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( i );

	i = iot_json_decode_array_iterator_next( decoder, root, NULL );
	assert_null( i );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_next_null_json( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( i );

	i = iot_json_decode_array_iterator_next( NULL, root, i );
	assert_null( i );

	iot_json_decode_terminate( decoder );
}


static void test_iot_json_decode_array_iterator_next_null_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;
	size_t i, size;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( iter );

	size = iot_json_decode_array_size( decoder, root );
	assert_true( size > 0 );

	for ( i = 0u; i < size; ++i )
	{
		iter = iot_json_decode_array_iterator_next( decoder, root, iter );
		if ( i == size - 1u )
			assert_null( iter );
		else
			assert_non_null( iter );
	}

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_value_null_item( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;
	const iot_json_item_t *item = (const iot_json_item_t *)0x1;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_array_iterator_value( decoder, NULL, iter, &item );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( item );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_value_null_iterator( void **state )
{
	char buf[256u];
	iot_json_decoder_t *decoder;
	const iot_json_item_t *arr = (const iot_json_item_t *)0x1;
	const iot_json_item_t *item = (const iot_json_item_t *)0x2;
	iot_status_t result;

	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );

	result = iot_json_decode_array_iterator_value( decoder, arr, NULL, &item );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( item );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_value_null_json( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *obj = NULL;
	const iot_json_object_iterator_t *iter;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_array_iterator_value( NULL, root, iter, &obj );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( obj );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_iterator_value_null_out( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_array_iterator_value( decoder, root, iter, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );

	iot_json_decode_terminate( decoder );
}


static void test_iot_json_decode_array_iterator_value_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *obj = NULL;
	const iot_json_object_iterator_t *iter;

	snprintf( json, 256u, "[ \"item1\", \"item2\", \"item3\", \"item4\" ]" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_array_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_array_iterator_value( decoder, root, iter, &obj );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( obj );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_array_size_null_decoder( void **state )
{
	const iot_json_item_t *item = (const iot_json_item_t *)0x1;
	size_t size;
	size = iot_json_decode_array_size( NULL, item );
	assert_int_equal( size, 0u );
}

static void test_iot_json_decode_array_size_null_item( void **state )
{
	iot_json_decoder_t *decoder = (iot_json_decoder_t *)0x1;
	size_t size;
	size = iot_json_decode_array_size( decoder, NULL );
	assert_int_equal( size, 0u );
}

static void test_iot_json_decode_array_size_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *obj;
	const iot_json_item_t *root = NULL;
	size_t size;

	snprintf( json, 256u, "{\"array0\":[],\"array1\":[1],\"array2\":[1,2,3,4]}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	/* array0 */
	obj = iot_json_decode_object_find( decoder, root, "array0" );
	assert_non_null( obj );
	size = iot_json_decode_array_size( decoder, obj );
	assert_int_equal( size, 0u );

	/* array1 */
	obj = iot_json_decode_object_find( decoder, root, "array1" );
	assert_non_null( obj );
	size = iot_json_decode_array_size( decoder, obj );
	assert_int_equal( size, 1u );

	/* array2 */
	obj = iot_json_decode_object_find( decoder, root, "array2" );
	assert_non_null( obj );
	size = iot_json_decode_array_size( decoder, obj );
	assert_int_equal( size, 4u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_bool_null_item( void **state )
{
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x1;
	iot_status_t result;
	iot_bool_t value;

	result = iot_json_decode_bool( json, NULL, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( value, IOT_FALSE );
}

static void test_iot_json_decode_bool_null_json( void **state )
{
	const iot_json_item_t *item = (const iot_json_item_t*)0x1;
	iot_status_t result;
	iot_bool_t value;

	result = iot_json_decode_bool( NULL, item, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( value, IOT_FALSE );
}


static void test_iot_json_decode_bool_valid( void **state )
{
	struct result_value_map {
		const char *key;
		iot_status_t status;
		iot_bool_t value;
	};
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	struct result_value_map results[] = {
		{ "bool1", IOT_STATUS_SUCCESS, IOT_TRUE },
		{ "bool2", IOT_STATUS_SUCCESS, IOT_FALSE },
		{ "int", IOT_STATUS_BAD_REQUEST, IOT_FALSE },
		{ NULL, IOT_STATUS_SUCCESS, IOT_FALSE }
	};
	const struct result_value_map *rv = &results[0u];

	snprintf( json, 256u, "{\"bool1\":true,\"bool2\":false,\"int\":0}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	while ( rv->key )
	{
		const iot_json_item_t *obj;
		iot_status_t status;
		iot_json_type_t type;
		iot_bool_t value;
		obj = iot_json_decode_object_find( decoder, root, rv->key );
		assert_non_null( obj );
		status = iot_json_decode_bool( decoder, obj, &value );
		assert_int_equal( status, rv->status );
		assert_int_equal( value, rv->value );

		type = iot_json_decode_type( decoder, obj );
		if ( rv->status == IOT_STATUS_SUCCESS )
			assert_int_equal( type, IOT_JSON_TYPE_BOOL );
		else
			assert_int_not_equal( type, IOT_JSON_TYPE_BOOL );
		++rv;
	}
	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_initialize_null( void **state )
{
	iot_json_decoder_t *result;
#ifndef IOT_STACK_ONLY
	will_return_always( __wrap_os_realloc, 1 );
#endif
	result = iot_json_decode_initialize( NULL, 0u, 0u );
#ifdef IOT_STACK_ONLY
	assert_null( result );
#else
	assert_non_null( result );
	iot_json_decode_terminate( result );
#endif
}

static void test_iot_json_decode_initialize_too_small( void **state )
{
	char buf[1u];
	iot_json_decoder_t *result;
	result = iot_json_decode_initialize( buf, 1u, 0u );
	assert_null( result );
}

static void test_iot_json_decode_initialize_valid( void **state )
{
	char buf[256u];
	iot_json_decoder_t *result;
	result = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( result );
	iot_json_decode_terminate( result );
}

static void test_iot_json_decode_integer_null_item( void **state )
{
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x1;
	iot_status_t result;
	iot_int64_t value;

	result = iot_json_decode_integer( json, NULL, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( value, 0u );
}

static void test_iot_json_decode_integer_null_json( void **state )
{
	const iot_json_item_t* item = (const iot_json_item_t*)0x1;
	iot_status_t result;
	iot_int64_t value;

	result = iot_json_decode_integer( NULL, item, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( value, 0u );
}

static void test_iot_json_decode_integer_valid( void **state )
{
	struct result_value_map {
		const char *key;
		iot_status_t status;
		iot_int64_t value;
	};
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	struct result_value_map results[] = {
		{ "int1", IOT_STATUS_SUCCESS, 0 },
		{ "int2", IOT_STATUS_SUCCESS, 123456789 },
		{ "int3", IOT_STATUS_SUCCESS, -123456789 },
		{ "real1", IOT_STATUS_BAD_REQUEST, 0 },
		{ "real2", IOT_STATUS_BAD_REQUEST, 0 },
		{ NULL, IOT_STATUS_SUCCESS, IOT_FALSE }
	};
	const struct result_value_map *rv = &results[0u];

	snprintf( json, 256u, "{"
		"\"int1\":0,"
		"\"int2\":123456789,"
		"\"int3\":-123456789,"
		"\"real1\":1.234,"
		"\"real2\":3.242344242e+09"
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	while ( rv->key )
	{
		const iot_json_item_t *obj;
		iot_status_t status;
		iot_json_type_t type;
		iot_int64_t value;
		obj = iot_json_decode_object_find( decoder, root, rv->key );
		assert_non_null( obj );
		status = iot_json_decode_integer( decoder, obj, &value );
		assert_int_equal( status, rv->status );
		assert_int_equal( value, rv->value );

		type = iot_json_decode_type( decoder, obj );
		if ( rv->status == IOT_STATUS_SUCCESS )
			assert_int_equal( type, IOT_JSON_TYPE_INTEGER );
		else
			assert_int_not_equal( type, IOT_JSON_TYPE_INTEGER );
		++rv;
	}
	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_number_null_item( void **state )
{
	const iot_json_decoder_t *json = (const iot_json_decoder_t*)0x1;
	iot_status_t result;
	iot_float64_t value;

	result = iot_json_decode_number( json, NULL, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_json_decode_number_null_json( void **state )
{
	const iot_json_item_t *item = (const iot_json_item_t*)0x1;
	iot_status_t result;
	iot_float64_t value;

	result = iot_json_decode_number( NULL, item, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_json_decode_number_valid( void **state )
{
	struct result_value_map {
		const char *key;
		iot_status_t status;
		iot_json_type_t type;
		iot_float64_t value;
	};
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	struct result_value_map results[] = {
		{ "real1", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_REAL, 0.0 },
		{ "real2", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_REAL, 0.00000123456 },
		{ "real3", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_REAL, -123456789.0 },
		{ "real4", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_REAL, -0.000123456789 },
		{ "real5", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_REAL, 3242344242.0 },
		{ "real6", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_REAL, 0.0324234 },
		{ "real7", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_REAL, 2147483634.000000 },
		{ "int1", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_INTEGER, 0.0 },
		{ "int2", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_INTEGER, 123456789.0 },
		{ "int3", IOT_STATUS_SUCCESS, IOT_JSON_TYPE_INTEGER, -123456789.0 },
		{ "string", IOT_STATUS_BAD_REQUEST, IOT_JSON_TYPE_STRING, 0.0 },
		{ NULL, IOT_STATUS_SUCCESS, IOT_JSON_TYPE_NULL, IOT_FALSE }
	};
	const struct result_value_map *rv = &results[0u];

	snprintf( json, 256u, "{"
		"\"real1\":0.0,"
		"\"real2\":0.00000123456,"
		"\"real3\":-123456789.0,"
		"\"real4\":-0.000123456789,"
		"\"real5\":3.242344242e+09,"
		"\"real6\":3.24234E-02,"
		"\"real7\":2.147483634e+09,"
		"\"int1\":0,"
		"\"int2\":123456789,"
		"\"int3\":-123456789,"
		"\"string\":\"blah\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	while ( rv->key )
	{
		float v1, v2;
		const iot_json_item_t *obj;
		iot_status_t status;
		iot_json_type_t type;
		iot_float64_t value = 0.999999999999;
		obj = iot_json_decode_object_find( decoder, root, rv->key );
		assert_non_null( obj );
		status = iot_json_decode_number( decoder, obj, &value );
		assert_int_equal( status, rv->status );
		/* down convert to value due to intrinsic
		 * limitations of comparing doubles */
		v1 = (float)value;
		v2 = (float)rv->value;
		assert_true( ((v1 - FLT_MIN) <= v2) && ((v1 + FLT_MIN) >= v2) );

		type = iot_json_decode_type( decoder, obj );
		assert_int_equal( type, rv->type );
		++rv;
	}
	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_find_invalid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *item;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	item = iot_json_decode_object_find( decoder, root, "item0" );
	assert_null( item );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_find_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *item;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	item = iot_json_decode_object_find( decoder, root, "item3" );
	assert_non_null( item );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_find_null_item( void **state )
{
	char buf[256u];
	iot_json_decoder_t *json;
	const iot_json_item_t *item;

	json = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( json );
	item = iot_json_decode_object_find( json, NULL, "item3" );
	assert_null( item );

	iot_json_decode_terminate( json );
}

static void test_iot_json_decode_object_find_null_json( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *item;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	item = iot_json_decode_object_find( NULL, root, "item3" );
	assert_null( item );

	iot_json_decode_terminate( decoder );
}


static void test_iot_json_decode_object_find_null_key( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *item;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	item = iot_json_decode_object_find( decoder, root, NULL );
	assert_null( item );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_null_item( void **state )
{
	const iot_json_object_iterator_t *i;
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x1;
	i = iot_json_decode_object_iterator( json, NULL );
	assert_null( i );
}

static void test_iot_json_decode_object_iterator_null_json( void **state )
{
	const iot_json_object_iterator_t *i;
	iot_json_item_t *item = (iot_json_item_t*)0x1;
	i = iot_json_decode_object_iterator( NULL, item );
	assert_null( i );
}


static void test_iot_json_decode_object_iterator_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( i );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_key_none_defined( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;
	const char *key;
	size_t key_len;

	snprintf( json, 256u, "{"
		"\"\":\"value1\","
		"\"\":\"value2\","
		"\"\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_key( decoder, root, iter, &key, &key_len );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( key );
	assert_int_equal( key_len, 0u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_key_null_item( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;
	const char *key;
	size_t key_len;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_key( decoder, NULL, iter, &key, &key_len );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( key );
	assert_int_equal( key_len, 0u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_key_null_iterator( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const char *key;
	size_t key_len;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	result = iot_json_decode_object_iterator_key( decoder, root, NULL, &key, &key_len );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( key );
	assert_int_equal( key_len, 0u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_key_null_json( void **state )
{
	char buf[1024u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;
	const char *key;
	size_t key_len;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 1024u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_key( NULL, root, iter, &key, &key_len );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( key );
	assert_int_equal( key_len, 0u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_key_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;
	const char *key = NULL;
	size_t key_len = 0u;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_key( decoder, root, iter, &key, &key_len );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( key );
	assert_int_equal( key_len, 5u );
	assert_memory_equal( key, "item1", key_len );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_next_null_item( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( i );

	i = iot_json_decode_object_iterator_next( decoder, NULL, i );
	assert_null( i );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_next_null_iterator( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( i );

	i = iot_json_decode_object_iterator_next( decoder, root, NULL );
	assert_null( i );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_next_null_json( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *i;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	i = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( i );

	i = iot_json_decode_object_iterator_next( NULL, root, i );
	assert_null( i );

	iot_json_decode_terminate( decoder );
}


static void test_iot_json_decode_object_iterator_next_null_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;
	size_t i, size;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	size = iot_json_decode_object_size( decoder, root );
	assert_true( size > 0 );

	for ( i = 0u; i < size; ++i )
	{
		iter = iot_json_decode_object_iterator_next( decoder, root, iter );
		if ( i == size - 1u )
			assert_null( iter );
		else
			assert_non_null( iter );
	}

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_value_null_item( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *item = (const iot_json_item_t *)0x1;
	const iot_json_object_iterator_t *iter;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_value( decoder, NULL, iter, &item );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( item );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_value_null_iterator( void **state )
{
	char buf[256u];
	iot_json_decoder_t *decoder;
	const iot_json_item_t *obj = (const iot_json_item_t *)0x1;
	const iot_json_item_t *item = (const iot_json_item_t *)0x2;
	iot_status_t result;

	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );

	result = iot_json_decode_object_iterator_value( decoder, obj, NULL, &item );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( item );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_value_null_json( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *obj = NULL;
	const iot_json_object_iterator_t *iter;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_value( NULL, root, iter, &obj );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( obj );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_value_null_out( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_object_iterator_t *iter;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_value( decoder, root, iter, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_iterator_value_valid( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	const iot_json_item_t *obj = NULL;
	const iot_json_object_iterator_t *iter;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iter = iot_json_decode_object_iterator( decoder, root );
	assert_non_null( iter );

	result = iot_json_decode_object_iterator_value( decoder, root, iter, &obj );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( obj );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_size_empty( void **state )
{
	char buf[256u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	size_t size;
	const iot_json_item_t *root = NULL;

	snprintf( json, 256u, "{}" );
	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	size = iot_json_decode_object_size( decoder, root );
	assert_int_equal( size, 0u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_size_non_object( void **state )
{
	char buf[256u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	size_t size;
	const iot_json_item_t *root = NULL;

	snprintf( json, 256u, "[ 1, 2, 3, 4]" );
	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	size = iot_json_decode_object_size( decoder, root );
	assert_int_equal( size, 0u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_size_null_decoder( void **state )
{
	size_t size;
	const iot_json_item_t *item = (const iot_json_item_t *)0x1;
	size = iot_json_decode_object_size( NULL, item );
	assert_int_equal( size, 0u );
}

static void test_iot_json_decode_object_size_null_item( void **state )
{
	size_t size;
	iot_json_decoder_t *decoder = (iot_json_decoder_t *)0x1;
	size = iot_json_decode_object_size( decoder, NULL );
	assert_int_equal( size, 0u );
}

static void test_iot_json_decode_object_size_single( void **state )
{
	char buf[256u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	size_t size;
	const iot_json_item_t *root = NULL;

	snprintf( json, 256u, "{\"item1\":\"value1\"}" );
	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	size = iot_json_decode_object_size( decoder, root );
	assert_int_equal( size, 1u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_object_size_multiple( void **state )
{
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	size_t size;
	const iot_json_item_t *root = NULL;

	snprintf( json, 256u, "{"
		"\"item1\":\"value1\","
		"\"item2\":\"value2\","
		"\"item3\":\"value3\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );
	size = iot_json_decode_object_size( decoder, root );
	assert_int_equal( size, 3u );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_parse_dynamic( void **state )
{
	char json[256u];
	iot_json_decoder_t *decoder;
#ifndef IOT_STACK_ONLY
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	will_return_always( __wrap_os_realloc, 1 );
#endif

	snprintf( json, 256u, "{\"item1\":\"value1\"}" );
#ifdef IOT_STACK_ONLY
	decoder = iot_json_decode_initialize( NULL, 0u, 0u );
	assert_null( decoder );
#else
	decoder = iot_json_decode_initialize( NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iot_json_decode_terminate( decoder );
#endif
}

static void test_iot_json_decode_parse_invalid_character( void **state )
{
	char buf[1024u];
	char error[128u];
	char json[300u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;

	snprintf( json, 300u,
	"{\n"
	"	\"cloud\"{\n"
	"		\"host\":\"api.devicewise.com\",\n"
	"		\"port\": 8883,\n"
	"		\"token\":\"1yqBX0sOhLB8Dtxo\"\n"
	"	}+\n" /* <---comma character is "+" character */
	"	\"validate_cloud_cert\":true,\n"
	"	\"ca_bundle_file\":\"/path/to/ca-certificates.crt\",\n"
	"	\"proxy\":{\n"
	"		\"host\":\"128.224.150.214\",\n"
	"		\"port\":1080,\n"
	"		\"type\":\"SOCKS5\",\n"
	"		\"username\":\"hdc\",\n"
	"		\"password\":\"hdc\"\n"
	"	}\n"
	"}\n" );
	decoder = iot_json_decode_initialize( buf, 1024u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, error, 128u );
	assert_int_equal( result, IOT_STATUS_PARSE_ERROR );
#if defined( IOT_JSON_JANSSON )
	assert_string_equal( error, "':' expected near '{' (line: 2, column: 9)" );
#elif defined( IOT_JSON_JSONC )
	assert_string_equal( error, "object property name separator ':' expected" );
#else /* defined( IOT_JSON_JSMN ) */
	assert_string_equal( error, "invalid character" );
#endif /* defined( IOT_JSON_JSMN ) */
	assert_null( root );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_parse_invalid_partial( void **state )
{
	char buf[256u];
	char error[128u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;

	snprintf( json, 256u, "{ \"not\": 12.34" );
	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, error, 128u );
	assert_int_equal( result, IOT_STATUS_PARSE_ERROR );
#if defined( IOT_JSON_JANSSON )
	assert_string_equal( error, "'}' expected near end of file (line: 1, column: 14)" );
#elif defined( IOT_JSON_JSONC )
	assert_string_equal( error, "object value separator ',' expected" );
#else /* defined( IOT_JSON_JSMN ) */
	assert_string_equal( error, "incomplete json string" );
#endif /* defined( IOT_JSON_JSMN ) */
	assert_null( root );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_parse_null_json( void **state )
{
	char buf[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;

	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, NULL, 0u, &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( root );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_parse_null_root( void **state )
{
	char buf[256u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;

	snprintf( json, 256u, "{\"item1\":\"value1\"}" );
	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), NULL, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_parse_valid( void **state )
{
	char buf[256u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;

	snprintf( json, 256u, "{\"item1\":\"value1\"}" );
	decoder = iot_json_decode_initialize( buf, 256u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_real_null_item( void **state )
{
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x1;
	iot_status_t result;
	iot_float64_t value;

	result = iot_json_decode_real( json, NULL, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_json_decode_real_null_json( void **state )
{
	const iot_json_item_t *item = (const iot_json_item_t*)0x1;
	iot_status_t result;
	iot_float64_t value;

	result = iot_json_decode_real( NULL, item, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_json_decode_real_valid( void **state )
{
	struct result_value_map {
		const char *key;
		iot_status_t status;
		iot_float64_t value;
	};
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	struct result_value_map results[] = {
		{ "real1", IOT_STATUS_SUCCESS, 0.0 },
		{ "real2", IOT_STATUS_SUCCESS, 0.00000123456 },
		{ "real3", IOT_STATUS_SUCCESS, -123456789.0 },
		{ "real4", IOT_STATUS_SUCCESS, -0.000123456789 },
		{ "real5", IOT_STATUS_SUCCESS, 3242344242.0 },
		{ "real6", IOT_STATUS_SUCCESS, 0.0324234 },
		{ "int", IOT_STATUS_BAD_REQUEST, 0.0 },
		{ "string", IOT_STATUS_BAD_REQUEST, 0.0 },
		{ NULL, IOT_STATUS_SUCCESS, IOT_FALSE }
	};
	const struct result_value_map *rv = &results[0u];

	snprintf( json, 256u, "{"
		"\"real1\":0.0,"
		"\"real2\":0.00000123456,"
		"\"real3\":-123456789.0,"
		"\"real4\":-0.000123456789,"
		"\"real5\":3.242344242e+09,"
		"\"real6\":3.24234E-02,"
		"\"int\":0,"
		"\"string\":\"blah\""
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	while ( rv->key )
	{
		float v1, v2;
		const iot_json_item_t *obj;
		iot_status_t status;
		iot_json_type_t type;
		iot_float64_t value = 0.999999999999;
		obj = iot_json_decode_object_find( decoder, root, rv->key );
		assert_non_null( obj );
		status = iot_json_decode_real( decoder, obj, &value );
		assert_int_equal( status, rv->status );
		/* down convert to value due to intrinsic
		 * limitations of comparing doubles */
		v1 = (float)value;
		v2 = (float)rv->value;
		assert_true( ((v1 - FLT_MIN) <= v2) && ((v1 + FLT_MIN) >= v2) );

		type = iot_json_decode_type( decoder, obj );
		if ( rv->status == IOT_STATUS_SUCCESS )
			assert_int_equal( type, IOT_JSON_TYPE_REAL );
		else
			assert_int_not_equal( type, IOT_JSON_TYPE_REAL );
		++rv;
	}
	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_string_null_item( void **state )
{
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x1;
	iot_status_t result;
	const char *value = "";
	size_t value_len = 1u;

	result = iot_json_decode_string( json, NULL, &value, &value_len );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( value );
	assert_int_equal( value, 0u );
}

static void test_iot_json_decode_string_null_json( void **state )
{
	const iot_json_item_t *item = (const iot_json_item_t*)0x1;
	iot_status_t result;
	const char *value = "";
	size_t value_len = 1u;

	result = iot_json_decode_string( NULL, item, &value, &value_len );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_null( value );
	assert_int_equal( value, 0u );
}

static void test_iot_json_decode_string_valid( void **state )
{
	struct result_value_map {
		const char *key;
		iot_status_t status;
		const char *value;
		size_t value_len;
	};
	char buf[512u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;
	struct result_value_map results[] = {
		{ "str1", IOT_STATUS_SUCCESS, "", 0u },
		{ "str2", IOT_STATUS_SUCCESS, "The quick brown fox jumps over the lazy dog", 43u },
		{ "str3", IOT_STATUS_SUCCESS, "123456789", 9u },
		{ "int", IOT_STATUS_BAD_REQUEST, NULL, 0u },
		{ NULL, IOT_STATUS_SUCCESS, NULL, 0u }
	};
	const struct result_value_map *rv = &results[0u];

	snprintf( json, 256u, "{"
		"\"str1\":\"\","
		"\"str2\":\"The quick brown fox jumps over the lazy dog\","
		"\"str3\":\"123456789\","
		"\"int\":123"
		"}" );
	decoder = iot_json_decode_initialize( buf, 512u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	while ( rv->key )
	{
		const iot_json_item_t *obj;
		iot_status_t status;
		iot_json_type_t type;
		const char *value;
		size_t value_len;
		obj = iot_json_decode_object_find( decoder, root, rv->key );
		assert_non_null( obj );
		status = iot_json_decode_string( decoder, obj, &value, &value_len );
		assert_int_equal( status, rv->status );
		assert_int_equal( value_len, rv->value_len );
		assert_memory_equal( value, rv->value, rv->value_len );

		type = iot_json_decode_type( decoder, obj );
		if ( rv->status == IOT_STATUS_SUCCESS )
			assert_int_equal( type, IOT_JSON_TYPE_STRING );
		else
			assert_int_not_equal( type, IOT_JSON_TYPE_STRING );
		++rv;
	}
	iot_json_decode_terminate( decoder );
}

static void test_iot_json_decode_type_null_item( void **state )
{
	iot_json_decoder_t *json = (iot_json_decoder_t*)0x1;
	iot_json_type_t result;
	result = iot_json_decode_type( json, NULL );
	assert_int_equal( result, IOT_JSON_TYPE_NULL );
}

static void test_iot_json_decode_type_null_json( void **state )
{
	iot_json_item_t *item = (iot_json_item_t*)0x1;
	iot_json_type_t result;
	result = iot_json_decode_type( NULL, item );
	assert_int_equal( result, IOT_JSON_TYPE_NULL );
}

static void test_iot_json_decode_type_valid( void **state )
{
	struct key_type_map {
		const char *key;
		iot_json_type_t type;
	};
	char buf[1024u];
	char json[256u];
	iot_json_decoder_t *decoder;
	iot_status_t result;
	const iot_json_item_t *root = NULL;

	struct key_type_map types[] = {
		{ "string", IOT_JSON_TYPE_STRING },
		{ "bool1", IOT_JSON_TYPE_BOOL },
		{ "bool2", IOT_JSON_TYPE_BOOL },
		{ "int", IOT_JSON_TYPE_INTEGER },
		{ "real1", IOT_JSON_TYPE_REAL },
		{ "real2", IOT_JSON_TYPE_REAL },
		{ "array", IOT_JSON_TYPE_ARRAY },
		{ "object", IOT_JSON_TYPE_OBJECT },
		{ NULL, IOT_JSON_TYPE_NULL }
	};
	const struct key_type_map *km = &types[0u];

	snprintf( json, 256u, "{"
		"\"string\":\"value\","
		"\"bool1\":true,"
		"\"bool2\":false,"
		"\"int\":1234,"
		"\"real1\":1.234,"
		"\"real2\":3.242344242e+09,"
		"\"array\":[1,2,3,4],"
		"\"object\":{}"
		"}" );
	decoder = iot_json_decode_initialize( buf, 1024u, 0u );
	assert_non_null( decoder );
	result = iot_json_decode_parse( decoder, json, strlen(json), &root, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( root );

	while ( km->key )
	{
		const iot_json_item_t *obj;
		iot_json_type_t type;
		obj = iot_json_decode_object_find( decoder, root, km->key );
		assert_non_null( obj );
		type = iot_json_decode_type( decoder, obj );
		assert_int_equal( type, km->type );
		++km;
	}

	iot_json_decode_terminate( decoder );
}

/* main */
int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_json_decode_array_at_invalid ),
		cmocka_unit_test( test_iot_json_decode_array_at_null_array ),
		cmocka_unit_test( test_iot_json_decode_array_at_null_item ),
		cmocka_unit_test( test_iot_json_decode_array_at_null_json ),
		cmocka_unit_test( test_iot_json_decode_array_at_valid ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_null_item ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_null_json ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_valid ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_next_null_item ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_next_null_iterator ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_next_null_json ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_next_null_valid ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_value_null_item ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_value_null_iterator ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_value_null_json ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_value_null_out ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_value_valid ),
		cmocka_unit_test( test_iot_json_decode_array_size_null_decoder ),
		cmocka_unit_test( test_iot_json_decode_array_size_null_item ),
		cmocka_unit_test( test_iot_json_decode_array_size_valid ),
		cmocka_unit_test( test_iot_json_decode_bool_null_item ),
		cmocka_unit_test( test_iot_json_decode_bool_null_json ),
		cmocka_unit_test( test_iot_json_decode_bool_valid ),
		cmocka_unit_test( test_iot_json_decode_initialize_null ),
		cmocka_unit_test( test_iot_json_decode_initialize_too_small ),
		cmocka_unit_test( test_iot_json_decode_initialize_valid ),
		cmocka_unit_test( test_iot_json_decode_integer_null_item ),
		cmocka_unit_test( test_iot_json_decode_integer_null_json ),
		cmocka_unit_test( test_iot_json_decode_integer_valid ),
		cmocka_unit_test( test_iot_json_decode_number_null_item ),
		cmocka_unit_test( test_iot_json_decode_number_null_json ),
		cmocka_unit_test( test_iot_json_decode_number_valid ),
		cmocka_unit_test( test_iot_json_decode_object_find_invalid ),
		cmocka_unit_test( test_iot_json_decode_object_find_valid ),
		cmocka_unit_test( test_iot_json_decode_object_find_null_item ),
		cmocka_unit_test( test_iot_json_decode_object_find_null_json ),
		cmocka_unit_test( test_iot_json_decode_object_find_null_key ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_null_item ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_null_json ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_valid ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_key_none_defined ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_key_null_item ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_key_null_iterator ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_key_null_json ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_key_valid ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_next_null_item ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_next_null_iterator ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_next_null_json ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_next_null_valid ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_value_null_item ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_value_null_iterator ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_value_null_json ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_value_null_out ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_value_valid ),
		cmocka_unit_test( test_iot_json_decode_object_size_empty ),
		cmocka_unit_test( test_iot_json_decode_object_size_non_object ),
		cmocka_unit_test( test_iot_json_decode_object_size_null_decoder ),
		cmocka_unit_test( test_iot_json_decode_object_size_null_item ),
		cmocka_unit_test( test_iot_json_decode_object_size_single ),
		cmocka_unit_test( test_iot_json_decode_object_size_multiple ),
		cmocka_unit_test( test_iot_json_decode_parse_dynamic ),
		cmocka_unit_test( test_iot_json_decode_parse_invalid_character ),
		cmocka_unit_test( test_iot_json_decode_parse_invalid_partial ),
		cmocka_unit_test( test_iot_json_decode_parse_null_json ),
		cmocka_unit_test( test_iot_json_decode_parse_null_root ),
		cmocka_unit_test( test_iot_json_decode_parse_valid ),
		cmocka_unit_test( test_iot_json_decode_real_null_item ),
		cmocka_unit_test( test_iot_json_decode_real_null_json ),
		cmocka_unit_test( test_iot_json_decode_real_valid ),
		cmocka_unit_test( test_iot_json_decode_string_null_item ),
		cmocka_unit_test( test_iot_json_decode_string_null_json ),
		cmocka_unit_test( test_iot_json_decode_string_valid ),
		cmocka_unit_test( test_iot_json_decode_type_null_item ),
		cmocka_unit_test( test_iot_json_decode_type_null_json ),
		cmocka_unit_test( test_iot_json_decode_type_valid ),
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
