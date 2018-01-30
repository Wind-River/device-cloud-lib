/**
 * @file
 * @brief Main source file for the Wind River IoT control configuration files
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
#include "control_config.h"

#include "iot-connect.schema.json.h"
#include "iot_build.h"
#include "os.h"

#include <stdarg.h>                    /* for va_list, va_start, va_end */
#include <stdlib.h>                    /* for EXIT_FAILURE */

/**
 * @brief Handles obtaining values for a JSON array when required by a schema
 *
 * @param[in]      encoder             json encoder
 * @param[in]      schema              schema generating request
 * @param[in]      obj                 current item in the schema
 * @param[in]      key                 (optional) key of parent object
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      invalid schema
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t control_config_schema_array(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set );

/**
 * @brief Handles obtaining values for a JSON boolean when required by a schema
 *
 * @param[in]      encoder             json encoder
 * @param[in]      schema              schema generating request
 * @param[in]      obj                 current item in the schema
 * @param[in]      key                 (optional) key of parent object
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      invalid schema
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t control_config_schema_bool(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set );

/**
 * @brief Handles obtaining values for a JSON object when required by a schema
 *
 * @param[in]      encoder             json encoder
 * @param[in]      schema              schema generating request
 * @param[in]      obj                 current item in the schema
 * @param[in]      key                 (optional) key of parent object
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      invalid schema
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t control_config_schema_object(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	size_t key_len,
	iot_bool_t *value_set );

/**
 * @brief Handles obtaining values from user input when required by a schema
 *
 * @param[in]      schema              schema generating request
 * @param[in]      obj                 current item in the schema
 * @param[in]      key                 (optional) key of parent object
 * @param[in]      show_user_input     whether or not to echo user input
 * @param[out]     out                 output retrieved from the user
 * @param[in]      out_len             length of the output buffer
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      invalid schema
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t control_config_schema_input(
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t show_user_input,
	char *out,
	size_t out_len,
	iot_bool_t *value_set );

/**
 * @brief Handles obtaining values for a JSON integer when required by a schema
 *
 * @param[in]      encoder             json encoder
 * @param[in]      schema              schema generating request
 * @param[in]      obj                 current item in the schema
 * @param[in]      key                 (optional) key of parent object
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t control_config_schema_integer(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set );

/**
 * @brief Handles obtaining values for a JSON real number when required by a schema
 *
 * @param[in]      encoder             json encoder
 * @param[in]      schema              schema generating request
 * @param[in]      obj                 current item in the schema
 * @param[in]      key                 (optional) key of parent object
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t control_config_schema_real(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set );

/**
 * @brief Handles obtaining values for a JSON string when required by a schema
 *
 * @param[in]      encoder             json encoder
 * @param[in]      schema              schema generating request
 * @param[in]      obj                 current item in the schema
 * @param[in]      key                 (optional) key of parent object
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t control_config_schema_string(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set );

/**
 * @brief User interface for prompting user for input
 *
 * @param[in,out]  encoder             json encoder building configuration file
 * @param[out]     value_set           (optional) whether a value was set
 *
 * @retval IOT_STATUS_NO_MEMORY        not enough memory available
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t control_config_user_interface(
	iot_json_encoder_t *encoder,
	iot_bool_t *value_set );

/**
 * @brief Helper function to display a prompt to the user
 *
 * @param[in,out]  out                 destination buffer
 * @param[in]      len                 size of destination buffer
 * @param[in]      show_user_input     show what user is typing
 * @param[in]      prompt              prompt for user in printf format
 * @param[in]      ...                 variable arguments based on @c prompt
 */
static IOT_SECTION void control_config_user_prompt(
	char *out,
	size_t len,
	iot_bool_t show_user_input,
	const char *prompt, ... ) __attribute__((format(printf,4,5)));

