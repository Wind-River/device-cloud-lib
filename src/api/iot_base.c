/**
 * @file
 * @brief source file for base IoT library functionality
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

#include "iot_build.h"            /* for version information from build */

#include "iot_common.h"
#include "public/iot_json.h"      /* for iot json library structures */
#include "shared/iot_types.h"     /* for internal library structures */

#include <os.h>
#include <stdarg.h> /* for va_arg */

/** @brief Maximum log message line length */
#define IOT_LOG_MSG_MAX 16384u

#ifdef IOT_STACK_ONLY
/** @brief static library on the stack */
static struct iot IOT_LIB;
#endif

/** @brief Structure defining string representations of log levels */
static const char *IOT_LOG_LEVEL_MAP[] =
{
	"FATAL",    /* maps to IOT_LOG_LEVEL_FATAL = 0 */
	"ALERT",    /* maps to IOT_LOG_LEVEL_ALERT = 1 */
	"CRITICAL", /* maps to IOT_LOG_LEVEL_CRITICAL = 2 */
	"ERROR",    /* maps to IOT_LOG_LEVEL_ERROR = 3 */
	"WARNING",  /* maps to IOT_LOG_LEVEL_WARNING = 4 */
	"NOTICE",   /* maps to IOT_LOG_LEVEL_NOTICE = 5 */
	"INFO",     /* maps to IOT_LOG_LEVEL_INFO = 6 */
	"DEBUG",    /* maps to IOT_LOG_LEVEL_DEBUG = 7 */
	"TRACE",    /* maps to IOT_LOG_LEVEL_TRACE = 8 */
	"ALL"       /* maps to IOT_LOG_LEVEL_ALL = 9 */
};


/** @brief Structure to associate a status id with a message */
typedef struct iot_status_msg {
	iot_status_t id;    /**< @brief status id */
	const char *msg;    /**< @brief status message */
} iot_status_msg_t;

/** @brief Associates error ids with messages */
static const iot_status_msg_t IOT_STATUS_MESSAGES[] = {
	{ IOT_STATUS_SUCCESS, "success" },
	{ IOT_STATUS_INVOKED, "invoked" },
	{ IOT_STATUS_BAD_PARAMETER, "invalid parameter" },
	{ IOT_STATUS_BAD_REQUEST, "bad request" },
	{ IOT_STATUS_EXECUTION_ERROR, "execution error" },
	{ IOT_STATUS_EXISTS, "already exists" },
	{ IOT_STATUS_FILE_OPEN_FAILED, "file open failed" },
	{ IOT_STATUS_FULL, "storage is full" },
	{ IOT_STATUS_IO_ERROR, "input/output error" },
	{ IOT_STATUS_NO_MEMORY, "out of memory" },
	{ IOT_STATUS_NO_PERMISSION, "permission denied" },
	{ IOT_STATUS_NOT_EXECUTABLE, "not executable" },
	{ IOT_STATUS_NOT_FOUND, "not found" },
	{ IOT_STATUS_NOT_INITIALIZED, "not initialized" },
	{ IOT_STATUS_NOT_SUPPORTED, "not supported" },
	{ IOT_STATUS_OUT_OF_RANGE, "value out of range" },
	{ IOT_STATUS_PARSE_ERROR, "error parsing message" },
	{ IOT_STATUS_TIMED_OUT, "timed out" },
	{ IOT_STATUS_TRY_AGAIN, "try again" },

	/* should be the last option */
	{ IOT_STATUS_FAILURE, "internal error" }
};

/**
 * @brief Sets the value of a piece of configuration data
 *
 * @param[in,out]  handle             library handle
 * @param[in]      name               configuration option name
 * @param[in]      data               configuration option value to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER   invalid parameter passed to the function
 * @retval IOT_STAUTS_FULL            maximum number of options reached
 * @retval IOT_STATUS_SUCCESS         on success
 */
static IOT_SECTION iot_status_t iot_config_set_data(
	iot_t *handle,
	const char *name,
	const struct iot_data *data );

#ifndef IOT_NO_THREAD_SUPPORT
/**
 * @brief default main thread
 *
 * @param[in,out]  user_data           pointer to the library instance
 *
 * @retval NULL    always on thread termination
 */
static OS_THREAD_DECL iot_base_main_thread( void *user_data );
/**
 * @brief worker thread main function
 *
 * @param[in,out]  user_data           pointer to the library instance
 *
 * @retval NULL    always on thread termination
 */
static OS_THREAD_DECL iot_base_worker_thread_main( void *user_data );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

