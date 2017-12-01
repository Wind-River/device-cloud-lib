/**
 * @file
 * @brief Definitions for common log functionality for applications
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
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
#ifndef APP_LOG_H
#define APP_LOG_H

#include "api/public/iot.h"            /* for iot_log_* definitions */

/**
 * @brief Callback to print log messages
 *
 * @param[in]      log_level                     log level of the message
 * @param[in]      source                        log message source information
 * @param[in]      message                       message string to print out
 * @param[in]      user_data                     user specific data (unused)
 */
IOT_SECTION void app_log( iot_log_level_t log_level,
	const iot_log_source_t *source, const char *message, void *user_data );

#endif /* ifndef APP_LOG_H */

