/**
 * @file
 * @brief Header file for JSON schema operations within the IoT library
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
#ifndef IOT_JSON_SCHEMA_H
#define IOT_JSON_SCHEMA_H

#include <iot_json.h>

#ifdef __cplusplus
extern C {
#endif

/** @brief Represents a base JSON schema object validator */
typedef struct iot_json_schema iot_json_schema_t;
/** @brief Represents an item in a JSON schema object */
typedef void iot_json_schema_item_t;
/** @brief Represents an JSON object within a schema iterating through items */
typedef void iot_json_schema_object_iterator_t;

/**
 * @brief Validates the provided values with an array defined in a JSON schema object
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing an array
 * @param[in]      value               value to validate against schema
 * @param[in]      value_len           length of the value string
 * @param[out]     error_msg           message indicating reason for schema
 *                                     validation failure
 *
 * @retval IOT_TRUE                    value matches schema for the array
 * @retval IOT_FALSE                   value doesn't match or item isn't an
 *                                     array schema
 *
 * @see iot_json_schema_bool
 * @see iot_json_schema_integer
 * @see iot_json_schema_parse
 * @see iot_json_schema_number
 * @see iot_json_schema_real
 * @see iot_json_schema_string
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_array(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg );

/**
 * @brief Validates the provided values with a boolean defined in a JSON schema object
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing a boolean
 * @param[in]      value               value to validate against schema
 * @param[in]      value_len           length of the value string
 * @param[out]     error_msg           message indicating reason for schema
 *                                     validation failure
 *
 * @retval IOT_TRUE                    value matches schema for the boolean
 * @retval IOT_FALSE                   value doesn't match or item isn't an
 *                                     array schema
 *
 * @see iot_json_schema_array
 * @see iot_json_schema_integer
 * @see iot_json_schema_parse
 * @see iot_json_schema_number
 * @see iot_json_schema_real
 * @see iot_json_schema_string
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_bool(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg );

/**
 * @brief Determines if the dependencies for a item have been met
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema
 * @param[in]      keys_set            an array of keys that were set
 * @param[in]      keys_set_len        length of the array of keys are set
 *
 * @retval IOT_TRUE                    all dependencies have been met
 * @retval IOT_FALSE                   no all dependencies have been met
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_dependencies_achieved(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char *const *keys_set,
	size_t keys_set_len );

/**
 * @brief Returns the description, if defined, for an item in the schema
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema
 * @param[out]     description         returned description for the item,
 *                                     NULL if not defined
 * @param[out]     description_len     length of the description for the item,
 *                                     NULL if not defined
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        item was not found in schema
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_json_schema_format
 * @see iot_json_schema_parse
 * @see iot_json_schema_required
 * @see iot_json_schema_title
 * @see iot_json_schema_type
 */
IOT_API IOT_SECTION iot_status_t iot_json_schema_description(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char **description,
	size_t *description_len );

/**
 * @brief Returns the format string, if defined, for an item in the schema
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema
 * @param[out]     format              returned format string for the item,
 *                                     NULL if not defined
 * @param[out]     format_len          length of the format string for the item,
 *                                     NULL if not defined
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        item was not found in schema
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_json_schema_description
 * @see iot_json_schema_parse
 * @see iot_json_schema_required
 * @see iot_json_schema_title
 * @see iot_json_schema_type
 */
IOT_API IOT_SECTION iot_status_t iot_json_schema_format(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char **format,
	size_t *format_len );

/**
 * @brief Initializes the schema validator
 *
 * @param[in]      buf                 buffer to use for storing memory
 * @param[in]      len                 size of the buffer
 * @param[in]      flags               flags to use for schema validation
 *
 * @return a valid schema validator object
 *
 * @see iot_json_schema_parse
 * @see iot_json_schema_terminate
 */
IOT_API IOT_SECTION iot_json_schema_t *iot_json_schema_initialize(
	void *buf,
	size_t len,
	unsigned int flags );