/**
 * @brief Gets the connect configuration
 *
 * @param[in,out]  lib                 library handle
 * @param[in,out]  max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely) (optional),
 *                                     returns amount of time remaining
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_base_configuration_load(
	iot_t *lib,
	iot_millisecond_t *max_time_out );

/**
 * @brief Reads and processes the configuration file content
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      file_path           path of configuration file to read
 * @param[in,out]  max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely) (optional),
 *                                     returns amount of time remaining
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_base_configuration_read(
	iot_t *lib,
	const char *file_path,
	iot_millisecond_t *max_time_out );

/**
 * @brief Parses the configuration file
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      file_path           path of configuration file read
 * @param[in]      buf                 configuration file contents
 * @param[in]      len                 buffer length
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_base_configuration_parse(
	iot_t *lib,
	const char *file_path,
	const char *buf,
	const size_t len );

/**
 * @brief Parses the base object in a configuration file
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      json                json decoder
 * @param[in]      obj                 object being parsed
 * @param[in]      key                 current object key
 * @param[in]      key_len             length of the key
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_base_configuration_parse_object(
	iot_t *lib,
	iot_json_decoder_t *json,
	const iot_json_item_t *obj,
	char *key,
	size_t key_len );

/**
 * @brief Sets the device id from a file (or generates one if file doesn't exist)
 *
 * @param[in,out]  lib                 library handle
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_base_device_id_set(
	iot_t *lib );

iot_status_t iot_config_get(
	const iot_t *handle,
	const char *name,
	iot_bool_t convert,
	iot_type_t type, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( handle && name )
	{
		result = IOT_STATUS_NOT_FOUND;
		if ( handle->options_config )
		{
			va_list args;
			va_start( args, type );
			result = iot_options_get_args(
				handle->options_config, name, convert,
				type, args );
			va_end( args );
		}
	}
	return result;
}

iot_status_t iot_config_get_raw(
	const iot_t *handle,
	const char *name,
	iot_bool_t convert,
	size_t *length,
	const void **data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data )
	{
		struct iot_data_raw raw_data;
		os_memzero( &raw_data, sizeof( struct iot_data_raw ) );
		result = iot_config_get( handle, name, convert,
			IOT_TYPE_RAW, &raw_data );
		if ( length )
			*length = raw_data.length;
		*data = raw_data.ptr;
	}
	return result;
}

iot_status_t iot_config_set(
	iot_t *handle,
	const char* name,
	iot_type_t type, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( handle && name && *name )
	{
		va_list args;
		struct iot_data data;

		os_memzero( &data, sizeof( struct iot_data ) );
		va_start( args, type );
		iot_common_arg_set( &data, IOT_TRUE, type, args );
		va_end( args );
		result = iot_config_set_data( handle, name, &data );
		os_free_null( (void **)&data.heap_storage );
	}
	return result;
}

iot_status_t iot_config_set_data(
	iot_t *handle,
	const char *name,
	const struct iot_data *data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( handle && data )
	{
		/* if first option, set global options list */
		result = IOT_STATUS_NO_MEMORY;
		if ( !handle->options_config )
		{
			handle->options_config =
				iot_options_allocate( handle );
		}

		if ( handle->options_config )
		{
			result = iot_options_set_data(
				handle->options_config, name, data );
		}
	}
	return result;
}

iot_status_t iot_config_set_raw(
	iot_t *handle,
	const char* name,
	size_t length,
	const void *ptr )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( handle && name && *name )
	{
		struct iot_data data;
		os_memzero( &data, sizeof( struct iot_data ) );
		data.type = IOT_TYPE_RAW;
		data.value.raw.ptr = ptr;
		data.value.raw.length = length;
		data.has_value = IOT_TRUE;
		result = iot_config_set_data( handle, name, &data );
	}
	return result;
}

#ifndef IOT_NO_THREAD_SUPPORT
OS_THREAD_DECL iot_base_main_thread( void *user_data )
{
	struct iot *lib = (struct iot *)user_data;
	iot_loop_forever( lib );
	return (OS_THREAD_RETURN)0;
}

OS_THREAD_DECL iot_base_worker_thread_main( void *user_data )
{
	struct iot *lib = (struct iot *)user_data;
	iot_status_t result = IOT_STATUS_SUCCESS;
	while( lib && lib->to_quit == IOT_FALSE &&
		result == IOT_STATUS_SUCCESS )
		result = iot_action_process( lib, 0u );
	return (OS_THREAD_RETURN)0;
}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

iot_status_t iot_connect(
	iot_t *lib,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		const char *log_level = NULL;

		result = iot_base_configuration_load( lib, &max_time_out );
		if ( result != IOT_STATUS_SUCCESS )
			IOT_LOG( lib, IOT_LOG_ERROR, "%s",
				"Failed getting connect configuration" );

		/* set log level */
		iot_config_get( lib, "log_level", IOT_FALSE,
			IOT_TYPE_STRING, &log_level );
		if ( log_level )
			iot_log_level_set_string( lib, log_level );

		result = iot_plugin_perform( lib,
			NULL, &max_time_out,
			IOT_OPERATION_CLIENT_CONNECT,
			NULL, NULL, NULL );

		if ( result == IOT_STATUS_SUCCESS )
		{
			IOT_LOG( lib, IOT_LOG_NOTICE, "%s",
				"Connected successfully" );
#ifndef IOT_NO_THREAD_SUPPORT
			if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) &&
				(result == IOT_STATUS_SUCCESS) )
			{
				result = iot_loop_start( lib );
				if ( result != IOT_STATUS_SUCCESS )
					IOT_LOG( lib,
					IOT_LOG_ERROR, "%s",
					"Failed to start main loop" );
			}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
		}
		else
		{
			IOT_LOG( lib, IOT_LOG_ERROR, "%s",
				"Failed to connect" );
			result = IOT_STATUS_FAILURE;
		}
	}
	return result;
}