iot_status_t control_config_generate( void )
{
	iot_json_encoder_t *encoder;
	iot_status_t result = IOT_STATUS_NO_MEMORY;

#ifdef IOT_STACK_ONLY
	char buffer[1024u];
	encoder = iot_json_encode_initialize( buffer, 1024u,
		IOT_JSON_FLAG_INDENT(2) | IOT_JSON_FLAG_EXPAND );
#else
	encoder = iot_json_encode_initialize( NULL, 0u, IOT_JSON_FLAG_DYNAMIC |
		IOT_JSON_FLAG_INDENT(2) | IOT_JSON_FLAG_EXPAND );
#endif
	if ( encoder )
	{
		iot_bool_t value_set = IOT_FALSE;
		result = control_config_user_interface( encoder, &value_set );

		if ( result == IOT_STATUS_SUCCESS && value_set == IOT_TRUE )
		{
			char config_file[ PATH_MAX + 1u ];
			os_file_t connection_file;

			/* generate connection configuration file */
			/** @todo fix this to use API for configuration directory */
			os_snprintf( config_file,
				PATH_MAX, "%s%c%s%s",
				IOT_DEFAULT_DIR_CONFIG, OS_DIR_SEP,
				IOT_DEFAULT_FILE_CONFIG,
				IOT_DEFAULT_FILE_CONFIG_EXT );

			if ( os_file_exists( config_file ) )
				os_file_delete( config_file );

			connection_file = os_file_open( config_file,
				OS_WRITE | OS_CREATE );

			if ( connection_file )
			{
				os_printf(
					"Wrote configuration to file (%s)...\n",
					config_file );
				os_fprintf( connection_file, "%s",
					iot_json_encode_dump( encoder ) );
				os_file_close( connection_file );
			}
			else
			{
				os_fprintf( OS_STDERR,
					"Failed to write configuration file (%s)... %s\n",
					config_file, iot_error( IOT_STATUS_FAILURE ) );
				result = EXIT_FAILURE;
			}
		}

		iot_json_encode_terminate( encoder );
	}
	return result;
}

iot_status_t control_config_schema_object(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	size_t key_len,
	iot_bool_t *value_set )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( schema && obj )
	{
		iot_json_type_t t;
		char *k = NULL;
		t = iot_json_schema_type( schema, obj );

		/* null-terminate the item key */
		result = IOT_STATUS_SUCCESS;
		if ( t != IOT_JSON_TYPE_OBJECT || key_len > 0u )
		{
			result = IOT_STATUS_NO_MEMORY;
			k = (char*)os_malloc( key_len + 1u );
			if ( k )
			{
				os_strncpy( k, key, key_len );
				k[key_len] = '\0';
				result = IOT_STATUS_SUCCESS;
			}
		}

		if ( result == IOT_STATUS_SUCCESS )
		{
			switch ( t )
			{
			case IOT_JSON_TYPE_ARRAY:
			{
				result = control_config_schema_array( encoder, schema,
					obj, k, value_set );
				break;
			}
			case IOT_JSON_TYPE_OBJECT:
			{
				char **set_items = NULL;
				size_t set_items_count = 0u;
				iot_bool_t any_val_set = IOT_FALSE;

				iot_json_schema_object_iterator_t *iter =
					iot_json_schema_object_iterator( schema, obj );

				iot_json_encode_object_start( encoder, k );
				while ( iter )
				{
					iot_json_schema_item_t *item = NULL;
					iot_json_schema_object_iterator_key(
						schema, obj, iter, &key, &key_len );
					iot_json_schema_object_iterator_value(
						schema, obj, iter, &item );

					/* only show option if dependencies are met */
					if ( iot_json_schema_dependencies_achieved(
						schema, item, (const char* const*)set_items, set_items_count  ) != IOT_FALSE )
					{
						iot_bool_t val_set = IOT_FALSE;
						control_config_schema_object( encoder,
							schema, item, key, key_len, &val_set );

						/* if item was set a value */
						if ( val_set != IOT_FALSE )
						{
							char *new_item;
							new_item = os_malloc( key_len + 1u );
							os_strncpy( new_item, key, key_len );
							new_item[key_len] = '\0';
							set_items = os_realloc( set_items,
								sizeof(char*) * (set_items_count + 1u) );
							set_items[ set_items_count ] = new_item;
							++set_items_count;

							any_val_set = val_set;
							if ( value_set )
								*value_set = val_set;
						}
					}
					iter = iot_json_schema_object_iterator_next(
						schema, obj, iter );
				}

				if ( any_val_set == IOT_FALSE )
					iot_json_encode_object_cancel( encoder );
				else
					iot_json_encode_object_end( encoder );

				if ( set_items )
				{
					size_t i;
					for ( i = 0u; i < set_items_count; ++i )
						if ( set_items[i] )
							os_free( set_items[i] );
					os_free( set_items );
				}
				break;
			}
			case IOT_JSON_TYPE_BOOL:
				result = control_config_schema_bool( encoder,
					schema, obj, k, value_set );
				break;
			case IOT_JSON_TYPE_INTEGER:
				result = control_config_schema_integer( encoder,
					schema, obj, k, value_set );
				break;
			case IOT_JSON_TYPE_REAL:
				result = control_config_schema_real( encoder,
					schema, obj, k, value_set );
				break;
			case IOT_JSON_TYPE_STRING:
				result = control_config_schema_string( encoder,
					schema, obj, k, value_set );
				break;
			case IOT_JSON_TYPE_NULL:
			default:
				result = IOT_STATUS_BAD_REQUEST;
				break;
			}
		}

		if ( k )
			os_free( k );
	}
	return result;
}

