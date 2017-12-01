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

#if defined( IOT_JSON_JANSSON )
#	include <os.h> /* for os_memzero, os_snprintf  */
#elif defined( IOT_JSON_JSONC )
#	include <os.h> /* for os_snprintf */
#else /* defined( IOT_JSON_JSMN ) */
/**
 * @brief helper function for decoding real numbers with JSMN
 *
 * This function is a real number decoder for JSMN supporting
 * expotential notation as well as decoding of both integer values and
 * real numbers.  The flag @c is_integer can be used to determine if
 * the number indicated only contained an integer number
 *
 * @param[in]      decoder             JSON decoder object
 * @param[in]      item                JSON integer or JSON real number
 * @param[in,out]  value               returned number value
 * @param[in,out]  is_integer          whether the number only contained an
 *                                     integer portion
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      @c item does not point to a JSON integer
 *                                     or real number
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_json_decode_integer
 * @see iot_json_decode_number
 * @see iot_json_decode_real
 * @see iot_json_decode_type
 */
static IOT_SECTION iot_status_t iot_jsmn_decode_number(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_float64_t *value,
	iot_bool_t *is_integer );

iot_status_t iot_jsmn_decode_number(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_float64_t *value,
	iot_bool_t *is_integer )
{
	const jsmntok_t *const cur = (const jsmntok_t *)item;
	iot_bool_t is_int = IOT_TRUE;
	iot_status_t result = IOT_STATUS_BAD_REQUEST;
	iot_float64_t v = 0.0;

	if ( cur->type == JSMN_PRIMITIVE )
	{
		iot_bool_t is_neg = IOT_FALSE;
		iot_bool_t is_neg_e = IOT_FALSE;
		unsigned long denom = 0u; /* false */
		unsigned int d_count = 0u; /* decimal count */
		int e = 0u; /* exponent portion */
		iot_bool_t has_e = IOT_FALSE;
		int i;

		result = IOT_STATUS_SUCCESS;
		for ( i = cur->start;
		      i < cur->end && result == IOT_STATUS_SUCCESS; ++i )
		{
			const char c = decoder->buf[i];
			if ( c >= '0' && c <= '9' )
			{
				if ( has_e )
					e = (e * 10) + (c - '0');
				else if ( denom >= 10u )
				{
					v = v + (iot_float64_t)((iot_float64_t)(c - '0') / (iot_float64_t)denom);
					denom *= 10u;
					++d_count;
				}
				else
					v = (v * 10) + (c - '0');
			}
			else if ( c == '-' && i == cur->start )
				is_neg = IOT_TRUE;
			else if ( c == '.' && i != cur->start && denom == 0u )
				denom = 10u;
			else if ( ( c == 'e' || c == 'E' ) && i != cur->start && has_e == IOT_FALSE )
				has_e = IOT_TRUE;
			else if ( c == '-' && has_e == IOT_TRUE && is_neg_e == IOT_FALSE )
				is_neg_e = IOT_TRUE;
			else if ( c != '+' )
				result = IOT_STATUS_BAD_REQUEST;
		}

		/* handle exponent in number */
		/* exponent was a negative */
		if ( is_neg_e )
			e *= -1;

		if ( e < 0 )
			for ( ; e < 0; ++e )
				v /= 10.0;
		else
		{
			int e2 = e;
			for ( ; e2 > 0; --e2 )
				v *= 10.0;

			/* exponent is larger than decimal places, so let's remove any
			 * rounding error, and ensure it's an exact integer */
			if ( d_count <= (unsigned int)e )
				v = (iot_float64_t)((iot_int64_t)(v + 0.5));
		}

		/* was a real number */
		if ( denom > 0u )
			is_int = IOT_FALSE;

		if ( result != IOT_STATUS_SUCCESS )
			v = 0.0;
		else if ( is_neg )
			v *= -1.0;
	}

	if ( value )
		*value = v;
	if ( is_integer )
		*is_integer = is_int;
	return result;
}
#endif /* defined( IOT_JSON_JSMN ) */