/**
 * @brief Validates the provided values with an integer defined in a JSON schema object
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing an integer
 * @param[in]      value               value to validate against schema
 * @param[in]      value_len           length of the value string
 * @param[out]     error_msg           message indicating reason for schema
 *                                     validation failure
 *
 * @retval IOT_TRUE                    value matches schema for the integer
 * @retval IOT_FALSE                   value doesn't match or item isn't an
 *                                     array schema
 * @see iot_json_schema_array
 * @see iot_json_schema_bool
 * @see iot_json_schema_parse
 * @see iot_json_schema_number
 * @see iot_json_schema_real
 * @see iot_json_schema_string
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_integer(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg );

/**
 * @brief Returns an iterator for validating an object definition in a schema
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing an object
 *
 * @return an iterator to use for going through an object definition in a schema
 *         or null if items doesn't point to an object or any parameter is
 *         invalid
 *
 * @see iot_json_schema_object_iterator_next
 * @see iot_json_schema_object_iterator_key
 * @see iot_json_schema_object_iterator_value
 */
IOT_API IOT_SECTION iot_json_schema_object_iterator_t *
	iot_json_schema_object_iterator(
		iot_json_schema_t *schema,
		iot_json_schema_item_t *item );

/**
 * @brief Returns the key for the item being pointed to by the object iterator
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing an object
 * @param[in]      iter                iterator for the position in the object
 * @param[out]     key                 returned key for the item
 * @param[out]     key_len             length of the key for the item
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        item was not found in schema
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_json_schema_object_iterator
 * @see iot_json_schema_object_iterator_next
 * @see iot_json_schema_object_iterator_value
 */
IOT_API IOT_SECTION iot_status_t iot_json_schema_object_iterator_key(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	iot_json_schema_object_iterator_t *iter,
	const char **key,
	size_t *key_len );

/**
 * @brief Returns next item for validating an object definition in a schema
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing an object
 * @param[in]      iter                iterator for the position in the object
 *
 * @return an iterator pointing the next item in an object defition or NULL if
 *         it is at the end of the object definition
 *
 * @see iot_json_schema_object_iterator
 * @see iot_json_schema_object_iterator_key
 * @see iot_json_schema_object_iterator_value
 */
IOT_API IOT_SECTION iot_json_schema_object_iterator_t *
	iot_json_schema_object_iterator_next(
		iot_json_schema_t *schema,
		iot_json_schema_item_t *item,
		iot_json_schema_object_iterator_t *iter );

/**
 * @brief Returns the value for the item being pointed to by the object iterator
 *
 * @param[in]      schema              JSON schema object
 * @param[in]      item                item in the schema containing an object
 * @param[in]      iter                iterator for the position in the object
 * @param[out]     out                 returned item being pointed to within the
 *                                     object
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        item was not found in schema
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_json_schema_object_iterator
 * @see iot_json_schema_object_iterator_key
 * @see iot_json_schema_object_iterator_next
 */
IOT_API IOT_SECTION iot_status_t iot_json_schema_object_iterator_value(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	iot_json_schema_object_iterator_t *iter,
	iot_json_schema_item_t **out );

/**
 * @brief Parses a given JSON string for a schema and returns the root element
 *
 * @note the JSON string provided in the @c js parameter may not be checked for
 * the null-terminating character, always indicated the amount of valid
 * characters using the @c len parameter
 *
 * @param[in]      schema              JSON schema object
 * @param[in]      js                  pointer to a string containing JSON text
 * @param[in]      len                 length of the JSON string
 * @param[out]     root                the root schema element
 * @param[in,out]  error               error text on failure (optional)
 * @param[in]      error_len           size of the error string buffer
 *                                     (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NO_MEMORY        not enough memory available
 * @retval IOT_STATUS_PARSE_ERROR      invalid JSON schema object passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_json_schema_initialize
 */
IOT_API IOT_SECTION iot_status_t iot_json_schema_parse(
	iot_json_schema_t *schema,
	const char *js,
	size_t len,
	iot_json_schema_item_t **root,
	char *error,
	size_t error_len );

