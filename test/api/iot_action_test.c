/**
 * @file
 * @brief unit testing for IoT library (action source file)
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

static iot_status_t test_callback_func( iot_action_request_t *request, void *user_data )
{
	assert_non_null( request );
	return mock_type( iot_status_t );
}

static void test_iot_action_allocate_existing( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;
#ifndef IOT_STACK_ONLY
	char _name[IOT_ACTION_STACK_MAX][ IOT_NAME_MAX_LEN + 1u ];
#endif

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
#ifdef IOT_STACK_ONLY
		lib.action_ptr[i]->name = lib.action_ptr[i]->_name;
#else
		lib.action_ptr[i]->name = &_name[i][0];
#endif
	}

	strncpy( lib.action_ptr[0]->name, "1action", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "5action", IOT_NAME_MAX_LEN );
	lib.action_count = 2u;

#ifndef IOT_STACK_ONLY
	/* allocate room for name */
	will_return( __wrap_os_malloc, 1 );
#endif
	action = iot_action_allocate( &lib, "3action" );
	assert_non_null( action );
	assert_ptr_equal( lib.action_ptr[1], action );
	assert_int_equal( lib.action_count, 3u );
	assert_ptr_equal( action->lib, &lib );
	assert_string_equal( action->name, "3action" );
	assert_string_equal( lib.action_ptr[0]->name, "1action" );
	assert_string_equal( lib.action_ptr[2]->name, "5action" );

#ifndef IOT_STACK_ONLY
	os_free( action->name );
#endif
}

static void test_iot_action_allocate_first( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 0u;

#ifndef IOT_STACK_ONLY
	/* allocate room for name */
	will_return( __wrap_os_malloc, 1 );
#endif
	action = iot_action_allocate( &lib, "someaction" );
	assert_non_null( action );
	assert_ptr_equal( lib.action_ptr[0], action );
	assert_int_equal( lib.action_count, 1u );
	assert_string_equal( action->name, "someaction" );
	assert_ptr_equal( action->lib, &lib );
#ifndef IOT_STACK_ONLY
	os_free( action->name );
#endif
}

static void test_iot_action_allocate_full( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN];
#ifndef IOT_STACK_ONLY
	char _name[IOT_ACTION_MAX][ IOT_NAME_MAX_LEN + 1u ];
#endif
	iot_action_t *stack_actions = NULL;

	stack_actions = (iot_action_t *)test_calloc( IOT_ACTION_MAX - IOT_ACTION_STACK_MAX,
	                                             sizeof( iot_action_t ) );
	assert_non_null( stack_actions );

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_MAX; ++i )
	{
		if ( i < IOT_ACTION_STACK_MAX )
			lib.action_ptr[i] = &lib.action[i];
		else
			lib.action_ptr[i] = &stack_actions[i - IOT_ACTION_STACK_MAX];
#ifdef IOT_STACK_ONLY
		lib.action_ptr[i]->name = lib.action_ptr[i]->_name;
#else
		lib.action_ptr[i]->name = &_name[i][0];
#endif
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
	}

	lib.action_count = IOT_ACTION_MAX;
	action = iot_action_allocate( &lib, "newaction" );
	assert_null( action );
	assert_int_equal( lib.action_count, IOT_ACTION_MAX );
	for ( i = 0u; i < IOT_ACTION_MAX; ++i )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
		assert_string_equal( lib.action_ptr[i]->name, name );
	}
	test_free( stack_actions );
}

static void test_iot_action_allocate_stack_full( void **state )
{
	size_t i;
	iot_action_t *action;
	iot_t lib;
	char name[IOT_NAME_MAX_LEN];
#ifndef IOT_STACK_ONLY
	char _name[IOT_ACTION_STACK_MAX][ IOT_NAME_MAX_LEN + 1u ];
#endif

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
#ifdef IOT_STACK_ONLY
		lib.action_ptr[i]->name = lib.action_ptr[i]->_name;
#else
		lib.action_ptr[i]->name = &_name[i][0];
#endif
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
	}
	lib.action_count = IOT_ACTION_STACK_MAX;
#ifndef IOT_STACK_ONLY
	if ( IOT_ACTION_MAX > IOT_ACTION_STACK_MAX )
	{
		will_return( __wrap_os_malloc, 1 ); /* for new object */
		will_return( __wrap_os_malloc, 1 ); /* for item name */
	}
#endif
	action = iot_action_allocate( &lib, "newaction" );
	if ( IOT_ACTION_MAX > IOT_ACTION_STACK_MAX )
	{
		assert_non_null( action );
		assert_int_equal( lib.action_count, IOT_ACTION_STACK_MAX + 1u );
	}
	else
	{
		assert_null( action );
		assert_int_equal( lib.action_count, IOT_ACTION_STACK_MAX );
	}
	for ( i = 0u; i < IOT_ACTION_STACK_MAX + 1u && i < IOT_ACTION_MAX; ++i )
	{
		if ( i < IOT_ACTION_STACK_MAX )
			snprintf( name, IOT_NAME_MAX_LEN, "%luaction", i + 1u );
		else
			snprintf( name, IOT_NAME_MAX_LEN, "newaction" );
		assert_string_equal( lib.action_ptr[i]->name, name );
	}

#ifndef IOT_STACK_ONLY
	if ( IOT_ACTION_MAX > IOT_ACTION_STACK_MAX )
	{
		os_free( lib.action_ptr[IOT_ACTION_STACK_MAX]->name );
		os_free( lib.action_ptr[IOT_ACTION_STACK_MAX] );
	}
#endif
}

static void test_iot_action_allocate_null_lib( void **state )
{
	iot_action_t *action;
	action = iot_action_allocate( NULL, "newaction" );
	assert_null( action );
}

static void test_iot_action_deregister_deregistered( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_DEREGISTERED;
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_action_deregister_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_deregister( NULL, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_deregister_null_lib( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = NULL;
	action->state = IOT_ITEM_REGISTERED;
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_REGISTERED );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_action_deregister_transmit_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTER_PENDING );
	assert_int_equal( result, IOT_STATUS_FAILURE );
}