size_t iot_directory_name_get(
	iot_dir_type_t type,
	char *buf,
	size_t buf_len )
{
	size_t result = 0u;
	const char *dir_fmt = NULL;
	switch( type )
	{
		case IOT_DIR_CONFIG:
			dir_fmt = IOT_DEFAULT_DIR_CONFIG;
			break;
		case IOT_DIR_RUNTIME:
			dir_fmt = IOT_DEFAULT_DIR_RUNTIME;
			break;
		default:
			break;
	}

	if ( dir_fmt )
	{
		result = os_strlen( dir_fmt );
		if ( buf && buf_len > 0u )
		{
			os_strncpy( buf, dir_fmt, buf_len - 1u );
			/** @todo expand environment variables in
			 * path here */
			buf[ buf_len - 1u ] = '\0';
		}
	}
	return result;
}

iot_status_t iot_base_configuration_load(
	iot_t *lib,
	iot_millisecond_t *max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		char file_path[PATH_MAX + 1u];
		size_t file_path_len = 0u;
		size_t config_dir_len = 0u;
		unsigned int i;

		result = IOT_STATUS_FAILURE;

		config_dir_len = iot_directory_name_get(
			IOT_DIR_CONFIG, file_path, PATH_MAX );
		for ( i = 0u; i < 2u; ++i )
		{
			if ( i == 0u && config_dir_len < PATH_MAX )
			{
				/* global configuration file */
				os_snprintf( &file_path[config_dir_len],
					PATH_MAX - config_dir_len,
					"%c%s%s", OS_DIR_SEP,
					IOT_DEFAULT_FILE_CONFIG,
					IOT_DEFAULT_FILE_CONFIG_EXT );
				file_path_len = config_dir_len + 1u +
					os_strlen( IOT_DEFAULT_FILE_CONFIG ) +
					os_strlen( IOT_DEFAULT_FILE_CONFIG_EXT );
			}
			else
			{
				/* load app specific configuration file */
				if ( lib->cfg_file_path )
				{
					/* option 1: specified config file */
					os_strncpy( file_path,
						lib->cfg_file_path, PATH_MAX );
					file_path_len = os_strlen( lib->cfg_file_path );
				}
				else if ( config_dir_len < PATH_MAX )
				{
					/* option 2: app name config file */
					os_snprintf( &file_path[config_dir_len],
						PATH_MAX - config_dir_len,
						"%c%s%s", OS_DIR_SEP, lib->id,
						IOT_DEFAULT_FILE_CONFIG_EXT );
					file_path_len = config_dir_len + 1u +
						os_strlen( lib->id ) +
						os_strlen( IOT_DEFAULT_FILE_CONFIG_EXT );
				}
			}

			file_path[ file_path_len ] = '\0';
			if ( file_path_len > 0u )
			{
				iot_status_t interim_result;
				IOT_LOG( lib, IOT_LOG_TRACE,
					"Reading configuration from %s", file_path );

				/* process the connect configure file */
				interim_result = iot_base_configuration_read(
					lib, file_path, max_time_out );
				if ( result != IOT_STATUS_SUCCESS )
					result = interim_result;
			}
		}
	}
	return result;
}

