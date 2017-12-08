/**
 * @file
 * @brief source file for IoT library json decoding functionality
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

#include "../public/iot_json.h"

#include "iot_json_base.h"

#include <os.h>

#if defined( IOT_JSON_JANSSON ) || defined( IOT_JSON_JSONC )
/**
 * @brief Maximum supportable json depth
 */
#define JSON_MAX_DEPTH                10u

#if defined( IOT_JSON_JSONC )
typedef struct json_object json_t;
#endif /* ifdefined( IOT_JSON_JSONC ) */

/**
 * @brief object encapsulating the encode and decode functions
 */
struct iot_json_encoder
{
	unsigned int depth;              /**< @brief current depth */
	unsigned int flags;              /**< @brief output flags */
	json_t **j_cur;                  /**< @brief current object for each level */
	char *output;                    /**< @brief saved dumped string */
};

/**
 * @brief helper function for encoding primative types with jansson
 *
 * @param[in]      encoder             JSON encoder object
 * @param[in]      key                 key for the new object
 * @param[in]      obj                 new object (steals the reference)
 */
static IOT_SECTION iot_status_t iot_json_encode_key(
	iot_json_encoder_t *encoder,
	const char *key,
	json_t *obj );

#else /* defined( IOT_JSON_JSMN ) */
/** @brief Maximum output depth */
typedef unsigned long iot_json_encode_struct_t;

/**
 * @brief An error occurred during adding a new item, so don't encode
 */
#define JSON_STRUCT_BITS                 3       /* # of bits to define struct */

/**
 * @brief Maximum supportable json depth
 */
#define JSON_MAX_DEPTH                 ((sizeof(iot_json_encode_struct_t)* 8)/JSON_STRUCT_BITS)

/**
 * @brief internal structure for composing JSON messages (16 bytes)
 */
struct iot_json_encoder
{
	char *buf;                       /**< @brief start of write buffer */
	char *cur;                       /**< @brief position in write buffer */
	unsigned int flags;              /**< @brief output flags */
	size_t len;                      /**< @brief size of JSON buffer */
	iot_json_encode_struct_t structs;    /**< @brief array of structures */
};

/** @brief JSON tokens for the start of objects & arrays */
static const char JSON_CHARS_START[] = { '[', '{', '{' };
/** @brief JSON tokens for the end of objects & arrays */
static const char JSON_CHARS_END[] = { ']', '}', '}' };

/**
 * @brief determines the current depth of structures at the current position
 *
 * @param[in]      encoder             JSON encoder object
 *
 * @return the depth at the current position
 */
static IOT_SECTION unsigned int iot_json_encode_depth(
	const iot_json_encoder_t *encoder );

/**
 * @brief calculated the number number of printable characters in an integer
 *
 * @param[in]      i                   integer to calculate number of characters
 * @param[in]      neg                 add characters for negative number
 *
 * @return number of printable characters in the number
 */
static IOT_SECTION size_t iot_json_encode_intlen(
	iot_uint64_t i,
	iot_bool_t neg );

/**
 * @brief helper function to start a new object
 *
 * @param[in,out]  encoder             JSON encoder object
 * @param[in]      key                 (optional) key for the new item
 * @param[in]      value_len           length of the value for the new object
 * @param[out]     added_parent        (optional) whether this call required
 *                                     adding a new parent JSON object to ensure
 *                                     the output remains a valid JSON string
 */
static IOT_SECTION iot_status_t iot_json_encode_key(
	iot_json_encoder_t *encoder,
	const char *key,
	size_t value_len,
	iot_bool_t *added_parent );

/**
 * @brief helper function to calculate the length of a string for encoding
 *        in JSON
 *
 * @note This function handles adding counts for extra characters required to
 *       black-slash characters in JSON string
 *
 * @param[in]      str                 sring to get length of
 *
 * @return the length of the string in characters (adding for escape characters)
 */
static IOT_SECTION size_t iot_json_encode_strlen(
	const char* str );

/**
 * @brief copies the string in src to the destination buffer in JSON format
 *
 * @note This function handles adding escape characters into the destination
 *       buffer when copying the string
 *
 * @param[in,out]  dest                destination buffer
 * @param[in]      src                 source buffer
 * @param[in]      num                 size of the destination buffer
 *
 * @return a pointer to the destination buffer
 */
static IOT_SECTION char *iot_json_encode_strncpy(
	char *dest,
	const char *src,
	size_t num );

/**
 * @brief helper function for starting a new JSON object or array structure
 *
 * @param[in,out]  encoder             JSON encoder object
 * @param[in]      s                   type of object to start
 */
static IOT_SECTION iot_status_t iot_json_encode_struct_end(
	iot_json_encoder_t *encoder,
	iot_json_type_t s );

/**
 * @brief helper function for ending a new object or array json structure
 *
 * @param[in,out]  encoder             JSON encoder object
 * @param[in]      s                   type of object to start
 */
static iot_status_t iot_json_encode_struct_start(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_json_type_t s );
#endif /* defined( IOT_JSON_JSMN ) */

