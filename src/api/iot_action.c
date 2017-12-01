/**
 * @file
 * @brief source file containing functions for handling actions
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

#include "public/iot.h"

#include "iot_common.h"
#include "shared/iot_base64.h"
#include "shared/iot_types.h"

#include <limits.h> /* for CHAR_BIT */
#include <stdarg.h>

/** @brief Maximum size of action command output */
#define IOT_ACTION_COMMAND_OUTPUT_MAX_LEN        1024u
/** @brief Name of the parameter containing the return value for a command */
#define IOT_ACTION_COMMAND_RETVAL                "retval"
/** @brief Name of the parameter containing standard error for a command */
#define IOT_ACTION_COMMAND_STDERR                "stderr"
/** @brief Name of the parameter containing standard out for a command */
#define IOT_ACTION_COMMAND_STDOUT                "stdout"
/** @brief Characters that cannot be used in parameter names */
#define IOT_PARAMETER_NAME_BAD_CHARACTERS        "=\\;&|"

/**
 * @brief Executes the action specified
 *
 * @param[in]      action              action to execute
 * @param[in,out]  request             request from the cloud (updated to send
 *                                     the result back)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      action parameter is of bad type, unknown
 *                                     (i.e. not expected), or required and not
 *                                     specified
 * @retval IOT_STATUS_INVOKED          action executed with no status return
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_EXECUTION_ERROR  specified command is not executable
 * @retval IOT_STATUS_FAILURE          no handler specified for action
 */
static IOT_SECTION iot_status_t iot_action_execute(
	const struct iot_action *action,
	struct iot_action_request *request,
	iot_millisecond_t max_time_out );

/**
 * @brief Executes a system command registered with an action
 *
 * @param[in]      action              ction containing the registered command
 * @param[in,out]  request             request received from the cloud
 * @param[in]      max_time_out        maximum time to wait in milliseconds for
 *                                     execution
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_INVOKED          command executed with no status return
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_EXECUTION_ERROR  failed to execute command
 */
static IOT_SECTION iot_status_t iot_action_execute_command(
	const struct iot_action *action,
	struct iot_action_request *request,
	iot_millisecond_t max_time_out );

/**
 * @brief Deletes all occurances of a word from a string
 *
 * @param[in,out]  command_param       command param need to be adjusted
 * @param[in]      word                the word need to be deleted
 *
 * @return         the number of times word was found
 */
static IOT_SECTION size_t iot_action_parameter_adjustment(
	char *command_param,
	const char *word );

/**
 * @brief Sets the value of an action option
 *
 * @param[in,out]  action              action object to set
 * @param[in]      name                option name
 * @param[in]      data                option value to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STAUTS_FULL             maximum number of options reached
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_action_option_set_data(
	iot_action_t *action,
	const char *name,
	const struct iot_data *data );

/**
 * @brief Sets the value of an action request option
 *
 * @param[in,out]  request             action request object to set
 * @param[in]      name                option name
 * @param[in]      data                option value to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STAUTS_FULL             maximum number of options reached
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t iot_action_request_option_set_data(
	iot_action_request_t *request,
	const char *name,
	const struct iot_data *data );

/**
 * @brief Sets a parameter value for an action request to be executed
 *
 * @param[in,out]  request             action request
 * @param[in]      name                parameter name
 * @param[in]      type                data type
 * @param[in]      args                argument containing data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 * @retval IOT_STATUS_FULL             no space to store new parameter
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_request_parameter_set
 */
static IOT_SECTION iot_status_t iot_action_request_parameter_set_args(
	iot_action_request_t *request,
	const char *name,
	iot_type_t type,
	va_list args );


/**
 * @brief Sets a parameter value for an action request to be executed
 *
 * @param[in,out]  request             action request
 * @param[in]      status              status to be set
 * @param[in]      err_msg_fmt         error message to set (in printf format)
 * @param[in]      ...                 arguments to add to error message
 *
 * @see iot_action_request_status
 */
static IOT_SECTION void iot_action_request_set_status(
	struct iot_action_request *request,
	iot_status_t status,
	const char *err_msg_fmt, ... )
	__attribute__((format(printf,3,4)));


iot_action_t *iot_action_allocate(
	iot_t *lib,
	const char *name )
{
	struct iot_action *result = NULL;
	if ( lib && name && *name != '\0' )
	{
		if ( lib->action_count < IOT_ACTION_MAX )
		{
			const unsigned int count = lib->action_count;
#ifndef IOT_STACK_ONLY
			iot_bool_t is_in_heap = IOT_FALSE;
#endif /* ifndef IOT_STACK_ONLY */

			/* look for free action in stack */
			result = lib->action_ptr[count];

#ifndef IOT_STACK_ONLY
			/* allocate action in heap if none is available in stack */
			if ( !result )
			{
				result = (struct iot_action *)os_malloc(
					sizeof( struct iot_action ) );
				is_in_heap = IOT_TRUE;
			}
#endif /* ifndef IOT_STACK_ONLY */

			if ( result )
			{
				unsigned int cur_idx = 0u;
				unsigned int min_idx = 0u;
				unsigned int max_idx = count;
				size_t name_len;

				os_memzero( result, sizeof( struct iot_action ) );

				name_len = os_strlen( name );
				if ( name_len > IOT_NAME_MAX_LEN )
					name_len = IOT_NAME_MAX_LEN;
#ifdef IOT_STACK_ONLY
				result->option = result->_option;
				result->name = result->_name;
				result->parameter = result->_parameter;
#else /* ifdef IOT_STACK_ONLY */
				result->name = os_malloc( name_len + 1u );
#endif /* else IOT_STACK_ONLY */
				if ( result->name )
				{
					os_strncpy( result->name, name,
						name_len );
					result->name[ name_len ] = '\0';
					result->lib = lib;
#ifndef IOT_STACK_ONLY
					result->is_in_heap = is_in_heap;
#endif /* ifndef IOT_STACK_ONLY */
					/* place in alphabetical order */
					while ( max_idx - min_idx > 0u )
					{
						int cmp_result;
						cur_idx = (max_idx - min_idx) / 2u + min_idx;
						cmp_result = os_strncasecmp( name,
							lib->action_ptr[cur_idx]->name,
							IOT_NAME_MAX_LEN );
						if ( cmp_result > 0 )
						{
							++cur_idx;
							min_idx = cur_idx;
						}
						else
							max_idx = cur_idx;
					}

					/* insert into proper spot in list */
					os_memmove( &lib->action_ptr[cur_idx + 1u],
						&lib->action_ptr[cur_idx],
						sizeof( struct iot_action * ) * (count - cur_idx) );
					lib->action_ptr[cur_idx] = result;
					++lib->action_count;
				}
#ifndef IOT_STACK_ONLY
				else if ( is_in_heap )
					os_free_null( (void**)&result );
#endif /* ifndef IOT_STACK_ONLY */
			}
			else
				IOT_LOG( lib, IOT_LOG_ERROR,
					"failed to allocate memory for action: %s",
					name );
		}
		else
			IOT_LOG( lib, IOT_LOG_ERROR,
				"no remaining space (max: %u) for action: %s",
				IOT_ACTION_MAX, name );
	}
	return result;
}

