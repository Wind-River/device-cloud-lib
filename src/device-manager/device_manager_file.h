/**
 * @file
 * @brief Header file for handling file operations
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
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

#ifndef DEVICE_MANAGER_FILE_H
#define DEVICE_MANAGER_FILE_H

#include <iot.h>

#ifdef _MSC_VER
#	pragma warning( push, 1 )
#endif /* ifdef _MSC_VER */

#include <curl/curl.h>             /* for performing REST requests */

#ifdef _MSC_VER
#	pragma warning( pop )
#endif /* ifdef _MSC_VER */

#include "os.h"
#include "api/shared/iot_types.h"      /* for iot_proxy structure */

#ifndef _WRS_KERNEL
#	include "device_manager_md5.h"
#	include "device_manager_sha256.h"
#endif /* ifndef _WRS_KERNEL */
#include "device_manager_ota.h"

/** @brief Maximum length of a token from the web */
#define DEVICE_MANAGER_FILE_MAX_TOKEN_LENGTH  512u
/** @brief Maximum length of "x-access-token: " */
#define DEVICE_MANAGER_FILE_HEADER_TOKEN_KEY_LENGTH 16u
/** @brief Maximum length of checksum */
#define DEVICE_MANAGER_CHECKSUM_LENGTH \
	((DEVICE_MANAGER_SHA256_DIGEST_HEX_LENGTH) >= ( DEVICE_MANAGER_MD5_DIGEST_HEX_LENGTH ) \
	? ( DEVICE_MANAGER_SHA256_DIGEST_HEX_LENGTH ) : ( DEVICE_MANAGER_MD5_DIGEST_HEX_LENGTH ) )

struct device_manager_info;

/** @brief Types of checksum */
enum file_transfer_checksum_type
{
	CHECKSUM_MD5 = 0,
	CHECKSUM_SHA256,
};
/** @brief States of a file transfer session */
enum file_transfer_state
{
	FILE_TRANSFER_NOT_STARTED = 0,
	FILE_TRANSFER_IN_PROGRESS,
	FILE_TRANSFER_PENDING,
	FILE_TRANSFER_COMPLETED,
	FILE_TRANSFER_FAILED
};

/** @brief Types of a file transfer */
enum file_transfer_type
{
	FILE_TRANSFER_UPLOAD = 0,
	FILE_TRANSFER_DOWNLOAD,
	FILE_TRANSFER_OTA
};

/** @brief Contains information about a file transfer */
struct device_manager_file_transfer
{
	/** @brief Pointer to library handle */
	iot_t **iot_lib;
	/** @brief Whether a request to cancel the operation */
	iot_bool_t cancel;
	/** @brief Total data downloaded/uploaded previously */
	long prev_byte;
	/** @brief The last time progress log is updated since the transfer begins in seconds */
	double last_update_time;
	/** @brief Path to the file to be uploaded or downloaded */
	char file_path[ PATH_MAX + 1u ];
	/** @brief Rename uploaded file */
	char file_name[ PATH_MAX + 1u ];
	/** @brief Remote URL of the transfer */
	char transfer_url[ PATH_MAX + 1u ];
	/** @brief Remote URL of the transfer's response */
	char response_url[ PATH_MAX + 1u ];
	/** @brief token required to access remote url */
	char token[ DEVICE_MANAGER_FILE_MAX_TOKEN_LENGTH + DEVICE_MANAGER_FILE_HEADER_TOKEN_KEY_LENGTH + 1u ];
	/** @brief Libcurl instance handling the transfer connection */
	CURL *transfer_lib_curl;
	/** @brief Libcurl instance handling the response connection */
	CURL *response_lib_curl;
	/** @brief Curl list to contain token and to be kept until transfer is completed */
	struct curl_slist *transfer_list;
	/** @brief Curl list to contain token and to be kept until transfer is completed */
	struct curl_slist *response_list;
	/** @brief Curl form to contain information regarding upload and to be kept until transfer is completed */
	struct curl_httppost *formpost;
	/** @brief File handle to the file being downloaded */
	os_file_t file_handle;
	/** @brief Expected checksum for a downloaded file */
	char checksum[ DEVICE_MANAGER_CHECKSUM_LENGTH + 1u ];
	/** @brief Contains information about ota manifest */
	struct device_manager_ota_manifest ota_transfer_info;
	/** @brief Types of checksum */
	enum file_transfer_checksum_type checksum_type;
	/** @brief State of a file transfer session */
	enum file_transfer_state state;
	/** @brief Type of file transfer */
	enum file_transfer_type type;
	/** @brief Time when a transfer session is expired */
	iot_timestamp_t expiry_time;
	/** @brief Time when a paused transfer session is resumed */
	iot_timestamp_t resume_time;
	/** @brief Mutex protecting file transer data */
	os_thread_mutex_t *file_transfer_mutex;
};

