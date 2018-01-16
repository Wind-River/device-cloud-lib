/**
 * @file
 * @brief declares common internal types within the api
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
#ifndef IOT_TYPES_H
#define IOT_TYPES_H

#include "os.h"
#include "iot_build.h"
#include "iot_defs.h"
#include "iot_plugin.h"

/* Flags */
/** @brief Run in a single thread */
#define IOT_FLAG_SINGLE_THREAD                   0x01

/** @brief Type containing information required for file transfer */
typedef struct iot_file_transfer                 iot_file_transfer_t;

/**
 * @brief Possible types for iot directories
 */
typedef enum {
	/**
	 * @brief Configuration directory (Linux default: "/etc/iot")
	 */
	IOT_DIR_CONFIG = 0,
	/**
	 * @brief Run-time directory (Linux default: "/var/lib/iot")
	 */
	IOT_DIR_RUNTIME,
} iot_dir_type_t;

/**
 * @brief Raw data type structure
 */
struct iot_data_raw
{
	/** @brief Length of the raw data */
	size_t length;
	/** @brief Pointer to the raw data (for writing) */
	const void *ptr;
};

/**
 * @brief Structure representing a generic data type
 */
struct iot_data
{
	/**
	 * @brief Holds the value of the data
	 */
	union
	{
		/** @brief boolean */
		iot_bool_t                 boolean;
		/** @brief 32-bit floating point number */
		iot_float32_t              float32;
		/** @brief 64-bit floating point number */
		iot_float64_t              float64;
		/** @brief 8-bit signed integer */
		iot_int8_t                 int8;
		/** @brief 16-bit signed integer */
		iot_int16_t                int16;
		/** @brief 32-bit signed integer */
		iot_int32_t                int32;
		/** @brief 64-bit signed integer */
		iot_int64_t                int64;
		/** @brief raw data */
		struct iot_data_raw        raw;
		/** @brief location object */
		const struct iot_location  *location;
		/** @brief string */
		const char                 *string;
		/** @brief 8-bit unsigned integer */
		iot_uint8_t                uint8;
		/** @brief 16-bit unsigned integer */
		iot_uint16_t               uint16;
		/** @brief 32-bit unsigned integer */
		iot_uint32_t               uint32;
		/** @brief 64-bit unsigned integer */
		iot_uint64_t               uint64;
	} value;
	/** @brief a value has been set for the object */
	iot_bool_t has_value;
	/** @brief any associated data stored on the heap */
	void *heap_storage;
	/** @brief type of stored data */
	iot_type_t type;
};

/**
 * @brief Macro to test if an @p iot_data object has a value set
 *
 * @param[in]      o                   object to test
 * @param[in]      t                   type to check if value is set
 */
#define IOT_DATA_HAS_VALUE( o, t ) \
	( ( o.has_value != IOT_FALSE ) && ( o.type == t ) )

/**
 * @brief parameter details for an action
 */
struct iot_action_parameter
{
	/** @brief parameter name */
	char *name;
	/** @brief data for the parameter */
	struct iot_data data;
	/** @brief type of parameter */
	iot_parameter_type_t type;
#ifdef IOT_STACK_ONLY
	/** @brief storage of name value on heap
	 *
	 * @note This is not to be used directly, use @p name instead
	 */
	char _name[ IOT_NAME_MAX_LEN + 1u ];
#endif /* ifdef IOT_STACK_ONLY */
};

/**
 * @brief option details
 */
struct iot_option
{
	/** @brief option name */
	char *name;
	/** @brief option data */
	struct iot_data data;
#ifdef IOT_STACK_ONLY
	/** @brief storage of name value on heap
	 *
	 * @note This is not to be used directly, use @p name instead
	 */
	char _name[ IOT_NAME_MAX_LEN + 1u ];
#endif /* IOT_STACK_ONLY */
};

/**
 * @brief map of options
 */
struct iot_options
{
	/** @brief pointer to library */
	struct iot *lib;
	/** @brief array of option values */
	struct iot_option *option;
	/** @brief number of current options */
	iot_uint8_t option_count;
#ifdef IOT_STACK_ONLY
	/** @brief storage of name value on heap
	 *
	 * @note This is not to be used directly, use @p name instead
	 */
	struct iot_option _option[ IOT_OPTION_STACK_MAX ];
#endif /* IOT_STACK_ONLY */
};

/**
 * @brief action details
 */
