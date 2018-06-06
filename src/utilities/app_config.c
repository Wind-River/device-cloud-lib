/**
 * @file
 * @brief Wind River IoT configuration file reader
 *
 * @copyright Copyright (C) 2016-2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "app_config.h"

/** @brief Structure containing configuration information */
struct app_config
{
	app_json_decoder_t *json;
	const app_json_item_t *json_root;
	const app_json_item_t *json_array;
	const app_json_item_t *json_array_object;
	const app_json_array_iterator_t *json_iterator;
};

/*FIXME*/
/**
 * @brief Helper function to get the path to proxy configuration file
 *
 * @param[out]     file_path           path to connection file
 * @param[in]      len                 length of buffer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameters
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
/* static iot_status_t app_config_get_proxy_file_path( */
/* 	char *file_path, size_t len ); */

iot_status_t app_config_close( struct app_config *config )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config )
	{
		app_json_decode_terminate( config->json );
		os_free( config );
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

struct app_config *app_config_open( const char *file_path )
{
	/** @TODO replace IOT_LOG with app_config_log, or use shared logging function */
	char config_file[ PATH_MAX + 1u ];
	iot_bool_t file_found = IOT_FALSE;
	unsigned int i;
	const char *paths[] = { file_path, NULL, NULL, NULL };
	struct app_config *result = NULL;
	char config_dir[ PATH_MAX + 1u ];
	char runtime_dir[ PATH_MAX + 1u ];
	char current_dir[ PATH_MAX + 1u ];
	char exe_dir[ PATH_MAX + 1u ];

	/* required json init variables */
	app_json_decoder_t *json = NULL;
	char *json_string = NULL;
	size_t json_size = 0;
	const app_json_item_t *json_root = NULL; /* root json object */
	char err_msg[1024u];
	size_t err_len = 1024u ;
	os_file_t fd = NULL;

	/* locate the config file */
	config_file[0] = '\0';
	if ( !file_path || *file_path == '\0' )
	{
		/* if user doesn't specify file_path, check in
		 * config_dir, runtime_dir, current_dir and exe_dir
		 * for default configuration file */
		if ( app_path_directory_name_get( IOT_DIR_CONFIG,
				config_dir, PATH_MAX ) > 0u && config_dir[0] != '\0' )
			paths[0u] = config_dir;
		if ( app_path_directory_name_get( IOT_DIR_RUNTIME,
				runtime_dir, PATH_MAX ) >0u && runtime_dir[0] != '\0' )
			paths[1u] = runtime_dir;
		if ( os_directory_current( current_dir, PATH_MAX )
			== OS_STATUS_SUCCESS && current_dir[0] != '\0' )
			paths[2u] = current_dir;
		if ( (os_status_t) app_path_executable_directory_get( exe_dir, PATH_MAX )
			== OS_STATUS_SUCCESS && exe_dir[0] != '\0' )
			paths[3u] = exe_dir;
	}

	for ( i = 0u; i < sizeof( paths ) / sizeof( const char * ) &&
		paths[i] && file_found == IOT_FALSE; ++i )
	{
		const char *const path = paths[i];
		config_file[0] = '\0';
		if ( path )
		{
			if ( !file_path || *file_path == '\0' )
			{
				os_make_path( config_file, PATH_MAX,
					path, IOT_DEFAULT_FILE_DEVICE_MANAGER, NULL );
			}
			else
				os_strncpy( config_file, path,
					PATH_MAX + 1u );
			os_env_expand( config_file, 0u, PATH_MAX );
		}

		if ( *config_file != '\0' )
		{
			/* IOT_LOG( iot_lib, IOT_LOG_INFO, */
			/* 	"Looking for configuration file: %s", config_file ); */

			if ( os_file_exists( config_file ) )
			{
				fd = os_file_open( config_file, OS_READ );
				if ( fd == NULL )
				{
					 /* for compiler warnings */
					file_found = IOT_FALSE;
					/* IOT_LOG( iot_lib, IOT_LOG_ERROR, */
					/* 	"Cannot read configuration: %s", */
						/* config_file ); */
				}
				else
					file_found = IOT_TRUE;
			}
		}
	}

	if ( file_found != IOT_FALSE )
	{
		/* IOT_LOG( iot_lib, IOT_LOG_INFO, */
		/* 	"Configuration file found: %s", config_file ); */
	
#ifdef IOT_STACK_ONLY
		/* initialize json decoder */
		char buffer[1024u];
		json = app_json_decode_initialize(
			buffer, 1024u, 0u );
#else
		json = app_json_decode_initialize( NULL, 0u,
			APP_JSON_FLAG_DYNAMIC );
#endif
		result = os_malloc(
				sizeof( struct app_config ) );
		json_size = (size_t)os_file_size( config_file );
		if ( result && json && json_size )
		{
			iot_status_t status = IOT_STATUS_NO_MEMORY ;
			os_memzero( result,
				sizeof( struct app_config ) );
			json_string = (char *)os_malloc( json_size + 1 );
			if ( json_string )
			{
				json_size = os_file_read( json_string, 1,
					json_size, fd );
				json_string[json_size] = '\0';
				if ( json_size > 0 )
				{
					status = app_json_decode_parse( json,
						json_string,
						json_size, &json_root, err_msg, err_len  );
				}
			}

			if ( status != IOT_STATUS_SUCCESS )
			{
				/* IOT_LOG( iot_lib, IOT_LOG_ERROR, */
				/* 	"Error loading configuration file %s; " */
					/* "app_json_decode_parse failed : %d", */
					/* config_file, status ); */
				os_free( (void *)result );
				result = NULL;
			}
			else
			{
				result->json = json;
				result->json_root = json_root;
			}
		}
		/* else */
			/* IOT_LOG( iot_lib, IOT_LOG_ERROR, */
			/* 	"Error loading configuration file %s; " */
			/* 	"Unable to allocate sufficient memory", */
				/* config_file ); */
		os_file_close ( fd );
	}
	return result;
}

iot_status_t app_config_read_boolean( struct app_config *config,
	const char *group, const char *field, iot_bool_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config && config->json && config->json_root && field && value )
	{
		app_json_decoder_t * json = config->json;
		const app_json_item_t *json_root = config->json_array_object ?
			config->json_array_object : config->json_root;
		const app_json_item_t *json_bool = NULL;
		if ( group )
		{
			const app_json_item_t *json_group = NULL;
			json_group = app_json_decode_object_find( json,
				json_root, group );
			json_bool = app_json_decode_object_find( json,
				json_group, field );
		}
		else
			json_bool = app_json_decode_object_find( json, json_root, field );

		if ( json_bool )
			result = app_json_decode_bool( json, json_bool, value );
	}
	return result;
}

