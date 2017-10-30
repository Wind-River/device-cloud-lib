/**
 * @file
 * @brief Definitions for common log functionality for applications
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
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

