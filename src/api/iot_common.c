/**
 * @file
 * @brief source file defining internal library functions
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "iot_common.h"

#include <float.h>      /* for FLT_MIN, DBL_MIN */
#include <math.h>       /* for fabs */

/**
 * @brief Helper function to convert between data types when extracting
 *        information
 *
 * @param[in,out]  to                  object to convert to
 * @param[in]      from                object to convert from
 * @param[in]      convert             whether to allow conversion of types
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_BAD_REQUEST      unable to perform conversion
 * @retval IOT_STATUS_SUCCESS                    on success
 */
static IOT_SECTION iot_status_t iot_perform_conversion( struct iot_data *to,
	const struct iot_data *from, iot_bool_t convert );

/**
 * @brief Helper function to determine if a real number has no decimal portion
 *
 * @param[in]      number              number to check
 *
 * @retval IOT_TRUE                    number has not decimal portion
 * @retval IOT_FALSE                   number has a decimal portion
 */
static IOT_SECTION iot_bool_t iot_common_data_no_decimal( double number );

iot_status_t iot_perform_conversion( struct iot_data *to,
	const struct iot_data *from, iot_bool_t convert )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( to && from )
	{
		/* we can handle types that are the same */
		result = IOT_STATUS_BAD_REQUEST;
		if ( to->type == from->type )
		{
			os_memcpy( to, from, sizeof( struct iot_data ) );
			to->heap_storage = NULL; /* new object doesn't own heap */
			result = IOT_STATUS_SUCCESS;
		}
		else if ( from->has_value != IOT_FALSE && convert != IOT_FALSE )
		{
			to->heap_storage = NULL; /* new object doesn't own heap */
			switch ( to->type )
			{
			case IOT_TYPE_BOOL:
				if ( from->type == IOT_TYPE_INT8 )
					to->value.boolean =
						( from->value.int8 == 0 ?
							IOT_FALSE : IOT_TRUE );
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					/* do comparison for close to 0.0 */
					to->value.boolean = IOT_FALSE;
					if ( ( (float) fabs(
						(double)( from->value.float32 - 0.0f ) ) ) < FLT_MIN )
						to->value.boolean = IOT_TRUE;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					/* do comparison for close to 0.0 */
					to->value.boolean = IOT_FALSE;
					if ( fabs( from->value.float64 - 0.0 ) < DBL_MIN )
						to->value.boolean = IOT_TRUE;
				}
				else if ( from->type == IOT_TYPE_INT16 )
					to->value.boolean =
						( from->value.int16 == 0 ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_INT32 )
					to->value.boolean =
						( from->value.int32 == 0 ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_INT64 )
					to->value.boolean =
						( from->value.int64 == 0 ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_UINT8 )
					to->value.boolean =
						( from->value.uint8 == 0u ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_UINT16 )
					to->value.boolean =
						( from->value.uint16 == 0u ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_UINT32 )
					to->value.boolean =
						( from->value.uint32 == 0u ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_UINT64 )
					to->value.boolean =
						( from->value.uint64 == 0u ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_STRING )
				{
					/* false values are:
					   no, NO, FALSE, false, 0, "", NULL
					   (checking the firsta character)
					   else it's true */
					if ( from->value.string )
					{
						to->value.boolean = IOT_TRUE;
						if ( from->value.string[0] == 'F' ||
							from->value.string[0] == 'f' ||
							from->value.string[0] == 'n' ||
							from->value.string[0] == 'N' ||
							from->value.string[0] == '0' ||
							from->value.string[0] == '\0' )
							to->value.boolean = IOT_FALSE;
					}
					else
						to->value.boolean = IOT_FALSE;
				}
				else if ( from->type == IOT_TYPE_RAW )
					to->value.boolean =
						( from->value.raw.ptr == NULL ? IOT_FALSE: IOT_TRUE );
				else if ( from->type == IOT_TYPE_NULL )
					to->value.boolean = IOT_FALSE;
				result = IOT_STATUS_SUCCESS;
				break;
			case IOT_TYPE_FLOAT32:
			case IOT_TYPE_FLOAT64:
				/** @todo build conversion functions for
				 * float32 & float64 */
				break;
			case IOT_TYPE_INT8:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.int8 = (iot_int8_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.int8 = (iot_int8_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.int8 = (iot_int8_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT16 )
				{
					to->value.int8 = (iot_int8_t)from->value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT32 )
				{
					to->value.int8 = (iot_int8_t)from->value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT64 )
				{
					to->value.int8 = (iot_int8_t)from->value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT8 )
				{
					to->value.int8 = (iot_int8_t)from->value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT16 )
				{
					to->value.int8 = (iot_int8_t)from->value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT32 )
				{
					to->value.int8 = (iot_int8_t)from->value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT64 )
				{
					to->value.int8 = (iot_int8_t)from->value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.int8 = 0;
					if ( from->value.string )
						to->value.int8 =
							(iot_int8_t)os_strtol( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.int8 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_INT16:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.int16 = (iot_int16_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.int16 = (iot_int16_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.int16 = (iot_int16_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT8 )
				{
					to->value.int16 = (iot_int16_t)from->value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT32 )
				{
					to->value.int16 = (iot_int16_t)from->value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT64 )
				{
					to->value.int16 = (iot_int16_t)from->value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT8 )
				{
					to->value.int16 = (iot_int16_t)from->value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT16 )
				{
					to->value.int16 = (iot_int16_t)from->value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT32 )
				{
					to->value.int16 = (iot_int16_t)from->value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT64 )
				{
					to->value.int16 = (iot_int16_t)from->value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.int16 = 0;
					if ( from->value.string )
						to->value.int16 =
							(iot_int16_t)os_strtol( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.int16 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_INT32:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.int32 = (iot_int32_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.int32 = (iot_int32_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.int32 = (iot_int32_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT8 )
				{
					to->value.int32 = (iot_int32_t)from->value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT16 )
				{
					to->value.int32 = (iot_int32_t)from->value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT64 )
				{
					to->value.int32 = (iot_int32_t)from->value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT8 )
				{
					to->value.int32 = (iot_int32_t)from->value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT16 )
				{
					to->value.int32 = (iot_int32_t)from->value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT32 )
				{
					to->value.int32 = (iot_int32_t)from->value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT64 )
				{
					to->value.int32 = (iot_int32_t)from->value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.int32 = 0;
					if ( from->value.string )
						to->value.int32 =
							(iot_int32_t)os_strtol( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.int32 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_INT64:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.int64 = (iot_int64_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.int64 = (iot_int64_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.int64 = (iot_int64_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT8 )
				{
					to->value.int64 = (iot_int64_t)from->value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT16 )
				{
					to->value.int64 = (iot_int64_t)from->value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT32 )
				{
					to->value.int64 = (iot_int64_t)from->value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT8 )
				{
					to->value.int64 = (iot_int64_t)from->value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT16 )
				{
					to->value.int64 = (iot_int64_t)from->value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT32 )
				{
					to->value.int64 = (iot_int64_t)from->value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT64 )
				{
					to->value.int64 = (iot_int64_t)from->value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.int64 = 0;
					if ( from->value.string )
						to->value.int64 =
							(iot_int64_t)os_strtol( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.int64 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_LOCATION:
				/* no thing to do */
				break;
			case IOT_TYPE_UINT8:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.uint8 = (iot_uint8_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT8 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT16 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT32 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT64 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT16 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT32 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT64 )
				{
					to->value.uint8 = (iot_uint8_t)from->value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.uint8 = 0;
					if ( from->value.string )
						to->value.uint8 =
							(iot_uint8_t)os_strtoul( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.uint8 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_UINT16:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.uint16 = (iot_uint16_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT8 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT16 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT32 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT64 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT8 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT32 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT64 )
				{
					to->value.uint16 = (iot_uint16_t)from->value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.uint16 = 0;
					if ( from->value.string )
						to->value.uint16 =
							(iot_uint16_t)os_strtoul( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.uint16 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_UINT32:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.uint32 = (iot_uint32_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT8 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT16 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT32 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT64 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT8 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT16 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT64 )
				{
					to->value.uint32 = (iot_uint32_t)from->value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.uint32 = 0;
					if ( from->value.string )
						to->value.uint32 =
							(iot_uint32_t)os_strtoul( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.uint32 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_UINT64:
				if ( from->type == IOT_TYPE_BOOL )
				{
					to->value.uint64 = (iot_uint64_t)from->value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT32 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_FLOAT64 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT8 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT16 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT32 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_INT64 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT8 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT16 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_UINT32 )
				{
					to->value.uint64 = (iot_uint64_t)from->value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_STRING )
				{
					to->value.uint64 = 0;
					if ( from->value.string )
						to->value.uint64 =
							(iot_uint64_t)os_strtoul( from->value.string,
								NULL );
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.uint64 = 0;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_RAW:
				if ( from->type == IOT_TYPE_STRING )
				{
					if ( from->value.string )
					{
						to->value.raw.length =
							os_strlen( from->value.string );
						to->value.raw.ptr = from->value.string;
					}
					else
					{
						to->value.raw.length = 0u;
						to->value.raw.ptr = NULL;
					}
					result = IOT_STATUS_SUCCESS;
				}
				else if ( from->type == IOT_TYPE_NULL )
				{
					to->value.raw.length = 0u;
					to->value.raw.ptr = NULL;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			case IOT_TYPE_STRING:
				/* nothing can be converted to string
				   (due to required memory allocation)
				 */
				break;
			case IOT_TYPE_NULL:
				/* all types convert to NULL */
				result = IOT_STATUS_SUCCESS;
			}

			if ( result == IOT_STATUS_SUCCESS )
				to->has_value = IOT_TRUE;
		}
	}
	return result;
}

iot_status_t iot_common_arg_get( const struct iot_data *obj,
	iot_bool_t convert, iot_type_t type, va_list args )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( obj )
	{
		struct iot_data from_data;

		os_memzero( &from_data, sizeof( struct iot_data ) );
		from_data.type = type;
		result = iot_perform_conversion( &from_data, obj, convert );
		if ( result == IOT_STATUS_SUCCESS &&
			from_data.has_value == IOT_TRUE )
		{
			switch( type )
			{
			case IOT_TYPE_BOOL:
			{
				iot_bool_t* dest = (iot_bool_t*)va_arg(
					args, iot_bool_t* );
				if ( dest )
				{
					*dest = from_data.value.boolean;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_FLOAT32:
			{
				iot_float32_t* dest = (iot_float32_t*)va_arg(
					args, iot_float32_t* );
				if ( dest )
				{
					*dest = from_data.value.float32;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_FLOAT64:
			{
				iot_float64_t* dest = (iot_float64_t*)va_arg(
					args, iot_float64_t* );
				if ( dest )
				{
					*dest = from_data.value.float64;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_INT8:
			{
				iot_int8_t* dest = (iot_int8_t*)va_arg(
					args, iot_int8_t* );
				if ( dest )
				{
					*dest = from_data.value.int8;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_INT16:
			{
				iot_int16_t* dest = (iot_int16_t*)va_arg(
					args, iot_int16_t* );
				if ( dest )
				{
					*dest = from_data.value.int16;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_INT32:
			{
				iot_int32_t* dest = (iot_int32_t*)va_arg(
					args, iot_int32_t* );
				if ( dest )
				{
					*dest = from_data.value.int32;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_INT64:
			{
				iot_int64_t* dest = (iot_int64_t*)va_arg(
					args, iot_int64_t* );
				if ( dest )
				{
					*dest = from_data.value.int64;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_LOCATION:
			{
				const struct iot_location **data_obj =
					va_arg( args, const struct iot_location ** );
				if ( data_obj )
				{
					*data_obj = from_data.value.location;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_RAW:
			{
				struct iot_data_raw *data_obj =
					va_arg( args, struct iot_data_raw * );
				if ( data_obj )
				{
					data_obj->length = from_data.value.raw.length;
					data_obj->ptr = from_data.value.raw.ptr;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_NULL:
				/* do nothing */
				result = IOT_STATUS_SUCCESS;
				break;
			case IOT_TYPE_STRING:
			{
				const char **dest = (const char **)va_arg(
					args, const char** );
				if ( dest )
				{
					*dest = from_data.value.string;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_UINT8:
			{
				iot_uint8_t* dest = (iot_uint8_t*)va_arg(
					args, iot_uint8_t* );
				if ( dest )
				{
					*dest = from_data.value.uint8;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_UINT16:
			{
				iot_uint16_t* dest = (iot_uint16_t*)va_arg(
					args, iot_uint16_t* );
				if ( dest )
				{
					*dest = from_data.value.uint16;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_UINT32:
			{
				iot_uint32_t* dest = (iot_uint32_t*)va_arg(
					args, iot_uint32_t* );
				if ( dest )
				{
					*dest = from_data.value.uint32;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			case IOT_TYPE_UINT64:
			{
				iot_uint64_t* dest = (iot_uint64_t*)va_arg(
					args, iot_uint64_t* );
				if ( dest )
				{
					*dest = from_data.value.uint64;
					result = IOT_STATUS_SUCCESS;
				}
				break;
			}
			}
		}
	}
	return result;
}

iot_status_t iot_common_arg_set( struct iot_data *obj, iot_bool_t heap_alloc,
	 iot_type_t type, va_list args )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( obj )
	{
		result = IOT_STATUS_SUCCESS;

		os_memzero( obj, sizeof( struct iot_data ) );
		obj->type = type;
		obj->has_value = IOT_TRUE;
		switch( type )
		{
		case IOT_TYPE_BOOL:
			obj->value.boolean = (iot_bool_t)va_arg( args, int );
			break;
		case IOT_TYPE_FLOAT32:
			obj->value.float32 = (iot_float32_t)va_arg( args, double );
			break;
		case IOT_TYPE_FLOAT64:
			obj->value.float64 = (iot_float64_t)va_arg( args, double );
			break;
		case IOT_TYPE_INT8:
			obj->value.int8 = (iot_int8_t)va_arg( args, int );
			break;
		case IOT_TYPE_INT16:
			obj->value.int16 = (iot_int16_t)va_arg( args, int );
			break;
		case IOT_TYPE_INT32:
			obj->value.int32 = (iot_int32_t)va_arg( args, int );
			break;
		case IOT_TYPE_INT64:
			obj->value.int64 = (iot_int64_t)va_arg( args, uint64_t );
			break;
		case IOT_TYPE_LOCATION:
		{
			struct iot_location *const data_obj =
				va_arg( args, struct iot_location * );
			obj->value.location = NULL;
			if ( data_obj )
			{
				if ( heap_alloc != IOT_FALSE )
				{
					obj->heap_storage =
						os_realloc(
							obj->heap_storage,
							sizeof( struct iot_location )
							);
					if ( obj->heap_storage )
					{
						os_memcpy( obj->heap_storage,
							data_obj,
							sizeof( struct iot_location ) );
						obj->value.location = obj->heap_storage;
					}
				}
				else
					obj->value.location = data_obj;
			}
			break;
		}
		case IOT_TYPE_RAW:
		{
			const struct iot_data_raw *data_obj =
				va_arg( args, const struct iot_data_raw * );
			obj->value.raw.length = 0u;
			obj->value.raw.ptr = NULL;
			if ( data_obj && data_obj->ptr )
			{
				if ( heap_alloc != IOT_FALSE )
				{
					obj->heap_storage =
						os_realloc(
							obj->heap_storage,
							data_obj->length );
					if ( obj->heap_storage )
					{
						os_memcpy( obj->heap_storage,
							data_obj->ptr,
							data_obj->length );
						obj->value.raw.length = data_obj->length;
						obj->value.raw.ptr = obj->heap_storage;
					}
				}
				else
				{
					obj->value.raw.length = data_obj->length;
					obj->value.raw.ptr = data_obj->ptr;
				}
			}
			else
				result = IOT_STATUS_BAD_PARAMETER;
			break;
		}
		case IOT_TYPE_NULL:
			os_free_null( (void **)&obj->heap_storage );
			obj->value.raw.ptr = NULL;
			obj->value.raw.length = 0u;
			obj->type = type;
			obj->has_value = IOT_FALSE;
			break;
		case IOT_TYPE_STRING:
		{
			const char *src_str = va_arg( args, const char * );
			if ( heap_alloc != IOT_FALSE )
			{
				char *dest_str;
				size_t str_len = 0u;
				if ( src_str )
					str_len = os_strlen( src_str );
				dest_str = (char*)os_realloc(
					obj->heap_storage, str_len + 1u );
				if ( dest_str )
				{
					if ( src_str )
						os_memcpy( dest_str,
							src_str, str_len );
					dest_str[str_len] = '\0';
					obj->heap_storage = dest_str;
				}
				obj->value.string = dest_str;
			}
			else
				obj->value.string = src_str;
			break;
		}
		case IOT_TYPE_UINT8:
			obj->value.uint8 = (iot_uint8_t)va_arg( args, int );
			break;
		case IOT_TYPE_UINT16:
			obj->value.uint16 = (iot_uint16_t)va_arg( args, int );
			break;
		case IOT_TYPE_UINT32:
			obj->value.uint32 = (iot_uint32_t)va_arg( args, int );
			break;
		case IOT_TYPE_UINT64:
			obj->value.uint64 = va_arg( args, iot_uint64_t );
			break;
		}
	}
	return result;
}

iot_status_t iot_common_data_copy( struct iot_data *to,
	const struct iot_data *from, iot_bool_t copy_dynamic_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( to && from )
	{
		result = IOT_STATUS_SUCCESS;
		if ( to != from )
		{
			os_free_null( (void**)&to->heap_storage );
			os_memcpy( to, from, sizeof( struct iot_data ) );
		}
		if ( copy_dynamic_data != IOT_FALSE &&
			to->has_value != IOT_FALSE )
		{
			size_t mem_size = 0u;
			if ( to->type == IOT_TYPE_RAW )
			{
				if ( to->value.raw.ptr )
					mem_size = to->value.raw.length;
				else
				{
					to->has_value = IOT_FALSE;
					to->value.raw.length = 0u;
				}
			}
			else if ( to->type == IOT_TYPE_STRING )
			{
				/* supporting blank string from the cloud */
				if ( to->value.string )
					mem_size = os_strlen(
						to->value.string ) + 1u;
			}
			else if ( to->type == IOT_TYPE_LOCATION )
			{
				if ( to->value.location )
					mem_size = sizeof( struct iot_location );
			}

			if ( mem_size > 0u && ( to != from || !to->heap_storage ) )
			{
				to->heap_storage =
					os_malloc( mem_size );
				if ( to->heap_storage )
				{
					if ( to->type == IOT_TYPE_RAW )
					{
						os_memcpy(
							to->heap_storage,
							to->value.raw.ptr,
							to->value.raw.length );
						to->value.raw.ptr =
							to->heap_storage;
					}
					else if ( to->type == IOT_TYPE_STRING )
					{
						os_strncpy(
							(char*)to->heap_storage,
							to->value.string,
							mem_size );
						to->value.string = (const char*)
							to->heap_storage;
					}
					else if ( to->type == IOT_TYPE_LOCATION )
					{
						os_memcpy(
							to->heap_storage,
							to->value.location,
							sizeof( struct iot_location ) );
						to->value.location =
							to->heap_storage;
					}
				}
				else
				{
					if ( to->type == IOT_TYPE_RAW )
					{
						to->value.raw.length = 0u;
						to->value.raw.ptr = NULL;
					}
					else if ( to->type ==
						IOT_TYPE_STRING )
						to->value.string = NULL;
					else if ( to->type ==
						IOT_TYPE_LOCATION )
						to->value.location = NULL;
					to->has_value = IOT_FALSE;
					result = IOT_STATUS_NO_MEMORY;
				}
			}
			else if ( to != from )
				to->heap_storage = NULL;
		}
		else if ( to != from )
			to->heap_storage = NULL;
	}
	return result;
}

/**
 * @brief helper macro used in performing direct conversions (i.e. a cast).
 *
 * This function is just to make the code easier to read.
 *
 * @param[out]     to                  object that is converted to
 * @param[in]      type                type to cast to
 * @param[in]      from                object that is converted from
 *
 * @retval IOT_TRUE                    always
 */
#define IOT_CONVERT( to, type, from ) { (to) = (type)(from); result = IOT_TRUE; }

iot_bool_t iot_common_data_convert(
	iot_conversion_type_t conversion,
	iot_type_t to_type,
	struct iot_data *obj )
{
	iot_bool_t result = IOT_FALSE;
	if ( obj )
	{
		/* handle no conversion */
		result = ( to_type == obj->type ? IOT_TRUE : IOT_FALSE );

		/* handle basic conversion */
		if ( result == IOT_FALSE && conversion >= IOT_CONVERSION_BASIC )
		{
			if ( obj->has_value == IOT_FALSE )
			{
				os_memzero( obj, sizeof( struct iot_data ) );
				result = IOT_TRUE;
			}

			switch ( to_type )
			{
			case IOT_TYPE_FLOAT32:
				if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= (iot_float64_t)FLT_MAX )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.float64 )
				else if ( obj->type == IOT_TYPE_INT8 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_INT16 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_INT32 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_UINT16 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.uint16 )
				else if ( obj->type == IOT_TYPE_UINT32 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.uint32 )
				else if ( obj->type == IOT_TYPE_UINT64 )
					IOT_CONVERT( obj->value.float32, iot_float32_t, obj->value.uint64 )
				break;
			case IOT_TYPE_FLOAT64:
				if ( obj->type == IOT_TYPE_FLOAT32 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_INT8 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_INT16 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_INT32 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_UINT16 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.uint16 )
				else if ( obj->type == IOT_TYPE_UINT32 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.uint32 )
				else if ( obj->type == IOT_TYPE_UINT64 )
					IOT_CONVERT( obj->value.float64, iot_float64_t, obj->value.uint64 )
				break;
			case IOT_TYPE_INT8:
				if ( obj->type == IOT_TYPE_INT16 && obj->value.int16 >= SCHAR_MIN && obj->value.int16 <= SCHAR_MAX )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_INT32 && obj->value.int32 >= SCHAR_MIN && obj->value.int32 <= SCHAR_MAX )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 && obj->value.int64 >= SCHAR_MIN && obj->value.int64 <= SCHAR_MAX )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT8 && obj->value.uint8 <= SCHAR_MAX )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_UINT16 && obj->value.uint16 <= SCHAR_MAX )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.uint16 )
				else if ( obj->type == IOT_TYPE_UINT32 && obj->value.uint32 <= SCHAR_MAX )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.uint32 )
				else if( obj->type == IOT_TYPE_UINT64 && obj->value.uint64 <= SCHAR_MAX )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.uint64)
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= SCHAR_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= SCHAR_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.int8, iot_int8_t, obj->value.float64 )
				break;
			case IOT_TYPE_INT16:
				if ( obj->type == IOT_TYPE_INT8 )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_INT32 && obj->value.int32 >= SHRT_MIN && obj->value.int32 <= SHRT_MAX )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 && obj->value.int64 >= SHRT_MIN && obj->value.int64 <= SHRT_MAX )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT16 && obj->value.uint16 <= SHRT_MAX )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_UINT32 && obj->value.uint32 <= SHRT_MAX )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_UINT64 && obj->value.uint64 <= SHRT_MAX )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= SHRT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= SHRT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.int16, iot_int16_t, obj->value.float64 )
				break;
			case IOT_TYPE_INT32:
				if ( obj->type == IOT_TYPE_INT8 )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_INT16 )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_UINT16 )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.uint16 )
				else if ( obj->type == IOT_TYPE_INT64 && obj->value.int64 >= INT_MIN && obj->value.int64 <= INT_MAX )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT32 && obj->value.uint32 <= INT_MAX )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.uint32 )
				else if ( obj->type == IOT_TYPE_UINT64 && obj->value.uint64 <= INT_MAX )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= INT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= INT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.int32, iot_int32_t, obj->value.float64 )
				break;
			case IOT_TYPE_INT64:
				if ( obj->type == IOT_TYPE_INT8 )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.uint8 )
				else if (obj->type == IOT_TYPE_INT16 )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_UINT16 )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.uint16 )
				else if (obj->type == IOT_TYPE_INT32 )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.uint32 )
				else if ( obj->type == IOT_TYPE_UINT32 )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_UINT64 && obj->value.uint64 <= LONG_MAX )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= LONG_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= LONG_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.int64, iot_int64_t, obj->value.float64 )
				break;
			case IOT_TYPE_UINT8:
				if ( obj->type == IOT_TYPE_INT8 && obj->value.int8 >= 0 )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_INT16 && obj->value.int16 >= 0 && obj->value.int16 <= UCHAR_MAX )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_INT32 && obj->value.int32 >= 0 && obj->value.int32 <= UCHAR_MAX )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 && obj->value.int64 >= 0 && obj->value.int64 <= UCHAR_MAX )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT16 && obj->value.uint16 <= UCHAR_MAX )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.uint16 )
				else if ( obj->type == IOT_TYPE_UINT32 && obj->value.uint32 <= UCHAR_MAX )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.uint32 )
				else if ( obj->type == IOT_TYPE_UINT64 && obj->value.uint64 <= UCHAR_MAX )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= UCHAR_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= UCHAR_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.uint8, iot_uint8_t, obj->value.float64 )
				break;
			case IOT_TYPE_UINT16:
				if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_INT8 && obj->value.int8 >= 0 )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_INT16 && obj->value.int16 >= 0 )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_INT32 && obj->value.int32 >= 0 && obj->value.int32 <= USHRT_MAX )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 && obj->value.int64 >= 0 && obj->value.int64 <= USHRT_MAX )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT32 && obj->value.uint32 <= USHRT_MAX )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.uint32 )
				else if ( obj->type == IOT_TYPE_UINT64 && obj->value.uint64 <= USHRT_MAX )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= USHRT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= USHRT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.uint16, iot_uint16_t, obj->value.float64 )
				break;
			case IOT_TYPE_UINT32:
				if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_UINT16 )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.uint16 )
				else if ( obj->type == IOT_TYPE_INT8 && obj->value.int8 >= 0 )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_INT16 && obj->value.int16 >= 0 )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_INT32 && obj->value.int32 >= 0 )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 && obj->value.int64 >= 0 && obj->value.int64 <= UINT_MAX )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_UINT64 && obj->value.uint64 <= UINT_MAX )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.uint64 )
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= UINT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= UINT_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.uint32, iot_uint32_t, obj->value.float64 )
				break;
			case IOT_TYPE_UINT64:
				if ( obj->type == IOT_TYPE_UINT8 )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.uint8 )
				else if ( obj->type == IOT_TYPE_UINT16 )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.uint16 )
				else if ( obj->type == IOT_TYPE_UINT32 )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.uint32 )
				else if ( obj->type == IOT_TYPE_INT8 && obj->value.int8 >= 0 )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.int8 )
				else if ( obj->type == IOT_TYPE_INT16 && obj->value.int16 >= 0 )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.int16 )
				else if ( obj->type == IOT_TYPE_INT32 && obj->value.int32 >= 0 )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.int32 )
				else if ( obj->type == IOT_TYPE_INT64 && obj->value.int64 >= 0 )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.int64 )
				else if ( obj->type == IOT_TYPE_FLOAT32 && obj->value.float32 <= ULONG_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float32 ) )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.float32 )
				else if ( obj->type == IOT_TYPE_FLOAT64 && obj->value.float64 <= ULONG_MAX &&
					  iot_common_data_no_decimal( (double)obj->value.float64 ) )
					IOT_CONVERT( obj->value.uint64, iot_uint64_t, obj->value.float64 )
				break;
			case IOT_TYPE_BOOL:
			case IOT_TYPE_LOCATION:
			case IOT_TYPE_NULL:
			case IOT_TYPE_RAW:
			case IOT_TYPE_STRING:
			default: /* all non-convertible types */
				break;
			}
		}

		/* handle advanced conversion */
		if ( result == IOT_FALSE && conversion >= IOT_CONVERSION_ADVANCED )
		{
			switch ( to_type )
			{
			case IOT_TYPE_STRING:
				if ( obj->type == IOT_TYPE_BOOL )
					result = IOT_TRUE;
				/* fall through */
			case IOT_TYPE_BOOL:
				if ( obj->type == IOT_TYPE_RAW )
					result = IOT_TRUE;
				/* fall through */
			case IOT_TYPE_FLOAT32:
			case IOT_TYPE_FLOAT64:
			case IOT_TYPE_INT8:
			case IOT_TYPE_INT16:
			case IOT_TYPE_INT32:
			case IOT_TYPE_INT64:
			case IOT_TYPE_UINT8:
			case IOT_TYPE_UINT16:
			case IOT_TYPE_UINT32:
			case IOT_TYPE_UINT64:
				if ( obj->type == IOT_TYPE_NULL ||
				     obj->type == IOT_TYPE_BOOL ||
				     obj->type == IOT_TYPE_STRING ||
				     obj->type == IOT_TYPE_FLOAT32 ||
				     obj->type == IOT_TYPE_FLOAT64 ||
				     obj->type == IOT_TYPE_UINT8 ||
				     obj->type == IOT_TYPE_UINT16 ||
				     obj->type == IOT_TYPE_UINT32 ||
				     obj->type == IOT_TYPE_UINT64 ||
				     obj->type == IOT_TYPE_INT8 ||
				     obj->type == IOT_TYPE_INT16 ||
				     obj->type == IOT_TYPE_INT32 ||
				     obj->type == IOT_TYPE_INT64 )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_RAW:
				/** @todo test if valid base64 string here */
				if ( obj->type == IOT_TYPE_STRING || obj->type == IOT_TYPE_NULL )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_LOCATION:
			case IOT_TYPE_NULL:
			default: /* all non-convertible types */
				break;
			}
		}

		/* update object type */
		if ( result != IOT_FALSE )
			obj->type = to_type;
	}
	return result;
}

