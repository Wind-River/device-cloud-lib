/**
 * @file
 * @brief unit testing for common functions (argument parsing file)
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

#include "iot_build.h"
#include "utilities/app_arg.h"

#include <stdlib.h>
#include <string.h>

static const char *ARG_VALUE[] = { NULL, NULL, NULL, NULL, NULL, NULL };
static const struct app_arg TEST_ARGS[] = {
	{ '1', "one",           APP_ARG_FLAG_OPTIONAL,                                  NULL,  NULL,          "one description",   1u },
	{ '2', "two",           APP_ARG_FLAG_REQUIRED | APP_ARG_FLAG_MULTI,             NULL,  NULL,          "two required",      2u },
	{ '3', "three",         APP_ARG_FLAG_OPTIONAL,                                  "arg", &ARG_VALUE[0], "three description", 3u },
	{ '4', "four",          APP_ARG_FLAG_OPTIONAL,                                  NULL,  &ARG_VALUE[1], "four description",  4u },
	{ '5', "five",          APP_ARG_FLAG_OPTIONAL | APP_ARG_FLAG_PARAM_OPTIONAL,    "arg", &ARG_VALUE[2], "five description",  5u },
	{ '6', "six",           APP_ARG_FLAG_OPTIONAL | APP_ARG_FLAG_PARAM_OPTIONAL,    "arg", &ARG_VALUE[3], "six description",   6u },
	{ '7', "seven",         APP_ARG_FLAG_OPTIONAL,                                  "arg", &ARG_VALUE[4], "seven description", 7u },
	{ '8', "eight",         APP_ARG_FLAG_OPTIONAL,                                  NULL,  NULL,          "eight description", 8u },
	{ '9', "nine",          APP_ARG_FLAG_OPTIONAL,                                  NULL,  NULL,          "nine description",  9u },
	{ 0u,  "name-only",     APP_ARG_FLAG_OPTIONAL,                                  NULL,  NULL,          "name-only #1",      10u },
	{ 0u,  "name-required", APP_ARG_FLAG_REQUIRED,                                  NULL,  NULL,          "name-only #2",      11u },
	{ 0u,  "name-with-arg", APP_ARG_FLAG_OPTIONAL,                                  "arg", &ARG_VALUE[5], "name-only #3",      12u },
	{ 0u,  NULL,            0,                                                      NULL,  NULL,          NULL,                0u }
};


/* static helper functions */
static void* internal_struct_copy( const void* struct_ptr, size_t item_size,
	size_t item_count )
{
	void *result = NULL;
	void *out_ptr = NULL;
	assert_non_null( struct_ptr );
	if ( item_count > 0u )
	{
		result = test_calloc( item_count, item_size );
		assert_non_null( result );
		out_ptr = result;
		while ( item_count > 0u )
		{
			memcpy( out_ptr, struct_ptr, item_size );
			struct_ptr = (const char *)struct_ptr + item_size;
			out_ptr = (char*)out_ptr + item_size;

			--item_count;
		}
	}
	return result;
}

static void internal_struct_free( void* struct_ptr )
{
	if ( struct_ptr )
		test_free( struct_ptr );
}

static char** internal_args_allocate( const char **args, int argc )
{
	char **result = NULL;
	if ( argc > 0 )
	{
		int i;
		result = test_malloc( sizeof( char * ) * argc );
		assert_non_null( result );
		printf( "parsing: [" );
		for ( i = 0; i < argc; ++i )
			printf( " %s", args[i] );
		printf( " ]\n" );
	}
	while ( argc > 0 )
	{
		size_t str_len;
		--argc;
		str_len = strlen( args[argc] ) + 1u;
		result[argc] = test_malloc( str_len );
		assert_non_null( result[argc] );
		strncpy( result[argc], args[argc], str_len );
	}
	return result;
}

static void internal_args_free( char **args, int argc )
{
	if ( args )
	{
		while( argc > 0 )
		{
			--argc;
			test_free( args[ argc ] );
		}
		test_free( args );
	}
}