iot_status_t iot_action_option_get(
	const iot_action_t *action,
	const char *name,
	iot_bool_t convert,
	iot_type_t type, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action && name )
	{
		va_list args;
		size_t i;
		result = IOT_STATUS_NOT_FOUND;
		va_start( args, type );
		for ( i = 0u; i < action->option_count &&
			result == IOT_STATUS_NOT_FOUND; ++i )
		{
			if ( os_strncmp( action->option[i].name,
				name, IOT_NAME_MAX_LEN ) == 0 )
				result = iot_common_arg_get(
					&action->option[i].data,
					convert, type, args );
		}
		va_end( args );
	}
	return result;
}

iot_status_t iot_action_option_set(
	iot_action_t *action,
	const char* name,
	iot_type_t type, ... )
{
	va_list args;
	struct iot_data data;
	os_memzero( &data, sizeof( struct iot_data ) );
	va_start( args, type );
	iot_common_arg_set( &data, IOT_TRUE, type, args );
	va_end( args );
	return iot_action_option_set_data( action, name, &data );
}

iot_status_t iot_action_option_set_data(
	iot_action_t *action,
	const char *name,
	const struct iot_data *data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action && data )
	{
		unsigned int i;
		struct iot_option *attr = NULL;

		/* see if this is an option update */
		for ( i = 0u;
			attr == NULL && i < action->option_count; ++i )
		{
			if ( os_strcasecmp( action->option[i].name, name ) == 0 )
				attr = &action->option[i];
		}

		/* adding a new option */
		result = IOT_STATUS_FULL;
		if ( !attr && action->option_count < IOT_OPTION_MAX )
		{
#ifndef IOT_STACK_ONLY
			void *ptr = os_realloc( action->option,
				sizeof( struct iot_option ) *
				( action->option_count + 1u ) );
			if ( ptr )
				action->option = ptr;

			if ( action->option && ptr )
#endif /* ifndef IOT_STACK_ONLY */
			{
				attr = &action->option[action->option_count];
				os_memzero( attr, sizeof( struct iot_option ) );
				++action->option_count;
			}
		}

		/* add or update the option */
		if ( attr )
		{
			iot_bool_t update = IOT_TRUE;
			/* add name if not already given */
			if ( !attr->name || *attr->name == '\0' )
			{
				size_t name_len = os_strlen( name );
				if ( name_len > IOT_NAME_MAX_LEN )
					name_len = IOT_NAME_MAX_LEN;
#ifndef IOT_STACK_ONLY
				attr->name = os_malloc( sizeof( char ) * (name_len + 1u) );
#endif /* ifndef IOT_STACK_ONLY */
				if ( !attr->name )
					update = IOT_FALSE;
				else
				{
					os_strncpy( attr->name, name, name_len );
					attr->name[name_len] = '\0';
				}
			}

			if ( update )
			{
				/** @todo fix this to take ownership */
				os_memcpy( &attr->data, data,
					sizeof( struct iot_data ) );
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

iot_status_t iot_action_option_set_raw(
	iot_action_t *action,
	const char* name,
	size_t length,
	const void *ptr )
{
	struct iot_data data;
	os_memzero( &data, sizeof( struct iot_data ) );
	data.type = IOT_TYPE_RAW;
	data.value.raw.ptr = ptr;
	data.value.raw.length = length;
	data.has_value = IOT_TRUE;
	return iot_action_option_set_data( action, name, &data );
}

iot_status_t iot_action_deregister(
	iot_action_t *action,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action )
	{
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( action->lib )
		{
			if ( action->state != IOT_ITEM_DEREGISTERED )
			{
				result = iot_plugin_perform( action->lib, txn,
					&max_time_out,
					IOT_OPERATION_ACTION_DEREGISTER,
					action, NULL, NULL );
				if ( result == IOT_STATUS_SUCCESS )
					action->state = IOT_ITEM_DEREGISTERED;
				else
					action->state = IOT_ITEM_DEREGISTER_PENDING;
			}
		}
	}
	return result;
}

iot_status_t iot_action_execute(
	const struct iot_action *action,
	struct iot_action_request *request,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action && request )
	{
		result = IOT_STATUS_NOT_FOUND;
		if ( action->lib )
		{
			/* test to ensure that parameters are valid */
			const char *param_required_name = NULL;
			const char *param_bad_type_name = NULL;
			const char *param_unknown_name = NULL;
			iot_uint8_t i;

			for( i = 0u; i < action->parameter_count &&
				param_required_name == NULL &&
				param_bad_type_name == NULL; ++i )
			{
				iot_uint8_t j;
				const struct iot_action_parameter *reg_param =
					&action->parameter[i];
				/* requested parameter */
				struct iot_action_parameter *req_param = NULL;
				for ( j = 0u; req_param == NULL &&
					j < request->parameter_count; ++j )
				{
					if ( os_strncasecmp( reg_param->name,
						request->parameter[j].name,
						IOT_NAME_MAX_LEN ) == 0 )
					{
						req_param =
							&request->parameter[j];

						/* set the type of the request
						 * parameter, so below we can
						 * check if we know about this
						 * parameter */
						req_param->type =
							reg_param->type;
					}
				}

				/* check registered param vs. request param */
				if ( ( reg_param->type & IOT_PARAMETER_IN_REQUIRED )
					&& ( !req_param ||
					req_param->data.has_value == IOT_FALSE ) )
					param_required_name = reg_param->name;
				else if ( req_param && iot_common_data_convert(
					IOT_CONVERSION_BASIC,
					reg_param->data.type,
					&req_param->data ) == IOT_FALSE )
					param_bad_type_name = reg_param->name;
			}

			/* check for unknown parameters */
			for ( i = 0u; i < request->parameter_count; ++i )
			{
				if ( request->parameter[i].type == IOT_PARAMETER_OUT )
				{
					param_unknown_name =
						request->parameter[i].name;

					IOT_LOG( action->lib,
						IOT_LOG_TRACE,
						"Parameter \"%s\" [%d of %d] "
						"of type: %d\n for: %s",
						param_unknown_name,
						(int)i,
						(int)request->parameter_count,
						(int)request->parameter[i].type,
						action->name );
				}
			}

			if ( param_required_name == NULL &&
				param_bad_type_name == NULL &&
				param_unknown_name == NULL )
			{
				/* perform command execution */
				result = IOT_STATUS_FAILURE;
				request->flags = action->flags;
				request->time_limit = action->time_limit;
				if ( action->callback )
					result = (*(action->callback))( request,
						action->user_data );
				else if ( action->command &&
					*action->command != '\0' )
					result = iot_action_execute_command(
						action, request, max_time_out );
				else
				{
					iot_action_request_set_status(
						request, result,
						"no execution method "
						"registered for: %s",
						action->name );
				}
			}
			else if ( param_required_name != NULL )
			{
				result = IOT_STATUS_BAD_REQUEST;
				iot_action_request_set_status(
					request, result,
					"required IN parameter missing: %s",
					param_required_name );
			}
			else if ( param_bad_type_name != NULL )
			{
				result = IOT_STATUS_BAD_REQUEST;
				iot_action_request_set_status(
					request, result,
					"invalid value for parameter: %s",
					param_bad_type_name );
			}
			else
			{
				result = IOT_STATUS_BAD_REQUEST;
				iot_action_request_set_status(
					request, result,
					"unknown parameter: %s",
					param_unknown_name );
			}

			/* ensure all required out parameters have values */
			for ( i = 0u; result == IOT_STATUS_SUCCESS &&
				i < action->parameter_count; ++i )
			{
				iot_uint8_t j;
				const struct iot_action_parameter *reg_param =
					&action->parameter[i];
				const struct iot_action_parameter *req_param =
					NULL;
				/* requested parameter */
				if ( reg_param->type & IOT_PARAMETER_OUT_REQUIRED )
				{
					for ( j = 0u; req_param == NULL &&
						j < request->parameter_count; ++j )
					{
						if ( os_strncasecmp( reg_param->name,
							request->parameter[j].name,
							IOT_NAME_MAX_LEN ) == 0 )
						{
							req_param =
								&request->parameter[j];
						}
					}

					if ( req_param == NULL ||
						req_param->data.has_value == IOT_FALSE )
					{
						result = IOT_STATUS_BAD_REQUEST;
						iot_action_request_set_status(
							request, result,
							"required OUT "
							"parameter missing: %s",
							reg_param->name );
					}
				}
			}
		}
	}
	return result;
}

iot_status_t iot_action_execute_command(
	const struct iot_action *action,
	struct iot_action_request *request,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action && request )
	{
		const char *const command_path = action->command;
		if ( command_path )
		{
			char *buf[2u];
			size_t buf_len[2u];
			char buf_stderr[ IOT_ACTION_COMMAND_OUTPUT_MAX_LEN ];
			char buf_stdout[ IOT_ACTION_COMMAND_OUTPUT_MAX_LEN ];
			char command_with_params[PATH_MAX + 1u];
			size_t i;
			const char * const output_params[2u] = {
				IOT_ACTION_COMMAND_STDOUT,
				IOT_ACTION_COMMAND_STDERR,
			};
			os_status_t system_res;
			int system_ret;

			os_memzero( command_with_params, PATH_MAX + 1u );
			os_strncpy( command_with_params, command_path,
				PATH_MAX );
			command_with_params[ PATH_MAX ] = '\0';
			for ( i = 0u; i < request->parameter_count; ++i )
			{
				const size_t cur_len =
					os_strlen( command_with_params );
				const struct iot_action_parameter *p =
					&request->parameter[i];

				size_t space_left = PATH_MAX - cur_len;
				char* param_pos = &command_with_params[cur_len];
				if ( space_left > 0u )
				{
					*param_pos = ' ';
					--space_left;
					++param_pos;
					if ( *p->name != '\0' )
					{
						const size_t name_len =
							os_strlen( p->name ) + 3u;
						if ( space_left > name_len )
						{
							os_snprintf(
								param_pos,
								space_left,
								"--%s=",
								p->name );
							space_left -= name_len;
							param_pos += name_len;
						}
					}
				}
				switch( p->data.type )
				{
					case IOT_TYPE_NULL:
						os_snprintf( param_pos,
							space_left, "[NULL]" );
						break;
					case IOT_TYPE_BOOL:
						os_snprintf( param_pos,
							space_left, "%u",
							p->data.value.boolean ==
							IOT_FALSE ? 0u : 1u );
						break;
					case IOT_TYPE_FLOAT32:
						os_snprintf( param_pos,
							space_left, "%f",
							(double)p->data.value.float32 );
						break;
					case IOT_TYPE_FLOAT64:
						os_snprintf( param_pos,
							space_left, "%f",
							p->data.value.float64 );
						break;
					case IOT_TYPE_INT8:
						os_snprintf( param_pos,
							space_left, "%hhi",
							p->data.value.int8 );
						break;
					case IOT_TYPE_INT16:
						os_snprintf( param_pos,
							space_left, "%hi",
							p->data.value.int16 );
						break;
					case IOT_TYPE_INT32:
						os_snprintf( param_pos,
							space_left, "%i",
							p->data.value.int32 );
						break;
					case IOT_TYPE_INT64:
						os_snprintf( param_pos,
							space_left, "%lli",
							(long long int)p->data.value.int64 );
						break;
					case IOT_TYPE_LOCATION:
					{
						iot_float64_t lon = 0.0;
						iot_float64_t lat = 0.0;
						if ( p->data.value.location )
						{
							lon = p->data.value.location->longitude;
							lat = p->data.value.location->latitude;
						}
						os_snprintf( param_pos,
							space_left, "[%f,%f]",
							lon, lat );
						break;
					}
					case IOT_TYPE_RAW:
					{
						/* convert data to base64 */
						size_t const min_size =
							iot_base64_encode_size(
								p->data.value.raw.length );
						if ( space_left > min_size )
						{
							iot_base64_encode(
								(uint8_t*)param_pos,
								space_left,
								(const uint8_t*)p->data.value.raw.ptr,
								p->data.value.raw.length );
							param_pos[min_size] = '\0';
							param_pos += min_size;
						}
						break;
					}
					case IOT_TYPE_STRING:
					{
						const char *s = p->data.value.string;
						if ( space_left > os_strlen( s ) + 2u )
						{
							*param_pos = '"';
							++param_pos;
							--space_left;
							while ( space_left > 0u && *s != '\0' )
							{
								if ( *s == '"' || *s == '\\' )
								{
									if ( space_left > 1u )
									{
										*param_pos = '\\';
										++param_pos;
										*param_pos = *s;
										++param_pos;
										space_left -= 2u;
										++s;
									}
									else
										space_left = 0u;
								}
								else
								{
									*param_pos = *s;
									++param_pos;
									--space_left;
									++s;
								}
							}
							if ( space_left > 0u )
							{
								*param_pos = '"';
								++param_pos;
								--space_left;
								if ( space_left > 0u )
									*param_pos = '\0';
							}
						}
						break;
					}
					case IOT_TYPE_UINT8:
						os_snprintf( param_pos,
							space_left, "%hhu",
							p->data.value.uint8 );
						break;
					case IOT_TYPE_UINT16:
						os_snprintf( param_pos,
							space_left, "%hu",
							p->data.value.uint16 );
						break;
					case IOT_TYPE_UINT32:
						os_snprintf( param_pos,
							space_left, "%u",
							p->data.value.uint32 );
						break;
					case IOT_TYPE_UINT64:
						os_snprintf( param_pos,
							space_left, "%llu",
							(long long unsigned int)p->data.value.uint64 );
						break;
				}
			}

			for ( i = 0u; i < 2u; ++i )
			{
				buf[i] = NULL;
				buf_len[i] = 0u;
			}

			/* script is returnable, set the output buffers */
			if ( !( action->flags & IOT_ACTION_NO_RETURN ) )
			{
				buf[0] = buf_stdout;
				buf_len[0] = IOT_ACTION_COMMAND_OUTPUT_MAX_LEN;
				buf[1] = buf_stderr;
				buf_len[1] = IOT_ACTION_COMMAND_OUTPUT_MAX_LEN;
			}

			/* base64 encoded string, may contain "\r\n" characters.
			 * Some OSs, (i.e. Windows), will next execute if the
			 * line contains "\r\n".
			 * Remove the CRLF in command parameter.
			 */
			iot_action_parameter_adjustment( command_with_params,
				"\r\n" );

			IOT_LOG( action->lib, IOT_LOG_DEBUG,
				"Executing command: %s", command_with_params );

			/* if there is a time limit and it's less then our
			 * maximum time to wait then set it to the action
			 * limit */
			if ( !( action->flags & IOT_ACTION_NO_TIME_LIMIT )
				&& ( max_time_out == 0u ||
				     max_time_out > action->time_limit ) )
			{
				max_time_out = action->time_limit;
			}

			system_res = os_system_run_wait( command_with_params,
				&system_ret, buf, buf_len, max_time_out );
			if ( system_res == OS_STATUS_SUCCESS )
			{
				if ( !( action->flags & IOT_ACTION_NO_RETURN ) )
				{
					/* return the return code from the script */
					iot_action_request_parameter_set( request,
						IOT_ACTION_COMMAND_RETVAL,
						IOT_TYPE_INT32, system_ret );
					/* if stdout or stderr contained data update
					   the output parameters */
					for ( i = 0u; i < 2u; ++i )
					{
						if ( buf[i] && buf[i][0] != '\0' )
						{
							iot_action_request_parameter_set(
								request,
								output_params[i],
								IOT_TYPE_STRING,
								buf[i] );
						}
					}
				}
				IOT_LOG( action->lib, IOT_LOG_INFO,
					"Command \"%s\", exited with: %d",
					action->name, system_ret );
				if ( system_ret != 0 )
					result = IOT_STATUS_EXECUTION_ERROR;
				else
					result = IOT_STATUS_SUCCESS;
			}
			else if ( ( action->flags & IOT_ACTION_NO_RETURN ) &&
				system_res == OS_STATUS_INVOKED )
			{
				result = IOT_STATUS_INVOKED;
				IOT_LOG( action->lib, IOT_LOG_INFO,
					"Command \"%s\", has been invoked",
					action->name );
			}
			else
			{
				result = IOT_STATUS_FAILURE;
				IOT_LOG( action->lib, IOT_LOG_ERROR,
					"Command \"%s\" failed, reason: %s\n",
					action->name,
					os_system_error_string(
						os_system_error_last() ) );
			}
		}
	}
	return result;
}

