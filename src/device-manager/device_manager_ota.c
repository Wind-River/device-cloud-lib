/**
 * @file
 * @brief Source file for service handling in the IoT control application
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#include "device_manager_ota.h"
#include "utilities/app_path.h"
#include "device_manager_main.h"
#include "device_manager_file.h"
#include "api/shared/iot_base64.h"
#include "api/shared/iot_types.h"     /* for IOT_ACTION_NO_RETURN, IOT_RUNTIME_DIR */
#include "iot_build.h"
#include "iot.h"

#include <archive.h>                   /* for archiving functions */
#include <archive_entry.h>             /* for adding files to an archive */

/** @brief Name of the parameter to software update action */
#define DEVICE_MANAGER_OTA_PKG_PARAM   "package"
/** @brief Name of the parameter for download timeout */
#define DEVICE_MANAGER_OTA_TIMEOUT   "ota_timeout"
/** @brief Name of the manifest action */
#define DEVICE_MANAGER_UPDATE_CMD      "software_update"


/**
 * @brief Callback function to handle ota action
 *
 * @param[in,out]  request             request from the cloud
 * @param[in,out]  user_data           pointer to user defined data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    user data provided is invalid
 * @retval IOT_STATUS_SUCCESS          file download started
 * @retval IOT_STATUS_FAILURE          failed to start file download
 */
static iot_status_t device_manager_ota( iot_action_request_t *request,
					 void *user_data );
/**
 * @brief Function to pull data from a read archive and write it to a write handle
 *
 * @param[in]      ar              structure containing source archive data
 * @param[in]      aw              structure containing a handle to write
 *
 * @retval archive error code      on failure
 * @retval ARCHIVE_OK              on success
 */
int device_manager_ota_copy_data(struct archive *ar, struct archive *aw);
/**
 * @brief Execute ota install
 *
 * @param[in]  device_manager_info  pointer to device manager data structure
 * @param[in]  package_path         pointer to ota package directory
 * @param[in]  file_name            pointer to ota package name
 *
 * @retval IOT_STATUS_BAD_PARAMETER    on failure
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
 iot_status_t device_manager_ota_install_execute(
	struct device_manager_info *device_manager_info,
	const char *package_path, const char* file_name );
/**
 * @brief  parameter adjustment
 *
 * @param[in,out]  command_param       param to be adjusted
 * @param[in]      word                the word to be deleted
 *
 * @return         the length of the input param after adjustment
 */
/*FIXME*/
/*static size_t device_manager_software_update_del_characters(*/
/*char *command_param, const char *word );*/
/**
 * @brief  Extracts an OTA package
 *
 * @param[in,out]  iot_lib             library handle
 * @param[in]      package_path        path to the package
 * @param[in]      file_name           extraction file name
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to function
 * @retval IOT_STATUS_FAILURE          system failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t device_manager_ota_extract_package(
	iot_t *iot_lib, const char *package_path, const char *file_name );
/**
 * @brief Function to extract ota package
 *
 * @param[in,out]  iot_lib             library handle
 * @param[in]      sw_update_package        ota package file
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to function
 * @retval IOT_STATUS_FAILURE          system failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t device_manager_ota_extract_package_perform(
	iot_t *iot_lib, const char * sw_update_package );

iot_status_t device_manager_ota_deregister(
	struct device_manager_info  *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_action_t *software_update = device_manager->software_update;
		iot_t *const iot_lib = device_manager->iot_lib;

		/* manifest(ota) action */
		result = iot_action_deregister( software_update, NULL, 0u );
		if ( result == IOT_STATUS_SUCCESS )
		{
			iot_action_free( software_update, 0u );
			device_manager->software_update = NULL;
		}
		else
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to deregister action %s",
				"manifest(ota)" );

		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t device_manager_ota_register(
	struct device_manager_info  *device_manager )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager )
	{
		iot_t *const iot_lib = device_manager->iot_lib;
		iot_action_t *software_update = NULL;

		/* ota will take one parameter required and one
		 * optional TBD */
		software_update = iot_action_allocate( iot_lib,
			DEVICE_MANAGER_UPDATE_CMD );
		iot_action_parameter_add( software_update,
			DEVICE_MANAGER_OTA_PKG_PARAM,
			IOT_PARAMETER_IN_REQUIRED, IOT_TYPE_STRING, 0u );

		/* this parameter is not used by the c lib */
		iot_action_parameter_add( software_update,
			DEVICE_MANAGER_OTA_TIMEOUT,
			IOT_PARAMETER_IN, IOT_TYPE_INT64, 0u );

		/*FIXME: this is not working yet*/
		/*iot_action_flags_set( software_update,*/
		/*IOT_ACTION_EXCLUSIVE_DEVICE );*/
		result = iot_action_register_callback( software_update,
			&device_manager_ota,device_manager, NULL, 0u );
		if ( result == IOT_STATUS_SUCCESS )
		{
			device_manager->software_update = software_update;
			IOT_LOG( iot_lib, IOT_LOG_DEBUG,
			"Registered action: %s", DEVICE_MANAGER_UPDATE_CMD );
		}
		else
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
			"Failed to register action: %s; reason: %s",
			DEVICE_MANAGER_UPDATE_CMD, iot_error( result ) );

		result = IOT_STATUS_SUCCESS;
	}
	return result;
}


