/**
 * @file
 * @brief unit testing for IoT library (base source file)
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
#include "api/shared/iot_types.h"
#include "iot_build.h"

#include <string.h>

static void test_log_callback( iot_log_level_t log_level,
                               const iot_log_source_t *log_source,
                               const char *message,
                               void *user_data )
{
	check_expected( log_level );
	check_expected( message );
	check_expected( user_data );
}

/* iot_config_get */
static void test_iot_config_get_not_found( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt;
	struct iot_data data;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_RAW;
	opt.data.heap_storage = test_malloc( 1u );
	opt.data.has_value = IOT_TRUE;
	opt.data.value.raw.ptr = opt.data.heap_storage;
	opt.data.value.raw.length = 1u;

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt;
	opts.option_count = 1u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_get( &lib, "opt_bool", IOT_FALSE,
		IOT_TYPE_BOOL, &data.value.boolean );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_float32", IOT_FALSE,
		IOT_TYPE_FLOAT32, &data.value.float32 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_float64", IOT_FALSE,
		IOT_TYPE_FLOAT64, &data.value.float64 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_int8", IOT_FALSE,
		IOT_TYPE_INT8, &data.value.int8 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_int16", IOT_FALSE,
		IOT_TYPE_INT16, &data.value.int16 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_int32", IOT_FALSE,
		IOT_TYPE_INT32, &data.value.int32 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_int64", IOT_FALSE,
		IOT_TYPE_INT64, &data.value.int64 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_raw", IOT_FALSE,
		IOT_TYPE_RAW, &data.value.raw );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_string", IOT_FALSE,
		IOT_TYPE_RAW, &data.value.string );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_uint8", IOT_FALSE,
		IOT_TYPE_UINT8, &data.value.uint8 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_uint16", IOT_FALSE,
		IOT_TYPE_UINT16, &data.value.uint16 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_uint32", IOT_FALSE,
		IOT_TYPE_UINT32, &data.value.uint32 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	result = iot_config_get( &lib, "opt_uint64", IOT_FALSE,
		IOT_TYPE_UINT64, &data.value.uint64 );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );

	/* clean up */
	os_free( opt.data.heap_storage );
}

