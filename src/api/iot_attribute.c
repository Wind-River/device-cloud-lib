/**
 * @file
 * @brief source file for IoT library attribute support
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

#include "shared/iot_types.h"     /* for iot_plugin_perform */

iot_status_t iot_attribute_publish_string(
	iot_t *lib,
	iot_transaction_t *txn,
	const iot_options_t *options,
	const char *key,
	const char *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && key && value )
	{
		result = iot_plugin_perform( lib, txn, NULL,
			IOT_OPERATION_ATTRIBUTE_PUBLISH, key, value, options );
	}
	return result;
}

