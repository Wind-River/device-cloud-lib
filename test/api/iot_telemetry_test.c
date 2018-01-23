/**
 * @file
 * @brief unit testing for IoT library (telemetry source file)
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

#include <string.h>

static void test_iot_telemetry_allocate_empty( void **state )
{
	size_t i;
	iot_t lib;
	char name[] = "telemetry";
	iot_telemetry_t *result = NULL;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 0u;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* for name */
#endif
	result = iot_telemetry_allocate( &lib, name, IOT_TYPE_INT32 );
	assert_non_null( result );
	assert_int_equal( lib.telemetry_count, 1u );
	assert_ptr_equal( result, lib.telemetry_ptr[0] );
#ifndef IOT_STACK_ONLY
	os_free( result->name );
#endif
}

static void test_iot_telemetry_allocate_full( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN + 1u];
	char *t_names;
	iot_telemetry_t *result;
	iot_telemetry_t *stack_telemetry;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_TELEMETRY_MAX );
	assert_non_null( t_names );

	stack_telemetry = (iot_telemetry_t *)test_calloc( IOT_TELEMETRY_MAX - IOT_TELEMETRY_STACK_MAX,
	                                                  sizeof( iot_telemetry_t ) );
	assert_non_null( stack_telemetry );
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_MAX; i++ )
	{
		if ( i < IOT_TELEMETRY_STACK_MAX )
			lib.telemetry_ptr[i] = &lib.telemetry[i];
		else
			lib.telemetry_ptr[i] = &stack_telemetry[i - IOT_TELEMETRY_STACK_MAX];
		lib.telemetry_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.telemetry_ptr[i]->name, IOT_NAME_MAX_LEN, "telemetry %03lu", i );
	}
	snprintf( name, IOT_NAME_MAX_LEN, "telemetry %03d.5", IOT_TELEMETRY_MAX / 2u );
	lib.telemetry_count = IOT_TELEMETRY_MAX;
	result = iot_telemetry_allocate( &lib, name, IOT_TYPE_INT32 );
	assert_null( result );
	assert_int_equal( lib.telemetry_count, IOT_TELEMETRY_MAX );
	test_free( t_names );
	test_free( stack_telemetry );
}

static void test_iot_telemetry_allocate_stack_full( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN + 1u];
	char *t_names;
	iot_telemetry_t *result;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_TELEMETRY_STACK_MAX );
	assert_non_null( t_names );
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
	{
		lib.telemetry_ptr[i] = &lib.telemetry[i];
		lib.telemetry_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.telemetry_ptr[i]->name, IOT_NAME_MAX_LEN, "telemetry %03lu", i );
	}
	snprintf( name, IOT_NAME_MAX_LEN, "telemetry %03d.5", IOT_TELEMETRY_STACK_MAX / 2u );
	lib.telemetry_count = IOT_TELEMETRY_STACK_MAX;

#ifndef IOT_STACK_ONLY
	if ( IOT_TELEMETRY_MAX > IOT_TELEMETRY_STACK_MAX )
		will_return( __wrap_os_malloc, 1 ); /* for object */
	will_return( __wrap_os_malloc, 1 ); /* for name */
#endif
	result = iot_telemetry_allocate( &lib, name, IOT_TYPE_INT32 );

#ifdef IOT_STACK_ONLY
	assert_null( result );
#else
	if ( IOT_TELEMETRY_MAX > IOT_TELEMETRY_STACK_MAX )
	{
		assert_non_null( result );
		assert_int_equal( lib.telemetry_count, IOT_TELEMETRY_STACK_MAX + 1u );
		os_free( result->name );
		os_free( result );
	}
	else
	{
		os_free( result->name );
		assert_null( result );
		assert_int_equal( lib.telemetry_count, IOT_TELEMETRY_MAX );
	}
#endif
	test_free( t_names );
}