iot_status_t iot_action_flags_set(
	iot_action_t *action,
	iot_uint8_t flags )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action )
	{
		action->flags = flags;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_action_free( iot_action_t *action,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action )
	{
		iot_t *const lib = action->lib;
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( lib )
		{
			unsigned int i, max;
			result = iot_action_deregister( action, NULL,
				max_time_out );

			/* find action within the library */
			max = lib->action_count;
			for ( i = 0u; i < max &&
				lib->action_ptr[ i ] != action;
				++i );

			result = IOT_STATUS_NOT_FOUND;
			if ( i < max )
			{
#ifndef IOT_STACK_ONLY
				/* free any heap allocated storage */
				size_t j;
				for ( j = 0u; j < action->parameter_count; ++j )
				{
					os_free_null(
						(void **)&action->parameter[j].data.heap_storage );
					os_free_null( (void **)&action->parameter[j].name );
				}

				for ( j = 0u; j < action->option_count; ++j )
				{
					os_free_null(
						(void **)&action->option[j].data.heap_storage );
					os_free_null( (void **)&action->option[j].name );
				}
#endif /* ifndef IOT_STACK_ONLY */

				/* remove from client */
				os_memmove(
					&lib->action_ptr[ i ],
					&lib->action_ptr[ i + 1u ],
					sizeof( struct iot_action * ) * ( max - i - 1u ) );

				/* set lib to NULL */
				action->lib = NULL;

				/* clear/free the action */
				--lib->action_count;
#ifdef IOT_STACK_ONLY
				lib->action_ptr[
					lib->action_count] = action;
#else /* ifdef IOT_STACK_ONLY */
				os_free_null( (void **)&action->option );
				os_free_null( (void **)&action->name );
				os_free_null( (void **)&action->parameter );
				os_free_null( (void **)&action->command );
				if ( action->is_in_heap == IOT_FALSE )
				{
					lib->action_ptr[
						lib->action_count] = action;
				}
				else
				{
					lib->action_ptr[
						lib->action_count] = NULL;
					os_free_null( (void **)&action );
				}
#endif /* else IOT_STACK_ONLY */
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

iot_status_t iot_action_parameter_add(
	iot_action_t *action,
	const char *name,
	iot_parameter_type_t param_type,
	iot_type_t data_type,
	iot_millisecond_t UNUSED(max_time_out) )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action && name )
	{
		const char *bad_char = NULL;
		result = IOT_STATUS_BAD_REQUEST;
		if ( ( bad_char = os_strpbrk( name,
			IOT_PARAMETER_NAME_BAD_CHARACTERS ) ) == NULL )
		{
			result = IOT_STATUS_FULL;
			if ( action->parameter_count < IOT_PARAMETER_MAX )
			{
				size_t i;
				result = IOT_STATUS_SUCCESS;

				/* check if parameter name is already used */
				for ( i = 0u; result == IOT_STATUS_SUCCESS &&
					i < action->parameter_count; ++i )
				{
					if ( os_strncasecmp( action->parameter[i].name,
						name, IOT_NAME_MAX_LEN ) == 0 )
						result = IOT_STATUS_BAD_REQUEST;
				}

				if ( result == IOT_STATUS_SUCCESS )
				{
					struct iot_action_parameter *p = NULL;
					size_t name_len = os_strlen( name );
					if ( name_len > IOT_NAME_MAX_LEN )
						name_len = IOT_NAME_MAX_LEN;
#ifndef IOT_STACK_ONLY
					{
						struct iot_action_parameter* p_alloc;
						char *p_name;

						result = IOT_STATUS_NO_MEMORY;
						p_alloc = os_realloc( action->parameter,
							(sizeof( struct iot_action_parameter ) *
							(action->parameter_count + 1u) ) );
						p_name = os_malloc( sizeof( char ) * (name_len + 1u) );
						if ( p_alloc && p_name )
						{
							action->parameter = p_alloc;
							p = &action->parameter[action->parameter_count];
							p->name = p_name;
						}
						else
							os_free_null( (void**)&p_name );
					}
#else
					p = &action->parameter[
						action->parameter_count ];
					p->name = p->_name;
#endif /* ifndef IOT_STACK_ONLY */
					if ( p && p->name )
					{
						/* if required, set equivilant
						 * in/out flag */
						if ( param_type & IOT_PARAMETER_IN_REQUIRED )
							param_type |= IOT_PARAMETER_IN;
						if ( param_type & IOT_PARAMETER_OUT_REQUIRED )
							param_type |= IOT_PARAMETER_OUT;
						os_strncpy( p->name, name, name_len );
						p->name[ name_len ] = '\0';
						p->type = param_type;
						p->data.type = data_type;
						p->data.heap_storage = NULL;
						++action->parameter_count;
						result = IOT_STATUS_SUCCESS;
					}
				}
				else
					IOT_LOG( action->lib,
						IOT_LOG_ERROR,
						"Parameter already exists: %s",
						name );
			}
			else
				IOT_LOG( action->lib,
					IOT_LOG_ERROR,
					"Maximum parameters reached: %d",
					IOT_PARAMETER_MAX );
		}
		else
			IOT_LOG( action->lib,
				IOT_LOG_ERROR,
				"Invalid character in parameter name: %c",
				*bad_char );
	}
	return result;
}

size_t iot_action_parameter_adjustment( char *command_param, const char *word )
{
	size_t count = 0;
	if ( command_param && word )
	{
		const size_t word_length = os_strlen( word );
		if ( word_length > 0u )
		{
			char *param = command_param;

			while ( ( param = os_strstr( param, word ) ) != NULL )
			{
				char *dst = param;
				char *src = param + word_length;
				while ( ( *dst++ = *src++ ) );
				++count;
			}
		}
	}
	return count;
}

iot_status_t iot_action_parameter_get(
	const iot_action_request_t *request,
	const char *name,
	iot_bool_t convert,
	iot_type_t type, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request && name )
	{
		va_list args;
		size_t i;
		result = IOT_STATUS_NOT_FOUND;
		va_start( args, type );
		for( i = 0u; i < request->parameter_count &&
			result == IOT_STATUS_NOT_FOUND; ++i )
		{
			if ( os_strncasecmp( request->parameter[i].name,
				name, IOT_NAME_MAX_LEN ) == 0 )
			{
				result = iot_common_arg_get(
					&request->parameter[i].data,
					convert, type, args );
			}
		}
		va_end( args );
	}
	return result;
}

iot_status_t iot_action_parameter_get_raw(
	const iot_action_request_t *request,
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
		result = iot_action_parameter_get( request, name, convert,
			IOT_TYPE_RAW, &raw_data );
		if ( length )
			*length = raw_data.length;
		*data = raw_data.ptr;
	}
	return result;
}