static void test_iot_action_deregister_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_deregister( action, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

static void test_iot_action_flags_set_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_flags_set( NULL, 5 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_flags_set_valid( void **state )
{
	iot_action_t action;
	iot_status_t result;

	bzero( &action, sizeof( iot_action_t ) );
	result = iot_action_flags_set( &action, 5 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.flags, 5 );
}

static void test_iot_action_free_options( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].option = lib.action[i]._option;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( sizeof( char ) + ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	}
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	action->option_count = 3u;
#ifdef IOT_STACK_ONLY
	action->option = action->_option;
	action->option[0].name = action->option[0]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	action->option = os_malloc( sizeof( struct iot_option ) * IOT_OPTION_MAX );
	bzero( action->option, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	will_return( __wrap_os_malloc, 1 );
	action->option[0].name = os_malloc( sizeof( char ) + ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action->option[0].name, "option 1", IOT_NAME_MAX_LEN );
	action->option[0].data.type = IOT_TYPE_STRING;
	will_return( __wrap_os_malloc, 1 );
	action->option[0].data.heap_storage = os_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->option[0].data.value.string = (char *)action->option[0].data.heap_storage;
	strncpy( (char *)action->option[0].data.heap_storage,
		"here is some text", IOT_NAME_MAX_LEN );
#ifdef IOT_STACK_ONLY
	action->option[1].name = action->option[1]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	action->option[1].name = os_malloc( sizeof( char ) + ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action->option[1].name, "option 2", IOT_NAME_MAX_LEN );
	action->option[1].data.type = IOT_TYPE_STRING;
	will_return( __wrap_os_malloc, 1 );
	action->option[1].data.heap_storage = os_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->option[1].data.value.string = (char *)action->option[1].data.heap_storage;
	strncpy( (char *)action->option[1].data.heap_storage,
		"some more text", IOT_NAME_MAX_LEN );
#ifdef IOT_STACK_ONLY
	action->option[2].name = action->option[2]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	action->option[2].name = os_malloc( sizeof( char ) + ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action->option[2].name, "option 3", IOT_NAME_MAX_LEN );
	action->option[2].data.type = IOT_TYPE_RAW;
	will_return( __wrap_os_malloc, 1 );
	action->option[2].data.heap_storage = os_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->option[2].data.value.raw.ptr = action->option[2].data.heap_storage;
	action->option[2].data.value.raw.length = IOT_NAME_MAX_LEN;
	strncpy( (char *)action->option[2].data.heap_storage,
		"oh look more text", IOT_NAME_MAX_LEN );

	/* remove an action */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.action_count, 2u );

	/* result */
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[2] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[1] );

	/* action should be deregistered */
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		if ( lib.action[i].name )
			os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_free_not_found( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	bzero( &action, sizeof( iot_action_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	}
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
#ifdef IOT_STACK_ONLY
	action.name = action._name;
#else
	will_return( __wrap_os_malloc, 1 );
	action.name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action.name, "action 4", IOT_NAME_MAX_LEN );
	action.lib = &lib;
	action.state = IOT_ITEM_REGISTERED;
	action.callback = &test_callback_func;

	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_free( &action, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( lib.action_count, 3u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[1] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[2] );
	assert_int_equal( action.state, IOT_ITEM_DEREGISTERED );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		if ( lib.action[i].name )
			os_free( lib.action[i].name );
	}
	os_free( action.name );
#endif
}

static void test_iot_action_free_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_free( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_free_null_handle( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	}
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = NULL;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
	assert_int_equal( lib.action_count, 3u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[1] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[2] );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		if ( lib.action[i].name )
			os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_free_parameters( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	}
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	action->parameter_count = 3u;
#ifdef IOT_STACK_ONLY
	action->parameter = action->_parameter;
	action->parameter[0].name = action->parameter[0]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	action->parameter = os_malloc( sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
	will_return( __wrap_os_malloc, 1 );
	action->parameter[0].name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action->parameter[0].name, "parameter 1", IOT_NAME_MAX_LEN );
	action->parameter[0].data.type = IOT_TYPE_STRING;
	action->parameter[0].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->parameter[0].data.value.string = (char *)action->parameter[0].data.heap_storage;
	strncpy(
	    (char *)action->parameter[0].data.heap_storage, "here is some text", IOT_NAME_MAX_LEN );
#ifdef IOT_STACK_ONLY
	action->parameter[1].name = action->parameter[1]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	action->parameter[1].name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action->parameter[1].name, "parameter 2", IOT_NAME_MAX_LEN );
	action->parameter[1].data.type = IOT_TYPE_STRING;
	action->parameter[1].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->parameter[1].data.value.string = (char *)action->parameter[1].data.heap_storage;
	strncpy( (char *)action->parameter[1].data.heap_storage, "some more text", IOT_NAME_MAX_LEN );
#ifdef IOT_STACK_ONLY
	action->parameter[2].name = action->parameter[2]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	action->parameter[2].name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action->parameter[2].name, "parameter 3", IOT_NAME_MAX_LEN );
	action->parameter[2].data.type = IOT_TYPE_RAW;
	action->parameter[2].data.heap_storage = test_malloc( sizeof( char ) * IOT_NAME_MAX_LEN );
	action->parameter[2].data.value.raw.ptr = action->parameter[2].data.heap_storage;
	action->parameter[2].data.value.raw.length = IOT_NAME_MAX_LEN;
	strncpy(
	    (char *)action->parameter[2].data.heap_storage, "oh look more text", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.action_count, 2u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[2] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[1] );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTERED );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		if ( lib.action[i].name )
			os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_free_transmit_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_action_t *action;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		lib.action_ptr[i] = &lib.action[i];
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	}
	lib.action_count = 3u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[1]->name, "action 2", IOT_NAME_MAX_LEN );
	strncpy( lib.action_ptr[2]->name, "action 3", IOT_NAME_MAX_LEN );
	action = lib.action_ptr[1];
	action->lib = &lib;
	action->state = IOT_ITEM_REGISTERED;
	action->callback = &test_callback_func;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_action_free( action, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.action_count, 2u );
	assert_ptr_equal( lib.action_ptr[0], &lib.action[0] );
	assert_ptr_equal( lib.action_ptr[1], &lib.action[2] );
	assert_ptr_equal( lib.action_ptr[2], &lib.action[1] );
	assert_int_equal( action->state, IOT_ITEM_DEREGISTER_PENDING );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_option_get_not_there( void **state )
{
	iot_action_t action;
	iot_status_t result;
	iot_int32_t data = 0;
	size_t i;

	bzero( &action, sizeof( iot_action_t ) );
#ifdef IOT_STACK_ONLY
	action.option = action._option;
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
		action.option[i].name = action._option[i]._name;
#else
	action.option = test_malloc( sizeof( struct iot_option ) * IOT_OPTION_MAX );
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
	{
		action.option[i].name = test_malloc( IOT_NAME_MAX_LEN + 1u );
		assert_non_null( action.option[i].name );
	}
#endif
	assert_non_null( action.option );

	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_INT32;
	action.option[0].data.value.int32 = 12345;
	action.option[0].data.has_value = IOT_TRUE;
	strncpy( action.option[1].name, "someotheroption", IOT_NAME_MAX_LEN );
	action.option[1].data.type = IOT_TYPE_FLOAT32;
	action.option[1].data.value.float32 = 123.456f;
	action.option[1].data.has_value = IOT_TRUE;
	action.option_count = 2u;
	result = iot_action_option_get( &action, "yetanotheroption",
		IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( data, 0 );

#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
		test_free( action.option[i].name  );
	test_free( action.option );
#endif
}

static void test_iot_action_option_get_null_action( void **state )
{
	iot_status_t result;
	iot_int32_t data = 0;

	result = iot_action_option_get( NULL, "someoption", IOT_TRUE,
		IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( data, 0 );
}

static void test_iot_action_option_get_null_name( void **state )
{
	iot_action_t action;
	iot_status_t result;
	iot_int32_t data = 0;
	size_t i;

	bzero( &action, sizeof( iot_action_t ) );
#ifdef IOT_STACK_ONLY
	action.option = action._option;
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
		action.option[i].name = action._option[i]._name;
#else
	action.option = test_malloc( sizeof( struct iot_option ) * IOT_OPTION_MAX );
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
	{
		action.option[i].name = test_malloc( IOT_NAME_MAX_LEN + 1u );
		assert_non_null( action.option[i].name );
	}
#endif
	assert_non_null( action.option );
	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_INT32;
	action.option[0].data.value.int32 = 12345;
	action.option[0].data.has_value = IOT_TRUE;
	action.option_count = 1u;
	result = iot_action_option_get( &action, NULL, IOT_TRUE,
		IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( data, 0 );

#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
		test_free( action.option[i].name  );
	test_free( action.option );
#endif
}

static void test_iot_action_option_get_valid( void **state )
{
	iot_action_t action;
	iot_status_t result;
	iot_int32_t data = 0;

	bzero( &action, sizeof( iot_action_t ) );
#ifdef IOT_STACK_ONLY
	action.option = action._option;
	action.option->name = action.option->_name;
#else
	action.option = test_malloc( sizeof( struct iot_option ) );
	assert_non_null( action.option );
	action.option->name = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
	assert_non_null( action.option->name );
#endif
	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_INT32;
	action.option[0].data.value.int32 = 12345;
	action.option[0].data.has_value = IOT_TRUE;
	action.option_count = 1u;
	result = iot_action_option_get( &action, "someoption", IOT_TRUE, IOT_TYPE_INT32, &data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( data, 12345 );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( action.option->name );
	test_free( action.option );
#endif
}

static void test_iot_action_option_set_add( void **state )
{
	iot_action_t action;
	iot_status_t result;

	bzero( &action, sizeof( iot_action_t ) );
#ifdef IOT_STACK_ONLY
	action.option = action._option;
	action.option->name = action.option->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	action.option = os_malloc( sizeof( struct iot_option ) );
	will_return( __wrap_os_malloc, 1 );
	action.option->name = os_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
	will_return( __wrap_os_realloc, 1 ); /* extend array for new parameter */
	will_return( __wrap_os_malloc, 1 ); /* name of new parameter */
#endif
	strncpy( action.option[0].name, "someotheroption", IOT_NAME_MAX_LEN );
	action.option_count = 1u;
	result = iot_action_option_set( &action, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.option[1].data.value.int8, 35 );
	assert_string_equal( action.option[0].name, "someotheroption" );
	assert_string_equal( action.option[1].name, "someoption" );
	assert_int_equal( action.option_count, 2u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( action.option[1].name );
	os_free( action.option[0].name );
	os_free( action.option );
#endif
}

static void test_iot_action_option_set_full( void **state )
{
	size_t i;
	iot_action_t action;
	iot_status_t result;
	char _name[ IOT_OPTION_MAX ][ IOT_NAME_MAX_LEN + 1u ];
	struct iot_option _options[IOT_OPTION_MAX];

	bzero( &action, sizeof( iot_action_t ) );
	bzero( _options, sizeof( struct iot_option ) * IOT_OPTION_MAX );
	action.option = _options;
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
	{
		action.option[i].name = _name[i];
		snprintf( action.option[i].name, IOT_NAME_MAX_LEN,
			"option%lu", i + 1 );
	}
	action.option_count = IOT_OPTION_MAX;
	result = iot_action_option_set( &action, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_FULL );
	for ( i = 0u; i < IOT_OPTION_MAX; ++i )
		assert_string_not_equal( action.option[i].name, "someoption" );
	assert_int_equal( action.option_count, IOT_OPTION_MAX );
}

static void test_iot_action_option_set_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_option_set( NULL, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_option_set_null_data( void **state )
{
	iot_action_t action;
	iot_status_t result;

	bzero( &action, sizeof( iot_action_t ) );
	action.option_count = 0u;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 ); /* for array */
	will_return( __wrap_os_malloc, 1 ); /* for name */
#endif
	result = iot_action_option_set( &action,
		"someoption", IOT_TYPE_NULL, NULL );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.option_count, 1u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( action.option[0].name );
	os_free( action.option );
#endif
}

static void test_iot_action_option_set_update( void **state )
{
	iot_action_t action;
	iot_status_t result;

	bzero( &action, sizeof( iot_action_t ) );
#ifdef IOT_STACK_ONLY
	action.option = action._option;
	action.option[0].name = action.option[0]._name;
#else
	/* allocate space for option array */
	will_return( __wrap_os_malloc, 1 );
	action.option = os_malloc( sizeof( struct iot_option ) );
	/* allocate space for option name */
	will_return( __wrap_os_malloc, 1 );
	action.option[0].name = os_malloc(
		sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
	strncpy( action.option[0].name, "someoption", IOT_NAME_MAX_LEN );
	action.option[0].data.type = IOT_TYPE_FLOAT32;
	action.option[0].data.value.float32 = 12.3f;
	action.option[0].data.has_value = IOT_TRUE;
	action.option_count = 1u;
	result = iot_action_option_set( &action, "someoption", IOT_TYPE_INT8, 35 );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action.option[0].data.value.int8, 35 );
	assert_int_equal( action.option[0].data.type, IOT_TYPE_INT8 );
	assert_string_equal( action.option[0].name, "someoption" );
	assert_int_equal( action.option_count, 1u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( action.option[0].name );
	os_free( action.option );
#endif
}

static void test_iot_action_option_set_raw_add( void **state )
{
	iot_action_t action;
	iot_status_t result;
	char data[20] = "this is text\0";

	bzero( &action, sizeof( iot_action_t ) );
#ifdef IOT_STACK_ONLY
	action.option = action._option;
	action.option[0].name = action.option[0]._name;
#else
	/* allocate space for option array */
	will_return( __wrap_os_malloc, 1 );
	action.option = os_malloc( sizeof( struct iot_option ) );
	/* allocate space for option name */
	will_return( __wrap_os_malloc, 1 );
	action.option[0].name = os_malloc(
		sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );

	will_return( __wrap_os_realloc, 1 ); /* for array expansion */
	will_return( __wrap_os_malloc, 1 ); /* for new option name */
#endif
	strncpy( action.option[0].name, "someotheroption", IOT_NAME_MAX_LEN );
	action.option_count = 1u;
	result = iot_action_option_set_raw( &action, "someoption", sizeof( data ), (void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)action.option[1].data.value.raw.ptr, "this is text" );
	assert_string_equal( action.option[0].name, "someotheroption" );
	assert_string_equal( action.option[1].name, "someoption" );
	assert_int_equal( action.option_count, 2u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( action.option[1].name );
	os_free( action.option[0].name );
	os_free( action.option );
#endif
}

static void test_iot_action_parameter_add_bad_name( void **state )
{
	iot_status_t result;
	iot_action_t action;
	iot_t lib;

	bzero( &lib, sizeof( iot_t ) );
	bzero( &action, sizeof( iot_action_t ) );
	action.lib = &lib;
	action.parameter_count = 0u;
	result = iot_action_parameter_add(
	    &action, "new\\ | p&ar;a=meter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_equal( action.parameter_count, 0u );
}

static void test_iot_action_parameter_add_exists( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char name[IOT_NAME_MAX_LEN];

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
#ifdef IOT_STACK_ONLY
	action->parameter = action->_parameter;
#else
	will_return( __wrap_os_malloc, 1 );
	action->parameter = os_malloc(
		sizeof( struct iot_action_parameter ) * ( IOT_PARAMETER_MAX / 2 ) );
#endif
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; ++i )
	{
#ifdef IOT_STACK_ONLY
		action->parameter = action->_parameter;
#else
		/* allocate space for option name */
		will_return( __wrap_os_malloc, 1 );
		action->parameter[i].name = os_malloc(
			sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
		snprintf( action->parameter[i].name, IOT_NAME_MAX_LEN,
			"parameter name %ld", i );
	}
	action->parameter_count = IOT_PARAMETER_MAX / 2;
	result = iot_action_parameter_add(
	    action, "parameter name 1", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_equal( action->parameter_count, IOT_PARAMETER_MAX / 2 );
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; ++i )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
		assert_string_equal( action->parameter[i].name, name );
	}

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; ++i )
		os_free( action->parameter[i].name );
	os_free( action->parameter );
#endif
}

static void test_iot_action_parameter_add_null_action( void **state )
{
	iot_status_t result;
	iot_t lib;

	bzero( &lib, sizeof( iot_t ) );
	result =
	    iot_action_parameter_add( NULL, "new parameter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_add_null_name( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->parameter_count = 0u;
	result = iot_action_parameter_add( action, NULL, IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( action->parameter_count, 0u );
}

static void test_iot_action_parameter_add_parameters_empty( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->parameter_count = 0u;
#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 );
	will_return( __wrap_os_malloc, 1 );
#endif
	result = iot_action_parameter_add( action, "new parameter",
		IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action->parameter_count, 1u );
	assert_string_equal( action->parameter[0].name, "new parameter" );
	assert_int_equal( action->parameter[0].data.type, IOT_TYPE_INT32 );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( action->parameter[0].name );
	os_free( action->parameter );
#endif
}

static void test_iot_action_parameter_add_parameters_full( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char name[IOT_NAME_MAX_LEN];

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
#ifdef IOT_STACK_ONLY
	action->parameter = action->_parameter;
#else
	will_return( __wrap_os_malloc, 1 );
	action->parameter = os_malloc(
		sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
#endif
	for ( i = 0u; i < IOT_PARAMETER_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		action->parameter = action->_parameter;
#else
		/* allocate space for option name */
		will_return( __wrap_os_malloc, 1 );
		action->parameter[i].name = os_malloc(
			sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
		snprintf( action->parameter[i].name, IOT_NAME_MAX_LEN,
			"parameter name %ld", i );
	}
	action->parameter_count = IOT_PARAMETER_MAX;
	result = iot_action_parameter_add( action,
		"new parameter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_FULL );
	assert_int_equal( action->parameter_count, IOT_PARAMETER_MAX );
	for ( i = 0u; i < IOT_PARAMETER_MAX; ++i )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
		assert_string_equal( action->parameter[i].name, name );
	}

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_PARAMETER_MAX; ++i )
		os_free( action->parameter[i].name );
	os_free( action->parameter );
#endif
}

static void test_iot_action_parameter_add_parameters_half_full( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char name[IOT_NAME_MAX_LEN];

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
#ifdef IOT_STACK_ONLY
	action->parameter = action->_parameter;
#else
	will_return( __wrap_os_malloc, 1 );
	action->parameter = os_malloc( sizeof( struct iot_action_parameter ) *
		( IOT_PARAMETER_MAX / 2 ) );
#endif
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; ++i )
	{
#ifdef IOT_STACK_ONLY
		action->parameter = action->_parameter;
#else
		/* allocate space for option name */
		will_return( __wrap_os_malloc, 1 );
		action->parameter[i].name = os_malloc(
			sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
#endif
		snprintf( action->parameter[i].name, IOT_NAME_MAX_LEN,
			"parameter name %ld", i );
	}
	action->parameter_count = IOT_PARAMETER_MAX / 2;

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 );
	will_return( __wrap_os_malloc, 1 );
#endif
	result = iot_action_parameter_add( action,
		"new parameter", IOT_PARAMETER_IN, IOT_TYPE_INT32, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action->parameter_count, IOT_PARAMETER_MAX / 2 + 1 );
	for ( i = 0u; i < IOT_PARAMETER_MAX / 2; ++i )
	{
		snprintf( name, IOT_NAME_MAX_LEN, "parameter name %ld", i );
		assert_string_equal( action->parameter[i].name, name );
	}
	assert_string_equal( action->parameter[IOT_PARAMETER_MAX / 2].name, "new parameter" );
	assert_int_equal( action->parameter[IOT_PARAMETER_MAX / 2].data.type, IOT_TYPE_INT32 );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < (IOT_PARAMETER_MAX / 2) + 1; ++i )
		os_free( action->parameter[i].name );
	os_free( action->parameter );
#endif
}

static void test_iot_action_parameter_get_not_found( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.uint8 = 4u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get( &request,
		"param3", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( arg, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_get_no_parameters( void **state )
{
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 0u;
	result = iot_action_parameter_get( &request,
		"param2", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( arg, 0u );
}

static void test_iot_action_parameter_get_null_name( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.uint8 = 4u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get( &request,
		NULL, IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( arg, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_get_null_request( void **state )
{
	iot_status_t result;
	iot_uint8_t arg;

	arg = 0u;
	result = iot_action_parameter_get( NULL,
		"param2", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( arg, 0u );
}

static void test_iot_action_parameter_get_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	iot_uint8_t arg;

	arg = 0u;
	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.uint8 = 4u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get( &request,
		"param2", IOT_FALSE, IOT_TYPE_UINT8, &arg );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( arg, 4u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_get_raw_null_data( void **state )
{
	size_t i;
	iot_status_t result;
	size_t length;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	length = 0u;
	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get_raw( &request, "param2", IOT_FALSE, &length, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_int_equal( length, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_get_raw_null_length( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const void *arg;
	const char data[10] = "some text\0";

	arg = NULL;
	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get_raw( &request, "param2", IOT_FALSE, NULL, &arg );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)arg, "some text" );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_get_raw_valid( void **state )
{
	size_t i;
	iot_status_t result;
	size_t length;
	iot_action_request_t request;
	const void *arg;
	const char data[10] = "some text\0";

	arg = NULL;
	length = 0u;
	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	request.parameter[1].type = IOT_PARAMETER_IN;
	result = iot_action_parameter_get_raw( &request, "param2", IOT_FALSE, &length, &arg );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( length, 10u );
	assert_string_equal( (const char *)arg, "some text" );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_bad_name( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set( &request, "param\\3", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_equal( request.parameter_count, 2u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_max_parameters( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = IOT_PARAMETER_MAX;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif

	for ( i = 0u; i < request.parameter_count; ++i )
	{
		char _name[IOT_NAME_MAX_LEN + 1u];
		snprintf( _name, IOT_NAME_MAX_LEN, "param%u", (unsigned int)i );
		strncpy( request.parameter[i].name, _name, IOT_NAME_MAX_LEN );
	}
	result = iot_action_parameter_set( &request, "param", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_FULL );
	assert_int_equal( request.parameter_count, IOT_PARAMETER_MAX );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_new_parameter( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	request.parameter = os_malloc( request.parameter_count *
		sizeof( struct iot_action_parameter ) );
	for ( i = 0u; i < request.parameter_count; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		request.parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}

	will_return( __wrap_os_realloc, 1 );
	will_return( __wrap_os_malloc, 1 );
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set( &request, "param3", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( request.parameter[2].data.value.uint16, 13u );
	assert_int_equal( request.parameter_count, 3u );
	assert_true( request.parameter[2].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[2].type & IOT_PARAMETER_OUT ) != 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < request.parameter_count; ++i )
		os_free( request.parameter[i].name );
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_null_name( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set( &request, NULL, IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_null_request( void **state )
{
	iot_status_t result;

	result = iot_action_parameter_set( NULL, "param2", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_set_type_null( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_NULL;
	result = iot_action_parameter_set( &request, "param2", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( request.parameter[1].data.type, IOT_TYPE_UINT16 );
	assert_int_equal( request.parameter[1].data.value.uint16, 13u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0 );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_type_wrong( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const iot_int32_t data = 1234;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_UINT16;
	result = iot_action_parameter_set( &request, "param2", IOT_TYPE_INT32, data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_int_not_equal( request.parameter[1].data.value.int32, data );
	assert_true( request.parameter[1].data.has_value == IOT_FALSE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) == 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_UINT16;
	result = iot_action_parameter_set( &request, "param2", IOT_TYPE_UINT16, 13u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( request.parameter[1].data.value.uint16, 13u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_raw_max_parameters( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = IOT_PARAMETER_MAX;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif

	for ( i = 0u; i < request.parameter_count; ++i )
	{
		char _name[ IOT_NAME_MAX_LEN + 1u ];
		snprintf( _name, IOT_NAME_MAX_LEN, "param%u", (unsigned)(i+1u) );
		strncpy( request.parameter[i].name, _name, IOT_NAME_MAX_LEN );
	}
	result = iot_action_parameter_set_raw( &request,
		"param", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_FULL );
	assert_int_equal( request.parameter_count, IOT_PARAMETER_MAX );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_raw_new_parameter( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	request.parameter = os_malloc( request.parameter_count *
		sizeof( struct iot_action_parameter )  );
	bzero( request.parameter, request.parameter_count *
		sizeof( struct iot_action_parameter ) );
	for ( i = 0u; i < request.parameter_count; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		request.parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif

	for ( i = 0u; i < request.parameter_count; ++i )
	{
		char _name[ IOT_NAME_MAX_LEN + 1u ];
		snprintf( _name, IOT_NAME_MAX_LEN, "param%u", (unsigned)(i + 1u ) );
		strncpy( request.parameter[i].name, _name, IOT_NAME_MAX_LEN );
	}

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 );
	will_return( __wrap_os_malloc, 1 );
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_action_parameter_set_raw( &request, "param", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)request.parameter[2].data.value.raw.ptr, "some text" );
	assert_int_equal( request.parameter[2].data.value.raw.length, 10u );
	assert_int_equal( request.parameter_count, 3u );
	assert_true( request.parameter[2].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[2].type & IOT_PARAMETER_OUT ) != 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < request.parameter_count; ++i )
	{
		os_free( request.parameter[i].name );
		os_free( request.parameter[i].data.heap_storage );
	}
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_raw_null_data( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	result = iot_action_parameter_set_raw( &request, "param2", 10u, NULL );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_raw_null_name( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	result = iot_action_parameter_set_raw( &request, NULL, 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_raw_null_request( void **state )
{
	iot_status_t result;
	const char data[10] = "some text\0";

	result = iot_action_parameter_set_raw( NULL, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_parameter_set_raw_type_null( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	request.parameter = os_malloc( request.parameter_count *
		sizeof( struct iot_action_parameter )  );
	bzero( request.parameter, request.parameter_count *
		sizeof( struct iot_action_parameter ) );
	for ( i = 0u; i < request.parameter_count; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		request.parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_NULL;

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_action_parameter_set_raw( &request, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)request.parameter[1].data.value.raw.ptr, "some text" );
	assert_int_equal( request.parameter[1].data.value.raw.length, 10u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < request.parameter_count; ++i )
	{
		os_free( request.parameter[i].name );
		os_free( request.parameter[i].data.heap_storage );
	}
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_raw_type_wrong( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	request.parameter = test_malloc( request.parameter_count *
		(sizeof( struct iot_action_parameter ) + IOT_NAME_MAX_LEN + 1u) );
	assert_non_null( request.parameter );
	request.parameter[0].name = (char*)request.parameter +
		(sizeof( struct iot_action_parameter ) * request.parameter_count);
	for ( i = 1u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i-1u].name + IOT_NAME_MAX_LEN + 1u;
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_UINT8;
	result = iot_action_parameter_set_raw( &request, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_BAD_REQUEST );
	assert_true( request.parameter[1].data.has_value == IOT_FALSE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) == 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( request.parameter );
#endif
}

static void test_iot_action_parameter_set_raw_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
	for ( i = 0u; i < request.parameter_count; ++i )
		request.parameter[i].name = request.parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	request.parameter = os_malloc( request.parameter_count *
		sizeof( struct iot_action_parameter )  );
	bzero( request.parameter, request.parameter_count *
		sizeof( struct iot_action_parameter ) );
	for ( i = 0u; i < request.parameter_count; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		request.parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( request.parameter[0].name, "param1", IOT_NAME_MAX_LEN );
	strncpy( request.parameter[1].name, "param2", IOT_NAME_MAX_LEN );
	request.parameter[1].data.has_value = IOT_FALSE;
	request.parameter[1].data.type = IOT_TYPE_RAW;

#ifndef IOT_STACK_ONLY
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_action_parameter_set_raw( &request, "param2", 10u, (const void *)data );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( (const char *)request.parameter[1].data.value.raw.ptr, "some text" );
	assert_int_equal( request.parameter[1].data.value.raw.length, 10u );
	assert_true( request.parameter[1].data.has_value == IOT_TRUE );
	assert_true( ( request.parameter[1].type & IOT_PARAMETER_OUT ) != 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < request.parameter_count; ++i )
	{
		os_free( request.parameter[i].name );
		os_free( request.parameter[i].data.heap_storage );
	}
	os_free( request.parameter );
#endif
}

static void test_iot_action_process_actions_empty( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 0u;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_error, "Not Found" );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );
}

static void test_iot_action_process_actions_full( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = IOT_ACTION_STACK_MAX;
	for ( i = 0u; i < lib.action_count; ++i )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	snprintf( lib.request_queue_wait[0]->name,
	          IOT_NAME_MAX_LEN,
	          "action name %d",
	          IOT_ACTION_STACK_MAX / 2 );
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_actions_not_found( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 1u; i < lib.action_count; ++i )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_error, "Not Found" );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_command_no_return( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->flags = IOT_ACTION_NO_RETURN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_os_system_run_wait, command, "script_path" );
	will_return( __wrap_os_system_run_wait, 0u ); /* IOT_STATUS_INVOKED ); */
	will_return( __wrap_os_system_run_wait, IOT_STATUS_INVOKED );
	/*will_return( __wrap_os_system_run_wait, IOT_STATUS_INVOKED );*/
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		os_free( lib.action[i].command );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_bool( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "bool", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_BOOL;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	lib.request_queue_wait[0]->parameter[0].name = lib.request_queue_wait[0]->parameter[0]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter[0].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "bool", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_BOOL;
	lib.request_queue_wait[0]->parameter[0].data.value.boolean = IOT_TRUE;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	expect_string( __wrap_os_system_run_wait, command, "script_path --bool=1" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_float( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 2u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "float32", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_FLOAT32;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[1].name, "float64", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[1].data.type = IOT_TYPE_FLOAT64;
	lib.action_ptr[0]->parameter[1].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	lib.request_queue_wait[0]->parameter[0].name = lib.request_queue_wait[0]->parameter[0]._name;
	lib.request_queue_wait[0]->parameter[1].name = lib.request_queue_wait[0]->parameter[1]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) * 2u );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) * 2u );
	for ( i = 0u; i < 2u; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue_wait[0]->parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 2u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "float32", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_FLOAT32;
	lib.request_queue_wait[0]->parameter[0].data.value.float32 = 32.32f;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[1].name, "float64", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[1].data.type = IOT_TYPE_FLOAT64;
	lib.request_queue_wait[0]->parameter[1].data.value.float64 = 64.64;
	lib.request_queue_wait[0]->parameter[1].data.has_value = IOT_TRUE;
	expect_string( __wrap_os_system_run_wait, command,
		"script_path --float32=32.320000 --float64=64.640000" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_int( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 4u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "int8", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_INT8;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[1].name, "int16", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[1].data.type = IOT_TYPE_INT16;
	lib.action_ptr[0]->parameter[1].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[2].name, "int32", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[2].data.type = IOT_TYPE_INT32;
	lib.action_ptr[0]->parameter[2].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[3].name, "int64", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[3].data.type = IOT_TYPE_INT64;
	lib.action_ptr[0]->parameter[3].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	for ( i = 0u; i < 4u; ++i )
		lib.request_queue_wait[0]->parameter[i].name = lib.request_queue_wait[0]->parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) * 4u );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) * 4u );
	for ( i = 0u; i < 4u; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue_wait[0]->parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 4u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "int8", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_INT8;
	lib.request_queue_wait[0]->parameter[0].data.value.int8 = 8;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[1].name, "int16", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[1].data.type = IOT_TYPE_INT16;
	lib.request_queue_wait[0]->parameter[1].data.value.int16 = 16;
	lib.request_queue_wait[0]->parameter[1].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[2].name, "int32", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[2].data.type = IOT_TYPE_INT32;
	lib.request_queue_wait[0]->parameter[2].data.value.int32 = 32;
	lib.request_queue_wait[0]->parameter[2].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[3].name, "int64", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[3].data.type = IOT_TYPE_INT64;
	lib.request_queue_wait[0]->parameter[3].data.value.int64 = 64;
	lib.request_queue_wait[0]->parameter[3].data.has_value = IOT_TRUE;
	expect_string( __wrap_os_system_run_wait, command,
		"script_path --int8=8 --int16=16 --int32=32 --int64=64" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_location( void **state )
{
	size_t i;
	iot_t lib;
	struct iot_location *loc;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_LOCATION;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	for ( i = 0u; i < 1u; ++i )
		lib.request_queue_wait[0]->parameter[i].name = lib.request_queue_wait[0]->parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) * 4u );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) * 4u );
	for ( i = 0u; i < 1u; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue_wait[0]->parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_LOCATION;
	loc = test_malloc( sizeof( struct iot_location ) );
	assert_non_null( loc );
	loc->longitude = 40.446195;
	loc->latitude = -79.982195;
	lib.request_queue_wait[0]->parameter[0].data.heap_storage = loc;
	lib.request_queue_wait[0]->parameter[0].data.value.location = loc;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	expect_string( __wrap_os_system_run_wait, command,
		"script_path --param=[40.446195,-79.982195]" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_null( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_NULL;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	for ( i = 0u; i < 1u; ++i )
		lib.request_queue_wait[0]->parameter[i].name = lib.request_queue_wait[0]->parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) * 4u );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) * 4u );
	for ( i = 0u; i < 1u; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue_wait[0]->parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_NULL;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	expect_string( __wrap_os_system_run_wait, command, "script_path --param=[NULL]" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_raw( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_RAW;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	for ( i = 0u; i < 1u; ++i )
		lib.request_queue_wait[0]->parameter[i].name = lib.request_queue_wait[0]->parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) * 4u );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) * 4u );
	for ( i = 0u; i < 1u; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue_wait[0]->parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_RAW;
	lib.request_queue_wait[0]->parameter[0].data.heap_storage = test_malloc( sizeof( char ) * 25 );
	lib.request_queue_wait[0]->parameter[0].data.value.raw.ptr =
	    lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy(
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage, "raw data value", 25 );
	lib.request_queue_wait[0]->parameter[0].data.value.raw.length = 14u;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( __wrap_iot_base64_encode, 8u );
	expect_string( __wrap_os_system_run_wait, command,
		"script_path --param=bbbbbbbb" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_string( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 1u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	for ( i = 0u; i < 1u; ++i )
		lib.request_queue_wait[0]->parameter[i].name = lib.request_queue_wait[0]->parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) * 4u );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) * 4u );
	for ( i = 0u; i < 1u; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue_wait[0]->parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.heap_storage = test_malloc( sizeof( char ) * 25 );
	assert_non_null( lib.request_queue_wait[0]->parameter[0].data.heap_storage );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "string\r\n \\ \"value\"",
	         25 );
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	expect_string( __wrap_os_system_run_wait, command,
		"script_path --param=\"string \\\\ \\\"value\\\"\"" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_parameter_uint( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter_count = 4u;
	strncpy( lib.action_ptr[0]->parameter[0].name, "uint8", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[0].data.type = IOT_TYPE_UINT8;
	lib.action_ptr[0]->parameter[0].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[1].name, "uint16", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[1].data.type = IOT_TYPE_UINT16;
	lib.action_ptr[0]->parameter[1].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[2].name, "uint32", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[2].data.type = IOT_TYPE_UINT32;
	lib.action_ptr[0]->parameter[2].type = IOT_PARAMETER_IN;
	strncpy( lib.action_ptr[0]->parameter[3].name, "uint64", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->parameter[3].data.type = IOT_TYPE_UINT64;
	lib.action_ptr[0]->parameter[3].type = IOT_PARAMETER_IN;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter = lib.request_queue_wait[0]->_parameter;
	for ( i = 0u; i < 4u; ++i )
		lib.request_queue_wait[0]->parameter[i].name = lib.request_queue_wait[0]->parameter[i]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc( sizeof( struct iot_action_parameter ) * 4u );
	bzero( lib.request_queue_wait[0]->parameter, sizeof( struct iot_action_parameter ) * 4u );
	for ( i = 0u; i < 4u; ++i )
	{
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue_wait[0]->parameter[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	}
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 4u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "uint8", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_UINT8;
	lib.request_queue_wait[0]->parameter[0].data.value.uint8 = 8u;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[1].name, "uint16", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[1].data.type = IOT_TYPE_UINT16;
	lib.request_queue_wait[0]->parameter[1].data.value.uint16 = 16u;
	lib.request_queue_wait[0]->parameter[1].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[2].name, "uint32", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[2].data.type = IOT_TYPE_UINT32;
	lib.request_queue_wait[0]->parameter[2].data.value.uint32 = 32u;
	lib.request_queue_wait[0]->parameter[2].data.has_value = IOT_TRUE;
	strncpy( lib.request_queue_wait[0]->parameter[3].name, "uint64", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[3].data.type = IOT_TYPE_UINT64;
	lib.request_queue_wait[0]->parameter[3].data.value.uint64 = 64u;
	lib.request_queue_wait[0]->parameter[3].data.has_value = IOT_TRUE;
	expect_string( __wrap_os_system_run_wait, command,
		"script_path --uint8=8 --uint16=16 --uint32=32 --uint64=64" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		os_free( lib.action[i].command );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_script_return_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_os_system_run_wait, command, "script_path" );
	will_return( __wrap_os_system_run_wait, 1u ); /* script exit status */
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		os_free( lib.action[i].command );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_system_run_fail( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_os_system_run_wait, command, "script_path" );
	will_return( __wrap_os_system_run_wait, -1 );
	will_return( __wrap_os_system_run_wait, "\0" );
	will_return( __wrap_os_system_run_wait, "\0" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_NOT_EXECUTABLE );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		os_free( lib.action[i].command );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_command_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].command = lib.action[i]._command;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].command = os_malloc( PATH_MAX + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	strncpy( lib.action_ptr[0]->command, "script_path", PATH_MAX );
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	expect_string( __wrap_os_system_run_wait, command, "script_path" );
	will_return( __wrap_os_system_run_wait, 0u );
	will_return( __wrap_os_system_run_wait, "this is stdout" );
	will_return( __wrap_os_system_run_wait, "this is stderr" );
	will_return( __wrap_os_system_run_wait, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* add parameter: retval */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	/* add parameter: stdout */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
	/* add parameter: stderr */
	will_return( __wrap_os_realloc, 1 ); /* increase parameter array */
	will_return( __wrap_os_malloc, 1 ); /* space to hold name */
	will_return( __wrap_os_realloc, 1 ); /* copy string value */
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		os_free( lib.action[i].command );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_exclusive( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.action_ptr[0]->flags = IOT_ACTION_EXCLUSIVE_APP;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_lib_to_quit( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.to_quit = IOT_TRUE;
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_no_handler( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = NULL;
	lib.action_ptr[0]->command = NULL;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* space to store error message */
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_null_lib( void **state )
{
	iot_status_t result;

	result = iot_action_process( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_process_options( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; ++i )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->option =
		lib.request_queue_wait[0]->_option;
	lib.request_queue_wait[0]->option[0].name =
		lib.request_queue_wait[0]->option[0]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->option =
		os_malloc( sizeof( struct iot_option ) );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->option[0].name =
		os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->option_count = 1u;
	strncpy( lib.request_queue_wait[0]->option[0].name, "attr", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->option[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	assert_non_null( lib.request_queue_wait[0]->option[0].data.heap_storage );
	lib.request_queue_wait[0]->option[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->option[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->option[0].data.heap_storage,
	         "some text",
	         IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->option[0].data.type = IOT_TYPE_STRING;
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_parameters_bad_type( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; ++i )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %lu", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.action_ptr[1]->parameter_count = 1u;
	strncpy( lib.action_ptr[1]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[1]->parameter[0].type = IOT_PARAMETER_IN_REQUIRED;
	lib.action_ptr[1]->parameter[0].data.type = IOT_TYPE_INT32;
	lib.action_ptr[1]->parameter[0].data.has_value = IOT_FALSE;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter =
		lib.request_queue_wait[0]->_parameter;
	lib.request_queue_wait[0]->parameter[0].name =
		lib.request_queue_wait[0]->parameter[0]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter =
		os_malloc( sizeof( struct iot_action_parameter ) );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter[0].name =
		os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "some text", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
#ifndef IOT_STACK_ONLY
	/* space to store error message */
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_parameters_missing_required( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].parameter = lib.action[i]._parameter;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			lib.action[i].parameter[j].name =
				lib.action[i].parameter[j]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * IOT_PARAMETER_MAX );
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
		{
			will_return( __wrap_os_malloc, 1 );
			lib.action[i].parameter[j].name =
				os_malloc( IOT_NAME_MAX_LEN + 1u );
		}
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; ++i )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
		lib.action_ptr[i]->parameter_count = 0u;
	}
	lib.action_ptr[1]->parameter_count = 1u;
	strncpy( lib.action_ptr[1]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[1]->parameter[0].type = IOT_PARAMETER_IN_REQUIRED;
	lib.action_ptr[1]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.action_ptr[1]->parameter[0].data.has_value = IOT_FALSE;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name,
		lib.action_ptr[1]->name, IOT_NAME_MAX_LEN );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );

#ifndef IOT_STACK_ONLY
	/* space to store error message */
	will_return( __wrap_os_realloc, 1 );
#endif
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		size_t j;
		for ( j = 0u; j < IOT_PARAMETER_MAX; ++j )
			os_free( lib.action[i].parameter[j].name );
		os_free( lib.action[i].parameter );
		os_free( lib.action[i].name );
	}
#endif
}

static void test_iot_action_process_parameters_undeclared( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; ++i )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %ld", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter =
		lib.request_queue_wait[0]->_parameter;
	lib.request_queue_wait[0]->parameter[0].name =
		lib.request_queue_wait[0]->parameter[0]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc(
		sizeof( struct iot_action_parameter ) * 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter[0].name =
		os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "some text",
	         IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_parameters_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
		lib.action[i].parameter = lib.action[i]._parameter;
		lib.action[i].parameter[0].name = lib.action[i].parameter[0]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter = os_malloc(
			sizeof( struct iot_action_parameter ) * 1u );
		bzero( lib.action[i].parameter,
			sizeof( struct iot_action_parameter ) * 1u );
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].parameter[0].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		bzero( lib.action[i].name, IOT_NAME_MAX_LEN + 1u );
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = IOT_ACTION_STACK_MAX - 1u;
	for ( i = 0u; i < lib.action_count; ++i )
	{
		snprintf( lib.action_ptr[i]->name, IOT_NAME_MAX_LEN, "action name %lu", i );
		lib.action_ptr[i]->lib = &lib;
		lib.action_ptr[i]->callback = &test_callback_func;
	}
	lib.action_ptr[1]->parameter_count = 1u;
	strncpy( lib.action_ptr[1]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.action_ptr[1]->parameter[0].type = IOT_PARAMETER_IN_REQUIRED;
	lib.action_ptr[1]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.action_ptr[1]->parameter[0].data.has_value = IOT_FALSE;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
#ifdef IOT_STACK_ONLY
	lib.request_queue_wait[0]->name = lib.request_queue_wait[0]->_name;
	lib.request_queue_wait[0]->parameter =
		lib.request_queue_wait[0]->_parameter;
	lib.request_queue_wait[0]->parameter[0].name =
		lib.request_queue_wait[0]->parameter[0]._name;
#else
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->name = os_malloc( IOT_NAME_MAX_LEN + 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter = os_malloc(
		sizeof( struct iot_action_parameter ) * 1u );
	bzero( lib.request_queue_wait[0]->parameter,
		sizeof( struct iot_action_parameter ) * 1u );
	will_return( __wrap_os_malloc, 1 );
	lib.request_queue_wait[0]->parameter[0].name =
		os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
	strncpy( lib.request_queue_wait[0]->name, "action name 1", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter_count = 1u;
	strncpy( lib.request_queue_wait[0]->parameter[0].name, "param", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.heap_storage =
	    test_malloc( ( IOT_NAME_MAX_LEN + 1 ) * sizeof( char ) );
	assert_non_null( lib.request_queue_wait[0]->parameter[0].data.heap_storage );
	lib.request_queue_wait[0]->parameter[0].data.value.string =
	    (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage;
	strncpy( (char *)lib.request_queue_wait[0]->parameter[0].data.heap_storage,
	         "some text", IOT_NAME_MAX_LEN );
	lib.request_queue_wait[0]->parameter[0].data.type = IOT_TYPE_STRING;
	lib.request_queue_wait[0]->parameter[0].data.has_value = IOT_TRUE;
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
		os_free( lib.action[i].name );
		os_free( lib.action[i].parameter[0].name );
		os_free( lib.action[i].parameter );
	}
#endif
}

static void test_iot_action_process_valid( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		bzero( lib.action[i].name, IOT_NAME_MAX_LEN + 1u );
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 1u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 1u;
	lib.request_queue_free_count = 1u;
	for ( i = 0u; i < lib.request_queue_wait_count; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.request_queue[i].name = lib.request_queue[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		bzero( lib.request_queue[i].name, IOT_NAME_MAX_LEN + 1u );
		lib.request_queue_wait[i] = &lib.request_queue[i];
	}
	strncpy( lib.request_queue_wait[0]->name, "action name", IOT_NAME_MAX_LEN );
	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_wait_queue_empty( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action name", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;
	lib.request_queue_wait[0] = &lib.request_queue[0];
	for ( i = 0u; i < IOT_ACTION_QUEUE_MAX; ++i )
		lib.request_queue_free[i] = &lib.request_queue[i];
	lib.request_queue_wait_count = 0u;
	lib.request_queue_free_count = 0u;
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_FOUND );
	assert_int_equal( lib.request_queue_wait_count, 0u );
	assert_int_equal( lib.request_queue_free_count, 0u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
#endif
}

static void test_iot_action_process_wait_queue_full( void **state )
{
	size_t i;
	iot_t lib;
	iot_status_t result;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.action[i].name = lib.action[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.action[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		lib.action_ptr[i] = &lib.action[i];
	}
	lib.action_count = 1u;
	strncpy( lib.action_ptr[0]->name, "action 1", IOT_NAME_MAX_LEN );
	lib.action_ptr[0]->lib = &lib;
	lib.action_ptr[0]->callback = &test_callback_func;

	/* number of jobs that are waiting */
	lib.request_queue_wait_count = IOT_ACTION_QUEUE_MAX;
	/* number of spaces that are free */
	lib.request_queue_free_count = IOT_ACTION_QUEUE_MAX;
	for ( i = 0u; i < IOT_ACTION_QUEUE_MAX; ++i )
	{
#ifdef IOT_STACK_ONLY
		lib.request_queue[i].name = lib.request_queue[i]._name;
#else
		will_return( __wrap_os_malloc, 1 );
		lib.request_queue[i].name = os_malloc( IOT_NAME_MAX_LEN + 1u );
#endif
		snprintf( lib.request_queue[i].name, IOT_NAME_MAX_LEN,
			"action %u", (unsigned)(i+1u) );
		lib.request_queue_wait[i] = &lib.request_queue[i];
	}

	will_return( test_callback_func, IOT_STATUS_SUCCESS );
	/* perform action */
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_process( &lib, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );

	/* 1 less waiting, as it's now put into the working queue */
	assert_int_equal( lib.request_queue_wait_count, IOT_ACTION_QUEUE_MAX - 1u );
	assert_int_equal( lib.request_queue_free_count, IOT_ACTION_QUEUE_MAX - 1u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		os_free( lib.action[i].name );
	for ( i = 0u; i < IOT_ACTION_QUEUE_MAX; ++i )
		os_free( lib.request_queue[i].name );
#endif
}

static void test_iot_action_register_callback_null_action( void **state )
{
	iot_status_t result;
	char data[10] = "some text\0";

	result = iot_action_register_callback( NULL, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_register_callback_null_lib( void **state )
{
	iot_status_t result;
	iot_action_t action;
	char data[10] = "some text\0";

	bzero( &action, sizeof( iot_action_t ) );
	action.state = IOT_ITEM_DEREGISTERED;
	action.lib = NULL;
	result = iot_action_register_callback( &action, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( action.state, IOT_ITEM_DEREGISTERED );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
	assert_non_null( action.user_data );
	assert_ptr_equal( action.user_data, data );
	assert_ptr_equal( action.callback, &test_callback_func );
}

static void test_iot_action_register_callback_transmit_fail( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char data[10] = "some text\0";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_DEREGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_action_register_callback( action, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_REGISTER_PENDING );
	assert_int_equal( result, IOT_STATUS_FAILURE );
	assert_non_null( action->user_data );
	assert_ptr_equal( action->user_data, data );
	assert_ptr_equal( action->callback, &test_callback_func );
}

static void test_iot_action_register_callback_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;
	char data[10] = "some text\0";

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->lib = &lib;
	action->state = IOT_ITEM_DEREGISTERED;
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_register_callback( action, &test_callback_func, (void *)data, NULL, 0u );
	assert_int_equal( action->state, IOT_ITEM_REGISTERED );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_non_null( action->user_data );
	assert_ptr_equal( action->user_data, data );
	assert_ptr_equal( action->callback, &test_callback_func );
}

static void test_iot_action_register_command_null_action( void **state )
{
	iot_status_t result;

	result = iot_action_register_command( NULL, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_register_command_null_lib( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->state = IOT_ITEM_DEREGISTERED;
	action->lib = NULL;
	result = iot_action_register_command( action, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_register_command_transmit_fail( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->state = IOT_ITEM_DEREGISTERED;
	action->lib = &lib;
#ifndef IOT_STACK_ONLY
	/* command path */
	will_return( __wrap_os_realloc, 1 );
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_FAILURE );
	result = iot_action_register_command( action, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_FAILURE );
	assert_int_equal( action->state, IOT_ITEM_REGISTER_PENDING );
	assert_null( action->user_data );
	assert_null( action->callback );
	assert_string_equal( action->command, "script_path" );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( action->command );
#endif
}

static void test_iot_action_register_command_valid( void **state )
{
	size_t i;
	iot_status_t result;
	iot_t lib;
	iot_action_t *action;

	bzero( &lib, sizeof( iot_t ) );
	for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
		lib.action_ptr[i] = &lib.action[i];
	lib.action_count = 1u;
	action = lib.action_ptr[0];
	action->state = IOT_ITEM_DEREGISTERED;
	action->lib = &lib;
#ifndef IOT_STACK_ONLY
	/* command path */
	will_return( __wrap_os_realloc, 1 );
#endif
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_register_command( action, "script_path", NULL, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_int_equal( action->state, IOT_ITEM_REGISTERED );
	assert_null( action->user_data );
	assert_null( action->callback );
	assert_string_equal( action->command, "script_path" );

	/* clean up */
#ifndef IOT_STACK_ONLY
	os_free( action->command );
#endif
}

static void test_iot_action_request_copy_raw( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[10] = "some text\0";
	char data2[15] = "even more text\0";
	const size_t data_dest_size = ( sizeof( char ) * 25u ) +
		( sizeof( struct iot_action_parameter ) * 3u );
	void *data_dest = test_malloc( data_dest_size );

	bzero( data_dest, data_dest_size );
	bzero( &dest, sizeof( iot_action_request_t ) );
	bzero( &src, sizeof( iot_action_request_t ) );
	src.parameter_count = 3u;
#ifdef IOT_STACK_ONLY
	src.name = src._name;
	src.parameter = src._parameter;
#else
	src.name = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
	assert_non_null( src.name );
	src.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		src.parameter_count );
	assert_non_null( src.parameter );
	bzero( src.parameter, sizeof( struct iot_action_parameter ) *
		src.parameter_count );
#endif
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter[0].data.has_value = IOT_FALSE;
	src.parameter[1].data.value.raw.ptr = (void *)data1;
	src.parameter[1].data.value.raw.length = 10u;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_RAW;
	src.parameter[2].data.value.raw.ptr = (void *)data2;
	src.parameter[2].data.value.raw.length = 15u;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_RAW;
	result = iot_action_request_copy( &dest, &src, data_dest, data_dest_size );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( dest.name, "thisisarequest" );
	assert_string_equal( (const char *)dest.parameter[1].data.value.raw.ptr, "some text" );
	assert_string_equal( (const char *)dest.parameter[2].data.value.raw.ptr, "even more text" );
	assert_int_equal( dest.parameter[1].data.value.raw.length, 10u );
	assert_int_equal( dest.parameter[2].data.value.raw.length, 15u );

	/* clean up */
	test_free( data_dest );
#ifndef IOT_STACK_ONLY
	test_free( src.parameter );
	test_free( src.name );
#endif
}

static void test_iot_action_request_copy_raw_too_big( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[50] = "gaerioivonerilmtgruiesnchyuiolhtrsthgherhskgownvd\0";
	char data2[50] = "srcshkltbhmirshltilhtnkgfjkhsurthlesghrgjsehrgeor\0";
	char data3[50] = "tyrjeiojtgiohjgidhgfkldjhgkguqiwueifgaehthrshkger\0";
	char data4[50] = "rjeklarieoghirovhieaojruiehafuileghuielghaurilgre\0";
	char data5[50] = "vbtiroangrheaugjrkeanrgjhkealgurheagrtejhnaihruei\0";
	char data6[50] = "grteksxdvfhwjbyrheuknguitrmnuyhtnirsuibgrseukgrfe\0";
	void *data_dest = test_malloc( sizeof( char ) * 200 );

	bzero( &dest, sizeof( iot_action_request_t ) );
	bzero( &src, sizeof( iot_action_request_t ) );
	bzero( data_dest, sizeof( char ) * 200 );
	src.parameter_count = 6u;
#ifdef IOT_STACK_ONLY
	src.name = src._name;
	src.parameter = src._parameter;
#else
	src.name = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
	assert_non_null( src.name );
	src.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		src.parameter_count );
	assert_non_null( src.parameter );
	bzero( src.parameter, sizeof( struct iot_action_parameter ) *
		src.parameter_count );
#endif
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter[0].data.value.raw.ptr = (void *)data1;
	src.parameter[0].data.value.raw.length = 50u;
	src.parameter[0].data.has_value = IOT_TRUE;
	src.parameter[0].data.type = IOT_TYPE_RAW;
	src.parameter[1].data.value.raw.ptr = (void *)data2;
	src.parameter[1].data.value.raw.length = 50u;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_RAW;
	src.parameter[2].data.value.raw.ptr = (void *)data3;
	src.parameter[2].data.value.raw.length = 50u;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_RAW;
	src.parameter[3].data.value.raw.ptr = (void *)data4;
	src.parameter[3].data.value.raw.length = 50u;
	src.parameter[3].data.has_value = IOT_TRUE;
	src.parameter[3].data.type = IOT_TYPE_RAW;
	src.parameter[4].data.value.raw.ptr = (void *)data5;
	src.parameter[4].data.value.raw.length = 50u;
	src.parameter[4].data.has_value = IOT_TRUE;
	src.parameter[4].data.type = IOT_TYPE_RAW;
	src.parameter[5].data.value.raw.ptr = (void *)data6;
	src.parameter[5].data.value.raw.length = 50u;
	src.parameter[5].data.has_value = IOT_TRUE;
	src.parameter[5].data.type = IOT_TYPE_RAW;
	result = iot_action_request_copy( &dest, &src, (void *)data_dest, 200u );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );
	assert_string_equal( dest.name, "thisisarequest" );

	/* clean up */
	test_free( data_dest );
#ifndef IOT_STACK_ONLY
	test_free( src.parameter );
	test_free( src.name );
#endif
}

static void test_iot_action_request_copy_string( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[10] = "some text\0";
	char data2[15] = "even more text\0";
	const size_t data_dest_size = ( sizeof( char ) * 25u ) +
		( sizeof( struct iot_action_parameter ) * 3u );
	void *data_dest = test_malloc( data_dest_size );
	assert_non_null( data_dest );

	bzero( data_dest, data_dest_size );
	bzero( &dest, sizeof( iot_action_request_t ) );
	bzero( &src, sizeof( iot_action_request_t ) );
	src.parameter_count = 3u;
#ifdef IOT_STACK_ONLY
	src.name = src._name;
	src.parameter = src._parameter;
#else
	src.name = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
	assert_non_null( src.name );
	src.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		src.parameter_count );
	assert_non_null( src.parameter );
	bzero( src.parameter, sizeof( struct iot_action_parameter ) *
		src.parameter_count );
#endif
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter[0].data.has_value = IOT_FALSE;
	src.parameter[1].data.value.string = data1;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_STRING;
	src.parameter[2].data.value.string = data2;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_STRING;
	result = iot_action_request_copy( &dest, &src, data_dest, data_dest_size );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
	assert_string_equal( dest.name, "thisisarequest" );
	assert_string_equal( dest.parameter[1].data.value.string, "some text" );
	assert_string_equal( dest.parameter[2].data.value.string, "even more text" );

	/* clean up */
	test_free( data_dest );
#ifndef IOT_STACK_ONLY
	test_free( src.parameter );
	test_free( src.name );
#endif
}

static void test_iot_action_request_copy_string_too_big( void **state )
{
	iot_action_request_t dest;
	iot_action_request_t src;
	iot_status_t result;
	char data1[50] = "gaerioivonerilmtgruiesnchyuiolhtrsthgherhskgownvd\0";
	char data2[50] = "srcshkltbhmirshltilhtnkgfjkhsurthlesghrgjsehrgeor\0";
	char data3[50] = "tyrjeiojtgiohjgidhgfkldjhgkguqiwueifgaehthrshkger\0";
	char data4[50] = "rjeklarieoghirovhieaojruiehafuileghuielghaurilgre\0";
	char data5[50] = "vbtiroangrheaugjrkeanrgjhkealgurheagrtejhnaihruei\0";
	char data6[50] = "grteksxdvfhwjbyrheuknguitrmnuyhtnirsuibgrseukgrfe\0";
	void *data_dest = test_malloc( sizeof( char ) * 200 );
	assert_non_null( data_dest );

	bzero( &dest, sizeof( iot_action_request_t ) );
	bzero( &src, sizeof( iot_action_request_t ) );
	bzero( data_dest, sizeof( char ) * 200 );
	src.parameter_count = 6u;
#ifdef IOT_STACK_ONLY
	src.name = src._name;
	src.parameter = src._parameter;
#else
	src.name = test_malloc( sizeof( char ) * ( IOT_NAME_MAX_LEN + 1u ) );
	assert_non_null( src.name );
	src.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		src.parameter_count );
	assert_non_null( src.parameter );
	bzero( src.parameter, sizeof( struct iot_action_parameter ) *
		src.parameter_count );
#endif
	strncpy( src.name, "thisisarequest", IOT_NAME_MAX_LEN );
	src.parameter[0].data.value.string = data1;
	src.parameter[0].data.has_value = IOT_TRUE;
	src.parameter[0].data.type = IOT_TYPE_STRING;
	src.parameter[1].data.value.string = data2;
	src.parameter[1].data.has_value = IOT_TRUE;
	src.parameter[1].data.type = IOT_TYPE_STRING;
	src.parameter[2].data.value.string = data3;
	src.parameter[2].data.has_value = IOT_TRUE;
	src.parameter[2].data.type = IOT_TYPE_STRING;
	src.parameter[3].data.value.string = data4;
	src.parameter[3].data.has_value = IOT_TRUE;
	src.parameter[3].data.type = IOT_TYPE_STRING;
	src.parameter[4].data.value.string = data5;
	src.parameter[4].data.has_value = IOT_TRUE;
	src.parameter[4].data.type = IOT_TYPE_STRING;
	src.parameter[5].data.value.string = data6;
	src.parameter[5].data.has_value = IOT_TRUE;
	src.parameter[5].data.type = IOT_TYPE_STRING;
	result = iot_action_request_copy( &dest, &src, (void *)data_dest, 200u );
	assert_int_equal( result, IOT_STATUS_NO_MEMORY );

	/* clean up */
	test_free( data_dest );
#ifndef IOT_STACK_ONLY
	test_free( src.parameter );
	test_free( src.name );
#endif
}

static void test_iot_action_request_copy_size_no_parameters( void **state )
{
	size_t size;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 0u;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, 0u );
}

static void test_iot_action_request_copy_size_no_pointers( void **state )
{
	size_t size;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
#else
	request.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		request.parameter_count );
	assert_non_null( request.parameter );
	bzero( request.parameter, sizeof( struct iot_action_parameter ) *
		request.parameter_count );
#endif
	request.parameter[0].data.value.uint8 = 4u;
	request.parameter[0].data.has_value = IOT_TRUE;
	request.parameter[0].data.type = IOT_TYPE_UINT8;
	request.parameter[1].data.value.float32 = 12.34f;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_FLOAT32;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, sizeof( struct iot_action_parameter ) *
		request.parameter_count );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( request.parameter );
#endif
}

static void test_iot_action_request_copy_size_no_values( void **state )
{
	size_t size;
	iot_action_request_t request;

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
#else
	request.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		request.parameter_count );
	assert_non_null( request.parameter );
	bzero( request.parameter, sizeof( struct iot_action_parameter ) *
		request.parameter_count );
#endif
	request.parameter[0].data.has_value = IOT_FALSE;
	request.parameter[1].data.has_value = IOT_FALSE;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, sizeof( struct iot_action_parameter ) *
		request.parameter_count );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( request.parameter );
#endif
}

static void test_iot_action_request_copy_size_null_request( void **state )
{
	size_t size;

	size = iot_action_request_copy_size( NULL );
	assert_int_equal( size, 0u );
}

static void test_iot_action_request_copy_size_raw( void **state )
{
	size_t size;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
#else
	request.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		request.parameter_count );
	assert_non_null( request.parameter );
	bzero( request.parameter, sizeof( struct iot_action_parameter ) *
		request.parameter_count );
#endif
	request.parameter[0].data.has_value = IOT_FALSE;
	request.parameter[1].data.value.raw.ptr = (const void *)data;
	request.parameter[1].data.value.raw.length = 10u;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_RAW;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, ( sizeof( struct iot_action_parameter ) *
		request.parameter_count ) + 10u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( request.parameter );
#endif
}

static void test_iot_action_request_copy_size_string( void **state )
{
	size_t size;
	iot_action_request_t request;
	const char data[10] = "some text\0";

	bzero( &request, sizeof( iot_action_request_t ) );
	request.parameter_count = 2u;
#ifdef IOT_STACK_ONLY
	request.parameter = request._parameter;
#else
	request.parameter = test_malloc( sizeof( struct iot_action_parameter ) *
		request.parameter_count );
	assert_non_null( request.parameter );
	bzero( request.parameter, sizeof( struct iot_action_parameter ) *
		request.parameter_count );
#endif
	request.parameter[0].data.has_value = IOT_FALSE;
	request.parameter[1].data.value.string = data;
	request.parameter[1].data.has_value = IOT_TRUE;
	request.parameter[1].data.type = IOT_TYPE_STRING;
	size = iot_action_request_copy_size( &request );
	assert_int_equal( size, ( sizeof( struct iot_action_parameter ) *
		request.parameter_count ) + 10u );

	/* clean up */
#ifndef IOT_STACK_ONLY
	test_free( request.parameter );
#endif
}

static void test_iot_action_request_execute_invalid_request( void **state )
{
	struct iot_action_request req;
	iot_status_t result;
	bzero( &req, sizeof( struct iot_action_request ) );
	result = iot_action_request_execute( &req, 0u );
	assert_int_equal( result, IOT_STATUS_NOT_INITIALIZED );
}

static void test_iot_action_request_execute_full_queue( void **state )
{
	struct iot lib;
	struct iot_action_request req;
	iot_status_t result;
	bzero( &lib, sizeof( struct iot ) );
	bzero( &req, sizeof( struct iot_action_request ) );

	/* sets the queue to full */
	lib.request_queue_wait_count = IOT_ACTION_QUEUE_MAX;

	req.lib = &lib;
	will_return( __wrap_iot_error, "request queue is full" );
	will_return( __wrap_iot_plugin_perform, IOT_STATUS_SUCCESS );
	result = iot_action_request_execute( &req, 0u );
	assert_int_equal( result, IOT_STATUS_FULL );
}

static void test_iot_action_request_execute_null_request( void **state )
{
	iot_status_t result;
	result = iot_action_request_execute( NULL, 0u );
	assert_int_equal( result, IOT_STATUS_BAD_PARAMETER );
}

static void test_iot_action_request_execute_success( void **state )
{
	struct iot lib;
	struct iot_action_request req;
	iot_status_t result;
	bzero( &lib, sizeof( struct iot ) );
	bzero( &req, sizeof( struct iot_action_request ) );
	req.lib = &lib;
	result = iot_action_request_execute( &req, 0u );
	assert_int_equal( result, IOT_STATUS_SUCCESS );
}

int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_action_allocate_existing ),
		cmocka_unit_test( test_iot_action_allocate_first ),
		cmocka_unit_test( test_iot_action_allocate_full ),
		cmocka_unit_test( test_iot_action_allocate_stack_full ),
		cmocka_unit_test( test_iot_action_allocate_null_lib ),
		cmocka_unit_test( test_iot_action_deregister_deregistered ),
		cmocka_unit_test( test_iot_action_deregister_null_action ),
		cmocka_unit_test( test_iot_action_deregister_null_lib ),
		cmocka_unit_test( test_iot_action_deregister_transmit_fail ),
		cmocka_unit_test( test_iot_action_deregister_valid ),
		cmocka_unit_test( test_iot_action_flags_set_null_action ),
		cmocka_unit_test( test_iot_action_flags_set_valid ),
		cmocka_unit_test( test_iot_action_free_options ),
		cmocka_unit_test( test_iot_action_free_not_found ),
		cmocka_unit_test( test_iot_action_free_null_action ),
		cmocka_unit_test( test_iot_action_free_null_handle ),
		cmocka_unit_test( test_iot_action_free_parameters ),
		cmocka_unit_test( test_iot_action_free_transmit_fail ),
		cmocka_unit_test( test_iot_action_option_get_not_there ),
		cmocka_unit_test( test_iot_action_option_get_null_action ),
		cmocka_unit_test( test_iot_action_option_get_null_name ),
		cmocka_unit_test( test_iot_action_option_get_valid ),
		cmocka_unit_test( test_iot_action_option_set_add ),
		cmocka_unit_test( test_iot_action_option_set_full ),
		cmocka_unit_test( test_iot_action_option_set_null_action ),
		cmocka_unit_test( test_iot_action_option_set_null_data ),
		cmocka_unit_test( test_iot_action_option_set_update ),
		cmocka_unit_test( test_iot_action_option_set_raw_add ),
		cmocka_unit_test( test_iot_action_parameter_add_bad_name ),
		cmocka_unit_test( test_iot_action_parameter_add_exists ),
		cmocka_unit_test( test_iot_action_parameter_add_null_action ),
		cmocka_unit_test( test_iot_action_parameter_add_null_name ),
		cmocka_unit_test( test_iot_action_parameter_add_parameters_empty ),
		cmocka_unit_test( test_iot_action_parameter_add_parameters_full ),
		cmocka_unit_test( test_iot_action_parameter_add_parameters_half_full ),
		cmocka_unit_test( test_iot_action_parameter_get_not_found ),
		cmocka_unit_test( test_iot_action_parameter_get_no_parameters ),
		cmocka_unit_test( test_iot_action_parameter_get_null_name ),
		cmocka_unit_test( test_iot_action_parameter_get_null_request ),
		cmocka_unit_test( test_iot_action_parameter_get_valid ),
		cmocka_unit_test( test_iot_action_parameter_get_raw_null_data ),
		cmocka_unit_test( test_iot_action_parameter_get_raw_null_length ),
		cmocka_unit_test( test_iot_action_parameter_get_raw_valid ),
		cmocka_unit_test( test_iot_action_parameter_set_bad_name ),
		cmocka_unit_test( test_iot_action_parameter_set_max_parameters ),
		cmocka_unit_test( test_iot_action_parameter_set_new_parameter ),
		cmocka_unit_test( test_iot_action_parameter_set_null_name ),
		cmocka_unit_test( test_iot_action_parameter_set_null_request ),
		cmocka_unit_test( test_iot_action_parameter_set_type_null ),
		cmocka_unit_test( test_iot_action_parameter_set_type_wrong ),
		cmocka_unit_test( test_iot_action_parameter_set_valid ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_max_parameters ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_new_parameter ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_null_data ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_null_name ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_null_request ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_type_null ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_type_wrong ),
		cmocka_unit_test( test_iot_action_parameter_set_raw_valid ),
		cmocka_unit_test( test_iot_action_process_actions_empty ),
		cmocka_unit_test( test_iot_action_process_actions_full ),
		cmocka_unit_test( test_iot_action_process_actions_not_found ),
		cmocka_unit_test( test_iot_action_process_command_no_return ),
		cmocka_unit_test( test_iot_action_process_command_parameter_bool ),
		cmocka_unit_test( test_iot_action_process_command_parameter_float ),
		cmocka_unit_test( test_iot_action_process_command_parameter_int ),
		cmocka_unit_test( test_iot_action_process_command_parameter_location ),
		cmocka_unit_test( test_iot_action_process_command_parameter_null ),
		cmocka_unit_test( test_iot_action_process_command_parameter_raw ),
		cmocka_unit_test( test_iot_action_process_command_parameter_string ),
		cmocka_unit_test( test_iot_action_process_command_parameter_uint ),
		cmocka_unit_test( test_iot_action_process_command_script_return_fail ),
		cmocka_unit_test( test_iot_action_process_command_system_run_fail ),
		cmocka_unit_test( test_iot_action_process_command_valid ),
		cmocka_unit_test( test_iot_action_process_exclusive ),
		cmocka_unit_test( test_iot_action_process_lib_to_quit ),
		cmocka_unit_test( test_iot_action_process_no_handler ),
		cmocka_unit_test( test_iot_action_process_null_lib ),
		cmocka_unit_test( test_iot_action_process_options ),
		cmocka_unit_test( test_iot_action_process_parameters_bad_type ),
		cmocka_unit_test( test_iot_action_process_parameters_missing_required ),
		cmocka_unit_test( test_iot_action_process_parameters_undeclared ),
		cmocka_unit_test( test_iot_action_process_parameters_valid ),
		cmocka_unit_test( test_iot_action_process_valid ),
		cmocka_unit_test( test_iot_action_process_wait_queue_empty ),
		cmocka_unit_test( test_iot_action_process_wait_queue_full ),
		cmocka_unit_test( test_iot_action_register_callback_null_action ),
		cmocka_unit_test( test_iot_action_register_callback_null_lib ),
		cmocka_unit_test( test_iot_action_register_callback_transmit_fail ),
		cmocka_unit_test( test_iot_action_register_callback_valid ),
		cmocka_unit_test( test_iot_action_register_command_null_action ),
		cmocka_unit_test( test_iot_action_register_command_null_lib ),
		cmocka_unit_test( test_iot_action_register_command_transmit_fail ),
		cmocka_unit_test( test_iot_action_register_command_valid ),
		cmocka_unit_test( test_iot_action_request_copy_raw ),
		cmocka_unit_test( test_iot_action_request_copy_raw_too_big ),
		cmocka_unit_test( test_iot_action_request_copy_string ),
		cmocka_unit_test( test_iot_action_request_copy_string_too_big ),
		cmocka_unit_test( test_iot_action_request_copy_size_no_parameters ),
		cmocka_unit_test( test_iot_action_request_copy_size_no_pointers ),
		cmocka_unit_test( test_iot_action_request_copy_size_no_values ),
		cmocka_unit_test( test_iot_action_request_copy_size_null_request ),
		cmocka_unit_test( test_iot_action_request_copy_size_raw ),
		cmocka_unit_test( test_iot_action_request_copy_size_string ),
		cmocka_unit_test( test_iot_action_request_execute_invalid_request ),
		cmocka_unit_test( test_iot_action_request_execute_full_queue ),
		cmocka_unit_test( test_iot_action_request_execute_null_request ),
		cmocka_unit_test( test_iot_action_request_execute_success )
	};
	MOCK_SYSTEM_ENABLED = 1;
	result = cmocka_run_group_tests( tests, NULL, NULL );
	MOCK_SYSTEM_ENABLED = 0;
	return result;
}
