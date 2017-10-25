/**
 * @file
 * @brief source file for IoT library plug-in support
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "shared/iot_types.h"
#include "os.h"

/**
 * @brief helper function to disable a plug-in specified by pointer
 *
 * @param[in]      lib                 library containing loaded plug-ins
 * @param[in]      p                   plug-in to disable
 * @param[in]      force               whether plug-in will be remove regardless
 *                                     of disable callback return status
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        plug-in not found or not enabled
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval ...                         status returned by the disable callback
 */
IOT_SECTION static iot_status_t iot_plugin_disable_by_ptr(
	iot_t *lib, iot_plugin_t *p, iot_bool_t force );

/**
 * @brief helper function to enable a plug-in specified by pointer
 *
 * @param[in]      lib                 library containing loaded plug-ins
 * @param[in]      p                   plug-in to enable
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_EXECUTION_ERROR  enable routine of plug-in failed
 * @retval IOT_STATUS_EXISTS           plug-in already enabled
 * @retval IOT_STATUS_FULL             maximum number already enabled
 * @retval IOT_STATUS_NOT_FOUND        plug-in not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION static iot_status_t iot_plugin_enable_by_ptr(
	iot_t *lib, iot_plugin_t *p );


iot_status_t iot_plugin_disable( iot_t *lib, const char *name )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && name )
	{
		unsigned int i;
		iot_plugin_t *p = NULL;

		/* search for the enabled plug-in */
		for ( i = 0u; i < lib->plugin_enabled_count && !p; ++i )
		{
			p = lib->plugin_enabled[i].ptr;
			if ( p && os_strcmp( p->name, name ) != 0 )
				p = NULL;
		}

		result = iot_plugin_disable_by_ptr( lib, p, IOT_FALSE );
	}
	return result;
}

iot_status_t iot_plugin_disable_all( iot_t *lib )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		while ( lib->plugin_enabled_count > 0u )
		{
			iot_plugin_disable_by_ptr( lib,
				lib->plugin_enabled[lib->plugin_enabled_count - 1u].ptr,
				IOT_TRUE );
		}
	}
	return result;
}

iot_status_t iot_plugin_disable_by_ptr( iot_t *lib, iot_plugin_t *p,
	iot_bool_t force )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		unsigned int i = 0u;

		result = IOT_STATUS_NOT_FOUND;
		while ( i < lib->plugin_enabled_count &&
			lib->plugin_enabled[i].ptr != p )
			++i;

		if ( p && i < lib->plugin_enabled_count )
		{

			result = IOT_STATUS_SUCCESS;
			if ( p->disable )
				result = p->disable( lib, p->data, force );

			if ( result == IOT_STATUS_SUCCESS || force )
			{
				/* plug-in enabled, insert into plug-in list */
				--lib->plugin_enabled_count;
				os_memmove( &lib->plugin_enabled[i],
					&lib->plugin_enabled[i + 1u],
					sizeof( struct iot_plugin_enabled ) *
						(lib->plugin_enabled_count - i) );
			}
		}
	}
	return result;
}

iot_status_t iot_plugin_enable( iot_t *lib, const char *name )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lib && name )
	{
		result = IOT_STATUS_FULL;
		if ( lib->plugin_enabled_count < IOT_PLUGIN_MAX )
		{
			unsigned int i;

			iot_plugin_t *p = NULL;
			for ( i = 0u; i < lib->plugin_count && !p; ++i )
			{
				p = lib->plugin_ptr[i];
				if ( !p || !p->name || os_strcmp( p->name, name ) != 0 )
					p = NULL;
			}

			result = iot_plugin_enable_by_ptr( lib, p );
		}
	}
	return result;
}

iot_status_t iot_plugin_enable_by_ptr( iot_t *lib, iot_plugin_t *p )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib )
	{
		result = IOT_STATUS_NOT_FOUND;
		if ( p )
		{
			iot_int32_t order = 0;
			unsigned int i;
			for ( i = 0u; i < lib->plugin_enabled_count; ++i )
			{
				if ( lib->plugin_enabled[i].ptr == p )
				{
					/* plug-in is already enabled */
					p = NULL;
					result = IOT_STATUS_EXISTS;
				}
			}

			/* enable plug-in */
			result = IOT_STATUS_FAILURE;
			if ( p && p->info( NULL, &order, NULL, NULL, NULL ) )
			{
				/* find order for plug-in */
				unsigned int cur_idx = 0u;
				unsigned int min_idx = 0u;
				unsigned int max_idx = lib->plugin_enabled_count;
				while ( max_idx - min_idx > 0u )
				{
					cur_idx = (max_idx - min_idx) / 2u + min_idx;
					if ( order > lib->plugin_enabled[cur_idx].order )
					{
						++cur_idx;
						min_idx = cur_idx;
					}
					else
						max_idx = cur_idx;
				}

				/* call plug-in enable routine */
				result = IOT_STATUS_SUCCESS;
				if ( p->enable )
					result = p->enable( lib, p->data );

				if ( result == IOT_STATUS_SUCCESS )
				{
					/* plug-in enabled, insert into plug-in list */
					os_memmove( &lib->plugin_enabled[cur_idx + 1u],
						&lib->plugin_enabled[cur_idx],
						sizeof( struct iot_plugin_enabled ) *
							(lib->plugin_enabled_count - cur_idx) );
					lib->plugin_enabled[cur_idx].ptr = p;
					lib->plugin_enabled[cur_idx].order = order;
					++lib->plugin_enabled_count;
				}
			}
		}
	}
	return result;
}

