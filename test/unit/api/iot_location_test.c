/**
 * @file
 * @brief unit testing for IoT library (location source file)
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
#include "api/shared/iot_types.h"
#include "iot_build.h"

#include <stdlib.h>
#include <string.h>

/** @brief Maximum value of location heading property */
#define IOT_LOCATION_HEADING_MAX 360u
/** @brief Minimum value of location latitude property */
#define IOT_LOCATION_LATITUDE_MAX 90u
/** @brief Minimum value of location longitude property */
#define IOT_LOCATION_LONGITUDE_MIN -180

/* iot_location_accuracy_set */
static void test_iot_location_accuracy_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t value = 1.0;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_accuracy_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_ACCURACY );
	assert_false( sample.accuracy - value );
}

static void test_iot_location_accuracy_set_null_sample( void **state )
{
	iot_status_t result;
	const iot_float64_t value = 1.0;

	result = iot_location_accuracy_set( NULL, value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* iot_location_allocate */
static void test_iot_location_allocate( void **state )
{
	struct iot_location *sample;
	const iot_float64_t latitude = 1.23456789;
	const iot_float64_t longitude = 9.87654321;

	will_return( __wrap_os_malloc, 1 );
	sample = iot_location_allocate( latitude, longitude );
	assert_non_null( sample );

	iot_location_free( sample );
}

static void test_iot_location_allocate_latitude_out_of_range( void **state )
{
	struct iot_location *sample;
	const iot_float64_t latitude = IOT_LOCATION_LATITUDE_MAX + 1;
	const iot_float64_t longitude = 9.87654321;

	sample = iot_location_allocate( latitude, longitude );
	assert_null( sample );
}

static void test_iot_location_allocate_longitude_out_of_range( void **state )
{
	struct iot_location *sample;
	const iot_float64_t latitude = 1.23456789;
	const iot_float64_t longitude = IOT_LOCATION_LONGITUDE_MIN - 1;

	sample = iot_location_allocate( latitude, longitude );
	assert_null( sample );
}

static void test_iot_location_allocate_no_memory( void **state )
{
	struct iot_location *sample;
	const iot_float64_t latitude = 1.23456789;
	const iot_float64_t longitude = 9.87654321;

	will_return( __wrap_os_malloc, 0 );
	sample = iot_location_allocate( latitude, longitude );
	assert_null( sample );
}

/* test_iot_location_altitude_accuracy_set */
static void test_iot_location_altitude_accuracy_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t value = 1.0;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_altitude_accuracy_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_ALTITUDE_ACCURACY );
	assert_false( sample.altitude_accuracy - value );
}

static void test_iot_location_altitude_accuracy_set_null_sample( void **state )
{
	iot_status_t result;
	const iot_float64_t value = 1.0;

	result = iot_location_altitude_accuracy_set( NULL, value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* iot_location_altitude_set */
static void test_iot_location_altitude_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t value = 1.0;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_altitude_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_ALTITUDE );
	assert_false( sample.altitude - value );
}