iot_status_t control_config_schema_array(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( key )
	{
		/* must get array user input here */
		result = IOT_STATUS_BAD_REQUEST;
		if ( iot_json_schema_array( schema, obj, NULL, 0u, NULL ) != IOT_FALSE )
		{
			iot_json_encode_array_start( encoder, key );
			if ( value_set )
				*value_set = IOT_TRUE;
			/* todo add items here */
			iot_json_encode_array_end( encoder );
		}
	}
	return result;
}

iot_status_t control_config_schema_bool(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( key )
	{
		while ( result != IOT_STATUS_SUCCESS )
		{
			const char *error_msg = "value required";
			char input[10u];
			result = control_config_schema_input( schema, obj, key,
				IOT_TRUE, input, 10u, value_set );
			if ( result == IOT_STATUS_SUCCESS &&
				iot_json_schema_bool( schema, obj, input,
					os_strlen( input ), &error_msg ) != IOT_FALSE )
			{
				/* get user input */
				iot_bool_t value = IOT_TRUE;
				if ( input[0] == 'n' || input[0] == 'N' ||
					input[0] == 'f' || input[0] == 'F' ||
					input[0] == '0' ||
					(input[0] == 'o' && input[1] == 'f') ||
					(input[0] == 'O' && input[1] == 'F') ||
					(input[0] == 'O' && input[1] == 'f') ||
					(input[0] == 'o' && input[1] == 'F') )
					value = IOT_FALSE;
				result = iot_json_encode_bool(
					encoder, key, value );
			}
			else
				result = IOT_STATUS_BAD_REQUEST;

			if ( result != IOT_STATUS_SUCCESS )
				os_fprintf( OS_STDERR, "Error: %s\n",
					error_msg );
		}
	}
	return result;
}

