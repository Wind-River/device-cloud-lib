/**
 * @brief Source file for the device-manager app.
 *
 * @copyright Copyright (C) 2016-2018 Wind River Systems, Inc. All Rights Reserved.
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

#include "device_manager_main.h"

#include "device_manager_file.h"

#include "utilities/app_arg.h"        /* for struct app_arg & functions */
#include "utilities/app_log.h"        /* for app_log function */
#include "utilities/app_path.h"       /* for app_path_which function */

#include "iot_build.h"
#include "iot_json.h"                 /* for json */

#include <os.h>                       /* for os specific functions */
#include <stdlib.h>                   /* for EXIT_SUCCESS, EXIT_FAILURE */

/** @brief Name of "host" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_HOST                "host"
/** @brief Name of "protocol" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_PROTOCOL            "protocol"
/** @brief Name of "url" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_URL                 "url"
/** @brief Name of "debug" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_DEBUG               "debug-mode"

/** @brief Name of action to update the list of supported remote login protocols */
#define REMOTE_LOGIN_UPDATE_ACTION            "get_remote_access_info"

/** @brief Name of the parameter to save file as */
#define DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME "file_name"
/** @brief Name of the parameter for using global file store */
#define DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE "use_global_store"
/** @brief Name of the parameter for file path on device */
#define DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH "file_path"

#if defined( _WIN32 )
	/** @brief IoT device manager service ID */
#	define IOT_DEVICE_MANAGER_ID         IOT_DEVICE_MANAGER_TARGET
	/** @brief Remote Desktop service ID on Windows */
#	define IOT_REMOTE_DESKTOP_ID         "TermService"
#endif /* if defined( _WIN32 ) */

#if defined( __ANDROID__ )
/** @brief Command and parameters to enable telnetd on the device */
#define ENABLE_TELNETD_LOCALHOST \
	"if [ 0 -eq $( netstat | grep 23 | grep -c LISTEN ) ]; then busybox telnetd -l /system/bin/sh -b 127.0.0.1:23; fi"
#endif /* __ANDROID__ */

/**
 * @brief Structure containing application specific data
 */
extern struct device_manager_info APP_DATA;
struct device_manager_info APP_DATA;


/**
 * @brief Sets the basic details of an action initially in the device manager
 *        global structure
 *
 * @param[in,out]  s                   device manager configuration structure
 * @param[in]      idx                 index of action to initialize
 * @param[in]      action_name         name of action to initialize
 * @param[in]      config_id           configuration file id to enable/disable
 * @param[in]      default_enabled     whether action is enabled by default
 */
static void device_manager_action_initialize( struct device_manager_info *s,
	enum device_manager_config_idx idx, const char *action_name,
	const char *config_id, iot_bool_t default_enabled );

/* function definitions */
#if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 )
/**
 * @brief Deregisters device-manager related actions
 *
 * @param[in]  device_manager                    application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_actions_deregister(
	struct device_manager_info *device_manager );
#endif /*if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 ) */

/**
 * @brief Registers device-manager related actions
 *
 * @param[in]  device_manager                    application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_actions_register(
	struct device_manager_info *device_manager );

/**
 * @brief Reads the agent configuration file
 *
 * @param[in,out]  device_manager_info           pointer to the agent data structure
 * @param[in]      app_path                      path to the executable calling the
 *                                               function
 * @param[in]      config_file                   command line provided configuation file
 *                                               path (if specified)
 *
 * @retval IOT_STATUS_BAD_PARAMETER              invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND                  could not find the agent configuration
 *                                               file
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_config_read(
	struct device_manager_info *device_manager_info,
	const char *app_path, const char *config_file );

/**
 * @brief Checks if service (or port) is actively listening on the localhost
 *
 * @note @p service can be a numeric port number.  If @p service is not a number
 * then an entry is looked for in the /etc/services file for the corresponding
 * port
 *
 * @param[in]  service                 service to check
 *
 * @retval 0       The service name was not found or not active
 * @retval >0      The port number of the service, if active.
 */
static iot_uint16_t check_listening_service(
	const char *service );

/**
 * @brief Callback function to download a file from the cloud
 *
 * @param[in,out]  request             request from the cloud
 * @param[in,out]  user_data           pointer to user defined data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    request or user data provided is invalid
 * @retval IOT_STATUS_SUCCESS          file download started
 * @retval IOT_STATUS_FAILURE          failed start file download
 */
static iot_status_t device_manager_file_download(
	iot_action_request_t *request,
	void *user_data );

/**
 * @brief Callback function to upload a file to the cloud
 *
 * @param[in,out]  request             request from the cloud
 * @param[in,out]  user_data           pointer to user defined data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    request or user data provided is invalid
 * @retval IOT_STATUS_SUCCESS          file upload started
 * @retval IOT_STATUS_FAILURE          failed start file upload
 */
static iot_status_t device_manager_file_upload(
	iot_action_request_t *request,
	void *user_data );

/**
 * @brief Callback function to handle progress updates on file transfers
 *
 * @param[in,out]  progress            object containing progress information
 * @param[in]      user_data           user data: pointer to the library handle
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static void device_manager_file_progress(
		const iot_file_progress_t *progress,
		void *user_data );

/**
 * @brief Initializes the application
 *
 * @param[in]      app_path                      path to the application
 * @param[in,out]  device_manager                application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_initialize( const char *app_path,
	struct device_manager_info *device_manager );

/**
 * @brief Correctly formats the command to call iot-control with the full path
 *        and some options.
 *
 * @param[out]     full_path                     buffer to write command to
 * @param[in]      max_len                       size of destination buffer
 * @param[in]      device_manager                application specific data
 * @param[in]      options                       string with option flags for
 *                                               iot-control
 *
 * @retval IOT_STATUS_BAD_PARAMETER              Invalid parameter
 * @retval IOT_STATUS_FULL                       Buffer is not large enough
 * @retval IOT_STATUS_SUCCESS                    Successfully created command
 */
static iot_status_t device_manager_make_control_command( char *full_path,
	size_t max_len, struct device_manager_info *device_manager,
	const char *options );

/**
 * @brief Run OS command
 *
 * @param[in]      cmd                           command
 * @param[in]      blocking_action               blocking_action
 *                                               IOT_TRUE: blocking
 *                                               IOT_FALSE: non_blocking
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_FAILURE                    on failure
 * @retval IOT_STATUS_BAD_PARAMETER              bad parameter
 */
static iot_status_t device_manager_run_os_command( const char *cmd,
	iot_bool_t blocking_action );

/**
 * @brief Handles terminatation signal and tears down gracefully
 *
 * @param[in]      signum                        signal number that was caught
 */
static void device_manager_sig_handler( int signum );

/**
 * @brief Cleans up the application before exitting
 *
 * @param[in]  device_manager                    application specific data
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_*                          on failure
 */
static iot_status_t device_manager_terminate(
	struct device_manager_info *device_manager );

