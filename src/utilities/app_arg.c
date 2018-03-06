/**
 * @file
 * @brief source file for argument parsing functionality for an application
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

#include "app_arg.h"

#include <os.h>

/** @brief Prefix to use for short argument id */
#define APP_ARG_PREFIX_SHORT           '-'
/** @brief Prefix to use for long argument ids */
#define APP_ARG_PREFIX_LONG            "--"
/** @brief Character to use to split between a key & value pairs */
#define APP_ARG_VALUE_SPLIT            '='
/** @brief Defult parameter name to use if not specified */
#define APP_ARG_DEFAULT_PARAMETER_NAME "value"

unsigned int app_arg_count( const struct app_arg *args, char ch,
	const char *name )
{
	unsigned int result = 0u;
	int found = 0;

	while ( !found && args && ( args->ch || args->name ) )
	{
		if ( ( ch != 0 && ch == args->ch ) ||
			( name && args->name &&
		 os_strcmp( name, args->name ) == 0 ) )
		{
			result = args->hit;
			found = 1;
		}
		++args;
	}
	return result;
}

app_arg_iterator_t *app_arg_find(
	int argc,
	char **argv,
	app_arg_iterator_t *iter,
	char ch,
	const char *name )
{
	struct app_arg_iterator *rv = NULL;
	if ( iter )
	{
		iter->idx = 0;
		iter->ch = ch;
		iter->name = name;
		rv = app_arg_find_next( argc, argv, iter );
	}
	return rv;
}

struct app_arg_iterator *app_arg_find_next(
	int argc,
	char **argv,
	struct app_arg_iterator *iter )
{
	struct app_arg_iterator *rv = NULL;
	if ( iter && argv )
	{
		const size_t arg_prefix_long_len = os_strlen( APP_ARG_PREFIX_LONG );
		int match_found = 0;
		int cnt = iter->idx + 1;
		int no_key_count = 0;

		while ( cnt < argc && !match_found && no_key_count < 2 )
		{
			const char *c = argv[cnt];
			if ( c && os_strncmp( c, APP_ARG_PREFIX_LONG,
				arg_prefix_long_len ) == 0 )
			{
				c += arg_prefix_long_len;
				if ( *c == '\0' ) /* handle "--" case */
					no_key_count = 2u;
				else
				{
					const char *p;
					p = os_strchr( c, APP_ARG_VALUE_SPLIT );
					if ( iter->name )
					{
						if ( p )
						{
							if ( os_strncmp( iter->name,
								c, (size_t)(p - c) ) == 0 )
								match_found = 1;
						}
						else if ( os_strncmp( iter->name,
							c, os_strlen(iter->name) ) == 0 )
							match_found = 1;
					}
					else if ( iter->ch == '\0' ) /* return everything */
						match_found = 1;
				}
			}
			else if ( c && *c == APP_ARG_PREFIX_SHORT )
			{
				if ( iter->ch != '\0' )
				{
					if ( *(c+1) == iter->ch )
						++match_found;
				}
				else if ( !iter->name ) /* return everything */
					++match_found;
			}
			else
				++no_key_count;

			if ( !match_found && no_key_count < 2 )
				++cnt;
		}

		/* a match was found! */
		iter->idx = cnt;
		if ( match_found )
			rv = iter;
	}
	return rv;
}

int app_arg_iterator_key(
	int argc,
	char **argv,
	const app_arg_iterator_t *iter,
	size_t *key_len,
	const char **key )
{
	int rv = 0;
	if ( iter && iter->idx < argc )
	{
		const size_t arg_prefix_long_len = os_strlen( APP_ARG_PREFIX_LONG );
		const char *key_out = argv[iter->idx];
		size_t key_len_out = 0u;

		if ( key_out )
		{
			if ( os_strncmp( key_out, APP_ARG_PREFIX_LONG,
				arg_prefix_long_len ) == 0 )
			{
				const char *p;
				key_out += arg_prefix_long_len;
				p = os_strchr( key_out, APP_ARG_VALUE_SPLIT );
				if ( p )
					key_len_out = (size_t)(p - key_out);
				else
					key_len_out = os_strlen( key_out );
			}
			else if ( key_out[0u] == APP_ARG_PREFIX_SHORT &&
				  key_out[1u] != '\0' &&
				  key_out[1u] != APP_ARG_VALUE_SPLIT )
			{
				key_len_out = 1u;
				++key_out;
			}
		}

		/* return results */
		if ( key_len_out > 0u )
		{
			if ( key )
				*key = key_out;
			if ( key_len )
				*key_len = key_len_out;
			rv = 1;
		}
	}
	return rv;
}