/**
 * @brief Validates the provided values with a number defined in a JSON schema object
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing a number
 * @param[in]      value               value to validate against schema
 * @param[in]      value_len           length of the value string
 * @param[out]     error_msg           message indicating reason for schema
 *                                     validation failure
 *
 * @retval IOT_TRUE                    value matches schema for the number
 * @retval IOT_FALSE                   value doesn't match or item isn't an
 *                                     array schema
 *
 * @see iot_json_schema_array
 * @see iot_json_schema_bool
 * @see iot_json_schema_integer
 * @see iot_json_schema_parse
 * @see iot_json_schema_real
 * @see iot_json_schema_string
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_number(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg );

/**
 * @brief Validates the provided values with a real number defined in a JSON
 *        schema
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing a real
 *                                     number
 * @param[in]      value               value to validate against schema
 * @param[in]      value_len           length of the value string
 * @param[out]     error_msg           message indicating reason for schema
 *                                     validation failure
 *
 * @retval IOT_TRUE                    value matches schema for the real number
 * @retval IOT_FALSE                   value doesn't match or item isn't an
 *                                     array schema
 *
 *
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_real(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg );

/**
 * @brief Returns true if the required flag is for a schema item
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema
 *
 * @retval IOT_TRUE                    the required flag is set
 * @retval IOT_FALSE                   the required flag is not set
 *
 * @see iot_json_schema_description
 * @see iot_json_schema_format
 * @see iot_json_schema_parse
 * @see iot_json_schema_type
 * @see iot_json_schema_title
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_required(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item );

/**
 * @brief Validates the provided values with an array defined in a JSON schema object
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema containing an array
 * @param[in]      value               value to validate against schema
 * @param[in]      value_len           length of the value string
 * @param[out]     error_msg           message indicating reason for schema
 *                                     validation failure
 *
 * @retval IOT_TRUE                    value matches schema for the array
 * @retval IOT_FALSE                   value doesn't match or item isn't an
 *                                     array schema
 *
 * @see iot_json_schema_array
 * @see iot_json_schema_bool
 * @see iot_json_schema_integer
 * @see iot_json_schema_parse
 * @see iot_json_schema_number
 * @see iot_json_schema_real
 */
IOT_API IOT_SECTION iot_bool_t iot_json_schema_string(
	iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char *value,
	size_t value_len,
	const char **error_msg );

/**
 * @brief Returns the title, if defined, for an item in the schema
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema
 * @param[out]     title               returned title for the item,
 *                                     NULL if not defined
 * @param[out]     title_len           length of the title for the item,
 *                                     NULL if not defined
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        item was not found in schema
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_json_schema_description
 * @see iot_json_schema_format
 * @see iot_json_schema_parse
 * @see iot_json_schema_required
 * @see iot_json_schema_type
 */
IOT_API IOT_SECTION iot_status_t iot_json_schema_title(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item,
	const char **title,
	size_t *title_len );

/**
 * @brief Frees any memory allocated for parsing a schema file
 *
 * @param[in]      schema              JSON schema object to free memory for
 *
 * @see iot_json_schema_initialize
 */
IOT_API IOT_SECTION void iot_json_schema_terminate(
	iot_json_schema_t *schema );

/**
 * @brief Returns type of item being pointed to withn a schema
 *
 * @param[in]      schema              JSON schema object containing details
 * @param[in]      item                item in the schema
 *
 * @retval IOT_JSON_TYPE_NULL          not a valid schema object
 * @retval IOT_JSON_TYPE_ARRAY         array definition in the schema
 * @retval IOT_JSON_TYPE_OBJECT        object definition in the schema
 * @retval IOT_JSON_TYPE_BOOL          boolean definition in the schema
 * @retval IOT_JSON_TYPE_INTEGER       integer definition in the schema
 * @retval IOT_JSON_TYPE_REAL          (real) number definition in the schema
 * @retval IOT_JSON_TYPE_STRING        string definition in the schema
 *
 * @see iot_json_schema_parse
 * @see iot_json_schema_required
 * @see iot_json_type
 */
IOT_API IOT_SECTION iot_json_type_t iot_json_schema_type(
	const iot_json_schema_t *schema,
	const iot_json_schema_item_t *item );

#ifdef __cplusplus
};
#endif

#endif /* ifndef IOT_JSON_SCHEMA_H */
