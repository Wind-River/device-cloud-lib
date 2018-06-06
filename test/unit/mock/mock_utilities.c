/**
 * @file
 * @brief Source code for mocking the IoT client library
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

#include "api/shared/iot_types.h"
#include "api/public/iot_json.h"
#include "utilities/app_json.h"

/* clang-format off */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
#include <string.h> /* for strlen */
/* clang-format on */

/* mock definitions */
/* mock app_json functions */
iot_status_t __wrap_app_json_decode_bool(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_bool_t *value );
app_json_decoder_t *__wrap_app_json_decode_initialize(
	char *buf,
	size_t len,
	unsigned int flags );
iot_status_t __wrap_app_json_decode_integer(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_int64_t *value );
app_json_object_iterator_t *__wrap_app_json_decode_object_iterator(
	const app_json_decoder_t *json,
	app_json_item_t *item );
iot_status_t __wrap_app_json_decode_object_iterator_key(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	app_json_object_iterator_t *iter,
	const char **key,
	size_t *key_len );
app_json_object_iterator_t *__wrap_app_json_decode_object_iterator_next(
	const app_json_decoder_t *json,
	app_json_item_t *item,
	app_json_object_iterator_t *iter );
iot_status_t __wrap_app_json_decode_object_iterator_value(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	app_json_object_iterator_t *iter,
	app_json_item_t **out );
iot_status_t __wrap_app_json_decode_parse(
	app_json_decoder_t *json,
	const char* js,
	size_t len,
	app_json_item_t **root,
	char *error,
	size_t error_len );
iot_status_t __wrap_app_json_decode_real(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_float64_t *value );
iot_status_t __wrap_app_json_decode_number(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_float64_t *value );
iot_status_t __wrap_app_json_decode_string(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	const char **value,
	size_t *value_len );
void __wrap_app_json_decode_terminate(
	app_json_decoder_t *json );
app_json_type_t __wrap_app_json_decode_type(
	const app_json_decoder_t *json,
	const app_json_item_t *item );
app_json_type_t __wrap_app_json_decode_array_at(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	size_t index,
	const app_json_item_t **out );
const app_json_array_iterator_t *__wrap_app_json_decode_array_iterator(
	const app_json_decoder_t *json,
	const app_json_item_t *item );
iot_status_t __wrap_app_json_decode_array_iterator_value(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	const app_json_array_iterator_t *iter,
	const app_json_item_t *out );
const app_json_array_iterator_t *__wrap_app_json_decode_array_iterator_next(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	const app_json_array_iterator_t *iter );
size_t __wrap_app_json_decode_array_size(
	const app_json_decoder_t *json,
	const app_json_item_t *item );
const app_json_item_t *__wrap_app_json_decode_object_find(
	const app_json_decoder_t *json,
	const app_json_item_t *item );
const app_json_item_t *__wrap_app_json_decode_object_find_len(
	const app_json_decoder_t *json,
	const app_json_item_t *object,
	const char *key,
	size_t key_len );
size_t __wrap_app_json_decode_object_size(
	const app_json_decoder_t *json,
	const app_json_item_t *object );

iot_status_t __wrap_app_json_encode_object_start(
	const app_json_encoder_t *json,
	const char* key );
iot_status_t __wrap_app_json_encode_object_cancel(
	const app_json_encoder_t *json,
	const char* key );
iot_status_t __wrap_app_json_encode_object_clear(
	const app_json_encoder_t *json,
	const char* key );
iot_status_t __wrap_app_json_encode_object_end(
	const app_json_encoder_t *json,
	const char* key );
iot_status_t __wrap_app_json_encode_integer(
	const app_json_encoder_t *json,
	const char* key,
	iot_int64_t value);
iot_status_t __wrap_app_json_encode_bool(
	const app_json_encoder_t *json,
	const char* key,
	iot_bool_t value);
iot_status_t __wrap_app_json_encode_real(
	const app_json_encoder_t *json,
	const char* key,
	iot_float64_t value);
iot_status_t __wrap_app_json_encode_string(
	const app_json_encoder_t *json,
	const char* key,
	const char* value);
iot_status_t __wrap_app_json_encode_array_start(
	const app_json_encoder_t *json,
	const char* key );
iot_status_t __wrap_app_json_encode_array_end(
	const app_json_encoder_t *json );
const char *__wrap_app_json_encode_dump(
	const app_json_encoder_t *json );
app_json_encoder_t *__wrap_app_json_encode_initialize(
	void *buf,
	size_t len,
	unsigned int flags);
void __wrap_app_json_encode_terminate(
	const app_json_encoder_t *json );

/* mock functions */
/* mock app_json_decode functions */
iot_status_t __wrap_app_json_decode_bool(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_bool_t *value )
{
	if ( value ) *value = IOT_TRUE;
	return IOT_STATUS_SUCCESS;
}

