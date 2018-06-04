/**
 * @file
 * @brief source file for IoT library json decoding functionality
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

#include "app_json_schema.h"

#include "app_json_base.h"

#include <os.h>
#include <math.h>

/** @brief value for the field is required (applies to all types) */
#define APP_JSON_SCHEMA_FLAG_REQUIRED          0x1  /* 0001 */
#if 0
/** @brief whether 'maximum' value is exclusive (only applicable to numbers) */
#define APP_JSON_SCHEMA_FLAG_MAXIMUM_EXCLUSIVE 0x4  /* 0100 (number) */
/** @brief whether 'minimum' value is exclusive (only applicable to numbers) */
#define APP_JSON_SCHEMA_FLAG_MINIMUM_EXCLUSIVE 0x8  /* 1000 (number) */
#endif
/** @brief whether all items must be unique (only applicable to arrays) */
#define APP_JSON_SCHEMA_FLAG_UNIQUE            0x4  /* 0100 (array) */
/** @brief additional items are accepted (only applicable to arrays & objects) */
#define APP_JSON_SCHEMA_FLAG_ADDITIONAL        0x8 /*  1000 (array) */

/** @brief String prepending acceptable options */
#define APP_JSON_SCHEMA_ACCEPTABLE_PRE         "(acceptable values are: "
/** @brief String after acceptable options */
#define APP_JSON_SCHEMA_ACCEPTABLE_POST        ")"

/** @brief enumerator for all JSON schema keywords */
enum app_json_schema_keyword_id
{
	APP_JSON_SCHEMA_KEYWORD_ADDITIONAL_ITEMS = 0,
	APP_JSON_SCHEMA_KEYWORD_ADDITIONAL_PROPERTIES,
	APP_JSON_SCHEMA_KEYWORD_ARRAY,
	APP_JSON_SCHEMA_KEYWORD_BOOLEAN,
	APP_JSON_SCHEMA_KEYWORD_DEFAULT,
	APP_JSON_SCHEMA_KEYWORD_DESCRIPTION,
	APP_JSON_SCHEMA_KEYWORD_DEPENDENCIES,
	APP_JSON_SCHEMA_KEYWORD_ENUM,
	APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MAXIMUM,
	APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MINIMUM,
	APP_JSON_SCHEMA_KEYWORD_FORMAT,
	APP_JSON_SCHEMA_KEYWORD_INTEGER,
	APP_JSON_SCHEMA_KEYWORD_ITEMS,
	APP_JSON_SCHEMA_KEYWORD_NUMBER,
	APP_JSON_SCHEMA_KEYWORD_OBJECT,
	APP_JSON_SCHEMA_KEYWORD_STRING,
	APP_JSON_SCHEMA_KEYWORD_TYPE,
	APP_JSON_SCHEMA_KEYWORD_MAXIMUM,
	APP_JSON_SCHEMA_KEYWORD_MAXIMUM_ITEMS,
	APP_JSON_SCHEMA_KEYWORD_MAXIMUM_LENGTH,
	APP_JSON_SCHEMA_KEYWORD_MAXIMUM_PROPERTIES,
	APP_JSON_SCHEMA_KEYWORD_MINIMUM,
	APP_JSON_SCHEMA_KEYWORD_MINIMUM_ITEMS,
	APP_JSON_SCHEMA_KEYWORD_MINIMUM_LENGTH,
	APP_JSON_SCHEMA_KEYWORD_MINIMUM_PROPERTIES,
	APP_JSON_SCHEMA_KEYWORD_MULTIPLE_OF,
	APP_JSON_SCHEMA_KEYWORD_PATTERN,
	APP_JSON_SCHEMA_KEYWORD_PROPERTIES,
	APP_JSON_SCHEMA_KEYWORD_REQUIRED,
	APP_JSON_SCHEMA_KEYWORD_TITLE,
	APP_JSON_SCHEMA_KEYWORD_UNIQUE_ITEMS
};

/** @brief table of JSON schema keywords */
static const char *APP_JSON_SCHEMA_KEYWORDS[] ={
	"additionalItems",
	"additionalProperties",
	"array",
	"boolean",
	"default",
	"description",
	"dependencies",
	"enum",
	"exclusiveMinimum",
	"exclusiveMaximum",
	"format",
	"integer",
	"items",
	"number",
	"object",
	"string",
	"type",
	"maximum",
	"maxItems",
	"maxLength",
	"maxProperties",
	"minimum",
	"minItems",
	"minLength",
	"minProperties",
	"multipleOf",
	"pattern",
	"properties",
	"required",
	"title",
	"uniqueItems"
};


/**
 * @brief represents an item in the schema that defines something
 */
struct app_json_schema_item
{
	/** @brief flags for the item */
	uint8_t       flags;
	/** @brief key item */
	const app_json_item_t   *item;
	/** @brief name */
	const char        *name;
	/** @brief length of name */
	size_t            name_len;
	/** @brief index of last child */
	unsigned int      last_child;
	/** @brief parent object */
	unsigned int      parent;
	/** @brief pointer to json dependencies (can be either array or object) */
	const app_json_item_t   *dependencies;
};

/**
 * @brief base schema object
 */
struct app_json_schema
{
	/** @brief internal JSON decoder */
	app_json_decoder_t *decoder;
#ifndef IOT_STACK_ONLY
	/** @brief the root item, (for cleanup and determining index) */
	struct app_json_schema_item *root;
	/** @brief error message on the heap */
	char *error_msg;
#endif /* ifndef IOT_STACK_ONLY */
};

/**
 * @brief helper function to get the string value of a field from a
 *        schema object
 *
 * @param[in]      schema              base schema object
 * @param[in]      item                object within the schema
 * @param[in]      keyword_id          id of the keyword being sought
 * @param[in,out]  out                 pointer to string found  (NULL if not)
 * @param[in,out]  out_len             length of output string (0u if not found)
 */
static iot_status_t app_json_schema_item_string_value(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	enum app_json_schema_keyword_id keyword_id,
	const char **out,
	size_t *out_len );

static iot_status_t app_json_schema_allocate_item(
	app_json_schema_t *schema,
	const app_json_item_t *item,
	app_json_type_t item_type,
	unsigned int parent_idx,
	const char *name,
	size_t name_len,
	struct app_json_schema_item **out,
	size_t *count,
	const app_json_item_t *j_required_arr,
	const app_json_item_t *j_dependencies,
	const char **error_msg );

static iot_status_t app_json_schema_parse_schema_json(
	app_json_schema_t *schema,
	const app_json_item_t *root,
	unsigned int parent_idx,
	const char *name,
	size_t name_len,
	struct app_json_schema_item **out,
	size_t *count,
	const app_json_item_t *j_required_arr,
	const app_json_item_t *j_dependencies,
	const char **error_msg );