struct iot_action
{
	/** @brief library handle */
	struct iot *lib;
	/** @brief action name */
	char *name;
	/** @brief action specific flags */
	iot_uint8_t flags;
	/** @brief required action */
	enum iot_item_state state;
	/** @brief action callback */
	iot_action_callback_t *callback;
	/** @brief user data to pass to action callback */
	void *user_data;
	/** @brief command to execute */
	char *command;
	/** @brief value of option*/
	struct iot_option *option;
	/** @brief number of options*/
	iot_uint8_t option_count;
	/** @brief parameter details */
	struct iot_action_parameter *parameter;
	/** @brief number of parameters */
	iot_uint8_t parameter_count;
	/** @brief maximum amount of time to wait before returning failure */
	iot_millisecond_t time_limit;
#ifdef IOT_STACK_ONLY
	/** @brief storage of options on the stack
	 *
	 * @note This is not to be used directly, use @c option instead
	 */
	struct iot_option _option[ IOT_OPTION_MAX ];
	/** @brief storage of the command to execute on the stack
	 *
	 * @note This is not to be used directly, use @c command instead
	 */
	char _command[PATH_MAX + 1u];
	/** @brief storage of name value on the stack
	 *
	 * @note This is not to be used directly, use @c name instead
	 */
	char _name[ IOT_NAME_MAX_LEN + 1u ];
	/** @brief storage of parameters on the stack
	 *
	 * @note This is not to be used directly, use @c parameter instead
	 */
	struct iot_action_parameter _parameter[ IOT_PARAMETER_MAX ];
#else
	/** @brief location of the action, heap or stack */
	iot_bool_t is_in_heap;
#endif /* IOT_STACK_ONLY */
};

/**
 * @brief an action request from the cloud
 */
struct iot_action_request
{
	/** @brief library handle */
	struct iot *lib;
	/** @brief value of option*/
	struct iot_option *option;
	/** @brief number of option*/
	iot_uint8_t option_count;
	/** @brief detailed error message (may be null) */
	char *error;
	/** @brief Name of the action */
	char *name;
	/** @brief action specific flags */
	iot_uint8_t flags;
	/** @brief Request source */
	char *source;
	/** @brief Array of parameters for the action */
	struct iot_action_parameter *parameter;
	/** @brief number of parameters */
	iot_uint8_t parameter_count;
	/** @brief maximum amount of time to wait before returning failure */
	iot_millisecond_t time_limit;
	/** @brief result of the action */
	iot_status_t result;
#ifdef IOT_STACK_ONLY
	/** @brief holds value of options */
	struct iot_option _option[ IOT_OPTION_MAX ];
	/** @brief error message details */
	char _error[ IOT_NAME_MAX_LEN + 1u ];
	/** @brief storage of name value on heap
	 *
	 * @note This is not to be used directly, use @c name instead
	 */
	char _name[ IOT_NAME_MAX_LEN + 1u ];
	/** @brief parameter details */
	struct iot_action_parameter _parameter[ IOT_PARAMETER_MAX ];
	/** @brief Request source from the cloud */
	char _source[ IOT_ID_MAX_LEN + 1u];
#endif /* IOT_STACK_ONLY */
};

/**
 * @brief Alarm information
 */
struct iot_alarm
{
	/** @brief library handle */
	struct iot *lib;
	/** @brief alarm name */
	char *name;

#ifdef IOT_STACK_ONLY
	/** @brief storage of name value on heap
	 *
	 * @note This is not to be used directly, use @c name instead
	 */
	const char _name[ IOT_NAME_MAX_LEN + 1u ];
#else
	/** @brief location of the action, heap or stack */
	iot_bool_t is_in_heap;
#endif
};

/**
 * @brief Alarm publication data
 */
typedef struct iot_alarm_data
{
	/** @brief alarm severity */
	iot_severity_t severity;
	/** @brief alarm message */
	const char *message;
} iot_alarm_data_t;

/**
 * @brief Location information
 */
struct iot_location
{
	/** @brief Accuracy of latitude & longitude in metres */
	iot_float64_t accuracy;
	/** @brief Altitude in metres */
	iot_float64_t altitude;
	/** @brief Range of the altitude in metres */
	iot_float64_t altitude_accuracy;
	/** @brief Direction heading */
	iot_float64_t heading;
	/** @brief Flags for optional fields */
	iot_uint32_t flags;
	/** @brief Latitude in degrees */
	iot_float64_t latitude;
	/** @brief Longitude in degrees */
	iot_float64_t longitude;
	/** @brief Location source type */
	iot_uint32_t source;
	/** @brief Speed being currently traveled in metres/second */
	iot_float64_t speed;
	/** @brief Location tag */
	char *tag;
#ifdef IOT_STACK_ONLY
	/** @brief storage of the location tag value on stack
	 *
	 * @note This is not to be used directly, use @c tag instead
	 */
	char _tag[ IOT_NAME_MAX_LEN + 1u ];
#endif /* ifdef IOT_STACK_ONLY */
};

