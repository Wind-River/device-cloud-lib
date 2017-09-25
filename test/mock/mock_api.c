/**
 * @file
 * @brief Source code for mocking the IoT client library
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "api/shared/iot_types.h"
#include "api/public/iot_json.h"

/* clang-format off */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
/* clang-format on */

/* mock definitions */
iot_status_t __wrap_iot_action_process( iot_t *lib_handle, iot_millisecond_t max_time_out );
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
iot_status_t __wrap_iot_telemetry_free( iot_telemetry_t *telemetry, iot_millisecond_t max_time_out );

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

iot_status_t __wrap_iot_action_free( iot_action_t *action, iot_millisecond_t max_time_out )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_iot_alarm_deregister( iot_telemetry_t *alarm )
{
	return IOT_STATUS_SUCCESS;
}

size_t __wrap_iot_base64_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
	size_t place;
	size_t max_len;
	assert_non_null( out );
	assert_non_null( out_len );
	assert_non_null( in );
	assert_non_null( in_len );
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
	return IOT_STATUS_SUCCESS;
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
	return (iot_json_decoder_t*)0x1;
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
	if ( key ) *key = NULL;
	if ( key_len ) *key_len = 0u;
	return IOT_STATUS_SUCCESS;
}

iot_json_object_iterator_t *__wrap_iot_json_decode_object_iterator_next(
	const iot_json_decoder_t *json,
	iot_json_item_t *item,
	iot_json_object_iterator_t *iter )
{
	return NULL;
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

iot_status_t __wrap_iot_json_decode_string(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	const char **value,
	size_t *value_len )
{
	if ( value ) *value = NULL;
	if ( value_len ) *value_len = 0u;
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
	return IOT_JSON_TYPE_INTEGER;
}

