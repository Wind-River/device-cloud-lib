/**
 * @file
 * @brief Source code for mocking the IoT client library
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

#include "api/shared/iot_types.h"
#include "api/public/iot_json.h"

/* clang-format off */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <string.h> /* for strlen */
/* clang-format on */

/* mock definitions */
iot_status_t __wrap_iot_action_process( iot_t *lib_handle, iot_millisecond_t max_time_out );
iot_status_t __wrap_iot_action_check( iot_t *lib_handle, iot_millisecond_t max_time_out );
iot_status_t __wrap_iot_action_free( iot_action_t *action, iot_millisecond_t max_time_out );
iot_status_t __wrap_iot_alarm_deregister( iot_telemetry_t *alarm );
size_t __wrap_iot_base64_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );
size_t __wrap_iot_base64_encode_size( size_t in_bytes );
const char *__wrap_iot_error( iot_status_t code );
iot_status_t __wrap_iot_log( iot_t *handle,
                             iot_log_level_t log_level,
                             const char *function_name,
                             const char *file_name,
                             unsigned int line_number,
                             const char *log_msg_fmt,
                             ... );

/* plug-in support */
iot_status_t __wrap_iot_plugin_perform( iot_t *lib,
                                        iot_transaction_t *txn,
                                        iot_operation_t op,
                                        iot_millisecond_t max_time_out,
                                        const void *item,
                                        const void *new_value );
unsigned int __wrap_iot_plugin_builtin_load( iot_t *lib, unsigned int max );
iot_bool_t __wrap_iot_plugin_builtin_enable( iot_t *lib );
iot_status_t __wrap_iot_plugin_disable_all( iot_t *lib );
iot_status_t __wrap_iot_plugin_enable( iot_t *lib, const char *name );
void __wrap_iot_plugin_initialize( iot_plugin_t *p );
void __wrap_iot_plugin_terminate( iot_plugin_t *p );
iot_status_t __wrap_iot_telemetry_free( iot_telemetry_t *telemetry,
	iot_millisecond_t max_time_out );

/* mock iot_json functions */
iot_status_t __wrap_iot_json_decode_bool(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_bool_t *value );
iot_json_decoder_t *__wrap_iot_json_decode_initialize(
	char *buf,
	size_t len,
	unsigned int flags );
iot_status_t __wrap_iot_json_decode_integer(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_int64_t *value );
iot_json_object_iterator_t *__wrap_iot_json_decode_object_iterator(
	const iot_json_decoder_t *json,
	iot_json_item_t *item );
iot_status_t __wrap_iot_json_decode_object_iterator_key(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_json_object_iterator_t *iter,
	const char **key,
	size_t *key_len );
iot_json_object_iterator_t *__wrap_iot_json_decode_object_iterator_next(
	const iot_json_decoder_t *json,
	iot_json_item_t *item,
	iot_json_object_iterator_t *iter );
iot_status_t __wrap_iot_json_decode_object_iterator_value(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_json_object_iterator_t *iter,
	iot_json_item_t **out );
iot_status_t __wrap_iot_json_decode_parse(
	iot_json_decoder_t *json,
	const char* js,
	size_t len,
	iot_json_item_t **root,
	char *error,
	size_t error_len );
iot_status_t __wrap_iot_json_decode_real(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_float64_t *value );
iot_status_t __wrap_iot_json_decode_number(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_float64_t *value );
iot_status_t __wrap_iot_json_decode_string(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	const char **value,
	size_t *value_len );
void __wrap_iot_json_decode_terminate(
	iot_json_decoder_t *json );
iot_json_type_t __wrap_iot_json_decode_type(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item );

/* mock functions */
iot_status_t __wrap_iot_action_process( iot_t *lib_handle, iot_millisecond_t max_time_out )
{
	return mock_type( iot_status_t );
}

iot_status_t __wrap_iot_action_check( iot_t *lib_handle, iot_millisecond_t max_time_out )
{
	return mock_type( iot_status_t );
}

iot_status_t __wrap_iot_action_free( iot_action_t *action, iot_millisecond_t max_time_out )
{
	return mock_type( iot_status_t );
}

iot_status_t __wrap_iot_alarm_deregister( iot_telemetry_t *alarm )
{
	return mock_type( iot_status_t );
}

size_t __wrap_iot_base64_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
	size_t place;
	size_t max_len = 0u;
	assert_non_null( out );
	assert_non_null( out_len );
	if ( in && in_len > 0u )
		max_len = mock_type( size_t );
	for ( place = 0u; place < max_len; place++ )
		out[place] = (uint8_t)'b';
	/*out[place] = 1u;*/
	return place;
}

