/**
 * @file
 * @brief source file for base IoT library JSON functionality
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

#ifndef IOT_STACK_ONLY
/** @brief internal pointer to use for freeing dynamically allocated memory */
static iot_json_free_t *JSON_FREE = NULL;
/** @brief internal pointer to use for dynamically allocating memory */
static iot_json_realloc_t *JSON_REALLOC = NULL;

#ifdef IOT_JSON_JANSSON
/**
 * @brief helper function to dynamically allocate memory for JSON
 *
 * @param[in]      t                   amount of memory in bytes to allocate
 *
 * @return a pointer to the the memory allocated or NULL on failure
 */
static void *iot_jansson_malloc( size_t t )
{
	void *p;
	if ( JSON_REALLOC )
		p = (*JSON_REALLOC)( NULL, t );
	else
		p = os_malloc( t );
	return p;
}
/**
 * @brief helper function to free dynamic memory allocated
 *
 * @param[in]      p                   pointer to memory to free
 *
 * @return a pointer to the the memory allocated or NULL on failure
 */
static void iot_jansson_free( void *p )
{
	if ( JSON_FREE )
		(*JSON_FREE)( p );
	else
		os_free( p );
}
#endif /* ifdef IOT_JSON_JANSSON */

void iot_json_allocation_get( iot_json_realloc_t **mptr, iot_json_free_t **fptr )
{
	if ( mptr )
		*mptr = JSON_REALLOC;
	if ( fptr )
		*fptr = JSON_FREE;
}

void iot_json_allocation_set( iot_json_realloc_t* mptr, iot_json_free_t* fptr )
{
#ifdef IOT_JSON_JANSSON
	json_set_alloc_funcs( iot_jansson_malloc, iot_jansson_free );
#endif /* ifdef IOT_JSON_JANSSON */
	JSON_REALLOC = mptr;
	JSON_FREE = fptr;
}

void *iot_json_realloc( void *ptr, size_t size )
{
	if ( JSON_REALLOC )
		return (*JSON_REALLOC)(ptr,size);
	else
		return os_realloc(ptr,size);
}

void iot_json_free( void* ptr )
{
	if ( JSON_FREE )
		(*JSON_FREE)(ptr);
	else
		os_free(ptr);
}
#endif /* ifndef IOT_STACK_ONLY */