/**
 * @brief Callback function to return the ota progress 
 *
 * @param[in]      progress            progress structure
 * @param[in]      user_data           User data
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static void device_manager_ota_progress(
		const iot_file_progress_t *progress,
		void *user_data)
{
	iot_file_progress_t *ctx = (iot_file_progress_t *)user_data;
	printf("%s status %d completed %d\n", __func__,progress->status, (int) progress->completed);
	if ( progress->completed == IOT_TRUE)
	{
		/* Optional: check the status for these in the calling function */
		ctx->completed = progress->completed;
		ctx->status = progress->status;
	}

}

/**
 * @brief Callback function to return the remote login
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */

iot_status_t device_manager_ota( iot_action_request_t *request, void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( request && user_data )
	{
		struct device_manager_info * device_manager_info =
			(struct device_manager_info * )user_data;
        	const char *file_to_download = NULL;
		iot_t *const iot_lib = device_manager_info->iot_lib;

		/* get the parameter */
		result = iot_action_parameter_get( request,
						DEVICE_MANAGER_OTA_PKG_PARAM,
						IOT_FALSE, IOT_TYPE_STRING,
						&file_to_download);
		if ( result != IOT_STATUS_SUCCESS )
		{
			result = IOT_STATUS_BAD_PARAMETER;
			printf( "Failed to get value for parameter: %s\n",
				DEVICE_MANAGER_OTA_PKG_PARAM);
		}
		else
		{
			char sw_update_dir[ PATH_MAX + 1u ];
			char local_archive_path[ PATH_MAX + 1u ];
			char sw_update_log[  PATH_MAX + 1u ];
			char runtime_dir[ PATH_MAX + 1u];

			printf( "Value for parameter: %s = %s\n",
				DEVICE_MANAGER_OTA_PKG_PARAM,
				file_to_download);

			iot_directory_name_get( IOT_DIR_RUNTIME, runtime_dir,
				PATH_MAX );

			/* set the software update and package download directories */
			result = IOT_STATUS_FAILURE;
			if ( OS_STATUS_SUCCESS == os_make_path( sw_update_dir,
				PATH_MAX, runtime_dir, "update", NULL ) )
			{
				/*
				 * Create update directory before starting.
				 * Clean old update directory if it exists
				 */
				if ( os_directory_exists( sw_update_dir ) )
					os_directory_delete( sw_update_dir,
						NULL, IOT_TRUE );

				if ( os_directory_create(
					sw_update_dir,
					DIRECTORY_CREATE_MAX_TIMEOUT ) == OS_STATUS_SUCCESS )
				{
					IOT_LOG( iot_lib, IOT_LOG_INFO,
						"Created Update Directory: %s\n",
						sw_update_dir );
					result = IOT_STATUS_SUCCESS;
				}
			}
			if ( result == IOT_STATUS_SUCCESS )
			{
				iot_file_progress_t ctx;
				iot_options_t *const options =
					iot_options_allocate(
						device_manager_info->iot_lib );

				os_memset(&ctx, 0,sizeof(iot_file_progress_t));
				ctx.status = IOT_STATUS_FAILURE;

				IOT_LOG( iot_lib, IOT_LOG_DEBUG,
					"Checking global file store for pkg: %s download to %s\n",
					file_to_download, sw_update_dir);

				/* FIXME: this should be an optional
				 * parameter to the cb */
				iot_options_set_bool( options, "global",
					IOT_TRUE );

				/* Setup the local path */
				os_snprintf(local_archive_path, PATH_MAX,
					"%s%c%s",sw_update_dir,
					OS_DIR_SEP, file_to_download );

				IOT_LOG( iot_lib, IOT_LOG_INFO,
					"Downloading to %s (%s%c%s)",
					local_archive_path, sw_update_dir,OS_DIR_SEP,file_to_download);

				result = iot_file_download(
						iot_lib,
						NULL,
						options,
						file_to_download,
						local_archive_path,
						&device_manager_ota_progress, &ctx);
				iot_options_free( options );
			}
			if ( result == IOT_STATUS_SUCCESS)
			{
				iot_bool_t done = IOT_FALSE;

				/* need to wait for the file to arrive */
				do {
					if ( os_file_exists(local_archive_path))
						done = IOT_TRUE;
					else
						os_time_sleep(1000, IOT_FALSE);

					IOT_LOG( iot_lib, IOT_LOG_DEBUG,
						"Waiting for file %s\n", local_archive_path);

				} while( done != IOT_TRUE );

				IOT_LOG( iot_lib, IOT_LOG_DEBUG,
					"File %s downloaded successfully\n", local_archive_path);

				result = device_manager_ota_install_execute(
						device_manager_info,
						sw_update_dir,
						file_to_download);
			}
			IOT_LOG( iot_lib , IOT_LOG_TRACE,
				"software update install result: %d", result );

			/* upload the log file, when we can read the
			 * device id, prepend it */
			os_snprintf(sw_update_log, PATH_MAX, "%s%c%s%c%s",
				runtime_dir, OS_DIR_SEP,
				"update", OS_DIR_SEP,
				IOT_UPDATE_LOGFILE);

			/* don't check the retcode here, it is not
			 * important*/
			iot_file_upload( iot_lib,     /* lib handle */
				NULL,                 /* transaction id */
				NULL,                 /* options */
				NULL,                 /* file name to rename on cloud */
				sw_update_log,        /* path to send */
				NULL,                 /* callback func */
				NULL );               /* user data */
		}
	}
	return result;
}

