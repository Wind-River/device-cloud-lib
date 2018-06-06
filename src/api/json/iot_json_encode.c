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

iot_status_t iot_json_encode_array_end(
	iot_json_encoder_t *encoder )
{
	return app_json_encode_array_end(
		(app_json_encoder_t *)encoder );
}

iot_status_t iot_json_encode_array_start(
	iot_json_encoder_t *encoder,
	const char *key )
{
	return app_json_encode_array_start(
		(app_json_encoder_t*)encoder, key );
}

iot_status_t iot_json_encode_bool(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_bool_t value )
{
	return app_json_encode_bool(
		(app_json_encoder_t *)encoder, key, value );
}

const char *iot_json_encode_dump(
	iot_json_encoder_t *encoder )
{
	return app_json_encode_dump( (app_json_encoder_t *)encoder );
}

iot_json_encoder_t *iot_json_encode_initialize(
	void *buf,
	size_t len,
	unsigned int flags )
{
	return (iot_json_encoder_t *)app_json_encode_initialize(
		buf, len, flags );
}

iot_status_t iot_json_encode_integer(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_int64_t value )
{
	return app_json_encode_integer(
		(app_json_encoder_t *)encoder, key, value );
}

iot_status_t iot_json_encode_object_cancel(
	iot_json_encoder_t *encoder )
{
	return app_json_encode_object_cancel(
		(app_json_encoder_t *)encoder );
}

iot_status_t iot_json_encode_object_clear(
	iot_json_encoder_t *encoder )
{
	return app_json_encode_object_clear(
		(app_json_encoder_t *)encoder );
}

iot_status_t iot_json_encode_object_end(
	iot_json_encoder_t *encoder )
{
	return app_json_encode_object_end(
		(app_json_encoder_t *)encoder );
}

iot_status_t iot_json_encode_object_start(
	iot_json_encoder_t *encoder,
	const char *key )
{
	return app_json_encode_object_start(
		(app_json_encoder_t *)encoder, key );
}

iot_status_t iot_json_encode_real(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_float64_t value )
{
	return app_json_encode_real(
		(app_json_encoder_t *)encoder, key, value );
}

iot_status_t iot_json_encode_string(
	iot_json_encoder_t *encoder,
	const char *key,
	const char *value )
{
	return app_json_encode_string(
		(app_json_encoder_t *)encoder, key, value );
}

void iot_json_encode_terminate(
	iot_json_encoder_t *encoder )
{
	app_json_encode_terminate( (app_json_encoder_t *)encoder );
}

