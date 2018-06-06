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

#include "api/public/iot.h"
#include "api/public/iot_json.h"

#include <float.h> /* for DBL_MIN */
#include <math.h> /* for fabs */
#include <stdlib.h>
#include <string.h>

/* iot_json_* functions are just wrappers around app_json,
 * we should do very simple tests that essentially just call
 * mocked function */

static void test_iot_json_encode_object_start( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_object_start( json, "key" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_object_cancel( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_object_cancel( json );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_object_clear( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_object_clear( json );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_object_end( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_object_end( json );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_integer( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_integer( json, "integer", 1u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_bool( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_bool( json, "bool", IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_real( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_real( json, "real", 1.2345 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_string( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_string( json, "string", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_array_start( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_array_start( json, "array" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_array_end( void **state )
{
	iot_status_t result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_array_end( json );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_json_encode_dump( void **state )
{
	const char *result;
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	result = iot_json_encode_dump( json );
	assert_string_equal( result, "dump" );
}

static void test_iot_json_encode_initialize( void **state )
{
	char buf[8];
	iot_json_encoder_t *result;
	result = iot_json_encode_initialize( buf, 0u, 0u );
	assert_ptr_equal( result, ( iot_json_encoder_t * )0x1 );
}
static void test_iot_json_encode_terminate( void **state )
{
	iot_json_encoder_t *json = (iot_json_encoder_t *) 0x1;
	iot_json_encode_terminate( json );
}

/* main */
int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_json_encode_object_start ),
		cmocka_unit_test( test_iot_json_encode_object_cancel ),
		cmocka_unit_test( test_iot_json_encode_object_clear ),
		cmocka_unit_test( test_iot_json_encode_object_end ),
		cmocka_unit_test( test_iot_json_encode_bool ),
		cmocka_unit_test( test_iot_json_encode_integer ),
		cmocka_unit_test( test_iot_json_encode_real ),
		cmocka_unit_test( test_iot_json_encode_string ),
		cmocka_unit_test( test_iot_json_encode_array_start ),
		cmocka_unit_test( test_iot_json_encode_array_end ),
		cmocka_unit_test( test_iot_json_encode_dump ),
		cmocka_unit_test( test_iot_json_encode_initialize ),
		cmocka_unit_test( test_iot_json_encode_terminate ),
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
