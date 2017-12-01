/**
 * @file
 * @brief Header file for calculating checksums within the IoT library
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
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

#ifndef IOT_CHECKSUM_H
#define IOT_CHECKSUM_H

#include <iot.h>
#include <os.h>

/** @brief type checksum algorithm supported */
typedef enum iot_checksum_type
{
	/** @brief use CRC32 checksum algorithm */
	IOT_CHECKSUM_TYPE_CRC32 = 0,
	/** @brief use MD5 checksum algorithm */
	IOT_CHECKSUM_TYPE_MD5,
	/** @brief use SHA256 checksum algorithm */
	IOT_CHECKSUM_TYPE_SHA256
} iot_checksum_type_t;

/**
 * @brief Calculates the checksum of a file
 *
 * @param[in]      lib                 library handle
 * @param[in]      file                handle to an open file
 * @param[in]      type                checksum algorithm to use
 * @param[out]     checksum            checksum output
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t iot_checksum_file_get(
	iot_t *lib,
	os_file_t file,
	iot_checksum_type_t type,
	iot_uint64_t *checksum );

#endif /* IOT_CHECKSUM_H */