static void test_iot_telemetry_allocate_null_lib( void **state )
{
	char name[] = "telemetry";
	iot_telemetry_t *result = NULL;

	result = iot_telemetry_allocate( NULL, name, IOT_TYPE_INT32 );
	assert_null( result );
}

static void test_iot_telemetry_allocate_null_name( void **state )
{
	size_t i;
	iot_t lib;
	iot_telemetry_t *result = NULL;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 0u;
	result = iot_telemetry_allocate( &lib, NULL, IOT_TYPE_INT32 );
	assert_null( result );
	assert_int_equal( lib.telemetry_count, 0u );
}

static void test_iot_telemetry_allocate_valid( void **state )
{
	size_t i;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN];
	iot_telemetry_t *result = NULL;
	char *t_names;

	t_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_TELEMETRY_STACK_MAX );
	assert_non_null( t_names );

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
	{
		lib.telemetry_ptr[i] = &lib.telemetry[i];
		lib.telemetry_ptr[i]->name = &t_names[( IOT_NAME_MAX_LEN + 1u ) * i];
		snprintf( lib.telemetry_ptr[i]->name, IOT_NAME_MAX_LEN, "telemetry %03lu", i );
	}
	lib.telemetry_count = IOT_TELEMETRY_STACK_MAX - 1u;
	snprintf( name, IOT_NAME_MAX_LEN, "telemetry %03d.5", IOT_TELEMETRY_STACK_MAX / 2u );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 );
#endif
	result = iot_telemetry_allocate( &lib, name, IOT_TYPE_INT32 );
	assert_non_null( result );
	assert_int_equal( lib.telemetry_count, IOT_TELEMETRY_STACK_MAX );
	assert_ptr_equal( result, lib.telemetry_ptr[IOT_TELEMETRY_STACK_MAX / 2u + 1] );
#ifndef IOT_STACK_ONLY
	os_free( result->name );
#endif
	test_free( t_names );
}

static void test_iot_telemetry_option_get_not_found( void **state )
{
	char *a_names;
	struct iot_option attrs[ IOT_OPTION_MAX ];
	size_t i;
	iot_status_t result;
	iot_telemetry_t telemetry;
	iot_int32_t value = 0;

	a_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_OPTION_MAX );
	assert_non_null( a_names );
	bzero( &telemetry, sizeof( iot_telemetry_t ) );
	telemetry.option = attrs;
	telemetry.option_count = IOT_OPTION_MAX;
	for ( i = 0u; i < IOT_OPTION_MAX; i++ )
	{
#ifndef IOT_STACK_ONLY
		telemetry.option[i].name = &a_names[( IOT_NAME_MAX_LEN + 1u ) * i];
#endif
		snprintf( telemetry.option[i].name, IOT_NAME_MAX_LEN, "option %lu", i );
		telemetry.option[i].data.value.int32 = 2352;
		telemetry.option[i].data.type = IOT_TYPE_INT32;
		telemetry.option[i].data.has_value = IOT_TRUE;
	}
	result = iot_telemetry_option_get( &telemetry, "waldo", IOT_FALSE, IOT_TYPE_INT32, &value );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( value, 0 );
	test_free( a_names );
}