static void test_iot_config_get_null_lib( void **state )
{
	struct iot_data data;
	iot_status_t result;

	result = iot_config_get( NULL, "opt_bool", IOT_FALSE,
		IOT_TYPE_BOOL, &data.value.boolean );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_float32", IOT_FALSE,
		IOT_TYPE_FLOAT32, &data.value.float32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_float64", IOT_FALSE,
		IOT_TYPE_FLOAT64, &data.value.float64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_int8", IOT_FALSE,
		IOT_TYPE_INT8, &data.value.int8 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_int16", IOT_FALSE,
		IOT_TYPE_INT16, &data.value.int16 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_int32", IOT_FALSE,
		IOT_TYPE_INT32, &data.value.int32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_int64", IOT_FALSE,
		IOT_TYPE_INT64, &data.value.int64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_raw", IOT_FALSE,
		IOT_TYPE_RAW, &data.value.raw );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_string", IOT_FALSE,
		IOT_TYPE_RAW, &data.value.string );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_uint8", IOT_FALSE,
		IOT_TYPE_UINT8, &data.value.uint8 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_uint16", IOT_FALSE,
		IOT_TYPE_UINT16, &data.value.uint16 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_uint32", IOT_FALSE,
		IOT_TYPE_UINT32, &data.value.uint32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( NULL, "opt_uint64", IOT_FALSE,
		IOT_TYPE_UINT64, &data.value.uint64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_get_null_name( void **state )
{
	struct iot_data data;
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_BOOL, &data.value.boolean );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_FLOAT32, &data.value.float32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_FLOAT64, &data.value.float64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_INT8, &data.value.int8 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_INT16, &data.value.int16 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_INT32, &data.value.int32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_INT64, &data.value.int64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_RAW, &data.value.raw );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_RAW, &data.value.string );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_UINT8, &data.value.uint8 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_UINT16, &data.value.uint16 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_UINT32, &data.value.uint32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_get( &lib, NULL, IOT_FALSE,
		IOT_TYPE_UINT64, &data.value.uint64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_get_valid( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt;
	const char *const test_value = "test_data";
	const char *data = NULL;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );

#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_STRING;
	opt.data.heap_storage =
		test_malloc( strlen( test_value ) + 1u );
	strcpy( (char*)opt.data.heap_storage, test_value );
	opt.data.has_value = IOT_TRUE;
	opt.data.value.string =
		(const char*)opt.data.heap_storage;

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt;
	opts.option_count = 1u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_get( &lib, "opt_name", IOT_FALSE,
		IOT_TYPE_STRING, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( data );
	assert_string_equal( data, test_value );

	/* clean up */
	os_free( opt.data.heap_storage );
}

static void test_iot_config_get_valid_convert_int32( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt;
	iot_int32_t test_value = 32;
	struct iot_data data;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_INT32;
	opt.data.has_value = IOT_TRUE;
	opt.data.value.int32 = test_value;

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt;
	opts.option_count = 1u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_UINT8, &data.value.uint8 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.uint8, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_UINT16, &data.value.uint16 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.uint16, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_UINT32, &data.value.uint32 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.uint32, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_UINT64, &data.value.uint64 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.uint64, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_INT8, &data.value.int8 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.int8, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_INT16, &data.value.int16 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.int16, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_INT32, &data.value.int32 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.int32, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_INT64, &data.value.int64 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data.value.int64, test_value );

	result = iot_config_get( &lib, "opt_name", IOT_TRUE,
		IOT_TYPE_NULL, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_config_get_wrong_type( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt;
	const char *data = NULL;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_INT32;
	opt.data.has_value = IOT_TRUE;
	opt.data.value.int32 = 32;

	/* setup option */
	opts.lib = &lib;
	opts.option_count = 1u;
	opts.option = &opt;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_get( &lib, "opt_name", IOT_FALSE,
		IOT_TYPE_STRING, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_null( data );
}


/* iot_config_get_raw */
static void test_iot_config_get_raw_not_found( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt;
	const void *data = NULL;
	size_t length = 0u;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_RAW;
	opt.data.heap_storage = test_malloc( 1u );
	opt.data.has_value = IOT_TRUE;
	opt.data.value.raw.ptr =
		opt.data.heap_storage;
	opt.data.value.raw.length = 1u;

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt;
	opts.option_count = 1u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_get_raw( &lib, "bad_name",
		IOT_FALSE, &length, &data );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );

	/* clean up */
	test_free( opt.data.heap_storage );
}

static void test_iot_config_get_raw_null_lib( void **state )
{
	const void *data = NULL;
	size_t length = 0u;
	iot_status_t result;

	result = iot_config_get_raw( NULL, "opt_name",
		IOT_FALSE, &length, &data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_get_raw_null_name( void **state )
{
	const void *data = NULL;
	size_t length = 0u;
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_config_get_raw( &lib, NULL,
		IOT_FALSE, &length, &data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_get_raw_null_data( void **state )
{
	size_t length = 0u;
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_config_get_raw( &lib, "opt_name",
		IOT_FALSE, &length, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_get_raw_valid( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt;
	const void *data = NULL;
	size_t length = 0u;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_RAW;
	opt.data.heap_storage = test_malloc( 1u );
	opt.data.has_value = IOT_TRUE;
	opt.data.value.raw.ptr = opt.data.heap_storage;
	opt.data.value.raw.length = 1u;

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt;
	opts.option_count = 1u;

	/* setup options */
	opts.lib = &lib;
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_get_raw( &lib, "opt_name",
		IOT_FALSE, &length, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( length, 1u );
	assert_ptr_equal( data, opt.data.value.raw.ptr );

	/* clean up */
	test_free( opt.data.heap_storage );
}

static void test_iot_config_get_raw_wrong_type( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt;
	const void *data = NULL;
	size_t length = 0u;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_INT32;
	opt.data.has_value = IOT_TRUE;
	opt.data.value.int32 = 32;

	/* setup option */
	opts.lib = &lib;
	opts.option_count = 1u;
	opts.option = &opt;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_get_raw( &lib, "opt_name",
		IOT_FALSE, &length, &data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_null( data );
	assert_int_equal( length, 0u );
}

/* iot_config_set */
static void test_iot_config_set_full( void **state )
{
#ifndef IOT_STACK_ONLY
	char name[ IOT_OPTION_MAX ][ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot_option opt[ IOT_OPTION_MAX ];
	unsigned int i;
	struct iot lib;
	struct iot_data_raw raw_data;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;
	const char *str_data = "test_string";

	raw_data.ptr = str_data;
	raw_data.length = strlen( str_data );

	bzero( &lib, sizeof( struct iot ) );

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt[0];
	opts.option_count = 0u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
	{
		char opt_name[25u];
		snprintf( opt_name, 25u, "opt-%d", i );
		++opts.option_count;
#ifndef IOT_STACK_ONLY
		opts.option[i].name = name[i];
#endif
		strncpy( opts.option[i].name, opt_name, IOT_NAME_MAX_LEN );
	}

	result = iot_config_set( &lib, "new_opt_bool",
		IOT_TYPE_BOOL, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_float32",
		IOT_TYPE_FLOAT32, 3.2 );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_float64",
		IOT_TYPE_FLOAT64, 0.000064 );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_int8",
		IOT_TYPE_INT8, 8 );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_int16",
		IOT_TYPE_INT16, 16 );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_int32",
		IOT_TYPE_INT32, 32 );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_int64",
		IOT_TYPE_INT64, 64 );
	assert_int_equal( result, IOT_STATUS_FULL );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* temp obj */
#endif
	result = iot_config_set( &lib, "new_opt_raw",
		IOT_TYPE_RAW, &raw_data );
	assert_int_equal( result, IOT_STATUS_FULL );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* temp obj */
#endif
	result = iot_config_set( &lib, "new_opt_string",
		IOT_TYPE_STRING, str_data );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_uint8",
		IOT_TYPE_UINT8, 8u );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_uint16",
		IOT_TYPE_UINT16, 16u );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_uint32",
		IOT_TYPE_UINT32, 32u );
	assert_int_equal( result, IOT_STATUS_FULL );
	result = iot_config_set( &lib, "new_opt_uint64",
		IOT_TYPE_UINT64, 64u );
	assert_int_equal( result, IOT_STATUS_FULL );
}

static void test_iot_config_set_null_lib( void **state )
{
	struct iot_data_raw raw_data;
	iot_status_t result;
	const char *str_data = "test_string";

	raw_data.ptr = str_data;
	raw_data.length = strlen( str_data );

	result = iot_config_set( NULL, "new_opt_bool",
		IOT_TYPE_BOOL, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_float32",
		IOT_TYPE_FLOAT32, 3.2 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_float64",
		IOT_TYPE_FLOAT64, 0.000064 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_int8",
		IOT_TYPE_INT8, 8 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_int16",
		IOT_TYPE_INT16, 16 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_int32",
		IOT_TYPE_INT32, 32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_int64",
		IOT_TYPE_INT64, 64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_raw",
		IOT_TYPE_RAW, &raw_data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_string",
		IOT_TYPE_STRING, str_data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_uint8",
		IOT_TYPE_UINT8, 8u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_uint16",
		IOT_TYPE_UINT16, 16u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_uint32",
		IOT_TYPE_UINT32, 32u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( NULL, "new_opt_uint64",
		IOT_TYPE_UINT64, 64u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_set_null_name( void **state )
{
	const char *data = "test_string";
	struct iot lib;
	struct iot_data_raw raw_data;
	iot_status_t result;

	raw_data.ptr = data;
	raw_data.length = strlen( data );

	bzero( &lib, sizeof( struct iot ) );
	result = iot_config_set( &lib, NULL, IOT_TYPE_BOOL, IOT_FALSE );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_FLOAT32, 3.2 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_FLOAT64, 0.000064 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_INT8, 8 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_INT16, 16 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_INT32, 32 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_INT64, 64 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_RAW, &raw_data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_STRING, data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_UINT8, 8u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_UINT16, 16u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_UINT32, 32u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	result = iot_config_set( &lib, NULL, IOT_TYPE_UINT64, 64u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_set_null_data( void **state )
{
	struct iot lib;
	iot_status_t result;

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* new options list object */
	will_return( __wrap_os_realloc, 1 ); /* extend array holding options lists */
	will_return( __wrap_os_realloc, 1 ); /* extend options list for new option */
	will_return( __wrap_os_malloc, 1 ); /* option name */
#endif

	/* add raw option item */
	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* option array */
	will_return( __wrap_os_malloc, 1 ); /* option name */
#endif
	result = iot_config_set( &lib, "raw", IOT_TYPE_RAW, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option_count, 1 );
	assert_string_equal( lib.options_config->option[0].name, "raw" );
	assert_null( lib.options_config->option[0].data.value.raw.ptr );
	assert_int_equal( lib.options_config->option[0].data.has_value, IOT_FALSE );

	/* add string option item */
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* option array */
	will_return( __wrap_os_malloc, 1 ); /* option name */
#endif
	result = iot_config_set( &lib, "string", IOT_TYPE_STRING, NULL );
	assert_non_null( lib.options_config );
#ifdef IOT_STACK_ONLY
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_int_equal( lib.options_config->option_count, 1 );
	assert_string_equal( lib.options_config->option[0].name, "raw" );
#else
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.options_config->option_count, 2 );
	assert_string_equal( lib.options_config->option[0].name, "raw" );
	assert_string_equal( lib.options_config->option[1].name, "string" );
	assert_non_null( lib.options_config->option[1].data.value.string );
	assert_string_equal( lib.options_config->option[1].data.value.string, "" );
	assert_int_equal( lib.options_config->option[1].data.has_value, IOT_TRUE );
#endif

	/* add another raw option item */
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* option array */
	will_return( __wrap_os_malloc, 1 ); /* option name */
#endif
	result = iot_config_set( &lib, "opt_raw", IOT_TYPE_RAW, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
#ifdef IOT_STACK_ONLY
	assert_int_equal( lib.options_config->option_count, 2 );
	assert_string_equal( lib.options_config->option[0].name, "opt_raw" );
	assert_string_equal( lib.options_config->option[1].name, "raw" );
#else
	assert_int_equal( lib.options_config->option_count, 3 );
	assert_string_equal( lib.options_config->option[0].name, "opt_raw" );
	assert_string_equal( lib.options_config->option[1].name, "raw" );
	assert_string_equal( lib.options_config->option[2].name, "string" );
#endif
	assert_null( lib.options_config->option[0].data.value.raw.ptr );
	assert_true( lib.options_config->option[0].data.has_value == IOT_FALSE );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( lib.options[0]->option[2].data.heap_storage );
	os_free( lib.options[0]->option[2].name );
	os_free( lib.options[0]->option[1].name );
	os_free( lib.options[0]->option[0].name );

	os_free( lib.options[0]->option );
	os_free( lib.options[0] );
	os_free( lib.options );
#endif
}

static void test_iot_config_set_overwrite( void **state )
{
	struct iot_option opt;
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	const char *data = "test_string";
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	struct iot_data_raw raw_data;
	iot_status_t result;

	raw_data.ptr = data;
	raw_data.length = strlen( data );

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_RAW;
	will_return( __wrap_os_malloc, 1 );
	opt.data.heap_storage = os_malloc( 1u );
	opt.data.has_value = IOT_TRUE;
	opt.data.value.raw.ptr = opt.data.heap_storage;
	opt.data.value.raw.length = 1u;

	/* setup option */
	opts.lib = &lib;
	opts.option_count = 1u;
	opts.option = &opt;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_BOOL, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_BOOL );
	assert_int_equal( lib.options_config->option[0].data.value.boolean, IOT_TRUE );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_FLOAT32, 3.2 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_FLOAT32 );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_FLOAT64, 0.000064 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_FLOAT64 );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_INT8, 8 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_INT8 );
	assert_int_equal( lib.options_config->option[0].data.value.int8, 8 );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_INT16, 16 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_INT16 );
	assert_int_equal( lib.options_config->option[0].data.value.int16, 16 );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_INT32, 32 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_INT32 );
	assert_int_equal( lib.options_config->option[0].data.value.int32, 32 );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_INT64, (iot_int64_t)64 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_INT64 );
	assert_int_equal( lib.options_config->option[0].data.value.int64, 64 );

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 );
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_RAW, &raw_data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_RAW );

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 );
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_STRING, data );
#ifdef IOT_STACK_ONLY
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
#else
	assert_int_equal( result, IOT_STATUS_SUCCESS );
#endif
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_STRING );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_UINT8, 8u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_UINT8 );
	assert_int_equal( lib.options_config->option[0].data.value.uint8, 8u );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_UINT16, 16u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_UINT16 );
	assert_int_equal( lib.options_config->option[0].data.value.uint16, 16u );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_UINT32, 32u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_UINT32 );
	assert_int_equal( lib.options_config->option[0].data.value.uint32, 32u );

	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_UINT64, (iot_uint64_t)64u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_UINT64 );
	assert_int_equal( lib.options_config->option[0].data.value.uint64, 64u );
}

