/**
 * @file
 * @brief Source file for software updates specific functions
 *
 * @copyright Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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
#include "update_main.h"

#include "iot_build.h"
#include "iot_json.h"

#include "utilities/app_arg.h"
#include "../api/shared/iot_types.h"

#include <os.h>
#include <errno.h>
#include <stdarg.h>     /* for va_list, va_start, va_end */

/*#define UPDATE_DEBUG*/

/**
 * @brief Maximum length for ota script/command output
 */
#define IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN 1024u
/**
 * @brief Json file for ota
 */
#define IOT_UPDATE_JSON "update.json"
/**
 * @brief Maximum length for iot each message log
 */
#define IOT_UPDATE_LOG_MAX_LEN 128u
/**
 * @brief Maximum length for iot update timestamp
 */
#define IOT_UPDATE_TIMESTAMP_LENGTH 16u

/**
 * @def PRIVLEGE_MODE
 * @brief designates the run-level for certain commands
 */
#if defined( __unix__ ) && !defined( __ANDROID__ )
#	define PRIVILEGE_MODE     OS_TRUE
#else
#	define PRIVILEGE_MODE     OS_FALSE
#endif

/** @brief iot update log output */
enum update_log_output
{
	/** @brief log to send to cloud */
	IOT_UPDATE_LOG_CLOUD_ONLY = 0,
	/** @brief log to file */
	IOT_UPDATE_LOG_FILE_ONLY,
	/** @brief log to file and cloud */
	IOT_UPDATE_LOG_FILE_CLOUD
};

/** @brief IDP MEC availability */
enum update_mec_available
{
	/** @brief IDP MEC is unavailable */
	IOT_UPDATE_MEC_UNAVAILABLE = 0,
	/** @brief IDP MECis available */
	IOT_UPDATE_MEC_AVAILABLE
};

/** @brief Enable/disable IDP MEC */
enum update_mec_enable
{
	/** @brief Disable IDP MEC */
	IOT_UPDATE_MEC_DISABLE = 0,
	/** @brief Enable IDP MEC */
	IOT_UPDATE_MEC_ENABLE
};

/** @brief IDP MEC security status */
enum update_mec_status
{
	/** @brief MEC security enabled */
	IOT_UPDATE_MEC_STATUS_ENABLED = 0,
	/** @brief MEC security not enabled or in update status*/
	IOT_UPDATE_MEC_STATUS_NOT_ENABLED,
	/** @brief MEC security unknown status */
	IOT_UPDATE_MEC_STATUS_UNKNOWN
};

/** @brief iot update install phase */
enum update_install_phase
{
	/** @brief pre-install phase */
	OTA_PHASE_PRE_INSTALL = 0,
	/** @brief install phase */
	OTA_PHASE_INSTALL,
	/** @brief post-install phase */
	OTA_PHASE_POST_INSTALL,
	/** @brief an occured error during installation */
	OTA_PHASE_ERROR
};

/**
 * @brief Structure containing information for iot update script
 */
struct update_install_script {
	/** @brief install phase */
	const char *name;
	/** @brief install script/command */
	char *script;
};

/**
 * @brief Contains main code for iot update install
 *
 * @param[in]      sw_update_path      update package directory
 *
 * @retval IOT_STATUS_BAD_PARAMETER    on failure
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t update(
	const char * sw_update_path );

/**
 * @brief Enable/disable IDP mec security
 *
 * @param[in]      enable              action to enable/disable MEC
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t update_mec_enable(enum update_mec_enable enable );
/**
 * @brief Check if it is IDP security system
 *
 * @return a MEC available or MEC unavailable status
 */
static enum update_mec_available update_mec_is_available( void );
/**
 * @brief Check IDP mec security status
 *
 * @return a MEC status
 */
static enum update_mec_status update_mec_status( void );
 /**
 * @brief Function to log iot update information
 *
 * @param[in,out]  log_file            log file to write to
 * @param[in]      output              log to file/cloud
 * @param[in]      fmt                 string format
 */
static void update_log(os_file_t log_file,
	enum update_log_output output, const char *fmt,... )
	__attribute__((format(printf,3,4)));
