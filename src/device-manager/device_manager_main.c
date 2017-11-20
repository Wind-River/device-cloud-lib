/**
 * @brief Source file for the device-manager app.
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "device_manager_main.h"

#ifndef _WRS_KERNEL
#	include "device_manager_file.h"
#endif /* ifndef _WRS_KERNEL */

#include "os.h"                       /* for os specific functions */
#include "utilities/app_arg.h"        /* for struct app_arg & functions */
#include "utilities/app_log.h"        /* for app_log function */
#include "utilities/app_path.h"       /* for app_path_which function */

#include "iot_build.h"
#include "iot_json.h"                 /* for json */

#include <stdlib.h>                   /* for EXIT_SUCCESS, EXIT_FAILURE */

/** @brief Name of the Dump Log Files action*/
#define DEVICE_MANAGER_DUMP_LOG_FILES          "Dump Log Files"

/** @brief Name of the decommission device action */
#define DEVICE_MANAGER_DECOMMISSION_DEVICE     "decommission_device"

/** @brief Name of the device shutdown action*/
#define DEVICE_MANAGER_DEVICE_SHUTDOWN         "shutdown_device"
/** @brief Name of the device reboot action*/
#define DEVICE_MANAGER_DEVICE_REBOOT           "reboot_device"
/** @brief Name of the agent reset action*/
#define DEVICE_MANAGER_AGENT_RESET             "reset_agent"

/** @brief Name of the remote login action */
#define DEVICE_MANAGER_REMOTE_LOGIN            "remote-access"
/** @brief Name of "host" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_HOST                "host"
/** @brief Name of "protocol" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_PROTOCOL            "protocol"
/** @brief Name of "url" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_URL                 "url"
/** @brief Name of "debug" parameter for remote login action */
#define REMOTE_LOGIN_PARAM_DEBUG               "debug-mode"

/** @brief Name of action to get a file from the cloud */
#define DEVICE_MANAGER_FILE_CLOUD_DOWNLOAD      "file_download"
/** @brief Name of action to send a file to the cloud */
#define DEVICE_MANAGER_FILE_CLOUD_UPLOAD        "file_upload"
/** @brief Name of the parameter to save file as */
#define DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME "file_name"
/** @brief Name of the parameter for using global file store */
#define DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE "use_global_store"
/** @brief Name of the parameter for file path on device */
#define DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH "file_path"

#if defined( __unix__ ) && !defined( __ANDROID__ )
#	define COMMAND_PREFIX                      "sudo "
#else
#	define COMMAND_PREFIX                      ""
#endif

#ifdef _WIN32
	/** @brief IoT device manager service ID */
#	define IOT_DEVICE_MANAGER_ID         IOT_DEVICE_MANAGER_TARGET
	/** @brief Remote Desktop service ID on Windows */
#	define IOT_REMOTE_DESKTOP_ID  "TermService"
#endif

#if defined( __ANDROID__ )
/** @brief Command and parameters to enable telnetd on the device */
#define ENABLE_TELNETD_LOCALHOST \
	"if [ 0 -eq $( netstat | grep 23 | grep -c LISTEN ) ]; then busybox telnetd -l /system/bin/sh -b 127.0.0.1:23; fi"
#endif /* __ANDROID__ */

/**
 * @brief Structure containing application specific data
 */
#ifndef _WRS_KERNEL
extern struct device_manager_info APP_DATA;
struct device_manager_info APP_DATA;
#else
/** @todo fix after enabling read cofigure file in vxWorks*/
static struct device_manager_info APP_DATA;
#endif

/**
 * @brief Structure containing names of actions
 */
static const char *const ACTION_CFG_NAMES[] ={
	"software_update",
	"file_transfers",
	"decommission_device",
	"restore_factory_images",
	"dump_log_files",
	"shutdown_device",
	"reboot_device",
	"reset_agent",
	"remote_login",
	NULL
};

/**
 * @brief Structure defining information about remote login protocols
 */
struct remote_login_protocol
{
	const char *name;           /**< @brief protocol name */
	iot_uint16_t port;          /**< @brief protocol port */
};


/* function definitions */
/**
 * @brief Enables or disables certain actions
 *
 * @param[in,out]  device_manager_info           application specific data
 * @param[in]      flag                          flag for the action to set
 * @param[in]      value                         value to set
 *
 * @retval IOT_STATUS_SUCCESS                    on success
 * @retval IOT_STATUS_BAD_PARAMETER              bad parameter
 */