#if defined( IOT_JSON_JANSSON ) || defined( IOT_JSON_JSONC )
iot_status_t iot_json_encode_key(
	iot_json_encoder_t *encoder,
	const char *key,
	json_t *obj )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( encoder && obj )
	{
#if defined( IOT_JSON_JANSSON )
		json_type parent_type = JSON_NULL;
#elif defined( IOT_JSON_JSONC )
		json_type parent_type = json_type_null;
#endif
		json_t *j_parent = NULL;
		result = IOT_STATUS_FULL;
		if ( encoder->depth > 0u )
		{
			j_parent = encoder->j_cur[encoder->depth - 1u];
#if defined( IOT_JSON_JANSSON )
			parent_type = json_typeof( j_parent );
#elif defined( IOT_JSON_JSONC )
			parent_type = json_object_get_type( j_parent );
#endif
		}
		else if ( !key &&
#if defined( IOT_JSON_JANSSON )
			( json_typeof( obj ) != JSON_ARRAY &&
			  json_typeof( obj ) != JSON_OBJECT )
#elif defined( IOT_JSON_JSONC )
			( json_object_is_type( obj, json_type_array ) == FALSE &&
			  json_object_is_type( obj, json_type_object ) == FALSE )
#endif
			)
		{
			/* root element can only be an array of object */
			result = IOT_STATUS_BAD_REQUEST;
		}

		/* trying to add past the maximum allowed depth */
		if ( result == IOT_STATUS_FULL &&
			encoder->depth < JSON_MAX_DEPTH )
		{
			unsigned int items_to_add = 0u;
			json_t **ptr = encoder->j_cur;

#if defined( IOT_JSON_JANSSON )
			if ( ( key && parent_type != JSON_OBJECT ) || !encoder->j_cur )
				++items_to_add;

			if ( json_is_array( obj ) || json_is_object( obj ) )
				++items_to_add;
#elif defined( IOT_JSON_JSONC )
			if ( ( key && parent_type != json_type_object ) || !encoder->j_cur )
				++items_to_add;

			if ( json_object_is_type( obj, json_type_array ) != FALSE ||
			     json_object_is_type( obj, json_type_object ) != FALSE )
				++items_to_add;
#endif

			result = IOT_STATUS_SUCCESS;
			if ( items_to_add > 0u )
			{
				ptr = (json_t**)iot_json_realloc( encoder->j_cur,
					sizeof(json_t*) * (encoder->depth + items_to_add + 1) );
				result = IOT_STATUS_NO_MEMORY;
				if ( ptr )
				{
					encoder->j_cur = ptr;
#if defined( IOT_JSON_JANSSON )
					if ( key && parent_type != JSON_OBJECT )
					{
						json_t *new_parent = json_object();
						if ( parent_type == JSON_ARRAY )
							json_array_append_new(
								j_parent, new_parent );
#elif defined( IOT_JSON_JSONC )
					if ( key && parent_type != json_type_object )
					{
						struct json_object *new_parent = json_object_new_object();
						if ( parent_type == json_type_array )
							json_object_array_add(
								j_parent, new_parent );
#endif
						j_parent = new_parent;
						encoder->j_cur[encoder->depth] = j_parent;
						++encoder->depth;
					}

					/* add the object */
					if ( j_parent || !key )
					{
						encoder->j_cur[encoder->depth] = obj;
						++encoder->depth;
						result = IOT_STATUS_SUCCESS;
					}
				}
			}

			if ( result == IOT_STATUS_SUCCESS )
			{
#if defined( IOT_JSON_JANSSON )
				if ( json_is_object( j_parent ) )
				{
					if ( !key )
						key = "";
					json_object_set_new( j_parent, key, obj );
				}
				else if ( json_is_array( j_parent ) )
					json_array_append_new( j_parent, obj );
#elif defined( IOT_JSON_JSONC )
				if ( json_object_is_type( j_parent, json_type_object ) != FALSE )
				{
					if ( !key )
						key = "";
					json_object_object_add( j_parent, key, obj );
				}
				else if ( json_object_is_type( j_parent, json_type_array ) != FALSE )
					json_object_array_add( j_parent, obj );
#endif
			}
		}
	}

	if ( result != IOT_STATUS_SUCCESS )
	{
#if defined( IOT_JSON_JANSSON )
		json_decref( obj );
#elif defined( IOT_JSON_JSONC )
		json_object_put( obj );
#endif /* if defined( IOT_JSON_JSONC ) */
	}
	return result;
}
#endif /* if defined( IOT_JSON_JANSSON ) */

iot_status_t iot_json_encode_array_end(
	iot_json_encoder_t *encoder )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