iot_status_t control_config_schema_input(
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t show_user_input,
	char *out,
	size_t out_len,
	iot_bool_t *value_set )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( schema && obj && out && out_len > 0u )
	{
		const char *desc = NULL;
		size_t desc_len = 0u;
		const char *title = NULL;
		size_t title_len = 0u;

		iot_json_schema_description( schema, obj, &desc, &desc_len );
		iot_json_schema_title( schema, obj, &title, &title_len );

		/* set the title, if not explicitly set */
		if ( !title )
		{
			title = key;
			title_len = os_strlen( key );
		}

		out[0] = '\0';
		if ( desc && desc_len > 0u )
		{
			os_printf( "%.*s: %.*s\n",
				(int)title_len, title,
				(int)desc_len, desc );
		}

		/* for arrays we must provide a loop for multiple inputs */
		result = IOT_STATUS_BAD_REQUEST;
		control_config_user_prompt( out, out_len, show_user_input,
			"Enter a value for %.*s:\n",
				(int)title_len, title );

		if ( out[0] != '\0' && value_set )
			*value_set = IOT_TRUE;

		if ( out[0] != '\0' ||
			iot_json_schema_required( schema, obj ) == IOT_FALSE )
		{
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t control_config_schema_integer(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( key )
	{
		while ( result != IOT_STATUS_SUCCESS )
		{
			char input[25u];
			const char *error_msg = "value required";
			size_t in_len;
			iot_int64_t value = 0;
			result = control_config_schema_input( schema, obj, key,
				IOT_TRUE, input, 25u, value_set );

			in_len = os_strlen( input );

			if ( result == IOT_STATUS_SUCCESS &&
				iot_json_schema_integer( schema, obj, input, in_len,
					&error_msg ) != IOT_FALSE )
			{
				value = atol( input );
				result = iot_json_encode_integer( encoder,
					key, value );
			}
			else if ( result == IOT_STATUS_SUCCESS )
				result = IOT_STATUS_BAD_REQUEST;

			if ( result != IOT_STATUS_SUCCESS )
				os_fprintf( OS_STDERR, "Error: %s\n", error_msg );
		}
	}
	return result;
}

iot_status_t control_config_schema_real(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( key )
	{
		while ( result != IOT_STATUS_SUCCESS )
		{
			char input[125u];
			const char *error_msg = "value required";
			result = control_config_schema_input( schema, obj, key,
				IOT_TRUE, input, 125u, value_set );

			if ( result == IOT_STATUS_SUCCESS &&
				iot_json_schema_real( schema, obj, input,
					os_strlen( input ),
					&error_msg ) != IOT_FALSE )
			{
				iot_float64_t value = 0.0;
				if ( sscanf( input, "%lf", &value ) == EOF )
				{
					error_msg = "invalid number";
					result = IOT_STATUS_BAD_REQUEST;
				}
				else
				{
					result = iot_json_encode_real( encoder,
						key, value );
				}
			}

			if ( result != IOT_STATUS_SUCCESS )
				os_fprintf( OS_STDERR, "Error: %s\n", error_msg );
		}
	}
	return result;
}

iot_status_t control_config_schema_string(
	iot_json_encoder_t *encoder,
	iot_json_schema_t *schema,
	iot_json_schema_item_t *obj,
	const char *key,
	iot_bool_t *value_set )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( key )
	{
		const char *format = NULL;
		size_t format_len = 0u;
		iot_bool_t show_user_input = IOT_TRUE;

		iot_json_schema_format( schema, obj, &format, &format_len );
		if ( format && os_strncmp( format, "password", format_len ) == 0 )
			show_user_input = IOT_FALSE;

		while ( result != IOT_STATUS_SUCCESS )
		{
			char input[256u];
			const char *error_msg = "value required";

			result = control_config_schema_input( schema, obj, key,
				show_user_input, input, 256u, value_set );

			if ( result == IOT_STATUS_SUCCESS &&
				iot_json_schema_string( schema, obj, input,
					os_strlen( input ), &error_msg ) )
			{
				if ( os_strlen( input ) > 0 )
				{
					result = iot_json_encode_string(
						encoder, key, input );
				}
			}
			else
				result = IOT_STATUS_BAD_REQUEST;

			if ( result != IOT_STATUS_SUCCESS )
				os_fprintf( OS_STDERR, "Error: %s\n",
					error_msg );
		}
	}
	return result;
}

iot_status_t control_config_user_interface(
	iot_json_encoder_t *encoder,
	iot_bool_t *value_set )
{
	iot_status_t result;
	iot_json_schema_t *schema;
	iot_json_schema_item_t *root = NULL;
	char json_error[ 256u ];

#ifdef IOT_STACK_ONLY
	char buffer[4096u];
	schema = iot_json_schema_initialize( buffer, sizeof(buffer), 0u );
#else
	schema = iot_json_schema_initialize( NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
#endif

	result = iot_json_schema_parse( schema, IOT_CONNECT_SCHEMA,
		os_strlen( IOT_CONNECT_SCHEMA ), &root, json_error, 256u );

	if ( result == IOT_STATUS_SUCCESS && root )
	{
		result = control_config_schema_object( encoder,
			schema, root, NULL, 0u, value_set );
	}
	else
		os_fprintf( OS_STDERR, "Failed to parse JSON schema: %s\n",
			json_error );

	iot_json_schema_terminate( schema );

	return result;
}

void control_config_user_prompt(
	char *out,
	size_t len,
	iot_bool_t show_user_input,
	const char *prompt, ... )
{
	if ( out )
	{
		size_t i = 0;
		char temp[ PATH_MAX + 1u ];
		os_memzero( temp, sizeof( temp ) );
		os_memzero( out, len );
		if ( prompt )
		{
			va_list args;
			va_start( args, prompt );
			os_vfprintf( OS_STDOUT, prompt, args );
			va_end( args );
			os_flush( OS_STDOUT );
		}
		if ( show_user_input == IOT_FALSE )
			os_stream_echo_set( OS_STDIN, OS_FALSE );

		os_file_gets( temp, PATH_MAX, OS_STDIN );

		if ( show_user_input == IOT_FALSE )
		{
			os_fprintf( OS_STDOUT, "\n" );
			os_stream_echo_set( OS_STDIN, OS_TRUE );
		}

		while ( temp[i] != '\n' && temp[i] != '\r' &&
			temp[i] != '\0' && i < len )
			++i;
		temp[i] = '\0';
		os_strncpy( out, temp, len );
	}
}