/**
 * @brief telemetry details
 */
struct iot_telemetry
{
	/** @brief library handle */
	struct iot *lib;
	/** @brief telemetry is registered */
	enum iot_item_state state;
	/** @brief name of telemetry */
	char *name;
	/** @brief holds value of option */
	struct iot_option *option;
	/** @brief number of options*/
	iot_uint8_t option_count;
	/** @brief sample time stamp */
	iot_timestamp_t time_stamp;
	/** @brief telemetry type */
	iot_type_t type;
#ifdef IOT_STACK_ONLY
	/** @brief storage of options on the stack
	 *
	 * @note This is not to be used directly, use @c option instead
	 */
	struct iot_option _option[ IOT_OPTION_MAX ];
	/** @brief storage of name value on the stack
	 *
	 * @note This is not to be used directly, use @c name instead
	 */
	char _name[ IOT_NAME_MAX_LEN + 1u ];
#else /* ifdef IOT_STACK_ONLY */
	/** @brief location of the telemetry, heap or stack */
	iot_bool_t is_in_heap;
#endif /* else IOT_STACK_ONLY */
};

/** @brief structure containing informaiton about a file upload or download */
struct iot_file_transfer
{
	/** @brief progress function callback */
	iot_file_progress_callback_t *callback;
	/** @brief cloud's file name */
	const char *name;
	/** @brief local file path */
	const char *path;
	/** @brief callback's user data */
	void *user_data;
};

/** @brief structure containing information about a file transfer progress */
struct iot_file_progress
{
	/** @brief flag for transfer completion */
	iot_bool_t completed;
	/** @brief transfer's percentage done */
	iot_float32_t percentage;
	/** @brief transfer's status */
	iot_status_t status;
};

#if 0
/**
 * @brief information about a transaction status for later querying
 */
struct iot_transaction
{
	/** @brief indicates status */
	iot_status_t status[IOT_PROTOCOL_STACKS];
};

/**
 * @brief information for passing data between two (or more) clients
 */
struct iot_message
{
	/** @brief source for the data */
	const char *source_id;
	/** @brief data destination */
	const char *dest_id;
	/** @brief data payload */
	struct iot_data payload;
};
#endif
/**
 * @brief structure holding data for eanble plug-ins
 */
struct iot_plugin_enabled
{
	/** @brief plug-in order index */
	int                         order;
	/** @brief pointer to the plug-in data */
	iot_plugin_t                *ptr;
};

/**
 * @brief library connection details
 */
struct iot
{
	/** @brief registered actions stored on the stack */
	struct iot_action           action[ IOT_ACTION_STACK_MAX ];
	/** @brief number of registered actions */
	iot_uint8_t                 action_count;
	/**
	 * @brief Pointer to which action objects are used or available
	 *
	 * @note if the index is < action_count are used.
	 *       if the index is >= action_count are available for use.
	 */
	struct iot_action           *action_ptr[ IOT_ACTION_MAX ];

	/** @brief registered alarms stored on the stack */
	struct iot_alarm            alarm[ IOT_ALARM_STACK_MAX ];
	/** @brief number of registered alarms */
	iot_uint8_t                 alarm_count;
	/**
	 * @brief Pointer to which alarm objects are used or available
	 *
	 * @note if the index is < alarm_count are used.
	 *       if the index is >= alarm_count are available for use.
	 */
	struct iot_alarm            *alarm_ptr[ IOT_ALARM_MAX ];

	/** @brief options lists */
	struct iot_options          **options;
	/** @brief global configuration option list */
	struct iot_options          *options_config;
	/** @brief number of options */
	iot_uint8_t                 options_count;

	/** @brief path to the configuration file */
	char                        *cfg_file_path;
	/** @brief unique id of the device */
	char                        *device_id;
	/** @brief unique id of the client or application */
	char                        *id;
	/** @brief initialization flags */
	iot_uint8_t                 flags;

	/** @brief holds plug-ins that are currently loaded */
	iot_plugin_t                plugin[ IOT_PLUGIN_MAX ];
	/** @brief holds pointer to used and avaiable plug-in slots */
	iot_plugin_t                *plugin_ptr[ IOT_PLUGIN_MAX ];
	/** @brief number of plug-ins loaded */
	unsigned int                plugin_count;
	/** @brief holds pointer to plugs-ins that are currently enabled */
	struct iot_plugin_enabled   plugin_enabled[ IOT_PLUGIN_MAX ];
	/** @brief number of plug-ins enabled */
	unsigned int                plugin_enabled_count;