#if defined( IOT_JSON_JANSSON ) || defined( IOT_JSON_JSONC )
	if ( encoder )
	{
		result = IOT_STATUS_BAD_REQUEST;
		if ( encoder->depth > 0u )
		{
			json_t *const j_obj = encoder->j_cur[encoder->depth - 1u];
#if defined( IOT_JSON_JANSSON )
			if ( json_typeof( j_obj ) == JSON_ARRAY )
#elif defined( IOT_JSON_JSONC )
			if ( json_object_is_type( j_obj, json_type_array ) != FALSE )
#endif
			{
				--encoder->depth;
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
#else /* defined( IOT_JSON_JSMN ) */
	result = iot_json_encode_struct_end( encoder, IOT_JSON_TYPE_ARRAY );
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

iot_status_t iot_json_encode_array_start(
	iot_json_encoder_t *encoder,
	const char *key )
{
	iot_status_t result;
#if defined( IOT_JSON_JANSSON )
	result = iot_json_encode_key( encoder, key, json_array() );
#elif defined( IOT_JSON_JSONC )
	result = iot_json_encode_key( encoder, key, json_object_new_array() );
#else /* defined( IOT_JSON_JSMN ) */
	result = iot_json_encode_struct_start( encoder, key, IOT_JSON_TYPE_ARRAY );
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

iot_status_t iot_json_encode_bool(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_bool_t value )
{
	iot_status_t result;
#if defined( IOT_JSON_JANSSON )
	result = iot_json_encode_key( encoder, key, json_boolean( value ) );
#elif defined( IOT_JSON_JSONC )
	result = iot_json_encode_key( encoder, key, json_object_new_boolean( value ) );
#else /* defined( IOT_JSON_JSMN ) */
	/* can't add boolean as root element */
	if ( !key && ( encoder && encoder->structs == 0u ) )
		result = IOT_STATUS_BAD_REQUEST;
	else
	{
		iot_bool_t added_parent = IOT_FALSE;
		const size_t value_len = (size_t)(5 + ( value ? -1 : 0 ));
		result = iot_json_encode_key( encoder, key, value_len + 2u, &added_parent );
		if ( result == IOT_STATUS_SUCCESS )
		{
			if ( value )
				os_strncpy( encoder->cur, "true", value_len );
			else
				os_strncpy( encoder->cur, "false", value_len );
			encoder->cur += value_len;

			if ( added_parent )
				result = iot_json_encode_struct_end( encoder,
					IOT_JSON_TYPE_OBJECT << 1u );
		}
	}
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

#if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC )
unsigned int iot_json_encode_depth( const iot_json_encoder_t *encoder )
{
	unsigned int i = 0u;
	iot_json_encode_struct_t j = encoder->structs;
	while ( j )
	{
		if ( j & 0x1 ) ++i;
		j >>= 1;
	}
	return i;
}
#endif /* !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC ) */

const char *iot_json_encode_dump(
	iot_json_encoder_t *encoder )
{
	const char *result = NULL;
#if defined( IOT_JSON_JANSSON )
	/* setup jansson flags */
	size_t flags = JSON_PRESERVE_ORDER;
	if ( encoder && !( encoder->flags & IOT_JSON_FLAG_EXPAND ) )
		flags |= JSON_COMPACT;
	if ( encoder && encoder->flags >> IOT_JSON_INDENT_OFFSET )
		flags |= JSON_INDENT( encoder->flags >> IOT_JSON_INDENT_OFFSET );

	/* if previous dump free memory */
	if ( encoder && encoder->output )
	{
#ifdef IOT_STACK_ONLY
		json_free_t free_fn = os_free;
#if JANSSON_VERSION_HEX >= 0x020800
		json_get_alloc_funcs( NULL, &free_fn );
#endif /* JANSSON_VERSION_HEX >= 0x020800 */
		if ( free_fn )
			free_fn( encoder->output );
#else /* ifdef IOT_STACK_ONLY */
		iot_json_free( encoder->output );
#endif /* else IOT_STACK_ONLY */
		encoder->output = NULL;
	}

	if ( encoder && encoder->j_cur )
		result = encoder->output = json_dumps( encoder->j_cur[0u], flags );
#elif defined( IOT_JSON_JSONC )
	if ( encoder )
	{
		int flags = JSON_C_TO_STRING_PLAIN;
		if ( encoder->output )
		{
			os_free( encoder->output );
			encoder->output = NULL;
		}

		if ( encoder->flags & IOT_JSON_FLAG_EXPAND )
			flags |= JSON_C_TO_STRING_SPACED;

		if ( encoder->flags >> IOT_JSON_INDENT_OFFSET )
			flags |= JSON_C_TO_STRING_PRETTY;

		if ( encoder->j_cur && encoder->j_cur[0u] )
		{
			const char *const output =
				json_object_to_json_string_ext(
					encoder->j_cur[0u], flags );
			if ( output )
			{
				const size_t len = os_strlen( output );
				encoder->output = os_malloc( len + 1u );
				if ( encoder->output )
					os_strncpy( encoder->output, output,
						len + 1u );
				result = encoder->output;
			}
		}
	}
#else /* defined( IOT_JSON_JSMN ) */
	if ( encoder && encoder->buf && *encoder->buf != '\0' )
	{
		/* complete any open objects in the output string */
		char *p_cur = encoder->cur;
		if ( p_cur )
		{
			unsigned int indent = (encoder->flags >> IOT_JSON_INDENT_OFFSET);
			unsigned int depth = iot_json_encode_depth( encoder );
			iot_json_encode_struct_t s = encoder->structs;

			while ( s )
			{
				int i;
				for ( i = 0; i < JSON_STRUCT_BITS; ++i )
				{
					if ( s & ((iot_json_encode_struct_t)(1u) << i))
					{
						/* no need to check for running of
						 * of buffer as check was done when
						 * adding struct */
						const char end_ch = JSON_CHARS_END[i];
						if ( indent )
						{
							unsigned int j = indent * (depth - 1);
							if ( *(p_cur - 1) != JSON_CHARS_START[0] &&
							     *(p_cur - 1) != JSON_CHARS_START[1] )
							{
								*p_cur++ = '\n';

								while ( j )
								{
									*p_cur++ = ' ';
									--j;
								}
							}
							--depth;
						}
						*p_cur++ = end_ch;
					}
				}
				s >>= JSON_STRUCT_BITS;
			}
			*p_cur = '\0';
			result = encoder->buf;
		}
	}
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

iot_json_encoder_t *iot_json_encode_initialize(
	void *buf,
	size_t len,
	unsigned int flags )
{
	struct iot_json_encoder *encoder = NULL;

#if defined( IOT_JSON_JANSSON ) || defined( IOT_JSON_JSONC )
	size_t extra_space = 0u;
#else /* defined( IOT_JSON_JSMN ) */
	size_t extra_space = 1u; /* for '\0' */
#endif /* defined( IOT_JSON_JSMN ) */

#ifndef IOT_STACK_ONLY
	if ( !buf )
		flags |= IOT_JSON_FLAG_DYNAMIC;
#endif /* ifndef IOT_STACK_ONLY */

	if (
#ifndef IOT_STACK_ONLY
	     ( flags & IOT_JSON_FLAG_DYNAMIC ) ||
#endif /* ifndef IOT_STACK_ONLY */
	     ( buf && len >= sizeof(struct iot_json_encoder) + extra_space ) )
	{
#ifndef IOT_STACK_ONLY
		if ( flags & IOT_JSON_FLAG_DYNAMIC )
		{
			encoder = (struct iot_json_encoder *)iot_json_realloc(
				NULL, sizeof(struct iot_json_encoder) + extra_space );
			if ( encoder )
			{
#if defined( IOT_JSON_JANSSON ) || defined( IOT_JSON_JSONC )
				os_memzero( encoder, sizeof( struct iot_json_encoder ) );
#else /* defined ( IOT_JSON_JSMN ) */
				encoder->buf = NULL;
				encoder->cur = NULL;
				encoder->len = 0u;
#endif /* defined (IOT_JSON_JSMN ) */
			}
		}
		else
		{
#endif /* ifndef IOT_STACK_ONLY */
			encoder = (struct iot_json_encoder*)(buf);
#if defined( IOT_JSON_JANSSON ) || defined( IOT_JSON_JSONC )
			os_memzero( encoder, sizeof( struct iot_json_encoder ) );
#else /* defined( IOT_JSON_JSMN ) */
			encoder->buf = (char*)buf + sizeof(struct iot_json_encoder);
			encoder->cur = NULL;
			encoder->len = len - sizeof(struct iot_json_encoder);
#endif /* defined( IOT_JSON_JSMN ) */
#ifndef IOT_STACK_ONLY
		}

		if ( encoder )
		{
#endif /* ifndef IOT_STACK_ONLY */
#if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC )
			encoder->structs = 0u;
#endif /* if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC ) */
			encoder->flags = flags;
#ifndef IOT_STACK_ONLY
		}
#endif /* ifndef IOT_STACK_ONLY */
	}
	return encoder;
}

iot_status_t iot_json_encode_integer(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_int64_t value )
{
	iot_status_t result;
#if defined( IOT_JSON_JANSSON )
	result = iot_json_encode_key( encoder, key, json_integer( value ) );
#elif defined( IOT_JSON_JSONC )
	result = iot_json_encode_key( encoder, key, json_object_new_int64( value ) );
#else /* defined( IOT_JSON_JSMN ) */
	/* can't add boolean as root element */
	if ( !key && ( encoder && encoder->structs == 0u ) )
		result = IOT_STATUS_BAD_REQUEST;
	else
	{
		iot_bool_t added_parent = IOT_FALSE;
		iot_uint64_t pos_value;
		size_t value_len;
		if ( value < 0 )
			pos_value = (iot_uint64_t)(value * -1);
		else
			pos_value = (iot_uint64_t)value;

		value_len = iot_json_encode_intlen( pos_value, value < 0 );
		result = iot_json_encode_key( encoder, key, value_len,
			&added_parent );
		if ( result == IOT_STATUS_SUCCESS )
		{
			char *dest = &encoder->cur[value_len - 1u];
			if ( value <= 0 )
			{
				if ( value < 0 )
				{
					*encoder->cur = '-';
					value *= -1;
				}
				else
					*encoder->cur = '0';
				++encoder->cur;
			}
			while ( pos_value > 0 )
			{
				*dest = (pos_value % 10 + '0');
				pos_value /= 10;
				--dest;
				++encoder->cur;
			}

			if ( added_parent )
				result = iot_json_encode_struct_end( encoder,
					IOT_JSON_TYPE_OBJECT << 1u );
		}
	}
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

#if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC )
size_t iot_json_encode_intlen( iot_uint64_t i, iot_bool_t neg )
{
	size_t len = 0u;
	if ( neg || i == 0 )
		++len;
	while ( i > 0 )
	{
		i /= 10;
		++len;
	}
	return len;
}

iot_status_t iot_json_encode_key(
	iot_json_encoder_t *encoder,
	const char *key,
	size_t value_len,
	iot_bool_t *added_parent )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( encoder )
	{
		size_t extra_space = 0u;
		size_t key_len = 0u;
		result = IOT_STATUS_SUCCESS;

		if ( key && !( encoder->structs & IOT_JSON_TYPE_OBJECT ) )
		{
			if ( encoder->structs )
			{
				result = iot_json_encode_struct_start( encoder,
					NULL, IOT_JSON_TYPE_OBJECT << 1 );
				if ( added_parent )
					*added_parent = IOT_TRUE;
			} else /* add root item if not there */
				result = iot_json_encode_struct_start( encoder,
					NULL, IOT_JSON_TYPE_OBJECT );
		}
		else if ( !key && encoder->structs & IOT_JSON_TYPE_OBJECT )
			key = ""; /* we are inside object we must have a key */

		if ( key )
		{
			key_len = iot_json_encode_strlen( key );
			extra_space += 3u; /* for '"' around key, ':' */
		}

		if ( result == IOT_STATUS_SUCCESS )
		{
			size_t space = encoder->len - (size_t)(encoder->cur - encoder->buf);
			iot_bool_t add_comma = 0;
			unsigned int indent = (encoder->flags >> IOT_JSON_INDENT_OFFSET);
			const unsigned int depth = iot_json_encode_depth( encoder );

			/* space required for closing current level */
			if ( indent )
			{
				unsigned int i;
				for ( i = 0u; i < depth; ++i )
					extra_space += indent * i + 1u;
			}

			/* space required for initial ',' + optional space */
			if ( encoder->cur > encoder->buf &&
				*(encoder->cur - 1) != JSON_CHARS_START[0] &&
				*(encoder->cur - 1) != JSON_CHARS_START[1] &&
				*(encoder->cur - 1) != JSON_CHARS_START[2] )
			{
				add_comma = 1;
				++extra_space;
				if ( !indent && ( encoder->flags & IOT_JSON_FLAG_EXPAND ) )
					++extra_space;
			}

			/* space required to add space around `:` */
			if ( encoder->flags & IOT_JSON_FLAG_EXPAND )
				++extra_space;

			/* how much space is required to indent/close the object */
			if ( indent )
				extra_space += ( indent * 2u * depth ) + 1u; /* +1 for '\n' */

#ifndef IOT_STACK_ONLY
			if ( encoder->flags & IOT_JSON_FLAG_DYNAMIC )
			{
				const size_t new_space =
					encoder->len + key_len + value_len + extra_space;
				void *const new_buf = iot_json_realloc(
					encoder->buf, new_space + 1u );
				if ( new_buf )
				{
					encoder->cur = (char*)new_buf +
						(encoder->cur - encoder->buf);
					encoder->buf = new_buf;
					encoder->len = new_space;
					space = new_space;
				}
			}
			else
#endif /* ifndef IOT_STACK_ONLY */
			{
				if ( !encoder->cur )
					encoder->cur = encoder->buf;
			}

			if ( key_len + value_len + extra_space <= space )
			{
				if ( add_comma )
				{
					*encoder->cur = ',';
					++encoder->cur;
					if ( !indent && ( encoder->flags & IOT_JSON_FLAG_EXPAND ) )
					{
						*encoder->cur = ' ';
						++encoder->cur;
					}
				}
				if ( indent )
				{
					if ( depth > 0u )
					{
						*encoder->cur = '\n';
						++encoder->cur;
					}
					indent *= depth;
					while ( indent )
					{
						*encoder->cur = ' ';
						++encoder->cur;
						--indent;
					}
				}
				if ( key )
				{
					*encoder->cur = '"';
					++encoder->cur;
					iot_json_encode_strncpy(
						encoder->cur, key, key_len );
					encoder->cur += key_len;
					os_strncpy( encoder->cur, "\":", 2u );
					encoder->cur += 2u;
					if ( encoder->flags & IOT_JSON_FLAG_EXPAND )
					{
						*encoder->cur = ' ';
						++encoder->cur;
					}
				}
				result = IOT_STATUS_SUCCESS;
			}
			else
				result = IOT_STATUS_NO_MEMORY;
		}
	}
	return result;
}
#endif /* if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC ) */

iot_status_t iot_json_encode_object_cancel(
	iot_json_encoder_t *encoder )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( encoder )
	{
		result = IOT_STATUS_BAD_REQUEST;
#if defined( IOT_JSON_JANSSON )
		if ( encoder->depth >= 1u )
		{
			json_type parent_type;
			json_t *const j_parent =
				encoder->j_cur[encoder->depth - 1u];
			parent_type = json_typeof( j_parent );
			if ( j_parent && parent_type == JSON_OBJECT )
			{
				/* not the root item */
				if ( encoder->depth > 1u )
				{
					void *j_iter;
					json_t *const p =
						encoder->j_cur[encoder->depth - 2u];

					/* remove the reference to the item */
					j_iter = json_object_iter( p );
					result = IOT_STATUS_NOT_FOUND;
					while ( result != IOT_STATUS_SUCCESS && j_iter )
					{
						if ( json_object_iter_value( j_iter ) == j_parent )
						{
							const char *const key =
								json_object_iter_key( j_iter );
							json_object_del( p, key );
							result = IOT_STATUS_SUCCESS;
						}
						else
							j_iter = json_object_iter_next( p, j_iter );
					}
				}
				else
				{
					/* remove the reference */
					json_decref( j_parent );
					result = IOT_STATUS_SUCCESS;
				}

				encoder->j_cur[encoder->depth - 1u ] = NULL;
				--encoder->depth;
			}
		}
#elif defined( IOT_JSON_JSONC )
		if ( encoder->depth >= 1u )
		{
			struct json_object *const j_parent =
				encoder->j_cur[encoder->depth - 1u];
			if ( json_object_is_type( j_parent, json_type_object ) )
			{
				if ( encoder->depth > 1u )
				{
					struct json_object_iterator j_iter;
					struct json_object_iterator j_end;
					struct json_object *const p =
						encoder->j_cur[encoder->depth - 2u];
					j_iter = json_object_iter_begin( p );
					j_end = json_object_iter_end( p );
					result = IOT_STATUS_NOT_FOUND;
					while ( result != IOT_STATUS_SUCCESS &&
						json_object_iter_equal( &j_iter, &j_end ) == FALSE )
					{
						if ( json_object_iter_peek_value( &j_iter ) == j_parent )
						{
							const char *const key =
								json_object_iter_peek_name( &j_iter );
							json_object_object_del( p, key );
							result = IOT_STATUS_SUCCESS;
						}
						else
							json_object_iter_next( &j_iter );
					}
				}
				else
				{
					json_object_put( j_parent );
					result = IOT_STATUS_SUCCESS;
				}
				encoder->j_cur[encoder->depth - 1u] = NULL;
				--encoder->depth;
			}
		}
#else /* defined( IOT_JSON_JSMN ) */
		if ( encoder->structs >> 1 ) /* inside an object */
		{
			char *new_pos = encoder->cur - 1;
			char *save_pos;
			unsigned int depth_count = 0u;

			/* remove the object */
			while ( new_pos && new_pos > encoder->buf &&
				( depth_count > 0u ||
				( *new_pos != JSON_CHARS_START[2] ) ) )
			{
				if ( *new_pos == JSON_CHARS_START[2] )
					--depth_count;
				if ( *new_pos == JSON_CHARS_END[2] )
					++depth_count;
				--new_pos;
			}

			save_pos = new_pos;
			/* remove any spaces before '{' character */
			while ( new_pos > encoder->buf && *(--new_pos) == ' ' );

			/* remove the key to the object (may not exist) */
			if ( new_pos > encoder->buf && *new_pos == ':' )
			{
				--new_pos;
				while( new_pos > encoder->buf &&
					*new_pos != ',' &&
					*new_pos != '{' &&
					*new_pos != '[' )
				{
					--new_pos;
				}
			}
			else
				new_pos = save_pos + 1;
			encoder->cur = new_pos;
			encoder->structs >>= 3;

			/* removed the root element */
			if ( encoder->structs == 0 )
				*encoder->buf = '\0';
			result = IOT_STATUS_SUCCESS;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return result;
}

iot_status_t iot_json_encode_object_clear(
	iot_json_encoder_t *encoder )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( encoder )
	{
		result = IOT_STATUS_BAD_REQUEST;
#if defined( IOT_JSON_JANSSON )
		if ( encoder->depth > 0u )
		{
			json_t *j_parent;
			json_type parent_type;
			j_parent = encoder->j_cur[encoder->depth - 1u];
			parent_type = json_typeof( j_parent );
			if ( j_parent && parent_type == JSON_OBJECT )
			{
				json_object_clear( j_parent );
				result = IOT_STATUS_SUCCESS;
			}
		}
#elif defined( IOT_JSON_JSONC )
		if ( encoder->depth > 0u )
		{
			struct json_object *j_parent;
			j_parent = encoder->j_cur[encoder->depth - 1u];
			if ( j_parent &&
				json_object_is_type( j_parent,
					json_type_object ) != IOT_FALSE )
			{
				struct json_object_iterator it =
					json_object_iter_begin( j_parent );
				struct json_object_iterator it_end =
					json_object_iter_end( j_parent );
				while ( json_object_iter_equal( &it, &it_end ) == IOT_FALSE )
				{
					const char *key = json_object_iter_peek_name( &it );
					json_object_object_del( j_parent, key );
					it = json_object_iter_begin( j_parent );
					it_end = json_object_iter_end( j_parent );
				}

				result = IOT_STATUS_SUCCESS;
			}
		}
#else /* defined( IOT_JSON_JSMN ) */
		if ( encoder->structs >> 1 ) /* inside an object */
		{
			char *new_pos = encoder->cur - 1;
			unsigned int depth_count = 0u;
			while ( new_pos && new_pos > encoder->buf &&
				( depth_count > 0u ||
				  *new_pos != JSON_CHARS_START[2] ) )
			{
				if ( *new_pos == JSON_CHARS_START[2] )
					--depth_count;
				if ( *new_pos == JSON_CHARS_END[2] )
					++depth_count;
				--new_pos;
			}
			++new_pos;
			encoder->cur = new_pos;
			result = IOT_STATUS_SUCCESS;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return result;
}

iot_status_t iot_json_encode_object_end(
	iot_json_encoder_t *encoder )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
#if defined( IOT_JSON_JANSSON ) || defined( IOT_JSON_JSONC )
	if ( encoder )
	{
		result = IOT_STATUS_BAD_REQUEST;
		if ( encoder->depth > 0u )
		{
			json_t *const j_obj = encoder->j_cur[encoder->depth - 1u];
#if defined( IOT_JSON_JANSSON )
			if ( json_typeof( j_obj ) == JSON_OBJECT )
#elif defined( IOT_JSON_JSONC )
			if ( json_object_is_type( j_obj, json_type_object ) != FALSE )
#endif
			{
				--encoder->depth;
				result = IOT_STATUS_SUCCESS;
			}
		}
	}
#elif defined( IOT_JSON_JSONC )
	(void)encoder;
#else /* defined( IOT_JSON_JSMN ) */
	result = iot_json_encode_struct_end( encoder, IOT_JSON_TYPE_OBJECT );
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

iot_status_t iot_json_encode_object_start(
	iot_json_encoder_t *encoder,
	const char *key )
{
	iot_status_t result;
#if defined( IOT_JSON_JANSSON )
	result = iot_json_encode_key( encoder, key, json_object() );
#elif defined( IOT_JSON_JSONC )
	result = iot_json_encode_key( encoder, key, json_object_new_object() );
#else /* defined( IOT_JSON_JSMN ) */
	result = iot_json_encode_struct_start( encoder, key, IOT_JSON_TYPE_OBJECT );
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

/**
 * @def JSON_ENCODE_MAX_DECIMALS
 * @brief number of decimals to display for real data
 */
#define JSON_ENCODE_MAX_DECIMALS 6

iot_status_t iot_json_encode_real(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_float64_t value )
{
	iot_status_t result;
#if defined( IOT_JSON_JANSSON )
	result = iot_json_encode_key( encoder, key, json_real( (double)value ) );
#elif defined( IOT_JSON_JSONC )
	result = iot_json_encode_key( encoder, key, json_object_new_double( (double)value ) );
#else /* defined( IOT_JSON_JSMN ) */
	/* can't add boolean as root element */
	if ( !key && ( encoder && encoder->structs == 0u ) )
		result = IOT_STATUS_BAD_REQUEST;
	else
	{
		iot_bool_t added_parent = 0;

		iot_uint64_t i = (iot_uint64_t)value;
		double frac = value - i;
		size_t int_len;
		size_t value_len;

		/* handle negative numbers */
		if ( value < 0.0 )
		{
			i = (iot_uint64_t)(value * -1.0);
			frac = (value * -1.0) - i;
		}

		int_len = iot_json_encode_intlen( i, value < 0.0 );
		value_len = int_len + JSON_ENCODE_MAX_DECIMALS + 1u;
		result = iot_json_encode_key( encoder, key, value_len, &added_parent );
		if ( result == IOT_STATUS_SUCCESS )
		{
			char *dest = &encoder->cur[int_len - 1u];
			if ( value < 0.0 )
			{
				*encoder->cur = '-';
				++encoder->cur;
			}

			if ( i == 0 )
			{
				*encoder->cur = '0';
				++encoder->cur;
			}

			while ( i > 0u )
			{
				*dest = (i % 10 + '0');
				i /= 10;
				--dest;
				++encoder->cur;
			}

			*encoder->cur++ = '.';
			i = 0u;
			do {
				int j;
				frac *= 10.0;
				j = (int)(frac);
				*encoder->cur++ = '0' + (char)j;
				frac -= j;
				++i;
			} while ( frac > 0.0 && i < JSON_ENCODE_MAX_DECIMALS );
			if ( added_parent )
				result = iot_json_encode_struct_end( encoder,
					IOT_JSON_TYPE_OBJECT << 1u );
		}
	}
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

iot_status_t iot_json_encode_string(
	iot_json_encoder_t *encoder,
	const char *key,
	const char *value )
{
	iot_status_t result;
#if defined( IOT_JSON_JANSSON )
	result = iot_json_encode_key( encoder, key, json_string( value ) );
#elif defined( IOT_JSON_JSONC )
	struct json_object *v = NULL;
	if ( value )
		v = json_object_new_string( value );
	result = iot_json_encode_key( encoder, key, v );
#else /* defined( IOT_JSON_JSMN ) */
	/* can't add boolean as root element */
	if ( !key && ( encoder && encoder->structs == 0u ) )
		result = IOT_STATUS_BAD_REQUEST;
	else
	{
		iot_bool_t added_parent = 0;
		size_t value_len;

		if ( !value )
			value = "";
		value_len = iot_json_encode_strlen( value );
		result = iot_json_encode_key( encoder, key, value_len + 2u,
			&added_parent );
		if ( encoder && result == IOT_STATUS_SUCCESS )
		{
			*encoder->cur++ = '"';
			iot_json_encode_strncpy(
				encoder->cur, value, value_len );
			encoder->cur += value_len;
			*encoder->cur++ = '"';
			if ( added_parent )
				result = iot_json_encode_struct_end( encoder,
					IOT_JSON_TYPE_OBJECT << 1u );
		}
	}
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

#if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC )
size_t iot_json_encode_strlen(
	const char* str )
{
	size_t result = 0u;
	while ( *str != '\0' )
	{
		/* add additional character for null-terminator */
		if ( *str == '\"' || *str == '\\' || *str == '\b' ||
		     *str == '\f' || *str == '\n' || *str == '\r' ||
		     *str == '\t' )
			++result;
		++result;
		++str;
	}
	return result;
}

char *iot_json_encode_strncpy(
	char *dest,
	const char *src,
	size_t num )
{
	iot_bool_t skip_char = IOT_FALSE;
	char *out = dest;
	size_t i = 0u;
	while ( i < num && *src != '\0' )
	{
		if ( *src == '\"' || *src == '\\' || *src == '\b' ||
		     *src == '\f' || *src == '\n' || *src == '\r' ||
		     *src == '\t' )
		{
			if ( i + 1u < num )
			{
				*out = '\\';
				++out;
				++i;
			}
			else
				skip_char = IOT_TRUE;
		}

		if ( skip_char == IOT_FALSE )
		{
			switch ( *src )
			{
			case '\b':
				*out = 'b';
				break;
			case '\f':
				*out = 'f';
				break;
			case '\n':
				*out = 'n';
				break;
			case '\r':
				*out = 'r';
				break;
			case '\t':
				*out = 't';
				break;
			default:
				*out = *src;
			}
			++src;
			++out;
			++i;
		}
	}

	/* pad remaining with zero's */
	for ( ; i < num; ++i )
		*out = '\0';
	return dest;
}

iot_status_t iot_json_encode_struct_end(
	iot_json_encoder_t *encoder,
	iot_json_type_t s )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( encoder )
	{
		result = IOT_STATUS_BAD_REQUEST;
		if ( encoder->structs & s )
		{
			unsigned int depth = iot_json_encode_depth( encoder ) - 1u;
			const unsigned int indent = (encoder->flags >> IOT_JSON_INDENT_OFFSET);
			size_t space = encoder->len - (size_t)(encoder->cur - encoder->buf);
			iot_json_encode_struct_t i;

			/* ] } */
			result = IOT_STATUS_SUCCESS;
			for ( i = 0u; i < JSON_STRUCT_BITS; ++i )
			{
				if ( ((iot_json_encode_struct_t)(1u) << (i)) &
					encoder->structs )
				{
					/* +2 for '\n' + ']' or '}' character */
					if ( (indent * depth) + 2u > space )
					{
#ifndef IOT_STACK_ONLY
						if ( encoder->flags & IOT_JSON_FLAG_DYNAMIC )
						{
							const size_t new_space =
								(depth * indent) + 2u;
							void *const new_buf = iot_json_realloc(
								encoder->buf,
								encoder->len + new_space );
							if ( new_buf )
							{
								encoder->cur = (char*)new_buf +
									(encoder->cur - encoder->buf);
								encoder->buf = new_buf;
								encoder->len += new_space;
								space += new_space;
							}
							else
								result = IOT_STATUS_NO_MEMORY;
						}
						else
#endif /* ifndef IOT_STACK_ONLY */
							result = IOT_STATUS_NO_MEMORY;
					}

					if ( result == IOT_STATUS_SUCCESS )
					{
						const char end_ch = JSON_CHARS_END[i];
						if ( indent )
						{
							unsigned int j = indent * depth;
							if ( *(encoder->cur - 1) != JSON_CHARS_START[0] &&
							     *(encoder->cur - 1) != JSON_CHARS_START[1] )
							{
								*encoder->cur++ = '\n';

								while ( j )
								{
									*encoder->cur++ = ' ';
									--j;
								}
								space -= (indent + 1u);
							}
							--depth;
						}
						*encoder->cur++ = end_ch;
						--space;
					}
				}
			}
			encoder->structs >>= JSON_STRUCT_BITS;
		}
	}

	return result;
}

iot_status_t iot_json_encode_struct_start(
	iot_json_encoder_t *encoder,
	const char *key,
	iot_json_type_t s )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( encoder )
	{
		result = IOT_STATUS_FULL;
		if ( iot_json_encode_depth( encoder ) < JSON_MAX_DEPTH )
		{
			iot_bool_t added_parent = IOT_FALSE;
			unsigned int indent = (encoder->flags >> IOT_JSON_INDENT_OFFSET);
			/* +2u for '[' & ']' characters */
			result = iot_json_encode_key( encoder, key, indent + 2u,
				&added_parent );
			if ( result == IOT_STATUS_SUCCESS )
			{
				unsigned int i;
				for ( i = JSON_STRUCT_BITS; i > 0u; --i )
				{
					/* {, [ */
					unsigned int i_1 = i - 1u;
					if ( s & ( (iot_json_encode_struct_t)(1u) << ( i_1 ) ) )
					{
						indent = (encoder->flags >> IOT_JSON_INDENT_OFFSET);
						*encoder->cur++ = JSON_CHARS_START[i_1];
					}
				}
				if ( !added_parent )
					encoder->structs <<= JSON_STRUCT_BITS;
				encoder->structs |= (iot_json_encode_struct_t)s;
			}
		}
	}
	return result;
}
#endif /* if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC ) */

void iot_json_encode_terminate(
	iot_json_encoder_t *encoder )
{
#if defined( IOT_JSON_JANSSON )
	if ( encoder )
	{
		if ( encoder->output )
		{
			json_free_t free_fn = os_free;
#if JANSSON_VERSION_HEX >= 0x020800
			json_get_alloc_funcs( NULL, &free_fn );
#endif /* if JANSSON_VERSION_HEX >= 0x020800 */
			if ( free_fn )
				free_fn( encoder->output );
		}
		if ( encoder->j_cur )
			json_decref( encoder->j_cur[0] );
		iot_json_free( encoder->j_cur );
	}
#elif defined( IOT_JSON_JSONC )
	if ( encoder )
	{
		if ( encoder->output )
			iot_json_free( encoder->output );
		if ( encoder->j_cur && encoder->j_cur[0] )
			json_object_put( encoder->j_cur[0] );
		iot_json_free( encoder->j_cur );
	}
#endif /* if defined( IOT_JSON_JANSSON ) */

#ifdef IOT_STACK_ONLY
	(void)encoder;
#else /* ifdef IOT_STACK_ONLY */
	if ( encoder && ( encoder->flags & IOT_JSON_FLAG_DYNAMIC ) )
	{
#if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC )
		if ( encoder->buf )
			iot_json_free( encoder->buf );
#endif /* if !defined( IOT_JSON_JANSSON ) && !defined( IOT_JSON_JSONC ) */
		iot_json_free( encoder );
	}
#endif /* else IOT_STACK_ONLY */
}