static void test_iot_location_altitude_set_null_sample( void **state )
{
	iot_status_t result;
	const iot_float64_t value = 1.0;

	result = iot_location_altitude_set( NULL, value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* test_iot_location_free */
static void test_iot_location_free( void **state )
{
	struct iot_location *sample;
	iot_status_t result;
	const iot_float64_t latitude = 1.23456789;
	const iot_float64_t longitude = 9.87654321;

	will_return( __wrap_os_malloc, 1 ); /* for location */
	sample = iot_location_allocate( latitude, longitude );
	assert_non_null( sample );

	result = iot_location_free( sample );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_location_free_null_sample( void **state )
{
	iot_status_t result;

	result = iot_location_free( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* iot_location_heading_set */
static void test_iot_location_heading_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t value = 1.0;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_heading_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_HEADING );
	assert_false( sample.heading - value );
}

static void test_iot_location_heading_set_null_sample( void **state )
{
	iot_status_t result;
	const iot_float64_t value = 1.0;

	result = iot_location_heading_set( NULL, value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_location_heading_set_out_of_range( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t value = IOT_LOCATION_HEADING_MAX + 0.1;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_heading_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_OUT_OF_RANGE );
	assert_false( sample.flags & IOT_FLAG_LOCATION_HEADING );
	assert_false( sample.heading );
}

/* iot_location_set */
static void test_iot_location_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t latitude = 1.23456789;
	const iot_float64_t longitude = 9.87654321;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_set( &sample, latitude, longitude );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_false( sample.latitude - latitude );
	assert_false( sample.longitude - longitude );
}

static void test_iot_location_set_latitude_out_of_range( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t latitude = IOT_LOCATION_LATITUDE_MAX + 1.23456789;
	const iot_float64_t longitude = 9.87654321;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_set( &sample, latitude, longitude );
	assert_int_equal( result, IOT_STATUS_OUT_OF_RANGE );
	assert_false( sample.latitude );
	assert_false( sample.longitude );
}

static void test_iot_location_set_longitude_out_of_range( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t latitude = 1.23456789;
	const iot_float64_t longitude = IOT_LOCATION_LONGITUDE_MIN - 9.87654321;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_set( &sample, latitude, longitude );
	assert_int_equal( result, IOT_STATUS_OUT_OF_RANGE );
	assert_false( sample.latitude );
	assert_false( sample.longitude );
}

static void test_iot_location_set_null_sample( void **state )
{
	iot_status_t result;
	const iot_float64_t latitude = 1.23456789;
	const iot_float64_t longitude = 9.87654321;

	result = iot_location_set( NULL, latitude, longitude );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* iot_location_source_set */
static void test_iot_location_source_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const unsigned int value = rand() % ( IOT_LOCATION_SOURCE_WIFI + 1 );

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_source_set( &sample, (iot_location_source_t)value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_SOURCE );
	assert_false( sample.source - value );
}

static void test_iot_location_source_set_invalid_source( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_int32_t value = IOT_LOCATION_SOURCE_WIFI + 1;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_source_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_false( sample.flags & IOT_FLAG_LOCATION_SOURCE );
	assert_false( sample.source );
}

static void test_iot_location_source_set_null_sample( void **state )
{
	iot_status_t result;
	const iot_int32_t value = rand() % ( IOT_LOCATION_SOURCE_WIFI + 1 );

	result = iot_location_source_set( NULL, (iot_location_source_t)value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* iot_location_speed_set */
static void test_iot_location_speed_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const iot_float64_t value = 98.7654321;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_speed_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_SPEED );
	assert_false( sample.speed - value );
}

static void test_iot_location_speed_set_null_sample( void **state )
{
	iot_status_t result;
	const iot_float64_t value = 98.7654321;

	result = iot_location_speed_set( NULL, value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* iot_location_tag_set */
static void test_iot_location_tag_set( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const char *value = "test tag";

	memset( &sample, 0, sizeof( struct iot_location ) );

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* for tag */
#endif
	result = iot_location_tag_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_TAG );
	assert_string_equal( sample.tag, value );
#ifndef IOT_STACK_ONLY
	os_free( sample.tag );
#endif
}

static void test_iot_location_tag_set_empty_tag( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const char *value = "";

	memset( &sample, 0, sizeof( struct iot_location ) );

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* for tag */
#endif
	result = iot_location_tag_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( sample.flags & IOT_FLAG_LOCATION_TAG );
	assert_string_equal( sample.tag, value );
#ifndef IOT_STACK_ONLY
	os_free( sample.tag );
#endif
}

static void test_iot_location_tag_set_null_sample( void **state )
{
	iot_status_t result;
	const char *value = "test tag";

	result = iot_location_tag_set( NULL, value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_location_tag_set_null_tag( void **state )
{
	struct iot_location sample;
	iot_status_t result;
	const char *value = NULL;

	memset( &sample, 0, sizeof( struct iot_location ) );

	result = iot_location_tag_set( &sample, value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_false( sample.flags & IOT_FLAG_LOCATION_TAG );
	assert_null( sample.tag );
}

/* main */
int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_location_accuracy_set ),
		cmocka_unit_test( test_iot_location_accuracy_set_null_sample ),
		cmocka_unit_test( test_iot_location_allocate ),
		cmocka_unit_test( test_iot_location_allocate_latitude_out_of_range ),
		cmocka_unit_test( test_iot_location_allocate_longitude_out_of_range ),
		cmocka_unit_test( test_iot_location_allocate_no_memory ),
		cmocka_unit_test( test_iot_location_altitude_accuracy_set ),
		cmocka_unit_test( test_iot_location_altitude_accuracy_set_null_sample ),
		cmocka_unit_test( test_iot_location_altitude_set ),
		cmocka_unit_test( test_iot_location_altitude_set_null_sample ),
		cmocka_unit_test( test_iot_location_free ),
		cmocka_unit_test( test_iot_location_free_null_sample ),
		cmocka_unit_test( test_iot_location_heading_set ),
		cmocka_unit_test( test_iot_location_heading_set_null_sample ),
		cmocka_unit_test( test_iot_location_heading_set_out_of_range ),
		cmocka_unit_test( test_iot_location_set ),
		cmocka_unit_test( test_iot_location_set_latitude_out_of_range ),
		cmocka_unit_test( test_iot_location_set_longitude_out_of_range ),
		cmocka_unit_test( test_iot_location_set_null_sample ),
		cmocka_unit_test( test_iot_location_source_set ),
		cmocka_unit_test( test_iot_location_source_set_invalid_source ),
		cmocka_unit_test( test_iot_location_source_set_null_sample ),
		cmocka_unit_test( test_iot_location_speed_set ),
		cmocka_unit_test( test_iot_location_speed_set_null_sample ),
		cmocka_unit_test( test_iot_location_tag_set ),
		cmocka_unit_test( test_iot_location_tag_set_empty_tag ),
		cmocka_unit_test( test_iot_location_tag_set_null_sample ),
		cmocka_unit_test( test_iot_location_tag_set_null_tag )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
