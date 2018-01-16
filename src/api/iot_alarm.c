/**
 * @file
 * @brief source file containing alarm implementation
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

#include "public/iot.h"
#include "shared/iot_types.h"

iot_alarm_t *iot_alarm_register(
	iot_t *lib,
	const char *name )
{
	struct iot_alarm *alarm = NULL;
	if( lib && name && *name != '\0' )
	{
#ifndef IOT_NO_THREAD_SUPPORT
		os_thread_mutex_lock( &lib->alarm_mutex );
#endif
		if ( lib->alarm_count < IOT_ALARM_MAX )
		{
			const unsigned int count = lib->alarm_count;
#ifndef IOT_STACK_ONLY
			iot_bool_t is_in_heap = IOT_FALSE;
#endif

			/* look for free alarm in stack */
			alarm = lib->alarm_ptr[count];

#ifndef IOT_STACK_ONLY
			/* allocate alarm in heap if none is available in stack */
			if ( !alarm )
			{
				alarm = (struct iot_alarm *)
					os_malloc(sizeof( struct iot_alarm ));
				is_in_heap = IOT_TRUE;
			}
#endif

			if( alarm )
			{
				unsigned int cur_idx = 0u;
				unsigned int min_idx = 0u;
				unsigned int max_idx = count;
				size_t name_len = os_strlen(name);

				if( name_len > IOT_NAME_MAX_LEN )
					name_len = IOT_NAME_MAX_LEN;

				os_memzero( alarm, sizeof( struct iot_alarm ) );

#ifdef IOT_STACK_ONLY
				alarm->name = alarm->_name;
#else
				alarm->name = os_malloc( name_len + 1u );
#endif
				if( alarm->name )
				{
					os_strncpy( alarm->name, name, name_len );
					alarm->name[name_len] = '\0';
					alarm->lib = lib;

#ifndef IOT_STACK_ONLY
					alarm->is_in_heap = is_in_heap;
#endif /* ifndef IOT_STACK_ONLY */

					/* place in alphabetical order */
					while ( max_idx - min_idx > 0u )
					{
						int cmp_result;
						cur_idx = (max_idx - min_idx) / 2u + min_idx;
						cmp_result = os_strncmp( name,
							lib->alarm_ptr[cur_idx]->name,
							name_len );
						if ( cmp_result > 0 )
						{
							++cur_idx;
							min_idx = cur_idx;
						}
						else
							max_idx = cur_idx;
					}

					/* insert into proper spot in list */
					os_memmove( &lib->alarm_ptr[cur_idx + 1u],
						&lib->alarm_ptr[cur_idx],
						sizeof(struct iot_alarm *) * (count - cur_idx) );
					lib->alarm_ptr[cur_idx] = alarm;
					++lib->alarm_count;
				}
#ifndef IOT_STACK_ONLY
				else
				{
					if ( is_in_heap )
						os_free( alarm );
					alarm = NULL;
				}
#endif /* ifndef IOT_STACK_ONLY */
			}

			if ( !alarm )
				IOT_LOG( lib, IOT_LOG_ERROR,
					"failed to allocate memory for alarm: %s",
					name );
		}else
			IOT_LOG( lib, IOT_LOG_ERROR,
				"no remaining space (max: %u) for alarm: %s",
				IOT_ALARM_MAX, name );
#ifndef IOT_NO_THREAD_SUPPORT
		os_thread_mutex_unlock( &lib->alarm_mutex );
#endif
	}
	return alarm;
}

iot_status_t iot_alarm_deregister(
	iot_alarm_t *alarm )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if( alarm )
	{
		struct iot *const lib = alarm->lib;
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( lib )
		{
			unsigned int i, max;
#ifndef IOT_NO_THREAD_SUPPORT
			os_thread_mutex_lock( &lib->alarm_mutex );
#endif
			/* find alarm within the library */
			max = lib->alarm_count;
			for ( i = 0u; ( i < max ) &&
				( lib->alarm_ptr[ i ] != alarm );
				++i );

			if ( i < max )
			{
#ifndef IOT_STACK_ONLY
				const iot_bool_t is_in_heap =
					alarm->is_in_heap;

				if ( alarm->name )
					os_free( alarm->name );
#endif /* ifndef IOT_STACK_ONLY */

				/* remove from library */
				os_memmove(
					&lib->alarm_ptr[ i ],
					&lib->alarm_ptr[ i + 1u ],
					sizeof( struct iot_alarm * ) * ( max - i - 1u ) );

				/* set lib to NULL */
				alarm->lib = NULL;

				/* clear/free the alarm */
				--lib->alarm_count;
#ifndef IOT_STACK_ONLY
				if ( is_in_heap == IOT_FALSE )
				{
					lib->alarm_ptr[
						lib->alarm_count] = alarm;
				}
				else
				{
					lib->alarm_ptr[
						lib->alarm_count] = NULL;
					os_free( alarm );
					alarm = NULL;
				}
#else
				lib->alarm_ptr[
					lib->alarm_count] = alarm;
#endif /* ifndef IOT_STACK_ONLY */
				result = IOT_STATUS_SUCCESS;
			}
#ifndef IOT_NO_THREAD_SUPPORT
			os_thread_mutex_unlock( &lib->alarm_mutex );
#endif
		}
	}

	return result;
}

iot_status_t iot_alarm_publish(
	const iot_alarm_t *alarm,
	iot_transaction_t *txn,
	const iot_options_t *options,
	iot_severity_t severity )
{
	return iot_alarm_publish_string( alarm, txn, options, severity, NULL );
}

iot_status_t iot_alarm_publish_string(
	const iot_alarm_t *alarm,
	iot_transaction_t *txn,
	const iot_options_t *options,
	iot_severity_t severity,
	const char *message )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( alarm )
	{
		result = IOT_STATUS_NOT_INITIALIZED;
		if ( alarm->lib )
		{
			iot_alarm_data_t payload;

			payload.severity = severity;
			if ( message )
				payload.message = message;
			else
				payload.message = NULL;

			result = iot_plugin_perform(
				alarm->lib, txn, NULL,
				IOT_OPERATION_ALARM_PUBLISH,
				alarm, &payload, options );
		}
	}
	return result;
}