static iot_status_t device_manager_action_enable(
	struct device_manager_info *device_manager_info,
	iot_uint16_t flag, iot_bool_t value );

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
#ifndef _WRS_KERNEL
static iot_status_t device_manager_config_read(
	struct device_manager_info *device_manager_info,
	const char *app_path, const char *config_file );
#endif

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

#if defined( __ANDROID__ )
/**
 * @brief Run OS command
 *
 * @param[in]      command                       command
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
#endif /* __ANDROID__ */

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

#if defined( __ANDROID__ )
/**
 * @brief Callback function to return the agent decommission
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_decommission(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to return the agent reboot
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_reboot(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to return the agent reset
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_reset(
	iot_action_request_t* request,
	void *user_data );

/**
 * @brief Callback function to return the agent shutdown
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_agent_shutdown(
	iot_action_request_t* request,
	void *user_data );
#endif /* __ANDROID__ */

#ifndef _WRS_KERNEL
/**
 * @brief Callback function to return the remote login
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_remote_login(
		iot_action_request_t* request,
		void *user_data );
#endif /* !_WRS_KERNEL */


/* function implementations */
iot_status_t device_manager_action_enable(
	struct device_manager_info *device_manager_info,
	iot_uint16_t flag, iot_bool_t value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager_info )
	{
		if ( value )
			device_manager_info->enabled_actions |= flag;
		else
			device_manager_info->enabled_actions &= ~flag;
	}
	return result;
}