/**
 * @brief Callback function to reset the device manager
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in,out]  user_data           pointer to a struct device_manager_info
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_reset(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to quit the device manager
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in,out]  user_data           pointer to a struct device_manager_info
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_quit(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to perform device decommission
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in,out]  user_data           pointer to a struct device_manager_info
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_device_decommission(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to perform device reboot
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in,out]  user_data           pointer to a struct device_manager_info
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_device_reboot(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to return the agent shutdown
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in,out]  user_data           pointer to a struct device_manager_info
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_device_shutdown(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to return the remote login
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_remote_login(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to update the cloud attribute containing the list
 *        of supported remote login protocols
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          failed to obtain list
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_remote_login_update(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function diagnostic action to respond with timestamp
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in,out]  user_data           pointer to a struct device_manager_info
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_ping(
	iot_action_request_t* request,
	void *user_data );



/* function implementations */
void device_manager_action_initialize( struct device_manager_info *s,
	enum device_manager_config_idx idx, const char *action_name,
	const char *config_id, iot_bool_t default_enabled )
{
	if ( s )
	{
		s->actions[idx].action_name = action_name;
		s->actions[idx].config_id = config_id;
		s->actions[idx].enabled = default_enabled;
		s->actions[idx].ptr = NULL;
	}
}

#if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 )
iot_status_t device_manager_actions_deregister(
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{

		iot_action_t *const dump_log_files = device_manager->dump_log_files;
		iot_action_t *const agent_reset = device_manager->agent_reset;
		iot_action_t *const decommission_device = device_manager->decommission_device;
		iot_action_t *const device_shutdown = device_manager->device_shutdown;
		iot_action_t *const remote_login = device_manager->remote_login;
		iot_action_t *file_upload = NULL;
		iot_action_t *file_download = device_manager->file_download;
		iot_action_t *const device_reboot = device_manager->device_reboot;

#if !defined( _WIN32 ) && !defined( __VXWORKS__ )
		iot_action_t *const restore_factory_images = device_manager->restore_factory_images;

		/* restore_factory_images */
		if ( restore_factory_images )
		{
			iot_action_deregister( restore_factory_images, NULL, 0u );
			iot_action_free( restore_factory_images, 0u );
			device_manager->restore_factory_images = NULL;
		}
#endif /* if !defined( _WIN32 ) && !defined( __VXWORKS__ ) */

		/* device shutdown */
		if ( device_shutdown )
		{
			iot_action_deregister( device_shutdown, NULL, 0u );
			iot_action_free( device_shutdown, 0u );
			device_manager->device_shutdown = NULL;
		}

		/* device reboot */
		if ( device_reboot )
		{
			iot_action_deregister( device_reboot, NULL, 0u );
			iot_action_free( device_reboot, 0u );
			device_manager->device_reboot = NULL;
		}

		/* decommission device */
		if ( decommission_device )
		{
			iot_action_deregister( decommission_device, NULL, 0u );
			iot_action_free( decommission_device, 0u );
			device_manager->decommission_device = NULL;
		}

		/* agent reset */
		if ( agent_reset )
		{
			iot_action_deregister( agent_reset, NULL, 0u );
			iot_action_free( agent_reset, 0u );
			device_manager->agent_reset = NULL;
		}

		/* dump log files */
		if ( dump_log_files )
		{
			iot_action_deregister( dump_log_files, NULL, 0u );
			iot_action_free( dump_log_files, 0u );
			device_manager->dump_log_files = NULL;
		}

		/* remote login */
		if ( remote_login )
		{
			iot_action_deregister( remote_login, NULL, 0u );
			iot_action_free( remote_login, 0u );
			device_manager->remote_login = NULL;
		}

		/* manifest(ota) */
		device_manager_ota_deregister( device_manager );

#if !defined( NO_FILEIO_SUPPORT )
		/* file-io */
		if ( file_download )
		{
			iot_action_deregister( file_download, NULL, 0u );
			iot_action_free( file_download, 0u );
			device_manager->file_download = NULL;
		}
		if ( file_upload )
		{
			iot_action_deregister( file_upload, NULL, 0u );
			iot_action_free( file_upload, 0u );
			device_manager->file_upload = NULL;
		}
#endif /* if !defined( NO_FILEIO_SUPPORT ) */

		result = IOT_STATUS_SUCCESS;
	}

	return result;
}
#endif /* if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 ) */

iot_status_t device_manager_actions_register(
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		struct device_manager_action *action;
		char command_path[ PATH_MAX + 1u ];
		iot_t *const iot_lib = device_manager->iot_lib;

#if !defined( NO_FILEIO_SUPPORT )
		/* file transfer */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_FILE_DOWNLOAD];
		if ( action->enabled != IOT_FALSE )
		{
			/* File Download */
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );

			/* global store is optional */
			iot_action_parameter_add( action->ptr,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
				IOT_PARAMETER_IN, IOT_TYPE_BOOL, 0u );

			/* file name */
			iot_action_parameter_add( action->ptr,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME,
				IOT_PARAMETER_IN_REQUIRED, IOT_TYPE_STRING, 0u );

			/* file path */
			iot_action_parameter_add( action->ptr,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
				IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

			result = iot_action_register_callback( action->ptr,
				&device_manager_file_download, device_manager, NULL, 0u );

			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}

		action = &device_manager->actions[DEVICE_MANAGER_IDX_FILE_UPLOAD];
		if ( action->enabled != IOT_FALSE )
		{
			/* File Upload */
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );

			/* global store is optional */
			iot_action_parameter_add( action->ptr,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
				IOT_PARAMETER_IN, IOT_TYPE_BOOL, 0u );

			/* file name  */
			iot_action_parameter_add( action->ptr,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME,
				IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

			/* file path */
			iot_action_parameter_add( action->ptr,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
				IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

			result = iot_action_register_callback( action->ptr,
				&device_manager_file_upload, device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}
#endif /* if !defined( NO_FILEIO_SUPPORT ) */

		/* agent quit */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_AGENT_QUIT];
		if ( action->enabled != IOT_FALSE )
		{

			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );
			iot_action_flags_set( action->ptr,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );

			result = iot_action_register_callback(
				action->ptr, &on_action_agent_quit,
				(void*)device_manager, NULL, 0u );

			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}

		/* ping */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_PING];
		if ( action->enabled != IOT_FALSE )
		{
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );

			result = iot_action_register_callback(
				action->ptr, &on_action_ping,
				(void*)device_manager, NULL, 0u );

			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}

		/* device shutdown */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_DEVICE_SHUTDOWN];
		if ( action->enabled != IOT_FALSE )
		{

			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );
			iot_action_flags_set( action->ptr,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
			result = iot_action_register_callback(
				action->ptr, &on_action_device_shutdown,
				(void*)device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}

		/* decommission device */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_DEVICE_DECOMMISSION];
		if ( action->enabled != IOT_FALSE )
		{
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );
			iot_action_flags_set( action->ptr,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
			result = iot_action_register_callback( action->ptr,
					&on_action_device_decommission,
					(void*)device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}

		/* agent reset */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_AGENT_RESET];
		if ( action->enabled != IOT_FALSE )
		{
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );
			iot_action_flags_set( action->ptr,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );

			result = iot_action_register_callback(
				action->ptr, &on_action_agent_reset,
				(void*)device_manager, NULL, 0u );

			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}

#if !defined( NO_FILEIO_SUPPORT )
		/* dump log files */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_DUMP_LOG_FILES];
		if ( action->enabled != IOT_FALSE )
		{
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );
			iot_action_flags_set( action->ptr,
				IOT_ACTION_EXCLUSIVE_APP );
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --dump" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command(
					action->ptr, command_path, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}