iot_status_t app_json_schema_allocate_item(
	app_json_schema_t *schema,
	const app_json_item_t *item,
	app_json_type_t item_type,
	unsigned int parent_idx,
	const char *name,
	size_t name_len,
	struct app_json_schema_item **out,
	size_t *count,
	const app_json_item_t *j_required_arr,
	const app_json_item_t *j_dependencies,
	const char **error_msg )
{
	struct app_json_schema_item *base = NULL;
	iot_status_t result = IOT_STATUS_NO_MEMORY;

#ifndef IOT_STACK_ONLY
	if ( schema->decoder->flags & APP_JSON_FLAG_DYNAMIC && count )
	{
		base = app_json_realloc( *out,
			sizeof( struct app_json_schema_item ) * (*count + 1u) );
		if ( base )
		{
			*out = base;
			base += *count;
		}
	}
#else
	if ( count )
		base = (struct app_json_schema_item *)(void *)(
			(char*)schema + sizeof( struct app_json_schema )) +
			(sizeof( struct app_json_schema_item ) * (*count));
#endif /* ifndef IOT_STACK_ONLY */
	if ( base && item_type != APP_JSON_TYPE_NULL )
	{
		const app_json_item_t *j_item;
		enum app_json_schema_keyword_id max_field_id =
			APP_JSON_SCHEMA_KEYWORD_MAXIMUM;
		enum app_json_schema_keyword_id min_field_id =
			APP_JSON_SCHEMA_KEYWORD_MINIMUM;
		app_json_type_t min_max_type = APP_JSON_TYPE_INTEGER;

		switch ( item_type )
		{
		case APP_JSON_TYPE_ARRAY:
			max_field_id = APP_JSON_SCHEMA_KEYWORD_MAXIMUM_ITEMS;
			min_field_id = APP_JSON_SCHEMA_KEYWORD_MINIMUM_ITEMS;
			break;
		case APP_JSON_TYPE_REAL:
			min_max_type = APP_JSON_TYPE_REAL;
			/* fall through */
		case APP_JSON_TYPE_INTEGER:
			max_field_id = APP_JSON_SCHEMA_KEYWORD_MAXIMUM;
			min_field_id = APP_JSON_SCHEMA_KEYWORD_MINIMUM;
			break;
		case APP_JSON_TYPE_OBJECT:
			max_field_id = APP_JSON_SCHEMA_KEYWORD_MAXIMUM_PROPERTIES;
			min_field_id = APP_JSON_SCHEMA_KEYWORD_MINIMUM_PROPERTIES;
			break;
		case APP_JSON_TYPE_STRING:
			max_field_id = APP_JSON_SCHEMA_KEYWORD_MAXIMUM_LENGTH;
			min_field_id = APP_JSON_SCHEMA_KEYWORD_MINIMUM_LENGTH;
			break;
		case APP_JSON_TYPE_BOOL:
		case APP_JSON_TYPE_NULL:
		default:
			break;
		}

		base->item = item;
		base->last_child = 0u;
		base->name = name;
		base->name_len = name_len;
		base->parent = parent_idx;
		base->dependencies = NULL;
		*count = *count + 1;

		/* flags */
		base->flags = 0u;
		if ( item_type == APP_JSON_TYPE_ARRAY )
		{
			const app_json_item_t *u_item;
			/* uniqueItems flag */
			u_item = app_json_decode_object_find( schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_UNIQUE_ITEMS] );
			if ( u_item && app_json_decode_type( schema->decoder,
				u_item ) != APP_JSON_TYPE_BOOL )
			{
				*error_msg = "'uniqueItems' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
			else if ( u_item )
			{
				int unique = IOT_FALSE;
				app_json_decode_bool( schema->decoder, u_item,
					&unique );
				if ( unique != IOT_FALSE )
					base->flags |= APP_JSON_SCHEMA_FLAG_UNIQUE;
			}

			/* additionalItems flag */
			u_item = app_json_decode_object_find( schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_ADDITIONAL_ITEMS] );
			if ( u_item && app_json_decode_type( schema->decoder,
				u_item ) != APP_JSON_TYPE_BOOL )
			{
				*error_msg = "'additionalItems' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
			else if ( u_item )
			{
				int unique = IOT_FALSE;
				app_json_decode_bool( schema->decoder, u_item,
					&unique );
				if ( unique != IOT_FALSE )
					base->flags |= APP_JSON_SCHEMA_FLAG_ADDITIONAL;
			}
		}
		else if ( item_type == APP_JSON_TYPE_INTEGER ||
			item_type == APP_JSON_TYPE_REAL )
		{
			const app_json_item_t *u_item;
			/* exclusiveMaximum */
			u_item = app_json_decode_object_find( schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MAXIMUM] );
			if ( u_item && app_json_decode_type( schema->decoder,
				u_item ) != APP_JSON_TYPE_BOOL )
			{
				*error_msg = "'exclusiveMaximum' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
			else if ( u_item )
			{
				int unique = IOT_FALSE;
				app_json_decode_bool( schema->decoder, u_item,
					&unique );
				if ( unique != IOT_FALSE )
					base->flags |= APP_JSON_SCHEMA_FLAG_UNIQUE;
			}

			/* exclusiveMinimum */
			u_item = app_json_decode_object_find( schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MINIMUM] );
			if ( u_item && app_json_decode_type( schema->decoder,
				u_item ) != APP_JSON_TYPE_BOOL )
			{
				*error_msg = "'exclusiveMinimum' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
			else if ( u_item )
			{
				int unique = IOT_FALSE;
				app_json_decode_bool( schema->decoder, u_item,
					&unique );
				if ( unique != IOT_FALSE )
					base->flags |= APP_JSON_SCHEMA_FLAG_ADDITIONAL;
			}
		}

		if ( item_type == APP_JSON_TYPE_ARRAY )
		{
			j_item = app_json_decode_object_find(
				schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_ITEMS] );
			if ( j_item && app_json_decode_type( schema->decoder,
				j_item ) != APP_JSON_TYPE_ARRAY )
			{
				*error_msg = "'items' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}

			j_item = app_json_decode_object_find(
				schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_ADDITIONAL_ITEMS] );
			if ( j_item && app_json_decode_type( schema->decoder,
				j_item ) != APP_JSON_TYPE_BOOL )
			{
				*error_msg = "'additionalItems' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
		}
		else if ( item_type == APP_JSON_TYPE_STRING )
		{
			j_item = app_json_decode_object_find(
				schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_PATTERN] );
			if ( j_item && app_json_decode_type( schema->decoder,
				j_item ) != APP_JSON_TYPE_STRING )
			{
				*error_msg = "'pattern' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}

			j_item = app_json_decode_object_find(
				schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_FORMAT] );
			if ( j_item && app_json_decode_type( schema->decoder,
				j_item ) != APP_JSON_TYPE_STRING )
			{
				*error_msg = "'format' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
		}
		else if ( item_type == APP_JSON_TYPE_INTEGER ||
			item_type == APP_JSON_TYPE_REAL )
		{
			j_item = app_json_decode_object_find(
				schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_MULTIPLE_OF] );
			if ( j_item && app_json_decode_type( schema->decoder,
				j_item ) != item_type )
			{
				*error_msg = "'multipleOf' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
		}
		else if ( item_type == APP_JSON_TYPE_OBJECT )
		{
			j_item = app_json_decode_object_find(
				schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_ADDITIONAL_PROPERTIES] );
			if ( j_item && app_json_decode_type( schema->decoder,
				j_item ) != APP_JSON_TYPE_BOOL &&
				app_json_decode_type( schema->decoder,
				j_item ) != APP_JSON_TYPE_OBJECT )
			{
				*error_msg = "'additionalProperties' is not of correct type";
				result = IOT_STATUS_BAD_REQUEST;
			}
		}

		result = IOT_STATUS_SUCCESS;

		j_item = app_json_decode_object_find(
			schema->decoder, item,
			APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_DEFAULT] );
		/* default */
		if ( j_item && app_json_decode_type( schema->decoder,
			j_item ) != item_type )
		{
			*error_msg = "'default' is not of correct type";
			result = IOT_STATUS_BAD_REQUEST;
		}

		/* description */
		j_item = app_json_decode_object_find(
			schema->decoder, item,
			APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_DESCRIPTION] );
		if ( j_item && app_json_decode_type( schema->decoder,
			j_item ) != APP_JSON_TYPE_STRING )
		{
			*error_msg = "'description' is not a string";
			result = IOT_STATUS_BAD_REQUEST;
		}

		/* maximum */
		j_item = app_json_decode_object_find(
			schema->decoder, item,
			APP_JSON_SCHEMA_KEYWORDS[max_field_id] );
		if ( j_item && app_json_decode_type( schema->decoder,
			j_item ) != min_max_type )
		{
			*error_msg = "item maximum is of wrong type";
			result = IOT_STATUS_BAD_REQUEST;
		}

		/* minimum */
		j_item = app_json_decode_object_find(
			schema->decoder, item,
			APP_JSON_SCHEMA_KEYWORDS[min_field_id] );
		if ( j_item && app_json_decode_type( schema->decoder,
			j_item ) != min_max_type )
		{
			*error_msg = "item minimum is of wrong type";
			result = IOT_STATUS_BAD_REQUEST;
		}

		/* title */
		j_item = app_json_decode_object_find(
			schema->decoder, item,
			APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_TITLE] );
		if ( j_item && app_json_decode_type( schema->decoder,
			j_item ) != APP_JSON_TYPE_STRING )
		{
			*error_msg = "'title' is not a string";
			result = IOT_STATUS_BAD_REQUEST;
		}

		/* enum */
		j_item = app_json_decode_object_find(
			schema->decoder, item,
			APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_ENUM] );
		if ( j_item && app_json_decode_type( schema->decoder,
			j_item ) != APP_JSON_TYPE_ARRAY )
		{
			*error_msg = "'enum' is not an array";
			result = IOT_STATUS_BAD_REQUEST;
		}
		else if ( j_item && item_type != APP_JSON_TYPE_OBJECT )
		{
			const app_json_array_iterator_t *iter =
				app_json_decode_array_iterator( schema->decoder,
					j_item );
			while ( result == IOT_STATUS_SUCCESS && iter )
			{
				const app_json_item_t *enum_item = NULL;
				app_json_decode_array_iterator_value(
					schema->decoder, j_item, iter, &enum_item );
				if ( app_json_decode_type( schema->decoder, enum_item )
					!= item_type )
				{
					*error_msg = "'enum' contains invalid item";
					result = IOT_STATUS_BAD_REQUEST;
				}
				iter = app_json_decode_array_iterator_next(
					schema->decoder, j_item, iter );
			}
		}

		/* handle 'dependencies' */
		if ( name && name_len > 0u && j_dependencies )
		{
			if ( app_json_decode_type( schema->decoder,
				j_dependencies ) == APP_JSON_TYPE_OBJECT )
			{
				j_item = app_json_decode_object_find_len(
					schema->decoder, j_dependencies,
					name, name_len );

				if ( j_item )
					base->dependencies = j_item;
			}
			else
				*error_msg = "'dependencies' should be an object";
		}

		/* handle 'required' flag */
		if ( name && name_len > 0u && j_required_arr )
		{
			int found_item = IOT_FALSE;
			const app_json_array_iterator_t *arr_iter;

			arr_iter = app_json_decode_array_iterator(
				schema->decoder, j_required_arr );
			while ( result == IOT_STATUS_SUCCESS &&
				found_item == IOT_FALSE && arr_iter )
			{
				const app_json_item_t *obj = NULL;

				result = app_json_decode_array_iterator_value(
					schema->decoder,
					j_required_arr,
					arr_iter, &obj );

				if ( result == IOT_STATUS_SUCCESS && obj )
				{
					const char *req = NULL;
					size_t req_len = 0u;
					result = app_json_decode_string(
						schema->decoder,
						obj, &req, &req_len );

					/* item is in the array */
					if ( req && os_strncmp( name, req, req_len ) == 0 )
					{
						base->flags |= APP_JSON_SCHEMA_FLAG_REQUIRED;
						found_item = IOT_TRUE;
					}
				}
				else
					*error_msg = "'required' should be an array of strings";
				arr_iter = app_json_decode_array_iterator_next(
					schema->decoder, j_required_arr, arr_iter );
			}
		}

		/* handle 'properties' for objects */
		if ( item_type == APP_JSON_TYPE_OBJECT )
		{
			unsigned int idx = (unsigned int)(*count - 1u);
			const app_json_item_t *j_properties;
			const app_json_item_t *j_required;
			const app_json_item_t *j_deps;
			const app_json_object_iterator_t *iter;
			struct app_json_schema_item *parent_obj;

			j_properties = app_json_decode_object_find( schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_PROPERTIES] );
			j_required = app_json_decode_object_find( schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_REQUIRED] );
			j_deps = app_json_decode_object_find( schema->decoder, item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_DEPENDENCIES] );

			/* iterate through all properties in the object */
			iter = app_json_decode_object_iterator( schema->decoder, j_properties );
			while( result == IOT_STATUS_SUCCESS && iter )
			{
				size_t key_len = 0u;
				const char *key = NULL;
				const app_json_item_t *value;
				result = app_json_decode_object_iterator_key(
					schema->decoder, j_properties, iter, &key, &key_len );
				result = app_json_decode_object_iterator_value(
					schema->decoder, j_properties, iter, &value );
				if ( result == IOT_STATUS_SUCCESS )
				{
					result = app_json_schema_parse_schema_json(
						schema, value, idx,
						key, key_len, out, count,
						j_required, j_deps, error_msg );
				}
				else
					*error_msg = "object doesn't contain value";
				iter = app_json_decode_object_iterator_next(
					schema->decoder, j_properties, iter );
			}