iot_bool_t iot_common_data_convert_check(
	iot_conversion_type_t conversion,
	iot_type_t to_type,
	const struct iot_data *from )
{
	iot_bool_t result = IOT_FALSE;
	if ( from )
	{
		/* no conversion case */
		result = (to_type == from->type ) ? IOT_TRUE : IOT_FALSE;

		/* basic conversion checks */
		if ( result == IOT_FALSE && conversion >= IOT_CONVERSION_BASIC )
		{
			/* handle case of types with no value */
			result = from->has_value == IOT_FALSE ? IOT_TRUE : IOT_FALSE;

			switch ( to_type )
			{
			case IOT_TYPE_FLOAT32:
				if ( ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= (iot_float64_t)FLT_MAX ) ||
				     from->type == IOT_TYPE_INT8 ||
				     from->type == IOT_TYPE_INT16 ||
				     from->type == IOT_TYPE_INT32 ||
				     from->type == IOT_TYPE_INT64 ||
				     from->type == IOT_TYPE_UINT8 ||
				     from->type == IOT_TYPE_UINT16 ||
				     from->type == IOT_TYPE_UINT32 ||
				     from->type == IOT_TYPE_UINT64 )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_FLOAT64:
				if ( from->type == IOT_TYPE_FLOAT32 ||
				     from->type == IOT_TYPE_INT8 ||
				     from->type == IOT_TYPE_INT16 ||
				     from->type == IOT_TYPE_INT32 ||
				     from->type == IOT_TYPE_INT64 ||
				     from->type == IOT_TYPE_UINT8 ||
				     from->type == IOT_TYPE_UINT16 ||
				     from->type == IOT_TYPE_UINT32 ||
				     from->type == IOT_TYPE_UINT64 )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_INT8:
				if ( ( from->type == IOT_TYPE_INT16 && from->value.int16 >= SCHAR_MIN && from->value.int16 <= SCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_INT32 && from->value.int32 >= SCHAR_MIN && from->value.int32 <= SCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_INT64 && from->value.int64 >= SCHAR_MIN && from->value.int64 <= SCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_UINT8 && from->value.uint8 <= SCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_UINT16 && from->value.uint16 <= SCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_UINT32 && from->value.uint32 <= SCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_UINT64 && from->value.uint64 <= SCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= SCHAR_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= SCHAR_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_INT16:
				if ( from->type == IOT_TYPE_INT8 || from->type == IOT_TYPE_UINT8 ||
				     ( from->type == IOT_TYPE_INT32 && from->value.int32 >= SHRT_MIN && from->value.int32 <= SHRT_MAX ) ||
				     ( from->type == IOT_TYPE_INT64 && from->value.int64 >= SHRT_MIN && from->value.int64 <= SHRT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT16 && from->value.uint16 <= SHRT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT32 && from->value.uint32 <= SHRT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT64 && from->value.uint64 <= SHRT_MAX ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= SHRT_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= SHRT_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_INT32:
				if ( from->type == IOT_TYPE_INT8 || from->type == IOT_TYPE_UINT8 ||
				     from->type == IOT_TYPE_INT16 || from->type == IOT_TYPE_UINT16 ||
				     ( from->type == IOT_TYPE_INT64 && from->value.int64 >= INT_MIN && from->value.int64 <= INT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT32 && from->value.uint32 <= INT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT64 && from->value.uint64 <= INT_MAX ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= INT_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= INT_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_INT64:
				if ( from->type == IOT_TYPE_INT8 || from->type == IOT_TYPE_UINT8 ||
				     from->type == IOT_TYPE_INT16 || from->type == IOT_TYPE_UINT16 ||
				     from->type == IOT_TYPE_INT32 || from->type == IOT_TYPE_UINT32 ||
				     ( from->type == IOT_TYPE_UINT64 && from->value.uint64 <= LONG_MAX ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= LONG_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= LONG_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_UINT8:
				if ( ( from->type == IOT_TYPE_INT8 && from->value.int8 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT16 && from->value.int16 >= 0 && from->value.int16 <= UCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_INT32 && from->value.int32 >= 0 && from->value.int32 <= UCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_INT64 && from->value.int64 >= 0 && from->value.int64 <= UCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_UINT16 && from->value.uint16 <= UCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_UINT32 && from->value.uint32 <= UCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_UINT64 && from->value.uint64 <= UCHAR_MAX ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= UCHAR_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= UCHAR_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_UINT16:
				if ( from->type == IOT_TYPE_UINT8 ||
				     ( from->type == IOT_TYPE_INT8 && from->value.int8 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT16 && from->value.int16 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT32 && from->value.int32 >= 0 && from->value.int32 <= USHRT_MAX ) ||
				     ( from->type == IOT_TYPE_INT64 && from->value.int64 >= 0 && from->value.int64 <= USHRT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT32 && from->value.uint32 <= USHRT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT64 && from->value.uint64 <= USHRT_MAX ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= USHRT_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= USHRT_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_UINT32:
				if ( from->type == IOT_TYPE_UINT8 || from->type == IOT_TYPE_UINT16 ||
				     ( from->type == IOT_TYPE_INT8 && from->value.int8 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT16 && from->value.int16 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT32 && from->value.int32 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT64 && from->value.int64 >= 0 && from->value.int64 <= UINT_MAX ) ||
				     ( from->type == IOT_TYPE_UINT64 && from->value.uint64 <= UINT_MAX ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= UINT_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= UINT_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_UINT64:
				if ( from->type == IOT_TYPE_UINT8 || from->type == IOT_TYPE_UINT16 || from->type == IOT_TYPE_UINT32 ||
				     ( from->type == IOT_TYPE_INT8 && from->value.int8 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT16 && from->value.int16 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT32 && from->value.int32 >= 0 ) ||
				     ( from->type == IOT_TYPE_INT64 && from->value.int64 >= 0 ) ||
				     ( from->type == IOT_TYPE_FLOAT32 && from->value.float32 <= ULONG_MAX && iot_common_data_no_decimal( (double)from->value.float32 ) ) ||
				     ( from->type == IOT_TYPE_FLOAT64 && from->value.float64 <= ULONG_MAX && iot_common_data_no_decimal( (double)from->value.float64 ) ) )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_BOOL:
			case IOT_TYPE_LOCATION:
			case IOT_TYPE_NULL:
			case IOT_TYPE_RAW:
			case IOT_TYPE_STRING:
			default: /* all non-convertible types */
				break;
			}
		}

		/* check for advanced conversion */
		if ( result == IOT_FALSE && conversion >= IOT_CONVERSION_ADVANCED )
		{
			switch ( to_type )
			{
			case IOT_TYPE_STRING:
				if ( from->type == IOT_TYPE_BOOL )
					result = IOT_TRUE;
				/* fall through */
			case IOT_TYPE_BOOL:
				if ( from->type == IOT_TYPE_RAW )
					result = IOT_TRUE;
				/* fall through */
			case IOT_TYPE_FLOAT32:
			case IOT_TYPE_FLOAT64:
			case IOT_TYPE_INT8:
			case IOT_TYPE_INT16:
			case IOT_TYPE_INT32:
			case IOT_TYPE_INT64:
			case IOT_TYPE_UINT8:
			case IOT_TYPE_UINT16:
			case IOT_TYPE_UINT32:
			case IOT_TYPE_UINT64:
				if ( from->type == IOT_TYPE_NULL ||
				     from->type == IOT_TYPE_BOOL ||
				     from->type == IOT_TYPE_STRING ||
				     from->type == IOT_TYPE_FLOAT32 ||
				     from->type == IOT_TYPE_FLOAT64 ||
				     from->type == IOT_TYPE_UINT8 ||
				     from->type == IOT_TYPE_UINT16 ||
				     from->type == IOT_TYPE_UINT32 ||
				     from->type == IOT_TYPE_UINT64 ||
				     from->type == IOT_TYPE_INT8 ||
				     from->type == IOT_TYPE_INT16 ||
				     from->type == IOT_TYPE_INT32 ||
				     from->type == IOT_TYPE_INT64 )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_RAW:
				/** @todo test if valid base64 string here */
				if ( from->type == IOT_TYPE_STRING || from->type == IOT_TYPE_NULL )
					result = IOT_TRUE;
				break;
			case IOT_TYPE_LOCATION:
			case IOT_TYPE_NULL:
			default: /* all non-convertible types */
				break;
			}
		}
	}
	return result;
}

iot_bool_t iot_common_data_no_decimal( double number )
{
	iot_bool_t result = IOT_FALSE;
	const double frac_part = (double)( number - (iot_int64_t)number );
	if ( ((float)frac_part - FLT_MIN) <= 0.0f &&
	     ((float)frac_part + FLT_MIN) >= 0.0f )
		result = IOT_TRUE;
	return result;
}