iot_status_t app_config_read_string( struct app_config *config,
	const char *group, const char *field, const char **value, size_t* str_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config && config->json && config->json_root && field && value )
	{
		app_json_decoder_t * json = config->json;
		/* check if we're in array context */
		const app_json_item_t *json_root = config->json_array_object ?
			config->json_array_object : config->json_root;
		const app_json_item_t *json_str = NULL;
		if ( group )
		{
			const app_json_item_t *json_group = NULL;
			json_group = app_json_decode_object_find( json,
				json_root, group );
			json_str = app_json_decode_object_find( json,
				json_group, field );
		}
		else
			json_str = app_json_decode_object_find( json, json_root, field );

		if (json_str )
			result = app_json_decode_string( json, json_str, value, str_len );
	}
	return result;
}

iot_status_t app_config_read_json_array_start( struct app_config *config,
	const char *field )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	if ( field && config && config->json && config->json_root )
	{
		const app_json_array_iterator_t *j_itr;
		app_json_decoder_t * json = config->json;
		/* check if we're in array context */
		const app_json_item_t *json_root = config->json_array_object ?
			config->json_array_object : config->json_root;
		const app_json_item_t *json_array =  NULL;

		/* get json array object */
		json_array = app_json_decode_object_find( json, json_root, field );
		if ( json_array )
		{
			j_itr = app_json_decode_array_iterator( json, json_array );
			if( j_itr )
			{
				config->json_iterator = j_itr;
				config->json_array = json_array;
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

iot_status_t app_config_read_json_array_end( struct app_config *config )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config && config->json && config->json_root )
	{
		config->json_iterator = NULL;
		config->json_array = NULL;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t app_config_read_json_array_next( struct app_config *config )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	if ( config && config->json && config->json_iterator && config->json_array )
	{
		const app_json_array_iterator_t *j_itr;
		j_itr = app_json_decode_array_iterator_next( config->json,
			config->json_array, config->json_iterator );

		/* check if array is finished */
		/* if its not finished, we return success */
		/* otherwise, we switch the context and return failure */
		if ( j_itr )
		{
			app_json_decode_array_iterator_value( config->json,
				config->json_array, j_itr, &config->json_array_object );
			result = IOT_STATUS_SUCCESS;
			config->json_iterator = j_itr;
		}
		else
		{
			config->json_iterator = NULL;
			config->json_array = NULL;
		}

	}
	return result;
}

iot_status_t app_config_read_integer( struct app_config *config,
	const char *group, const char *field, iot_int64_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( config && config->json && config->json_root && field && value )
	{
		app_json_decoder_t * json = config->json;
		const app_json_item_t *json_root = config->json_array_object ?
			config->json_array_object : config->json_root;
		const app_json_item_t *json_int = NULL;
		if ( group )
		{
			const app_json_item_t *json_group = NULL;
			json_group = app_json_decode_object_find( json,
				json_root, group );
			json_int = app_json_decode_object_find( json,
				json_group, field );
		}
		else
			json_int = app_json_decode_object_find( json, json_root, field );

		if (json_int )
			result = app_json_decode_integer( json, json_int, value );
	}
	return result;
}

/*FIXME*/
/*

iot_status_t app_config_get_proxy_file_path(
	char *path, size_t size )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( path )
	{
		char config_dir[ PATH_MAX + 1u ];
		os_memzero( path, size );
		result = app_path_directory_name_get( JSON_DIR_CONFIG,
			config_dir, PATH_MAX );
		if ( result == IOT_STATUS_SUCCESS )
			os_make_path( path, size, config_dir, JSON_DEFAULT_FILE_PROXY, NULL );
	}
	return result;
}

iot_status_t app_config_read_proxy_file(
	struct iot_proxy *proxy_info )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( proxy_info )
	{
		char file_path[ PATH_MAX + 1u ];

		os_memzero( proxy_info, sizeof( struct iot_proxy ) );
		result = app_config_get_proxy_file_path(
			file_path, sizeof( file_path ) );
		if ( result == IOT_STATUS_SUCCESS &&
			file_path[0] != '\0' && os_file_exists( file_path ) )
		{
			const char *const proxy_group = "proxy";
			struct app_config *const config =
				app_config_open( file_path );
			result = IOT_STATUS_NOT_FOUND;
			if ( config )
			{
				char *host= NULL;
				char *username = NULL ;
				char *password = NULL;

				const char *temp_string = NULL;
				size_t temp_string_len;
				iot_int64_t temp_value = 0u;

				result = app_config_read_string( config,
					proxy_group, "host", &temp_string, &temp_string_len );


				if ( result == IOT_STATUS_SUCCESS &&
					temp_string[0] != '\0' )
				{
					host = os_malloc( temp_string_len + 1 );
					os_strncpy( host, temp_string, temp_string_len );
					host[temp_string_len] = '\0';
					proxy_info->host = host;
					if ( result == IOT_STATUS_SUCCESS )
					{
						result = app_config_read_integer(
							config, proxy_group,
							"port", &temp_value );
						if ( result == IOT_STATUS_SUCCESS )
							proxy_info->port = temp_value;
					}
				}

				if ( result == IOT_STATUS_SUCCESS )
				{
					result = app_config_read_string(
						config, proxy_group, "type",
						&temp_string, &temp_string_len );

					if ( os_strcmp( temp_string, "HTTP" ) == 0 )
						proxy_info->type = JSON_PROXY_HTTP;
					else if ( os_strcmp( temp_string, "SOCKS5" ) )
						proxy_info->type = JSON_PROXY_SOCKS5;
					else
						proxy_info->type = JSON_PROXY_UNKNOWN;
				}

				if ( result == IOT_STATUS_SUCCESS )
				{
					iot_status_t result2;
					result2 = app_config_read_string(
						config, proxy_group,
						"username", &temp_string, &temp_string_len );
					if ( result2 == IOT_STATUS_SUCCESS &&
						temp_string[0] != '\0' )
					{
						username = os_malloc( temp_string_len + 1 );
						os_strncpy( username, temp_string, temp_string_len );
						username[temp_string_len] = '\0';
						proxy_info->username = username;

						result2 = app_config_read_string(
							config, proxy_group,
							"password", &temp_string, &temp_string_len );
						if ( result2 == IOT_STATUS_SUCCESS
							&& temp_string[0] != '\0' )
						{
							password = os_malloc( temp_string_len + 1 );
							os_strncpy( password, temp_string, temp_string_len );
							password[temp_string_len] = '\0';
							proxy_info->password = password;
						}
					}
				}
				app_config_close( config );
			}
		}
	}
	return result;
}
*/