#endif /* if !defined( NO_FILEIO_SUPPORT ) */

		/* manifest (ota) */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_SOFTWARE_UPDATE];
		if ( action->enabled != IOT_FALSE )
			if ( device_manager_ota_register( device_manager ) != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",
					"Failed to register software update actions" );

		/* remote_login */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_REMOTE_LOGIN];
		if ( action->enabled != IOT_FALSE )
		{
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );

			/* param to call set on  */
			iot_action_parameter_add( action->ptr,
				REMOTE_LOGIN_PARAM_HOST,
				IOT_PARAMETER_IN,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( action->ptr,
				REMOTE_LOGIN_PARAM_PROTOCOL,
				IOT_PARAMETER_IN_REQUIRED,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( action->ptr,
				REMOTE_LOGIN_PARAM_URL,
				IOT_PARAMETER_IN_REQUIRED,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( action->ptr,
				REMOTE_LOGIN_PARAM_DEBUG,
				IOT_PARAMETER_IN,
				IOT_TYPE_BOOL, 0u );

			result = iot_action_register_callback(
				action->ptr, &on_action_remote_login,
				(void*)device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}

			/* remote login protocols */
			if ( result == IOT_STATUS_SUCCESS )
			{
				iot_action_t *const ra_update =
					iot_action_allocate( iot_lib,
						REMOTE_LOGIN_UPDATE_ACTION );
				result = iot_action_register_callback(
					ra_update,
					&on_action_remote_login_update,
					(void*)device_manager, NULL, 0u );
				if ( result != IOT_STATUS_SUCCESS )
				{
					IOT_LOG( iot_lib, IOT_LOG_ERROR,
						"Failed to register %s action. Reason: %s",
						REMOTE_LOGIN_UPDATE_ACTION,
						iot_error( result ) );
					iot_action_free( ra_update, 0u );
				}
			}
		}

		/* device reboot */
		action = &device_manager->actions[DEVICE_MANAGER_IDX_DEVICE_REBOOT];
		if ( action->enabled != IOT_FALSE )
		{
			action->ptr = iot_action_allocate( iot_lib,
				action->action_name );
			iot_action_flags_set( action->ptr,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
			result = iot_action_register_callback(
				action->ptr, &on_action_device_reboot,
				(void*)device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					action->action_name,
					iot_error( result ) );
				iot_action_free( action->ptr, 0u );
				action->ptr = NULL;
			}
		}
	}
	return result;
}

iot_status_t device_manager_config_read(
	struct device_manager_info *device_manager_info,
	const char *app_path, const char *config_file )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	IOT_LOG( NULL, IOT_LOG_INFO,
		"  * Checking for configuration file %s ...",
		IOT_DEFAULT_FILE_DEVICE_MANAGER );
	if ( device_manager_info && app_path )
	{
		size_t cfg_dir_len;
		char default_iot_cfg_path[PATH_MAX + 1u];
		os_file_t fd;
		struct device_manager_file_io_info *const file_io =
			&device_manager_info->file_io_info;

		/* Default values */
		iot_directory_name_get( IOT_DIR_RUNTIME,
			device_manager_info->runtime_dir,
			PATH_MAX );
		os_env_expand( device_manager_info->runtime_dir, 0u, PATH_MAX );
		device_manager_info->runtime_dir[PATH_MAX] = '\0';
		IOT_LOG( NULL, IOT_LOG_INFO,
			"  * Setting default runtime dir to %s",
			device_manager_info->runtime_dir );

		/* set default of uploaded file removal */
		file_io->upload_file_remove = IOT_DEFAULT_UPLOAD_REMOVE_ON_SUCCESS;

		/* Read config file */
		result = IOT_STATUS_NOT_FOUND;

		/* set the default path */
		cfg_dir_len = iot_directory_name_get( IOT_DIR_CONFIG,
			default_iot_cfg_path, PATH_MAX );
		os_snprintf( &default_iot_cfg_path[cfg_dir_len],
			PATH_MAX - cfg_dir_len, "%c%s",
			OS_DIR_SEP, IOT_DEFAULT_FILE_DEVICE_MANAGER );
		default_iot_cfg_path[PATH_MAX] = '\0';

		if ( !config_file || config_file[0] == '\0' )
			config_file = default_iot_cfg_path;

		IOT_LOG( NULL, IOT_LOG_INFO,
			"  * Reading config file %s", config_file );
		fd = os_file_open( config_file, OS_READ );
		if ( fd != OS_FILE_INVALID )
		{
			size_t json_size = 0u;
			size_t json_max_size = 4096u;
			iot_json_decoder_t *json;
			const iot_json_item_t *json_root = NULL;
			char *json_string = NULL;

			result = IOT_STATUS_NO_MEMORY;
			json_size = (size_t)os_file_size_handle( fd );
			if ( json_max_size > json_size || json_max_size == 0 )
			{
				json_string = (char *)os_malloc( json_size + 1 );
				if ( json_string )
				{
					json_size = os_file_read( json_string,
						1, json_size, fd );
					json_string[ json_size ] = '\0';
					if ( json_size > 0 )
						result = IOT_STATUS_SUCCESS;
				}
			}
			/* now parse the json string read above */
			if ( result == IOT_STATUS_SUCCESS && json_string )
			{
				char err_msg[1024u];
				const char *temp = NULL;
				size_t temp_len = 0u;
#ifdef IOT_STACK_ONLY
				char buffer[1024u];
				json = iot_json_decode_initialize(
					buffer, 1024u, 0u );
#else
				json = iot_json_decode_initialize( NULL, 0u,
					IOT_JSON_FLAG_DYNAMIC );
#endif
				if ( json && iot_json_decode_parse( json,
					json_string, json_size, &json_root,
					err_msg, 1024u ) == IOT_STATUS_SUCCESS )
				{
					enum device_manager_config_idx idx;
					const iot_json_item_t *j_action_top;
					const iot_json_item_t *const j_actions_enabled =
						iot_json_decode_object_find(
							json, json_root,
							"actions_enabled" );
					/* handle all the boolean default actions */
					IOT_LOG( NULL, IOT_LOG_INFO, "%s",
						"Default Configuration:" );
					for ( idx = DEVICE_MANAGER_IDX_FIRST;
						j_actions_enabled &&
						idx < DEVICE_MANAGER_IDX_LAST; ++idx )
					{
						struct device_manager_action *cfg =
							&device_manager_info->actions[idx];
						const iot_json_item_t *const j_action =
							iot_json_decode_object_find(
							json, j_actions_enabled,
							cfg->config_id );
						if ( j_action )
							iot_json_decode_bool( json,
								j_action, &cfg->enabled );
						if ( cfg->enabled == IOT_FALSE )
						{
							IOT_LOG( NULL, IOT_LOG_INFO,
								"  * %s is disabled",
								cfg->action_name );
						}
						else
						{
							IOT_LOG( NULL, IOT_LOG_INFO,
								"  * %s is enabled",
								cfg->action_name );
						}
					}

					/* get the runtime dir */
					j_action_top = iot_json_decode_object_find(
						json, json_root, "runtime_dir" );

					iot_json_decode_string( json,
						j_action_top, &temp,
						&temp_len  );
					if ( temp && temp[0] != '\0' )
					{
						if ( temp_len > PATH_MAX )
							temp_len = PATH_MAX;
						os_strncpy(device_manager_info->runtime_dir,
							temp, temp_len );

						os_env_expand( device_manager_info->runtime_dir, 0u, PATH_MAX );
						device_manager_info->runtime_dir[ temp_len ] = '\0';
						IOT_LOG( NULL, IOT_LOG_INFO,
							"  * runtime dir = %s", device_manager_info->runtime_dir );
						if ( os_directory_create(
							device_manager_info->runtime_dir,
							DIRECTORY_CREATE_MAX_TIMEOUT )
						!= OS_STATUS_SUCCESS )
							IOT_LOG( NULL, IOT_LOG_INFO,
								"Failed to create %s",
								device_manager_info->runtime_dir );
					}

					/* get the log level */
					j_action_top = iot_json_decode_object_find(
							json, json_root, "log_level" );

					iot_json_decode_string( json,
							j_action_top, &temp,
							&temp_len  );
					if ( temp && temp[0] != '\0' )
					{
						if ( temp_len > PATH_MAX )
							temp_len = PATH_MAX;
						os_strncpy( device_manager_info->log_level, temp, temp_len );
						device_manager_info->log_level[ temp_len ] = '\0';
						IOT_LOG( NULL, IOT_LOG_INFO,
							"  * log_level = %s",
							device_manager_info->log_level );
					}
				}
				else
					IOT_LOG( NULL, IOT_LOG_ERROR,
						"%s", err_msg );

				os_free( json_string );
				iot_json_decode_terminate( json );
			}
		}
		os_file_close( fd );
	}
	return result;
}