/* app_arg_count */
static void test_app_arg_count_found_by_id( void **state )
{
	unsigned int result;

	result = app_arg_count( TEST_ARGS, '3', NULL );
	assert_int_equal( result, 3u );
}

static void test_app_arg_count_found_by_name( void **state )
{
	unsigned int result;

	result = app_arg_count( TEST_ARGS, 0, "three" );
	assert_int_equal( result, 3u );
}

static void test_app_arg_count_no_id_or_name( void **state )
{
	unsigned int result;

	result = app_arg_count( TEST_ARGS, 0, NULL );
	assert_int_equal( result, 0 );
}

static void test_app_arg_count_null_obj( void **state )
{
	unsigned int result;

	result = app_arg_count( NULL, 'c', "count" );
	assert_int_equal( result, 0 );
}

/* app_arg_find */
static void test_app_arg_find_null_argv( void **state )
{
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t *result;
	result = app_arg_find( 0u, NULL, &iter, '\0', NULL );
	assert_null( result );
}

static void test_app_arg_find_null_iter( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find( argc, argv, NULL, '\0', NULL );
	assert_null( result );

	internal_args_free( argv, argc );
}

static void test_app_arg_find_all_items( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find( argc, argv, &iter, '\0', NULL );
	assert_non_null( result );
	assert_int_equal( result->idx, 1u );
	assert_true( result == &iter );

	internal_args_free( argv, argc );
}

static void test_app_arg_find_item_by_id( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-2", "value", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find( argc, argv, &iter, '3', NULL );
	assert_non_null( result );
	assert_int_equal( result->idx, 5u );
	assert_true( result == &iter );

	internal_args_free( argv, argc );
}

static void test_app_arg_find_item_by_name( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-2", "value", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find( argc, argv, &iter, '\0', "name" );
	assert_non_null( result );
	assert_int_equal( result->idx, 2u );
	assert_true( result == &iter );

	internal_args_free( argv, argc );
}

/* app_arg_find_next */
static void test_app_arg_find_next_null_argv( void **state )
{
	app_arg_iterator_t *result;
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	result = app_arg_find_next( 0u, NULL, &iter );
	assert_null( result );
}

static void test_app_arg_find_next_null_iter( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find_next( argc, argv, NULL );
	assert_null( result );

	internal_args_free( argv, argc );
}

static void test_app_arg_find_next_all_items( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	int idx = 0;
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find( argc, argv, &iter, '\0', NULL );
	assert_non_null( result );
	assert_int_equal( result->idx, ++idx );
	assert_true( result == &iter );

	while ( idx < argc )
	{
		result = app_arg_find_next( argc, argv, result );
		if ( idx == argc - 1 )
		{
			assert_null( result );
			++idx;
		}
		else
		{
			assert_non_null( result );
			assert_int_equal( result->idx, ++idx );
			assert_true( result == &iter );
		}
	}

	internal_args_free( argv, argc );
}

static void test_app_arg_find_next_item_by_id( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-2", "value",
		"-3=value", "-2value", "-2=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find( argc, argv, &iter, '2', NULL );
	assert_non_null( result );
	assert_int_equal( result->idx, 1 );
	assert_true( result == &iter );

	result = app_arg_find_next( argc, argv, result );
	assert_non_null( result );
	assert_int_equal( result->idx, 3 );

	result = app_arg_find_next( argc, argv, result );
	assert_non_null( result );
	assert_int_equal( result->idx, 6 );

	result = app_arg_find_next( argc, argv, result );
	assert_non_null( result );
	assert_int_equal( result->idx, 7 );

	result = app_arg_find_next( argc, argv, result );
	assert_null( result );

	internal_args_free( argv, argc );
}