#ifndef IOT_STACK_ONLY
			parent_obj = *out + idx;
#else
			parent_obj = (struct app_json_schema_item *)(void *)(
				(char*)schema + sizeof( struct app_json_schema )) + idx;
#endif
			parent_obj->last_child = (unsigned int)(*count);
		}
	}
	return result;
}

iot_status_t app_json_schema_parse(
	app_json_schema_t *schema,
	const char *js,
	size_t len,
	app_json_schema_item_t **root,
	char *error,
	size_t error_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( schema && root )
	{
		const app_json_item_t *root_item = NULL;
		result = app_json_decode_parse( schema->decoder, js,
			len, &root_item, error, error_len );

		if ( result == IOT_STATUS_SUCCESS && root_item )
		{
			struct app_json_schema_item *r = NULL;
			size_t count = 0u;
			const char *error_msg = NULL;
			result = app_json_schema_parse_schema_json(
				schema, root_item, (unsigned int)-1, NULL, 0u,
				&r, &count, NULL, NULL, &error_msg );
			if ( result == IOT_STATUS_SUCCESS && r )
			{
#ifndef IOT_STACK_ONLY
				schema->root = r;
#endif /* ifndef IOT_STACK_ONLY */
				*root = r;
			}
			else
			{
				if ( error && error_len > 0u )
				{
					size_t i = 0;
					if ( !error_msg )
						error_msg = "no root object";
					while ( i < error_len && error_msg[i] )
					{
						error[i] = error_msg[i];
						++i;
					}

					if ( i == error_len )
						--i;
					error[i] = '\0';
				}
				result = IOT_STATUS_PARSE_ERROR;
			}
		}
	}
	return result;
}