static void test_iot_telemetry_option_get_null_name( void **state )
{
	char *a_names;
	struct iot_option attrs[ IOT_OPTION_MAX ];
	size_t i;
	iot_status_t result;
	iot_telemetry_t telemetry;
	iot_int32_t value = 0;

	a_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_OPTION_MAX );
	assert_non_null( a_names );
	bzero( &telemetry, sizeof( iot_telemetry_t ) );
	bzero( &attrs, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	telemetry.option = attrs;
	telemetry.option_count = IOT_OPTION_MAX;
	for ( i = 0u; i < IOT_OPTION_MAX; i++ )
	{
#ifndef IOT_STACK_ONLY
		telemetry.option[i].name = &a_names[( IOT_NAME_MAX_LEN + 1u ) * i];
#endif
		snprintf( telemetry.option[i].name, IOT_NAME_MAX_LEN, "option %lu", i );
		telemetry.option[i].data.value.int32 = 2352;
		telemetry.option[i].data.type = IOT_TYPE_INT32;
		telemetry.option[i].data.has_value = IOT_TRUE;
	}
	result = iot_telemetry_option_get( &telemetry, NULL, IOT_FALSE, IOT_TYPE_INT32, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( value, 0 );
	test_free( a_names );
}

static void test_iot_telemetry_option_get_null_telemetry( void **state )
{
	iot_status_t result;
	iot_int32_t value = 0;

	result = iot_telemetry_option_get( NULL, "telemetry", IOT_FALSE, IOT_TYPE_INT32, &value );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( value, 0 );
}

static void test_iot_telemetry_option_get_valid( void **state )
{
	char *a_names;
	struct iot_option attrs[ IOT_OPTION_MAX ];
	size_t i;
	iot_status_t result;
	iot_telemetry_t telemetry;
	iot_int32_t value = 0;

	a_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_OPTION_MAX );
	assert_non_null( a_names );
	bzero( &telemetry, sizeof( iot_telemetry_t ) );
	bzero( &attrs, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	telemetry.option = attrs;
	telemetry.option_count = IOT_OPTION_MAX;
	for ( i = 0u; i < IOT_OPTION_MAX; i++ )
	{
#ifndef IOT_STACK_ONLY
		telemetry.option[i].name = &a_names[( IOT_NAME_MAX_LEN + 1u ) * i];
#endif
		snprintf( telemetry.option[i].name, IOT_NAME_MAX_LEN, "option %lu", i );
		telemetry.option[i].data.value.int32 = 2352;
		telemetry.option[i].data.type = IOT_TYPE_INT32;
		telemetry.option[i].data.has_value = IOT_TRUE;
	}
	result =
	    iot_telemetry_option_get( &telemetry, "option 2", IOT_FALSE, IOT_TYPE_INT32, &value );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( value, 2352 );
	test_free( a_names );
}

static void test_iot_telemetry_option_set_add( void **state )
{
	struct iot_option attrs[ IOT_OPTION_MAX ];
#ifndef IOT_STACK_ONLY
	char attr0_name[IOT_NAME_MAX_LEN + 1u];
#endif
	iot_status_t result;
	iot_telemetry_t telemetry;

	bzero( &telemetry, sizeof( iot_telemetry_t ) );
	bzero( &attrs, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	telemetry.option = attrs;
#ifndef IOT_STACK_ONLY
	attrs[0].name = attr0_name;
#endif
	telemetry.option_count = 1u;
	strncpy( telemetry.option[0].name, "option 0", IOT_NAME_MAX_LEN );
	telemetry.option[0].data.value.int32 = 2352;
	telemetry.option[0].data.type = IOT_TYPE_INT32;
	telemetry.option[0].data.has_value = IOT_TRUE;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* for name */
#endif
	result = iot_telemetry_option_set( &telemetry, "option 1", IOT_TYPE_INT32, 4527 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( telemetry.option_count, 2u );
	assert_string_equal( telemetry.option[1].name, "option 1" );
	assert_int_equal( telemetry.option[1].data.type, IOT_TYPE_INT32 );
	assert_int_equal( telemetry.option[1].data.value.int32, 4527 );
#ifndef IOT_STACK_ONLY
	os_free( telemetry.option[1].name );
#endif
}

static void test_iot_telemetry_option_set_full( void **state )
{
	char *a_names;
	struct iot_option attrs[ IOT_OPTION_MAX ];
	size_t i;
	iot_status_t result;
	iot_telemetry_t telemetry;

	a_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_OPTION_MAX );
	assert_non_null( a_names );
	bzero( &telemetry, sizeof( iot_telemetry_t ) );
	bzero( &attrs, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	telemetry.option = attrs;
	telemetry.option_count = IOT_OPTION_MAX;
	for ( i = 0u; i < IOT_OPTION_MAX; i++ )
	{
#ifndef IOT_STACK_ONLY
		telemetry.option[i].name = &a_names[( IOT_NAME_MAX_LEN + 1u ) * i];
#endif
		snprintf( telemetry.option[i].name, IOT_NAME_MAX_LEN, "option %lu", i );
		telemetry.option[i].data.value.int32 = 2352;
		telemetry.option[i].data.type = IOT_TYPE_INT32;
		telemetry.option[i].data.has_value = IOT_TRUE;
	}
	result = iot_telemetry_option_set( &telemetry, "another option", IOT_TYPE_INT32, 4527 );
	assert_int_equal( result, IOT_STATUS_FULL );
	test_free( a_names );
}

static void test_iot_telemetry_option_set_null_telemetry( void **state )
{
	iot_status_t result;

	result = iot_telemetry_option_set( NULL, "option", IOT_TYPE_INT32, 4527 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_telemetry_option_set_update( void **state )
{
	char *a_names;
	struct iot_option attrs[ IOT_OPTION_MAX ];
	size_t i;
	iot_status_t result;
	iot_telemetry_t telemetry;

	a_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * IOT_OPTION_MAX );
	assert_non_null( a_names );
	bzero( &telemetry, sizeof( iot_telemetry_t ) );
	bzero( &attrs, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	telemetry.option = attrs;
	telemetry.option_count = IOT_OPTION_MAX;
	for ( i = 0u; i < IOT_OPTION_MAX; i++ )
	{
#ifndef IOT_STACK_ONLY
		telemetry.option[i].name = &a_names[( IOT_NAME_MAX_LEN + 1u ) * i];
#endif
		snprintf( telemetry.option[i].name, IOT_NAME_MAX_LEN, "option %lu", i );
		telemetry.option[i].data.value.int32 = 2352;
		telemetry.option[i].data.type = IOT_TYPE_INT32;
		telemetry.option[i].data.has_value = IOT_TRUE;
	}
	result = iot_telemetry_option_set( &telemetry, "option 3", IOT_TYPE_INT16, 4527 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( telemetry.option[3].data.type, IOT_TYPE_INT16 );
	assert_int_equal( telemetry.option[3].data.value.int16, 4527 );
	test_free( a_names );
}

static void test_iot_telemetry_option_set_raw_valid( void **state )
{
	char *a_names;
	struct iot_option attrs[ 2u ];
#ifndef IOT_STACK_ONLY
	size_t i;
#endif
	iot_status_t result;
	iot_telemetry_t telemetry;
	char data[] = "this is some random data";

	a_names = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) * 2u );
	assert_non_null( a_names );
	bzero( &telemetry, sizeof( iot_telemetry_t ) );
	bzero( &attrs, sizeof( struct iot_option ) * 2u );
	telemetry.option = attrs;
	telemetry.option_count = 1u;
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < 2u; ++i )
		telemetry.option[i].name = &a_names[( IOT_NAME_MAX_LEN + 1u ) * i];
#endif
	strncpy( telemetry.option[0].name, "option 0", IOT_NAME_MAX_LEN );
	telemetry.option[0].data.value.int32 = 2352;
	telemetry.option[0].data.type = IOT_TYPE_INT32;
	telemetry.option[0].data.has_value = IOT_TRUE;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 );
#endif
	result = iot_telemetry_option_set_raw(
	    &telemetry, "option 1", sizeof( data ), (const void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( telemetry.option_count, 2u );
	assert_string_equal( telemetry.option[1].name, "option 1" );
	assert_int_equal( telemetry.option[1].data.type, IOT_TYPE_RAW );
	assert_int_equal( telemetry.option[1].data.has_value, IOT_TRUE );
	assert_int_equal( telemetry.option[1].data.value.raw.length, sizeof( data ) );
	assert_string_equal( (const char *)telemetry.option[1].data.value.raw.ptr, data );
	assert_ptr_equal( telemetry.option[1].data.value.raw.ptr, data );
#ifndef IOT_STACK_ONLY
	test_free( telemetry.option[1].name );
#endif
	test_free( a_names );
}

static void test_iot_telemetry_deregister_not_registered( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->state = IOT_ITEM_DEREGISTERED;
	result = iot_telemetry_deregister( telemetry, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( lib.telemetry_count, 1u );
	assert_int_equal( telemetry->state, IOT_ITEM_DEREGISTERED );
}

static void test_iot_telemetry_deregister_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = NULL;
	telemetry->state = IOT_ITEM_REGISTERED;
	result = iot_telemetry_deregister( telemetry, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( lib.telemetry_count, 1u );
	assert_int_equal( telemetry->state, IOT_ITEM_REGISTERED );
}

static void test_iot_telemetry_deregister_null_telemetry( void **state )
{
	iot_status_t result;

	result = iot_telemetry_deregister( NULL, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_telemetry_deregister_transmit_fail( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->state = IOT_ITEM_REGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_telemetry_deregister( telemetry, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_FAILURE );
	assert_int_equal( lib.telemetry_count, 1u );
	assert_int_equal( telemetry->state, IOT_ITEM_DEREGISTER_PENDING );
}

static void test_iot_telemetry_deregister_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->state = IOT_ITEM_REGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_deregister( telemetry, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.telemetry_count, 1u );
	assert_int_equal( telemetry->state, IOT_ITEM_DEREGISTERED );
}

static void test_iot_telemetry_free_options( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;
#ifdef IOT_STACK_ONLY
	struct iot_option _option[IOT_OPTION_MAX];
#endif

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 2u;
	telemetry = lib.telemetry_ptr[1];
	telemetry->lib = &lib;
	telemetry->state = IOT_ITEM_REGISTERED;
#ifdef IOT_STACK_ONLY
	telemetry->option = _option;
#else
	will_return( __wrap_os_malloc, 1 );
	telemetry->option = os_malloc( sizeof( struct iot_option ) * IOT_OPTION_MAX );
	assert_non_null( telemetry->option );
#endif
	bzero( telemetry->option, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	for ( i = 0u; i < IOT_OPTION_MAX; i++ )
	{
		telemetry->option[i].data.type = IOT_TYPE_RAW;
		will_return( __wrap_os_malloc, 1 );
		telemetry->option[i].data.heap_storage = os_malloc( 20 );
		telemetry->option[i].data.value.raw.ptr = telemetry->option[i].data.heap_storage;
		telemetry->option[i].data.value.raw.length = 20;
		telemetry->option[i].data.has_value = IOT_TRUE;
	}
	telemetry->option_count = IOT_OPTION_MAX;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_free( telemetry, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.telemetry_count, 1u );
	assert_int_equal( telemetry->state, IOT_ITEM_DEREGISTERED );
}

static void test_iot_telemetry_free_null_lib( void **state )
{
	iot_status_t result;
	result = iot_telemetry_free( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_telemetry_free_null_telemetry( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 2u;
	telemetry = lib.telemetry_ptr[1];
	telemetry->lib = NULL;
	telemetry->state = IOT_ITEM_REGISTERED;
	result = iot_telemetry_free( telemetry, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( lib.telemetry_count, 2u );
	assert_int_equal( telemetry->state, IOT_ITEM_REGISTERED );
}

static void test_iot_telemetry_publish_number_types( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	/* UINT8 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_UINT8;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_UINT8, 254 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* UINT16 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_UINT16;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_UINT16, 0xff00 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* UINT32 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_UINT32;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_UINT32, 0xff00ffee );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* UINT64 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_UINT64;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_UINT64, 0xff00ffeeaabbccddLL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* INT8 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_INT8;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_INT8, 254 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* INT16 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_INT16;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_INT16, 0xff00 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* INT32 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_INT32;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_INT32, 0xff00ffee );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* INT64 */
	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_INT64;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_INT64, 0xff00ffeeaabbccddLL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_telemetry_publish_location( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;
	iot_location_t data;
	iot_float64_t accuracy = 123.456;
	iot_float64_t altitude = 234.567;
	iot_float64_t altitude_accuracy = 345.678;
	iot_float64_t heading = 456.789;
	iot_float64_t latitude = 567.890;
	iot_float64_t longitude = 678.901;
	iot_uint32_t source = 1;
	iot_float64_t speed = 789.012;
	char tag[] = "somelocation";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_LOCATION;
	data.accuracy = accuracy;
	data.altitude = altitude;
	data.altitude_accuracy = altitude_accuracy;
	data.heading = heading;
	data.latitude = latitude;
	data.longitude = longitude;
	data.source = source;
	data.speed = speed;
	data.tag = tag;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_LOCATION, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_telemetry_publish_location_no_memory( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;
	iot_location_t data;
	iot_float64_t accuracy = 123.456;
	iot_float64_t altitude = 234.567;
	iot_float64_t altitude_accuracy = 345.678;
	iot_float64_t heading = 456.789;
	iot_float64_t latitude = 567.890;
	iot_float64_t longitude = 678.901;
	iot_uint32_t source = 1;
	iot_float64_t speed = 789.012;
	char tag[] = "somelocation";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_LOCATION;
	data.accuracy = accuracy;
	data.altitude = altitude;
	data.altitude_accuracy = altitude_accuracy;
	data.heading = heading;
	data.latitude = latitude;
	data.longitude = longitude;
	data.source = source;
	data.speed = speed;
	data.tag = tag;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_NO_MEMORY );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_LOCATION, &data );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
}

static void test_iot_telemetry_publish_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = NULL;
	telemetry->type = IOT_TYPE_INT32;
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_INT32, 32 );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_telemetry_publish_null_telemetry( void **state )
{
	iot_status_t result;

	result = iot_telemetry_publish( NULL, NULL, 0u, IOT_TYPE_INT32, 32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_telemetry_publish_null_type( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_NULL;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_INT32, 32 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_telemetry_publish_string( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;
	char data[] = "some text";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_STRING;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_STRING, data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_telemetry_publish_string_no_memory( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;
	char data[] = "some text";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_STRING;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_NO_MEMORY );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_STRING, data );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
}

static void test_iot_telemetry_publish_string_null( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_STRING;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish( telemetry, NULL, 0u, IOT_TYPE_STRING, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_telemetry_publish_raw_no_memory( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;
	char data[] = "some text";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_RAW;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_NO_MEMORY );
	result = iot_telemetry_publish_raw( telemetry, NULL, 0u,
		sizeof( data ), (void *)data );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
}

static void test_iot_telemetry_publish_raw_null( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_RAW;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish_raw( telemetry, NULL, 0u, 0u, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_telemetry_publish_raw_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;
	char data[] = "some text";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_RAW;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_publish_raw( telemetry, NULL, 0u,
		sizeof( data ), (void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_telemetry_register_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = NULL;
	telemetry->state = IOT_ITEM_DEREGISTERED;
	result = iot_telemetry_register( telemetry, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( telemetry->state, IOT_ITEM_DEREGISTERED );
}

static void test_iot_telemetry_register_null_telemetry( void **state )
{
	iot_status_t result;

	result = iot_telemetry_register( NULL, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_telemetry_register_transmit_fail( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->state = IOT_ITEM_DEREGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_telemetry_register( telemetry, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_FAILURE );
	assert_int_equal( telemetry->state, IOT_ITEM_REGISTER_PENDING );
}

static void test_iot_telemetry_register_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->state = IOT_ITEM_DEREGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_telemetry_register( telemetry, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( telemetry->state, IOT_ITEM_REGISTERED );
}

static void test_iot_telemetry_timestamp_set_null_obj( void **state )
{
	iot_status_t result = iot_telemetry_timestamp_set( NULL, 1234u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_telemetry_timestamp_set_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;
	iot_telemetry_t *telemetry;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; i++ )
		lib.telemetry_ptr[i] = &lib.telemetry[i];
	lib.telemetry_count = 1u;
	telemetry = lib.telemetry_ptr[0];
	telemetry->lib = &lib;
	telemetry->type = IOT_TYPE_INT32;

	result = iot_telemetry_timestamp_set( telemetry, 1234u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_telemetry_allocate_empty ),
		cmocka_unit_test( test_iot_telemetry_allocate_full ),
		cmocka_unit_test( test_iot_telemetry_allocate_stack_full ),
		cmocka_unit_test( test_iot_telemetry_allocate_null_lib ),
		cmocka_unit_test( test_iot_telemetry_allocate_null_name ),
		cmocka_unit_test( test_iot_telemetry_allocate_valid ),
		cmocka_unit_test( test_iot_telemetry_option_get_not_found ),
		cmocka_unit_test( test_iot_telemetry_option_get_null_name ),
		cmocka_unit_test( test_iot_telemetry_option_get_null_telemetry ),
		cmocka_unit_test( test_iot_telemetry_option_get_valid ),
		cmocka_unit_test( test_iot_telemetry_option_set_add ),
		cmocka_unit_test( test_iot_telemetry_option_set_full ),
		cmocka_unit_test( test_iot_telemetry_option_set_null_telemetry ),
		cmocka_unit_test( test_iot_telemetry_option_set_update ),
		cmocka_unit_test( test_iot_telemetry_option_set_raw_valid ),
		cmocka_unit_test( test_iot_telemetry_deregister_not_registered ),
		cmocka_unit_test( test_iot_telemetry_deregister_null_lib ),
		cmocka_unit_test( test_iot_telemetry_deregister_null_telemetry ),
		cmocka_unit_test( test_iot_telemetry_deregister_transmit_fail ),
		cmocka_unit_test( test_iot_telemetry_deregister_valid ),
		cmocka_unit_test( test_iot_telemetry_free_options ),
		cmocka_unit_test( test_iot_telemetry_free_null_lib ),
		cmocka_unit_test( test_iot_telemetry_free_null_telemetry ),
		cmocka_unit_test( test_iot_telemetry_publish_number_types ),
		cmocka_unit_test( test_iot_telemetry_publish_location ),
		cmocka_unit_test( test_iot_telemetry_publish_location_no_memory ),
		cmocka_unit_test( test_iot_telemetry_publish_null_lib ),
		cmocka_unit_test( test_iot_telemetry_publish_null_telemetry ),
		cmocka_unit_test( test_iot_telemetry_publish_null_type ),
		cmocka_unit_test( test_iot_telemetry_publish_string ),
		cmocka_unit_test( test_iot_telemetry_publish_string_no_memory ),
		cmocka_unit_test( test_iot_telemetry_publish_string_null ),
		cmocka_unit_test( test_iot_telemetry_publish_raw_no_memory ),
		cmocka_unit_test( test_iot_telemetry_publish_raw_null ),
		cmocka_unit_test( test_iot_telemetry_publish_raw_valid ),
		cmocka_unit_test( test_iot_telemetry_register_null_lib ),
		cmocka_unit_test( test_iot_telemetry_register_null_telemetry ),
		cmocka_unit_test( test_iot_telemetry_register_transmit_fail ),
		cmocka_unit_test( test_iot_telemetry_register_valid ),
		cmocka_unit_test( test_iot_telemetry_timestamp_set_null_obj ),
		cmocka_unit_test( test_iot_telemetry_timestamp_set_valid )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
