/**
 * @file
 * @brief Main source file for the Wind River IoT control application
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

#ifndef DEVICE_MANAGER_OTA_H
#define DEVICE_MANAGER_OTA_H
#pragma warning( push, 1 )
#include <curl/curl.h>             /* for performing REST requests */
#pragma warning( pop )

#include "os.h"
#include "iot.h"
#include "device_manager_md5.h"
#include "device_manager_sha256.h"
/** @brief Maximum length of field in manifest */
#define DEVICE_MANAGER_OTA_PKG_STRING_MAX_LENGTH 255
struct device_manager_info;

/** @brief Contains information about ota manifest */
struct device_manager_ota_manifest
{
	/** @brief Library handle */
	iot_t *iot_lib;
	/** @brief Manifest identifier */
	char identifier[ DEVICE_MANAGER_OTA_PKG_STRING_MAX_LENGTH + 1u ];
	/** @brief Manifest opertion type */
	char operation[ DEVICE_MANAGER_OTA_PKG_STRING_MAX_LENGTH + 1u ];
	/** @brief Manifest version */
	char version [ DEVICE_MANAGER_OTA_PKG_STRING_MAX_LENGTH + 1u ];
	/** @brief Expected sh256 checksum for a downloaded file */
	char checksum_sh256[ DEVICE_MANAGER_SHA256_DIGEST_HEX_LENGTH + 1u ];
	/** @brief Expected md5 checksum for a downloaded file */
	char checksum_md5[ DEVICE_MANAGER_MD5_DIGEST_HEX_LENGTH + 1u ];
	/** @brief Token for response URL */
	char jwt[ DEVICE_MANAGER_OTA_PKG_STRING_MAX_LENGTH + 1u ];
	/** @brief Path to excute the script */
	char command_path[ PATH_MAX + 1u ];
	/** @brief Path to excute the script */
	char pkg_name[ PATH_MAX + 1u ];
	/** @brief Libcurl instance handling the status response */
	CURL *response_lib_curl;
	/** @brief Path to the file to be uploaded or downloaded */
	char download_url[ PATH_MAX + 1u  ];
	/** @brief Path to the file to be uploaded or downloaded */
	char response_url[ PATH_MAX + 1u ];
};
/**
 * @brief Deregisters the functions for ota
 *
 * @param[in,out]  device_manager     pointer to app data
 *
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_BAD_PARAMETER    on failure
 */
iot_status_t device_manager_ota_deregister(
	struct device_manager_info  *device_manager );
/**
 * @brief Registers the functions for ota
 *
 * @param[in,out]  device_manager      pointer to app data
 *
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_*                on failure
 */
iot_status_t device_manager_ota_register(
	struct device_manager_info  *device_manager );

#endif /* ifndef DEVICE_MANAGER_OTA_H */