iot_status_t iot_json_decode_array_at(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	size_t index,
	const iot_json_item_t **out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	const iot_json_item_t *obj = NULL;
	if ( decoder && item && out )
	{
#if defined( IOT_JSON_JANSSON )
		const json_t *j = (const json_t*)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_is_array( j ) )
		{
			result = IOT_STATUS_NOT_FOUND;
			obj = json_array_get( j, index );
			if ( obj )
				result = IOT_STATUS_SUCCESS;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_object_is_type( j, json_type_array ) != FALSE )
		{
			result = IOT_STATUS_NOT_FOUND;
			obj = json_object_array_get_idx( j, index );
			if ( obj )
				result = IOT_STATUS_SUCCESS;
		}
#else /* defined ( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = (const jsmntok_t*)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( cur && cur->type == JSMN_ARRAY )
		{
			int array_idx = -1;
			unsigned int idx = 0u;
			unsigned int p_idx = 0u;

			p_idx = (unsigned int)(cur - decoder->tokens);
			idx = p_idx;
			++cur;
			++idx;
			while ( obj == NULL && idx < decoder->objs )
			{
#ifdef JSMN_PARENT_LINKS
				if ( cur->parent == (int)p_idx )
					++array_idx;
#else
				/** @todo make this work without parent links */
#endif /* ifdef JSMN_PARENT_LINKS */

				if ( (size_t)array_idx == index )
					obj = cur;
				++cur;
				++idx;
			}

			result = IOT_STATUS_NOT_FOUND;
			if ( obj )
				result = IOT_STATUS_SUCCESS;
		}
#endif /* defined ( IOT_JSON_JSMN ) */
	}

	if ( out )
		*out = obj;
	return result;
}

const iot_json_array_iterator_t *iot_json_decode_array_iterator(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	const iot_json_array_iterator_t *iter = NULL;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		if ( json_array_size( (const json_t*)item ) > 0u )
			iter = (struct iot_json_array_iterator *)1u;
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		if ( json_object_array_length( j ) > 0 )
			iter = (struct iot_json_array_iterator *)1u;
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = (const jsmntok_t *)item;
		if ( cur && cur->type == JSMN_ARRAY && cur->size > 0 )
			iter = cur + 1;
#endif /* defined ( IOT_JSON_JSMN ) */
	}
	return iter;
}

iot_status_t iot_json_decode_array_iterator_value(
	const iot_json_decoder_t *json,
	const iot_json_item_t *item,
	const iot_json_array_iterator_t *iter,
	const iot_json_item_t **out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	const iot_json_item_t *v = NULL;
	if ( json && item && iter && out )
	{
#if defined( IOT_JSON_JANSSON )
		const size_t i = (const size_t)iter;
		v = json_array_get( (const json_t*)item, i - 1u );
#elif defined( IOT_JSON_JSONC )
		const size_t i = (const size_t)iter;
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		v = json_object_array_get_idx( j, i - 1u );
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = iter;
		v = cur;
		if ( cur->type == JSMN_STRING && cur->size == 1 )
			v = cur + 1u;
#endif /* defined( IOT_JSON_JSMN ) */
		result = IOT_STATUS_SUCCESS;
	}
	if ( out )
		*out = v;
	return result;
}