iot_status_t app_json_schema_parse_schema_json(
	app_json_schema_t *schema,
	const app_json_item_t *root,
	unsigned int parent_idx,
	const char *name,
	size_t name_len,
	struct app_json_schema_item **out,
	size_t *count,
	const app_json_item_t *j_required_arr,
	const app_json_item_t *j_dependencies,
	const char **error_msg )
{
	const app_json_item_t *j_type;
	iot_status_t result;
	const char *value = NULL;
	size_t value_len = 0u;

	j_type = app_json_decode_object_find( schema->decoder, root, "type" );
	result = app_json_decode_string( schema->decoder, j_type, &value, &value_len );
	if ( result == IOT_STATUS_SUCCESS )
	{
		if ( os_strncmp( value,
			APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_ARRAY ],
			value_len ) == 0 )
		{
			result = app_json_schema_allocate_item(
				schema, root, APP_JSON_TYPE_ARRAY, parent_idx,
				name, name_len, out, count, j_required_arr,
				j_dependencies, error_msg );
		}
		else if ( os_strncmp( value,
			APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_OBJECT ],
			value_len ) == 0 )
		{
			result = app_json_schema_allocate_item(
				schema, root, APP_JSON_TYPE_OBJECT, parent_idx,
				name, name_len, out, count, j_required_arr,
				j_dependencies, error_msg );
		}
		else if ( *count > 0u &&
			os_strncmp( value,
			APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_BOOLEAN ],
			value_len ) == 0 )
		{
			result = app_json_schema_allocate_item(
				schema, root, APP_JSON_TYPE_BOOL, parent_idx,
				name, name_len, out, count, j_required_arr,
				j_dependencies, error_msg );
		}
		else if ( *count > 0u &&
			os_strncmp( value,
			APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_INTEGER ],
			value_len ) == 0 )
		{
			result = app_json_schema_allocate_item(
				schema, root, APP_JSON_TYPE_INTEGER, parent_idx,
				name, name_len, out, count, j_required_arr,
				j_dependencies, error_msg );
		}
		else if ( *count > 0u &&
			os_strncmp( value,
			APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_NUMBER ],
			value_len ) == 0 )
		{
			result = app_json_schema_allocate_item(
				schema, root, APP_JSON_TYPE_REAL, parent_idx,
				name, name_len, out, count, j_required_arr,
				j_dependencies, error_msg );
		}
		else if ( *count > 0u &&
			os_strncmp( value,
			APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_STRING ],
			value_len ) == 0 )
		{
			result = app_json_schema_allocate_item(
				schema, root, APP_JSON_TYPE_STRING, parent_idx,
				name, name_len, out, count, j_required_arr,
				j_dependencies, error_msg );
		}
	}
	else
		*error_msg = "'type' field not found";
	return result;
}

int app_json_schema_array(
	const app_json_schema_t *UNUSED(schema),
	const app_json_schema_item_t *UNUSED(item),
	const char *UNUSED(value),
	size_t UNUSED(value_len),
	const char **error_msg )
{
	if ( error_msg )
		*error_msg = "array validation not implemented";
	return IOT_FALSE;
}

int app_json_schema_bool(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg )
{
	iot_bool_t result = IOT_FALSE;
	const struct app_json_schema_item *i =
		(const struct app_json_schema_item *)item;
	if ( schema && i && value &&
		app_json_schema_type( schema, item ) == APP_JSON_TYPE_BOOL )
	{
		result = IOT_TRUE;
		/* check if specified */
		if ( !value || value_len == 0u || *value == '\0' )
		{
			if ( i->flags & APP_JSON_SCHEMA_FLAG_REQUIRED )
			{
				if ( error_msg )
					*error_msg = "value is required";
				result = IOT_FALSE;
			}
		}
		else
		{
			const char *const true_values[] =
				{ "y", "yes", "t", "true", "on", "1", NULL };
			const char *const false_values[] =
				{ "n", "no", "f", "false", "off", "0", NULL };
			size_t j;

			result = IOT_FALSE;

			/* check for true */
			j = 0u;
			while ( result == IOT_FALSE && true_values[j] )
			{
				if ( os_strncasecmp( value, true_values[j],
					value_len ) == 0 )
					result = IOT_TRUE;
				++j;
			}

			/* check for false */
			j = 0u;
			while ( result == IOT_FALSE && false_values[j] )
			{
				if ( os_strncasecmp( value, false_values[j],
					value_len ) == 0 )
					result = IOT_TRUE;
				++j;
			}

			if ( result == IOT_FALSE && error_msg )
				*error_msg = "invalid boolean value "
					APP_JSON_SCHEMA_ACCEPTABLE_PRE
					"y, n, yes, no, t, f, true, false, "
					"on, off, 1 or 0"
					APP_JSON_SCHEMA_ACCEPTABLE_POST;
		}
	}
	else if ( error_msg )
		*error_msg = "invalid object";
	return result;
}