iot_status_t iot_base_configuration_read(
	iot_t *lib,
	const char *file_path,
	iot_millisecond_t *UNUSED(max_time_out) )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && file_path )
	{
		result = IOT_STATUS_NOT_FOUND;
		/* the configuration file must be there to continue */
		IOT_LOG( lib, IOT_LOG_INFO,
			"Looking for configuration file: %s", file_path );
		if ( os_file_exists( file_path ) != IOT_FALSE )
		{
			os_file_t fd = OS_FILE_INVALID;
			fd = os_file_open( file_path, OS_READ );
			result = IOT_STATUS_FAILURE;
			if ( fd != OS_FILE_INVALID )
			{
				const size_t blk_size = 512u;
				char *buf = NULL;
				size_t buf_len = 0u;
				iot_bool_t more_to_read = IOT_TRUE;

				result = IOT_STATUS_SUCCESS;
				while ( result == IOT_STATUS_SUCCESS &&
					more_to_read == IOT_TRUE )
				{
					char *new_buf;
#ifdef IOT_STACK_ONLY
					char stack_buf[blk_size];
					new_buf = stack_buf;
					if ( buf_len < blk_size )
#else
					new_buf = os_realloc( buf,
						sizeof( char ) *
						( buf_len + blk_size ) );
					if ( new_buf )
#endif /* ifdef IOT_STACK_ONLY */
					{
						size_t bytes;

						buf = new_buf;
						bytes = os_file_read(
							&buf[buf_len],
							sizeof(char),
							blk_size, fd );
						if ( bytes == 0 )
						{
							if ( !os_file_eof( fd ) )
								result = IOT_STATUS_FAILURE;
							more_to_read = IOT_FALSE;
						}
						else
							buf_len += bytes;
					}
					else
						result = IOT_STATUS_NO_MEMORY;
				}

				if ( result == IOT_STATUS_SUCCESS )
				{
					/* process configuration file */
					result = iot_base_configuration_parse(
						lib, file_path, buf, buf_len );
				}

				if ( buf )
					os_free( buf );
				os_file_close ( fd );
				result = IOT_STATUS_SUCCESS;
			}

			if ( result != IOT_STATUS_SUCCESS )
			{
				IOT_LOG( lib, IOT_LOG_ERROR,
					"Failed to read configuration file: %s (%s)",
						file_path, iot_error( result ) );
			}
		}
	}
	return result;
}