int app_arg_iterator_value(
	int argc,
	char **argv,
	const app_arg_iterator_t *iter,
	size_t *value_len,
	const char **value )
{
	int rv = 0;
	if ( iter && iter->idx < argc )
	{
		const size_t arg_prefix_long_len =
			os_strlen( APP_ARG_PREFIX_LONG );
		const char *value_out = argv[iter->idx];
		size_t value_len_out = 0u;

		if ( value_out )
		{
			if ( os_strncmp( value_out, APP_ARG_PREFIX_LONG,
				arg_prefix_long_len ) == 0 ||
				*value_out == APP_ARG_PREFIX_SHORT )
			{
				if ( os_strncmp( value_out, APP_ARG_PREFIX_LONG,
					arg_prefix_long_len ) == 0 )
				{
					const char *p;
					value_out += arg_prefix_long_len;
					p = os_strchr( value_out, APP_ARG_VALUE_SPLIT );
					if ( p )
						value_out = p + 1;
					else
						value_out = NULL;
				}
				else
				{
					value_out += 2u;
					if ( *value_out == APP_ARG_VALUE_SPLIT )
						++value_out;
					else if ( *value_out == '\0' )
						value_out = NULL;
				}

				if ( !value_out && iter->idx + 1 < argc )
				{
					value_out = argv[iter->idx + 1];
					if ( value_out && (
						*value_out == APP_ARG_PREFIX_SHORT ||
						os_strncmp( value_out,
							APP_ARG_PREFIX_LONG,
							arg_prefix_long_len ) == 0 ))
						value_out = NULL;
				}
			}

			if ( value_out )
				value_len_out = os_strlen( value_out );
		}

		/* return results */
		if ( value_len_out > 0u )
		{
			if ( value )
				*value = value_out;
			if ( value_len )
				*value_len = value_len_out;
			rv = 1;
		}
	}
	return rv;
}