iot_status_t device_manager_file_download(
	iot_action_request_t *request,
	void *user_data )
{
	const char *file_name = NULL;

	/* FIXME: set default to true due to Android issue where if false, it
	 * does not get updated to true */
	const iot_bool_t use_global_store = IOT_TRUE;
	const char *file_path = NULL;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	/* get the params: note optional string params that are null
	 * return an error, so ignore */
	if ( request && user_data )
	{
		struct device_manager_info *dm =
			(struct device_manager_info *)user_data;

		/* file_name */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME,
			IOT_FALSE, IOT_TYPE_STRING, &file_name );
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %s result=%d",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME, file_name,
			(int)result);

		/* file_path */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
			IOT_FALSE, IOT_TYPE_STRING, &file_path );
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %s result=%d",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH, file_path,
			(int)result);

		/* use global store */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
			IOT_FALSE, IOT_TYPE_BOOL, &use_global_store);
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %d result=%d",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
			(int)use_global_store, (int)result);

		/* support a file_name, but no path.  That means,
		 * store it in the default runtime dir with the file_name */
		if ( ! file_path  )
			file_path = file_name;

		if ( dm )
		{
			iot_options_t *options = NULL;
			if ( use_global_store != IOT_FALSE )
			{
				options = iot_options_allocate( dm->iot_lib );
				iot_options_set_bool( options, "global",
					use_global_store );
			}

			/* download will return immediately.  Use the
			 * callback to track progress */
			result = iot_file_download(
				dm->iot_lib,
				NULL,
				options,
				file_name,
				file_path,
				&device_manager_file_progress,
				dm->iot_lib );

			if ( options )
				iot_options_free( options );
		}
	}
	return result;
}

iot_status_t device_manager_file_upload(
	iot_action_request_t *request,
	void *user_data )
{
	const char *file_name = NULL;
	const iot_bool_t use_global_store = IOT_FALSE;
	const char *file_path = NULL;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	/* get the params */
	if ( request && user_data )
	{
		struct device_manager_info *dm =
			(struct device_manager_info *)user_data;

		/* file_name */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME,
			IOT_FALSE, IOT_TYPE_STRING, &file_name );
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %s result=%d",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME, file_name,
			(int)result);

		/* file_path */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
			IOT_FALSE, IOT_TYPE_STRING, &file_path );
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %s result=%d",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH, file_path,
			(int)result);

		/* use global store */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
			IOT_FALSE, IOT_TYPE_BOOL, &use_global_store);
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %d result=%d",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
			(int)use_global_store, (int)result);

		if ( dm )
		{
			iot_options_t *options = NULL;
			if ( use_global_store != IOT_FALSE )
			{
				options = iot_options_allocate( dm->iot_lib );
				iot_options_set_bool( options, "global",
					use_global_store );
			}

			result = iot_file_upload(
				dm->iot_lib,
				NULL,
				options,
				file_name,
				file_path,
				NULL,
				NULL );

			iot_options_free( options );
		}
	}
	return result;
}

void device_manager_file_progress(
		const iot_file_progress_t *progress,
		void *user_data )
{
	iot_status_t status = IOT_STATUS_FAILURE;
	iot_float32_t percent = 0.0f;
	iot_bool_t complete = IOT_FALSE;

	iot_file_progress_get( progress, &status, &percent, &complete );
	IOT_LOG( (iot_t*)user_data, IOT_LOG_TRACE,
		"File Download Status: %s (completed: %s [%f %%])",
		iot_error( status ),
		( complete == IOT_FALSE ? "no" : "yes" ),
		(double)percent );
}

iot_uint16_t check_listening_service(
	const char *service )
{
	os_socket_t *socket;
	char localhost_addr[15];
	iot_uint16_t port_num = 0u;

	if ( service )
		port_num = (iot_uint16_t)os_atoi( service );

	/* support for ports by name */
	if ( port_num == 0u  )
	{
		os_service_entry_t *const ent =
			os_service_entry_by_name( service, NULL );
		if ( ent )
			port_num = (iot_uint16_t)ntohs( (uint16_t)ent->s_port );
		os_service_entry_close();
	}

	if ( port_num <= 0 ||
		os_get_host_address( "localhost", 0, localhost_addr, 15, AF_INET ) != 0 )
		port_num = 0u;

	if ( port_num > 0u && os_socket_open( &socket, localhost_addr, port_num,
		SOCK_STREAM, 0u, 1000u ) == OS_STATUS_SUCCESS )
	{
		if ( os_socket_connect( socket ) != OS_STATUS_SUCCESS )
			port_num = 0u;
		os_socket_close( socket );
	}
	return port_num;
}