int app_json_schema_dependencies_achieved(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char *const *keys_set,
	size_t keys_set_len )
{
	iot_bool_t result = IOT_FALSE;
	const struct app_json_schema_item *i =
		(const struct app_json_schema_item *)item;
	if ( schema && i && i->name && i->name_len > 0u )
	{
		const app_json_item_t *const j_deps = i->dependencies;
		result = IOT_TRUE;
		if ( j_deps )
		{
			size_t j;
			const app_json_type_t type = app_json_decode_type(
				schema->decoder, j_deps );

			result = IOT_FALSE;
			for ( j = 0u; j < keys_set_len && result == IOT_FALSE; ++j )
			{
				const app_json_item_t *j_str = NULL;
				const app_json_array_iterator_t *arr_it = NULL;

				if ( type == APP_JSON_TYPE_ARRAY )
				{
					arr_it =
						app_json_decode_array_iterator(
						schema->decoder, j_deps );

					app_json_decode_array_iterator_value(
						schema->decoder, j_deps,
						arr_it, &j_str );
				}
				else if ( type == APP_JSON_TYPE_STRING )
					j_str = j_deps;

				while ( result == IOT_FALSE && j_str )
				{
					const char *v = NULL;
					size_t v_len = 0u;
					if ( app_json_decode_string(
						schema->decoder,
						j_str, &v, &v_len ) == IOT_STATUS_SUCCESS )
					{
						if ( os_strncmp( keys_set[j],
							v, v_len ) == 0 )
							result = IOT_TRUE;
					}

					j_str = NULL;
					if ( arr_it )
					{
						arr_it = app_json_decode_array_iterator_next(
							schema->decoder, j_deps, arr_it );
						app_json_decode_array_iterator_value(
							schema->decoder, j_deps,
							arr_it, &j_str );
					}
				}
			}
		}
	}
	return result;
}

iot_status_t app_json_schema_description(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char **description,
	size_t *description_len )
{
	return app_json_schema_item_string_value( schema, item,
		APP_JSON_SCHEMA_KEYWORD_DESCRIPTION, description,
		description_len );
}

iot_status_t app_json_schema_format(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char **format,
	size_t *format_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( schema && item )
	{
		if ( app_json_schema_type( schema, item ) == APP_JSON_TYPE_STRING )
		{
			result = app_json_schema_item_string_value( schema, item,
				APP_JSON_SCHEMA_KEYWORD_FORMAT, format,
				format_len );
		}
	}
	return result;
}

app_json_schema_t *app_json_schema_initialize(
	void *buf,
	size_t len,
	unsigned int flags )
{
	app_json_schema_t *result = NULL;
#ifndef IOT_STACK_ONLY
	if ( !buf || ( flags & APP_JSON_FLAG_DYNAMIC ) )
	{
		result = app_json_realloc( NULL,
			sizeof( struct app_json_schema ) );
	}
#endif /* ifndef IOT_STACK_ONLY */
	if ( !result && buf && len > sizeof( struct app_json_schema ) )
	{
		result = (app_json_schema_t*)buf;
		buf = (char*)buf + sizeof( struct app_json_schema );
		len -= sizeof( struct app_json_schema );

#if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC )
		len = len * sizeof( jsmntok_t ) /
			sizeof( struct app_json_schema_item );
#endif /* if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC ) */
	}

	if ( result )
	{
		result->decoder = app_json_decode_initialize( buf, len, flags );
#ifndef IOT_STACK_ONLY
		result->root = NULL;
		result->error_msg = NULL;
#endif /* ifndef IOT_STACK_ONLY */
		if ( result->decoder == NULL )
		{
#ifndef IOT_STACK_ONLY
			if ( !buf || ( flags & APP_JSON_FLAG_DYNAMIC ) )
				app_json_free( result );
#endif /* ifndef IOT_STACK_ONLY */
			result = NULL;
		}
	}
	return result;
}

int app_json_schema_integer(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg )
{
	iot_bool_t result = IOT_FALSE;
	const struct app_json_schema_item *i =
		(const struct app_json_schema_item *)item;
	if ( schema && i &&
		app_json_schema_type( schema, item ) == APP_JSON_TYPE_INTEGER )
	{
		result = IOT_TRUE;
		/* check if specified */
		if ( !value || value_len == 0u || *value == '\0' )
		{
			if ( i->flags & APP_JSON_SCHEMA_FLAG_REQUIRED )
			{
				if ( error_msg )
					*error_msg = "value is required";
				result = IOT_FALSE;
			}
		}
		else
		{
			iot_int64_t int_value = 0L;

			/* convert value to integer */
			while ( result == IOT_TRUE && value_len > 0u )
			{
				if ( value[value_len - 1u] >= '0' &&
					value[value_len - 1u] <= '9' )
					int_value = int_value * 10 +
						(value[value_len - 1u] - '0');
				else if ( value_len > 1u ||
					( value[value_len - 1u] != '+' &&
					  value[value_len - 1u] != '-' ) )
				{
					if ( error_msg )
						*error_msg = "invalid number";
					result = IOT_FALSE;
				}
				--value_len;
			}
			if ( value[0] == '-' )
				int_value *= int_value;

			if ( result == IOT_TRUE )
			{
				int exclusive_maximum = IOT_FALSE;
				int exclusive_minimum = IOT_FALSE;
				const app_json_item_t *j_item;
				/* exclusiveMaximum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MAXIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_BOOL )
				{
					app_json_decode_bool( schema->decoder, j_item,
						&exclusive_maximum );
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'exclusiveMaximum' value";
					result = IOT_FALSE;
				}

				/* exclusiveMininmum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MINIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_BOOL )
				{
					app_json_decode_bool( schema->decoder, j_item,
						&exclusive_minimum );
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'exclusiveMinimum' value";
					result = IOT_FALSE;
				}

				/* maxmimum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MAXIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_INTEGER )
				{
					iot_int64_t maximum = 0L;
					app_json_decode_integer( schema->decoder, j_item, &maximum );
					if ( (int_value > maximum) ||
					     (exclusive_maximum != IOT_FALSE && int_value == maximum) )
					{
						if ( error_msg )
							*error_msg =  "value is greater than maximum";
						result = IOT_FALSE;
					}
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'maximum' value";
					result = IOT_FALSE;
				}

				/* minimum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MINIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_INTEGER )
				{
					iot_int64_t minimum = 0L;
					app_json_decode_integer( schema->decoder, j_item, &minimum );
					if ( (int_value < minimum) ||
					     (exclusive_minimum != IOT_FALSE && int_value == minimum) )
					{
						if ( error_msg )
							*error_msg =  "value is less than minimum";
						result = IOT_FALSE;
					}
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'minimum' value";
					result = IOT_FALSE;
				}

				/* multipleOf */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MULTIPLE_OF] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_INTEGER )
				{
					iot_int64_t multiple_of = 0L;
					app_json_decode_integer( schema->decoder, j_item, &multiple_of );
					if ( int_value % multiple_of != 0L )
					{
						if ( error_msg )
							*error_msg =  "value is not a valid multiple";
						result = IOT_FALSE;
					}
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'multipleOf' value";
					result = IOT_FALSE;
				}
			}
		}
	}
	else if ( error_msg )
		*error_msg = "invalid object";
	return result;
}