iot_status_t iot_action_parameter_set(
	iot_action_request_t *request,
	const char *name,
	iot_type_t type, ... )
{
	iot_status_t result;
	va_list args;
	va_start( args, type );
	result = iot_action_request_parameter_set_args( request, name, type, args );
	va_end( args );
	return result;
}

iot_status_t iot_action_parameter_set_raw(
	iot_action_request_t *request, const char *name,
	size_t length, const void *data )
{
	struct iot_data data_obj;
	os_memzero( &data_obj, sizeof( struct iot_data ) );
	data_obj.type = IOT_TYPE_RAW;
	data_obj.value.raw.ptr = data;
	data_obj.value.raw.length = length;
	data_obj.has_value = IOT_TRUE;
	return iot_action_parameter_set( request, name,
		IOT_TYPE_RAW, &data_obj );
}

iot_status_t iot_action_process(
	iot_t *lib,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		struct iot_action_request *request = NULL;

		result = IOT_STATUS_NOT_FOUND;
#ifndef IOT_NO_THREAD_SUPPORT
		if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
		{
			os_thread_mutex_lock( &lib->worker_mutex );
			/* nothing to do, so wait for signal to do work */
			if ( lib->request_queue_wait_count == 0u )
				os_thread_condition_wait(
					&lib->worker_signal,
					&lib->worker_mutex );
		}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
		/* if this thread was not woke just to quit,
		   then there must be a request */
		if ( lib->request_queue_wait_count > 0u )
		{
			request = lib->request_queue_wait[0u];
			--lib->request_queue_wait_count;

			/* move all items up by 1 in request wait queue */
			os_memcpy( &lib->request_queue_wait[0u],
				&lib->request_queue_wait[1u],
				sizeof( struct iot_action_request *) *
					 lib->request_queue_wait_count );
		}
#ifndef IOT_NO_THREAD_SUPPORT
		if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
			os_thread_mutex_unlock( &lib->worker_mutex );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

		if ( request )
		{
			size_t i;
			const iot_action_t *action = NULL;
			iot_status_t action_result = IOT_STATUS_NOT_FOUND;
			for ( i = 0u; action == NULL &&
				i < lib->action_count &&
				i < IOT_ACTION_MAX; ++i )
			{
				action = lib->action_ptr[i];
				if ( action && action->name )
				{
					if ( os_strncasecmp( action->name,
						request->name,
						IOT_NAME_MAX_LEN ) != 0 )
						action = NULL;
				}
			}

			if ( lib->to_quit == IOT_FALSE && action )
			{
#ifndef IOT_NO_THREAD_SUPPORT
				/* lock to support exclusive actions */
				if ( action->flags & IOT_ACTION_EXCLUSIVE_APP )
					os_thread_rwlock_write_lock(
						&lib->worker_thread_exclusive_lock );
				else
					os_thread_rwlock_read_lock(
						&lib->worker_thread_exclusive_lock );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
				IOT_LOG( lib, IOT_LOG_DEBUG,
					"Executing action: %s", action->name );
				action_result = iot_action_execute( action,
					request, max_time_out );

#ifndef IOT_NO_THREAD_SUPPORT
				/* done processing, unlock our operation */
				if ( action->flags & IOT_ACTION_EXCLUSIVE_APP )
					os_thread_rwlock_write_unlock(
						&lib->worker_thread_exclusive_lock );
				else
					os_thread_rwlock_read_unlock(
						&lib->worker_thread_exclusive_lock );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
			}
			else if ( lib->to_quit == IOT_FALSE )
				IOT_LOG( lib, IOT_LOG_NOTICE,
					"Not executing action: %s; "
					"reason: %s", request->name,
					iot_error( action_result ) );

			/* send command execution result to the cloud */
			request->result = action_result;
			result = iot_plugin_perform( lib, NULL, &max_time_out,
				IOT_OPERATION_ACTION_COMPLETE, action, request,
				NULL );

			/* free memory associated with the request */
			iot_action_request_free( request );

			/* mark request spot as clear */
#ifndef IOT_NO_THREAD_SUPPORT
			if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
			{
				os_thread_mutex_lock(
					&lib->worker_mutex );
			}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
			os_memzero( request,
				sizeof( struct iot_action_request ) );
			--lib->request_queue_free_count;

			/* add request space to last free spot */
			lib->request_queue_free[lib->request_queue_free_count] = request;
#ifndef  IOT_NO_THREAD_SUPPORT
			if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
				os_thread_mutex_unlock(
					&lib->worker_mutex );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t iot_action_register(
	iot_action_t *action,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action && action->lib )
	{
		IOT_LOG( action->lib, IOT_LOG_TRACE, "Registering %s",
			action->name );
		result = iot_plugin_perform( action->lib, txn, &max_time_out,
			IOT_OPERATION_ACTION_REGISTER, action, NULL, NULL );
		if ( result == IOT_STATUS_SUCCESS )
			action->state = IOT_ITEM_REGISTERED;
		else
			action->state = IOT_ITEM_REGISTER_PENDING;
	}
	return result;
}

iot_status_t iot_action_register_callback(
	iot_action_t *action,
	iot_action_callback_t *func,
	void *user_data,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action )
	{
#ifdef IOT_STACK_ONLY
		action->command[0] = '\0';
#else /* ifdef IOT_STACK_ONLY */
		os_free_null( (void**)&action->command );
#endif /* else IOT_STACK_ONLY */
		action->callback = func;
		action->user_data = user_data;
		result = iot_action_register( action, txn, max_time_out );
	}
	return result;
}

