/**
 * @file
 * @brief source file containing location implementation
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
#include "shared/iot_types.h"     /* for struct iot_location */

/** @brief Maximum value of location heading property */
#define IOT_LOCATION_HEADING_MAX       360.0
/** @brief Minimum value of location heading property */
#define IOT_LOCATION_HEADING_MIN       0.0
/** @brief Minimum value of location latitude property */
#define IOT_LOCATION_LATITUDE_MAX      90.0
/** @brief Minimum value of location latitude property */
#define IOT_LOCATION_LATITUDE_MIN      -90.0
/** @brief Maximum value of location longitude property */
#define IOT_LOCATION_LONGITUDE_MAX     180.0
/** @brief Minimum value of location longitude property */
#define IOT_LOCATION_LONGITUDE_MIN     -180.0

iot_status_t iot_location_accuracy_set(
	iot_location_t *sample,
	iot_float64_t accuracy )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		sample->accuracy = accuracy;
		sample->flags |= IOT_FLAG_LOCATION_ACCURACY;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_location_t *iot_location_allocate(
	iot_float64_t latitude,
	iot_float64_t longitude )
{
	struct iot_location* sample = NULL;

	if ( latitude >= IOT_LOCATION_LATITUDE_MIN &&
	     latitude <= IOT_LOCATION_LATITUDE_MAX &&
	     longitude >= IOT_LOCATION_LONGITUDE_MIN &&
	     longitude <= IOT_LOCATION_LONGITUDE_MAX )
	{
		sample = (struct iot_location*)os_malloc(
			sizeof( struct iot_location ) );
		if ( sample )
		{
			os_memzero( sample, sizeof( struct iot_location ) );
			sample->latitude = latitude;
			sample->longitude = longitude;
		}
	}
	return sample;
}

iot_status_t iot_location_altitude_accuracy_set(
	iot_location_t *sample,
	iot_float64_t accuracy )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		sample->altitude_accuracy = accuracy;
		sample->flags |= IOT_FLAG_LOCATION_ALTITUDE_ACCURACY;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_location_altitude_set(
	iot_location_t *sample,
	iot_float64_t altitude )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		sample->altitude = altitude;
		sample->flags |= IOT_FLAG_LOCATION_ALTITUDE;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_location_free(
	iot_location_t *sample )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
#ifndef IOT_STACK_ONLY
		os_free_null( (void **)&sample->tag );
#endif /* ifndef IOT_STACK_ONLY */
		/* clear memory to help in cases of use after free */
		os_memzero( sample, sizeof( struct iot_location ) );
		os_free_null( (void **)&sample );
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_location_heading_set(
	iot_location_t *sample,
	iot_float64_t heading )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		result = IOT_STATUS_OUT_OF_RANGE;
		if ( heading >= IOT_LOCATION_HEADING_MIN &&
			heading < IOT_LOCATION_HEADING_MAX )
		{
			sample->heading = heading;
			sample->flags |= IOT_FLAG_LOCATION_HEADING;
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t iot_location_set(
	iot_location_t *sample,
	iot_float64_t latitude,
	iot_float64_t longitude )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		result = IOT_STATUS_OUT_OF_RANGE;
		if ( latitude >= IOT_LOCATION_LATITUDE_MIN &&
		     latitude <= IOT_LOCATION_LATITUDE_MAX &&
		     longitude >= IOT_LOCATION_LONGITUDE_MIN &&
		     longitude <= IOT_LOCATION_LONGITUDE_MAX )
		{
			sample->latitude = latitude;
			sample->longitude = longitude;
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t iot_location_source_set(
	iot_location_t *sample,
	iot_location_source_t source )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		if ( source >= IOT_LOCATION_SOURCE_UNKNOWN &&
		     source <= IOT_LOCATION_SOURCE_WIFI )
		{
			sample->source = source;
			sample->flags |= IOT_FLAG_LOCATION_SOURCE;
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

iot_status_t iot_location_speed_set(
	iot_location_t *sample,
	iot_float64_t speed )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		sample->speed = speed;
		sample->flags |= IOT_FLAG_LOCATION_SPEED;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_location_tag_set(
	iot_location_t *sample,
	const char *tag )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( sample )
	{
		if ( tag )
		{
			size_t tag_len = os_strlen( tag );
#ifndef IOT_STACK_ONLY
			char *tag_ptr = sample->tag;
#endif /* ifndef IOT_STACK_ONLY */
			if ( tag_len > IOT_NAME_MAX_LEN )
				tag_len = IOT_NAME_MAX_LEN;
#ifdef IOT_STACK_ONLY
			sample->tag = sample->_tag;
#else /* ifdef IOT_STACK_ONLY */
			tag_ptr = os_realloc( sample->tag, tag_len + 1u );
			if ( tag_ptr )
				sample->tag = tag_ptr;
			else
				result = IOT_STATUS_NO_MEMORY;

			if ( tag_ptr )
#endif /* else IOT_STACK_ONLY */
			{
				os_strncpy( sample->tag, tag, tag_len );
				sample->tag[ tag_len ] = '\0';
				sample->flags |= IOT_FLAG_LOCATION_TAG;
				result = IOT_STATUS_SUCCESS;
			}
		}
		else
		{
#ifndef IOT_STACK_ONLY
			if ( sample->tag )
				os_free( sample->tag );
#endif  /* ifndef IOT_STACK_ONLY */
			sample->tag = NULL;
			sample->flags &= (unsigned int)~IOT_FLAG_LOCATION_TAG;
			result = IOT_STATUS_SUCCESS;
		}
	}
	return result;
}

