/**
 * @file
 * @brief source file for IoT library json decoding functionality
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

#include "api/public/iot_json.h"
#include "utilities/app_json.h"

iot_status_t iot_json_decode_array_at(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	size_t index,
	const iot_json_item_t **out )
{
	return app_json_decode_array_at(
		(const app_json_decoder_t *) decoder,
		(const app_json_item_t *) item, index,
		(const app_json_item_t **)out );
}

const iot_json_array_iterator_t *iot_json_decode_array_iterator(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	return app_json_decode_array_iterator(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item );
}

iot_status_t iot_json_decode_array_iterator_value(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	const iot_json_array_iterator_t *iter,
	const iot_json_item_t **out )
{
	return (iot_status_t)app_json_decode_array_iterator_value(
		(const app_json_decoder_t *)json,
		(const app_json_item_t *)item,
		(const app_json_array_iterator_t *)iter,
		(const app_json_item_t **)out );
}

const iot_json_array_iterator_t *iot_json_decode_array_iterator_next(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_array_iterator_t *iter )
{
	return app_json_decode_array_iterator_next(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item,
		(const app_json_array_iterator_t *)iter );
}

size_t iot_json_decode_array_size(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	return app_json_decode_array_size(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item );
}

iot_status_t iot_json_decode_bool(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_bool_t *value )
{
	return app_json_decode_bool(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item,
		value );
}

iot_json_decoder_t *iot_json_decode_initialize(
	void *buf,
	size_t len,
	unsigned int flags )
{
	return (iot_json_decoder_t *)
		app_json_decode_initialize( buf, len, flags );
}

iot_status_t iot_json_decode_integer(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_int64_t *value )
{
	return app_json_decode_integer(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item, value );
}

iot_status_t iot_json_decode_number(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_float64_t *value )
{
	return app_json_decode_number(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item, value );
}

const iot_json_item_t *iot_json_decode_object_find(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *object,
	const char *key )
{
	return app_json_decode_object_find(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)object, key );
}

const iot_json_item_t *iot_json_decode_object_find_len(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *object,
	const char *key,
	size_t key_len )
{
	return app_json_decode_object_find_len(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)object,
		key, key_len );
}

const iot_json_object_iterator_t *iot_json_decode_object_iterator(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	return app_json_decode_object_iterator(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item );
}

iot_status_t iot_json_decode_object_iterator_key(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_object_iterator_t *iter,
	const char **key,
	size_t *key_len )
{
	return app_json_decode_object_iterator_key(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item,
		(const app_json_array_iterator_t *)iter, key, key_len );
}

const iot_json_object_iterator_t *iot_json_decode_object_iterator_next(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_object_iterator_t *iter )
{
	return app_json_decode_object_iterator_next(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item,
		(const app_json_array_iterator_t *)iter );
}

iot_status_t iot_json_decode_object_iterator_value(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_object_iterator_t *iter,
	const iot_json_item_t **out )
{
	return app_json_decode_object_iterator_value(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item,
		(const app_json_array_iterator_t *)iter,
		(const app_json_item_t **)out );
}

size_t iot_json_decode_object_size(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	return app_json_decode_object_size(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item );
}

iot_status_t iot_json_decode_parse(
	iot_json_decoder_t *decoder,
	const char *js,
	size_t len,
	const iot_json_item_t **root,
	char *error,
	size_t error_len )
{
	return app_json_decode_parse(
		(app_json_decoder_t *)decoder, js, len,
		(const app_json_item_t **)root, error, error_len );
}

iot_status_t iot_json_decode_real(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_float64_t *value )
{
	return app_json_decode_real(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item, value );
}

iot_status_t iot_json_decode_string(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const char **value,
	size_t *value_len )
{
	return app_json_decode_string(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item, value, value_len );
}

void iot_json_decode_terminate(
	iot_json_decoder_t *decoder )
{
	app_json_decode_terminate( (app_json_decoder_t *)decoder );
}

iot_json_type_t iot_json_decode_type(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	return (iot_json_type_t)app_json_decode_type(
		(const app_json_decoder_t *)decoder,
		(const app_json_item_t *)item );
}