	/** @brief registered telemetry stored on the stack */
	struct iot_telemetry        telemetry[ IOT_TELEMETRY_STACK_MAX ];
	/** @brief number of registered telemetry */
	iot_uint8_t                 telemetry_count;
	/**
	 * @brief Pointer to which telemetry objects are used or available
	 *
	 * @note if the index is < telemetry_count are used.
	 *       if the index is >= telemetry_count are available for use.
	 */
	struct iot_telemetry        *telemetry_ptr[ IOT_TELEMETRY_MAX ];

	/** @brief about to disconnect & quit */
	iot_bool_t                  to_quit;

	/* incoming actions to execute */
	/**
	 * @brief Storage of action requests queued to execute or in progress
	 */
	struct iot_action_request   request_queue[IOT_ACTION_QUEUE_MAX];
	/** @brief Pointer of free to locations to store action requests */
	struct iot_action_request   *request_queue_free[IOT_ACTION_QUEUE_MAX];
	/** @brief Number of spaces available to queue action requests */
	iot_uint8_t                 request_queue_free_count;
	/** @brief Index of requests waiting for a slot for processing */
	struct iot_action_request   *request_queue_wait[IOT_ACTION_QUEUE_MAX];
	/** @brief Number of action requests waiting to be processed */
	iot_uint8_t                 request_queue_wait_count;

	/* log support */
	/** @brief Function to call to log a message */
	iot_log_callback_t          *logger;
	/** @brief log level to filter logger output */
	iot_log_level_t             logger_level;
	/** @brief user data to pass to log callback */
	void                        *logger_user_data;

#ifndef IOT_NO_THREAD_SUPPORT
	/** @brief handle to a mutex to allow log correctly with multiple
	 * threads */
	os_thread_mutex_t           log_mutex;
	/** @brief handle to the main thread */
	os_thread_t                 main_thread;
	/** @brief Mutex to protect telemetry samples */
	os_thread_mutex_t           telemetry_mutex;
	/** @brief Mutex to protect alarm registration/deregistration */
	os_thread_mutex_t           alarm_mutex;

	/* worker threads */
	/** @brief Array of all worker threads for handling commands */
	os_thread_t                 worker_thread[IOT_WORKER_THREADS];
	/** @brief Mutex to protect signal condition variable */
	os_thread_mutex_t           worker_mutex;
	/** @brief Signal for waking up waiting threads */
	os_thread_condition_t       worker_signal;
	/** @brief Lock for commands which cannot run concurrently */
	os_thread_rwlock_t          worker_thread_exclusive_lock;
#endif /* ifndef IOT_NO_THREAD_SUPPORT */

#ifdef IOT_STACK_ONLY
	/** @brief storage of connect configuration filename
	 *
	 * @note This is not to be used directly, use @c connect_cfg_file_path instead
	 */
	char                        _cfg_file_path[PATH_MAX + 1u];
	/** @brief storage on the stack for the connect configuration
	 * @note This is not to be used directly, use @c connect_setting instead
	 */
	struct iot_connect_setting  _connect_setting;
	/** @brief storage on the stack for the device id (use 'device_id' instead) */
	char                        _device_id[ IOT_ID_MAX_LEN + 1u ];
	/** @brief storage on the stack for the app id (use 'id' instead) */
	char                        _id[ IOT_ID_MAX_LEN + 1u ];
	/** @brief storage of option maps */
	struct iot_options          *_options[ IOT_OPTION_STACK_MAX ];
#endif /* ifdef IOT_STACK_ONLY */
};

/**
 * @brief Returns the value of a configuration setting
 *
 * @param[in]      handle              library handle
 * @param[in]      name                configuration option name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  ...                 pointer to a variable of the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_option_set_raw
 * @see iot_action_option_set
 */
IOT_API IOT_SECTION iot_status_t iot_config_get(
	const iot_t *handle,
	const char *name,
	iot_bool_t convert,
	iot_type_t type,
	... );

/**
 * @brief Returns a raw value for a configuration setting
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                configuration option name
 * @param[in]      convert             convert to raw, if possible
 * @param[out]     length              amount of raw data
 * @param[in,out]  data                pointer to the raw data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_config_get
 * @see iot_config_set_raw
 */
IOT_API IOT_SECTION iot_status_t iot_config_get_raw(
	const iot_t *lib,
	const char *name,
	iot_bool_t convert,
	size_t *length,
	const void **data );

/**
 * @brief Sets an option value
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                configuration option name
 * @param[in]      type                type of option data
 * @param[in]      ...                 value of option data in the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of options reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_config_set_raw
 */
IOT_API IOT_SECTION iot_status_t iot_config_set(
	iot_t *lib,
	const char *name,
	iot_type_t type,
	... );

/**
 * @brief Sets a raw option value
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                configuration option name
 * @param[in]      length              length of option data
 * @param[in]      ptr                 pointer to option data
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of options reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_config_set
 */