static void test_iot_config_set_valid( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* new options list object */
	will_return( __wrap_os_realloc, 1 ); /* extend array holding options lists */
	will_return( __wrap_os_realloc, 1 ); /* extend options list for new option */
	will_return( __wrap_os_malloc, 1 ); /* option name */
#endif
	result = iot_config_set( &lib, "opt_name",
		IOT_TYPE_INT64, (iot_int64_t)64 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_string_equal( lib.options_config->option[0].name, "opt_name" );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_INT64 );
	assert_int_equal( lib.options_config->option[0].data.value.int64, 64 );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( lib.options[0]->option[0].name );
	os_free( lib.options[0]->option );
	os_free( lib.options[0] );
	os_free( lib.options );
#endif
}

/* iot_config_set_raw */
static void test_iot_config_set_raw_full( void **state )
{
	struct iot_option opt[ IOT_OPTION_MAX ];
#ifndef IOT_STACK_ONLY
	char name[ IOT_OPTION_MAX ][ IOT_NAME_MAX_LEN + 1u ];
#endif
	unsigned int i;
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt[0];
	opts.option_count = 0u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
	{
		char opt_name[25u];
		snprintf( opt_name, 25u, "opt-%d", i );
		++opts.option_count;
#ifndef IOT_STACK_ONLY
		opt[i].name = name[i];
#endif
		strncpy( opt[i].name, opt_name,
			IOT_NAME_MAX_LEN );
	}
	result = iot_config_set_raw( &lib, "new_opt", 0u, NULL );
	assert_int_equal( result, IOT_STATUS_FULL );
}