iot_status_t device_manager_initialize( const char *app_path,
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_t *iot_lib = NULL;
		char *p_path = NULL;
#if defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
		struct device_manager_file_io_info *file_io = &device_manager->file_io_info;
		os_thread_mutex_t *file_transfer_lock = &file_io->file_transfer_mutex;
#endif /* defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */

		iot_lib = iot_initialize( "device-manager", NULL, 0u );
		if ( iot_lib == NULL )
		{
			os_fprintf( OS_STDERR,
				"Error: %s", "Failed to initialize IOT library" );
			return IOT_STATUS_FAILURE;
		}

#if defined( __ANDROID__ )
		/* start telnetd bind to localhost only */
		device_manager_run_os_command(
			ENABLE_TELNETD_LOCALHOST, IOT_TRUE );
#endif /* __ANDROID__ */

		/* Set user specified default log level */
		iot_log_level_set_string( iot_lib, device_manager->log_level );
		iot_log_callback_set( iot_lib, &app_log, NULL );

		/* Find the absolute path to where the application resides */
		os_strncpy( device_manager->app_path, app_path, PATH_MAX );
		p_path = os_strrchr( device_manager->app_path,
			OS_DIR_SEP );
		if ( p_path )
			*p_path = '\0';
		else
		{
#if defined( __VXWORKS__ )
			/* on vxworks applications reside in "/bin" directory */
			os_snprintf( device_manager->app_path, PATH_MAX,
				"%cbin", OS_DIR_SEP );
#elif defined( __ANDROID__ )
			/* on android applications reside in "/system/bin" */
			os_snprintf( device_manager->app_path, PATH_MAX,
				"%csystem%cbin", OS_DIR_SEP, OS_DIR_SEP );
#else /* elif defined( __ANDROID__ ) */
			/* applications reside in the current directory */
			os_strncpy( device_manager->app_path, ".", PATH_MAX );
#endif /* __ANDROID__ */
		}

		result = iot_connect( iot_lib, 0u );
		if ( result == IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Connected" );
		else
		{
			IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Failed to connect" );
			result = IOT_STATUS_FAILURE;
		}

		if ( result == IOT_STATUS_SUCCESS )
		{
			device_manager->iot_lib = iot_lib;

#if defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
			if ( os_thread_mutex_create( file_transfer_lock ) != OS_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",
					"Failed to create lock for file transfer" );
#endif /* if defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */
			if ( device_manager_actions_register( device_manager ) != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",
					"Failed to register device-manager actions" );
		}
		else
		{
			iot_terminate( iot_lib, 0u );
		}
	}
	return result;
}

int device_manager_main( int argc, char *argv[] )
{
	int result = EXIT_FAILURE;
	const char *config_file = NULL;
	struct app_arg args[] = {
		{ 'c', "configure", APP_ARG_FLAG_OPTIONAL,
			"file", &config_file, "configuration file", 0u },
		{ 'h', "help", APP_ARG_FLAG_OPTIONAL,
			NULL, NULL, "display help menu", 0u },
		{ 's', "service", APP_ARG_FLAG_OPTIONAL,
			NULL, NULL, "run as a service", 0u },
		{ 0, NULL, 0, NULL, NULL, NULL, 0u }
	};

	IOT_LOG( NULL, IOT_LOG_INFO, "%s", "Starting Device Manager" );
	result = app_arg_parse(args, argc, argv, NULL);
	if (result == EXIT_FAILURE || app_arg_count(args, 'h', NULL))
		app_arg_usage(args, 36u, argv[0],
		IOT_DEVICE_MANAGER_TARGET, NULL, NULL);
	else if (result == EXIT_SUCCESS)
	{
		enum device_manager_config_idx idx = DEVICE_MANAGER_IDX_FIRST;
		os_memzero( &APP_DATA, sizeof(struct device_manager_info) );
		device_manager_action_initialize( &APP_DATA, idx++,
			"reset_agent", "reset_agent",
			IOT_DEFAULT_ENABLE_AGENT_RESET );
		device_manager_action_initialize( &APP_DATA, idx++,
			"quit", "quit_app",
			IOT_DEFAULT_ENABLE_AGENT_QUIT );
		device_manager_action_initialize( &APP_DATA, idx++,
			"decommission_device", "decommission_device",
			IOT_DEFAULT_ENABLE_DECOMMISSION_DEVICE );
		device_manager_action_initialize( &APP_DATA, idx++,
			"reboot_device", "reboot_device",
			IOT_DEFAULT_ENABLE_DEVICE_REBOOT );
		device_manager_action_initialize( &APP_DATA, idx++,
			"shutdown_device", "shutdown_device",
			IOT_DEFAULT_ENABLE_DEVICE_SHUTDOWN );
		device_manager_action_initialize( &APP_DATA, idx++,
			"Dump Log Files", "dump_log_files",
			IOT_DEFAULT_ENABLE_DUMP_LOG_FILES );
		device_manager_action_initialize( &APP_DATA, idx++,
			"file_download", "file_transfers",
			IOT_DEFAULT_ENABLE_FILE_TRANSFERS );
		device_manager_action_initialize( &APP_DATA, idx++,
			"file_upload", "file_transfers",
			IOT_DEFAULT_ENABLE_FILE_TRANSFERS );
		device_manager_action_initialize( &APP_DATA, idx++,
			"ping", "ping",
			IOT_DEFAULT_ENABLE_PING);
		device_manager_action_initialize( &APP_DATA, idx++,
			"remote-access", "remote_login",
			IOT_DEFAULT_ENABLE_REMOTE_LOGIN );
		device_manager_action_initialize( &APP_DATA, idx++,
			"restore_factory_images", "restore_factory_images",
			IOT_DEFAULT_ENABLE_RESTORE_FACTORY_IMAGES );
		device_manager_action_initialize( &APP_DATA, idx++,
			"software_update", "software_update",
			IOT_DEFAULT_ENABLE_SOFTWARE_UPDATE );

		if ( idx != DEVICE_MANAGER_IDX_LAST )
		{
			IOT_LOG( NULL, IOT_LOG_FATAL, "%s",
			"Fatal error setting up internal actions structure" );
			return EXIT_FAILURE;
		}

/** @todo vxWorks checking iot.cfg will be implemented later */
#if !defined( _WRS_KERNEL )
		device_manager_config_read( &APP_DATA, argv[0], config_file );
#endif /* if !defined( _WRS_KERNEL ) */
		if ( app_arg_count( args, 's', "service" ) > 0u )
		{
			const char *remove_args[] = { "-s", "--service" };

/* android does not have an hdc supported service handler */
#if defined( __ANDROID__ )
			result = EXIT_SUCCESS;
#else /* if defined( __ANDROID__ ) */
			result = os_service_run(
				IOT_DEVICE_MANAGER_TARGET, device_manager_main,
				argc, argv,
				sizeof( remove_args ) / sizeof( const char* ),
				remove_args, &device_manager_sig_handler,
				APP_DATA.runtime_dir );
#endif /* else if defined( __ANDROID__ ) */
		}
		else
		{
			if ( device_manager_initialize(argv[0], &APP_DATA )
				== IOT_STATUS_SUCCESS )
			{
				/* setup device manager attributes */
				os_system_info_t os;
				iot_action_request_t *req;
				iot_status_t req_status;

#if !defined( IOT_STACK_ONLY )
				os_adapter_t adapters;
				if ( os_adapters_obtain( &adapters ) == OS_STATUS_SUCCESS )
				{
					char *macs = NULL;
					size_t macs_len = 0u;
					do {
						char mac[24u] = {0};
						if ( os_adapters_mac( &adapters, mac, sizeof(mac) ) == OS_STATUS_SUCCESS )
						{
							const size_t mac_len =
								os_strlen( mac );
							char *new_macs;
							const size_t new_macs_len =
								macs_len + mac_len + 1u;
							new_macs =
								os_realloc( macs, new_macs_len );
							if ( new_macs )
							{
								if ( macs && macs_len > 0u )
									new_macs[macs_len - 1u] = ' ';
								os_strncpy(
									&new_macs[macs_len],
									mac,
									mac_len );
								macs = new_macs;
								macs_len = new_macs_len;

								/* null-terminate */
								macs[macs_len - 1u] = '\0';
							}
						}
					} while ( os_adapters_next( &adapters ) == OS_STATUS_SUCCESS );
					os_adapters_release( &adapters );

					/* if any macs are found then publish it */
					if ( macs )
					{
						iot_attribute_publish_string(
							APP_DATA.iot_lib,
							NULL, NULL,
							"mac_address",
							macs );
						os_free( macs );
					}
				}
#endif /* if !defined( IOT_STACK_ONLY ) */

				iot_attribute_publish_string(
					APP_DATA.iot_lib, NULL, NULL,
					"api_version", iot_version_str() );

				os_memzero( &os, sizeof( os_system_info_t ) );
				if ( os_system_info( &os ) == OS_STATUS_SUCCESS )
				{
					iot_attribute_publish_string(
						APP_DATA.iot_lib,
						NULL, NULL,
						"hostname", os.host_name );
					iot_attribute_publish_string(
						APP_DATA.iot_lib,
						NULL, NULL,
						"kernel",
						os.kernel_version );
					iot_attribute_publish_string(
						APP_DATA.iot_lib,
						NULL, NULL,
						"os_name",
						os.system_name );
					iot_attribute_publish_string(
						APP_DATA.iot_lib,
						NULL, NULL,
						"os_version",
						os.system_version );
					iot_attribute_publish_string(
						APP_DATA.iot_lib,
						NULL, NULL,
						"architecture",
						os.system_platform );
					/* not published:
					 * - amt_guid
					 * - platform
					 * - hdc_version
					 * - mac address
					 * - relay_version
					 */
				}

				/* Publish JSON of login_protocols */
				/* call action to update remote login protocols */
				req = iot_action_request_allocate(
					APP_DATA.iot_lib,
					REMOTE_LOGIN_UPDATE_ACTION, NULL );
				req_status = iot_action_request_execute(
					req, IOT_MILLISECONDS_IN_SECOND );
				if ( req_status != IOT_STATUS_SUCCESS )
				{
					IOT_LOG( APP_DATA.iot_lib, IOT_LOG_ERROR,
						"Failed to update remote login protocols.  Reason: %s",
						iot_error( req_status ) );
				}

				/* handles termination for memory clean up */
				os_terminate_handler( device_manager_sig_handler );

				IOT_LOG( APP_DATA.iot_lib, IOT_LOG_INFO, "%s",
					"Ready for some actions..." );

				while ( APP_DATA.iot_lib &&
					APP_DATA.iot_lib->to_quit == IOT_FALSE )
				{
					os_time_sleep( POLL_INTERVAL_MSEC,
						IOT_FALSE );
				}
				IOT_LOG( APP_DATA.iot_lib, IOT_LOG_INFO, "%s",
					"Exiting..." );
				result = EXIT_SUCCESS;
			}
			else
			{
				IOT_LOG( NULL, IOT_LOG_INFO, "%s",
					"Failed to initialize device-manager" );
				result = IOT_STATUS_FAILURE;
			}

			device_manager_terminate(&APP_DATA);
		}
	}
	return result;
}