size_t __wrap_iot_base64_encode_size( size_t in_bytes )
{
	size_t result = 0u;
	if ( in_bytes > 0u )
		result = 4u * ( 1u + ( ( in_bytes - 1u ) / 3u ) );
	return result;
}

const char *__wrap_iot_error( iot_status_t code )
{
	return (char *)mock();
}

iot_status_t __wrap_iot_log( iot_t *handle,
                             iot_log_level_t log_level,
                             const char *function_name,
                             const char *file_name,
                             unsigned int line_number,
                             const char *log_msg_fmt,
                             ... )
{
	return IOT_STATUS_FAILURE;
}

iot_status_t __wrap_iot_plugin_perform( iot_t *lib,
                                        iot_transaction_t *txn,
                                        iot_operation_t op,
                                        iot_millisecond_t max_time_out,
                                        const void *item,
                                        const void *new_value )
{
	return (iot_status_t)mock();
}

unsigned int __wrap_iot_plugin_builtin_load( iot_t *lib, unsigned int max )
{
	lib->plugin_count = mock_type( unsigned int );
	return 0u;
}

iot_bool_t __wrap_iot_plugin_builtin_enable( iot_t *lib )
{
	return IOT_TRUE;
}

iot_status_t __wrap_iot_plugin_disable_all( iot_t *lib )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_iot_plugin_enable( iot_t *lib, const char *name )
{
	return IOT_STATUS_SUCCESS;
}

void __wrap_iot_plugin_initialize( iot_plugin_t *p )
{
}

void __wrap_iot_plugin_terminate( iot_plugin_t *p )
{
}

iot_status_t __wrap_iot_telemetry_free( iot_telemetry_t *telemetry,
	iot_millisecond_t max_time_out )
{
	return mock_type( iot_status_t );
}

iot_status_t __wrap_iot_json_decode_bool(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_bool_t *value )
{
	return IOT_STATUS_SUCCESS;
}

iot_json_decoder_t *__wrap_iot_json_decode_initialize(
	char *buf,
	size_t len,
	unsigned int flags )
{
	return mock_type( iot_json_decoder_t *);
}

iot_status_t __wrap_iot_json_decode_integer(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_int64_t *value )
{
	if ( value ) *value = 1;
	return IOT_STATUS_SUCCESS;
}

iot_json_object_iterator_t *__wrap_iot_json_decode_object_iterator(
	const iot_json_decoder_t *json,
	iot_json_item_t *item )
{
	return (iot_json_object_iterator_t*)0x2;
}

iot_status_t __wrap_iot_json_decode_object_iterator_key(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_json_object_iterator_t *iter,
	const char **key,
	size_t *key_len )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	const char *str = mock_type( const char * );
	size_t str_len = 0u;
	if ( str )
	{
		result = IOT_STATUS_SUCCESS;
		str_len = strlen( str );
	}
	if ( key ) *key = str;
	if ( key_len ) *key_len = str_len;
	return result;
}

iot_json_object_iterator_t *__wrap_iot_json_decode_object_iterator_next(
	const iot_json_decoder_t *json,
	iot_json_item_t *item,
	iot_json_object_iterator_t *iter )
{
	return mock_type( iot_json_object_iterator_t * );
}

iot_status_t __wrap_iot_json_decode_object_iterator_value(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_json_object_iterator_t *iter,
	iot_json_item_t **out )
{
	if ( out ) *out = (iot_json_item_t*)0x3;
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_iot_json_decode_parse(
	iot_json_decoder_t *json,
	const char* js,
	size_t len,
	iot_json_item_t **root,
	char *error,
	size_t error_len )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_iot_json_decode_real(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_float64_t *value )
{
	if ( value ) *value = 1.2345;
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_iot_json_decode_number(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	iot_float64_t *value )
{
	if ( value ) *value = 1.2345;
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_iot_json_decode_string(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	const char **value,
	size_t *value_len )
{
	const char *str = mock_type( const char *);
	size_t str_len = 0u;
	if ( str )
		str_len = strlen( str );
	if ( value ) *value = str;
	if ( value_len ) *value_len = str_len;
	return IOT_STATUS_SUCCESS;
}

void __wrap_iot_json_decode_terminate(
	iot_json_decoder_t *json )
{
}

iot_json_type_t __wrap_iot_json_decode_type(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item )
{
	return mock_type( iot_json_type_t );
}