iot_status_t iot_action_register_command( iot_action_t *action,
	const char *command, iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action && action->lib && command )
	{
		char *buf;
		size_t cmd_len;
		cmd_len = os_strlen( command );
		if ( cmd_len > PATH_MAX )
			cmd_len = PATH_MAX;
#ifdef IOT_STACK_ONLY
		buf = action->_command;
#else
		result = IOT_STATUS_NO_MEMORY;
		buf = (char*)os_realloc( action->command,
			cmd_len + 1u );
#endif /* else IOT_STACK_ONLY */
		if ( buf )
		{
			action->command = buf;
			action->callback = NULL;
			os_strncpy( buf, command, cmd_len );
			buf[ cmd_len ] = '\0';
			result = iot_action_register( action, txn, max_time_out );
		}
	}
	return result;
}

iot_action_request_t *iot_action_request_allocate(
	iot_t *lib,
	const char *name,
	const char *source )
{
	iot_action_request_t *result = NULL;
	if ( lib && name && *name )
	{
		size_t name_len = os_strlen( name );
		size_t source_len = 0u;
		if ( source )
			source_len = os_strlen( source );
		if ( name_len > IOT_NAME_MAX_LEN )
			name_len = IOT_NAME_MAX_LEN;
		if ( source_len > IOT_ID_MAX_LEN )
			source_len = IOT_ID_MAX_LEN;
#ifndef IOT_NO_THREAD_SUPPORT
		if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
		{
			os_thread_mutex_lock(
				&lib->worker_mutex );
		}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
		if ( lib->request_queue_free_count < IOT_ACTION_QUEUE_MAX )
			result = lib->request_queue_free[lib->request_queue_free_count];

		if ( result )
		{
			os_memzero( result, sizeof( struct iot_action_request ) );
			result->lib = lib;
#ifdef IOT_STACK_ONLY
			result->name = result->_name;
			result->source = result->_source;
#else /* ifdef IOT_STACK_ONLY */
			result->name = os_malloc( name_len + source_len + 2u );
			if ( source_len > 0u )
				result->source = result->name + name_len + 1u;
#endif /* else IOT_STACK_ONLY */
			if ( result->name )
			{
				os_strncpy( result->name, name, name_len );
				result->name[name_len] = '\0';
				if ( source_len > 0u )
				{
					os_strncpy( result->source,
						source, source_len + 1u);
					result->source[source_len] = '\0';
				}
			}
			++lib->request_queue_free_count;
		}
#ifndef  IOT_NO_THREAD_SUPPORT
		if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
			os_thread_mutex_unlock(
				&lib->worker_mutex );
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
	}
	return result;
}