static void test_iot_config_set_raw_null_lib( void **state )
{
	const char *data = "raw_data";
	iot_status_t result;

	result = iot_config_set_raw( NULL, "opt_name",
		strlen( data ), data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_set_raw_null_name( void **state )
{
	const char *data = "raw_data";
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_config_set_raw( &lib, NULL,
		strlen( data ), data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_config_set_raw_null_data( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* new options list object */
	will_return( __wrap_os_realloc, 1 ); /* extend array holding options lists */
	will_return( __wrap_os_realloc, 1 ); /* extend option list */
	will_return( __wrap_os_malloc, 1 ); /* option name */
#endif

	result = iot_config_set_raw( &lib, "opt_name", 0u, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( lib.options_config );
	assert_null( lib.options_config->option[0].data.value.raw.ptr );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( lib.options[0]->option[0].name );
	os_free( lib.options[0]->option );
	os_free( lib.options[0] );
	os_free( lib.options );
#endif
}

static void test_iot_config_set_raw_overwrite_data( void **state )
{
	struct iot_option opt;
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	const char *data = "raw_data";
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_RAW;
	will_return( __wrap_os_malloc, 1 );
	opt.data.heap_storage = os_malloc( 1u );
	opt.data.has_value = IOT_TRUE;
	opt.data.value.raw.ptr = opt.data.heap_storage;
	opt.data.value.raw.length = 1u;

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt;
	opts.option_count = 1u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* option value */
#endif
	result = iot_config_set_raw( &lib, "opt_name",
		strlen( data ), &data );
#ifdef IOT_STACK_ONLY
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_RAW );
	assert_null( lib.options_config->option[0].data.value.raw.ptr );
	assert_int_equal( lib.options_config->option[0].data.has_value, IOT_FALSE );
#else
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.options_config->option[0].data.type, IOT_TYPE_RAW );
	assert_non_null( lib.options_config->option[0].data.value.raw.ptr );
	assert_int_equal( lib.options_config->option[0].data.has_value, IOT_TRUE );
#endif

	/* clean up */
	os_free( opt.data.heap_storage );
}

static void test_iot_config_set_raw_overwrite_null( void **state )
{
	struct iot_option opt;
#ifndef IOT_STACK_ONLY
	char name[ IOT_NAME_MAX_LEN + 1u ];
#endif
	struct iot lib;
	struct iot_options opts;
	struct iot_options *opts_arr[1];
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	opt.name = name;
#endif
	strncpy( opt.name, "opt_name", IOT_NAME_MAX_LEN );
	opt.data.type = IOT_TYPE_RAW;
	will_return( __wrap_os_malloc, 1 );
	opt.data.heap_storage = os_malloc( 1u );
	opt.data.has_value = IOT_TRUE;
	opt.data.value.raw.ptr = opt.data.heap_storage;
	opt.data.value.raw.length = 1u;

	/* setup option */
	opts.lib = &lib;
	opts.option = &opt;
	opts.option_count = 1u;

	/* setup options */
	opts_arr[0] = &opts;
	lib.options = &opts_arr[0];
	lib.options_count = 1u;
	lib.options_config = &opts;

	result = iot_config_set_raw( &lib, "opt_name", 0u, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( lib.options_config->option[0].data.type == IOT_TYPE_RAW );
	assert_null( lib.options_config->option[0].data.value.raw.ptr );
	assert_true( lib.options_config->option[0].data.has_value == IOT_FALSE );
}

static void test_iot_config_set_raw_valid( void **state )
{
	const char *data = "raw_data";
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 ); /* new options list object */
	will_return( __wrap_os_realloc, 1 ); /* extend array holding options lists */
	will_return( __wrap_os_realloc, 1 ); /* extend options list for new option */
	will_return( __wrap_os_malloc, 1 ); /* option name */
	will_return( __wrap_os_malloc, 1 ); /* option value */
#endif
	result = iot_config_set_raw( &lib, "opt_name",
		strlen( data ), &data );
#ifdef IOT_STACK_ONLY
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_int_equal( lib.options_config->option_count, 0u );
#else
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.options_config->option_count, 1u );
#endif
	assert_non_null( lib.options );
	assert_non_null( lib.options_config );
	assert_int_equal( lib.options_count, 1u );
	assert_true( lib.options_config->option[0].data.type == IOT_TYPE_RAW );
#ifdef IOT_STACK_ONLY
	assert_null( lib.options_config->option[0].data.value.raw.ptr );
	assert_int_equal( lib.options_config->option[0].data.has_value, IOT_FALSE );
#else
	assert_non_null( lib.options_config->option[0].data.value.raw.ptr );
	assert_int_equal( lib.options_config->option[0].data.has_value, IOT_TRUE );
#endif

#ifndef IOT_STACK_ONLY
	/* clean up */
	os_free( lib.options_config->option[0].data.heap_storage );
	os_free( lib.options_config->option[0].name );
	os_free( lib.options_config->option );
	os_free( lib.options_config );
	os_free( lib.options );
#endif
}

/* iot_configuration_file_set */
static void test_iot_configuration_file_set_null_lib( void **state )
{
	iot_status_t result;
	result = iot_configuration_file_set( NULL, "some/path/to/file.cfg" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_configuration_file_set_null_path( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_configuration_file_set( &lib, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_configuration_file_set_no_memory( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 0u );
#endif
	result = iot_configuration_file_set( &lib, "some/path/to/file.cfg" );

#ifdef IOT_STACK_ONLY
	assert_int_equal( result, IOT_STATUS_SUCCESS );
#else
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
#endif
}

static void test_iot_configuration_file_set_valid( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1u );
#endif
	result = iot_configuration_file_set( &lib, "some/path/to/file.cfg" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

#ifndef IOT_STACK_ONLY
	os_free( lib.cfg_file_path );
#endif
}

/* iot_connect */
static void test_iot_connect_configuration_fail_to_parse( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.flags = IOT_FLAG_SINGLE_THREAD;
	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_TRUE );
	will_return( __wrap_os_file_open, 1u );
#ifdef IOT_STACK_ONLY
	will_return( __wrap_os_file_read, 1u );
#else
	will_return_count( __wrap_os_realloc, 1u, 3u );
	will_return_count( __wrap_os_file_read, 1u, 2u );
#endif
	will_return( __wrap_os_file_read, 0u );
	will_return( __wrap_os_file_eof, OS_TRUE );
	/* fail to parse configuration file */
	will_return( __wrap_iot_json_decode_initialize, 0 );

	result = iot_connect( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_PARSE_ERROR );
}

static void test_iot_connect_configuration_fail_to_read( void **state )
{
	struct iot lib;
	iot_status_t result;
	struct iot_option opt;
	struct iot_options opts;

	bzero( &lib, sizeof( struct iot ) );
	bzero( &opt, sizeof( struct iot_option ) );
	bzero( &opts, sizeof( struct iot_options ) );
	lib.id = test_malloc( 5u );
	assert_non_null( lib.id );
	strncpy( lib.id, "test", 5u );

#ifndef IOT_STACK_ONLY
	opt.name = test_malloc( 10u );
#endif /* ifndef IOT_STACK_ONLY */
	assert_non_null( opt.name );
	strncpy( opt.name, "log_level", 10u );
	opt.data.type = IOT_TYPE_STRING;
	opt.data.has_value = IOT_TRUE;
	opt.data.value.string = "INFO";
	opts.option = &opt;
	opts.option_count = 1u;
	lib.options_config = &opts;

	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* app_id.cfg */
	will_return( __wrap_os_file_exists, OS_TRUE );
	will_return( __wrap_os_file_open, 1u );
#ifdef IOT_STACK_ONLY
	will_return( __wrap_os_file_read, 1u );
#else
	will_return_count( __wrap_os_realloc, 1u, 3u );
	will_return_count( __wrap_os_file_read, 1u, 2u );
#endif
	will_return( __wrap_os_file_read, 0u );
	will_return( __wrap_os_file_eof, OS_FALSE );

	result = iot_connect( &lib, 100u );
	assert_int_equal( result, IOT_STATUS_FAILURE );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( opt.name );
#endif /* ifndef IOT_STACK_ONLY */
	test_free( lib.id );
}

static void test_iot_connect_configuration_no_memory( void **state )
{
	struct iot lib;
	iot_status_t result;
	struct iot_option opt;
	struct iot_options opts;

	bzero( &lib, sizeof( struct iot ) );
	bzero( &opt, sizeof( struct iot_option ) );
	bzero( &opts, sizeof( struct iot_options ) );
	lib.id = test_malloc( 5u );
	assert_non_null( lib.id );
	strncpy( lib.id, "test", 5u );

#ifndef IOT_STACK_ONLY
	opt.name = test_malloc( 10u );
#endif /* ifndef IOT_STACK_ONLY */
	assert_non_null( opt.name );
	strncpy( opt.name, "log_level", 10u );
	opt.data.type = IOT_TYPE_STRING;
	opt.data.has_value = IOT_TRUE;
	opt.data.value.string = "INFO";
	opts.option = &opt;
	opts.option_count = 1u;
	lib.options_config = &opts;

	/* iot-connect.cfg */
#ifdef IOT_STACK_ONLY
	will_return( __wrap_os_file_exists, OS_TRUE );
	will_return( __wrap_os_file_open, 1u );
	will_return_count( __wrap_os_file_read, 1u, 2u );
#else
	will_return( __wrap_os_file_exists, OS_TRUE );
	will_return( __wrap_os_file_open, 1u );
	will_return( __wrap_os_realloc, 0u );
#endif

	result = iot_connect( &lib, 100u );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( opt.name );
#endif /* ifndef IOT_STACK_ONLY */
	test_free( lib.id );
}

static void test_iot_connect_configuration_not_found( void **state )
{
#ifdef IOT_THREAD_SUPPORT
	unsigned int i;
#endif /* ifdef IOT_THREAD_SUPPORT */
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );

	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* app_id.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* client connect */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	/* loop start */
#ifdef IOT_THREAD_SUPPORT
	/* main thread */
	will_return( __wrap_os_thread_create, OS_STATUS_SUCCESS );
	/* loop iteration */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	will_return_always( __wrap_iot_action_process, IOT_STATUS_FAILURE );
	for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
		will_return( __wrap_os_thread_create, OS_STATUS_SUCCESS );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_connect( &lib, 100u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_connect_null_lib( void **state )
{
	iot_status_t result;
	result = iot_connect( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_connect_plugin_connect_fail( void **state )
{
	struct iot lib;
	iot_status_t result;
	struct iot_option opt;
	struct iot_options opts;

	bzero( &lib, sizeof( struct iot ) );
	bzero( &opt, sizeof( struct iot_option ) );
	bzero( &opts, sizeof( struct iot_options ) );
	lib.id = test_malloc( 5u );
	assert_non_null( lib.id );
	strncpy( lib.id, "test", 5u );

#ifndef IOT_STACK_ONLY
	opt.name = test_malloc( 10u );
#endif /* ifndef IOT_STACK_ONLY */
	assert_non_null( opt.name );
	strncpy( opt.name, "log_level", 10u );
	opt.data.type = IOT_TYPE_STRING;
	opt.data.has_value = IOT_TRUE;
	opt.data.value.string = "INFO";
	opts.option = &opt;
	opts.option_count = 1u;
	lib.options_config = &opts;

	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* app_id.cfg */
	will_return( __wrap_os_file_exists, OS_TRUE );
	will_return( __wrap_os_file_open, 1u );
#ifdef IOT_STACK_ONLY
	will_return( __wrap_os_file_read, 1u );
#else
	will_return_count( __wrap_os_realloc, 1u, 3u );
	will_return_count( __wrap_os_file_read, 1u, 2u );
#endif
	will_return( __wrap_os_file_read, 0u );
	will_return( __wrap_os_file_eof, IOT_TRUE );
	/* parse file */
	will_return( __wrap_iot_json_decode_initialize, 0x1 );
	will_return( __wrap_iot_json_decode_object_iterator_key, NULL );
	will_return( __wrap_iot_json_decode_type, IOT_JSON_TYPE_NULL );
	will_return( __wrap_iot_json_decode_object_iterator_next, NULL );

	/* client connect */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );

	result = iot_connect( &lib, 100u );
	assert_int_equal( result, IOT_STATUS_FAILURE );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( opt.name );
#endif /* ifndef IOT_STACK_ONLY */
	test_free( lib.id );
}

static void test_iot_connect_single_thread( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.flags = IOT_FLAG_SINGLE_THREAD;
	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* app_id.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );

	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_connect( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_connect_threads_fail( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* app_id.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );

	/* connect */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
#ifdef IOT_THREAD_SUPPORT
	will_return( __wrap_os_thread_create, OS_STATUS_FAILURE );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_connect( &lib, 0u );

#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_FAILURE );
#else
	assert_int_equal( result, IOT_STATUS_SUCCESS );
#endif /* ifdef IOT_THREAD_SUPPORT */
}

static void test_iot_connect_threads_main_loop_fail( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.cfg_file_path = test_malloc( 50u );
	assert_non_null( lib.cfg_file_path );
	strncpy( lib.cfg_file_path, "/explicit/path/to/file.cfg", 50u );

	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* app_id.cfg */
	will_return( __wrap_os_file_exists, OS_TRUE );
	will_return( __wrap_os_file_open, 1u );
#ifdef IOT_STACK_ONLY
	will_return( __wrap_os_file_read, 1u );
#else
	will_return_count( __wrap_os_realloc, 1u, 3u );
	will_return_count( __wrap_os_file_read, 1u, 2u );
#endif
	will_return( __wrap_os_file_read, 0u );
	will_return( __wrap_os_file_eof, OS_TRUE );

	/* read configuration file */
	will_return( __wrap_iot_json_decode_initialize, 0x1 );
	will_return( __wrap_iot_json_decode_object_iterator_key, "log_level" );
	will_return( __wrap_iot_json_decode_type, IOT_JSON_TYPE_STRING );
	will_return( __wrap_iot_json_decode_string, "INFO" );
	will_return( __wrap_os_malloc, 1u ); /* allocate global options array */
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1u ); /* resize pointer to option arrays */
	will_return( __wrap_os_realloc, 1u ); /* add to global options array */
	will_return( __wrap_os_malloc, 1u ); /* allocate key for option */
	will_return( __wrap_os_realloc, 1u ); /* allocate data storage */
	will_return( __wrap_os_malloc, 1u ); /* allocate data storage */
	will_return( __wrap_os_malloc, 1u ); /* allocate data storage */
#endif /* ifndef IOT_STACK_ONLY */
	will_return( __wrap_iot_json_decode_object_iterator_next, 1u );
	will_return( __wrap_iot_json_decode_object_iterator_key, "int_value" );
	will_return( __wrap_iot_json_decode_type, IOT_JSON_TYPE_INTEGER );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1u ); /* add to global options array */
	will_return( __wrap_os_malloc, 1u ); /* allocate key for option */
#endif /* ifndef IOT_STACK_ONLY */
	will_return( __wrap_iot_json_decode_object_iterator_next, 1u );
	will_return( __wrap_iot_json_decode_object_iterator_key, "bool_value" );
	will_return( __wrap_iot_json_decode_type, IOT_JSON_TYPE_BOOL );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1u ); /* add to global options array */
	will_return( __wrap_os_malloc, 1u ); /* allocate key for option */
#endif /* ifndef IOT_STACK_ONLY */
	will_return( __wrap_iot_json_decode_object_iterator_next, 1u );
	will_return( __wrap_iot_json_decode_object_iterator_key, "real_value" );
	will_return( __wrap_iot_json_decode_type, IOT_JSON_TYPE_REAL );
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1u ); /* add to global options array */
	will_return( __wrap_os_malloc, 1u ); /* allocate key for option */
#endif /* ifndef IOT_STACK_ONLY */
	will_return( __wrap_iot_json_decode_object_iterator_next, 1u );
	will_return( __wrap_iot_json_decode_object_iterator_key, "object" );
	will_return( __wrap_iot_json_decode_type, IOT_JSON_TYPE_OBJECT );
	will_return( __wrap_iot_json_decode_object_iterator_key, "item1" );
	will_return( __wrap_iot_json_decode_type, IOT_JSON_TYPE_ARRAY );
	will_return( __wrap_iot_json_decode_object_iterator_next, NULL );
	will_return( __wrap_iot_json_decode_object_iterator_next, NULL );

	/* client connect */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	/* loop start */
#ifdef IOT_THREAD_SUPPORT
	/* main thread */
	will_return( __wrap_os_thread_create, OS_STATUS_FAILURE );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_connect( &lib, 100u );
#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_FAILURE );
#else
	assert_int_equal( result, IOT_STATUS_SUCCESS );
#endif /* ifdef IOT_THREAD_SUPPORT */

	/* clean up */
#ifndef IOT_STACK_ONLY
	if ( lib.options )
	{
		size_t i;
		for ( i = 0u; i < lib.options_count; ++i )
		{
			if ( lib.options[i] )
			{
				size_t j;
				for ( j = 0u; j < (lib.options[i])->option_count; ++j )
				{
					struct iot_option *o = &(lib.options[i])->option[j];
					if ( o )
					{
						os_free( o->name );
						if ( o->data.heap_storage )
							os_free( o->data.heap_storage );
					}
				}
				os_free( lib.options[i]->option );
				os_free( lib.options[i] );
			}
		}
		os_free( lib.options );
	}
#endif /* ifndef IOT_STACK_ONLY */
	test_free( lib.cfg_file_path );
}

static void test_iot_connect_threads_success( void **state )
{
#ifdef IOT_THREAD_SUPPORT
	unsigned int i;
#endif /* ifdef IOT_THREAD_SUPPORT */
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );

	/* iot-connect.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* app_id.cfg */
	will_return( __wrap_os_file_exists, OS_FALSE );
	/* client connect */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
#ifdef IOT_THREAD_SUPPORT
	/* loop start */
	/* main thread */
	will_return( __wrap_os_thread_create, OS_STATUS_SUCCESS );
	/* loop iteration */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	will_return_always( __wrap_iot_action_process, IOT_STATUS_FAILURE );
	for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
		will_return( __wrap_os_thread_create, OS_STATUS_SUCCESS );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_connect( &lib, 100u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

/* iot_directory_name_get */
static void test_iot_directory_name_get_bad_type( void **state )
{
	char buf[ 125u ];
	size_t result;

	result = iot_directory_name_get( (iot_dir_type_t)100010, buf, sizeof( buf ) );
	assert_int_equal( result, 0u );
}

static void test_iot_directory_name_get_null_dest( void **state )
{
	const char *const dir_cfg = IOT_DEFAULT_DIR_CONFIG;
	size_t result;

	assert_non_null( dir_cfg );
	result = iot_directory_name_get( IOT_DIR_CONFIG, NULL, 0u );
	assert_int_equal( result, strlen( dir_cfg ) );
}

static void test_iot_directory_name_get_small_dest( void **state )
{
	char buf[5u];
	const char *const dir_cfg = IOT_DEFAULT_DIR_CONFIG;
	size_t result;

	assert_non_null( dir_cfg );
	assert_true( sizeof( buf ) < strlen( dir_cfg ) );
	result = iot_directory_name_get( IOT_DIR_CONFIG, buf, sizeof( buf ) );
	assert_int_equal( result, 0u );
}

static void test_iot_directory_name_get_valid_config_dir( void **state )
{
	char buf[125u];
	const char *const dir_cfg = IOT_DEFAULT_DIR_CONFIG;
	size_t result;

	assert_non_null( dir_cfg );
	assert_true( sizeof( buf ) > strlen( dir_cfg ) );
	result = iot_directory_name_get( IOT_DIR_CONFIG, buf, sizeof( buf ) );
	assert_int_equal( result, strlen( dir_cfg ) );
	assert_string_equal( buf, dir_cfg );
}

static void test_iot_directory_name_get_valid_runtime_dir( void **state )
{
	char buf[125u];
	const char *const dir_cfg = IOT_DEFAULT_DIR_RUNTIME;
	size_t result;

	assert_non_null( dir_cfg );
	assert_true( sizeof( buf ) > strlen( dir_cfg ) );
	result = iot_directory_name_get( IOT_DIR_RUNTIME, buf, sizeof( buf ) );
	assert_int_equal( result, strlen( dir_cfg ) );
	assert_string_equal( buf, dir_cfg );
}

/* iot_disconnect */
static void test_iot_disconnect_null_lib( void **state )
{
	iot_status_t result;
	result = iot_disconnect( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_disconnect_single_thread( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.flags = IOT_FLAG_SINGLE_THREAD;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_disconnect( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_disconnect_valid( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_disconnect( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

/* iot_error */
static void test_iot_error_unknown( void **state )
{
	const char *result;

	result = iot_error( ( iot_status_t )( IOT_STATUS_FAILURE + 1 ) );
	assert_string_equal( result, "unknown error" );
}

static void test_iot_error_valid( void **state )
{
	const char *result;
	unsigned int i;

	struct iot_status_msg
	{
		iot_status_t id; /**< @brief status id */
		const char *msg; /**< @brief status message */
	};

	struct iot_status_msg messages[] = { { IOT_STATUS_SUCCESS, "success" },
		                                 { IOT_STATUS_INVOKED, "invoked" },
		                                 { IOT_STATUS_BAD_PARAMETER, "invalid parameter" },
		                                 { IOT_STATUS_BAD_REQUEST, "bad request" },
		                                 { IOT_STATUS_EXECUTION_ERROR, "execution error" },
		                                 { IOT_STATUS_EXISTS, "already exists" },
		                                 { IOT_STATUS_FILE_OPEN_FAILED, "file open failed" },
		                                 { IOT_STATUS_FULL, "storage is full" },
		                                 { IOT_STATUS_IO_ERROR, "input/output error" },
		                                 { IOT_STATUS_NO_MEMORY, "out of memory" },
		                                 { IOT_STATUS_NOT_EXECUTABLE, "not executable" },
		                                 { IOT_STATUS_NOT_FOUND, "not found" },
		                                 { IOT_STATUS_NOT_INITIALIZED, "not initialized" },
		                                 { IOT_STATUS_NOT_SUPPORTED, "not supported" },
		                                 { IOT_STATUS_PARSE_ERROR, "error parsing message" },
		                                 { IOT_STATUS_TIMED_OUT, "timed out" },
		                                 { IOT_STATUS_TRY_AGAIN, "try again" },

		                                 /* should be the last option */
		                                 { IOT_STATUS_FAILURE, "internal error" } };

	for ( i = 0u; i < sizeof( messages ) / sizeof( struct iot_status_msg ); ++i )
	{
		struct iot_status_msg status_msg = messages[i];
		result = iot_error( status_msg.id );
		assert_string_equal( result, status_msg.msg );
	}
}

/* iot_id */
static void test_iot_id_null_lib( void **state )
{
	const char *result;
	result = iot_id( NULL );
	assert_null( result );
}

static void test_iot_id_null_id( void **state )
{
	struct iot lib;
	const char *result;

	bzero( &lib, sizeof( struct iot ) );
	lib.id = NULL;
	result = iot_id( &lib );
	assert_null( result );
}

static void test_iot_id_valid( void **state )
{
	struct iot lib;
	const char *result;

	bzero( &lib, sizeof( struct iot ) );
	lib.id = test_malloc( 25u );
	assert_non_null( lib.id );
	strncpy( lib.id, "some_device_id", 25u );
	result = iot_id( &lib );
	assert_non_null( result );
	assert_string_equal( result, "some_device_id" );

	/* clean up */
	test_free( lib.id );
}

/* iot_initialize */
static void test_iot_initialize_null( void **state )
{
	iot_t *lib = NULL;

	lib = iot_initialize( NULL, NULL, 0u );
	assert_null( lib );
}

static void test_iot_initialize_unable_to_write( void **state )
{
	iot_t *lib = NULL;

#ifndef IOT_STACK_ONLY
	/* library object */
	will_return( __wrap_os_malloc, 1 );
#endif
	/* read device-id file */
	will_return( __wrap_os_file_open, OS_FILE_INVALID );
	/* write device-id file */
	will_return( __wrap_os_file_open, OS_FILE_INVALID );
	lib = iot_initialize( "blah", NULL, 0u );
	assert_null( lib );
}

static void test_iot_initialize_valid_generate_uuid( void **state )
{
	iot_t *lib = NULL;

#ifndef IOT_STACK_ONLY
	/* library object */
	will_return( __wrap_os_malloc, 1 );
#endif
	/* read device-id file */
	will_return( __wrap_os_file_open, OS_FILE_INVALID );
	/* write device-id file */
	will_return( __wrap_os_file_open, 1 );
#ifndef IOT_STACK_ONLY
	/* device id */
	will_return( __wrap_os_realloc, 1 );
#endif /* ifndef IOT_STACK_ONLY */

	/* number of plug-ins */
	will_return( __wrap_iot_plugin_builtin_load, 0u );
	lib = iot_initialize( "blah", NULL, 0u );
	assert_non_null( lib );
	assert_string_equal( lib->id, "blah" );
	assert_int_equal( lib->logger_level, IOT_LOG_INFO );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( lib->device_id );
	os_free( lib );
#endif
}

static void test_iot_initialize_valid_read_uuid( void **state )
{
	iot_t *lib = NULL;

	/* library object */
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1 );
#endif /* ifndef IOT_STACK_ONLY */
	/* read device-id file */
	will_return( __wrap_os_file_open, 1u );
	will_return( __wrap_os_file_read, 1u );
#ifndef IOT_STACK_ONLY
	/* device id */
	will_return( __wrap_os_realloc, 1 );
#endif /* ifndef IOT_STACK_ONLY */
	/* number of plug-ins */
	will_return( __wrap_iot_plugin_builtin_load, 1u );

	lib = iot_initialize( "blah", NULL, 0u );
	assert_non_null( lib );
	assert_string_equal( lib->id, "blah" );
	assert_int_equal( lib->logger_level, IOT_LOG_INFO );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( lib->device_id );
	os_free( lib );
#endif
}

/* iot_log */
static void test_iot_log_invalid_level( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.logger_level = IOT_LOG_ALL;
	result = iot_log( &lib, IOT_LOG_ALL, "func", __FILE__, __LINE__, "invalid log level" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_log_null_callback( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.logger_level = IOT_LOG_ALL;
	result = iot_log( &lib, IOT_LOG_ERROR, "func", __FILE__, __LINE__, "null log callback" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_log_null_lib( void **state )
{
	iot_status_t result;
	result = iot_log( NULL, IOT_LOG_ERROR, "func", __FILE__, __LINE__, "null library" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_log_with_callback( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.logger_level = IOT_LOG_ALL;
	lib.logger = &test_log_callback;
	lib.logger_user_data = &lib;
	expect_value( test_log_callback, log_level, IOT_LOG_ERROR );
	expect_string( test_log_callback, message, "test message #1234" );
	expect_value( test_log_callback, user_data, &lib );
	result = iot_log( &lib, IOT_LOG_ERROR, "func", __FILE__, __LINE__, "test message #%d", 1234 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

/* iot_log_callback_set */
static void test_iot_log_callback_set_null_callback( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.logger = &test_log_callback;
	result = iot_log_callback_set( &lib, NULL, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_null( lib.logger );
}

static void test_iot_log_callback_set_null_lib( void **state )
{
	iot_status_t result;

	result = iot_log_callback_set( NULL, &test_log_callback, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_log_callback_set_valid( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_log_callback_set( &lib, &test_log_callback, &lib );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_ptr_equal( lib.logger, &test_log_callback );
	assert_ptr_equal( lib.logger_user_data, &lib );
}

/* iot_log_level_set */
static void test_iot_log_level_set_invalid( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.logger_level = IOT_LOG_ERROR;
	result = iot_log_level_set( &lib, ( iot_log_level_t )( IOT_LOG_ALL + 1 ) );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( lib.logger_level, IOT_LOG_ERROR );
}

static void test_iot_log_level_set_null_lib( void **state )
{
	iot_status_t result;

	result = iot_log_level_set( NULL, IOT_LOG_ERROR );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_log_level_set_valid( void **state )
{
	unsigned int i;
	struct iot lib;
	iot_status_t result;
	iot_log_level_t log_levels[] = {
		IOT_LOG_FATAL,
		IOT_LOG_ALERT,
		IOT_LOG_CRITICAL,
		IOT_LOG_ERROR,
		IOT_LOG_WARNING,
		IOT_LOG_NOTICE,
		IOT_LOG_INFO,
		IOT_LOG_DEBUG,
		IOT_LOG_TRACE,

		/* should be the last option */
		IOT_LOG_ALL,
	};

	bzero( &lib, sizeof( struct iot ) );
	for ( i = 0u; i < sizeof( log_levels ) / sizeof( iot_log_level_t ); ++i )
	{
		result = iot_log_level_set( &lib, log_levels[i] );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
		assert_int_equal( lib.logger_level, log_levels[i] );
	}
}

/* iot_log_level_set_string */
static void test_iot_log_level_set_string_invalid( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_log_level_set_string( &lib, "error" );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_log_level_set_string_null_lib( void **state )
{
	iot_status_t result;

	result = iot_log_level_set_string( NULL, "ERROR" );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_log_level_set_string_null_str( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_log_level_set_string( &lib, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_log_level_set_string_valid( void **state )
{
	unsigned int i;
	struct iot lib;
	iot_status_t result;

	struct iot_log_level_map
	{
		iot_log_level_t id;
		const char *name;
	};

	struct iot_log_level_map log_levels[] = {
		{ IOT_LOG_FATAL, "FATAL" },
		{ IOT_LOG_ALERT, "ALERT" },
		{ IOT_LOG_CRITICAL, "CRITICAL" },
		{ IOT_LOG_ERROR, "ERROR" },
		{ IOT_LOG_WARNING, "WARNING" },
		{ IOT_LOG_NOTICE, "NOTICE" },
		{ IOT_LOG_INFO, "INFO" },
		{ IOT_LOG_DEBUG, "DEBUG" },
		{ IOT_LOG_TRACE, "TRACE" },

		/* should be the last option */
		{ IOT_LOG_ALL, "ALL" },
	};

	bzero( &lib, sizeof( struct iot ) );
	for ( i = 0u; i < sizeof( log_levels ) / sizeof( struct iot_log_level_map ); ++i )
	{
		struct iot_log_level_map log_level = log_levels[i];
		result = iot_log_level_set_string( &lib, log_level.name );
		assert_int_equal( result, IOT_STATUS_SUCCESS );
		assert_int_equal( lib.logger_level, log_level.id );
	}
}

/* iot_loop_forever */
static void test_iot_loop_forever_null_lib( void **state )
{
	iot_status_t result;
	result = iot_loop_forever( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_loop_forever_single_thread( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.to_quit = IOT_FALSE;
	lib.flags = IOT_FLAG_SINGLE_THREAD;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	will_return_always( __wrap_iot_action_process, IOT_STATUS_FAILURE );
	result = iot_loop_forever( &lib );
	assert_int_equal( result, IOT_STATUS_FAILURE );
}

/* iot_loop_iteration */
static void test_iot_loop_iteration_null_lib( void **state )
{
	iot_status_t result;
	result = iot_loop_iteration( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_loop_iteration_single_thread( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.flags = IOT_FLAG_SINGLE_THREAD;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_action_process, IOT_STATUS_SUCCESS );
	result = iot_loop_iteration( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_loop_iteration_threads( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
#ifndef IOT_THREAD_SUPPORT
	will_return( __wrap_iot_action_process, IOT_STATUS_SUCCESS );
#endif /* ifndef IOT_THREAD_SUPPORT */
	result = iot_loop_iteration( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

/* iot_loop_start */
static void test_iot_loop_start_null_lib( void **state )
{
	iot_status_t result;
	result = iot_loop_start( NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_loop_start_single_thread( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.flags = IOT_FLAG_SINGLE_THREAD;
	lib.to_quit = IOT_TRUE;
	result = iot_loop_start( &lib );
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
	assert_int_equal( lib.to_quit, IOT_FALSE );
}

static void test_iot_loop_start_threads_fail( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.to_quit = IOT_TRUE;
#ifdef IOT_THREAD_SUPPORT
	will_return( __wrap_os_thread_create, IOT_STATUS_FAILURE );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_loop_start( &lib );

#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_FAILURE );
	assert_int_equal( lib.to_quit, IOT_TRUE );
	assert_int_equal( lib.main_thread, 0 );
#else
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
#endif /* ifdef IOT_THREAD_SUPPORT */
}

static void test_iot_loop_start_threads_success( void **state )
{
#ifdef IOT_THREAD_SUPPORT
	unsigned int i;
#endif /* ifdef IOT_THREAD_SUPPORT */
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.to_quit = IOT_TRUE;
#ifdef IOT_THREAD_SUPPORT
	will_return( __wrap_os_thread_create, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
		will_return( __wrap_os_thread_create, IOT_STATUS_SUCCESS );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_loop_start( &lib );

#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.to_quit, IOT_FALSE );
	assert_true( lib.main_thread != 0 );
	for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
		assert_true( lib.worker_thread[i] != 0 );
#else
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
#endif /* ifdef IOT_THREAD_SUPPORT */
}

static void test_iot_loop_start_threads_twice( void **state )
{
#ifdef IOT_THREAD_SUPPORT
	unsigned int i;
#endif /* ifdef IOT_THREAD_SUPPORT */
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.to_quit = IOT_TRUE;
#ifdef IOT_THREAD_SUPPORT
	will_return( __wrap_os_thread_create, IOT_STATUS_SUCCESS );
	for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
		will_return( __wrap_os_thread_create, IOT_STATUS_SUCCESS );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_loop_start( &lib );

#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_true( lib.main_thread != 0 );
#else
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_loop_start( &lib );

#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_SUCCESS );
#else
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
#endif /* ifdef IOT_THREAD_SUPPORT */
}

/* iot_loop_stop */
static void test_iot_loop_stop_null_lib( void **state )
{
	iot_status_t result;
	result = iot_loop_stop( NULL, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_loop_stop_single_thread( void **state )
{
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	lib.flags = IOT_FLAG_SINGLE_THREAD;
	result = iot_loop_stop( &lib, IOT_TRUE );
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
	assert_int_equal( lib.to_quit, IOT_TRUE );
}

static void test_iot_loop_stop_threads_force( void **state )
{
#ifdef IOT_THREAD_SUPPORT
	unsigned int i;
#endif /* ifdef IOT_THREAD_SUPPORT */
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifdef IOT_THREAD_SUPPORT
	lib.main_thread = (os_thread_t)1234;
	for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
		lib.worker_thread[i] = (os_thread_t)(i + 1u);
#endif /* ifdef IOT_THREAD_SUPPORT */
	result = iot_loop_stop( &lib, IOT_TRUE );

#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_SUCCESS );
#else
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
#endif /* ifdef IOT_THREAD_SUPPORT */
	assert_int_equal( lib.to_quit, IOT_TRUE );
}

static void test_iot_loop_stop_threads_no_force( void **state )
{
#ifdef IOT_THREAD_SUPPORT
	unsigned int i;
#endif /* ifdef IOT_THREAD_SUPPORT */
	struct iot lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
#ifdef IOT_THREAD_SUPPORT
	lib.main_thread = (os_thread_t)1234;
	for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
		lib.worker_thread[i] = (os_thread_t)(i + 1u);
#endif /* ifdef IOT_THREAD_SUPPORT */

	result = iot_loop_stop( &lib, IOT_FALSE );
#ifdef IOT_THREAD_SUPPORT
	assert_int_equal( result, IOT_STATUS_SUCCESS );
#else
	assert_int_equal( result, IOT_STATUS_NOT_SUPPORTED );
#endif /* ifdef IOT_THREAD_SUPPORT */
	assert_int_equal( lib.to_quit, IOT_TRUE );
}

/* iot_terminate */
static void test_iot_terminate_action( void **state )
{
	struct iot *lib;
	struct iot_action action;
	iot_status_t result;

	will_return( __wrap_os_malloc, 1 );
	lib = os_malloc( sizeof( struct iot ) );
	bzero( lib, sizeof( struct iot ) );
	bzero( &action, sizeof( struct iot_action ) );
	lib->action_count = 1u;
	lib->action_ptr[0] = &action;

	lib->plugin_count = 2u;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1u );
	lib->cfg_file_path = os_malloc( 10u );
	will_return( __wrap_os_malloc, 1u );
	lib->device_id = os_malloc( 10u );
#endif

#ifndef IOT_STACK_ONLY
	will_return( __wrap_iot_action_free, IOT_STATUS_FAILURE );
#endif /* ifndef IOT_STACK_ONLY */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );

	result = iot_terminate( lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* clean up */
#ifdef IOT_STACK_ONLY
	os_free( lib );
#endif
}

static void test_iot_terminate_alarm( void **state )
{
	struct iot *lib;
	struct iot_alarm alarm;
	iot_status_t result;

	will_return( __wrap_os_malloc, 1 );
	lib = os_malloc( sizeof( struct iot ) );
	bzero( lib, sizeof( struct iot ) );
	bzero( &alarm, sizeof( struct iot_alarm ) );
	lib->alarm_count = 1u;
	lib->alarm_ptr[0] = &alarm;

#ifndef IOT_STACK_ONLY
	will_return( __wrap_iot_alarm_deregister, IOT_STATUS_FAILURE );
#endif /* ifndef IOT_STACK_ONLY */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );

	result = iot_terminate( lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* clean up */
#ifdef IOT_STACK_ONLY
	os_free( lib );
#endif
}
static void test_iot_terminate_blank( void **state )
{
	struct iot *lib;
	iot_status_t result;

	will_return( __wrap_os_malloc, 1u );
	lib = os_malloc( sizeof( struct iot ) );
	bzero( lib, sizeof( struct iot ) );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_terminate( lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* clean up */
#ifdef IOT_STACK_ONLY
	os_free( lib );
#endif
}

static void test_iot_terminate_null_lib( void **state )
{
	iot_status_t result;
	result = iot_terminate( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_terminate_option( void **state )
{
	struct iot *lib;
	iot_status_t result;
	struct iot_option *opt;
	struct iot_options *opts;
	struct iot_options **opts_ptr;

	will_return( __wrap_os_malloc, 1u );
	lib = os_malloc( sizeof( struct iot ) );
	bzero( lib, sizeof( struct iot ) );
	will_return( __wrap_os_malloc, 1u );
	opt = os_malloc( sizeof( struct iot_option )  );
	bzero( opt, sizeof( struct iot_option ) );
	will_return( __wrap_os_malloc, 1u );
	opts = os_malloc( sizeof( struct iot_options ) );
	bzero( opts, sizeof( struct iot_options ) );
	will_return( __wrap_os_malloc, 1u );
	opts_ptr = os_malloc( sizeof( struct iot_options * ) );

	opts->lib = lib;
	opts->option = opt;
	opts->option_count = 1u;
	opts_ptr[0] = opts;
	lib->options = opts_ptr;
	lib->options_count = 1u;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_malloc, 1u );
	opt->name = os_malloc( IOT_NAME_MAX_LEN );
#endif
	strncpy( opt->name, "test", IOT_NAME_MAX_LEN );

#ifndef IOT_STACK_ONLY
	opt->data.type = IOT_TYPE_STRING;
	will_return( __wrap_os_malloc, 1u );
	opt->data.heap_storage = os_malloc( 1u );
	opt->data.value.string = (const char*)opt->data.heap_storage;
#endif

	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_terminate( lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* clean up */
#ifdef IOT_STACK_ONLY
	os_free( opts_ptr );
	os_free( opts );
	os_free( opt );
	os_free( lib );
#endif
}

static void test_iot_terminate_telemetry( void **state )
{
	struct iot *lib;
	struct iot_telemetry telemetry;
	iot_status_t result;

	will_return( __wrap_os_malloc, 1 );
	lib = os_malloc( sizeof( struct iot ) );
	bzero( lib, sizeof( struct iot ) );
	bzero( &telemetry, sizeof( struct iot_telemetry ) );
	lib->telemetry_count = 1u;
	lib->telemetry_ptr[0] = &telemetry;

#ifndef IOT_STACK_ONLY
	will_return( __wrap_iot_telemetry_free, IOT_STATUS_FAILURE );
#endif /* ifndef IOT_STACK_ONLY */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );

	result = iot_terminate( lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* clean up */
#ifdef IOT_STACK_ONLY
	os_free( lib );
#endif
}

/* iot_timestamp_now */
static void test_iot_timestamp_now_valid( void **state )
{
	const iot_timestamp_t result = iot_timestamp_now();
	assert_int_equal( result, 1234567u );
}

/* iot_transaction_status */
static void test_iot_transaction_status_bad( void **state )
{
	iot_t lib;
	iot_status_t result;
	iot_transaction_t txn = 1u;

	bzero( &lib, sizeof( struct iot ) );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_EXECUTION_ERROR );
	result = iot_transaction_status( &lib, &txn, 0u );
	assert_int_equal( result, IOT_STATUS_EXECUTION_ERROR );
}

static void test_iot_transaction_status_good( void **state )
{
	iot_t lib;
	iot_status_t result;
	iot_transaction_t txn = 2u;

	bzero( &lib, sizeof( struct iot ) );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_transaction_status( &lib, &txn, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_transaction_status_null_lib( void **state )
{
	iot_status_t result;
	iot_transaction_t txn = 3u;

	result = iot_transaction_status( NULL, &txn, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_transaction_status_null_txn( void **state )
{
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( struct iot ) );
	result = iot_transaction_status( &lib, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

/* iot_version */
static void test_iot_version( void **state )
{
	unsigned int expected_version;
	expected_version = ( IOT_VERSION_MAJOR & 0xFF ) << 24;
	expected_version |= ( IOT_VERSION_MINOR & 0xFF ) << 16;
	expected_version |= ( IOT_VERSION_PATCH & 0xFF ) << 8;
	expected_version |= ( IOT_VERSION_TWEAK & 0xFF );
	assert_int_equal( iot_version(), expected_version );
}

/* iot_version_str */
static void test_iot_version_str( void **state )
{
	assert_string_equal( iot_version_str(), IOT_VERSION );
}

/* main */
int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_config_get_not_found ),
		cmocka_unit_test( test_iot_config_get_null_lib ),
		cmocka_unit_test( test_iot_config_get_null_name ),
		cmocka_unit_test( test_iot_config_get_valid ),
		cmocka_unit_test( test_iot_config_get_valid_convert_int32 ),
		cmocka_unit_test( test_iot_config_get_wrong_type ),
		cmocka_unit_test( test_iot_config_get_raw_not_found ),
		cmocka_unit_test( test_iot_config_get_raw_null_lib ),
		cmocka_unit_test( test_iot_config_get_raw_null_name ),
		cmocka_unit_test( test_iot_config_get_raw_null_data ),
		cmocka_unit_test( test_iot_config_get_raw_valid ),
		cmocka_unit_test( test_iot_config_get_raw_wrong_type ),
		cmocka_unit_test( test_iot_config_set_full ),
		cmocka_unit_test( test_iot_config_set_null_lib ),
		cmocka_unit_test( test_iot_config_set_null_name ),
		cmocka_unit_test( test_iot_config_set_null_data ),
		cmocka_unit_test( test_iot_config_set_overwrite ),
		cmocka_unit_test( test_iot_config_set_valid ),
		cmocka_unit_test( test_iot_config_set_raw_full ),
		cmocka_unit_test( test_iot_config_set_raw_null_lib ),
		cmocka_unit_test( test_iot_config_set_raw_null_name ),
		cmocka_unit_test( test_iot_config_set_raw_null_data ),
		cmocka_unit_test( test_iot_config_set_raw_overwrite_data ),
		cmocka_unit_test( test_iot_config_set_raw_overwrite_null ),
		cmocka_unit_test( test_iot_config_set_raw_valid ),
		cmocka_unit_test( test_iot_configuration_file_set_null_lib ),
		cmocka_unit_test( test_iot_configuration_file_set_null_path ),
		cmocka_unit_test( test_iot_configuration_file_set_no_memory ),
		cmocka_unit_test( test_iot_configuration_file_set_valid ),
		cmocka_unit_test( test_iot_connect_configuration_fail_to_parse ),
		cmocka_unit_test( test_iot_connect_configuration_fail_to_read ),
		cmocka_unit_test( test_iot_connect_configuration_no_memory ),
		cmocka_unit_test( test_iot_connect_configuration_not_found ),
		cmocka_unit_test( test_iot_connect_null_lib ),
		cmocka_unit_test( test_iot_connect_plugin_connect_fail ),
		cmocka_unit_test( test_iot_connect_single_thread ),
		cmocka_unit_test( test_iot_connect_threads_fail ),
		cmocka_unit_test( test_iot_connect_threads_main_loop_fail ),
		cmocka_unit_test( test_iot_connect_threads_success ),
		cmocka_unit_test( test_iot_directory_name_get_bad_type ),
		cmocka_unit_test( test_iot_directory_name_get_null_dest ),
		cmocka_unit_test( test_iot_directory_name_get_small_dest ),
		cmocka_unit_test( test_iot_directory_name_get_valid_config_dir ),
		cmocka_unit_test( test_iot_directory_name_get_valid_runtime_dir ),
		cmocka_unit_test( test_iot_disconnect_null_lib ),
		cmocka_unit_test( test_iot_disconnect_single_thread ),
		cmocka_unit_test( test_iot_disconnect_valid ),
		cmocka_unit_test( test_iot_error_unknown ),
		cmocka_unit_test( test_iot_error_valid ),
		cmocka_unit_test( test_iot_id_null_lib ),
		cmocka_unit_test( test_iot_id_null_id ),
		cmocka_unit_test( test_iot_id_valid ),
		cmocka_unit_test( test_iot_initialize_null ),
		cmocka_unit_test( test_iot_initialize_unable_to_write ),
		cmocka_unit_test( test_iot_initialize_valid_generate_uuid ),
		cmocka_unit_test( test_iot_initialize_valid_read_uuid ),
		cmocka_unit_test( test_iot_log_invalid_level ),
		cmocka_unit_test( test_iot_log_null_callback ),
		cmocka_unit_test( test_iot_log_null_lib ),
		cmocka_unit_test( test_iot_log_with_callback ),
		cmocka_unit_test( test_iot_log_callback_set_null_callback ),
		cmocka_unit_test( test_iot_log_callback_set_null_lib ),
		cmocka_unit_test( test_iot_log_callback_set_valid ),
		cmocka_unit_test( test_iot_log_level_set_invalid ),
		cmocka_unit_test( test_iot_log_level_set_null_lib ),
		cmocka_unit_test( test_iot_log_level_set_valid ),
		cmocka_unit_test( test_iot_log_level_set_string_invalid ),
		cmocka_unit_test( test_iot_log_level_set_string_null_lib ),
		cmocka_unit_test( test_iot_log_level_set_string_null_str ),
		cmocka_unit_test( test_iot_log_level_set_string_valid ),
		cmocka_unit_test( test_iot_loop_forever_null_lib ),
		cmocka_unit_test( test_iot_loop_forever_single_thread ),
		cmocka_unit_test( test_iot_loop_iteration_null_lib ),
		cmocka_unit_test( test_iot_loop_iteration_single_thread ),
		cmocka_unit_test( test_iot_loop_iteration_threads ),
		cmocka_unit_test( test_iot_loop_start_null_lib ),
		cmocka_unit_test( test_iot_loop_start_single_thread ),
		cmocka_unit_test( test_iot_loop_start_threads_fail ),
		cmocka_unit_test( test_iot_loop_start_threads_success ),
		cmocka_unit_test( test_iot_loop_start_threads_twice ),
		cmocka_unit_test( test_iot_loop_stop_null_lib ),
		cmocka_unit_test( test_iot_loop_stop_single_thread ),
		cmocka_unit_test( test_iot_loop_stop_threads_force ),
		cmocka_unit_test( test_iot_loop_stop_threads_no_force ),
		cmocka_unit_test( test_iot_terminate_action ),
		cmocka_unit_test( test_iot_terminate_alarm ),
		cmocka_unit_test( test_iot_terminate_blank ),
		cmocka_unit_test( test_iot_terminate_null_lib ),
		cmocka_unit_test( test_iot_terminate_option ),
		cmocka_unit_test( test_iot_terminate_telemetry ),
		cmocka_unit_test( test_iot_timestamp_now_valid ),
		cmocka_unit_test( test_iot_transaction_status_bad ),
		cmocka_unit_test( test_iot_transaction_status_good ),
		cmocka_unit_test( test_iot_transaction_status_null_lib ),
		cmocka_unit_test( test_iot_transaction_status_null_txn ),
		cmocka_unit_test( test_iot_version ),
		cmocka_unit_test( test_iot_version_str )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