static void test_app_arg_find_next_item_by_name( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-2", "value",
		"--name=value", "--name", "value3", "--", "--name", "value4" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t *result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_find( argc, argv, &iter, '\0', "name" );
	assert_non_null( result );
	assert_int_equal( result->idx, 2 );
	assert_true( result == &iter );

	result = app_arg_find_next( argc, argv, result );
	assert_non_null( result );
	assert_int_equal( result->idx, 5 );

	result = app_arg_find_next( argc, argv, result );
	assert_non_null( result );
	assert_int_equal( result->idx, 6 );

	result = app_arg_find_next( argc, argv, result );
	assert_null( result );

	internal_args_free( argv, argc );
}

/* app_arg_iterator_key */
static void test_app_arg_iterator_key_null_argv( void **state )
{
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	size_t key_len = 0u;
	const char *key = NULL;
	int result;

	result = app_arg_iterator_key( 0u, NULL, &iter, &key_len, &key );
	assert_int_equal( result, 0 );
}

static void test_app_arg_iterator_key_null_iter( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	size_t key_len = 0u;
	const char *key = NULL;
	int result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_iterator_key( argc, argv, NULL, &key_len, &key );
	assert_int_equal( result, 0 );

	internal_args_free( argv, argc );
}

static void test_app_arg_iterator_key_valid( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-1", "--key2", "-3", "value1",
		"--key4", "value2", "-5=value3", "--key6=value4",
		"--", "-7=value5", "--key8", "value6" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t  *iter_ptr;
	size_t key_len = 0u;
	const char *key = NULL;
	int result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	iter_ptr = app_arg_find( argc, argv, &iter, '\0', NULL );
	assert_non_null( iter_ptr );

	while( iter_ptr )
	{
		const char *k;
		key_len = 0u;
		key = NULL;
		result = app_arg_iterator_key( argc, argv, iter_ptr, &key_len, &key );
		assert_int_equal( result, 1 );

		k = args[iter_ptr->idx];
		while ( *k == '-' )
			++k;
		printf( "expected key: %.*s==%.*s\n",
			(int)key_len, k, (int)key_len, key );
		assert_true( strncmp( key, k, key_len ) == 0 );
		iter_ptr = app_arg_find_next( argc, argv, iter_ptr );
	}

	key_len = 0u;
	key = NULL;
	result = app_arg_iterator_key( argc, argv, iter_ptr, &key_len, &key );
	assert_int_equal( result, 0 );
	assert_int_equal( key_len, 0 );
	assert_null( key );

	internal_args_free( argv, argc );
}

/* app_arg_iterator_value */
static void test_app_arg_iterator_value_null_argv( void **state )
{
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	size_t value_len = 0u;
	const char *value= NULL;
	int result;

	result = app_arg_iterator_value( 0u, NULL, &iter, &value_len, &value );
	assert_int_equal( result, 0 );
}

static void test_app_arg_iterator_value_null_iter( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-2", "--name", "-3=value" };
	int argc = sizeof( args ) / sizeof( const char * );
	size_t value_len = 0u;
	const char *value = NULL;
	int result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	result = app_arg_iterator_value( argc, argv, NULL, &value_len, &value );
	assert_int_equal( result, 0 );

	internal_args_free( argv, argc );
}

static void test_app_arg_iterator_value_valid( void **state )
{
	char **argv;
	const char *args[] = { "/path/to/app", "-1", "--key2", "-3", "value1",
		"--key4", "value2", "-5=value3", "--key6=value4",
		"--", "-7=value5", "--key8", "value6" };
	int argc = sizeof( args ) / sizeof( const char * );
	app_arg_iterator_t iter = APP_ARG_ITERATOR_INIT;
	app_arg_iterator_t  *iter_ptr;
	size_t value_len = 0u;
	const char *value = NULL;
	int result;

	argv = internal_args_allocate( args, argc );
	assert_non_null( argv );
	iter_ptr = app_arg_find( argc, argv, &iter, '\0', NULL );
	assert_non_null( iter_ptr );

	while( iter_ptr )
	{
		const char *v;
		value_len = 0u;
		value = NULL;
		result = app_arg_iterator_value( argc, argv,
			iter_ptr, &value_len, &value );

		v = strchr( args[iter_ptr->idx], '=' );
		if ( v )
			++v;
		else
		{
			v = args[iter_ptr->idx + 1];
			if ( *v == '-' )
				v = NULL;
		}

		if ( v )
			assert_int_equal( result, 1 );
		else
			assert_int_equal( result, 0 );

		printf( "expected value: %.*s==%.*s\n",
			(int)value_len, v, (int)value_len, value );
		assert_true( strncmp( value, v, value_len ) == 0 );
		iter_ptr = app_arg_find_next( argc, argv, iter_ptr );
	}

	value_len = 0u;
	value = NULL;
	result = app_arg_iterator_value( argc, argv,
		iter_ptr, &value_len, &value );
	assert_int_equal( result, 0 );
	assert_int_equal( value_len, 0u );
	assert_null( value );

	internal_args_free( argv, argc );
}