/**
 * @brief Function to parse iot update json file
 *
 * @param[in]      json_file           json file to be parsed
 * @param[in,out]  update_install  install script
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t update_parse_json(
	const char* json_file,
	struct update_install_script * update_install );

/**
 * @brief Obtains the device id
 *
 * @param[in]      buf                 buffer to store the device id
 * @param[in]      len                 number of bytes to read
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FAILURE          function encountered an error
 * @retval IOT_STATUS_SUCCESS          function completed successfully
 */
static iot_status_t update_get_device_id(
	char *buf,
	size_t len );


iot_status_t update_get_device_id(
	char *buf,
	size_t len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( buf )
	{
		char filename[PATH_MAX + 1u];
		size_t filename_len;

		result = IOT_STATUS_FAILURE;
		filename_len = iot_directory_name_get( IOT_DIR_RUNTIME,
			 filename, PATH_MAX );
		if ( filename_len < PATH_MAX )
		{
			os_file_t fd;

			os_snprintf( &filename[filename_len],
				PATH_MAX - filename_len, "%c%s",
				OS_DIR_SEP, IOT_DEFAULT_FILE_DEVICE_ID );
			filename[ PATH_MAX ] = '\0';

			fd  = os_file_open( filename, OS_READ );
			os_memzero( buf, len );
			if ( fd  != OS_FILE_INVALID )
			{
				size_t bytes;
				/* read uuid from the file */
				bytes = os_file_read( buf,
					sizeof(char), len, fd );
				os_file_close( fd );
				if( bytes == len )
					buf[bytes - 1u] = '\0';
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

iot_status_t update(
	const char * sw_update_path )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sw_update_path /*&& iot_os_directory_exists( sw_update_path )*/ )
	{
		iot_bool_t reboot = IOT_FALSE;
		char cwd[ PATH_MAX + 1u ];
		os_file_t log_fd = NULL;
		iot_t *iot_lib = NULL;
		char device_id[IOT_ID_MAX_LEN + 1u];
		result = update_get_device_id( device_id, IOT_ID_MAX_LEN );
		if ( result != IOT_STATUS_SUCCESS )
		{
			os_fprintf( OS_STDERR, "Failed to get device id.\n");
		}
		else

		if ( result == IOT_STATUS_SUCCESS )
		{
			/* attempt to connect */
#if defined( _VXWORKS )
			iot_lib = iot_initialize( IOT_TARGET_UPDATE, NULL, 0 );
			result = iot_connect( iot_lib, 0u );
			iot_terminate(iot_lib, 0u);
#else /* if defined( _VXWORKS ) */
#endif /* else if defined( _VXWORKS ) */
		}

		if ( result == IOT_STATUS_SUCCESS && iot_lib )
		{
			os_directory_current( cwd, PATH_MAX );
			os_directory_change(sw_update_path);

			log_fd = os_file_open(
				IOT_UPDATE_LOGFILE, OS_WRITE|OS_CREATE);
			if ( log_fd == NULL)
				result = IOT_STATUS_FAILURE;
			else
				update_log( log_fd,
					IOT_UPDATE_LOG_CLOUD_ONLY, "%s: Started",
					IOT_TARGET_UPDATE );

		}

		if ( result == IOT_STATUS_SUCCESS && iot_lib )
		{
			char pre_install_script[ PATH_MAX + 1u ];
			char install_script[ PATH_MAX + 1u ];
			char post_install_script[ PATH_MAX + 1u ];
			char err_install_script[ PATH_MAX + 1u ];
			char need_reboot[ 128u ];
			enum update_mec_status mec_original_status =
				IOT_UPDATE_MEC_STATUS_UNKNOWN;
			enum update_mec_available  mec_available =
				IOT_UPDATE_MEC_UNAVAILABLE;
			char *out_buf[2u];
			size_t out_len[2u];
			char buf_stderr[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN + 1u ];
			char buf_stdout[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN + 1u ];

			struct update_install_script update_install[] = {
				{ "pre_install" ,  pre_install_script },
				{ "install"     ,  install_script },
				{ "post_install",  post_install_script },
				{ "error_action" , err_install_script },
				{ "reboot",        need_reboot},
				{NULL, NULL}
			};

			out_buf[0] = buf_stdout;
			out_len[0] = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;
			out_buf[1] = buf_stderr;
			out_len[1] = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;

			update_log( log_fd,
				IOT_UPDATE_LOG_FILE_ONLY,
				"Downloading and extracting the Update Package ... Successful! " );

			result = IOT_STATUS_FAILURE;
			mec_available = update_mec_is_available();
			if ( mec_available == IOT_UPDATE_MEC_AVAILABLE )
			{
				mec_original_status = update_mec_status();

				if ( mec_original_status == IOT_UPDATE_MEC_STATUS_ENABLED)
				{
					result = update_mec_enable( IOT_UPDATE_MEC_DISABLE );
					if ( result == IOT_STATUS_SUCCESS )
						update_log( log_fd,
							IOT_UPDATE_LOG_FILE_ONLY,
							"MEC security is disabled" );
					else
						update_log( log_fd,
							IOT_UPDATE_LOG_FILE_ONLY,
							"Disable MEC failed" );
				}
				else
					update_log( log_fd,
						IOT_UPDATE_LOG_FILE_ONLY,
						"MEC original status is not enabled,"
						"don't need to be disabled" );
			}

			/*
			 * parse update.json file
			 */
			if ( os_file_exists ( IOT_UPDATE_JSON ) )
			{
				result = update_parse_json(
					IOT_UPDATE_JSON,
					update_install);
			}
			if ( result == IOT_STATUS_SUCCESS )
			{
				size_t i;
				size_t j;
				/*
				 * execute install scripts/commands
				 */
				for ( i = OTA_PHASE_PRE_INSTALL;
					i < OTA_PHASE_ERROR &&
					result == IOT_STATUS_SUCCESS;
					i++ )
				{
					if ( update_install[i].script[0] != '\0' &&
						os_strcmp( update_install[i].script, " " ) != 0)
					{
						os_system_run_args_t args =
							OS_SYSTEM_RUN_ARGS_INIT;
						char *param = update_install[i].script ;
						size_t  param_length =
							os_strlen( update_install[i].script );

						/* remove escape character backslash '\' added in Json file*/
						while ( ( param = os_strstr( param, "\\") ) != NULL )
						{
							char *dst = param;
							char *src = param + 1;
							os_memmove(dst, src, param_length + 1 );
							param_length -= 1;
							/* if the next character is still '\'
							 * it is character which is escaped
							 * don't delete it
							 */
							if(param && param[0] == '\\')
							{
								param ++;
								param_length -= 1;
							}
						}

						for ( j = 0u; j < 2u; ++j )
						{
							if ( out_buf[j] )
							{
								os_memset(
								out_buf[j], 0,out_len[j] );
							}
						}
						update_log( log_fd,
							IOT_UPDATE_LOG_FILE_CLOUD,
							"Executing %s (%s) ... Start!",
							update_install[i].name,
							update_install[i].script );

						result = IOT_STATUS_FAILURE;
						args.cmd = update_install[i].script;
						args.block = OS_TRUE;
						args.opts.block.std_out.buf = out_buf[0];
						args.opts.block.std_out.len = out_len[0];
						args.opts.block.std_err.buf = out_buf[1];
						args.opts.block.std_err.len = out_len[1];
						if ( os_system_run( &args )
							== OS_STATUS_SUCCESS )
							result = IOT_STATUS_SUCCESS;

						for ( j = 0u; j < 2u; ++j )
						{
							if ( out_buf[j] && out_buf[j][0] != '\0' )
							{
								os_fprintf(
								log_fd,
								"%s log: \n%s \n",
								update_install[i].name,
								out_buf[j] );
							}
						}

						if ( result != IOT_STATUS_SUCCESS || args.return_code != 0 )
						{
							update_log( log_fd,
								IOT_UPDATE_LOG_FILE_CLOUD,
								"Error: Executing %s (%s) ... Failed! ",
								update_install[i].name, update_install[i].script );

							result = IOT_STATUS_FAILURE;
							if ( update_install[OTA_PHASE_ERROR].script[0] != '\0' &&
								os_strcmp( update_install[OTA_PHASE_ERROR].script, " " ) != 0)
							{
								update_log( log_fd,
									IOT_UPDATE_LOG_FILE_CLOUD,
									"Executing %s (%s) ... Start!",
									update_install[OTA_PHASE_ERROR].name,
									update_install[OTA_PHASE_ERROR].script);

								args.cmd = update_install[OTA_PHASE_ERROR].script;
								if ( os_system_run( &args )
									== OS_STATUS_SUCCESS )
									result = IOT_STATUS_SUCCESS;

								if ( result == IOT_STATUS_SUCCESS )
								{
									update_log( log_fd,
										IOT_UPDATE_LOG_FILE_CLOUD,
										"Executing %s (%s) ... "
										"Successful!",
										update_install[OTA_PHASE_ERROR].name,
										update_install[OTA_PHASE_ERROR].script);
								}
								else
								{
									update_log( log_fd,
										IOT_UPDATE_LOG_FILE_CLOUD,
										"Executing %s (%s) ... "
										"Failed!",
										update_install[OTA_PHASE_ERROR].name,
										update_install[OTA_PHASE_ERROR].script);
								}
							}
							else
								update_log( log_fd,
									IOT_UPDATE_LOG_FILE_ONLY,
									"Info: No %s script provided ",
									update_install[OTA_PHASE_ERROR].name
									);
						}
						else
						{
							update_log( log_fd,
								IOT_UPDATE_LOG_FILE_CLOUD,
								"Executing %s (%s) ... Successful!",
								update_install[i].name,
								update_install[i].script );
						}
					}
					else
						update_log( log_fd,
							IOT_UPDATE_LOG_FILE_ONLY,
							"Info: No %s script provided ",
							update_install[i].name
							);
				}
				if ( mec_available == IOT_UPDATE_MEC_AVAILABLE )
				{
					if ( mec_original_status == IOT_UPDATE_MEC_STATUS_ENABLED )
					{
						if ( update_mec_status() ==
							IOT_UPDATE_MEC_STATUS_NOT_ENABLED )
						{
							if ( ( result = update_mec_enable(IOT_UPDATE_MEC_ENABLE)) !=
								IOT_STATUS_SUCCESS )
								update_log( log_fd,
									IOT_UPDATE_LOG_FILE_ONLY,
									"Enable MEC failed" );
							else
								update_log( log_fd,
									IOT_UPDATE_LOG_FILE_ONLY,
									"MEC is enabled" );
						}
					}
					else
						update_log( log_fd,
							IOT_UPDATE_LOG_FILE_ONLY,
							"MEC original status is not enabled,"
							"don't need to be enabled" );
				}
			}

			/*
			 * check if it need reboot
			 */
			if ( (result == IOT_STATUS_SUCCESS )
				&& os_strncmp( need_reboot,
				"yes", 3u ) == 0)
			{
				reboot = IOT_TRUE;
				update_log( log_fd,
					IOT_UPDATE_LOG_FILE_CLOUD,
					"Device will reboot after 1 minute... "
					);
			}
			/*
			 * ota install result
			 */
			if ( result == IOT_STATUS_SUCCESS )
			{
				update_log( log_fd,
					IOT_UPDATE_LOG_FILE_CLOUD,
					"Software Update Finish... Successful! "
					);
			}
			else
			{
				update_log( log_fd,
					IOT_UPDATE_LOG_FILE_CLOUD,
					"Software Update Finish... Failed! " );
			}

			if ( log_fd )
			{
				os_file_close( log_fd );
				log_fd = NULL;
			}

			/* FIXME upload the log file */

		}
		else
			update_log( log_fd,
				IOT_UPDATE_LOG_FILE_CLOUD,
				"Error: Open iot update json file ... Failed!\n " );
		if ( cwd[0] != '\0' )
			os_directory_change(cwd);

		/*
		 * reboot if required
		 */
		if ( reboot != IOT_FALSE )
		{
#if defined( _WIN32 )
			/* reboot after 1 minute */
			os_system_shutdown( IOT_TRUE, 1u );
#else /* if defined( _WIN32 ) */
			os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
			char command_with_params[ 32u ];
			os_strncpy( command_with_params,
				"iot-control --reboot 1",
				31u );
			command_with_params[31u] = '\0';
			args.cmd = command_with_params;
			args.block = OS_TRUE;
			args.privileged = OS_TRUE;
			os_system_run( &args );
#endif /* else if defined( _WIN32 ) */
		}
	}
	return result;
}

iot_status_t update_mec_enable(
	enum update_mec_enable enable )
{
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
	char command [32u];
	iot_status_t result = IOT_STATUS_FAILURE;

	if ( enable == IOT_UPDATE_MEC_ENABLE )
		os_snprintf( command, 31u, "sadmin eu" );
	else
		os_snprintf( command, 31u, "sadmin bu" );

	args.cmd = command;
	args.block = OS_TRUE;
	args.privileged = PRIVILEGE_MODE;
	if ( os_system_run( &args ) == OS_STATUS_SUCCESS )
		result = IOT_STATUS_SUCCESS;

	if ( result == IOT_STATUS_SUCCESS && args.return_code == 0 )
	{
		enum update_mec_status mec_status;
		result = IOT_STATUS_FAILURE;
		mec_status = update_mec_status();
		if( ( enable == IOT_UPDATE_MEC_ENABLE &&
			mec_status == IOT_UPDATE_MEC_STATUS_ENABLED ) ||
			( enable == IOT_UPDATE_MEC_DISABLE &&
			mec_status == IOT_UPDATE_MEC_STATUS_NOT_ENABLED )
			)
			result = IOT_STATUS_SUCCESS;
	}
	else
		result = IOT_STATUS_FAILURE;
	return result;
}

enum update_mec_available update_mec_is_available( void )
{
#if 0
	char update_path[PATH_MAX + 1u];
	enum update_mec_available  mec_available = IOT_UPDATE_MEC_UNAVAILABLE;

	if ( app_path_which( update_path, PATH_MAX, NULL, "sadmin" ))
		mec_available = IOT_UPDATE_MEC_AVAILABLE;
	return mec_available;
#else
	return IOT_UPDATE_MEC_UNAVAILABLE;
#endif
}
enum update_mec_status update_mec_status( void )
{
	os_system_run_args_t args = OS_SYSTEM_RUN_ARGS_INIT;
	char buf_stderr[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN ];
	char buf_stdout[ IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN ];
	os_status_t result = OS_STATUS_FAILURE;
	enum update_mec_status mec_status = IOT_UPDATE_MEC_STATUS_UNKNOWN;

	args.cmd = "sadmin status";
	args.privileged = PRIVILEGE_MODE;
	args.block = OS_TRUE;
	args.opts.block.std_out.buf = buf_stdout;
	args.opts.block.std_out.len = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;
	args.opts.block.std_err.buf = buf_stderr;
	args.opts.block.std_err.len = IOT_UPDATE_COMMAND_OUTPUT_MAX_LEN;
	result = os_system_run( &args );

	if ( result == OS_STATUS_SUCCESS && args.return_code == 0 )
	{
		if ( os_strstr( args.opts.block.std_out.buf, "enable" ) != NULL )
			mec_status = IOT_UPDATE_MEC_STATUS_ENABLED;
		else
			mec_status = IOT_UPDATE_MEC_STATUS_NOT_ENABLED;
	}

	return mec_status;
}

iot_status_t update_parse_json(
	const char* json_file, struct update_install_script * update_install)
{
	iot_status_t status = IOT_STATUS_BAD_PARAMETER;
	if (json_file && update_install )
	{
		os_file_t fd;
		char *json_string;
		long ssize = 0;
		char cwd[PATH_MAX];

		status = IOT_STATUS_FAILURE;
		os_directory_current( cwd, PATH_MAX );
		fd = os_file_open( json_file, OS_READ );
		if( fd )
		{
			long cur_pos = os_file_tell( fd );
			if ( os_file_seek( fd, 0, OS_FILE_SEEK_END ) == 0 )
			{
				ssize = os_file_tell( fd );
				if ( cur_pos != ssize )
					os_file_seek( fd, cur_pos, OS_FILE_SEEK_START );
			}
		}
		else
			os_fprintf(OS_STDERR,
				" failed to open json file!\n");

		if ( ssize != 0 )
		{
			size_t size = (size_t)ssize;
			json_string = (char *)os_malloc( size + 1 );
			if ( json_string )
			{
				char buf[1024u];
				iot_json_decoder_t *json;
				const iot_json_item_t *root;

				size = os_file_read( json_string, 1, (size_t)size, fd );
				json_string[size] = '\0';
				json = iot_json_decode_initialize( buf, 1024u, 0u );
				if ( json &&
					iot_json_decode_parse( json, json_string, size,
						&root, NULL, 0u) == IOT_STATUS_SUCCESS )
				{
					if ( 1 )
					{
						const char *bad_field = NULL;
						size_t i = 0;
						const iot_json_item_t *j_obj;
						/*iot_json_decode_array_start( json );*/
						while (update_install[i].name != NULL
							&& update_install[i].script
							&& bad_field == NULL)
						{
							j_obj = iot_json_decode_object_find(
								json, root, update_install[i].name );

							if ( j_obj &&
								iot_json_decode_type( json, j_obj )
								== IOT_JSON_TYPE_STRING )
							{
								const char *v = NULL;
								size_t v_len = 0u;

								iot_json_decode_string( json,
									j_obj,
									&v,
									&v_len );
								os_snprintf( update_install[i].script,
									PATH_MAX,
									"%.*s",
									(int)v_len,
									v );
								update_install[i].script[ PATH_MAX ] = '\0';
								os_printf("%s: %s\n",
									update_install[i].name,
									update_install[i].script);
								i ++;
							}
							else
								bad_field = update_install[i].name;
						}

						if (bad_field == NULL && i != 0 )
							status = IOT_STATUS_SUCCESS;
						else
							os_fprintf( OS_STDERR,
								"Error: invalid field: %s",
								bad_field);
					}
				}
				iot_json_decode_terminate( json );
				os_free( json_string );
			}
		}
		else
			os_fprintf(OS_STDERR,
				" the size of json file is 0!\n");
	}

	return status;
}

/* main entry point function */
int update_main( int argc, char *argv[] )
{
	int result = EXIT_FAILURE;
	if ( argc == 2 && (os_strcmp( argv[1], "-h") == 0
		|| os_strcmp( argv[1], "--help")) )
		os_printf("Command format:\n"
			"%s --path [software udpate package path]",
			IOT_TARGET_UPDATE );
	else if (argc == 3 )
	{
		if ( *argv[2] != '\0' &&
			( os_strcmp(argv[1], "-p") == 0 || os_strcmp(argv[1], "--path") == 0 ) )
		{
			result = update( argv[2] );
			if ( result == IOT_STATUS_SUCCESS )
				result = EXIT_SUCCESS;
		}
		else
			os_printf( "Wrong command parameter! It should be:\n"
				"%s --path [software udpate package path]\n",
				IOT_TARGET_UPDATE );
	}
	else
		os_printf("Wrong command!\n"
			"Please type %s --help for help\n", IOT_TARGET_UPDATE );

	return result;
}

void update_log(
	os_file_t log_file,
	enum update_log_output output,
	const char *fmt, ... )
{
	char sw_update_log[ IOT_UPDATE_LOG_MAX_LEN + 1u];
	char timestamp[IOT_UPDATE_TIMESTAMP_LENGTH  + 1u];
	os_timestamp_t t;
	va_list args;

	/* add time stamp */
	os_time(&t, NULL );
	os_time_format( timestamp, IOT_UPDATE_TIMESTAMP_LENGTH,
		"%Y-%m-%dT%H:%M:%S", t, IOT_FALSE );

	va_start( args, fmt );
	os_vsnprintf( sw_update_log, IOT_UPDATE_LOG_MAX_LEN, fmt, args );
	sw_update_log[ IOT_UPDATE_LOG_MAX_LEN ] = '\0';
	va_end( args );

	if ( log_file && ( output == IOT_UPDATE_LOG_CLOUD_ONLY ||
			output == IOT_UPDATE_LOG_FILE_CLOUD ) )
		os_fprintf(log_file,
			"------------------------------------------------------\n"
			"- %s\n"
			"- %s\n"
			"------------------------------------------------------\n",
			timestamp,
			sw_update_log );
}