iot_status_t iot_action_request_option_get(
	const iot_action_request_t *request,
	const char *name,
	iot_bool_t convert,
	iot_type_t type, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request && name )
	{
		va_list args;
		size_t i;
		result = IOT_STATUS_NOT_FOUND;
		va_start( args, type );
		for ( i = 0u; i < request->option_count &&
			result == IOT_STATUS_NOT_FOUND; ++i )
		{
			if ( os_strncmp( request->option[i].name,
				name, IOT_NAME_MAX_LEN ) == 0 )
				result = iot_common_arg_get(
					&request->option[i].data,
					convert, type, args );
		}
		va_end( args );
	}
	return result;
}

iot_status_t iot_action_request_option_set(
	iot_action_request_t *request,
	const char* name,
	iot_type_t type, ... )
{
	va_list args;
	struct iot_data data;
	os_memzero( &data, sizeof( struct iot_data ) );
	va_start( args, type );
	iot_common_arg_set( &data, IOT_TRUE, type, args );
	va_end( args );
	return iot_action_request_option_set_data( request, name, &data );
}

iot_status_t iot_action_request_option_set_data(
	iot_action_request_t *request,
	const char *name,
	const struct iot_data *data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request && data )
	{
		unsigned int i;
		struct iot_option *attr = NULL;

		/* see if this is an option update */
		for ( i = 0u;
			attr == NULL && i < request->option_count; ++i )
		{
			if ( os_strcmp( request->option[i].name, name ) == 0 )
				attr = &request->option[i];
		}

		/* adding a new option */
		result = IOT_STATUS_FULL;
		if ( !attr && request->option_count < IOT_OPTION_MAX )
		{
#ifndef IOT_STACK_ONLY
			void *ptr = os_realloc( request->option,
				sizeof( struct iot_option ) *
					( request->option_count + 1u ) );
			if ( ptr )
					request->option = ptr;
			if ( request->option && ptr )
#endif /* ifndef IOT_STACK_ONLY */
			{
				attr = &request->option[request->option_count];
				os_memzero( attr, sizeof( struct iot_option ) );
				++request->option_count;
			}
		}

		/* add or update the option */
		if ( attr )
		{
			iot_bool_t update = IOT_TRUE;
			/* add name if not already given */
			if ( !attr->name || *attr->name == '\0' )
			{
				size_t name_len = os_strlen( name );
				if ( name_len > IOT_NAME_MAX_LEN )
					name_len = IOT_NAME_MAX_LEN;
#ifndef IOT_STACK_ONLY
				attr->name = os_malloc( sizeof( char ) * (name_len + 1u) );
#else
				attr->name = attr->_name;
#endif /* ifndef IOT_STACK_ONLY */
				if ( !attr->name )
					update = IOT_FALSE;
				else
				{
					os_strncpy( attr->name, name, name_len );
					attr->name[name_len] = '\0';
				}
			}

			if ( update )
			{
				/** @todo fix this to take ownership */
				os_memcpy( &attr->data, data,
					sizeof( struct iot_data ) );
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
	return result;
}

iot_status_t iot_action_request_option_set_raw(
	iot_action_request_t *request,
	const char* name,
	size_t length,
	const void *ptr )
{
	struct iot_data data;
	os_memzero( &data, sizeof( struct iot_data ) );
	data.type = IOT_TYPE_RAW;
	data.value.raw.ptr = ptr;
	data.value.raw.length = length;
	data.has_value = IOT_TRUE;
	return iot_action_request_option_set_data( request, name, &data );
}

iot_status_t iot_action_request_copy( iot_action_request_t *dest,
	const iot_action_request_t *request, void *var_data,
	size_t var_data_size )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( dest && request && dest != request )
	{
		size_t i;
		os_memcpy( dest, request, sizeof( struct iot_action_request ) );

		/* copy variable data */
		result = IOT_STATUS_SUCCESS;
		dest->parameter = NULL;
		if ( request->parameter_count > 0u )
		{
#ifdef IOT_STACK_ONLY
			dest->parameter = dest->_parameter;
#else
			result = IOT_STATUS_NO_MEMORY;
			if ( var_data_size >=
				sizeof( struct iot_action_parameter ) *
				request->parameter_count )
			{
				dest->parameter = var_data;
				var_data_size -=
					sizeof( struct iot_action_parameter ) *
					request->parameter_count;
				var_data = (char*)var_data +
					( sizeof( struct iot_action_parameter ) *
					request->parameter_count );
				result = IOT_STATUS_SUCCESS;
			}
#endif
		}
		for ( i = 0u; dest->parameter && i < request->parameter_count; ++i )
		{
			if ( request->parameter[i].data.has_value != IOT_FALSE )
			{
				if ( request->parameter[i].data.type == IOT_TYPE_RAW )
				{
					if ( result == IOT_STATUS_SUCCESS &&
						var_data_size >=
						request->parameter[i].data.value.raw.length )
					{
						dest->parameter[i].data.value.raw.ptr
							= var_data;
						dest->parameter[i].data.value.raw.length =
							request->parameter[i].data.value.raw.length;
						os_memcpy( var_data,
							request->parameter[i].data.value.raw.ptr,
							request->parameter[i].data.value.raw.length );
						var_data_size -=
							request->parameter[i].data.value.raw.length;
						var_data = (char*)var_data +
							request->parameter[i].data.value.raw.length;
					}
					else
					{
						dest->parameter[i].data.value.raw.length = 0u;
						dest->parameter[i].data.value.raw.ptr = NULL;
						result = IOT_STATUS_NO_MEMORY;
					}
				}
				else if ( request->parameter[i].data.type == IOT_TYPE_STRING )
				{
					size_t str_len = 0u;
					if ( request->parameter[i].data.value.string )
						str_len = os_strlen( request->parameter[i].data.value.string ) + 1u;
					if ( result == IOT_STATUS_SUCCESS &&
						request->parameter[i].data.value.string &&
						var_data_size >= str_len )
					{
						dest->parameter[i].data.value.string = (const char *)var_data;
						os_memcpy( var_data,
							request->parameter[i].data.value.string,
							str_len ) ;
						var_data_size -= str_len;
						var_data = (char*)var_data + str_len;
					}
					else
					{
						dest->parameter[i].data.value.string = NULL;
						result = IOT_STATUS_NO_MEMORY;
					}
				}
			}
		}
	}
	return result;
}

size_t iot_action_request_copy_size(
	const iot_action_request_t *request )
{
	size_t result = 0u;
	if ( request )
	{
		size_t i;
#ifndef IOT_STACK_ONLY
		result = sizeof( struct iot_action_parameter ) *
			request->parameter_count;
#endif
		for ( i = 0u; i < request->parameter_count; ++i )
		{
			if ( request->parameter[i].data.has_value != IOT_FALSE )
			{
				if ( request->parameter[i].data.type == IOT_TYPE_RAW )
					result += request->parameter[i].data.value.raw.length;
				else if ( request->parameter[i].data.type == IOT_TYPE_STRING )
				{
					++result;
					if ( request->parameter[i].data.value.string )
						result += os_strlen( request->parameter[i].data.value.string );
				}
			}
		}
	}
	return result;
}

iot_status_t iot_action_request_execute(
	iot_action_request_t *request,
	iot_millisecond_t UNUSED(max_time_out) )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( request->lib )
		{
			struct iot *lib = request->lib;

#ifndef IOT_NO_THREAD_SUPPORT
			if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
			{
				os_thread_mutex_lock( &lib->worker_mutex );
			}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

			lib->request_queue_wait[
				lib->request_queue_wait_count] = request;
			++lib->request_queue_wait_count;

#ifndef  IOT_NO_THREAD_SUPPORT
			if ( !( lib->flags & IOT_FLAG_SINGLE_THREAD ) )
			{
				os_thread_mutex_unlock( &lib->worker_mutex );
				os_thread_condition_signal(
					&lib->worker_signal,
					&lib->worker_mutex );
			}
#endif /* ifndef IOT_NO_THREAD_SUPPORT */
		}
	}
	return result;
}