#if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 )
iot_status_t device_manager_actions_deregister(
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{

#ifndef _WRS_KERNEL
		iot_action_t *const dump_log_files = device_manager->dump_log_files;
		iot_action_t *const agent_reset = device_manager->agent_reset;
		iot_action_t *const decommission_device = device_manager->decommission_device;
		iot_action_t *const device_shutdown = device_manager->device_shutdown;
		iot_action_t *const remote_login = device_manager->remote_login;
		iot_action_t *file_upload = NULL;
		iot_action_t *file_download = device_manager->file_download;
#endif /* ifndef _WRS_KERNEL */
		iot_action_t *const device_reboot = device_manager->device_reboot;

#if !defined( WIN32 ) && !defined( _WRS_KERNEL )
		iot_action_t *const restore_factory_images = device_manager->restore_factory_images;

		/* restore_factory_images */
		if ( restore_factory_images )
		{
			iot_action_deregister( restore_factory_images, NULL, 0u );
			iot_action_free( restore_factory_images, 0u );
			device_manager->restore_factory_images = NULL;
		}
#endif /* if !defined( WIN32 ) && !defined( _WRS_KERNEL ) */

#ifndef _WRS_KERNEL
		/* device_shutdown */
		if ( device_shutdown )
		{
			iot_action_deregister( device_shutdown, NULL, 0u );
			iot_action_free( device_shutdown, 0u );
			device_manager->device_shutdown = NULL;
		}
#endif /* ifndef _WRS_KERNEL */

		/* device_reboot */
		if ( device_reboot )
		{
			iot_action_deregister( device_reboot, NULL, 0u );
			iot_action_free( device_reboot, 0u );
			device_manager->device_reboot = NULL;
		}

#ifndef _WRS_KERNEL
		/* decommission_device */
		if ( decommission_device )
		{
			iot_action_deregister( decommission_device, NULL, 0u );
			iot_action_free( decommission_device, 0u );
			device_manager->decommission_device = NULL;
		}

		/* agent_reset */
		if ( agent_reset )
		{
			iot_action_deregister( agent_reset, NULL, 0u );
			iot_action_free( agent_reset, 0u );
			device_manager->agent_reset = NULL;
		}

		/* dump_log_files */
		if ( dump_log_files )
		{
			iot_action_deregister( dump_log_files, NULL, 0u );
			iot_action_free( dump_log_files, 0u );
			device_manager->dump_log_files = NULL;
		}

		/* remote_login */
		if ( remote_login )
		{
			iot_action_deregister( remote_login, NULL, 0u );
			iot_action_free( remote_login, 0u );
			device_manager->remote_login = NULL;
		}

		/* manifest(ota) */
		device_manager_ota_deregister( device_manager );

#ifndef NO_FILEIO_SUPPORT
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
#endif /* ifndef NO_FILEIO_SUPPORT */
#endif /* ifndef _WRS_KERNEL */

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
		iot_t *const iot_lib = device_manager->iot_lib;
		iot_action_t *device_reboot = NULL;
		char command_path[ PATH_MAX + 1u ];
#ifndef _WRS_KERNEL
		iot_action_t *dump_log_files = NULL;
		iot_action_t *agent_reset = NULL;
		iot_action_t *decommission_device = NULL;
		iot_action_t *device_shutdown = NULL;
		iot_action_t *remote_login = NULL;
		iot_action_t *file_upload = NULL;
		iot_action_t *file_download = NULL;
#ifndef _WIN32
		/*iot_action_t *restore_factory_images = NULL;*/
		/*FIXME*/
		/* restore factory image */
		/*if ( device_manager->enabled_actions &*/
		/*DEVICE_MANAGER_ENABLE_RESTORE_FACTORY_IMAGES )*/
		/*{*/
		/*restore_factory_images = iot_action_allocate( iot_lib,*/
		/*DEVICE_MANAGER_RESTORE_FACTORY_IMAGES );*/
		/*iot_action_flags_set( restore_factory_images,*/
		/*IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );*/
		/*os_make_path( command_path, PATH_MAX,*/
		/*device_manager->app_path,*/
		/*DEVICE_MANAGER_RESTORE_FACTORY_IMAGES_SCRIPT, NULL );*/
		/*result = iot_action_register_command( restore_factory_images,*/
		/*command_path, NULL, 0u );*/
		/*if ( result == IOT_STATUS_SUCCESS )*/
		/*device_manager->restore_factory_images =*/
		/*restore_factory_images;*/
		/*else*/
		/*IOT_LOG( iot_lib, IOT_LOG_ERROR,*/
		/*"Failed to register %s action. Reason: %s",*/
		/*DEVICE_MANAGER_RESTORE_FACTORY_IMAGES,*/
		/*iot_error(result) );*/
		/*}*/
#endif /* ifndef _WIN32 */


		/* file transfer */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_FILE_TRANSFERS )
		{
			/* File Download */
			file_download = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_FILE_CLOUD_DOWNLOAD );

			/* global store is optional */
			iot_action_parameter_add( file_download,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
				IOT_PARAMETER_IN, IOT_TYPE_BOOL, 0u );

			/* file name */
			iot_action_parameter_add( file_download,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME,
				IOT_PARAMETER_IN_REQUIRED, IOT_TYPE_STRING, 0u );

			/* file path */
			iot_action_parameter_add( file_download,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
				IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

			result = iot_action_register_callback( file_download,
				&device_manager_file_download, device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action",
					DEVICE_MANAGER_FILE_CLOUD_DOWNLOAD );
			/* File Upload */
			file_upload = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_FILE_CLOUD_UPLOAD);

			/* global store is optional */
			iot_action_parameter_add( file_upload,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
				IOT_PARAMETER_IN, IOT_TYPE_BOOL, 0u );

			/* file name  */
			iot_action_parameter_add( file_upload,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME,
				IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

			/* file path */
			iot_action_parameter_add( file_upload,
				DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
				IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

			result = iot_action_register_callback( file_upload,
				&device_manager_file_upload, device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action",
					DEVICE_MANAGER_FILE_CLOUD_UPLOAD);
		}

		/* device shutdown */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_DEVICE_SHUTDOWN )
		{
			device_shutdown = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_DEVICE_SHUTDOWN );
			iot_action_flags_set( device_shutdown,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
#if defined( __ANDROID__ )
			result = iot_action_register_callback(
				device_shutdown, &on_action_agent_shutdown,
				(void*)device_manager, NULL, 0u );
#else
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --shutdown" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( device_shutdown,
					command_path, NULL, 0u );
#endif
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->device_shutdown = device_shutdown;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_DEVICE_SHUTDOWN,
					iot_error(result) );
		}

		/* decommission device */
		/*FIXME*/
		if ( device_manager->enabled_actions &
				DEVICE_MANAGER_ENABLE_DECOMMISSION_DEVICE )
		{
			decommission_device = iot_action_allocate( iot_lib,
					DEVICE_MANAGER_DECOMMISSION_DEVICE );
			iot_action_flags_set( decommission_device,
					IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
#if defined( __ANDROID__ )
			result = iot_action_register_callback(
					decommission_device, &on_action_agent_decommission,
					(void*)device_manager, NULL, 0u );
#else
			result = device_manager_make_control_command( command_path,
					PATH_MAX, device_manager, " --decommission" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( decommission_device,
						command_path, NULL, 0u );
#endif
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->decommission_device =
					decommission_device;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
						"Failed to register %s action. Reason: %s",
						DEVICE_MANAGER_DECOMMISSION_DEVICE,
						iot_error(result) );
		}

		/* agent reset */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_AGENT_RESET )
		{
			agent_reset = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_AGENT_RESET );
			iot_action_flags_set( agent_reset,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
#if defined( __ANDROID__ )
			result = iot_action_register_callback(
				agent_reset, &on_action_agent_reset,
				(void*)device_manager, NULL, 0u );
#else
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --restart" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( agent_reset,
					command_path, NULL, 0u );