iot_status_t app_json_schema_item_string_value(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	enum app_json_schema_keyword_id keyword_id,
	const char **out,
	size_t *out_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	const struct app_json_schema_item *i =
		 (const struct app_json_schema_item *)item;
	const char* o = NULL;
	size_t o_len = 0u;
	if ( schema && item )
	{
		const app_json_item_t *j_item;
		result = IOT_STATUS_NOT_FOUND;

		j_item = app_json_decode_object_find( schema->decoder, i->item,
			APP_JSON_SCHEMA_KEYWORDS[keyword_id] );
		if ( j_item )
		{
			result = app_json_decode_string(
				schema->decoder, j_item, &o, &o_len );
		}
	}
	if ( out )
		*out = o;
	if ( out_len )
		*out_len = o_len;
	return result;
}

app_json_schema_object_iterator_t *
	app_json_schema_object_iterator(
		app_json_schema_t *schema,
		app_json_schema_item_t *item )
{
	app_json_schema_object_iterator_t *result = NULL;
	struct app_json_schema_item *i =
		(struct app_json_schema_item *)item;
	if ( schema && i &&
		app_json_decode_type( schema->decoder, i->item ) == APP_JSON_TYPE_OBJECT )
	{
		const app_json_item_t *j_obj;
		const char *value = NULL;
		size_t value_len = 0u;

		j_obj = app_json_decode_object_find( schema->decoder, i->item, "type" );
		app_json_decode_string( schema->decoder, j_obj, &value, &value_len );

		/* ensure that it is a "type": "object" */
		if ( value && os_strncmp( value, "object", value_len ) == 0 )
		{
			size_t children;
#ifndef IOT_STACK_ONLY
			unsigned int p_idx = (unsigned int)(i - schema->root);
#else
			unsigned int p_idx =
				((unsigned int)((char*)item - (char*)schema) -
					sizeof( struct app_json_schema ))
					/ sizeof( struct app_json_schema_item );
#endif /* ifndef IOT_STACK_ONLY */
			j_obj = app_json_decode_object_find( schema->decoder, i->item,
				"properties" );
			children = app_json_decode_object_size( schema->decoder, j_obj );

			if ( children > 0u )
			{
				while ( i->parent != p_idx )
					++i;
				result = (app_json_schema_object_iterator_t *)(i);
			}
		}
	}
	return result;
}

iot_status_t app_json_schema_object_iterator_key(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	app_json_schema_object_iterator_t *iter,
	const char **key,
	size_t *key_len )
{
	const char *k = NULL;
	size_t k_len = 0u;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct app_json_schema_item *const it =
		(struct app_json_schema_item *)iter;
	if ( schema  && item && it )
	{
		k = it->name;
		k_len = it->name_len;
		result = IOT_STATUS_SUCCESS;
	}
	if ( key )
		*key = k;
	if ( key_len )
		*key_len = k_len;
	return result;
}

app_json_schema_object_iterator_t *
	app_json_schema_object_iterator_next(
		app_json_schema_t *schema,
		app_json_schema_item_t *item,
		app_json_schema_object_iterator_t *iter )
{
	app_json_schema_object_iterator_t *result = NULL;
	struct app_json_schema_item *i =
		(struct app_json_schema_item *)item;
	struct app_json_schema_item *it =
		(struct app_json_schema_item *)iter;
	if ( schema && i && it &&
		app_json_decode_type( schema->decoder, i->item ) == APP_JSON_TYPE_OBJECT )
	{
		const app_json_item_t *j_obj;
		const char *value = NULL;
		size_t value_len = 0u;

		j_obj = app_json_decode_object_find( schema->decoder, i->item, "type" );
		app_json_decode_string( schema->decoder, j_obj, &value, &value_len );

		/* ensure that it is a "type": "object" */
		if ( value && os_strncmp( value, "object", value_len ) == 0 )
		{
			const unsigned int p_idx = it->parent;
			const unsigned int last_child = i->last_child;
#ifndef IOT_STACK_ONLY
			unsigned int c_idx = (unsigned int)(it - schema->root);
#else
			unsigned int c_idx =
				((unsigned int)((char*)iter - (char*)schema) -
					sizeof( struct app_json_schema ))
					/ sizeof( struct app_json_schema_item );
#endif /* ifndef IOT_STACK_ONLY */

			if ( c_idx < last_child )
			{
				do {
					++it;
					++c_idx;
				} while ( c_idx < last_child && it->parent != p_idx );

				if ( c_idx < last_child )
					result = (app_json_schema_object_iterator_t *)(it);
			}
		}
	}
	return result;
}

iot_status_t app_json_schema_object_iterator_value(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	app_json_schema_object_iterator_t *iter,
	app_json_schema_item_t **out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	struct app_json_schema_item *const it =
		(struct app_json_schema_item *)iter;
	struct app_json_schema_item *i = NULL;

	if ( schema && item && it && out )
	{
		i = (struct app_json_schema_item *)it;
		result = IOT_STATUS_SUCCESS;
	}

	if ( out )
		*out = i;
	return result;
}

int app_json_schema_number(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg )
{
	return app_json_schema_real( schema, item, value, value_len, error_msg );
}

