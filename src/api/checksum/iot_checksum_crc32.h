/**
 * @file
 * @brief source file for calculating crc32 checksum of a file
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

#ifndef IOT_CHECKSUM_CRC32_H
#define IOT_CHECKSUM_CRC32_H

#include <iot.h>
#include <os.h>

/**
 * @brief helper function to retrieve a CRC-32 checksum of a file
 *
 * @param[in]      file                open file handle
 * @param[out]     checksum            calculated checksum
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          general operating system failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t iot_checksum_crc32_file_get(
	os_file_t file,
	iot_uint64_t *checksum );

#endif /* IOT_CHECKSUM_CRC32_H */