#endif
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->agent_reset = agent_reset;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_AGENT_RESET,
					iot_error(result) );
		}

		/* dump log files */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_DUMP_LOG_FILES )
		{
			dump_log_files = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_DUMP_LOG_FILES );
			iot_action_flags_set( dump_log_files,
				IOT_ACTION_EXCLUSIVE_APP );
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --dump" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( dump_log_files,
					command_path, NULL, 0u );
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->dump_log_files = dump_log_files;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_DUMP_LOG_FILES,
					iot_error(result) );
		}

#ifndef NO_FILEIO_SUPPORT
		/* file-io */
		/*FIXME*/
		/*if ( device_manager->enabled_actions &*/
		/*DEVICE_MANAGER_ENABLE_FILE_TRANSFERS )*/
		/*if ( device_manager_file_register( device_manager ) != IOT_STATUS_SUCCESS )*/
		/*IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",*/
		/*"Failed to register file-io actions" );*/
#endif /* ifndef NO_FILEIO_SUPPORT */

		/* manifest (ota) */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_SOFTWARE_UPDATE )
			if ( device_manager_ota_register( device_manager ) != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",
					"Failed to register software update actions" );

		/* remote_login */
		if ( ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_REMOTE_LOGIN ))
		{

			remote_login = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_REMOTE_LOGIN );

			/* param to call set on  */
			iot_action_parameter_add( remote_login,
				REMOTE_LOGIN_PARAM_HOST,
				IOT_PARAMETER_IN,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( remote_login,
				REMOTE_LOGIN_PARAM_PROTOCOL,
				IOT_PARAMETER_IN_REQUIRED,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( remote_login,
				REMOTE_LOGIN_PARAM_URL,
				IOT_PARAMETER_IN_REQUIRED,
				IOT_TYPE_STRING, 0u );
			iot_action_parameter_add( remote_login,
				REMOTE_LOGIN_PARAM_DEBUG,
				IOT_PARAMETER_IN,
				IOT_TYPE_BOOL, 0u );

			result = iot_action_register_callback(
				remote_login,
				&on_action_remote_login,
					(void*)device_manager, NULL, 0u );
			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action. Reason: %s",
					DEVICE_MANAGER_REMOTE_LOGIN,
					iot_error(result) );
			}
		}

		/* device reboot */
		if ( device_manager->enabled_actions &
			DEVICE_MANAGER_ENABLE_DEVICE_REBOOT )
#endif
		{
			device_reboot = iot_action_allocate( iot_lib,
				DEVICE_MANAGER_DEVICE_REBOOT );
			iot_action_flags_set( device_reboot,
				IOT_ACTION_NO_RETURN | IOT_ACTION_EXCLUSIVE_DEVICE );
#if defined( __ANDROID__ )
			result = iot_action_register_callback(
				device_reboot, &on_action_agent_reboot,
				(void*)device_manager, NULL, 0u );
#else
#	ifndef _WRS_KERNEL
			result = device_manager_make_control_command( command_path,
				PATH_MAX, device_manager, " --reboot" );
			if ( result == IOT_STATUS_SUCCESS )
				result = iot_action_register_command( device_reboot,
					command_path, NULL, 0u );
#	else
			/**
			 * @todo This part should be rewritten in the future when iot-control becomes
			 * available on VxWorks.  Currently, we just call a "reboot" command to
			 * stimulate a reboot
			 */
			result = iot_action_register_command( device_reboot,
				"reboot", NULL, 0u );
#	endif
#endif
			if ( result == IOT_STATUS_SUCCESS )
				device_manager->device_reboot = device_reboot;
			else
				IOT_LOG( iot_lib, IOT_LOG_ERROR,
					"Failed to register %s action",
					DEVICE_MANAGER_DEVICE_REBOOT );
		}
	}
	return result;
}