iot_status_t iot_action_request_free(
	iot_action_request_t *request )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request )
	{
#ifndef IOT_STACK_ONLY
		size_t i;

		/* free any space allocated */
		for ( i = 0u; i < request->option_count; ++i )
		{
			os_free_null( (void**)&request->option[i].name );
			os_free_null( (void**)&request->option[i].data.heap_storage );
		}
		for ( i = 0u; i < request->parameter_count; ++i )
		{
			os_free_null( (void**)&request->parameter[i].name );
			os_free_null( (void**)&request->parameter[i].data.heap_storage );
		}

		request->option_count = 0u;
		request->parameter_count = 0u;
		os_free_null( (void**)&request->option );
		os_free_null( (void**)&request->parameter );
		os_free_null( (void**)&request->error );
		os_free_null( (void**)&request->name );
#endif /* ifndef IOT_STACK_ONLY */
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_action_request_parameter_iterator(
	const iot_action_request_t *request,
	iot_parameter_type_t type,
	iot_action_request_parameter_iterator_t *iter )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_action_request_parameter_iterator_t out = 0u;
	if ( request && iter )
	{
		iot_bool_t param_found = IOT_FALSE;
		iot_uint8_t idx;
		/* if not specified, return all parameters */
		if ( type == 0 ) type = (iot_parameter_type_t)(~0);

		/* find first matching parameter */
		for ( idx = 0u; idx < request->parameter_count &&
			param_found == IOT_FALSE; ++idx )
		{
			if ( request->parameter[idx].type & type )
				param_found = IOT_TRUE;
		}

		result = IOT_STATUS_NOT_FOUND;
		if ( param_found != IOT_FALSE )
		{
			out = idx - 1u;
			out <<= (sizeof( iot_parameter_type_t ) * CHAR_BIT);
			out |= type & ((1 << (sizeof( iot_parameter_type_t ) * CHAR_BIT)) - 1);
			result = IOT_STATUS_SUCCESS;
		}
	}

	if ( iter )
		*iter = out;
	return result;
}

iot_type_t iot_action_request_parameter_iterator_data_type(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter )
{
	iot_type_t result = IOT_TYPE_NULL;
	if ( request )
	{
		/* determine index & type from iterator */
		const iot_parameter_type_t p_type =
			(iot_parameter_type_t)(iter & ((1 << (sizeof( iot_parameter_type_t ) * CHAR_BIT)) - 1));
		const iot_uint8_t idx = (iot_uint8_t)(iter >> (sizeof( iot_parameter_type_t ) * CHAR_BIT));
		if ( idx < request->parameter_count && p_type > 0u )
			result = request->parameter[idx].data.type;
	}
	return result;
}

iot_status_t iot_action_request_parameter_iterator_get(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter,
	iot_bool_t convert,
	iot_type_t type, ... )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request && iter )
	{
		va_list args;
		/* determine index & type from iterator */
		const iot_parameter_type_t p_type =
			(iot_parameter_type_t)(iter & ((1 << (sizeof( iot_parameter_type_t ) * CHAR_BIT)) - 1));
		const iot_uint8_t idx = (iot_uint8_t)(iter >> (sizeof( iot_parameter_type_t ) * CHAR_BIT));

		result = IOT_STATUS_NOT_FOUND;
		va_start( args, type );
		if ( idx < request->parameter_count && p_type > 0u )
		{
			result = iot_common_arg_get(
				&request->parameter[idx].data,
				convert, type, args );
		}
		va_end( args );
	}
	return result;
}