const iot_json_array_iterator_t *iot_json_decode_array_iterator_next(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_array_iterator_t *iter )
{
	const iot_json_array_iterator_t *result = NULL;
	if ( decoder && item && iter )
	{
#if defined( IOT_JSON_JANSSON )
		size_t i = (size_t)(iter);
		if ( i < json_array_size( item ) )
		{
			++i;
			result = (iot_json_array_iterator_t*)i;
		}
#elif defined( IOT_JSON_JSONC )
		size_t i = (size_t)(iter);
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		if ( i < (size_t)json_object_array_length( j ) )
		{
			++i;
			result = (iot_json_array_iterator_t*)i;
		}
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = (const jsmntok_t *)iter;
		unsigned int idx;
		const int obj_end_pos = ((const jsmntok_t*)item)->end;
#ifdef JSMN_PARENT_LINKS
		const int parent = (int)((const jsmntok_t *)item - decoder->tokens);
#endif /* ifdef JSMN_PARENT_LINKS */

		/* get index of current item */
		idx = (unsigned int)(cur - decoder->tokens);

		++cur;
		++idx;
#ifdef JSMN_PARENT_LINKS
		while ( cur->start < obj_end_pos &&
			cur->parent != parent &&
			idx < decoder->objs )
		{
			++cur;
			++idx;
		}
#else  /* ifdef JSMN_PARENT_LINKS */
		/** @todo make this work without JSMN_PARENT_LINKS */
#endif /* else JSMN_PARENT_LINKS */

		result = cur;
		/* hit end of list */
		if ( idx >= decoder->objs ||
			cur->start >= obj_end_pos )
			result = NULL;
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return result;
}

size_t iot_json_decode_array_size(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	size_t result = 0u;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		result = json_array_size( (const json_t*)item );
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		if ( json_object_is_type( j, json_type_array ) != FALSE )
			result = json_object_array_length( j );
#else /* defined( IOT_JSON_JSMN )*/
		const jsmntok_t *cur = (const jsmntok_t*)item;
		if ( cur->type == JSMN_ARRAY )
			result = (size_t)cur->size;
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return result;
}

iot_status_t iot_json_decode_bool(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_bool_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_bool_t v = IOT_FALSE;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		const json_t *j = (const json_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_is_boolean( j ) )
		{
			if ( json_is_true( j ) )
				v = IOT_TRUE;
			result = IOT_STATUS_SUCCESS;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_object_is_type( j, json_type_boolean ) != FALSE )
		{
			v = IOT_TRUE;
			if ( json_object_get_boolean( j ) == FALSE )
				v = IOT_FALSE;
			result = IOT_STATUS_SUCCESS;
		}
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *const cur = (const jsmntok_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( cur->type == JSMN_PRIMITIVE )
		{
			const char c = decoder->buf[cur->start];
			if ( c == 't' )
			{
				v = IOT_TRUE; /* true */
				result = IOT_STATUS_SUCCESS;
			}
			if ( c == 'f' )
				result = IOT_STATUS_SUCCESS;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	if ( value )
		*value = v;
	return result;
}

iot_json_decoder_t *iot_json_decode_initialize(
	void *buf,
	size_t len,
	unsigned int flags )
{
	struct iot_json_decoder *decoder = NULL;
#if defined( IOT_JSON_JANSSON )
	size_t additional_size = 0u;
#elif defined( IOT_JSON_JSONC )
	size_t additional_size = 0u;
#else /* defined( IOT_JSON_JSMN ) */
	size_t additional_size = sizeof( jsmntok_t );
#endif /* defined( IOT_JSON_JSMN ) */

#ifndef IOT_STACK_ONLY
	iot_bool_t on_heap = IOT_FALSE;
	if ( !buf )
		flags |= IOT_JSON_FLAG_DYNAMIC;
	if ( flags & IOT_JSON_FLAG_DYNAMIC )
	{
		additional_size = 0u;
		len = sizeof( struct iot_json_decoder );
		buf = iot_json_realloc( NULL, len );
		on_heap = IOT_TRUE;
	}
#endif /* ifndef IOT_STACK_ONLY */

	if ( buf && len >= ( sizeof( struct iot_json_decoder ) + additional_size ) )
	{
#if defined( IOT_JSON_JANSSON )
		decoder = (struct iot_json_decoder *)buf;
		decoder->flags = flags;
		decoder->j_root = NULL;
#elif defined( IOT_JSON_JSONC )
		decoder = (struct iot_json_decoder *)buf;
		decoder->flags = flags;
		decoder->j_root = NULL;
#else /* defined( IOT_JSON_JSMN ) */
		const size_t max_objs =
			(len - sizeof(struct iot_json_decoder)) /
				sizeof(jsmntok_t);

		if ( additional_size == 0u || max_objs >= 1u )
		{
			unsigned int i;
			decoder = (struct iot_json_decoder *)(void*)buf;
			decoder->objs = 0u;
			decoder->size = (unsigned int)max_objs;
			decoder->flags = flags;
			decoder->buf = NULL;
			decoder->len = 0u;
			decoder->tokens = NULL;
			if ( max_objs )
			{
				jsmntok_t *tok;
				tok = (jsmntok_t *)(void*)( (char*)buf + sizeof(struct iot_json_decoder) );
				decoder->tokens = tok;
				for ( i = 0u; i < max_objs; ++i )
				{
					tok->type = JSMN_UNDEFINED;
					tok->start = 0;
					tok->end = 0;
					tok->size = 0;
#ifdef JSMN_PARENT_LINKS
					tok->parent = 0;
#endif
					++tok;
				}
			}
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
#ifndef IOT_STACK_ONLY
	else if ( buf && on_heap != IOT_FALSE )
		iot_json_free( buf );
#endif /* ifndef IOT_STACK_ONLY */
	return decoder;
}

iot_status_t iot_json_decode_integer(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_int64_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_int64_t v = 0;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		const json_t *j = (const json_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_is_integer( j ) )
		{
			v = (iot_int64_t)json_integer_value( j );
			result = IOT_STATUS_SUCCESS;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_object_is_type( j, json_type_int ) != FALSE )
		{
			v = (iot_int64_t)json_object_get_int64( j );
			result = IOT_STATUS_SUCCESS;
		}
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *const cur = (const jsmntok_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( cur->type == JSMN_PRIMITIVE )
		{
			iot_bool_t is_neg = IOT_FALSE;
			int i;

			result = IOT_STATUS_SUCCESS;
			for ( i = cur->start;
			      i < cur->end && result == IOT_STATUS_SUCCESS; ++i )
			{
				const char c = decoder->buf[i];
				if ( c >= '0' && c <= '9' )
					v = (v * 10) + (c - '0');
				else if ( c == '-' && i == cur->start )
					is_neg = IOT_TRUE;
				else
					result = IOT_STATUS_BAD_REQUEST;
			}
			if ( result != IOT_STATUS_SUCCESS )
				v = 0;
			else if ( is_neg )
				v *= -1;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	if ( value )
		*value = v;
	return result;
}

iot_status_t iot_json_decode_number(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_float64_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_float64_t v = 0.0;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		const json_t *j = (const json_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_is_number( j ) )
		{
			v = (iot_float64_t)json_number_value( j );
			result = IOT_STATUS_SUCCESS;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_object_is_type( j, json_type_double ) != FALSE ||
			json_object_is_type( j, json_type_int ) != FALSE )
		{
			if ( json_object_is_type( j, json_type_double ) != FALSE )
				v = (iot_float64_t)json_object_get_double( j );
			else
				v = (iot_float64_t)json_object_get_int64( j );
			result = IOT_STATUS_SUCCESS;
		}
#else /* defined( IOT_JSON_JSMN ) */
		result = iot_jsmn_decode_number( decoder, item, &v, NULL );
#endif /* defined( IOT_JSON_JSMN ) */
	}

	if ( value )
		*value = v;
	return result;
}

const iot_json_item_t *iot_json_decode_object_find(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *object,
	const char *key )
{
	const iot_json_item_t *result = NULL;
#if defined( IOT_JSON_JANSSON )
	if ( decoder && object && key )
		result = json_object_get( object, key );
#elif defined( IOT_JSON_JSONC )
	if ( decoder && object && key )
	{
		json_object *out;
		struct json_object *j_obj;
		os_memcpy( &j_obj, &object, sizeof( struct json_object * ) );
		if ( json_object_object_get_ex( j_obj, key, &out ) == TRUE )
			result = out;
	}
#else /* defined( IOT_JSON_JSMN ) */
	result = iot_json_decode_object_find_len( decoder, object, key, 0u );
#endif /* defined( IOT_JSON_JSMN ) */
	return result;
}

const iot_json_item_t *iot_json_decode_object_find_len(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *object,
	const char *key,
	size_t key_len )
{
	const iot_json_item_t *result = NULL;
	if ( decoder && object && key )
	{
#if defined( IOT_JSON_JANSSON )
		if ( key_len > 0u )
		{
			char *buf = NULL;
			buf = iot_json_realloc( NULL, key_len + 1u );
			if ( buf )
			{
				os_strncpy( buf, key, key_len );
				buf[key_len] = '\0';
				result = json_object_get( object, buf );
				iot_json_free( buf );
			}
		}
		else
			result = json_object_get( object, key );
#elif defined( IOT_JSON_JSONC )
		(void)key_len;
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = object;
		if ( cur && cur->type == JSMN_OBJECT )
		{
			unsigned int idx = 0;
			unsigned int p_idx = 0u;
			p_idx = (unsigned int)(cur - decoder->tokens);
			idx = p_idx;
			++cur;
			++idx;
			while ( result == NULL && idx < decoder->objs )
			{
				if (
#ifdef JSMN_PARENT_LINKS
					cur->parent == (int)p_idx &&
#endif /* ifdef JSMN_PARENT_LINKS */
					cur->type == JSMN_STRING &&
					cur->size == 1 )
				{
					/* compare key */
					size_t k = 0u;
					const size_t k_len = (size_t)(cur->end - cur->start);
					if ( key_len == 0u )
						key_len = k_len;
					else if ( key_len > k_len )
						key_len = k_len;
					while ( k < key_len &&
						decoder->buf[(size_t)(cur->start) + k] == key[k] )
						++k;

					/* match found */
					if ( k == key_len )
						result = cur + 1;
				}
				++cur;
				++idx;
			}
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return result;
}

const iot_json_object_iterator_t *iot_json_decode_object_iterator(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	const iot_json_object_iterator_t *iter = NULL;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		json_t *j_obj;
		os_memcpy( &j_obj, &item, sizeof( json_t *) );
		iter = json_object_iter( j_obj );
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		if ( json_object_is_type( j, json_type_object ) != FALSE )
		{
			struct json_object_iterator i_begin;
			struct json_object_iterator i_end;
			i_begin = json_object_iter_begin( j );
			i_end = json_object_iter_end( j );
			if ( !json_object_iter_equal( &i_begin, &i_end ) )
				iter = i_begin.opaque_;
		}
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = (const jsmntok_t *)item;
		if ( cur && cur->type == JSMN_OBJECT && cur->size > 0 )
			iter = cur + 1u;
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return iter;
}

iot_status_t iot_json_decode_object_iterator_key(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_object_iterator_t *iter,
	const char **key,
	size_t *key_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	const char *k = NULL;
	size_t k_len = 0u;
	if ( decoder && item && iter )
	{
#if defined( IOT_JSON_JANSSON )
		void *j_iter;
		os_memcpy( &j_iter, &iter, sizeof( void *) );
		k = json_object_iter_key( j_iter );
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( k )
		{
			k_len = os_strlen( k );
			result = IOT_STATUS_SUCCESS;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object_iterator i;
		i.opaque_ = iter;
		result = IOT_STATUS_NOT_INITIALIZED;
		k = json_object_iter_peek_name( &i );
		if ( k )
		{
			k_len = os_strlen( k );
			result = IOT_STATUS_SUCCESS;
		}
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = iter;
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( cur->type == JSMN_STRING && cur->size == 1 )
		{
			k = &decoder->buf[cur->start];
			k_len = (size_t)(cur->end - cur->start);
			result = IOT_STATUS_SUCCESS;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	if ( key )
		*key = k;
	if ( key_len )
		*key_len = k_len;
	return result;
}

const iot_json_object_iterator_t *iot_json_decode_object_iterator_next(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_object_iterator_t *iter )
{
	const iot_json_object_iterator_t *result = NULL;
	if ( decoder && item && iter )
	{
#if defined( IOT_JSON_JANSSON )
		json_t *j_item;
		void *j_iter;
		os_memcpy( &j_item, &item, sizeof( json_t* ) );
		os_memcpy( &j_iter, &iter, sizeof( void* ) );
		result = json_object_iter_next( j_item, j_iter );
#elif defined( IOT_JSON_JSONC )
		struct json_object_iterator i;
		struct json_object_iterator i_end;
		struct json_object *j;
		i.opaque_ = iter;
		json_object_iter_next( &i );
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		i_end = json_object_iter_end( j );
		if ( !json_object_iter_equal( &i, &i_end ) )
			result = i.opaque_;
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = iter;
		unsigned int idx;
		const int obj_end_pos = ((const jsmntok_t*)item)->end;
#ifdef JSMN_PARENT_LINKS
		const int parent = (int)((const jsmntok_t *)item - decoder->tokens);
#endif /* ifdef JSMN_PARENT_LINKS */

		/* get index of current item */
		idx = (unsigned int)(cur - decoder->tokens);

		++cur;
		++idx;
#ifdef JSMN_PARENT_LINKS
		while ( cur->start < obj_end_pos &&
			cur->parent != parent &&
			idx < decoder->objs )
		{
			++cur;
			++idx;
		}
#else  /* ifdef JSMN_PARENT_LINKS */
		/** @todo make this work without JSMN_PARENT_LINKS */
#endif /* else JSMN_PARENT_LINKS */

		result = cur;
		/* hit end of list */
		if ( idx >= decoder->objs ||
			cur->start >= obj_end_pos )
			result = NULL;
#endif /* endif( IOT_JSON_JSMN ) */
	}
	return result;
}

iot_status_t iot_json_decode_object_iterator_value(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const iot_json_object_iterator_t *iter,
	const iot_json_item_t **out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	const iot_json_item_t *v = NULL;
	if ( decoder && item && iter && out )
	{
#if defined( IOT_JSON_JANSSON )
		void *j_iter;
		os_memcpy( &j_iter, &iter, sizeof( void* ) );
		v = json_object_iter_value( j_iter );
#elif defined( IOT_JSON_JSONC )
		struct json_object_iterator i;
		i.opaque_ = iter;
		result = IOT_STATUS_NOT_INITIALIZED;
		v = json_object_iter_peek_value( &i );
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = iter;
		v = cur;
		if ( cur->type == JSMN_STRING && cur->size == 1 )
			v = cur + 1u;
#endif /* defined( IOT_JSON_JSMN ) */
		result = IOT_STATUS_SUCCESS;
	}
	if ( out )
		*out = v;
	return result;
}

size_t iot_json_decode_object_size(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	size_t result = 0u;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		result = json_object_size( (const json_t*)item );
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		if ( json_object_is_type( j, json_type_object ) != FALSE )
			result = json_object_object_length( j );
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *cur = (const jsmntok_t *)item;
		if ( cur->type == JSMN_OBJECT )
			result = (size_t)cur->size;
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return result;
}

iot_status_t iot_json_decode_parse(
	iot_json_decoder_t *decoder,
	const char *js,
	size_t len,
	const iot_json_item_t **root,
	char *error,
	size_t error_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( decoder && root && js && len > 0u )
	{
#if defined( IOT_JSON_JANSSON )
		json_error_t j_error;
		decoder->j_root = json_loadb( js, len, 0u, &j_error );
		result = IOT_STATUS_PARSE_ERROR;
		if ( decoder->j_root )
		{
			*root = decoder->j_root;
			result = IOT_STATUS_SUCCESS;
		}
		else
		{
			os_snprintf( error, error_len,
				"%s (line: %d, column: %d)",
				j_error.text, j_error.line, j_error.column );
		}
#elif defined( IOT_JSON_JSONC )
		struct json_tokener *tok = json_tokener_new();
		enum json_tokener_error j_error;
		do
		{
			decoder->j_root = json_tokener_parse_ex( tok, js, len );
		} while ( (j_error = json_tokener_get_error( tok )) == json_tokener_continue );

		if ( decoder->j_root && j_error == json_tokener_success )
		{
			*root = decoder->j_root;
			result = IOT_STATUS_SUCCESS;
		}
		else
		{
			os_snprintf( error, error_len,
				"%s", json_tokener_error_desc( j_error ) );
			result = IOT_STATUS_PARSE_ERROR;
		}
		if ( tok )
			json_tokener_free( tok );
#else /* defined( IOT_JSON_JSMN ) */
		jsmn_parser parser;
		const char *error_text = NULL;
		int i;
		result = IOT_STATUS_PARSE_ERROR;
#ifndef IOT_NO_STACK
		if ( decoder->flags & IOT_JSON_FLAG_DYNAMIC )
		{
			i = JSMN_ERROR_NOMEM;
			if ( !decoder->tokens )
			{
				decoder->size = 1u;
				decoder->tokens =
					(jsmntok_t*)iot_json_realloc( NULL,
					sizeof( jsmntok_t ) * decoder->size );
			}
			while ( i == JSMN_ERROR_NOMEM && decoder->tokens )
			{
				jsmn_init( &parser );
				i = jsmn_parse( &parser, js, len,
					decoder->tokens, decoder->size );
				if ( i == JSMN_ERROR_NOMEM )
				{
					jsmntok_t *tokens;
					decoder->size = (decoder->size + 1) * 2u;
					tokens = iot_json_realloc(
						decoder->tokens,
						sizeof( jsmntok_t ) * decoder->size );
					if ( tokens )
					{
						size_t j = 0u;

						decoder->tokens = tokens;

						/* initialize tokens */
						while ( j < decoder->size )
						{
							decoder->tokens[j].type = JSMN_UNDEFINED;
							decoder->tokens[j].start = 0;
							decoder->tokens[j].end = 0;
							decoder->tokens[j].size = 0;
#ifdef JSMN_PARENT_LINKS
							decoder->tokens[j].parent = 0;
#endif /* ifdef JSMN_PARENT_LINKS */
							++j;
						}
					}
					else
					{
						iot_json_free( decoder->tokens );
						decoder->size = 0u;
					}
				}
			}
		}
		else
#endif /* ifndef IOT_NO_STACK */
		{
			size_t j;
			jsmn_init( &parser );
			for ( j = 0u; j < decoder->size; ++j )
			{
				decoder->tokens[j].type = JSMN_UNDEFINED;
				decoder->tokens[j].start = 0;
				decoder->tokens[j].end = 0;
				decoder->tokens[j].size = 0;
#ifdef JSMN_PARENT_LINKS
				decoder->tokens[j].parent = 0;
#endif /* ifdef JSMN_PARENT_LINKS */
			}

			i = jsmn_parse( &parser, js, len,
				decoder->tokens, decoder->size );
		}

		if ( i == JSMN_ERROR_NOMEM )
		{
			error_text = "out of memory";
			result = IOT_STATUS_NO_MEMORY;
		}
		else if ( i == JSMN_ERROR_INVAL )
			error_text = "invalid character";
		else if ( i == JSMN_ERROR_PART )
			error_text = "incomplete json string";
		else if ( i < 0 )
			error_text = "unknown error encountered";
		else
		{
			decoder->objs = (unsigned int)i;
			decoder->buf = js;
			decoder->len = len;
			*root = decoder->tokens;
			result = IOT_STATUS_SUCCESS;
		}

		/* copy error text */
		if ( error && error_len > 0u && error_text )
		{
			size_t j = 0u;
			--error_len;
			while ( j < error_len && *error_text != '\0' )
			{
				*error = *error_text;
				++error;
				++error_text;
				++j;
			}
			if ( j <= error_len )
				*error = '\0';
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	if ( result == IOT_STATUS_SUCCESS && error && error_len > 0u )
		*error = '\0';
	return result;
}

iot_status_t iot_json_decode_real(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	iot_float64_t *value )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_float64_t v = 0.0;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		const json_t *j = (const json_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_is_real( j ) )
		{
			v = (iot_float64_t)json_real_value( j );
			result = IOT_STATUS_SUCCESS;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_object_is_type( j, json_type_double ) != FALSE )
		{
			v = (iot_float64_t)json_object_get_double( j );
			result = IOT_STATUS_SUCCESS;
		}
#else /* defined( IOT_JSON_JSMN ) */
		iot_bool_t is_integer = IOT_TRUE;
		result = iot_jsmn_decode_number(
			decoder, item, &v, &is_integer );
		if ( is_integer == IOT_TRUE )
		{
			result = IOT_STATUS_BAD_REQUEST;
			v = 0.0;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	if ( value )
		*value = v;
	return result;
}

iot_status_t iot_json_decode_string(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item,
	const char **value,
	size_t *value_len )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	const char *v = NULL;
	size_t v_len = 0u;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		const json_t *j = (const json_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_is_string( j ) )
		{
			v = json_string_value( j );
#if JANSSON_VERSION_HEX >= 0x020700
			v_len = json_string_length( j );
#else
			if ( v )
				v_len = os_strlen( v );
#endif
			result = IOT_STATUS_SUCCESS;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		result = IOT_STATUS_BAD_REQUEST;
		if ( json_object_is_type( j, json_type_string ) != FALSE )
		{
			v = json_object_get_string( j );
			v_len = json_object_get_string_len( j );
			result = IOT_STATUS_SUCCESS;
		}
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *const cur = (const jsmntok_t *)item;
		result = IOT_STATUS_BAD_REQUEST;
		if ( cur->type == JSMN_STRING )
		{
			v = &decoder->buf[cur->start];
			v_len = (size_t)(cur->end - cur->start);
			result = IOT_STATUS_SUCCESS;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	if ( value )
		*value = v;
	if ( value_len )
		*value_len = v_len;
	return result;
}

void iot_json_decode_terminate(
	iot_json_decoder_t *decoder )
{
	if ( decoder )
	{
#if defined( IOT_JSON_JANSSON )
		if ( decoder->j_root )
			json_decref( decoder->j_root );
#elif defined( IOT_JSON_JSONC )
#else /* defined( IOT_JSON_JSMN ) */
		if ( decoder->flags & IOT_JSON_FLAG_DYNAMIC )
			iot_json_free( decoder->tokens );
#endif /* defined( IOT_JSON_JSMN ) */

#ifndef IOT_STACK_ONLY
		/* free object from heap */
		if ( decoder->flags & IOT_JSON_FLAG_DYNAMIC )
			iot_json_free( decoder );
	}
#endif /* ifndef IOT_STACK_ONLY */
}

iot_json_type_t iot_json_decode_type(
	const iot_json_decoder_t *decoder,
	const iot_json_item_t *item )
{
	iot_json_type_t result = IOT_JSON_TYPE_NULL;
	if ( decoder && item )
	{
#if defined( IOT_JSON_JANSSON )
		const json_t *j = (const json_t *)item;
		switch( json_typeof( j ) )
		{
			case JSON_ARRAY:
				result = IOT_JSON_TYPE_ARRAY;
				break;
			case JSON_FALSE:
			case JSON_TRUE:
				result = IOT_JSON_TYPE_BOOL;
				break;
			case JSON_INTEGER:
				result = IOT_JSON_TYPE_INTEGER;
				break;
			case JSON_OBJECT:
				result = IOT_JSON_TYPE_OBJECT;
				break;
			case JSON_REAL:
				result = IOT_JSON_TYPE_REAL;
				break;
			case JSON_STRING:
				result = IOT_JSON_TYPE_STRING;
				break;
			case JSON_NULL:
				result = IOT_JSON_TYPE_NULL;
		}
#elif defined( IOT_JSON_JSONC )
		struct json_object *j;
		os_memcpy( &j, &item, sizeof( struct json_object * ) );
		switch ( json_object_get_type( j ) )
		{
			case json_type_array:
				result = IOT_JSON_TYPE_ARRAY;
				break;
			case json_type_boolean:
				result = IOT_JSON_TYPE_BOOL;
				break;
			case json_type_double:
				result = IOT_JSON_TYPE_REAL;
				break;
			case json_type_int:
				result = IOT_JSON_TYPE_INTEGER;
				break;
			case json_type_object:
				result = IOT_JSON_TYPE_OBJECT;
				break;
			case json_type_string:
				result = IOT_JSON_TYPE_STRING;
				break;
			case json_type_null:
				result = IOT_JSON_TYPE_NULL;
		}
#else /* defined( IOT_JSON_JSMN ) */
		const jsmntok_t *const cur = (const jsmntok_t *)item;
		switch ( cur->type )
		{
		case JSMN_OBJECT:
			result = IOT_JSON_TYPE_OBJECT;
			break;
		case JSMN_ARRAY:
			result = IOT_JSON_TYPE_ARRAY;
			break;
		case JSMN_STRING:
			result = IOT_JSON_TYPE_STRING;
			break;
		case JSMN_PRIMITIVE:
			{
				int i;
				result = IOT_JSON_TYPE_INTEGER;
				for ( i = cur->start;
				      result == IOT_JSON_TYPE_INTEGER &&
				      i < cur->end; ++i )
				{
					const char c = decoder->buf[i];
					if ( c == '.' )
						result = IOT_JSON_TYPE_REAL;
					else if ( ( c < '0' || c > '9' ) && c != '-' )
						result = IOT_JSON_TYPE_BOOL;
				}
			}
			break;
		case JSMN_UNDEFINED:
		default:
			result = IOT_JSON_TYPE_NULL;
		}
#endif /* defined( IOT_JSON_JSMN ) */
	}
	return result;
}