iot_status_t iot_base_configuration_parse(
	iot_t *lib,
	const char *file_path,
	const char *buf,
	const size_t len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && buf && len > 0 )
	{
		iot_json_decoder_t *json;
		const iot_json_item_t *root;
		char err_msg[32u];

		json = iot_json_decode_initialize( NULL, 0u,
			IOT_JSON_FLAG_DYNAMIC );
		if ( json &&
			iot_json_decode_parse( json, buf, len, &root,
				err_msg, 32u ) == IOT_STATUS_SUCCESS )
		{
			char key_buff[256u];
			*key_buff = '\0';
			os_fprintf( OS_STDERR, "Current Configuration:\n" );
			iot_base_configuration_parse_object(
				lib, json, root, key_buff, 0u );
		}
		else
		{
			IOT_LOG( lib, IOT_LOG_ERROR,
				"Failed to parse configuration file: %s (%s)",
					file_path, err_msg );
		}
		iot_json_decode_terminate( json );
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_base_configuration_parse_object(
	iot_t *lib,
	iot_json_decoder_t *json,
	const iot_json_item_t *obj,
	char *key,
	size_t key_len )
{
	const iot_json_object_iterator_t *iter =
		iot_json_decode_object_iterator( json, obj );

	if ( key_len )
	{
		key[key_len] = '.';
		++key_len;
	}

	while( iter )
	{
		const char *cur_key = NULL;
		size_t cur_key_len = 0u;
		const iot_json_item_t *item = NULL;
		iot_json_type_t type;

		iot_json_decode_object_iterator_key( json,
			obj, iter, &cur_key, &cur_key_len );

		os_strncpy( &key[key_len], cur_key, cur_key_len );
		cur_key_len += key_len;
		key[cur_key_len] = '\0';

		iot_json_decode_object_iterator_value( json, obj,
			iter, &item );
		type = iot_json_decode_type( json, item );

		switch ( type )
		{
		case IOT_JSON_TYPE_BOOL:
		{
			iot_bool_t value = IOT_FALSE;
			iot_json_decode_bool( json, item, &value );
			iot_config_set( lib, key, IOT_TYPE_BOOL, value );
			os_fprintf( OS_STDERR, "%s: %s\n", key,
				(value == IOT_FALSE ? "false" : "true" ) );
			break;
		}
		case IOT_JSON_TYPE_INTEGER:
		{
			iot_int64_t value = 0;
			iot_json_decode_integer( json, item, &value );
			iot_config_set( lib, key, IOT_TYPE_INT64, value );
			os_fprintf( OS_STDERR, "%s: %llu\n", key,
				(long long int)value );
			break;
		}
		case IOT_JSON_TYPE_REAL:
		{
			iot_float64_t value = 0.0;
			iot_json_decode_real( json, item, &value );
			iot_config_set( lib, key, IOT_TYPE_FLOAT64, value );
			os_fprintf( OS_STDERR, "%s: %f\n", key, value );
			break;
		}
		case IOT_JSON_TYPE_STRING:
		{
			char *v_ptr;
			const char *value = NULL;
			size_t value_len = 0u;
			iot_json_decode_string( json, item, &value, &value_len );
			v_ptr = os_malloc( value_len + 1u );
			if ( v_ptr )
			{
				if ( value )
					os_strncpy( v_ptr, value, value_len );
				v_ptr[value_len] = '\0';
				iot_config_set( lib, key,
					IOT_TYPE_STRING, v_ptr );
				os_fprintf( OS_STDERR, "%s: %s\n", key, v_ptr );
				os_free( v_ptr );
			}
			break;
		}
		case IOT_JSON_TYPE_OBJECT:
		{
			iot_base_configuration_parse_object( lib, json,
				item, key, cur_key_len );
			break;
		}
		case IOT_JSON_TYPE_ARRAY:
		case IOT_JSON_TYPE_NULL:
		default:
			break;
		}
		iter = iot_json_decode_object_iterator_next( json, obj, iter );
	}
	return IOT_STATUS_SUCCESS;
}

iot_status_t iot_base_device_id_set(
	iot_t *lib )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		char file_path[ PATH_MAX + 1u ];
		size_t file_path_len;

		result = IOT_STATUS_FAILURE;
		file_path_len = iot_directory_name_get( IOT_DIR_RUNTIME,
			file_path, PATH_MAX );
		if ( file_path_len < PATH_MAX )
		{
			char device_id[ IOT_ID_MAX_LEN + 1u ];
			size_t device_id_len = 0u;
			os_file_t fd;
			/* determine full path to the device id file */
			os_snprintf( &file_path[file_path_len],
				PATH_MAX - file_path_len, "/%s",
				IOT_DEFAULT_FILE_DEVICE_ID );

			fd = os_file_open( file_path, OS_READ );
			*device_id = '\0';
			if ( fd  != OS_FILE_INVALID )
			{
				/* read uuid from the file */
				device_id_len =
					os_file_read( device_id, sizeof(char),
						IOT_ID_MAX_LEN, fd );
				os_file_close( fd );
				if( device_id_len >= IOT_ID_MAX_LEN )
					device_id_len = IOT_ID_MAX_LEN;
				device_id[device_id_len] = '\0';
				if ( device_id_len > 0u )
					IOT_LOG( NULL, IOT_LOG_INFO,
						"Device id: %s\n",
						device_id );
			}

			if ( device_id_len == 0u )
			{
				os_uuid_t uuid;
				os_uuid_generate( &uuid );
				os_uuid_to_string_lower( &uuid,
					device_id, IOT_ID_MAX_LEN );
				device_id[IOT_ID_MAX_LEN] = '\0';
				device_id_len = os_strlen( device_id );
				IOT_LOG( NULL, IOT_LOG_INFO,
					"Generated device id: %s", device_id );
				fd = os_file_open( file_path,
					OS_WRITE | OS_CREATE );
				if ( fd  != OS_FILE_INVALID )
				{
					device_id_len = os_file_write(
						device_id, sizeof( char ),
						device_id_len, fd );
					os_file_close( fd );
				}
				else
				{
					IOT_LOG( NULL, IOT_LOG_ERROR,
						"Failed to create file: %s",
						file_path );
					device_id_len = 0u;
				}
			}

			if ( device_id_len > 0u )
			{
#ifdef IOT_STACK_ONLY
				lib->device_id = lib->_device_id;
#else
				lib->device_id = os_realloc( lib->device_id,
					device_id_len + 1u );
				if ( lib->device_id )
#endif /* ifdef IOT_STACK_ONLY */
				{
					os_strncpy( lib->device_id, device_id, device_id_len );
					lib->device_id[ device_id_len ] = '\0';
					result = IOT_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}

iot_status_t iot_disconnect(
	iot_t *lib,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
#ifndef IOT_NO_THREAD_SUPPORT
		/* kill process loop */
		if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
			iot_loop_stop( lib, IOT_FALSE );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

		result = iot_plugin_perform( lib, NULL, &max_time_out,
			IOT_OPERATION_CLIENT_DISCONNECT, NULL, NULL, NULL );
	}
	return result;
}

const char *iot_error( iot_status_t code )
{
	unsigned int i;
	const char *result = NULL;

	/* find error message */
	if ( code == IOT_STATUS_FAILURE )
	{
		int error_number = os_system_error_last();
		if ( error_number != 0 )
			result = os_system_error_string( error_number );
	}

	for ( i = 0u; result == NULL && i <
		((unsigned int) sizeof( IOT_STATUS_MESSAGES ) /
			sizeof( iot_status_msg_t ) ); ++i )
	{
		const iot_status_msg_t *const err =
			&IOT_STATUS_MESSAGES[i];
		if ( err->id == code )
			result = err->msg;
	}

	if ( result == NULL )
		result = "unknown error";
	return result;
}

const char *iot_id( const iot_t *lib )
{
	const char *result = NULL;
	if ( lib )
		result = lib->id;
	return result;
}

iot_t *iot_initialize(
	const char *id,
	const char *cfg_path,
	unsigned int flags )
{
	iot_t *result = NULL;
	if ( id )
	{
		size_t len = IOT_ID_MAX_LEN;
#ifdef IOT_STACK_ONLY
		result = &IOT_LIB;
#else /* ifdef IOT_STACK_ONLY */
		len = os_strlen( id );
		result = (iot_t*)os_malloc( sizeof( struct iot ) +
			(sizeof( char ) * (len + 1u)) );
		if ( result )
#endif /* else IOT_STACK_ONlY */
		{
			unsigned int i;
			os_memzero( result, sizeof( struct iot ) );

			for ( i = 0u; i < IOT_PLUGIN_MAX; ++i )
				result->plugin_ptr[i] = &result->plugin[i];

			/* initialize data structures */
			for ( i = 0u; i < IOT_ACTION_STACK_MAX; ++i )
				result->action_ptr[i] = &result->action[i];
			for ( i = 0u; i < IOT_ALARM_STACK_MAX; ++i )
				result->alarm_ptr[i] = &result->alarm[i];
			for ( i = 0u; i < IOT_TELEMETRY_STACK_MAX; ++i )
				result->telemetry_ptr[i] = &result->telemetry[i];

			/* setup queue for handling requests */
			for ( i = 0u; i < IOT_ACTION_QUEUE_MAX; ++i )
				result->request_queue_free[i] = &result->request_queue[i];

			result->logger_level = IOT_LOG_INFO;
			if ( iot_configuration_file_set( result, cfg_path )
				!= IOT_STATUS_NO_MEMORY )
			{
				result->flags = (iot_uint8_t)flags;
#ifdef IOT_NO_THREAD_SUPPORT
				result->flags |= IOT_FLAG_SINGLE_THREAD;
#else /* ifndef IOT_NO_THREAD_SUPPORT */
				os_thread_mutex_create( &result->log_mutex );
				os_thread_mutex_create( &result->telemetry_mutex );
				os_thread_mutex_create( &result->alarm_mutex );
				os_thread_mutex_create( &result->worker_mutex );
				os_thread_condition_create( &result->worker_signal );
				os_thread_rwlock_create( &result->worker_thread_exclusive_lock );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

				/*os_socket_initialize();*/
#ifdef IOT_STACK_ONLY
				result->id = result->_id;
#else /* ifdef IOT_STACK_ONLY */
				result->id = (char*)result + sizeof(struct iot);
#endif /* else IOT_STACK_ONLY */
				/* setup the app (or client) id */
				os_strncpy( result->id, id, len );
				result->id[len] = '\0';

				if ( iot_base_device_id_set( result )
					!= IOT_STATUS_SUCCESS )
				{
#ifndef IOT_STACK_ONLY
					os_free( result );
#endif /* ifndef IOT_STACK_ONLY */
					result = NULL;
				}
				else
				{
					iot_plugin_builtin_load( result, IOT_PLUGIN_MAX );

					/* initialize all builtin plug-ins */
					for ( i = 0u; i < result->plugin_count; ++i )
						iot_plugin_initialize( result,
							result->plugin_ptr[i] );

					/* enable some builtin plugins */
					iot_plugin_builtin_enable( result );
				}
			}
		}
	}
	return result;
}

iot_status_t iot_log( iot_t *lib, iot_log_level_t log_level,
	const char *function_name, const char *file_name,
	unsigned int line_number, const char *log_msg_fmt, ... )
{

	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && log_level < IOT_LOG_ALL )
	{
		if ( lib->logger )
		{
			const char *file_only = NULL;
			char log_msg[IOT_LOG_MSG_MAX];
			struct iot_log_source source_info =
				{ file_name, function_name, line_number };
			va_list v_args;

			/* split path way from source file name */
			if ( file_name )
			{
				file_only = os_strrchr(
					file_name, OS_DIR_SEP );
				if ( file_only )
					source_info.file_name = file_only + 1u;
			}

			/* lock mutex to ensure safe logging between threads */
#			ifndef IOT_NO_THREAD_SUPPORT
			os_thread_mutex_lock( &lib->log_mutex );
#			endif /* ifndef IOT_NO_THREAD_SUPPORT */

			/* build log message */
			va_start( v_args, log_msg_fmt );
			os_vsnprintf( log_msg, IOT_LOG_MSG_MAX,
				log_msg_fmt, v_args );
			va_end( v_args );

			if ( log_level <= lib->logger_level )
				(*lib->logger)( log_level, &source_info, log_msg,
					lib->logger_user_data );

			/* unlock mutex if we locked it */
#			ifndef IOT_NO_THREAD_SUPPORT
			os_thread_mutex_unlock( &lib->log_mutex );
#			endif /* ifndef IOT_NO_THREAD_SUPPORT */
		}
		/* return success even if logger is not set */
		result = IOT_STATUS_SUCCESS;
	}
	else if ( log_level <= IOT_LOG_INFO )
	{
		va_list v_args;
		va_start( v_args, log_msg_fmt );
		os_vfprintf( OS_STDERR, log_msg_fmt, v_args );
		os_fprintf( OS_STDERR, "\n" );
		va_end( v_args );
	}
	return result;
}

iot_status_t iot_log_callback_set(
	iot_t *lib,
	iot_log_callback_t *log_callback,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		lib->logger = log_callback;
		lib->logger_user_data = user_data;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_log_level_set(
	iot_t *lib,
	iot_log_level_t level )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && level <= IOT_LOG_ALL )
	{
		lib->logger_level = level;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_log_level_set_string(
	iot_t *lib,
	const char *log_level_str )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lib && log_level_str != NULL )
	{
		iot_bool_t log_level_found = IOT_FALSE;
		iot_uint16_t log_level_index = 0;

		result = IOT_STATUS_NOT_FOUND;
		while( !log_level_found && log_level_index <
			sizeof(IOT_LOG_LEVEL_MAP)/sizeof(const char * ) )
		{
			if ( os_strcasecmp( log_level_str,
				IOT_LOG_LEVEL_MAP[log_level_index] ) == 0)
			{
				result = iot_log_level_set( lib,
					(iot_log_level_t)log_level_index );
				log_level_found = IOT_TRUE;
			}
			log_level_index++;
		}
	}
	return result;
}

