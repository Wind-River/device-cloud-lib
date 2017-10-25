/**
 * @file
 * @brief source file for IoT library event support
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "shared/iot_types.h"     /* for iot_plugin_perform */

iot_status_t iot_event_publish(
	iot_t *lib,
	const iot_options_t *options,
	const char *message )
{
	return iot_plugin_perform( lib, NULL, NULL,
		IOT_OPERATION_EVENT_PUBLISH, NULL, message, options );
}