int app_arg_parse( struct app_arg *args, int argc, char **argv,
	int *pos )
{
	const size_t app_arg_prefix_long_len = os_strlen( APP_ARG_PREFIX_LONG );
	struct app_arg *arg = args;
	app_arg_iterator_t iter;
	app_arg_iterator_t *iter_ptr;
	int result = EXIT_SUCCESS;

	while ( arg && ( arg->ch || arg->name ) )
	{
		arg->hit = 0u;
		++arg;
	}

	iter_ptr = app_arg_find( argc, argv, &iter, '\0', NULL );
	while ( iter_ptr && result == EXIT_SUCCESS )
	{
		const char *a = argv[iter_ptr->idx];
		const char *key = NULL;
		size_t key_len = 0u;

		if ( app_arg_iterator_key( argc, argv, iter_ptr,
			&key_len, &key ) )
		{
			int found = 0;

			arg = args;
			while ( arg && ( arg->ch || arg->name ) && !found )
			{
				if ( os_strncmp( a, APP_ARG_PREFIX_LONG,
					app_arg_prefix_long_len ) == 0 )
				{
					if ( os_strncmp( arg->name, key,
						key_len ) == 0 )
						++found;
				}
				else if ( *a == APP_ARG_PREFIX_SHORT )
				{
					if ( arg->ch == *key )
						++found;
				}

				if ( !found )
					++arg;
			}

			if ( found )
			{
				++arg->hit;
				if ( arg->hit > 1u && !(arg->flags & APP_ARG_FLAG_MULTI ) )
				{
					os_fprintf( OS_STDERR,
						"argument defined multiple "
						"times: %.*s\n",
						(int)key_len, key );
					result = EXIT_FAILURE;
				}
				else
				{
					const char *value = NULL;
					size_t value_len = 0u;
					if ( app_arg_iterator_value( argc, argv,
						iter_ptr, &value_len, &value ) )
					{
						if ( !arg->param &&
						     !arg->param_result &&
						     !(arg->flags & APP_ARG_FLAG_PARAM_OPTIONAL) )
						{
							os_fprintf( OS_STDERR,
								"unexpected value \"%.*s\" "
								"for argument argument: %.*s\n",
								(int)value_len, value,
								(int)key_len, key );
							result = EXIT_FAILURE;
						}
						else if ( arg->param_result )
							*arg->param_result = value;
					}
					else if ( !(arg->flags & APP_ARG_FLAG_PARAM_OPTIONAL) &&
						( arg->param || arg->param_result ) )
					{
						os_fprintf( OS_STDERR,
							"expected value for "
							"argument: %.*s\n",
							(int)key_len, key );
						result = EXIT_FAILURE;
					}
				}
			}
			else
			{
				os_fprintf( OS_STDERR,
					"unknown argument: %.*s\n",
						(int)key_len, key );
				result = EXIT_FAILURE;
			}
		}
		else
		{
			os_fprintf( OS_STDERR,
				"failed to parse argument: %s\n", a );
			result = EXIT_FAILURE;
		}
		iter_ptr = app_arg_find_next( argc, argv, iter_ptr );
	}

	/* check for required arguments */
	arg = args;
	while ( result == EXIT_SUCCESS && arg && ( arg->ch || arg->name ) )
	{
		if ( !arg->hit && !(arg->flags & APP_ARG_FLAG_OPTIONAL))
		{
			os_fprintf( OS_STDERR,"%s",
				"required argument not specified: " );
			if ( arg->ch )
				os_fprintf( OS_STDERR, "%c%c\n",
					APP_ARG_PREFIX_SHORT, arg->ch );
			else if ( arg->name )
				os_fprintf( OS_STDERR, "%s%s\n",
					APP_ARG_PREFIX_LONG, arg->name );
			result = EXIT_FAILURE;
		}
		++arg;
	}
	if ( result == EXIT_SUCCESS && (!pos && iter.idx < argc) )
	{
		os_fprintf( OS_STDERR, "unknown argument: %s\n",
			argv[iter.idx] );
		result = EXIT_FAILURE;
	}
	else if ( result == EXIT_SUCCESS && pos )
		*pos = iter.idx;
	return result;
}