#ifndef _WRS_KERNEL
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
		os_env_expand( device_manager_info->runtime_dir, PATH_MAX );
		device_manager_info->runtime_dir[PATH_MAX] = '\0';
		IOT_LOG( NULL, IOT_LOG_INFO,
			"  * Setting default runtime dir to %s",
			device_manager_info->runtime_dir );

		/* standard actions */
		device_manager_info->enabled_actions = 0u;

		/* compile time definition */
		if ( IOT_DEFAULT_ENABLE_SOFTWARE_UPDATE )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_SOFTWARE_UPDATE, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_FILE_TRANSFERS )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_FILE_TRANSFERS, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_DECOMMISSION_DEVICE )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DECOMMISSION_DEVICE, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_DUMP_LOG_FILES )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DUMP_LOG_FILES, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_DEVICE_SHUTDOWN )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DEVICE_SHUTDOWN, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_AGENT_RESET )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_AGENT_RESET, IOT_TRUE );
#ifndef _WIN32
		if ( IOT_DEFAULT_ENABLE_RESTORE_FACTORY_IMAGES )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_RESTORE_FACTORY_IMAGES, IOT_TRUE );
#endif /* ifndef _WIN32 */

		if ( IOT_DEFAULT_ENABLE_DEVICE_REBOOT )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_DEVICE_REBOOT, IOT_TRUE );
		if ( IOT_DEFAULT_ENABLE_REMOTE_LOGIN )
			device_manager_action_enable( device_manager_info,
				DEVICE_MANAGER_ENABLE_REMOTE_LOGIN, IOT_TRUE );

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
					json_size = os_file_read( json_string, 1, json_size, fd );
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

				json = iot_json_decode_initialize( NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
				if ( json && iot_json_decode_parse( json,
							json_string,
							json_size, &json_root,
						err_msg, 1024u ) == IOT_STATUS_SUCCESS )
				{
					int i = 0;
					unsigned int action_mask = 0u;
					const iot_json_item_t *j_action_top;
					const iot_json_item_t *const j_actions_enabled =
						iot_json_decode_object_find(
							json, json_root,
							"actions_enabled" );

					/* handle all the boolean default actions */
					IOT_LOG( NULL, IOT_LOG_INFO, "%s",
						"Default Configuration:" );
					for ( i = 0; j_actions_enabled &&
						ACTION_CFG_NAMES[i]; ++i )
					{
						iot_bool_t enabled = IOT_FALSE;
						const iot_json_item_t *const j_action =
							iot_json_decode_object_find(
							json, j_actions_enabled,
							ACTION_CFG_NAMES[i] );
						iot_json_decode_bool( json,
							j_action, &enabled );
						if ( enabled == IOT_TRUE )
						{
							IOT_LOG( NULL, IOT_LOG_INFO,
								"  * %s is enabled",
								ACTION_CFG_NAMES[i] );
							action_mask |= (1<<i);
						}
						else
							IOT_LOG( NULL, IOT_LOG_INFO,
								"  * %s is disabled",
								ACTION_CFG_NAMES[i] );
					}
					IOT_LOG( NULL, IOT_LOG_TRACE,
						"  * actions enabled mask = 0x%x", action_mask );
					device_manager_info->enabled_actions = (iot_uint16_t)action_mask;

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

						os_env_expand( device_manager_info->runtime_dir, PATH_MAX );
						device_manager_info->runtime_dir[ temp_len ] = '\0';
						IOT_LOG( NULL, IOT_LOG_INFO,
							"  * runtime dir = %s", device_manager_info->runtime_dir );
						if ( os_directory_create(
							device_manager_info->runtime_dir,
							DIRECTORY_CREATE_MAX_TIMEOUT )
						!= OS_STATUS_SUCCESS )
							IOT_LOG( NULL, IOT_LOG_INFO,
								"Failed to create %s ", device_manager_info->runtime_dir );
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
			}
		}
		os_file_close( fd );
	}
	return result;
}
#endif

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
			"param %s = %s result=%d\n",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME, file_name,
			(int)result);

		/* file_path */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
			IOT_FALSE, IOT_TYPE_STRING, &file_path );
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %s result=%d\n",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH, file_path,
			(int)result);

		/* use global store */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
			IOT_FALSE, IOT_TYPE_BOOL, &use_global_store);
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %d result=%d\n",
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
			"param %s = %s result=%d\n",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_NAME, file_name,
			(int)result);

		/* file_path */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH,
			IOT_FALSE, IOT_TYPE_STRING, &file_path );
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %s result=%d\n",
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_FILE_PATH, file_path,
			(int)result);

		/* use global store */
		result = iot_action_parameter_get( request,
			DEVICE_MANAGER_FILE_CLOUD_PARAMETER_USE_GLOBAL_STORE,
			IOT_FALSE, IOT_TYPE_BOOL, &use_global_store);
		IOT_LOG( dm->iot_lib, IOT_LOG_TRACE,
			"param %s = %d result=%d\n",
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
		"File Download Status: %s (completed: %s [%f %%])\n",
		iot_error( status ),
		( complete == IOT_FALSE ? "no" : "yes" ),
		(double)percent );
}