int app_json_schema_real(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg )
{
	iot_bool_t result = IOT_FALSE;
	const struct app_json_schema_item *i =
		(const struct app_json_schema_item *)item;
	if ( schema && i &&
		app_json_schema_type( schema, item ) == APP_JSON_TYPE_REAL )
	{
		result = IOT_TRUE;

		/* check if specified */
		if ( !value || value_len == 0u || *value == '\0' )
		{
			if ( i->flags & APP_JSON_SCHEMA_FLAG_REQUIRED )
			{
				if ( error_msg )
					*error_msg = "value is required";
				result = IOT_FALSE;
			}
		}
		else
		{
			int exclusive_maximum = IOT_FALSE;
			int exclusive_minimum = IOT_FALSE;
			const app_json_item_t *j_item;
			double real_value = 0.0;

			/* parse value from string */
			int is_neg = IOT_FALSE;
			int is_neg_e = IOT_FALSE;
			unsigned long denom = 0u; /* false */
			unsigned int d_count = 0u; /* decimal count */
			int e = 0u; /* exponent portion */
			int has_e = IOT_FALSE;
			size_t j;

			for ( j = 0u; j < value_len && result != IOT_FALSE; ++j )
			{
				const char c = value[j];
				if ( c >= '0' && c <= '9' )
				{
					if ( has_e )
						e = (e * 10) + (c - '0');
					else if ( denom >= 10u )
					{
						real_value = real_value +
							(double)((double)(c - '0') / (double)denom);
						denom *= 10u;
						++d_count;
					}
					else
						real_value = (real_value * 10) + (c - '0');
				}
				else if ( c == '-' && j == 0u )
					is_neg = IOT_TRUE;
				else if ( c == '.' && j != 0u && denom == 0u )
					denom = 10u;
				else if ( ( c == 'e' || c == 'E' ) && j != 0u && has_e == IOT_FALSE )
					has_e = IOT_TRUE;
				else if ( c == '-' && has_e == IOT_TRUE && is_neg_e == IOT_FALSE )
					is_neg_e = IOT_TRUE;
				else if ( c != '+' )
					result = IOT_FALSE;
			}

			/* handle exponent in number */
			/* exponent was a negative */
			if ( is_neg_e )
				e *= -1;

			if ( e < 0 )
				for ( ; e < 0; ++e )
					real_value /= 10.0;
			else
			{
				int e2 = e;
				for ( ; e2 > 0; --e2 )
					real_value *= 10.0;

				/* exponent is larger than decimal places, so let's remove any
				 * rounding error, and ensure it's an exact integer */
				if ( d_count <= (unsigned int)e )
					real_value = (double)((long)(real_value + 0.5));
			}

			if ( result == IOT_FALSE )
			{
				if ( error_msg )
					*error_msg = "invalid number";
				real_value = 0.0;
			}
			else if ( is_neg )
				real_value *= -1.0;

			if ( result != IOT_FALSE )
			{
				/* exclusiveMaximum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MAXIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_BOOL )
				{
					app_json_decode_bool( schema->decoder, j_item,
						&exclusive_maximum );
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'exclusiveMaximum' value";
					result = IOT_FALSE;
				}

				/* exclusiveMininmum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_EXCLUSIVE_MINIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_BOOL )
				{
					app_json_decode_bool( schema->decoder, j_item,
						&exclusive_minimum );
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'exclusiveMinimum' value";
					result = IOT_FALSE;
				}

				/* maxmimum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MAXIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_REAL )
				{
					double maximum = 0;
					app_json_decode_number( schema->decoder, j_item, &maximum );
					if ( (exclusive_maximum == IOT_FALSE && real_value > maximum) ||
					     (exclusive_maximum != IOT_FALSE && real_value >= maximum) )
					{
						if ( error_msg )
							*error_msg =  "value is greater than maximum";
						result = IOT_FALSE;
					}
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'maximum' value";
					result = IOT_FALSE;
				}

				/* minimum */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MINIMUM] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_REAL )
				{
					double minimum = 0;
					app_json_decode_number( schema->decoder, j_item, &minimum );
					if ( (exclusive_minimum == IOT_FALSE && real_value < minimum) ||
					     (exclusive_minimum != IOT_FALSE && real_value <= minimum) )
					{
						if ( error_msg )
							*error_msg =  "value is less than minimum";
						result = IOT_FALSE;
					}
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'minimum' value";
					result = IOT_FALSE;
				}

				/* multipleOf */
				j_item = app_json_decode_object_find(
					schema->decoder, i->item,
					APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MULTIPLE_OF] );
				if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
					APP_JSON_TYPE_REAL )
				{
					double multiple_of = 0.0;
					app_json_decode_number( schema->decoder, j_item, &multiple_of );
					if( fabs( remainder(real_value, multiple_of)) > (1E-9 * multiple_of))
					{
						if ( error_msg )
							*error_msg =  "value is not a valid multiple";
						result = IOT_FALSE;
					}
				}
				else if ( j_item )
				{
					if ( error_msg )
						*error_msg =  "invalid 'multipleOf' value";
					result = IOT_FALSE;
				}
			}
		}
	}
	else if ( error_msg )
		*error_msg = "invalid object";
	return result;
}

int app_json_schema_required(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item )
{
	iot_bool_t result = IOT_FALSE;
	if ( schema && item )
	{
		const struct app_json_schema_item *i =
			(const struct app_json_schema_item *)item;
		if ( i->flags & APP_JSON_SCHEMA_FLAG_REQUIRED )
			result = IOT_TRUE;
	}
	return result;
}

