/**
 * @file
 * @brief header file for base IoT library json functionality
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
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
#ifndef JSON_BASE_H
#define JSON_BASE_H

#if defined( IOT_JSON_JANSSON )
#include <jansson.h>
#elif defined( IOT_JSON_JSONC )
#include <json-c/json.h>
#else /* defined( IOT_JSON_JSMN ) */
#include <jsmn.h>
#endif

#if defined( IOT_JSON_JANSSON )
	/** @brief base structure used for decoding with JANSSON */
	struct app_json_decoder
	{
		/** @brief output flags */
		unsigned int flags;
		/** @brief pointer to the root object */
		json_t *j_root;
	};
#elif defined ( IOT_JSON_JSONC )
	/** @brief base structure used for decoding with json-c */
	struct app_json_decoder
	{
		/** @brief output flags */
		unsigned int flags;
		/** @brief pointer to the root object */
		struct json_object *j_root;
	};
#else /* defined( IOT_JSON_JSMN ) */
	/** @brief base structure used for decoding with JSMN */
	struct app_json_decoder
	{
		/** @brief start of the buffer */
		const char *buf;
		/** @brief size of json buffer */
		size_t len;
		/** @brief output flags */
		unsigned int flags;
		/** @brief current number of objects */
		unsigned int objs;
		/** @brief maximum number of objects */
		unsigned int size;
		/** @brief pointer to first token */
		jsmntok_t *tokens;
	};
#endif

/* functions */
#ifndef IOT_STACK_ONLY
/**
 * @brief internal dynamic memory allocation function
 *
 * @param[in]      ptr                 currently allocated buffer (optional)
 * @param[in]      size                new size to allocate buffer for
 *
 * @return a pointer to the new allocated memory large enough to hold atleast
 * @c size in bytes, NULL if memory is unable to be allocated
 *
 * @see app_json_free
 */
void *app_json_realloc( void *ptr, size_t size );

/**
 * @brief internal dynamic memory deallocation function
 *
 * @param[in]      ptr                 pointer of memory to free
 *
 * @see app_json_realloc
 */
void app_json_free( void* ptr );
#endif /* ifndef IOT_STACK_ONLY */

#endif

