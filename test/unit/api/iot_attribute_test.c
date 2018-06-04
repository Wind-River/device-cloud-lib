/**
 * @file
 * @brief unit testing for IoT library (attribute source file)
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
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
#include "api/shared/iot_types.h"
#include "iot_build.h"

#include <string.h> /* for memset */

static void test_iot_attribute_publish_string_null_lib( void **state )
{
	iot_status_t result;

	result = iot_attribute_publish_string(
		NULL, NULL, NULL, "key", "value" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_attribute_publish_string_null_key( void **state )
{
	struct iot lib;
	iot_status_t result;

	memset( &lib, 0, sizeof( struct iot ) );
	result = iot_attribute_publish_string(
		&lib, NULL, NULL, NULL, "value" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_attribute_publish_string_null_value( void **state )
{
	struct iot lib;
	iot_status_t result;

	memset( &lib, 0, sizeof( struct iot ) );
	result = iot_attribute_publish_string(
		&lib, NULL, NULL, "key", NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_attribute_publish_string_time_out( void **state )
{
	struct iot lib;
	struct iot_options opts;
	iot_status_t result;

	memset( &lib, 0, sizeof( struct iot ) );
	memset( &opts, 0, sizeof( struct iot_options ) );
	opts.lib = &lib;
	opts.option = test_malloc( sizeof( struct iot_option ) );
	assert_non_null( opts.option );
	opts.option_count = 1u;
#ifndef IOT_STACK_ONLY
	opts.option[0u].name = test_malloc( IOT_NAME_MAX_LEN + 1u );
	assert_non_null( opts.option[0u].name );
#endif /* ifndef IOT_STACK_ONLY */
	snprintf( opts.option[0u].name, IOT_NAME_MAX_LEN, "max_time_out" );
	opts.option[0u].name[ IOT_NAME_MAX_LEN ] = '\0';
	opts.option[0u].data.has_value = IOT_TRUE;
	opts.option[0u].data.type = IOT_TYPE_UINT64;
	opts.option[0u].data.value.uint64 = 1000ul;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_TIMED_OUT );
	result = iot_attribute_publish_string(
		&lib, NULL, &opts, "key", "value" );
	assert_int_equal( result, IOT_STATUS_TIMED_OUT );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( opts.option[0u].name );
#endif /* ifndef IOT_STACK_ONLY */
	test_free( opts.option );
}

static void test_iot_attribute_publish_string_transmit_fail( void **state )
{
	struct iot lib;
	iot_status_t result;

	memset( &lib, 0, sizeof( struct iot ) );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_attribute_publish_string(
		&lib, NULL, NULL, "key", "value" );
	assert_int_equal( result, IOT_STATUS_FAILURE );
}

static void test_iot_attribute_publish_string_valid( void **state )
{
	struct iot lib;
	iot_status_t result;

	memset( &lib, 0, sizeof( struct iot ) );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_attribute_publish_string(
		&lib, NULL, NULL, "key", "value" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_attribute_publish_string_null_lib ),
		cmocka_unit_test( test_iot_attribute_publish_string_null_key ),
		cmocka_unit_test( test_iot_attribute_publish_string_null_value ),
		cmocka_unit_test( test_iot_attribute_publish_string_time_out ),
		cmocka_unit_test( test_iot_attribute_publish_string_transmit_fail ),
		cmocka_unit_test( test_iot_attribute_publish_string_valid )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