void iot_plugin_terminate(
	iot_t *lib,
	iot_plugin_t *p )
{
	if ( p && p->terminate )
		p->terminate( lib, p->data );
}

void iot_plugin_initialize(
	iot_t *lib,
	iot_plugin_t *p )
{
	if ( p && p->initialize )
		p->initialize( lib, &p->data );
}

iot_status_t iot_plugin_perform(
	iot_t *lib,
	iot_transaction_t *UNUSED(txn),
	iot_millisecond_t *max_time_out,
	iot_operation_t op,
	const void *item,
	const void *value,
	const iot_options_t *options )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_millisecond_t time_remaining;

	/* support newer method of specifying max_time_out */
	iot_uint64_t u32 = 0u;
	iot_options_get( options, "max_time_out",
		IOT_TRUE, IOT_TYPE_UINT32, &u32 );
	time_remaining = (iot_millisecond_t)u32;

	if ( max_time_out )
		time_remaining += *max_time_out;

	if ( lib )
	{
		iot_bool_t ignore_time_out = IOT_FALSE;
		iot_step_t i;
		result = IOT_STATUS_SUCCESS;
		if ( time_remaining == 0u )
			ignore_time_out = IOT_TRUE;

		for ( i = IOT_STEP_BEFORE; i <= IOT_STEP_AFTER
			&& (ignore_time_out || time_remaining > 0u); ++i )
		{
			unsigned int j;
			for ( j = 0u; j < lib->plugin_enabled_count; ++j )
			{
				iot_plugin_t *const p = lib->plugin_enabled[j].ptr;
				if ( p && p->execute )
				{
					iot_status_t interim_result =
						p->execute( lib, p->data, op,
							time_remaining, &i,
							item, value, options );
					if ( interim_result > result )
						result = interim_result;
				}
			}
		}
	}
	if ( max_time_out )
		*max_time_out = time_remaining;


	return result;
}

iot_status_t iot_plugin_load(
	iot_t *lib,
	const char *file )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && file )
	{
		result = IOT_STATUS_FULL;
		if ( lib->plugin_count < IOT_PLUGIN_MAX )
		{
			os_lib_handle handle = os_library_open( file );
			result = IOT_STATUS_NOT_EXECUTABLE;
			if ( handle )
			{
				const char *name = NULL;
				iot_version_t min = 0u;
				iot_version_t max = 0u;
				iot_int32_t order = 0;
				const iot_version_t ver = iot_version();
				iot_plugin_info_fptr info_func;
				iot_plugin_load_fptr load_func;
				*(void **)(&info_func) =
					os_library_find( handle, "iot_info" );
				*(void **)(&load_func) =
					os_library_find( handle, "iot_load" );
				if ( info_func && load_func &&
					info_func( &name, &order, NULL, &min, &max ) )
				{
					result = IOT_STATUS_NOT_SUPPORTED;
					if ( (min == 0u || min >= ver) &&
					     (max == 0u || max <= ver))
					{
						iot_plugin_t *const p =
							&lib->plugin[lib->plugin_count];
						load_func( p );
						iot_plugin_initialize( lib, p );
						++lib->plugin_count;
						iot_plugin_enable_by_ptr( lib, p );
						result = IOT_STATUS_SUCCESS;
					}
				}

				/* enable plug-in */
				if ( result != IOT_STATUS_SUCCESS )
					os_library_close( handle );
			}
		}
	}
	return result;
}

iot_status_t iot_plugin_unload( iot_t *lib, const char *name )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( lib && name )
	{
		unsigned int i;
		iot_plugin_t *p = NULL;
		result = IOT_STATUS_NOT_FOUND;
		for ( i = 0u; i < lib->plugin_count && !p; ++i )
		{
			p = lib->plugin_ptr[i];
			if ( !p || !p->name || os_strcmp( p->name, name ) != 0 )
				p = NULL;
		}

		if ( p )
		{
			/* disable the plug-in if it is enabled */
			iot_plugin_disable_by_ptr( lib, p, IOT_TRUE );

			/* unload the plug-in */
			iot_plugin_terminate( lib, p );
			if ( p->handle )
				os_library_close( p->handle );

			/* remove the plug-in from the list */
			--lib->plugin_count;
			os_memmove( &lib->plugin_ptr[i], &lib->plugin_ptr[i+1],
				sizeof( iot_plugin_t * ) * ( lib->plugin_count - i ) );
			lib->plugin_ptr[ lib->plugin_count ] = p;

			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