static iot_status_t device_manager_make_control_command( char *full_path,
	size_t max_len, struct device_manager_info *device_manager,
	const char *options )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( full_path && device_manager && options )
	{
		char *ptr = NULL;
		size_t offset = 0u;

		ptr = full_path;
		result = IOT_STATUS_SUCCESS;
#if defined( _WIN32 )
		if ( max_len > 0u )
		{
			*ptr++ = '"';
			*ptr = '\0';
			++offset;
		}
		else
			result = IOT_STATUS_FULL;
#endif /* defined( _WIN32 ) */
		if ( result == IOT_STATUS_SUCCESS )
		{
			if ( os_make_path( ptr, max_len - offset,
				device_manager->app_path,
				IOT_CONTROL_TARGET, NULL ) == OS_STATUS_SUCCESS )
				result = IOT_STATUS_SUCCESS;
		}

		if ( result == IOT_STATUS_SUCCESS )
		{
			full_path[ max_len - 1u ] = '\0';
			offset = os_strlen( full_path );
			ptr = full_path + offset;
#if defined( _WIN32 )
			if ( offset < max_len - 1u )
			{
				*ptr++ = '"';
				*ptr = '\0';
				++offset;
			}
			else
				result = IOT_STATUS_FULL;
#endif /* defined( _WIN32 ) */
			if ( *options != ' ' && offset < max_len - 1u )
			{
				*ptr++ = ' ';
				*ptr = '\0';
				++offset;
			}
			if ( os_strlen( options ) < max_len - offset )
			{
				os_strncpy( ptr, options, max_len - offset );
				full_path[ max_len - 1u ] = '\0';
			}
			else
				result = IOT_STATUS_FULL;
		}
	}
	return result;
}

iot_status_t device_manager_run_os_command( const char *cmd,
	iot_bool_t blocking_action )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( cmd )
	{
		os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;

		args.cmd = cmd;
#if defined( __ANDROID__ )
		/* no sudo available on Android */
		args.privileged = OS_FALSE;
#else /* if defined( __ANDROID__ ) */
		args.privileged = OS_TRUE;
#endif /* else if defined( __ANDROID__ ) */
		args.block = OS_FALSE;
		if ( blocking_action != IOT_FALSE )
			args.block = OS_TRUE;

		if ( os_system_run( &args ) == OS_STATUS_SUCCESS &&
			args.return_code >= 0 )
			result = IOT_STATUS_SUCCESS;
		else
		{
			result = IOT_STATUS_FAILURE;
			IOT_LOG( APP_DATA.iot_lib, IOT_LOG_INFO,
				"Failed command: \"%s\" returned: %d",
				cmd, args.return_code );
		}
	}
	return result;
}

void device_manager_sig_handler( int signum )
{
	if ( signum == SIGTERM || signum == SIGINT )
	{
		IOT_LOG( NULL, IOT_LOG_INFO, "%s",
			"Received signal, Quitting..." );
		if ( APP_DATA.iot_lib )
			APP_DATA.iot_lib->to_quit = IOT_TRUE;
	}
	if ( signum == SIGCHLD )
		os_process_cleanup( );
}

iot_status_t device_manager_terminate(
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_t *iot_lib = device_manager->iot_lib;
#if defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
		os_thread_mutex_t *file_transfer_lock = &device_manager->file_io_info.file_transfer_mutex;
#endif /* if defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */

#if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 )
		device_manager_actions_deregister( device_manager );
#endif

#if defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
		os_thread_mutex_destroy( file_transfer_lock );
#endif /* if defined( IOT_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */

		iot_disconnect( iot_lib, 0u );
		iot_terminate( iot_lib, 0u );

#if !defined( NO_FILEIO_SUPPORT )
		/* must be done last */
		/*FIXME*/
		/*device_manager_file_terminate( device_manager );*/
#endif /* if !defined( NO_FILEIO_SUPPORT ) */
	}
	return result;
}

iot_status_t on_action_agent_reset(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;

	if ( device_manager && request )
	{
		char cmd[ PATH_MAX ];
		result = device_manager_make_control_command( cmd,
			PATH_MAX, device_manager,
			"--restart" );

		if ( result == IOT_STATUS_SUCCESS )
			result = device_manager_run_os_command(
				cmd, IOT_FALSE );
	}
	return result;
}

