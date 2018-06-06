/**
 * @file
 * @brief source file for base IoT library JSON functionality
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

#include "iot_json_base.h"
#include "api/public/iot_json.h"
#include "utilities/app_json_base.h"
#include "utilities/app_json.h"

/* all logic is encapsulated in the JSON library */
#if !defined( IOT_STACK_ONLY )
void iot_json_allocation_get( iot_json_realloc_t **mptr, iot_json_free_t **fptr )
{
	app_json_allocation_get( mptr, fptr );
}

void iot_json_allocation_set( iot_json_realloc_t* mptr, iot_json_free_t* fptr )
{
	app_json_allocation_set( mptr, fptr );
}

void *iot_json_realloc( void *ptr, size_t size )
{
	return app_json_realloc( ptr, size );
}

void iot_json_free( void* ptr )
{
	app_json_free( ptr );
}
#endif /* if !defined( IOT_STACK_ONLY ) */