int app_json_schema_string(
	app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg )
{
	iot_bool_t result = IOT_FALSE;
	const struct app_json_schema_item *i =
		(const struct app_json_schema_item *)item;
	if ( schema && i && value &&
		app_json_schema_type( schema, item ) == APP_JSON_TYPE_STRING )
	{
		const app_json_item_t *j_item;
		iot_int64_t length;

		result = IOT_TRUE;

		/* check if specified */
		if ( !value || value_len == 0u || *value == '\0' )
		{
			if ( i->flags & APP_JSON_SCHEMA_FLAG_REQUIRED )
			{
				if ( error_msg )
					*error_msg = "value is required";
				result = IOT_FALSE;
			}
		}
		else
		{
			/* enum */
			j_item = app_json_decode_object_find(
				schema->decoder, i->item,
				APP_JSON_SCHEMA_KEYWORDS[APP_JSON_SCHEMA_KEYWORD_ENUM] );
			if ( j_item && app_json_decode_type( schema->decoder, j_item ) ==
				APP_JSON_TYPE_ARRAY )
			{
				const app_json_array_iterator_t *it =
					app_json_decode_array_iterator( schema->decoder, j_item );
				result = IOT_FALSE;
				while ( result == IOT_FALSE && it )
				{
					const app_json_item_t *ar_item = NULL;
					if ( app_json_decode_array_iterator_value(
						schema->decoder, j_item, it, &ar_item )
						== IOT_STATUS_SUCCESS && ar_item )
					{
						const char *str = NULL;
						size_t str_len = 0u;
						if ( app_json_decode_string( schema->decoder,
							ar_item, &str, &str_len ) == IOT_STATUS_SUCCESS &&
							os_strncmp( value, str, str_len ) == 0 )
						{
							result = IOT_TRUE;
						}
					}
					it = app_json_decode_array_iterator_next( schema->decoder, j_item, it );
				}
				if ( result == IOT_FALSE && error_msg )
				{
#ifndef IOT_STACK_ONLY
					const size_t add_info_post_len =
						os_strlen( APP_JSON_SCHEMA_ACCEPTABLE_POST );
					size_t msg_len;
					char *error_msg_buf;
#endif /* ifndef IOT_STACK_ONLY */
					*error_msg = "value not in accepted list";
#ifndef IOT_STACK_ONLY
					msg_len = os_strlen( *error_msg )
						+ os_strlen( APP_JSON_SCHEMA_ACCEPTABLE_PRE )
						+ add_info_post_len + 1u;
					error_msg_buf = os_malloc( msg_len + 1u );
					if ( error_msg_buf )
					{
						int add_comma = IOT_FALSE;
						os_snprintf( error_msg_buf,
							msg_len, "%s %s",
							*error_msg,
							APP_JSON_SCHEMA_ACCEPTABLE_PRE );

						/* todo loop through additional options */
						it = app_json_decode_array_iterator(
							schema->decoder, j_item );
						while ( error_msg_buf && it )
						{
							const app_json_item_t *ar_item = NULL;
							const char *str = NULL;
							size_t str_len = 0u;
							if ( app_json_decode_array_iterator_value(
								schema->decoder, j_item, it, &ar_item )
								== IOT_STATUS_SUCCESS && ar_item &&
							 	app_json_decode_string( schema->decoder,
								ar_item, &str, &str_len ) == IOT_STATUS_SUCCESS )
							{
								size_t new_len =
									msg_len + str_len +
									(size_t)(add_comma * 2);
								error_msg_buf = os_realloc(
									error_msg_buf, new_len + 1u );
								if ( error_msg_buf )
								{
									if ( add_comma != IOT_FALSE )
									{
										os_strncpy(
											&error_msg_buf[msg_len - add_info_post_len],
											", ", 2u );
										msg_len += 2u;
									}

									os_strncpy( &error_msg_buf[msg_len - add_info_post_len],
										str, str_len );
									msg_len = new_len;
									add_comma = IOT_TRUE;
								}
							}
							it = app_json_decode_array_iterator_next(
								schema->decoder, j_item, it );
						}

						if ( schema->error_msg )
							os_free( schema->error_msg );
						if ( error_msg_buf )
						{
							os_strncpy(
								&error_msg_buf[msg_len - add_info_post_len],
								APP_JSON_SCHEMA_ACCEPTABLE_POST,
								add_info_post_len );
							error_msg_buf[ msg_len ] = '\0';
							schema->error_msg = error_msg_buf;
							*error_msg = schema->error_msg;
						}
					}
#endif /* ifndef IOT_STACK_ONLY */
				}
			}
			else if ( j_item )
			{
				if ( error_msg )
					*error_msg =  "invalid 'enum' array";
				result = IOT_FALSE;
			}

			/* maxLength */
			length = 0L;
			j_item = app_json_decode_object_find(
				schema->decoder, i->item,
				APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MAXIMUM_LENGTH] );
			if ( j_item && app_json_decode_integer(
				schema->decoder, j_item, &length ) == IOT_STATUS_SUCCESS )
			{
				if ( length < 0 || value_len > (size_t)length )
				{
					if ( error_msg )
						*error_msg =  "string is too long";
					result = IOT_FALSE;
				}
			}
			else if ( j_item )
			{
				if ( error_msg )
					*error_msg =  "invalid 'maximumItem' value";
				result = IOT_FALSE;
			}

			/* minLength */
			length = 0L;
			j_item = app_json_decode_object_find(
				schema->decoder, i->item,
				APP_JSON_SCHEMA_KEYWORDS[ APP_JSON_SCHEMA_KEYWORD_MINIMUM_LENGTH] );
			if ( j_item && app_json_decode_integer(
				schema->decoder, j_item, &length ) == IOT_STATUS_SUCCESS )
			{
				if ( length < 0L || value_len < (size_t)length )
				{
					if ( error_msg )
						*error_msg =  "string is too short";
					result = IOT_FALSE;
				}
			}
			else if ( j_item )
			{
				if ( error_msg )
					*error_msg =  "invalid 'minimumItem' value";
				result = IOT_FALSE;
			}

			/** @todo check format */
			/** @todo check pattern */
		}
	}
	else if ( error_msg )
		*error_msg = "invalid object";
	return result;
}

iot_status_t app_json_schema_title(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item,
	const char **title,
	size_t *title_len )
{
	return app_json_schema_item_string_value( schema, item,
		APP_JSON_SCHEMA_KEYWORD_TITLE, title, title_len );
}

void app_json_schema_terminate(
	app_json_schema_t *schema )
{
	if ( schema )
	{
		app_json_decoder_t *decoder = schema->decoder;
#ifndef IOT_STACK_ONLY
		if ( decoder &&
			( decoder->flags & APP_JSON_FLAG_DYNAMIC ) )
		{
			if ( schema->root )
				app_json_free( schema->root );
			if ( schema->error_msg )
				app_json_free( schema->error_msg );
			app_json_free( schema );
		}
#endif /* ifndef IOT_STACK_ONLY */
		app_json_decode_terminate( decoder );
	}
}

app_json_type_t app_json_schema_type(
	const app_json_schema_t *schema,
	const app_json_schema_item_t *item )
{
	app_json_type_t result = APP_JSON_TYPE_NULL;
	const struct app_json_schema_item *i =
		(const struct app_json_schema_item *)item;
	if ( i )
	{
		const char *value = NULL;
		size_t value_len = 0u;
		const app_json_item_t *j_type =
			app_json_decode_object_find( schema->decoder,
				i->item, "type" );

		if ( app_json_decode_string( schema->decoder, j_type, &value, &value_len )
			== IOT_STATUS_SUCCESS && value )
		{
			if ( os_strncmp( value, "array", value_len ) == 0 )
				result = APP_JSON_TYPE_ARRAY;
			else if ( os_strncmp( value, "object", value_len ) == 0 )
				result = APP_JSON_TYPE_OBJECT;
			else if ( os_strncmp( value, "boolean", value_len ) == 0 )
				result = APP_JSON_TYPE_BOOL;
			else if ( os_strncmp( value, "integer", value_len ) == 0 )
				result = APP_JSON_TYPE_INTEGER;
			else if ( os_strncmp( value, "number", value_len ) == 0 )
				result = APP_JSON_TYPE_REAL;
			else if ( os_strncmp( value, "string", value_len ) == 0 )
				result = APP_JSON_TYPE_STRING;
		}
	}
	return result;
}