app_json_decoder_t *__wrap_app_json_decode_initialize(
	char *buf,
	size_t len,
	unsigned int flags )
{
	return mock_type( app_json_decoder_t *);
}

iot_status_t __wrap_app_json_decode_integer(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_int64_t *value )
{
	if ( value ) *value = 1;
	return IOT_STATUS_SUCCESS;
}

app_json_object_iterator_t *__wrap_app_json_decode_object_iterator(
	const app_json_decoder_t *json,
	app_json_item_t *item )
{
	return (app_json_object_iterator_t*)0x2;
}

iot_status_t __wrap_app_json_decode_object_iterator_key(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	app_json_object_iterator_t *iter,
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

app_json_object_iterator_t *__wrap_app_json_decode_object_iterator_next(
	const app_json_decoder_t *json,
	app_json_item_t *item,
	app_json_object_iterator_t *iter )
{
	return mock_type( app_json_object_iterator_t * );
}

iot_status_t __wrap_app_json_decode_object_iterator_value(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	app_json_object_iterator_t *iter,
	app_json_item_t **out )
{
	if ( out ) *out = (app_json_item_t*)0x3;
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_decode_parse(
	app_json_decoder_t *json,
	const char* js,
	size_t len,
	app_json_item_t **root,
	char *error,
	size_t error_len )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_decode_real(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_float64_t *value )
{
	if ( value ) *value = 1.2345;
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_decode_number(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	iot_float64_t *value )
{
	if ( value ) *value = 1.2345;
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_decode_string(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
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

void __wrap_app_json_decode_terminate(
	app_json_decoder_t *json )
{
}

app_json_type_t __wrap_app_json_decode_type(
	const app_json_decoder_t *json,
	const app_json_item_t *item )
{
	return mock_type( app_json_type_t );
}

app_json_type_t __wrap_app_json_decode_array_at(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	size_t index,
	const app_json_item_t **out )
{
	return IOT_STATUS_SUCCESS;
}

const app_json_array_iterator_t *__wrap_app_json_decode_array_iterator(
	const app_json_decoder_t *json,
	const app_json_item_t *item )
{
	return mock_type( const app_json_array_iterator_t* );
}

iot_status_t __wrap_app_json_decode_array_iterator_value(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	const app_json_array_iterator_t *iter,
	const app_json_item_t *out )
{
	return IOT_STATUS_SUCCESS;
}

const app_json_array_iterator_t *__wrap_app_json_decode_array_iterator_next(
	const app_json_decoder_t *json,
	const app_json_item_t *item,
	const app_json_array_iterator_t *iter )
{
	return mock_type( const app_json_array_iterator_t* );
}

size_t __wrap_app_json_decode_array_size(
	const app_json_decoder_t *json,
	const app_json_item_t *item )
{
	return 1;
}

const app_json_item_t *__wrap_app_json_decode_object_find(
	const app_json_decoder_t *json,
	const app_json_item_t *item )
{
	return mock_type( const app_json_item_t* );
}

const app_json_item_t *__wrap_app_json_decode_object_find_len(
	const app_json_decoder_t *json,
	const app_json_item_t *object,
	const char *key,
	size_t key_len )
{
	return mock_type( const app_json_item_t* );
}

size_t __wrap_app_json_decode_object_size(
	const app_json_decoder_t *json,
	const app_json_item_t *object )
{
	return 1u;
}

/* mock app_json_encode functions */
iot_status_t __wrap_app_json_encode_object_start(
	const app_json_encoder_t *json,
	const char* key )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_object_cancel(
	const app_json_encoder_t *json,
	const char* key )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_object_clear(
	const app_json_encoder_t *json,
	const char* key )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_object_end(
	const app_json_encoder_t *json,
	const char* key )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_integer(
	const app_json_encoder_t *json,
	const char* key,
	iot_int64_t value)
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_bool(
	const app_json_encoder_t *json,
	const char* key,
	iot_bool_t value)
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_real(
	const app_json_encoder_t *json,
	const char* key,
	iot_float64_t value)
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_string(
	const app_json_encoder_t *json,
	const char* key,
	const char* value)
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_array_start(
	const app_json_encoder_t *json,
	const char* key )
{
	return IOT_STATUS_SUCCESS;
}

iot_status_t __wrap_app_json_encode_array_end(
	const app_json_encoder_t *json )
{
	return IOT_STATUS_SUCCESS;
}

const char *__wrap_app_json_encode_dump(
	const app_json_encoder_t *json )

{
	return "dump";
}

app_json_encoder_t *__wrap_app_json_encode_initialize(
	void *buf,
	size_t len,
	unsigned int flags)
{
	return (app_json_encoder_t *)0x1;
}

void __wrap_app_json_encode_terminate(
	const app_json_encoder_t *json )
{
}