/** @brief Structure containing information for file io */
struct device_manager_file_io_info
{
	/** @brief Mutex protecting file transer data */
	os_thread_mutex_t file_transfer_mutex;
	/** @brief Number of file transfers in progress */
	size_t file_transfer_count;
	/** @brief Structure holding proxy server information */
	/*struct iot_proxy proxy_info;*/
	/** @brief Structure holding pointers to file transfer information */
	struct device_manager_file_transfer *file_transfer_ptr[IOT_WORKER_THREADS];
	/** @brief Structure holding file transfer information */
	struct device_manager_file_transfer file_transfer[IOT_WORKER_THREADS];
	/** @brief Download directory */
	char download_dir[ PATH_MAX + 1u ];
	/** @brief List of upload directories */
	char **upload_dirs;
	/** @brief Number of upload directories */
	size_t upload_dirs_count;
	/** @brief file cancel command */
	iot_action_t *file_cancel;
	/** @brief file upload command */
	iot_action_t *file_upload;
	/** @brief file download command */
	iot_action_t *file_download;
	/** @brief certificate file path */
	char cert_path[PATH_MAX + 1u];
	/** @brief SSL validation */
	iot_bool_t ssl_validate;
	/** @brief value for ssl host verification */
	long ssl_host_verification;
	/** @brief value for ssl peer verification */
	long ssl_peer_verification;
	/** @brief number of main loops to wait to check for pending transfers */
	size_t loop_wait_pending_transfers;
	/** @brief remove file after it is uploaded */
	iot_bool_t upload_file_remove;
};

/**
 * @brief Add directories to the list of upload directories
 *
 * @param[in,out]  device_manager_info      pointer to app data
 * @param[in,out]  dirs                     list of directories to be added separated by tokens
 * @param[in]      token                    character separating directories
 *
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter
 */
iot_status_t device_manager_file_add_upload_directories(
	struct device_manager_info *device_manager_info,
	char *dirs, char token );

/**
 * @brief Create the default directories required for file transfers
 *
 * @param[in]  device_manager_info     pointer to app data
 * @param[in]  timeout                 maximum timeout for operation to complete in ms
 *
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter
 */
iot_status_t device_manager_file_create_default_directories(
	struct device_manager_info *device_manager_info,
	iot_millisecond_t timeout );

/**
 * @brief Deregisters the functions for file operations with the library
 *
 * @param[in,out]  device_manager_info      pointer to app data
 *
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter
 */
iot_status_t device_manager_file_deregister(
	struct device_manager_info *device_manager_info );

/**
 * @brief Registers the functions for file operations with the library
 *
 * @param[in,out]  device_manager_info      pointer to app data
 *
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter
 */
iot_status_t device_manager_file_register(
	struct device_manager_info *device_manager_info );

/**
 * @brief Initialized curl global library that is required for file-io operations
 *
 * @param[in,out]  device_manager_info      pointer to app data
 * @param[in]      init_curl_lib            init curl lib
 *
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter
 */
iot_status_t device_manager_file_initialize(
	struct device_manager_info *device_manager_info,
	iot_bool_t init_curl_lib );

/**
 * @brief  global libcurl and memory cleanup that is required for file-io operations
 *
 * @param[in,out]  device_manager_info      pointer to app data
 *
 */
void device_manager_file_terminate(
	struct device_manager_info *device_manager_info );

/**
 * @brief Registers the functions for file operations with the library
 *
 * @param[in,out]  device_manager_info      pointer to app data
 */
void device_manager_file_check_pending_transfers(
	struct device_manager_info *device_manager_info );

/**
 * @brief Registers the functions for file operations with the library
 *
 * @param[in,out]  device_manager_info      pointer to app data
 */
void device_manager_file_cancel_all(
	struct device_manager_info *device_manager_info );
/**
 * @brief Perform file download
 *
 * @param[in,out] device_manager_info            pointer to device_manager information structure
 * @param[in]      transfer_type                 transfer type,OTA/file download/file upload
 * @param[in]      download_url                  url to perform file download
 * @param[in]      response_url                  url to update status about the transfer
 * @param[in]      token                         JWT token used for authentication
 * @param[in]      file_name                     name for the file to be uploaded as (optional)
 * @param[in]      checksum                      checksum to verify downloaded file (optional)
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
iot_status_t device_manager_file_download_perform(
	struct device_manager_info *device_manager_info,
	enum file_transfer_type transfer_type,
	const char *download_url, const char *response_url,
	const char *token, const char *file_name,
	const char *checksum);
/**
 * @brief Set the default upload and download directories
 *
 * @param[in,out] device_manager_info            pointer to device_manager information structure
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_FAILURE                    on failure
 * @retval IOT_STATUS_BAD_PARAMETER              bad parameters
 */
iot_status_t device_manager_file_set_default_directories(
	struct device_manager_info *device_manager_info );
/**
 * @brief Perform file transfer to the cloud
 *
 * @param[in,out]  device_manager_info      pointer to device_manager
 *                                          information structure
 * @param[in,out]  transfer                 pointer to information regarding the
 *                                          file transfer session
 * @param[in]      max_retry                maximum number of retries, 0 and 1
 *                                          mean 1 retry only
 *
 * @retval IOT_STATUS_BAD_PARAMETER    request or user data provided is invalid
 * @retval IOT_STATUS_FAILURE          failed to start file transfer
 * @retval IOT_STATUS_SUCCESS          file transfer started
 */
iot_status_t device_manager_file_transfer_perform(
	struct device_manager_info *device_manager_info,
	struct device_manager_file_transfer *transfer,
	uint8_t max_retry );

#endif /* DEVICE_MANAGER_FILE_H */