/*FIXME*/
#if 0
size_t device_manager_software_update_del_characters(
		char *command_param, const char *word )
{
	size_t word_length = os_strlen( word );
	size_t param_length = 0;

	if ( word_length != 0 && command_param )
	{
		char *param = command_param;
		param_length = os_strlen( command_param );
		while ( ( param = os_strstr( param, word ) ) != NULL )
		{
			char *dst = param;
			char *src = param + word_length;
			os_memmove(dst, src, param_length + 1 );
			param_length -= word_length;
		}
	}
	return param_length;
}
#endif

iot_status_t device_manager_ota_install_execute(
	struct device_manager_info *device_manager_info,
	const char *package_path, const char* file_name )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( device_manager_info && package_path && package_path[0] != '\0' &&
		file_name && file_name[0] != '\0')
	{
		char iot_update_dup_path[PATH_MAX + 1u] = "";
		char command_with_params[PATH_MAX + 1u];
		iot_t *const iot_lib = device_manager_info->iot_lib;

		result = device_manager_ota_extract_package(iot_lib, package_path, file_name);

		IOT_LOG( iot_lib, IOT_LOG_TRACE,
				"software update package_path: %s, file_name: %s",
				package_path,
				file_name);
		if ( result ==IOT_STATUS_SUCCESS )
		{
			char iot_update_path[PATH_MAX + 1u];
			char exec_dir[PATH_MAX + 1u];

			result = IOT_STATUS_EXECUTION_ERROR;
			app_path_executable_directory_get(exec_dir, PATH_MAX);
			if ( app_path_which( iot_update_path, PATH_MAX, exec_dir, IOT_TARGET_UPDATE) )
			{
				/**
				  * IDP system Truested Path Execution (TPE) protection
				  * restricts the execution of files under certain circumastances
				  * determined by their path. The copy of iot-update in the
				  * directory on IDP must have execution permissions. It's hard to
				  * guarantee the directory have such permission for all IDP security
				  * combinations. It's safe to use the default execution directory to
				  * execute the copy of iot-update.
				  * It is also applicable to other systems execpt for Android dut to it has
				  * other permission restriction.
				  */
				const char *iot_update_dup_dir = NULL;
				os_status_t osal_status = OS_STATUS_FAILURE;
#ifdef  __ANDROID__
				char temp_dir[PATH_MAX + 1];
				iot_update_dup_dir = os_directory_get_temp_dir(
					temp_dir, PATH_MAX );
#else
				iot_update_dup_dir = exec_dir;
#endif /* #ifdef __ANDROID__*/
				if ( OS_STATUS_SUCCESS == os_make_path(
					iot_update_dup_path,
					PATH_MAX,
					iot_update_dup_dir,
					IOT_TARGET_UPDATE"-copy"IOT_EXE_SUFFIX,
					NULL ) )
				{
					osal_status = os_file_copy(
						iot_update_path,
						iot_update_dup_path );
					os_file_sync( iot_update_dup_path );
					printf("file copy status %d\n", (int)osal_status);
				}

				if (osal_status == OS_STATUS_SUCCESS )
				{
					if ( os_file_exists( iot_update_dup_path ) )
						os_snprintf( command_with_params,
							PATH_MAX,
							"\"%s\" --path \"%s\"",
							iot_update_dup_path,
							package_path );
				}
				else
				{
					os_snprintf( command_with_params,
						PATH_MAX,
						"\"%s\" --path \"%s\"",
						iot_update_path,
						package_path );
				}
			}
		}

		if ( command_with_params[0] != '\0' )
		{
			char buf[1u] = "\0";
			char *out_buf[2u] = { buf, buf };
			size_t out_len[2u] = { 1u, 1u };
			int system_ret = 1;

			IOT_LOG( iot_lib, IOT_LOG_TRACE,
				"Executing command: %s", command_with_params );

			result = IOT_STATUS_SUCCESS;
			os_system_run_wait( command_with_params,
				&system_ret, out_buf, out_len, 0U );

			IOT_LOG( iot_lib, IOT_LOG_TRACE,
				"Completed executing OTA script with result: %i",
				system_ret );

			if ( system_ret != 0 )
				result = IOT_STATUS_EXECUTION_ERROR;
		}

		if ( ( iot_update_dup_path[0] != '\0' ) &&
		     ( os_file_exists( iot_update_dup_path ) != IOT_FALSE ) )
			os_file_delete( iot_update_dup_path );
	}

	return result;
}