iot_status_t iot_action_request_parameter_iterator_get_raw(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter,
	iot_bool_t convert,
	size_t *length,
	const void **data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request && iter )
	{
		struct iot_data_raw raw_data;
		os_memzero( &raw_data, sizeof( struct iot_data_raw ) );
		result = iot_action_request_parameter_iterator_get(
			request, iter, convert, IOT_TYPE_RAW, &raw_data );
		if ( length )
			*length = raw_data.length;
		*data = raw_data.ptr;
	}
	return result;
}

const char *iot_action_request_parameter_iterator_name(
	const iot_action_request_t *request,
	const iot_action_request_parameter_iterator_t iter )
{
	const char *result = NULL;
	if ( request )
	{
		/* determine index & type from iterator */
		const iot_parameter_type_t p_type =
			(iot_parameter_type_t)(iter & ((1 << (sizeof( iot_parameter_type_t ) * CHAR_BIT)) - 1));
		const iot_uint8_t idx = (iot_uint8_t)(iter >> (sizeof( iot_parameter_type_t ) * CHAR_BIT));
		if ( idx < request->parameter_count && p_type > 0u )
			result = request->parameter[idx].name;
	}
	return result;
}

iot_status_t iot_action_request_parameter_iterator_next(
		const iot_action_request_t *request,
		iot_action_request_parameter_iterator_t *iter )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request && iter )
	{
		const iot_parameter_type_t type =
			(iot_parameter_type_t)(*iter & ((1 << (sizeof( iot_parameter_type_t ) * CHAR_BIT)) - 1));
		iot_uint8_t idx = (iot_uint8_t)(*iter >> (sizeof( iot_parameter_type_t ) * CHAR_BIT) );
		iot_bool_t param_found = IOT_FALSE;

		/* find next matching parameter */
		for ( idx = idx + 1u; idx < request->parameter_count &&
			param_found == IOT_FALSE; ++idx )
		{
			if ( request->parameter[idx].type & type )
				param_found = IOT_TRUE;
		}

		result = IOT_STATUS_NOT_FOUND;
		if ( param_found != IOT_FALSE )
		{
			*iter = idx - 1u;
			*iter <<= (sizeof( iot_parameter_type_t ) * CHAR_BIT);
			*iter |= type & ((1 << (sizeof( iot_parameter_type_t ) * CHAR_BIT)) - 1);
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t iot_action_request_parameter_set(
	iot_action_request_t *request,
	const char *name,
	iot_type_t type, ... )
{
	iot_status_t result;
	va_list args;
	va_start( args, type );
	result = iot_action_request_parameter_set_args( request, name, type, args );
	va_end( args );
	return result;
}

iot_status_t iot_action_request_parameter_set_args(
	iot_action_request_t *request,
	const char *name,
	iot_type_t type,
	va_list args )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request && name )
	{
		result = IOT_STATUS_BAD_REQUEST;
		if ( os_strpbrk( name,
			IOT_PARAMETER_NAME_BAD_CHARACTERS ) == NULL )
		{
			struct iot_action_parameter *p = NULL;
			size_t i;
			result = IOT_STATUS_NOT_FOUND;
			for( i = 0u; i < request->parameter_count &&
				result == IOT_STATUS_NOT_FOUND; ++i )
			{
				if ( os_strncasecmp( request->parameter[i].name,
					name, IOT_NAME_MAX_LEN ) == 0 )
				{
					result = IOT_STATUS_BAD_REQUEST;
					if ( request->parameter[i].data.type == type ||
						request->parameter[i].data.type == IOT_TYPE_NULL )
					{
						p = &request->parameter[i];
						result = IOT_STATUS_SUCCESS;
					}
				}
			}

			/* support for out only parameters */
			if ( result == IOT_STATUS_NOT_FOUND )
			{
				if ( request->parameter_count < IOT_PARAMETER_MAX )
				{
					char *p_name = NULL;
					size_t name_len = os_strlen( name );
					if ( name_len > IOT_NAME_MAX_LEN )
						name_len = IOT_NAME_MAX_LEN;
#ifdef IOT_STACK_ONLY
					p = &request->parameter[request->parameter_count];
					p_name = p->_name;
#else /* ifdef IOT_STACK_ONLY */
					p = os_realloc( request->parameter,
						sizeof( struct iot_action_parameter )
						* ( request->parameter_count + 1u ) );
					p_name = os_malloc( sizeof( char ) * name_len + 1u );
					if ( p && p_name )
					{
						request->parameter = p;
						p = &request->parameter[request->parameter_count];
					}
					else
					{
						os_free_null( (void**)&p );
						os_free_null( (void**)&p_name );
					}
#endif /* else IOT_STACK_ONLY */
					if ( p && p_name )
					{
						p->name = p_name;
						os_strncpy( p_name, name, name_len );
						p_name[name_len] = '\0';
						++request->parameter_count;
					}
				}
			}

			if ( result != IOT_STATUS_BAD_REQUEST )
			{
				result = IOT_STATUS_FULL;
				 if ( p )
				 {
					p->type = IOT_PARAMETER_OUT;
					result = iot_common_arg_set(
						&p->data, IOT_TRUE, type, args );
				 }
			}
		}
	}
	return result;
}

void iot_action_request_set_status(
	struct iot_action_request *request,
	iot_status_t status,
	const char *err_msg_fmt, ... )
{
	if ( request )
	{
		va_list args;
		va_start( args, err_msg_fmt );
		if ( status != IOT_STATUS_SUCCESS )
		{
#ifdef IOT_STACK_ONLY
			request->error = request->_error
			os_vsnprintf( request->error, IOT_NAME_MAX_LEN,
				err_msg_fmt, args );
			request->error[ IOT_NAME_MAX_LEN ] = '\0';
#else
			const int err_len = os_vsnprintf( NULL, 0u,
				err_msg_fmt, args );
			va_end( args );
			if ( err_len > 0 )
			{
				char *err_msg = os_realloc(
					request->error, (unsigned int)err_len + 2u );
				if ( err_msg )
				{
					request->error = err_msg;
					va_start( args, err_msg_fmt );
					os_vsnprintf( request->error,
						(unsigned int)err_len + 1u,
						err_msg_fmt, args );
					request->error[ (unsigned int)err_len + 1u ] = '\0';
				}
			}
#endif
			IOT_LOG( request->lib, IOT_LOG_ERROR, "%s",
				request->error );
		}
		va_end( args );

		request->result = status;
	}
}

const char *iot_action_request_source(
	const iot_action_request_t *request )
{
	const char *result = NULL;
	if ( request )
		result = request->source;
	return result;
}

iot_status_t iot_action_request_status(
	const iot_action_request_t *request,
	const char **message )
{
	const char *err_msg = NULL;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( request )
	{
		result = request->result;
		if ( result != IOT_STATUS_SUCCESS )
		{
			err_msg = request->error;
			if ( !err_msg )
				err_msg = iot_error( result );
		}
	}

	if ( message )
		*message = err_msg;
	return result;
}

iot_status_t iot_action_time_limit_set(
	iot_action_t *action,
	iot_millisecond_t duration )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( action )
	{
		if ( duration == 0u )
			action->flags ^= IOT_ACTION_NO_TIME_LIMIT;
		else
			action->flags |= IOT_ACTION_NO_TIME_LIMIT;
		action->time_limit = duration;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}