/* app_arg_parse */
static void test_app_arg_parse_argument_expected_value( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"-3=three_value" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_SUCCESS );
	assert_int_equal( opts[0].hit, 0u ); /* -1, --one */
	assert_int_equal( opts[1].hit, 1u ); /* -2, --two */
	assert_int_equal( opts[2].hit, 1u ); /* -3, --three */
	assert_string_equal( ARG_VALUE[0], "three_value" );
	assert_int_equal( opts[3].hit, 0u ); /* -4, --four */
	assert_int_equal( opts[4].hit, 0u ); /* -5, --five */
	assert_int_equal( opts[5].hit, 0u ); /* -6, --six */
	assert_int_equal( opts[6].hit, 0u ); /* -7, --seven */
	assert_int_equal( opts[7].hit, 0u ); /* -8, --eight */
	assert_int_equal( opts[8].hit, 0u ); /* -9, --nine */
	assert_int_equal( opts[9].hit, 0u ); /* --name-only */
	assert_int_equal( opts[10].hit, 1u ); /* --name-required */
	assert_int_equal( opts[11].hit, 0u ); /* --name-with-arg */

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_no_value( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"--three" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_optional_not_specified( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"--five", "--six" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_SUCCESS );
	assert_int_equal( opts[0].hit, 0u ); /* -1, --one */
	assert_int_equal( opts[1].hit, 1u ); /* -2, --two */
	assert_int_equal( opts[2].hit, 0u ); /* -3, --three */
	assert_int_equal( opts[3].hit, 0u ); /* -4, --four */
	assert_int_equal( opts[4].hit, 1u ); /* -5, --five */
	assert_null( ARG_VALUE[2] );
	assert_int_equal( opts[5].hit, 1u ); /* -6, --six */
	assert_int_equal( opts[6].hit, 0u ); /* -7, --seven */
	assert_int_equal( opts[7].hit, 0u ); /* -8, --eight */
	assert_int_equal( opts[8].hit, 0u ); /* -9, --nine */
	assert_int_equal( opts[9].hit, 0u ); /* --name-only */
	assert_int_equal( opts[10].hit, 1u ); /* --name-required */
	assert_int_equal( opts[11].hit, 0u ); /* --name-with-arg */

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_optional_specified( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"--five", "five_value" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_SUCCESS );
	assert_int_equal( opts[0].hit, 0u ); /* -1, --one */
	assert_int_equal( opts[1].hit, 1u ); /* -2, --two */
	assert_int_equal( opts[2].hit, 0u ); /* -3, --three */
	assert_int_equal( opts[3].hit, 0u ); /* -4, --four */
	assert_int_equal( opts[4].hit, 1u ); /* -5, --five */
	assert_string_equal( ARG_VALUE[2], "five_value" );
	assert_int_equal( opts[5].hit, 0u ); /* -6, --six */
	assert_int_equal( opts[6].hit, 0u ); /* -7, --seven */
	assert_int_equal( opts[7].hit, 0u ); /* -8, --eight */
	assert_int_equal( opts[8].hit, 0u ); /* -9, --nine */
	assert_int_equal( opts[9].hit, 0u ); /* --name-only */
	assert_int_equal( opts[10].hit, 1u ); /* --name-required */
	assert_int_equal( opts[11].hit, 0u ); /* --name-with-arg */

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_optional_specified_twice( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"--five", "five_value", "five_value2" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_required( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"--seven", "--eight" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_bad_key( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"-=" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_duplicate( void **state )
{
	const char *args[] = { "/path/to/app", "-1", "-2", "-1",
		"--name-required" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_unexpected_value( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"-8=eight_value" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_unknown_dash( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"--unknown-arg" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_argument_unknown_no_dash( void **state )
{
	const char *args[] = { "/path/to/app", "-2", "--name-required",
		"unknown-arg" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, NULL );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_missing_required( void **state )
{
	const char *args[] = { "/path/to/app", "-2" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int pos = 0u;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, &pos );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_mulitple( void **state )
{
	const char *args[] = { "/path/to/app", "--name-required",
		"-2", "-2", "-2", "-2", "-2", "-2", "-2" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int pos = 0u;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, &pos );
	assert_int_equal( result, EXIT_SUCCESS );
	assert_int_equal( opts[0].hit, 0u ); /* -1, --one */
	assert_int_equal( opts[1].hit, 7u ); /* -2, --two */
	assert_int_equal( opts[2].hit, 0u ); /* -3, --three */
	assert_int_equal( opts[3].hit, 0u ); /* -4, --four */
	assert_int_equal( opts[4].hit, 0u ); /* -5, --five */
	assert_int_equal( opts[5].hit, 0u ); /* -6, --six */
	assert_int_equal( opts[6].hit, 0u ); /* -7, --seven */
	assert_int_equal( opts[7].hit, 0u ); /* -8, --eight */
	assert_int_equal( opts[8].hit, 0u ); /* -9, --nine */
	assert_int_equal( opts[9].hit, 0u ); /* --name-only */
	assert_int_equal( opts[10].hit, 1u ); /* --name-required */
	assert_int_equal( opts[11].hit, 0u ); /* --name-with-arg */

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}

static void test_app_arg_parse_null_obj( void **state )
{
	const char *args[] = { "/path/to/app" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int pos = 0u;
	int result;

	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( NULL, argc, argv, &pos );
	assert_int_equal( result, EXIT_SUCCESS );

	internal_args_free( argv, argc );
}

static void test_app_arg_parse_no_args( void **state )
{
	const char *args[] = { "/path/to/app" };
	int argc = sizeof( args ) / sizeof( const char * );
	char **argv;
	int pos = 0u;
	int result;

	struct app_arg *opts =
		(struct app_arg *)internal_struct_copy( TEST_ARGS,
		sizeof( struct app_arg ),
		sizeof( TEST_ARGS ) / sizeof( struct app_arg ) );
	argv = internal_args_allocate( args, argc );

	result = app_arg_parse( opts, argc, argv, &pos );
	assert_int_equal( result, EXIT_FAILURE );

	internal_struct_free( opts );
	internal_args_free( argv, argc );
}


/* app_arg_usage */
static void test_app_arg_usage_null_obj( void **state )
{
	app_arg_usage( NULL, 10u, "app name", "app description",
		"pos_id", "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_null_app_description( void **state )
{
	app_arg_usage( TEST_ARGS, 10u, "app name", NULL,
		"pos_id", "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_null_app_name( void **state )
{
	app_arg_usage( TEST_ARGS, 10u, NULL, "app description",
		"pos_id", "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_null_positional_name( void **state )
{
	app_arg_usage( TEST_ARGS, 10u, "app name", "app description",
		NULL, "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_null_positional_description( void **state )
{
	app_arg_usage( TEST_ARGS, 20u, "path/to/app", "app description",
		"pos_id", NULL );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_positional_name_multi( void **state )
{
	app_arg_usage( TEST_ARGS, 20u, "path/to/app", "app description",
		"pos_id+", "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_positional_name_multi_optional( void **state )
{
	app_arg_usage( TEST_ARGS, 20u, "path/to/app", "app description",
		"[pos_id]+", "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_positional_name_optional( void **state )
{
	app_arg_usage( TEST_ARGS, 20u, "path/to/app", "app description",
		"[pos_id]", "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}

static void test_app_arg_usage_positional_name_single( void **state )
{
	app_arg_usage( TEST_ARGS, 200u, "path/to/app", "app description",
		"pos_id", "positional argument description" );
	/* above function shouldn't crash */
	assert_true( 1 );
}


/* main */
int main( int argc, char* argv[] )
{
	int result;
	const struct CMUnitTest tests[] =
	{
		cmocka_unit_test( test_app_arg_count_found_by_id ),
		cmocka_unit_test( test_app_arg_count_found_by_name ),
		cmocka_unit_test( test_app_arg_count_no_id_or_name ),
		cmocka_unit_test( test_app_arg_count_null_obj ),
		cmocka_unit_test( test_app_arg_find_null_argv ),
		cmocka_unit_test( test_app_arg_find_null_iter ),
		cmocka_unit_test( test_app_arg_find_all_items ),
		cmocka_unit_test( test_app_arg_find_item_by_id ),
		cmocka_unit_test( test_app_arg_find_item_by_name ),
		cmocka_unit_test( test_app_arg_find_next_null_argv ),
		cmocka_unit_test( test_app_arg_find_next_null_iter ),
		cmocka_unit_test( test_app_arg_find_next_all_items ),
		cmocka_unit_test( test_app_arg_find_next_item_by_id ),
		cmocka_unit_test( test_app_arg_find_next_item_by_name ),
		cmocka_unit_test( test_app_arg_iterator_key_null_argv ),
		cmocka_unit_test( test_app_arg_iterator_key_null_iter ),
		cmocka_unit_test( test_app_arg_iterator_key_valid ),
		cmocka_unit_test( test_app_arg_iterator_value_null_argv ),
		cmocka_unit_test( test_app_arg_iterator_value_null_iter ),
		cmocka_unit_test( test_app_arg_iterator_value_valid ),
		cmocka_unit_test( test_app_arg_parse_argument_bad_key ),
		cmocka_unit_test( test_app_arg_parse_argument_duplicate ),
		cmocka_unit_test( test_app_arg_parse_argument_expected_value ),
		cmocka_unit_test( test_app_arg_parse_argument_no_value ),
		cmocka_unit_test( test_app_arg_parse_argument_optional_not_specified ),
		cmocka_unit_test( test_app_arg_parse_argument_optional_specified ),
		cmocka_unit_test( test_app_arg_parse_argument_optional_specified_twice ),
		cmocka_unit_test( test_app_arg_parse_argument_required ),
		cmocka_unit_test( test_app_arg_parse_argument_unexpected_value ),
		cmocka_unit_test( test_app_arg_parse_argument_unknown_dash ),
		cmocka_unit_test( test_app_arg_parse_argument_unknown_no_dash ),
		cmocka_unit_test( test_app_arg_parse_missing_required ),
		cmocka_unit_test( test_app_arg_parse_mulitple ),
		cmocka_unit_test( test_app_arg_parse_null_obj ),
		cmocka_unit_test( test_app_arg_parse_no_args ),
		cmocka_unit_test( test_app_arg_usage_null_obj ),
		cmocka_unit_test( test_app_arg_usage_null_app_name ),
		cmocka_unit_test( test_app_arg_usage_null_app_description ),
		cmocka_unit_test( test_app_arg_usage_null_app_name ),
		cmocka_unit_test( test_app_arg_usage_null_positional_name ),
		cmocka_unit_test( test_app_arg_usage_null_positional_description ),
		cmocka_unit_test( test_app_arg_usage_positional_name_multi ),
		cmocka_unit_test( test_app_arg_usage_positional_name_optional ),
		cmocka_unit_test( test_app_arg_usage_positional_name_optional ),
		cmocka_unit_test( test_app_arg_usage_positional_name_single ),
	};
	MOCK_SYSTEM_ENABLED = 1;
	result = cmocka_run_group_tests( tests, NULL, NULL );
	MOCK_SYSTEM_ENABLED = 0;
	return result;
}

