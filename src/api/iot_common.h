/**
 * @file
 * @brief header file declaring internal library functions
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
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
#ifndef IOT_COMMON_H
#define IOT_COMMON_H

#include "shared/iot_types.h"

#include <stdarg.h>               /* for va_end, va_list, va_start */

/**
 * @brief type of conversion to perform
 */
enum iot_conversion_type
{
	/** @brief do not perform any conversion */
	IOT_CONVERSION_NONE = 0,

	/** @brief perform basic conversion (only perform if no loss of precision)
	 *
	 *  to float32:
	 *  - any float32 with a value < FLT_MAX
	 *  - all int8, int16, int32, int64, uint8, uint16, uint32, uint64 can be converted to float32
	 *
	 *  to float64:
	 *  - all float32 can be converted to float64
	 *  - all int8, int16, int32, int64, uint8, uint16, uint32, uint64 can be converted to float32
	 *
	 *  to int8 (:
	 *  - all int16, int32, int64 with a value in range: -128 - 127
	 *  - all uint8, uint16, uint32, uint64 with a value in range: 0 - 127
	 *
	 *  to int16:
	 *  - all int8, uint8 can be converted to int16
	 *  - all int32, int64 with a value in range: -32,768 - 32,767
	 *  - all uint16, uint32, uint64 with a value in range: 0 - 32,767
	 *
	 *  to int32:
	 *  - all int8, int16, uint8, uint16 can be converted to int32
	 *  - all int64 with a value in range: -2,147,483,648 - 2,147,483,647
	 *  - all uint32, uint64 with a value in range: 0 - 2,147,483,647
	 *
	 *  to int64:
	 *  - all int8, int16, int32, uint8, uint16, uint32 can all be converted to int64
	 *  - all uint64 with a value in range: 0 - 9,223,372,036,854,775,807
	 *
	 *  to boolean, location, raw, string:
	 *  - nothing converts
	 *
	 *  to uint8:
	 *  - all uint16, uint32, uint64 with a value: <= 255
	 *  - all int8, int16, int32, int64 with a value in range: 0 - 255
	 *
	 *  to uint16:
	 *  - all uint8 can be converted to uint16
	 *  - all uint32, uint64 with a value a value: <= 65,535
	 *  - all int8, int16, int32, int64 with a value in range: 0 - 65,535
	 *
	 *  to uint32:
	 *  - all uint8, uint16 can be converted to uint32
	 *  - all uint64 with a value: <= 4,294,967,295
	 *  - all int8, int16, int32, int64 with a value in range: 0 - 4,294,967,295
	 *
	 *  to uint64:
	 *  - all uint8, uint16, uint32 can all be converted to uint64
	 *  - all int8, int16, int32, int64 with a value >= 0
	 */
	IOT_CONVERSION_BASIC,

	/** @brief perform advanced conversion
	 *
	 *  to boolean:
	 *  - all numbers can be converted to boolean
	 *  - any raw containing data can be converted to boolean
	 *  - any string starting with:
	 *    - "("", '0', 'f', 'F', 'N', 'n', "Of", "OF", "oF" ) are IOT_FALSE
	 *    - all other values are IOT_TRUE
	 *
	 *  to float32:
	 *  to float64:
	 *  - all integers are converted to float32
	 *  - strings starting optionally with '-' or numbers and optionally a .
	 *
	 *  to int8:
	 *  to int16:
	 *  to int32:
	 *  to int64:
	 *  - all floats are down-casted converted to integers
	 *  - strings starting optionally with '-' until a non-number character
	 *
	 *  to location:
	 *  - nothing converts to locations
	 *
	 *  to raw:
	 *  - strings containing base64 data can be converted to raw
	 *
	 *  to string:
	 *  - all numbers (floats and integers) can be converted to strings
	 *  - boolean values can be converted to strings
	 *  - raw data can be converted to base64 string
	 *
	 *  to uint8:
	 *  to uint16:
	 *  to uint32:
	 *  to uint64:
	 *  - all floats are down-casted converted to unsigned integers
	 *  - strings until the first non-number character
	 */
	IOT_CONVERSION_ADVANCED
};

/** @brief Enumeration for the type of conversion to allow */
typedef enum iot_conversion_type iot_conversion_type_t;

/**
 * @brief Common function to retrieve a value from a iot_data structure
 *
 * @param[in]      obj                           structure to retrieve data from
 * @param[in]      convert                       allow conversion to desired
 *                                               type, if possible
 * @param[in]      type                          type of data to retrieve
 * @param[out]     args                          pointer to store result,
 *                                               based on @p type parameter
 *
 * @retval IOT_STATUS_BAD_PARAMETER              invalid parameter passed to
 *                                               function
 * @retval IOT_STATUS_BAD_REQUEST                type is either incorrect or
 *                                               value has not been set
 * @retval IOT_STATUS_SUCCESS                    on success
 *
 * @see iot_common_arg_set
 */
IOT_SECTION iot_status_t iot_common_arg_get( const struct iot_data *obj,
	iot_bool_t convert, iot_type_t type, va_list args );
/**
 * @brief Sets the value of a iot_data structure
 *
 * @param[in,out]  obj                           structure to set data of
 * @param[in]      heap_alloc                    allocate memory on heap and
 *                                               have @p obj take ownership, if
 *                                               required (raw & string types)
 * @param[in]      type                          type of data to set
 * @param[in]      args                          argument holding value to set,
 *                                               based on @p type parameter
 *
 * @retval IOT_STATUS_BAD_PARAMETER              invalid parameter passed to
 *                                               function
 * @retval IOT_STATUS_SUCCESS                    on success
 *
 * @see iot_commmon_arg_get
 */
IOT_SECTION iot_status_t iot_common_arg_set( struct iot_data *obj,
	iot_bool_t heap_alloc, iot_type_t type, va_list args );

/**
 * @brief Determines if two data objects can be converted
 *
 * @param[in]      conversion          type of conversion to perform
 * @param[in]      to_type             type being converted to
 * @param[in]      from                object being converted from
 *
 * @retval IOT_FALSE                   conversion is not possible
 * @retval IOT_TRUE                    conversion is possible
 */
IOT_SECTION iot_bool_t iot_common_data_convert_check(
	iot_conversion_type_t conversion,
	iot_type_t to_type,
	const struct iot_data *from );

IOT_SECTION iot_bool_t iot_common_data_convert(
	iot_conversion_type_t conversion,
	iot_type_t to_type,
	struct iot_data *obj );

#endif /* ifndef IOT_COMMON_H */