iot_status_t iot_loop_forever( iot_t *lib )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		result = IOT_STATUS_SUCCESS;
		while( result == IOT_STATUS_SUCCESS &&
			lib->to_quit == IOT_FALSE )
			result = iot_loop_iteration( lib, 1000u );
	}
	return result;
}

iot_status_t iot_loop_iteration( iot_t *lib, iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		result = iot_plugin_perform( lib, NULL, &max_time_out,
			IOT_OPERATION_ITERATION, NULL, NULL, NULL );

		if ( result == IOT_STATUS_SUCCESS
#ifndef IOT_NO_THREAD_SUPPORT
			&& ( lib->flags & IOT_FLAG_SINGLE_THREAD )
#endif /* ifndef IOT_NO_THEAD_SUPPORT */
			)
		{
			printf( "iteration!!!\n" );
			/* if this is single-threaded (ie. no worker threads)
			 * then any action requests must be processed in the
			 * main thread */
			result = iot_action_process( lib, max_time_out );
		}
	}
	return result;
}

iot_status_t iot_loop_start( iot_t *lib )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
#ifdef IOT_NO_THREAD_SUPPORT
		result = IOT_STATUS_NOT_SUPPORTED;
#else
		if ( lib->flags & IOT_FLAG_SINGLE_THREAD )
			result = IOT_STATUS_NOT_SUPPORTED;
		else if ( lib->main_thread == 0 )
		{
			size_t i;
			os_status_t os_result;
			result = IOT_STATUS_FAILURE;
			os_result = os_thread_create( &lib->main_thread,
				iot_base_main_thread, lib );
			for ( i = 0u; os_result == OS_STATUS_SUCCESS  &&
				i < IOT_WORKER_THREADS; ++i )
				os_result = os_thread_create(
					&lib->worker_thread[i],
					iot_base_worker_thread_main, lib );
			if ( os_result == OS_STATUS_SUCCESS )
				result = IOT_STATUS_SUCCESS;
		}
		else
			result = IOT_STATUS_SUCCESS;
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
		if ( result == IOT_STATUS_SUCCESS ||
			result == IOT_STATUS_NOT_SUPPORTED )
			lib->to_quit = IOT_FALSE;
	}
	return result;
}

