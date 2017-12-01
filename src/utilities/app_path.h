/**
 * @file
 * @brief header file for path helper operations for applications
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

#ifndef APP_PATH_H
#define APP_PATH_H

#include "api/public/iot.h"            /* for size_t, iot_bool_t */

/**
 * @brief Iterates through a path and creates all directories if they do not exist
 *
 * @param[in]    path_in               path to create
 * @param[in]    timeout               timeout (in milliseconds) before
 *                                     directory creation fails
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          system error
 * @retval IOT_STATUS_SUCCESS          on success
 */

IOT_SECTION iot_status_t app_path_create(
	const char *path_in,
	unsigned int timeout );

/**
 * @brief Get the path to configuration directory
 *
 * @param[out]     path                buffer to put path to executable's directory
 * @param[in]      size                size of the buffer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_path_config_directory_get(
	char *path, const size_t size );

/**
 * @brief Get the path to executable's directory
 *
 * @param[out]     path                buffer to put path to executable's directory
 * @param[in]      size                size of the buffer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_path_executable_directory_get(
	char *path, const size_t size );

/**
 * @brief if a given path is not absolute, it modifies it as such
 *
 * @param[in,out]  path                path to test and update as required
 * @param[in]      path_max            length of the path buffer
 * @param[in]      relative_to_install the given path (if not absolute) is
 *                                     relative to the installation directory
 *                                     and not the current directory
 *
 * @retval 0u      path is NULL or failed to obtain current directory or failed
 *                 to obtain the path to the executable
 * @retval >0u     characters in absolute path (without null-terminator)
 *
 * @note If the buffer is too small the return value is the number of characters
 *       required (-1 for the null-terminator).  Thus, one can check for success
 *       if the result is less than (<) @c path_max.
 */
IOT_SECTION size_t app_path_make_absolute( char *path, size_t path_max,
	iot_bool_t relative_to_install );

/**
 * @brief Get the path to runtime directory
 *
 * @param[out]     path                buffer to put path to executable's directory
 * @param[in]      size                size of the buffer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t app_path_runtime_directory_get(
	char *path, const size_t size );

/**
 * @brief Function very similar to the 'which' command on Linux/Unix, with
 *        support for defined operating system extensions + cross-platform
 *
 * This function finds the path to an executable given a 'current directory';
 * If the file doesn't exist in the 'current directory', then it tries to find
 * the file with all the extensions defined with the environment variable
 * 'PATHEXT'; fallowed by all the paths defined in the 'PATH' environment
 * variable + all extensions defined with 'PATHEXT' environment variable
 *
 * @param[in]      path                location to store result
 * @param[in]      path_max            maximum size of destination buffer
 * @param[in]      cur_dir             user specified current directory to start
 * @param[in]      file_name           file name to search for
 *
 * @retval 0u      path is NULL or failed to obtain current directory
 * @retval >0u     characters in absolute path (without null-terminator)
 *
 * @note If the buffer is too small the return value is the number of characters
 *       required (-1 for the null-terminator).  Thus, one can check for success
 *       if the result is less than (<) @c path_max.
 */
IOT_SECTION size_t app_path_which( char *path, size_t path_max,
	const char *cur_dir, const char *file_name );

#endif /* ifndef APP_PATH_H */