iot_status_t device_manager_ota_extract_package(iot_t *iot_lib,
	const char *package_path, const char *file_name )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
printf("%s:%d package_path=%s file_name=%s\n", __func__,__LINE__,package_path, file_name);
	if ( iot_lib && package_path && file_name )
	{
		if ( os_directory_exists ( package_path ) )
		{
			char cwd[1024u];
			if ( os_directory_current( cwd, PATH_MAX ) == OS_STATUS_SUCCESS
				&& cwd[0] != '\0' )
			{
				if ( OS_STATUS_SUCCESS == os_directory_change(package_path) )
					IOT_LOG( iot_lib, IOT_LOG_TRACE,
						"Msg: Change current working directory to %s\n ",
						package_path );
			}

			/*
			 * extract ota package
			*/
			if ( os_file_exists ( file_name ) )
				result = device_manager_ota_extract_package_perform(
					iot_lib, file_name ) ;
			if ( cwd [0] != '\0')
				os_directory_change( cwd );
		}
	}
	return result;
}

iot_status_t device_manager_ota_extract_package_perform(
	iot_t *iot_lib, const char * sw_update_package)
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if( sw_update_package )
	{
		struct archive *a;
		struct archive *ext;
		struct archive_entry *entry;
		int flags;

		result = IOT_STATUS_SUCCESS;
		/* Select which attributes we want to restore. */
		flags = ARCHIVE_EXTRACT_TIME;
		flags |= ARCHIVE_EXTRACT_PERM;
		flags |= ARCHIVE_EXTRACT_ACL;
		flags |= ARCHIVE_EXTRACT_FFLAGS;

		a = archive_read_new();
		archive_read_support_format_all(a);
		archive_read_support_filter_all(a);
		ext = archive_write_disk_new();
		archive_write_disk_set_options(ext, flags);
		archive_write_disk_set_standard_lookup(ext);
		if ( archive_read_open_filename(a, sw_update_package, 10240u)
			== ARCHIVE_OK)
		{
			int r;
			while ( result == IOT_STATUS_SUCCESS )
			{
				r = archive_read_next_header(a, &entry);

				if (r == ARCHIVE_EOF)
					break;
				if (r < ARCHIVE_OK)
				{
					IOT_LOG( iot_lib, IOT_LOG_ERROR,
						"Error: reading archive header: %s",
						archive_error_string(a));
						result = IOT_STATUS_FAILURE;
				}
				else if (r < ARCHIVE_WARN)
				{
					IOT_LOG( iot_lib, IOT_LOG_ERROR,
						"Error:reading archive header: %d",
						 r);
					result = IOT_STATUS_FAILURE;
				}
				if ( result == IOT_STATUS_SUCCESS)
				{
					r = archive_write_header(ext, entry);
					if (r < ARCHIVE_OK)
					{
						IOT_LOG( iot_lib, IOT_LOG_ERROR,
						"Error: writing archive header: %s",
						archive_error_string(ext));
						result = IOT_STATUS_FAILURE;
					}
					else if (archive_entry_size(entry) > 0)
					{
						r = device_manager_ota_copy_data(a, ext);
						if (r < ARCHIVE_OK)
						{
							IOT_LOG( iot_lib, IOT_LOG_ERROR,
							"Error: copy archive : %s",
							archive_error_string(ext));
							result = IOT_STATUS_FAILURE;
						}
						else if (r < ARCHIVE_WARN)
						{
							IOT_LOG( iot_lib, IOT_LOG_ERROR,
							"Error: copy archive: %d",
							r);
							result = IOT_STATUS_FAILURE;
						}
					}
				}
				if ( result == IOT_STATUS_SUCCESS )
				{
					r = archive_write_finish_entry(ext);
					if (r < ARCHIVE_OK)
					{
						IOT_LOG( iot_lib, IOT_LOG_ERROR,
						"Error: writing archive finish entry: %s",
						 archive_error_string(ext));
						result = IOT_STATUS_FAILURE;
					}
					else if (r < ARCHIVE_WARN)
					{
						IOT_LOG( iot_lib, IOT_LOG_ERROR,
						"Error: writing archive finish entry: %d",
						r);

						result = IOT_STATUS_FAILURE;
					}
				}
			}
		}
		else
			IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s",
				"Error: open archive filename");
		archive_read_close(a);
		archive_read_free(a);
		archive_write_close(ext);
		archive_write_free(ext);
	}
	return result;
}

int device_manager_ota_copy_data(struct archive *ar, struct archive *aw)
{
	int r = ARCHIVE_WARN;
	const void *buff = NULL;
	size_t size;
	int64_t offset;
	int status = IOT_TRUE;

	if ( ar && aw )
	{
		while( status == IOT_TRUE)
		{
			r = archive_read_data_block(ar, &buff, &size, &offset);
			if (r == ARCHIVE_EOF)
			{
				status = IOT_FALSE;
				r = ARCHIVE_OK;
			}
			if ( r < ARCHIVE_OK)
				status = IOT_FALSE;

			if ( status == IOT_TRUE )
			{
#if ARCHIVE_VERSION_NUMBER < 4000000
				__LA_SSIZE_T amount_wrote;
#else
				la_ssize_t amount_wrote;
#endif
				amount_wrote = archive_write_data_block(
					aw, buff, size, offset );
				if (amount_wrote < ARCHIVE_OK)
				{
					status = IOT_FALSE;
				}
			}
		}
	}
	return r;
}