iot_status_t on_action_agent_quit(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;

	if ( device_manager && request )
	{
		iot_t *const iot_lib = device_manager->iot_lib;
		if ( iot_lib )
		{
			iot_lib->to_quit = IOT_TRUE;
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}


iot_status_t on_action_device_decommission(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;

	if ( device_manager && request )
	{
		char cmd[ PATH_MAX ];
		result = device_manager_make_control_command( cmd,
			PATH_MAX, device_manager, "--decommission" );

		if ( result == IOT_STATUS_SUCCESS )
			result = device_manager_run_os_command(
				cmd, IOT_TRUE );

#if defined( __VXWORKS__ )
		if ( result == IOT_STATUS_SUCCESS )
			os_system_shutdown( OS_TRUE, 0u );
#endif /* defined( __VXWORKS__ ) */
	}
	return result;
}

iot_status_t on_action_device_reboot(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;

	if ( device_manager && request )
	{
#if defined( __VXWORKS__ )
		os_status_t reboot_status = os_system_shutdown( OS_TRUE, 0u );
		result = IOT_STATUS_FAILURE;
		if ( reboot_status == OS_STATUS_INVOKED ||
			reboot_status == OS_STATUS_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* if defined( __VXWORKS__ ) */
		char cmd[ PATH_MAX ];
		result = device_manager_make_control_command( cmd,
			PATH_MAX, device_manager,
			"--reboot" );

		if ( result == IOT_STATUS_SUCCESS )
			result = device_manager_run_os_command(
				cmd, IOT_FALSE );
#endif /* if defined( __VXWORKS__ ) */
	}
	return result;
}

iot_status_t on_action_ping(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
#	define DM_TIMESTAMP_LEN    25u

	if ( device_manager && request )
	{
		iot_t *const iot_lib = device_manager->iot_lib;
		if ( iot_lib )
		{
			size_t out_len;
			iot_timestamp_t ts;
			char ts_str[ DM_TIMESTAMP_LEN +1 ];
			const char *response = "acknowledged";
			ts = iot_timestamp_now();

			/* TR50 format: "YYYY-MM-DDTHH:MM:SSZ" */
			out_len = os_time_format( ts_str, DM_TIMESTAMP_LEN,
				"%Y-%m-%dT%H:%M:%SZ", ts, OS_FALSE );
			ts_str[ out_len ] = '\0';

			IOT_LOG( iot_lib, IOT_LOG_DEBUG,
				"Responding to ping request with %s %s",
				response, ts_str);

			/* now set the out parameters */
			iot_action_parameter_set( request, "response",
				IOT_TYPE_STRING, response);
			iot_action_parameter_set( request, "time_stamp",
				IOT_TYPE_STRING, ts_str);
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t on_action_device_shutdown(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;

	if ( device_manager && request )
	{
#if defined( __VXWORKS__ )
		os_status_t reboot_status = os_system_shutdown( OS_FALSE, 0u );
		result = IOT_STATUS_FAILURE;
		if ( reboot_status == OS_STATUS_INVOKED ||
			reboot_status == OS_STATUS_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* if defined( __VXWORKS__ ) */
		char cmd[ PATH_MAX ];
		result = device_manager_make_control_command( cmd,
			PATH_MAX, device_manager,
			"--shutdown" );

		if ( result == IOT_STATUS_SUCCESS )
			result = device_manager_run_os_command( cmd, IOT_FALSE );
#endif /* else if defined( __VXWORKS__ ) */
	}
	return result;
}

iot_status_t on_action_remote_login( iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		const char *host_in = NULL;
		const char *url_in = NULL;
		const char *protocol_in = NULL;
		const iot_bool_t debug_mode = IOT_FALSE;
		os_file_t out_files[2] = { NULL, NULL };

		/* Support a debug option that supports logging */
		char log_file[256];

		/* read parameters */
		iot_action_parameter_get( request,
			REMOTE_LOGIN_PARAM_HOST, IOT_TRUE, IOT_TYPE_STRING,
			&host_in );
		iot_action_parameter_get( request,
			REMOTE_LOGIN_PARAM_PROTOCOL, IOT_TRUE, IOT_TYPE_STRING,
			&protocol_in );
		iot_action_parameter_get( request,
			REMOTE_LOGIN_PARAM_URL, IOT_TRUE, IOT_TYPE_STRING,
			&url_in );
		iot_action_parameter_get( request,
			REMOTE_LOGIN_PARAM_DEBUG, IOT_TRUE, IOT_TYPE_BOOL,
			&debug_mode);

		/* for debugging, create two file handles */
		if ( debug_mode != IOT_FALSE )
		{
			os_snprintf( log_file, PATH_MAX, "%s%c%s-%s",
				device_manager->runtime_dir, OS_DIR_SEP,
				IOT_TARGET_RELAY, "stdout.log" );
			out_files[0] = os_file_open(log_file,OS_CREATE | OS_WRITE);

			os_snprintf( log_file, PATH_MAX, "%s%c%s-%s",
				device_manager->runtime_dir, OS_DIR_SEP,
				IOT_TARGET_RELAY, "stderr.log" );
			out_files[1] = os_file_open(log_file,OS_CREATE | OS_WRITE);
		}

		IOT_LOG( iot_lib, IOT_LOG_TRACE,
			"Remote login: host=%s, protocol=%s, debug=%d",
			host_in, protocol_in, debug_mode );

		if ( host_in && *host_in != '\0'
		     && protocol_in && *protocol_in != '\0'
		     && url_in && *url_in != '\0' )
		{
			os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
			const char *ca_bundle = NULL;
			char relay_cmd[ PATH_MAX + 1u ];
			size_t relay_cmd_len = 0u;
			int char_count;
			os_status_t run_status;
			iot_bool_t validate_cert = IOT_FALSE;

			/* obtain path to relay application */
#if defined( __VXWORKS__ )
			os_snprintf( relay_cmd, PATH_MAX, "%s%c",
				device_manager->app_path, OS_DIR_SEP );
			relay_cmd_len = os_strlen( relay_cmd );
#else /* if defined( __VXWORKS__ ) */
			if ( app_path_executable_directory_get(
				relay_cmd, PATH_MAX ) == IOT_STATUS_SUCCESS )
			{
				relay_cmd_len = os_strlen( relay_cmd );
				relay_cmd[ relay_cmd_len++ ] = OS_DIR_SEP;
			}
#endif /* defined( __VXWORKS__ ) */

			/* add name of relay application */
			if ((char_count = os_snprintf( &relay_cmd[relay_cmd_len],
				PATH_MAX - relay_cmd_len,
				IOT_TARGET_RELAY )) > 0)
				relay_cmd_len += (size_t)char_count;

			/* custom certification */
			iot_config_get( iot_lib,
				"ca_bundle_file", IOT_FALSE,
				IOT_TYPE_STRING, &ca_bundle );
			if ( ca_bundle )
			{
				if ((char_count = os_snprintf(
					&relay_cmd[ relay_cmd_len ],
					PATH_MAX - relay_cmd_len,
					" --cert=%s", ca_bundle )) > 0)
				relay_cmd_len += (size_t)char_count;
			}
	
			/* allow insecure connections (private signed certs) */
			iot_config_get( iot_lib,
				"validate_cloud_cert", IOT_FALSE,
				IOT_TYPE_BOOL, &validate_cert );
			if ( validate_cert != IOT_FALSE )
			{
				if ((char_count = os_snprintf(
					&relay_cmd[ relay_cmd_len ],
					PATH_MAX - relay_cmd_len,
					" --insecure" )) > 0)
				relay_cmd_len += (size_t)char_count;
			}

			/* add host and port to connect to */
			if ((char_count = os_snprintf( &relay_cmd[ relay_cmd_len ],
				PATH_MAX - relay_cmd_len,
				" --host=%s -p %d %s",
				host_in, os_atoi(protocol_in), url_in )) > 0)
				relay_cmd_len += (size_t)char_count;

			relay_cmd[relay_cmd_len] = '\0';
			IOT_LOG( iot_lib, IOT_LOG_TRACE,
				"Remote login cmd: %s", relay_cmd );

			args.cmd = relay_cmd;
			args.opts.nonblock.std_out = out_files[0];
			args.opts.nonblock.std_err = out_files[1];

			run_status = os_system_run( &args );
			IOT_LOG( iot_lib, IOT_LOG_TRACE,
				"System Run returned: %d", run_status );
			os_time_sleep( 10, IOT_FALSE );

			/* remote login protocol requires us to return
			 * success, or it will not open the cloud side relay
			 * connection.  So, check for invoked here and return
			 * success */
			result = IOT_STATUS_FAILURE;
			if ( run_status == OS_STATUS_SUCCESS ||
			     run_status == OS_STATUS_INVOKED )
				result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t on_action_remote_login_update( iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;
	if ( request && device_manager )
	{
		char cfg_path[ PATH_MAX + 1u ];
		size_t cfg_path_len;
		os_file_t fd;

		/* get path to configuration file (iot.cfg) */
		result = IOT_STATUS_FAILURE;
		cfg_path_len = iot_directory_name_get( IOT_DIR_CONFIG,
			cfg_path, PATH_MAX );
		os_snprintf( &cfg_path[cfg_path_len], PATH_MAX - cfg_path_len,
			"%c%s", OS_DIR_SEP, IOT_DEFAULT_FILE_DEVICE_MANAGER );

		/* step #1 read from configuration file */
		fd = os_file_open( cfg_path, OS_READ );
		if ( fd )
		{
			const os_uint16_t cfg_file_size =
				(os_uint16_t)os_file_size_handle( fd );
			char *const json_str = os_malloc( cfg_file_size + 1u );
			size_t json_size = 0u;
			if ( json_str )
			{
				json_size = os_file_read( json_str,
					sizeof( char ), cfg_file_size, fd );
				json_str[ json_size ] = '\0';
			}
			os_file_close( fd );

			if ( json_str )
			{
				iot_status_t interim_result;
				iot_json_decoder_t *json_dec;
				iot_json_encoder_t *json_enc;
				const iot_json_item_t *json_root = NULL;
#if defined( IOT_STACK_ONLY )
				char buffer_dec[1024u];
				char buffer_enc[1024u];
				json_dec = iot_json_decode_initialize(
					buffer_dec, sizeof(buffer_dec), 0u );
				json_enc = iot_json_encode_initialize(
					buffer_enc, sizeof(buffer_enc), 0u );
#else /* if defined( IOT_STACK_ONLY ) */
				json_dec = iot_json_decode_initialize(
					NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
				json_enc = iot_json_encode_initialize(
					NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
#endif /* else if defined( IOT_STACK_ONLY ) */
				iot_json_encode_array_start( json_enc, NULL );

				result = iot_json_decode_parse( json_dec,
					json_str, json_size, &json_root,
					NULL, 0u );
				if ( result == IOT_STATUS_SUCCESS )
				{
					const iot_json_item_t *const j_ra_support =
						iot_json_decode_object_find(
							json_dec, json_root,
							"remote_access_support" );
					const iot_json_array_iterator_t * j_itr =
						iot_json_decode_array_iterator(
							json_dec, j_ra_support );
					while ( j_itr )
					{
						const iot_json_item_t *j_ra_obj = NULL;
						const iot_json_item_t *j_ra_value = NULL;
						char *service_name = NULL;
						iot_uint16_t port_num;
						const char *str = NULL;
						size_t str_len = 0u;

						/* get remote access object */
						iot_json_decode_array_iterator_value(
							json_dec, j_ra_support,
							j_itr, &j_ra_obj );

						/* service_name */
						j_ra_value = iot_json_decode_object_find(
							json_dec, j_ra_obj, "port" );
						iot_json_decode_string( json_dec,
							j_ra_value, &str, &str_len );
						if ( str && str_len > 0u )
						{
							service_name = os_malloc( str_len + 1u );
							if ( service_name )
							{
								os_strncpy( service_name, str, str_len );
								service_name[ str_len ] = '\0';
							}
						}

						port_num = check_listening_service( service_name );
						if ( port_num > 0u )
						{
							char port_str[12u];
							char *name = service_name;
							long timeout = 0L;
							iot_json_encode_object_start( json_enc, NULL );

							/* name */
							j_ra_value = iot_json_decode_object_find(
								json_dec, j_ra_obj,
								"name" );
							iot_json_decode_string(
								json_dec, j_ra_value,
								&str, &str_len );

							if ( str && str_len > 0u )
							{
								name = os_malloc( str_len + 1u );
								if ( name )
								{
									os_strncpy( name, str, str_len );;
									name[ str_len ] = '\0';
								}
							}

							iot_json_encode_string(
								json_enc, "name", name );

							if ( name && str && str_len > 0u )
								os_free( name );

							os_snprintf( port_str,
								sizeof(port_str),
								"%d", (unsigned int)port_num );
							port_str[sizeof(port_str) - 1u] = '\0';

							iot_json_encode_string(
								json_enc, "port", port_str );

							/* session timeout */
							j_ra_value = iot_json_decode_object_find(
								json_dec, j_ra_obj,
								"session_timeout" );
							iot_json_decode_integer(
								json_dec, j_ra_value,
								&timeout );
							if ( timeout > 0L )
							{
								char timeout_str[32u];
								os_snprintf( timeout_str,
									sizeof(timeout_str),
									"%ld", timeout );
								timeout_str[sizeof(timeout_str) - 1u] = '\0';
								iot_json_encode_string( json_enc,
									"session_timeout", timeout_str );
							}
							iot_json_encode_object_end( json_enc );
						}

						if ( service_name )
							os_free( service_name );

						j_itr = iot_json_decode_array_iterator_next(
							json_dec, j_ra_support, j_itr );
					}
				}

				iot_json_decode_terminate( json_dec );
				os_free( json_str );

				/* step #2 update remote access attribute */
				iot_json_encode_array_end( json_enc );

				interim_result = iot_attribute_publish_string( iot_lib,
					NULL, NULL, "remote_access_support",
					iot_json_encode_dump( json_enc ) );

				if ( result == IOT_STATUS_SUCCESS )
					result = interim_result;

				iot_json_encode_terminate( json_enc );
			}
		}
	}
	return result;
}