iot_status_t iot_loop_stop( iot_t *lib, iot_bool_t force )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		lib->to_quit = IOT_TRUE;
#ifdef IOT_NO_THREAD_SUPPORT
		(void)force;
		result = IOT_STATUS_NOT_SUPPORTED;
#else
		if ( lib->flags & IOT_FLAG_SINGLE_THREAD )
			result = IOT_STATUS_NOT_SUPPORTED;
		else
		{
			size_t i;
			if ( lib->main_thread != 0 )
			{
				if ( force == IOT_FALSE )
					os_thread_wait(
						&lib->main_thread );
				else
					os_thread_destroy(
						&lib->main_thread );
				/* set to 0, in case this is called again */
				lib->main_thread = 0;
			}

			/* signal all worker threads to wake up */
			os_thread_condition_broadcast(
				&lib->worker_signal );
			for ( i = 0u; i < IOT_WORKER_THREADS; ++i )
			{
				if ( lib->worker_thread[i] != 0 )
				{
					if ( force == IOT_FALSE )
						os_thread_wait(
							&lib->worker_thread[i] );
					else
						os_thread_destroy(
							&lib->worker_thread[i] );
					/* set to 0, in case this is called again */
					lib->worker_thread[i] = 0;
				}
			}
			result = IOT_STATUS_SUCCESS;
		}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
	}
	return result;
}

iot_status_t iot_configuration_file_set(
	iot_t *lib, const char *file_path )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && file_path )
	{
		const size_t len = os_strlen( file_path );
		if ( len <= PATH_MAX )
		{
#ifdef IOT_STACK_ONLY
			lib->cfg_file_path = lib->_cfg_file_path;
#else
			lib->cfg_file_path = os_realloc(
				lib->cfg_file_path,
				sizeof(char) * (len + 1u) );
#endif
			if ( lib->cfg_file_path )
			{
				os_strncpy( lib->cfg_file_path,
					file_path, len );
				lib->cfg_file_path[len] = '\0';
				result = IOT_STATUS_SUCCESS;
			}
			else
			{
				IOT_LOG( lib, IOT_LOG_ERROR, "%s",
					"not enough memory to store connect "
					"configuration file path");
				result = IOT_STATUS_NO_MEMORY;
			}
		}
	}
	return result;
}

