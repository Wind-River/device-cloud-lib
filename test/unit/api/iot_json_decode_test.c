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
#include "utilities/app_json.h"

#include <float.h> /* for DBL_MIN */
#include <math.h> /* for fabs */
#include <stdlib.h>
#include <string.h>

/* iot_json_* functions are just wrappers around app_json,
 * we should do very simple tests that essentially just call
 * mocked function */

static void test_iot_json_decode_initialize( void **state )
{
	iot_json_decoder_t *result;
	will_return( __wrap_app_json_decode_initialize, NULL );
	result = iot_json_decode_initialize(  NULL, 0u, 0u );
	assert_null( result );
}

static void test_iot_json_decode_parse( void **state )
{
	iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	iot_status_t result;

	/* will_return( __wrap_app_json_decode_parse, IOT_STATUS_SUCCESS ); */

	result = iot_json_decode_parse( json, NULL, 0u, NULL, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_integer( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* i = ( iot_json_decoder_t* )0x1;
	iot_int64_t value;

	iot_status_t result;

	result = iot_json_decode_integer( json, i, &value );

	assert_int_equal( value,  1 );
	assert_int_equal( result,  IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_bool( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* i = ( iot_json_decoder_t* )0x1;
	iot_bool_t value;

	iot_status_t result;

	result = iot_json_decode_bool( json, i, &value );

	assert_int_equal( value,  IOT_TRUE );
	assert_int_equal( result,  IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_number( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* i = ( iot_json_decoder_t* )0x1;
	iot_float64_t value;

	iot_status_t result;
	result = iot_json_decode_number( json, i, &value );
	assert_int_equal( (int)value,  1 );
	assert_int_equal( result,  IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_real( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* i = ( iot_json_decoder_t* )0x1;
	iot_float64_t value;

	iot_status_t result;
	result = iot_json_decode_real( json, i, &value );

	assert_int_equal( (int)value,  1 );
	assert_int_equal( result,  IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_string( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* i = ( iot_json_decoder_t* )0x1;
	const char *value;
	size_t value_len;
	iot_status_t result;

	will_return( __wrap_app_json_decode_string, "test" );

	result = iot_json_decode_string( json, i, &value, &value_len );

	assert_string_equal( value,  "test" );
	assert_int_equal( value_len,  4 );
	assert_int_equal( result,  IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_terminate( void **state )
{
	iot_json_decoder_t* json = NULL;
	iot_json_decode_terminate( json ); /* essentially a noop */

	assert_null( json );
}


static void test_iot_json_decode_type( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* i = ( iot_json_decoder_t* )0x1;
	iot_json_type_t result;

	will_return( __wrap_app_json_decode_type, APP_JSON_TYPE_NULL );

	result = iot_json_decode_type( json, i );

	assert_null( result );
}

/* iot_json array related */

static void test_iot_json_decode_array_at( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* array = ( iot_json_item_t* )0x1;
	const iot_json_item_t* array_element = ( iot_json_item_t* )0x1;
	iot_status_t result;

	result = iot_json_decode_array_at( json, array, 0u, &array_element );

	assert_int_equal( result, IOT_STATUS_SUCCESS );
}


static void test_iot_json_decode_array_iterator( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* array = ( iot_json_item_t* )0x1;
	const iot_json_array_iterator_t * result;

	will_return( __wrap_app_json_decode_array_iterator, (iot_json_array_iterator_t *)0x1 );

	result = iot_json_decode_array_iterator( json, array );

	assert_non_null( result );
}

static void test_iot_json_decode_array_iterator_value( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* array = ( iot_json_item_t* )0x1;
	const iot_json_array_iterator_t * iter = NULL;
	iot_status_t result;

	result = iot_json_decode_array_iterator_value( json, array, iter, NULL );

	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_array_iterator_next( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* array = ( iot_json_item_t* )0x1;
	const iot_json_array_iterator_t * iter = NULL;
	const iot_json_array_iterator_t * result = NULL;

	will_return( __wrap_app_json_decode_array_iterator_next,
			( const iot_json_array_iterator_t *)0x1 );
	
	result = iot_json_decode_array_iterator_next( json, array, iter );

	assert_non_null( result );
}

static void test_iot_json_decode_array_size( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* array = ( iot_json_item_t* )0x1;
	size_t result;

	result = iot_json_decode_array_size( json, array );

	assert_int_equal( result, 1 );
}

static void test_iot_json_decode_object_find( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* item = ( iot_json_item_t* )0x1;
	const char *key = "key";
	const iot_json_item_t *result;

	will_return( __wrap_app_json_decode_object_find, (const iot_json_item_t *)0x1 );

	result = iot_json_decode_object_find( json, item, key );

	assert_non_null( result );
}

static void test_iot_json_decode_object_find_len( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* item = ( iot_json_item_t* )0x1;
	const char *key = "key";
	const iot_json_item_t *result;

	will_return( __wrap_app_json_decode_object_find_len, (const iot_json_item_t *)0x1 );

	result = iot_json_decode_object_find_len( json, item, key, 0 );

	assert_ptr_equal( result, (const iot_json_object_iterator_t *)0x1 );
}

static void test_iot_json_decode_object_iterator( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* item = ( iot_json_item_t* )0x1;
	const app_json_object_iterator_t *result;

	result = iot_json_decode_object_iterator( json, item );

	assert_ptr_equal( result, (const iot_json_object_iterator_t *)0x2  );
}

static void test_iot_json_decode_object_iterator_key( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* item = ( iot_json_item_t* )0x1;
	const iot_json_object_iterator_t *obj_iter = ( const iot_json_object_iterator_t* )0x1;
	const char *value;
	size_t value_len;
	iot_status_t result;

	will_return( __wrap_app_json_decode_object_iterator_key, "key" );

	result = iot_json_decode_object_iterator_key( json, item, obj_iter, &value, &value_len );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_decode_object_iterator_next( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* item = ( iot_json_item_t* )0x1;
	const iot_json_object_iterator_t *obj_iter = ( const iot_json_object_iterator_t* )0x1;
	const iot_json_item_t* result;

	will_return( __wrap_app_json_decode_object_iterator_next,
			(const iot_json_object_iterator_t *)0x1 );

	result = iot_json_decode_object_iterator_next( json, item, obj_iter );
	assert_ptr_equal( result, (const iot_json_object_iterator_t *)0x1 );
}

static void test_iot_json_decode_object_iterator_value( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* item = ( iot_json_item_t* )0x1;
	iot_json_object_iterator_t *obj_iter = ( iot_json_object_iterator_t* )0x2;
	iot_status_t result;
	const iot_json_item_t *value;

	result = iot_json_decode_object_iterator_value( json, item, obj_iter, &value );
	assert_int_equal( result,  IOT_STATUS_SUCCESS );
	assert_ptr_equal( value,  (const iot_json_item_t *)0x3 );
}

static void test_iot_json_decode_object_size( void **state )
{
	const iot_json_decoder_t* json = ( iot_json_decoder_t* )0x1;
	const iot_json_item_t* item = ( iot_json_item_t* )0x1;
	size_t result;

	result = iot_json_decode_object_size( json, item );
	assert_int_equal( result, 1u );
}

/* main */
int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_json_decode_initialize ),
		cmocka_unit_test( test_iot_json_decode_parse ),
		cmocka_unit_test( test_iot_json_decode_integer ),
		cmocka_unit_test( test_iot_json_decode_bool ),
		cmocka_unit_test( test_iot_json_decode_number ),
		cmocka_unit_test( test_iot_json_decode_real ),
		cmocka_unit_test( test_iot_json_decode_string ),
		cmocka_unit_test( test_iot_json_decode_type ),
		cmocka_unit_test( test_iot_json_decode_terminate ),
		cmocka_unit_test( test_iot_json_decode_array_at ),
		cmocka_unit_test( test_iot_json_decode_array_iterator ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_value ),
		cmocka_unit_test( test_iot_json_decode_array_iterator_next ),
		cmocka_unit_test( test_iot_json_decode_array_size ),
		cmocka_unit_test( test_iot_json_decode_object_find ),
		cmocka_unit_test( test_iot_json_decode_object_find_len ),
		cmocka_unit_test( test_iot_json_decode_object_iterator ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_key ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_next ),
		cmocka_unit_test( test_iot_json_decode_object_iterator_value ),
		cmocka_unit_test( test_iot_json_decode_object_size )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