IOT_API IOT_SECTION iot_status_t iot_config_set_raw(
	iot_t *lib,
	const char *name,
	size_t length,
	const void *ptr );

#ifndef iot_EXPORTS
#ifndef __clang__
/**
 * @brief Sets a configuration option value
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                configuration option name
 * @param[in]      type                configuration option type
 * @param[in]      data                configuration option value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FULL             maximum number of options reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 */
#	define iot_config_set( lib, name, type, data ) \
		iot_config_set( lib, name, type, data )

/**
 * @brief Returns the value of a confiugration option
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      name                configuration option name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  data                pointer to a variable of the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
#	define iot_config_get( lib, name, convert, type, data ) \
		iot_config_get( lib, name, convert, type, data )
#endif /* ifndef __clang__ */
#endif /* ifndef iot_EXPORTS */

/**
 * @brief Returns the value of a action option
 *
 * @param[in]      action              action object to get value from
 * @param[in]      name                option name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  ...                 pointer to a variable of the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_action_option_set_raw
 * @see iot_action_option_set
 */
IOT_API IOT_SECTION iot_status_t iot_action_option_get(
	const iot_action_t *action,
	const char *name,
	iot_bool_t convert,
	iot_type_t type,
	... );

#ifndef iot_EXPORTS
#ifndef __clang__
/**
 * @brief Gets an option for a action object
 *
 * @param[in,out]  action              object to set option for
 * @param[in]      name                option name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                option type
 * @param[in]      data                option value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
#	define iot_action_option_get( action, name, convert, type, data ) \
		iot_action_option_get( action, name, convert, type, data )
#endif /* ifndef __clang__ */
#endif /* ifndef iot_EXPORTS */

/**
 * @brief Processes a request if one is waiting for processing
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      max_time_out        maximum time to wait in milliseconds for
 *                                     request to process
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to function
 * @retval IOT_STATUS_SUCCESS          request successfully completed
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for request to be
 *                                     processed
 */
IOT_API IOT_SECTION iot_status_t iot_action_process( iot_t *lib,
	iot_millisecond_t max_time_out );

/**
 * @brief Internal function to handle action registration
 *
 * @param[in,out]  action              action to register
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait in milliseconds
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          pointer was successfully set
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for registration
 *
 * @see iot_action_register_callback
 * @see iot_action_register_command
 */