iot_status_t device_manager_initialize( const char *app_path,
	struct device_manager_info *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_t *iot_lib = NULL;
		char *p_path = NULL;
#ifndef NO_FILEIO_SUPPORT
		struct device_manager_file_io_info *file_io = &device_manager->file_io_info;
		os_thread_mutex_t *file_transfer_lock = &file_io->file_transfer_mutex;
#endif /* ifndef NO_FILEIO_SUPPORT */

		iot_lib = iot_initialize( "device-manager", NULL, 0u );
		if ( iot_lib == NULL )
		{
			os_fprintf( OS_STDERR,
				"Error: %s", "Failed to initialize IOT library" );
			return IOT_STATUS_FAILURE;
		}

#if defined( __ANDROID__ )
		/* start telnetd bind to localhost only */
		device_manager_run_os_command( ENABLE_TELNETD_LOCALHOST, IOT_TRUE );
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
#ifndef __ANDROID__
			os_strncpy( device_manager->app_path, ".", PATH_MAX );
#else
			os_strncpy( device_manager->app_path, "/system/bin", PATH_MAX );
#endif /* __ANDROID__ */
		}

		result = iot_connect( iot_lib, 0u );
		if ( result == IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Connected" );
		else
		{
			IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Failed to connect\n" );
			result = IOT_STATUS_FAILURE;
		}

		if ( result == IOT_STATUS_SUCCESS )
		{
			device_manager->iot_lib = iot_lib;

#if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
			if ( os_thread_mutex_create( file_transfer_lock ) != OS_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s", "Failed to create file_transfer_mutex" );
#endif /* if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */
			if ( device_manager_actions_register( device_manager ) != IOT_STATUS_SUCCESS )
				IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",	"Failed to register device-manager actions" );
#ifndef NO_FILEIO_SUPPORT
#	ifdef __ANDROID__
			/* check pending file transfer status
			 * after run time dir is available */

			/*FIXME*/
			/* to-be-do
			device_manager_file_initialize(
				device_manager, IOT_FALSE );
			*/
#	endif /* __ANDROID__ */
#endif /* ifndef NO_FILEIO_SUPPORT */
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
		{ 'c', "configure", 0, "file", &config_file,
			"configuration file", 0u },
		{ 'h', "help", 0, NULL, NULL, "display help menu", 0u },
		{ 's', "service", 0, NULL, NULL, "run as a service", 0u },
		{ 0, NULL, 0, NULL, NULL, NULL, 0u }
	};

	IOT_LOG( NULL, IOT_LOG_INFO, "%s", "Starting Device Manager" );
	result = app_arg_parse(args, argc, argv, NULL);
	if (result == EXIT_FAILURE || app_arg_count(args, 'h', NULL))
		app_arg_usage(args, 36u, argv[0],
		IOT_DEVICE_MANAGER_TARGET, NULL, NULL);
	else if (result == EXIT_SUCCESS)
	{
		os_memzero( &APP_DATA, sizeof(struct device_manager_info) );

/** @todo vxWorks checking iot.cfg will be implemented later */
#ifndef _WRS_KERNEL
		device_manager_config_read( &APP_DATA, argv[0], config_file );
#endif
		if ( app_arg_count( args, 's', "service" ) > 0u )
		{
			const char *remove_args[] = { "-s", "--service" };

/* android does not have an hdc supported service handler */
#ifdef __ANDROID__
			result = EXIT_SUCCESS;
#else
			result = os_service_run(
				IOT_DEVICE_MANAGER_TARGET, device_manager_main,
				argc, argv,
				sizeof( remove_args ) / sizeof( const char* ),
				remove_args, &device_manager_sig_handler,
				APP_DATA.runtime_dir );
#endif
		}
		else
		{

			if ( device_manager_initialize(argv[0], &APP_DATA )
				== IOT_STATUS_SUCCESS )
			{
				/* setup device manager attributes */
				os_system_info_t os;
				/*os_adapters_t *adapters = NULL;*/

				iot_attribute_publish_string(
					APP_DATA.iot_lib, NULL, NULL,
					IOT_PRODUCT_SHORT "_version",
					iot_version_str() );

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
					/* FIXME: disabled for now
					 * since it is not in the thing def and auto is false */
					/*iot_attribute_publish_string(*/
					/*APP_DATA.iot_lib,*/
					/*NULL, NULL,*/
					/*"os_variant",*/
					/*os.system_release );*/
					iot_attribute_publish_string(
						APP_DATA.iot_lib,
						NULL, NULL,
						"platform",
						os.system_platform );
				}

				/* FIXME: github issue #27 */
				/* obtain MAC addresses */
				/*if ( os_adapters_obtain( &adapters ) == OS_STATUS_SUCCESS )*/
				/*{*/
				/*#define IOT_MAC_ADDRESS_LEN 17u*/
				/*#define IOT_MAC_SEPERATOR   " "*/
				/*const size_t sep_len = os_strlen( IOT_MAC_SEPERATOR );*/
				/*char *mac_addr = NULL;*/
				/*size_t mac_pos = 0u;*/
				/*char *mem_ptr = NULL;*/
				/*do*/
				/*{*/
				/*mem_ptr = os_realloc( mac_addr,*/
				/*mac_pos + IOT_MAC_ADDRESS_LEN + sep_len );*/
				/*if ( mem_ptr )*/
				/*{*/
				/**//* not first item, add a seperator */
				/*mac_addr = mem_ptr;*/

				/*os_strncpy( &mac_addr[mac_pos],*/
				/*"00:00:00:00:00:00",*/
				/*IOT_MAC_ADDRESS_LEN );*/
				/*if ( os_adapters_mac(*/
				/*adapters,*/
				/*&mac_addr[mac_pos],*/
				/*IOT_MAC_ADDRESS_LEN + 1u ) == OS_STATUS_SUCCESS )*/
				/*{*/
				/*mac_pos += IOT_MAC_ADDRESS_LEN;*/
				/*os_strncpy( &mac_addr[mac_pos],*/
				/*IOT_MAC_SEPERATOR,*/
				/*sep_len );*/
				/*mac_pos += sep_len;*/
				/*}*/
				/*}*/
				/*} while ( mem_ptr && os_adapters_next( adapters ) == OS_STATUS_SUCCESS );*/

				/**//* null-terminate string */
				/*if ( mac_addr )*/
				/*{*/
				/*if ( mac_pos > 0u )*/
				/*mac_pos -= sep_len;*/
				/*mac_addr[mac_pos] = '\0';*/

				/*iot_attribute_publish_string(*/
				/*APP_DATA.iot_lib,*/
				/*NULL, NULL,*/
				/*"mac_address", mac_addr );*/

				/*os_free( mac_addr );*/
				/*}*/
				/*os_adapters_release( adapters );*/
				/*}*/

				/*os_terminate_handler(device_manager_sig_handler);*/

				IOT_LOG( APP_DATA.iot_lib, IOT_LOG_INFO, "%s",
					"Ready for some actions..." );

				while ( APP_DATA.iot_lib &&
					APP_DATA.iot_lib->to_quit == IOT_FALSE )
				{
					os_time_sleep( POLL_INTERVAL_MSEC,
						IOT_FALSE );
				}
				IOT_LOG(APP_DATA.iot_lib, IOT_LOG_INFO, "%s",
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

		result = IOT_STATUS_SUCCESS;
		os_strncpy( full_path, COMMAND_PREFIX, max_len );
		full_path[ max_len - 1u ] = '\0';
		offset = os_strlen( full_path );
		ptr = full_path + offset;
#if defined( _WIN32 )
		if ( offset < max_len - 1u )
		{
			*ptr = '"';
			++ptr;
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
				*ptr = '"';
				++ptr;
				*ptr = '\0';
				++offset;
			}
			else
				result = IOT_STATUS_FULL;
#endif /* defined( _WIN32 ) */
			if ( *options != ' ' && offset < max_len - 1u )
			{
				*ptr = ' ';
				++ptr;
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

#if defined( __ANDROID__ )
iot_status_t device_manager_run_os_command( const char *cmd,
	iot_bool_t blocking_action )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( cmd )
	{
		char buf[1] = "\0";
		char *out_buf[2u] = { buf, buf };
		size_t out_len[2u] = { 1u, 1u };
		int retval = -1;

		if ( blocking_action == IOT_FALSE )
		{
			size_t i;
			for ( i = 0u; i < 2u; ++i )
			{
				out_buf[i] = NULL;
				out_len[i] = 0u;
			}
		}
		if ( os_system_run_wait( cmd, &retval, out_buf,
			out_len, 0u ) == IOT_STATUS_SUCCESS &&
			     retval >= 0 )
			result = IOT_STATUS_SUCCESS;
		else
		{
			result = IOT_STATUS_FAILURE;
			IOT_LOG( APP_DATA.iot_lib, IOT_LOG_INFO,
				"OS cmd (%s): return value %d",
				cmd, retval );
		}
	}
	return result;
}
#endif /* __ANDROID__ */

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
#if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
		os_thread_mutex_t *file_transfer_lock = &device_manager->file_io_info.file_transfer_mutex;
#endif /* if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */

#if ( IOT_DEFAULT_ENABLE_PERSISTENT_ACTIONS == 0 )
		device_manager_actions_deregister( device_manager );
#endif

#if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT )
		os_thread_mutex_destroy( file_transfer_lock );
#endif /* if !defined( NO_THREAD_SUPPORT ) && !defined( NO_FILEIO_SUPPORT ) */

		iot_disconnect( iot_lib, 0u );
		iot_terminate( iot_lib, 0u );

#ifndef NO_FILEIO_SUPPORT
		/* must be done last */
		/*FIXME*/
		/*device_manager_file_terminate( device_manager );*/
#endif /* ifndef NO_FILEIO_SUPPORT */
	}
	return result;
}

#if defined( __ANDROID__ )
iot_status_t on_action_agent_decommission(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd_decommission[] = "iot-control --decommission";
		char cmd_reboot[] = "iot-control --reboot --delay 5000 &";
		result = device_manager_run_os_command(
			cmd_decommission, IOT_TRUE );
		/*if ( result == IOT_STATUS_SUCCESS )*/
		/*result = device_manager_run_os_command(*/
		/*cmd_reboot, IOT_FALSE );*/
	}
	return result;
}

iot_status_t on_action_agent_reboot(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd[] = "iot-control --reboot --delay 5000 &";
		result = device_manager_run_os_command( cmd, IOT_FALSE );
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
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd[] = "iot-control --restart --delay 5000 &";
		result = device_manager_run_os_command( cmd, IOT_FALSE );
	}
	return result;
}

iot_status_t on_action_agent_shutdown(
	iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

	if ( device_manager && request )
	{
		char cmd[] = "iot-control --shutdown --delay 5000 &";
		result = device_manager_run_os_command( cmd, IOT_FALSE );
	}
	return result;
}
#endif /* __ANDROID__ */

#ifndef _WRS_KERNEL
iot_status_t on_action_remote_login( iot_action_request_t* request,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct device_manager_info * const device_manager =
		(struct device_manager_info *) user_data;
	iot_t *const iot_lib = device_manager->iot_lib;

#	define RELAY_CMD_TEMPLATE "%s --host=%s --insecure -p %d %s "

	if ( device_manager && request )
	{
		char relay_cmd[ PATH_MAX + 1u ];
		size_t relay_cmd_len = 0u;
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
			os_snprintf(log_file, PATH_MAX, "%s%c%s",
				device_manager->runtime_dir, OS_DIR_SEP, "iot-relay-stdout.log");
			out_files[0] = os_file_open(log_file,OS_CREATE | OS_WRITE);

			os_snprintf(log_file, PATH_MAX, "%s%c%s", device_manager->runtime_dir,
				OS_DIR_SEP, "iot-relay-stderr.log");
			out_files[1] = os_file_open(log_file,OS_CREATE | OS_WRITE);
		}

		IOT_LOG( iot_lib, IOT_LOG_TRACE,
			"Remote login params host=%s, protocol=%s, url=%s, "
			"debug-mode=%d\n",
			host_in, protocol_in, url_in, debug_mode );

		if ( host_in && *host_in != '\0'
		     && protocol_in && *protocol_in != '\0'
		     && url_in && *url_in != '\0' )
		{
			os_status_t run_status;

			os_snprintf( &relay_cmd[ relay_cmd_len ],
				PATH_MAX,
				RELAY_CMD_TEMPLATE,
				IOT_TARGET_RELAY,
				host_in, os_atoi(protocol_in), url_in );

			IOT_LOG( iot_lib, IOT_LOG_TRACE, "Remote login cmd:\n%s\n",
				relay_cmd );

			run_status = os_system_run( relay_cmd, NULL, out_files);
			IOT_LOG( iot_lib, IOT_LOG_TRACE,
				"System Run returned %d\n", result );
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
#endif /* !_WRS_KERNEL */