iot_status_t iot_terminate(
	iot_t *lib,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		size_t idx;
		iot_uint8_t i;
#ifndef IOT_STACK_ONLY
		/* free memory allocated for samples */
		while ( lib->telemetry_count )
		{
			struct iot_telemetry *const telemetry =
				lib->telemetry_ptr[lib->telemetry_count - 1u];

			if ( iot_telemetry_free( telemetry, max_time_out )
				!= IOT_STATUS_SUCCESS )
				--lib->telemetry_count;
		}

		while ( lib->action_count )
		{
			struct iot_action *const action =
				lib->action_ptr[lib->action_count - 1u];
			if ( iot_action_free( action, max_time_out )
				!= IOT_STATUS_SUCCESS )
				--lib->action_count;
		}

		while ( lib->alarm_count )
		{
			struct iot_alarm *const alarm =
				lib->alarm_ptr[lib->alarm_count - 1u];

			if ( iot_alarm_deregister( alarm )
				!= IOT_STATUS_SUCCESS )
				--lib->alarm_count;
		}

		/* free memory allocated for each option */
		for ( i = 0u; i < lib->options_count; ++i )
		{
			iot_uint8_t j;
			struct iot_options *const options = lib->options[i];
			for ( j = 0u; options && j < options->option_count; ++j )
			{
				struct iot_option *const option = &options->option[j];
				if ( option )
				{
					os_free_null( (void**)&option->data.heap_storage );
					os_free_null( (void**)&option->name );
				}
			}
			if ( options )
			{
				options->option_count = 0u;
				os_free_null( (void**)&options->option );
			}
			os_free_null( (void **)&lib->options[i] );
		}
		os_free_null( (void **)&lib->options );
		lib->options_count = 0u;
#endif /* ifndef IOT_STACK_ONLY */

		if( !lib->to_quit )
			iot_disconnect( lib, max_time_out );
		/*os_socket_terminate();*/

		/* set lib pointers to NULL */
		for ( idx = 0u; idx < lib->action_count; ++idx )
			lib->action_ptr[idx]->lib = NULL;
		for ( idx = 0u; idx < lib->alarm_count; ++idx )
			lib->alarm_ptr[idx]->lib = NULL;
		for ( idx = 0u; idx < lib->telemetry_count; ++idx )
			lib->telemetry_ptr[idx]->lib = NULL;

		/* disable all plugins */
		iot_plugin_disable_all( lib );

		/* terminate all plug-ins */
		for ( i = (iot_uint8_t)lib->plugin_count; i > 0; --i )
			iot_plugin_terminate( lib, &lib->plugin[i - 1u] );
		result = IOT_STATUS_SUCCESS;

#ifndef IOT_NO_THREAD_SUPPORT
		os_thread_mutex_destroy( &lib->log_mutex );
		os_thread_mutex_destroy( &lib->telemetry_mutex );
		os_thread_mutex_destroy( &lib->alarm_mutex );
		os_thread_mutex_destroy( &lib->worker_mutex );
		os_thread_condition_destroy( &lib->worker_signal );
		os_thread_rwlock_destroy(
			&lib->worker_thread_exclusive_lock );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

#ifndef IOT_STACK_ONLY
		os_free_null( (void**)&lib->cfg_file_path );
		os_free_null( (void**)&lib->device_id );
		os_free( lib );
#endif /* ifdef IOT_STACK_ONLY */
	}
	return result;
}

iot_timestamp_t iot_timestamp_now( void )
{
	iot_timestamp_t time_stamp = 0u;
	os_time( &time_stamp, NULL );
	return time_stamp;
}

#if 0
iot_bool_t iot_transaction_status(
	const iot_transaction_t *txn,
	iot_millisecond_t UNUSED(max_time_out) )
{
	iot_bool_t result = IOT_FALSE;
	if ( txn )
	{
		size_t i;
		result = IOT_TRUE;
		for ( i = 0u; i < IOT_PROTOCOL_STACKS && result != IOT_FALSE;
			++i )
		{
			iot_status_t txn_status = txn->status[i];
			if ( txn_status != IOT_STATUS_SUCCESS &&
				txn_status != IOT_STATUS_NOT_SUPPORTED )
				result = IOT_FALSE;
		}
	}
	return result;
}
#endif

iot_version_t iot_version( void )
{
	return iot_version_encode( IOT_VERSION_MAJOR, IOT_VERSION_MINOR,
		IOT_VERSION_PATCH, IOT_VERSION_TWEAK );
}

const char *iot_version_str( void )
{
	return IOT_VERSION;
}