IOT_API IOT_SECTION iot_status_t iot_action_register(
	iot_action_t *action,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief Returns the value of a telemetry option
 *
 * @param[in,out]  telemetry           telemetry object to set
 * @param[in]      name                option name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  ...                 pointer to a variable of the type
 *                                     specified
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_telemetry_option_set_raw
 * @see iot_telemetry_option_set
 */
IOT_API IOT_SECTION iot_status_t iot_telemetry_option_get(
	const iot_telemetry_t *telemetry,
	const char *name,
	iot_bool_t convert,
	iot_type_t type,
	... );

#ifndef iot_EXPORTS
#ifndef __clang__
/**
 * @brief Gets an option for a telemetry object
 *
 * @param[in,out]  telemetry           object to set option for
 * @param[in]      name                option name
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                option type
 * @param[in]      data                option value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
#	define iot_telemetry_option_get( telemetry, name, convert, type, data ) \
		iot_telemetry_option_get( telemetry, name, convert, type, data )
#endif /* ifndef__clang__ */
#endif /* ifndef iot_EXPORTS */

/**
 * @brief Copies data stored in one object to another
 *
 * @param[in,out]  to                  destination to object
 * @param[in]      from                source object
 * @param[in]      copy_dynamic_data   whether to copy dynamic data into a new
 *                                     heap allocated buffer to point to data
 *                                     in the original structure
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NO_MEMORY        unable to allocate memory on the heap
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_API IOT_SECTION iot_status_t iot_common_data_copy( struct iot_data *to,
	const struct iot_data *from, iot_bool_t copy_dynamic_data );

#if 0
/* TODO ITEMS */

/** @brief Type repseenting client to client communication */
typedef struct iot_message                       iot_message_t;

/**
 * @brief Type for a callback function called when a message is received from
 * another iot device
 *
 * @param[in]      message             message received
 * @param[in]      user_data           pointer to user specific data
 */
typedef void (iot_message_callback_t)(
	iot_message_t *message,
	void *user_data );

/**
 * @brief Type for a callback function called when the agent state changes
 *
 * @param[in]      new_state           new state the agent just changed to
 * @param[in]      old_state           previous state the agent was in
 * @param[in]      user_data           pointer to user specific data
 */
typedef void (iot_state_callback_t)(
	iot_state_t new_state,
	iot_state_t old_state,
	void *user_data );
#endif

/* actions */
/**
 * @defgroup action_flags Flags for actions
 *
 * @see iot_actions_flags_set
 */
/** @brief Function will not return (fire and forget) */
#define IOT_ACTION_NO_RETURN           0x01
/** @brief Local exclusive lock */
#define IOT_ACTION_EXCLUSIVE_APP       0x02
/** @brief Remote exclusive lock */
#define IOT_ACTION_EXCLUSIVE_DEVICE    (0x04 | IOT_ACTION_EXCLUSIVE_APP)
/** @brief Truncate the service when being sent to client */
#define IOT_ACTION_TRUNCATE_SERVICE    0x08
/** @brief Ignore the time limit */
#define IOT_ACTION_NO_TIME_LIMIT       0x10
/** @} */

/**
 * @brief Sets flags on actions
 *
 * @param[in,out]  action              action to set exclusive flag
 * @param[in]      flags               flags to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_API IOT_SECTION iot_status_t iot_action_flags_set(
	iot_action_t *action,
	iot_uint8_t flags );

/**
 * @brief Gets directory name upon type
 *
 * @param[in]      type                directory type
 * @param[in]      buf                 buffer to write directory path to
 * @param[in]      buf_len             length of buffer
 *
 * @return the size of the path (not including the null-terminator)
 */
IOT_API IOT_SECTION size_t iot_directory_name_get(
	iot_dir_type_t type,
	char *buf,
	size_t buf_len );

/**
 * @brief Copies an action request object
 *
 * @param[in]      dest                copy destination
 * @param[in]      request             request to copy
 * @param[in]      var_data            start location to play variable data
 * @param[in]      var_data_size       size of variable location data buffer
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_NO_MEMORY        variable memory size was not large enough
 * @retval IOT_STATUS_SUCCESS          copy was successful
 *
 * @see iot_action_request_copy_size
 */
IOT_API IOT_SECTION iot_status_t iot_action_request_copy(
	iot_action_request_t *dest,
	const iot_action_request_t *request,
	void *var_data,
	size_t var_data_size );

/**
 * @brief Determines the amount of variable data size required to copy
 * an action request
 *
 * @param[in]      request             request object to calculate the
 *                                     amount variable data required
 *
 * @return the amount of variable data required to copy the request
 *
 * @see iot_action_request_copy
 */
size_t iot_action_request_copy_size(
	const iot_action_request_t *request );

#if 0
/* messaging */
/** @todo provide implementation */
IOT_SECTION iot_status_t iot_message_callback_set(
	iot_t *lib,
	iot_message_callback_t *func,
	void *user_data );

/** @todo provide implementation */
IOT_SECTION const char *iot_message_get_source(
	const iot_message_t *message );

/** @todo provide implementation */
IOT_SECTION const char *iot_message_get_destination(
	const iot_message_t *message );

/** @todo provide implementation */
IOT_SECTION iot_status_t iot_message_get(
	const iot_message_t *message,
	iot_type_t type,
	... );

/** @todo provide implementation */
IOT_SECTION iot_status_t iot_message_get_raw(
	const iot_message_t *message,
	size_t *length,
	const void **ptr );

/** @todo provide implementation */
IOT_SECTION iot_status_t iot_message_publish(
	iot_t *lib,
	const char *dest,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_type_t type,
	... );

/** @todo provide implementation */
IOT_SECTION iot_status_t iot_message_publish_raw(
	iot_t *lib,
	const char *dest,
	iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	size_t length,
	const void *ptr );

#ifndef iot_EXPORTS
#ifndef __clang__
/**
 * @brief Retreives data from an incoming message
 *
 * @param[in]      message             message to retreive value from
 * @param[in]      type                data type to receive
 * @param[out]     data_ptr            location to store value
 */
#	define iot_message_get( message, type, data_ptr ) \
		iot_message_get( message, type, data_ptr )

/**
 * @brief Publishes a message for other clients
 *
 * @param[in]      lib                 library handle
 * @param[in]      dest                destination client
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      max_time_out        maximum time to wait for publishing
 * @param[in]      type                type of data being sent
 * @param[in]      data                data to send
 */
#	define iot_message_publish( lib, dest, txn, max_time_out, type, data ) \
		iot_message_publish( lib, dest, txn, max_time_out, type, data )
#endif /* ifndef __clang__ */
#endif /* ifndef iot_EXPORTS */

/** @todo provide implementation */
IOT_SECTION iot_status_t iot_state_get(
	iot_t *lib,
	iot_state_t *state,
	iot_millisecond_t max_time_out );
/** @todo provide implementation */
IOT_SECTION iot_status_t iot_state_callback_set(
	iot_t *lib,
	iot_state_callback_t *state_callback,
	void *user_data );

/* transaction */
/**
 * @brief Determine the status of a transaction
 *
 * @param[in,out]  txn                 transaction to query
 * @param[in]      max_time_out        maximum time to wait for query to be
 *                                     performed
 *
 * @retval IOT_TRUE                    entire transaction was successful
 * @retval IOT_FALSE                   transaction did not succeed
 */
IOT_SECTION iot_bool_t iot_transaction_status(
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out );
#endif

/* loop */
/**
 * @brief Performs one main loop iteration for the library
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      max_time_out        maximum time to wait for the iteration to
 *                                     complete
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_TIMED_OUT        timed out while waiting for loop
 *                                     iteration
 *
 * @see iot_loop_forever
 * @see iot_loop_start
 * @see iot_loop_stop
 */
IOT_SECTION iot_status_t iot_loop_iteration(
	iot_t *lib,
	iot_millisecond_t max_time_out );

/**
 * @brief Performs main loop indefinitely in the current thread for the library
 *
 * @param[in,out]  lib                 library handle
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success, after quit flag set
 *
 * @see iot_loop_iteration
 */
IOT_SECTION iot_status_t iot_loop_forever(
	iot_t *lib );

/**
 * @brief Starts a new thread to perform the main loop for the library
 *
 * @param[in,out]  lib                 library handle
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_NOT_SUPPORTED    library is either not compiled and/or not
 *                                     initialized with thread support
 *
 * @see iot_loop_start
 */
IOT_SECTION iot_status_t iot_loop_start(
	iot_t *lib );

/**
 * @brief Stops the main thread for the library
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      force               force kill thread if required
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval IOT_STATUS_NOT_SUPPORTED    library is either not compiled and/or not
 *                                     initialized with thread support
 *
 * @see iot_loop_start
 */
IOT_SECTION iot_status_t iot_loop_stop(
	iot_t *lib,
	iot_bool_t force );

/* helper function for log level setting */
/**
 * @brief Sets a log level for the service based on a string
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      log_level_str       string filter for the service
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        log_level string not found
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_log
 * @see IOT_LOG
 */
IOT_API IOT_SECTION iot_status_t iot_log_level_set_string(
	iot_t *lib,
	const char *log_level_str );

/**
 * @brief Returns the value of an option within a list
 *
 * @param[in]      options             options list to read
 * @param[in]      name                option name to retrieve
 * @param[in]      convert             convert to type, if possible
 * @param[in]      type                type of data to return
 * @param[in,out]  args                variable argument list with value
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_BAD_REQUEST      type doesn't match parameter
 *                                     (and not convertible, if set)
 * @retval IOT_STATUS_NOT_FOUND        option not found
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_options_clear
 * @see iot_options_get
 * @see iot_options_set_args
 *
 */
IOT_SECTION iot_status_t iot_options_get_args(
	const iot_options_t *options,
	const char *name,
	iot_bool_t convert,
	iot_type_t type,
	va_list args );

/**
 * @brief Sets the value of an option within a list
 *
 * @param[in,out]  options             options list to modify
 * @param[in]      name                item name to modify
 * @param[in]      type                type of value
 * @param[in]      args                argument with value to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to function
 * @retval IOT_STATUS_FULL             maximum number of options reached
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see iot_options_clear
 * @see iot_options_get_args
 * @see iot_options_set
 * @see iot_options_set_data
 */
IOT_SECTION iot_status_t iot_options_set_args(
	iot_options_t *options,
	const char *name,
	iot_type_t type,
	va_list args );

/**
 * @brief Sets the value of a piece of configuration data
 *
 * @param[in,out]  options            options list to set value
 * @param[in]      name               option name
 * @param[in]      data               option value to set
 *
 * @retval IOT_STATUS_BAD_PARAMETER   invalid parameter passed to the function
 * @retval IOT_STAUTS_FULL            maximum number of options reached
 * @retval IOT_STATUS_SUCCESS         on success
 *
 * @see iot_options_get
 * @see iot_options_set
 * @see iot_options_set_args
 */
IOT_SECTION iot_status_t iot_options_set_data(
	iot_options_t *options,
	const char *name,
	const struct iot_data *data );


/**
 * @brief Loads any built-in plugins
 *
 * @param[in,out]  lib                 library to load plug-ins into
 * @param[in]      max                 maximum number of plug-ins allowed
 *
 * @return The number of built-in plugins loaded
 */
IOT_SECTION unsigned int iot_plugin_builtin_load(
	iot_t* lib,
	unsigned int max );

/**
 * @brief Enables built-in plugins that are to be initially enabled on startup
 *
 * @param[in]      lib                 library containing plug-ins to enable
 *
 * @retval IOT_TRUE                    on success
 * @retval IOT_FALSE                   on failure (at least 1 failed to load)
 */
IOT_SECTION iot_bool_t iot_plugin_builtin_enable(
	iot_t *lib );

/**
 * @brief Disables an enabled plug-in by it's given name
 *
 * @param[in]      lib                 library containing loaded plug-ins
 * @param[in]      name                name of the plug-in to disable
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        plug-in not found or not enabled
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval ...                         status returned by the disable callback
 */
IOT_SECTION iot_status_t iot_plugin_disable(
	iot_t *lib,
	const char *name );

/**
 * @brief Disables all enabled plug-in
 *
 * @param[in]      lib                 library containing loaded plug-ins
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        plug-in not found or not enabled
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval ...                         status returned by the disable callback
 */
IOT_SECTION iot_status_t iot_plugin_disable_all(
	iot_t *lib );

/**
 * @brief enables a plug-in
 *
 * @param[in]      lib                 library holding plug-ins
 * @param[in]      name                name of plug-in to load
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_EXISTS           plug-in already enabled
 * @retval IOT_STATUS_FULL             maximum number already enabled
 * @retval IOT_STATUS_NOT_EXECUTABLE   enable routine of plug-in failed
 * @retval IOT_STATUS_NOT_FOUND        plug-in not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION IOT_API iot_status_t iot_plugin_enable(
	iot_t *lib,
	const char *name );

/**
 * @brief initializes a loaded plug-in
 *
 * @param[in]      lib                 library loading plug-in
 * @param[in,out]  p                   plug-in to initialize
 */
IOT_SECTION void iot_plugin_initialize(
	iot_t * lib,
	iot_plugin_t *p );

/**
 * @brief loads an external plug-in
 *
 * @param[in]      lib                 library to load plug-in into
 * @param[in]      file                path to plug-in file
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operating system failure to load plug-in
 * @retval IOT_STATUS_FULL             maximum number already enabled
 * @retval IOT_STATUS_NOT_EXECUTABLE   not a vaild plug-in file
 * @retval IOT_STATUS_NOT_FOUND        plug-in not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t iot_plugin_load(
	iot_t * lib,
	const char *file );

/**
 * @brief triggers all the plug-ins to perform an operation
 *
 * @param[in]      lib                 library holding plug-ins
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      op                  operation to perform
 * @param[in,out]  max_time_out        maximum time to wait in milliseconds
 *                                     (0 = wait indefinitely) (optional),
 *                                     returns amount of time remaining
 * @param[in]      item                item operating is being performed on
 *                                     (optional)
 * @param[in]      new_value           new value for item, type is based on
 *                                     @p op (optional)
 * @param[in]      options             optional options for the plug-in
 *                                     (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success
 * @retval ...                         status returned by the perform callback
 */
IOT_SECTION iot_status_t iot_plugin_perform(
	iot_t *lib,
	iot_transaction_t *txn,
	iot_millisecond_t *max_time_out,
	iot_operation_t op,
	const void *item,
	const void *new_value,
	const iot_options_t *options );

/**
 * @brief terminates a loaded plug-in
 *
 * @param[in]      lib                 library holding plug-in
 * @param[in,out]  p                   plug-in to terminate
 */
IOT_SECTION void iot_plugin_terminate(
	iot_t *lib,
	iot_plugin_t *p );

/**
 * @brief unloads a loaded plug-in
 *
 * @param[in]      lib                 library to load plug-in into
 * @param[in]      name                name of plug-in to unload
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NOT_FOUND        plug-in not found
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t iot_plugin_unload(
	iot_t *lib,
	const char *name );

/**
 * @brief Sets the path to the customized configuration file
 *
 * @param[in,out]  lib                 library handle
 * @param[in]      file_path           connect configuration file path
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_NO_MEMORY        failure due to out of memory
 * @retval IOT_STATUS_NO_MEMORY        no memory to store configration filename
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_API IOT_SECTION iot_status_t iot_configuration_file_set(
	iot_t *lib,
	const char *file_path );

/**
 * @brief returns the name for a telemetry object
 *
 * @param[in]      t                   telemetry object to get name from
 *
 * @return the name of the telemetry item
 */
IOT_SECTION IOT_API const char *iot_telemetry_name_get(
	const iot_telemetry_t *t );

/**
 * @brief Gets the device uuid
 *
 * @param[in]      filename            device uuid filename
 * @param[in,out]  buf                 buffer to return uuid string
 * @param[in]      len                 buffer length
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
IOT_SECTION iot_status_t iot_get_device_uuid( const char *filename,
	char *buf, size_t len);
#endif /* ifndef IOT_TYPES_H */