void app_arg_usage( const struct app_arg *args, size_t col,
	const char *app, const char *desc, const char *pos,
	const char *pos_desc )
{
	unsigned int i;
	int has_type[] = { 0, 0 };
	const struct app_arg *arg;
	const char *app_name = "exec";
	size_t pos_len = 0u;
	if ( app )
	{
		app_name = os_strrchr( app, '/' );
		if ( app_name )
			++app_name;
		else
			app_name = app;
	}
	os_printf( "usage: %s", app_name );

	arg = args;
	while ( arg && ( arg->ch || arg->name ) )
	{
		const char *param_name = NULL;
		int optional = arg->flags & APP_ARG_FLAG_OPTIONAL;
		unsigned int show2items = 0;
		if ( !(arg->flags & APP_ARG_FLAG_OPTIONAL) &&
			arg->flags & APP_ARG_FLAG_MULTI )
			show2items = 1;

		/* calculate the size of the parameter id tag */
		if ( arg->param || (arg->flags & APP_ARG_FLAG_PARAM_OPTIONAL) || arg->param_result )
		{
			param_name = APP_ARG_DEFAULT_PARAMETER_NAME;
			if ( arg->param )
				param_name = arg->param;
		}
		for ( i = 0; i <= show2items; ++i )
		{
			os_printf( "%s", " " );
			if ( optional || (i > 0) )
			{
				os_printf( "%s", "[" );
				++has_type[1];
			}
			else
				++has_type[0];
			if ( arg->ch )
				os_printf( "%c%c", APP_ARG_PREFIX_SHORT, arg->ch );
			else
				os_printf( "%s%s", APP_ARG_PREFIX_LONG, arg->name );
			if ( param_name )
			{
				os_printf( "%s", " " );
				if ( arg->flags & APP_ARG_FLAG_PARAM_OPTIONAL )
					os_printf( "%s", "[" );
				os_printf( "%s", param_name );
				if ( arg->flags & APP_ARG_FLAG_PARAM_OPTIONAL )
					os_printf( "%s", "]" );
			}
			if ( optional || (i > 0) )
				os_printf( "%s", "]" );
			if ( arg->flags & APP_ARG_FLAG_MULTI &&
				(show2items == 0u || (i > 0) ) )
				os_printf( "+" );
		}
		++arg;
	}

	/* handle positional argument display */
	if ( pos )
	{
		int pos_opt = 0;
		int pos_multi = 0;
		pos_len = os_strlen( pos );
		if ( pos[pos_len - 1u] == '+' )
		{
			--pos_len;
			pos_multi = 1;
		}
		if ( *pos == '[' )
		{
			++pos;
			pos_opt = 1;
			pos_len -= 2u;
		}
		os_printf( "%s", " " );
		if ( !pos_opt )
		{
			os_printf( "%.*s", (int)pos_len, pos );
			if ( pos_multi )
				os_printf( "%s", " " );
		}
		if ( pos_multi || pos_opt )
			os_printf( "[%.*s", (int)pos_len, pos );
		if ( pos_multi )
			os_printf( "%s", " ..." );
		if ( pos_multi || pos_opt )
			os_printf( "%s", "]" );
	}
	os_printf( "%s", "\n" );
	if ( desc )
		os_printf( "\n%s\n", desc );
	if ( pos )
	{
		os_printf( "\npositional arguments:\n%-.*s%*s",
			(int)pos_len, pos, (int)(col - pos_len), "" );
		if ( pos_desc )
			os_printf( "%s", pos_desc );
		os_printf( "%s", "\n" );
	}

	if ( col ) --col;
	for ( i = 0u; i < 2u; ++i )
	{
		if ( has_type[i] )
		{
			os_printf( "%s", "\n" );
			if ( i )
				os_printf( "%s", "optional arguments:\n" );
			else
				os_printf( "%s", "required arguments:\n" );
			arg = args;
			while ( arg && ( arg->ch || arg->name ) )
			{
				if ( ( i && (arg->flags & APP_ARG_FLAG_OPTIONAL) )
				|| ( !i && !(arg->flags & APP_ARG_FLAG_OPTIONAL) ) )
				{
					size_t id_len = 0u;
					size_t line_len = 0u;
					const char *param_name = NULL;
					/* calculate the size of the parameter id tag */
					if ( arg->param || (arg->flags & APP_ARG_FLAG_PARAM_OPTIONAL) || arg->param_result )
					{
						param_name = APP_ARG_DEFAULT_PARAMETER_NAME;
						if ( arg->param )
							param_name = arg->param;
					}
					if ( param_name )
					{
						id_len = col;
						if ( arg->ch )
							id_len -= 3u; /* "-c " */
						if ( arg->name )
							id_len -=
							  ( os_strlen( arg->name ) + 3u ); /* "--name " */
						if ( arg->ch && arg->name )
						{
							id_len -= 2u; /* ", " */
							id_len /= 2u;
						}
						if ( id_len > os_strlen( param_name ) ) /* arg*/
							id_len = os_strlen( param_name );
					}
					if ( arg->ch )
					{
						os_printf( "%c%c",
							APP_ARG_PREFIX_SHORT,
							arg->ch );
						line_len = 2u;
						if ( param_name )
						{
							os_printf( " %*.*s",
								(int)id_len,
								(int)id_len,
								param_name );
							line_len += id_len + 1u;
						}
						if ( arg->name )
						{
							os_printf( "%s", ", " );
							line_len += 2u;
						}
					}
					if ( arg->name )
					{
						size_t max_name_len = col - line_len - 2u;
						if ( param_name )
							max_name_len -= id_len - 1u; /* " " */
						os_printf( "%s%.*s",
							APP_ARG_PREFIX_LONG,
							(int)max_name_len,
							arg->name );
						if ( os_strlen( arg->name )
							< max_name_len )
							line_len +=
								os_strlen( arg->name ) + 2u;
						else
							line_len += max_name_len + 2u;

						if ( param_name )
						{
							os_printf( " %*.*s",
								(int)id_len,
								(int)id_len,
								param_name );
							line_len += id_len + 1u;
						}
					}
					if ( line_len < col )
					{
						line_len = col - line_len;
						os_printf( "%*.*s",
							(int)line_len,
							(int)line_len, " " );
					}
					if ( arg->desc )
						os_printf( " %s",
							arg->desc );
					os_printf( "%s", "\n" );
				}
				++arg;
			}
		}
	}
}

