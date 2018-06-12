/**
 * @file
 * @brief source file for the tr50 (telit) plugin
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

#include "../../shared/iot_base64.h"
#include "../../shared/iot_defs.h"
#include "../../shared/iot_types.h"

#include <iot_checksum.h>
#include <iot_json.h>
#include <iot_mqtt.h>
#include <iot_plugin.h>
#include <os.h>
#include <curl/curl.h>

#ifdef IOT_STACK_ONLY
#define TR50_IN_BUFFER_SIZE                 1024u
#endif /* ifdef IOT_STACK_ONLY */

/** @brief Maximum concurrent file transfers */
#define TR50_FILE_TRANSFER_MAX              10u
/** @brief Time interval in seconds to check file
 *         transfer queue */
#define TR50_FILE_QUEUE_CHECK_INTERVAL      30u * IOT_MILLISECONDS_IN_SECOND /* 30 seconds */
/** @brief Time interval in seconds for a file
 *         transfer to expire if it keeps failing */
#define TR50_FILE_TRANSFER_EXPIRY_TIME      1u * IOT_MINUTES_IN_HOUR * \
                                            IOT_SECONDS_IN_MINUTE * \
                                            IOT_MILLISECONDS_IN_SECOND /* 1 hour */
/** @brief Amount to offset the request id by */
#define TR50_FILE_REQUEST_ID_OFFSET         256u
/** @brief number of seconds before sending a keep alive message */
#define TR50_MQTT_KEEP_ALIVE                60u
/** @brief Time interval to send a ping if not data received */
#define TR50_PING_INTERVAL                  60 * IOT_MILLISECONDS_IN_SECOND
/** @brief Time interval to check mailbox if nothing */
#define TR50_MAILBOX_CHECK_INTERVAL         120 * IOT_MILLISECONDS_IN_SECOND
/** @brief Maximum number of actions to receive per mailbox check */
#define TR50_MAILBOX_CHECK_LIMIT            1
/** @brief Number of pings that can be missed before reconnection */
#define TR50_PING_MISS_ALLOWED              0u
/** @brief default QOS level */
#define TR50_MQTT_QOS                       1
/** @brief number of seconds to show "Connection loss message" */
#define TR50_TIMEOUT_CONNECTION_LOSS_MSG_MS 20u * IOT_MILLISECONDS_IN_SECOND /* 20 seconds */
/** @brief number of milliseconds between reconnect attempts */
#define TR50_TIMEOUT_RECONNECT_MS           5u * IOT_MILLISECONDS_IN_SECOND /* 5 seconds */
/** @brief Maximum length for a "thingkey" */
#define TR50_THING_KEY_MAX_LEN              ( IOT_ID_MAX_LEN * 2u ) + 1u

#ifdef IOT_THREAD_SUPPORT
/** @brief File transfer progress interval in seconds */
#define TR50_FILE_TRANSFER_PROGRESS_INTERVAL 5.0
/** @brief Default value for ssl verify host */
#define TR50_DEFAULT_SSL_VERIFY_HOST        2u
/** @brief Default value for ssl verify peer */
#define TR50_DEFAULT_SSL_VERIFY_PEER        1u
/** @brief Extension for temporary downloaded file */
#define TR50_DOWNLOAD_EXTENSION             ".part"
#endif /* ifdef IOT_THREAD_SUPPORT */

/** @brief structure containing informaiton about a file transfer */
struct tr50_file_transfer
{
	/** @brief progress function callback */
	iot_file_progress_callback_t *callback;
	/** @brief flag to cancel transfer */
	iot_bool_t cancel;
	/** @brief crc32 checksum */
	iot_uint64_t crc32;
	/** @brief time when transfer expired */
	iot_timestamp_t expiry_time;
	/** @brief last time progress was sent */
	double last_update_time;
	/** @brief curl handle */
	CURL *lib_curl;
	/** @brief cloud's file name */
	char name[ PATH_MAX + 1u ];
	/** @brief file operation (get/put) */
	iot_operation_t op;
	/** @brief local file path */
	char path[ PATH_MAX + 1u ];
	/** @brief total byte transfered in previous session(s) */
	long prev_byte;
	/** @brief pointer to plugin data */
	void *plugin_data;
	/** @brief file size */
	iot_uint64_t size;
	/** @brief next time transfer is retried */
	iot_timestamp_t retry_time;
	/** @brief cloud download url */
	char url[ PATH_MAX + 1u ];
	/** @brief Use global file store */
	iot_bool_t use_global_store;
	/** @brief callback's user data */
	void *user_data;
	/** @brief callback's maximum number of retries */
	iot_int64_t max_retries;
};

/** @brief internal data required for the plug-in */
struct tr50_data
{
	/** @brief number of times connection lost reported */
	iot_uint32_t connection_lost_msg_count;
	/** @brief file transfer queue */
	struct tr50_file_transfer file_transfer_queue[ TR50_FILE_TRANSFER_MAX ];
	/** @brief number of ongoing file transfer */
	iot_uint8_t file_transfer_count;
	/** @brief time when file transfer queue is last checked */
	iot_timestamp_t file_queue_last_checked;
	/** @brief library handle */
	iot_t *lib;
#ifdef IOT_THREAD_SUPPORT
	/** @brief mail related mutex to prevent concurrent checks */
	os_thread_mutex_t mail_check_mutex;
	/** @brief pointer to the mqtt connection to the cloud */
#endif /* IOT_THREAD_SUPPORT */
	iot_mqtt_t *mqtt;
	/** @brief current number of pings missed */
	iot_uint8_t ping_miss_count;
	/** @brief proxy details */
	struct iot_proxy proxy;
	/** @brief number of times reconnection has been attempted */
	iot_uint32_t reconnect_count;
	/** @brief the key of the thing */
	char thing_key[ TR50_THING_KEY_MAX_LEN + 1u ];
	/** @brief time when mailbox was last checked */
	iot_timestamp_t time_last_mailbox_check;
	/** @brief time when last message was received from cloud */
	iot_timestamp_t time_last_msg_received;
	/** @brief transaction status based on id */
	iot_uint32_t transactions[16u];
};

/** @brief transaction status values */
enum tr50_transaction_status
{
	TR50_TRANSACTION_UNKNOWN = 0x0, /**< @brief unknown transaction */
	TR50_TRANSACTION_INVOKED = 0x1, /**< @brief transaction sent */
	TR50_TRANSACTION_FAILURE = 0x2, /**< @brief failure received */
	TR50_TRANSACTION_SUCCESS = 0x3, /**< @brief success received */
};


/**
 * @brief function called to respond to the cloud on an action complete
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      action              action being executed
 * @param[in]      request             action request
 * @param[in]      txn                 transaction status information
 * @param[in]      options             map containing an optional options set
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_action_complete(
	struct tr50_data *data,
	const iot_action_t *action,
	const iot_action_request_t *request,
	const iot_transaction_t *txn,
	const iot_options_t *options );

/**
 * @brief publishes an alarm to the cloud
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      alarm               alarm object to publish
 * @param[in]      payload             alarm payload to publish
 * @param[in]      txn                 transaction status information
 * @param[in]      options             map containing an optional options set
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_attribute_publish
 * @see tr50_event_publish
 */
static IOT_SECTION iot_status_t tr50_alarm_publish(
	struct tr50_data *data,
	const iot_alarm_t *alarm,
	const iot_alarm_data_t *payload,
	const iot_transaction_t *txn,
	const iot_options_t *options );

/**
 * @brief appends location information to json structure
 *
 * @param[in,out]  json                structure to append to
 * @param[in]      key                 (optional) key enclouse output in
 * @param[in]      location            location to append
 */
static IOT_SECTION void tr50_append_location(
	iot_json_encoder_t *json,
	const char *key,
	const iot_location_t *location );

/**
 * @brief appends a raw data value to to json structure
 *
 * @param[in,out]  json                structure to append to
 * @param[in]      key                 key to assoicate with object
 * @param[in]      value               value to append
 * @param[in]      len                 length of value to append
 */
static IOT_SECTION void tr50_append_value_raw(
	iot_json_encoder_t *json,
	const char *key,
	const void *value,
	size_t len );

/**
 * @brief publishes an attribute to the cloud
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      key                 attribute key
 * @param[in]      value               attribute value
 * @param[in]      txn                 transaction status information
 * @param[in]      options             map containing an optional options set
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_alarm_publish
 * @see tr50_event_publish
 */
static IOT_SECTION iot_status_t tr50_attribute_publish(
	struct tr50_data *data,
	const char *key,
	const char *value,
	const iot_transaction_t *txn,
	const iot_options_t *options );

/**
 * @brief Sends the message to check the mailbox for any cloud requests
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      txn                 transaction status information
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_check_mailbox(
	struct tr50_data *data,
	const iot_transaction_t *txn );

/**
 * @brief helper function for tr50 to connect to the cloud
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      data                plug-in specific data
 * @param[in]      txn                 transaction status information
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 * @param[in]      is_reconnect        whether this is a reconnection
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_connect_check
 * @see tr50_disconnect
 * @see tr50_ping
 */
static IOT_SECTION iot_status_t tr50_connect(
	iot_t *lib,
	struct tr50_data *data,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_bool_t is_reconnect );

/**
 * @brief helper function for tr50 to check connection to the cloud
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      data                plug-in specific data
 * @param[in]      txn                 transaction status information
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_connect
 * @see tr50_ping
 */
static IOT_SECTION iot_status_t tr50_connect_check(
	iot_t *lib,
	struct tr50_data *data,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief plug-in function called to disable the plug-in
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      plugin_data         plugin specific data
 * @param[in]      force               force the plug-in to be disabled
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_enable
 * @see tr50_terminate
 */
iot_status_t tr50_disable(
	iot_t *lib,
	void* plugin_data,
	iot_bool_t force );

/**
 * @brief helper function for tr50 to disconnect from the cloud
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      data                plug-in specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_connect
 */
static IOT_SECTION iot_status_t tr50_disconnect(
	iot_t *lib,
	struct tr50_data *data );

/**
 * @brief plug-in function called to enable the plug-in
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      plugin_data         plugin specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_disable
 * @see tr50_execute
 * @see tr50_initialize
 */
iot_status_t tr50_enable(
	iot_t *lib,
	void* plugin_data );

/**
 * @brief called when event log api publish is called
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      message             message to publish
 * @param[in]      txn                 transaction status information
 * @param[in]      options             map containing an optional options set
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_alarm_publish
 * @see tr50_attribute_publish
 */
static IOT_SECTION iot_status_t tr50_event_publish(
	struct tr50_data *data,
	const char *message,
	const iot_transaction_t *txn,
	const iot_options_t *options );

/**
 * @brief plug-in function called to perform work in the plug-in
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      plugin_data         plugin specific data
 * @param[in]      op                  operation to perform
 * @param[in]      txn                 transaction status information
 * @param[in]      max_time_out        maximum time to perform operation
 *                                     (0 = indefinite)
 * @param[in]      step                pointer to the operation step
 * @param[in]      item                item being modified in the operation
 * @param[in]      value               new value for the item
 * @param[in]      options             map containing an optional options set
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_disable
 * @see tr50_enable
 * @see tr50_initialize
 */
iot_status_t tr50_execute(
	iot_t *lib,
	void* plugin_data,
	iot_operation_t op,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_step_t *step,
	const void *item,
	const void *value,
	const iot_options_t *options );


/**
 * @brief sends file.get or file.put rest api to tr50 requesting
 *        for file id, file size and crc
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      file_transfer       info required to transfer file
 * @param[in]      options             map containing an optional options set
 * @param[in]      txn                 transaction status information
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad params
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_file_request_send(
	struct tr50_data *data, iot_operation_t op,
	const iot_file_transfer_t *file_transfer,
	const iot_transaction_t *txn,
	const iot_options_t *options );

#ifdef IOT_THREAD_SUPPORT
/**
 * @brief a thread to perform file transfer
 *
 * @param[in]      arg                 info required to transfer file
 *
 * @retval IOT_STATUS_BAD_PARAMETER    bad params
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION OS_THREAD_DECL tr50_file_transfer(
	void* arg );

/**
 * @brief Callback called for progress updates (and to cancel transfers)
 *
 * @param[in]      user_data           pointer to information about the transfer
 * @param[in]      down_total          total number of bytes to download
 * @param[in]      down_now            current number of bytes downloaded
 * @param[in]      up_total            total number of bytes to upload
 * @param[in]      up_now              current number of bytes uploaded
 *
 * @retval 0 continue the transfer
 * @retval 1 cancel the transfer
 */
static IOT_SECTION int tr50_file_progress( void *user_data,
	curl_off_t down_total, curl_off_t down_now,
	curl_off_t up_total, curl_off_t up_now );

/**
 * @brief Callback called for progress updates (and to cancel transfers)
 *        for older versions of libcurl
 *
 * @param[in]      user_data           pointer to information about the transfer
 * @param[in]      down_total          total number of bytes to download
 * @param[in]      down_now            current number of bytes downloaded
 * @param[in]      up_total            total number of bytes to upload
 * @param[in]      up_now              current number of bytes uploaded
 *
 * @retval 0 continue the transfer
 * @retval 1 cancel the transfer
 */
static IOT_SECTION int tr50_file_progress_old(
	void *user_data,
	double down_total, double down_now,
	double up_total, double up_now );
#endif /* ifdef IOT_THREAD_SUPPORT */

/**
 * @brief checks file transfer queue and execute those which need retrying
 *
 * @param[in]      data                plug-in specific data
 */
static IOT_SECTION void tr50_file_queue_check(
	struct tr50_data *data );

/**
 * @brief plug-in function called to initialize the plug-in
 *
 * @param[in]      lib                 library handle
 * @param[out]     plugin_data         plugin specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_enable
 * @see tr50_terminate
 */
iot_status_t tr50_initialize(
	iot_t *lib,
	void **plugin_data );

/**
 * @brief helper fuction to publish data using MQTT
 *
 * @param[in,out]  data                plug-in specific data
 * @param[in]      topic               mqtt topic to send data on
 * @param[in]      payload             pointer to data to send
 * @param[in]      payload_len         size of the data to send
 * @param[in]      txn                 transaction status information
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_mqtt_publish(
	struct tr50_data *data,
	const char *topic,
	const void *payload,
	size_t payload_len,
	const iot_transaction_t *txn );

/**
 * @brief callback function that is called when tr50 receives a message from the
 *        cloud
 *
 * @param[in]      user_data           user specific data
 * @param[in]      topic               topic the message was received on
 * @param[in]      payload             payload that was received
 * @param[in]      payload_len         length of the received payload
 * @param[in]      qos                 mqtt quality of service level
 * @param[in]      retain              whether the message is to be retained
 */
static IOT_SECTION void tr50_on_message(
	void *user_data,
	const char *topic,
	void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain );

/**
 * @brief appends an option to the encoder if the key is set properly in the
 *        options map
 *
 * @param[out]     json                json encoder to add options to
 * @param[in]      json_key            key to add the item to in the json
 * @param[in]      options             map containing optional settings
 * @param[in]      options_key         key to search in the options map
 * @param[in]      type                type of data that must be set in map
 */
static IOT_SECTION void tr50_optional(
	iot_json_encoder_t *json,
	const char *json_key,
	const iot_options_t *options,
	const char *options_key,
	iot_type_t type );

/**
 * @brief sends a ping to the server if required (i.e. timeout expired)
 *
 * @param[in]      lib                 loaded iot library
 * @param[in]      data                plug-in specific data
 * @param[in]      txn                 transaction status information
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 *
 * @see tr50_connect
 * @see tr50_connect_check
 */
static IOT_SECTION void tr50_ping(
	iot_t *lib,
	struct tr50_data *data,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out );

/**
 * @brief convert a timestamp to a formatted time as in RFC3339
 *
 * @param[in]      ts                  time stamp to convert
 * @param[in,out]  out                 output buffer
 * @param[in]      len                 size of the output buffer
 *
 * @return a pointer to the output buffer
 */
static IOT_SECTION char *tr50_strtime(
	iot_timestamp_t ts,
	char *out,
	size_t len );

/**
 * @brief publishes a piece of iot telemetry to the cloud
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      t                   telemetry object to publish
 * @param[in]      d                   data for telemetry object to publish
 * @param[in]      txn                 transaction status information
 * @param[in]      options             map containing an optional options set
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 */
static IOT_SECTION iot_status_t tr50_telemetry_publish(
	struct tr50_data *data,
	const iot_telemetry_t *t,
	const struct iot_data *d,
	const iot_transaction_t *txn,
	const iot_options_t *options );

/**
 * @brief plug-in function called to terminate the plug-in
 *
 * @param[in]      lib                 library handle
 * @param[in]      plugin_data         plugin specific data
 *
 * @retval IOT_STATUS_FAILURE          on failure
 * @retval IOT_STATUS_SUCCESS          on success
 *
 * @see tr50_disable
 * @see tr50_initialize
 */
iot_status_t tr50_terminate(
	iot_t *lib,
	void *plugin_data );

/**
 * @brief function called to determine the status of a transaction
 *
 * @param[in]      data                plug-in specific data
 * @param[in]      txn                 transaction
 * @param[in]      options             optional parameters set for request
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_INVOKED          message has been sent/queue no response
 * @retval IOT_STATUS_FAILURE          failure status returned from cloud
 * @retval IOT_STATUS_NOT_FOUND        transaction is unknown
 * @retval IOT_STATUS_SUCCESS          success status returned from cloud
 * @retval IOT_STATUS_TIMED_OUT        function timed out before status returned
 *
 * @see tr50_disable
 * @see tr50_initialize
 */
static iot_status_t tr50_transaction_status(
	const struct tr50_data *data,
	const iot_transaction_t *txn,
	const iot_options_t *options );

/**
 * @brief sets the transaction status for an incoming transaction
 *
 * @param[in,out]  data                plug-in specific data
 * @param[in]      txn_id              transaction identifier
 * @param[in]      tx_status           status to set
 */
static void tr50_transaction_status_set(
	struct tr50_data *data,
	iot_uint8_t txn_id,
	enum tr50_transaction_status tx_status );


iot_status_t tr50_action_complete(
	struct tr50_data *data,
	const iot_action_t *UNUSED(action),
	const iot_action_request_t *request,
	const iot_transaction_t *txn,
	const iot_options_t *UNUSED(options) )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data )
	{
		const char *source = iot_action_request_source( request );

		result = IOT_STATUS_SUCCESS;
		if ( source && os_strncmp( source, "tr50", 4 ) == 0 )
		{
			const char *req_id;
			result = iot_action_request_option_get(
				request, "id", IOT_FALSE, IOT_TYPE_STRING, &req_id );
			if ( data && result == IOT_STATUS_SUCCESS && req_id && *req_id )
			{
				char buf[ 512u ];
				iot_json_encoder_t *json;
				json = iot_json_encode_initialize( buf, 512u, 0 );
				result = IOT_STATUS_NO_MEMORY;
				if ( json )
				{
					char id[11u];
					const char *msg = NULL;
					iot_status_t status;
					iot_action_request_parameter_iterator_t  iter;

					if ( txn )
						os_snprintf( id, sizeof(id), "%u", (unsigned int)(*txn) );
					else
						os_snprintf( id, sizeof(id), "cmd" );

					status = iot_action_request_status( request, &msg );

					iot_json_encode_object_start( json, id );
					iot_json_encode_string( json, "command", "mailbox.ack" );
					iot_json_encode_object_start( json, "params" );
					iot_json_encode_string( json, "id", req_id );
					iot_json_encode_integer( json, "errorCode", (int)status );
					if ( status != IOT_STATUS_SUCCESS && msg )
						iot_json_encode_string( json,
							"errorMessage", msg );

					if ( iot_action_request_parameter_iterator(
						request, IOT_PARAMETER_OUT,
						&iter ) == IOT_STATUS_SUCCESS )
					{
						iot_status_t res;
						iot_json_encode_object_start( json, "params" );
						do
						{
							const char *name;
							iot_type_t type;
							name = iot_action_request_parameter_iterator_name( request, iter );
							type = iot_action_request_parameter_iterator_data_type( request, iter );
							switch ( type )
							{
							case IOT_TYPE_BOOL:
							{
								iot_bool_t value = IOT_FALSE;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_FALSE,
									IOT_TYPE_BOOL,
									&value );
								iot_json_encode_bool( json, name, value );
								break;
							}
							case IOT_TYPE_FLOAT32:
							case IOT_TYPE_FLOAT64:
							{
								iot_float64_t value = 0.0;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_TRUE,
									IOT_TYPE_FLOAT64,
									&value );
								iot_json_encode_real( json, name, value );
								break;
							}
							case IOT_TYPE_INT8:
							case IOT_TYPE_INT16:
							case IOT_TYPE_INT32:
							case IOT_TYPE_INT64:
							case IOT_TYPE_UINT8:
							case IOT_TYPE_UINT16:
							case IOT_TYPE_UINT32:
							case IOT_TYPE_UINT64:
							{
								iot_int64_t value = 0;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_TRUE,
									IOT_TYPE_INT64,
									&value );
								iot_json_encode_integer( json, name, value );
								break;
							}
							case IOT_TYPE_RAW:
							{
								size_t raw_len = 0u;
								const void *raw = NULL;
								iot_action_request_parameter_iterator_get_raw(
									request,
									iter,
									IOT_FALSE,
									&raw_len,
									&raw );
								tr50_append_value_raw( json, name,
									raw, raw_len );
								break;
							}
							case IOT_TYPE_STRING:
							{
								const char *value;
								iot_action_request_parameter_iterator_get(
									request,
									iter,
									IOT_FALSE,
									IOT_TYPE_STRING,
									&value );
								iot_json_encode_string( json, name, value );
								break;
							}
							case IOT_TYPE_LOCATION:
							case IOT_TYPE_NULL:
							default:
								break;
							}
							res = iot_action_request_parameter_iterator_next( request,
								&iter );
						} while ( res == IOT_STATUS_SUCCESS );
						iot_json_encode_object_end( json );
					}

					iot_json_encode_object_end( json );
					iot_json_encode_object_end( json );

					msg = iot_json_encode_dump( json );
					result = tr50_mqtt_publish(
						data,
						"api",
						msg,
						os_strlen( msg ),
						txn );
					iot_json_encode_terminate( json );
				}
			}
		}
	}
	return result;
}

iot_status_t tr50_alarm_publish(
	struct tr50_data *data,
	const iot_alarm_t *alarm,
	const iot_alarm_data_t *payload,
	const iot_transaction_t *txn,
	const iot_options_t *options )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	char id[11u];
	const char *out_msg;

#ifdef IOT_STACK_ONLY
	char buffer[1024u];
	iot_json_encoder_t *const json =
		iot_json_encode_initialize( buffer, 1024u, 0 );
#else
	iot_json_encoder_t *const json =
		iot_json_encode_initialize( NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
#endif

	if ( txn )
		os_snprintf( id, sizeof(id), "%u", (unsigned int)(*txn) );
	else
		os_snprintf( id, sizeof(id), "cmd" );
	iot_json_encode_object_start( json, id );
	iot_json_encode_string( json, "command", "alarm.publish" );
	iot_json_encode_object_start( json, "params" );
	iot_json_encode_string( json, "thingKey",
		data->thing_key );
	iot_json_encode_string( json, "key", alarm->name);

	iot_json_encode_real( json, "state", payload->severity );
	if( payload->message && *payload->message != '\0')
		iot_json_encode_string( json, "msg", payload->message );

	/* publish optional arguments */
	tr50_optional( json, "ts", options, "time_stamp", IOT_TYPE_NULL );
	tr50_optional( json, NULL, options, "location", IOT_TYPE_LOCATION );
	tr50_optional( json, "republish", options, "republish", IOT_TYPE_BOOL );

	iot_json_encode_object_end( json );
	iot_json_encode_object_end( json );

	out_msg = iot_json_encode_dump( json );
	result = tr50_mqtt_publish(
		data, "api", out_msg, os_strlen( out_msg ), txn );
	iot_json_encode_terminate( json );
	return result;
}

void tr50_append_location(
	iot_json_encoder_t *json,
	const char *key,
	const iot_location_t *location )
{
	if ( location )
	{
		if ( key )
			iot_json_encode_object_start( json, key );
		iot_json_encode_real( json, "lat", location->latitude );
		iot_json_encode_real( json, "lng", location->longitude );
		if ( location->flags & IOT_FLAG_LOCATION_HEADING )
			iot_json_encode_real( json, "heading", location->heading );
		if ( location->flags & IOT_FLAG_LOCATION_ALTITUDE )
			iot_json_encode_real( json, "altitude", location->altitude );
		if ( location->flags & IOT_FLAG_LOCATION_SPEED )
			iot_json_encode_real( json, "speed", location->speed );
		if ( location->flags & IOT_FLAG_LOCATION_ACCURACY )
			iot_json_encode_real( json, "fixAcc", location->accuracy );
		if ( location->flags & IOT_FLAG_LOCATION_SOURCE )
		{
			const char *source;
			switch ( location->source )
			{
				case IOT_LOCATION_SOURCE_FIXED:
					source = "fixed";
					break;
				case IOT_LOCATION_SOURCE_GPS:
					source = "gps";
					break;
				case IOT_LOCATION_SOURCE_WIFI:
					source = "wifi";
					break;
				case IOT_LOCATION_SOURCE_UNKNOWN:
				default:
					source = "unknown";
			}
			iot_json_encode_string( json, "fixType", source );
		}
		if ( location->flags & IOT_FLAG_LOCATION_TAG )
			iot_json_encode_string( json, "street", location->tag );

		if ( key )
			iot_json_encode_object_end( json );
	}
}

void tr50_append_value_raw(
	iot_json_encoder_t *json,
	const char *key,
	const void *value,
	size_t len )
{
	uint8_t *heap = NULL;

	if ( !value )
		value = "";
	else if ( len != (size_t)-1 )
	{
		size_t req_len = iot_base64_encode_size( len );
		heap = os_malloc( sizeof(uint8_t) * req_len + 1u );
		if ( heap )
		{
			iot_base64_encode( (char*)heap, req_len, value, len );
			heap[req_len] = '\0';
			value = heap;
		}
	}

	/* write raw or string data */
	iot_json_encode_string( json, key, (const char *)value );

	if ( heap )
		os_free( heap );
}

iot_status_t tr50_attribute_publish(
	struct tr50_data *data,
	const char *key,
	const char *value,
	const iot_transaction_t *txn,
	const iot_options_t *options )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data && key && value )
	{
		iot_json_encoder_t *json;
#ifdef IOT_STACK_ONLY
		char buffer[1024u];
		json = iot_json_encode_initialize( buffer, 1024u, 0 );
#else
		json = iot_json_encode_initialize(
			NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
#endif
		result = IOT_STATUS_NO_MEMORY;
		if ( json )
		{
			char id[11u];
			const char *msg;

			if ( txn )
				os_snprintf( id, sizeof(id), "%u", (unsigned int)(*txn) );
			else
				os_snprintf( id, sizeof(id), "cmd" );
			iot_json_encode_object_start( json, id );
			iot_json_encode_string( json, "command",
				"attribute.publish" );
			iot_json_encode_object_start( json, "params" );
			iot_json_encode_string( json, "thingKey",
				data->thing_key );
			iot_json_encode_string( json, "key",
				key );
			iot_json_encode_string( json, "value",
				value );

			tr50_optional( json, "ts", options, "time_stamp",
				IOT_TYPE_NULL );
			tr50_optional( json, "republish", options, "republish",
				IOT_TYPE_BOOL );

			iot_json_encode_object_end( json );
			iot_json_encode_object_end( json );

			msg = iot_json_encode_dump( json );
			result = tr50_mqtt_publish(
				data, "api", msg, os_strlen( msg ), txn );
			iot_json_encode_terminate( json );
		}
	}
	return result;
}

iot_status_t tr50_check_mailbox(
	struct tr50_data *data,
	const iot_transaction_t *txn )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_bool_t check_mailbox = IOT_TRUE;
	if( data->lib->request_queue_free_count >= IOT_ACTION_QUEUE_MAX )
		check_mailbox = IOT_FALSE;

	/* check for any outstanding messages on the cloud */
	if ( check_mailbox != IOT_FALSE && data && data->lib )
	{
		char id[11u];
		const char *msg;
		char req_buf[376u];
		iot_json_encoder_t *req_json;

		/* build the message: "{\"cmd\":{\"command\":\"mailbox.check\"}}" */
		if ( txn )
			os_snprintf( id, sizeof(id), "%u", (unsigned int)(*txn) );
		else
			os_snprintf( id, sizeof(id), "check" );
		req_json = iot_json_encode_initialize(
			req_buf, 376u, 0 );
		iot_json_encode_object_start( req_json, id );
		iot_json_encode_string( req_json, "command", "mailbox.check" );
		iot_json_encode_object_start( req_json, "params" );
		iot_json_encode_integer( req_json, "limit", TR50_MAILBOX_CHECK_LIMIT );
		iot_json_encode_bool( req_json, "autoComplete", IOT_FALSE );
		iot_json_encode_object_end( req_json );
		iot_json_encode_object_end( req_json );
		msg = iot_json_encode_dump( req_json );
		result = IOT_STATUS_FAILURE;
		if ( msg )
		{
			/* Do not allow simultaneous pending outgoing checks */
#ifdef IOT_THREAD_SUPPORT
			os_thread_mutex_lock( &data->mail_check_mutex );
#endif /* IOT_THREAD_SUPPORT */
			if ( iot_timestamp_now() - data->time_last_mailbox_check
				> TR50_MAILBOX_CHECK_INTERVAL )
			{
				/* keep mqtt out of critical section */
				/* due to potential paho.mqtt deadlock */
				/* worst case scenario (publish failed)
				 * we wait out the timer */
				data->time_last_mailbox_check = iot_timestamp_now();
#ifdef IOT_THREAD_SUPPORT
				os_thread_mutex_unlock( &data->mail_check_mutex );
#endif /* IOT_THREAD_SUPPORT */
				result = tr50_mqtt_publish(
					data, "api", msg, os_strlen( msg ), txn );
			}
#ifdef IOT_THREAD_SUPPORT
			else
				os_thread_mutex_unlock( &data->mail_check_mutex );
#endif /* IOT_THREAD_SUPPORT */
		}
		else
			IOT_LOG( data->lib, IOT_LOG_ERROR, "%s",
				"Error failed to obtain device requests" );
		iot_json_encode_terminate( req_json );
	}
	return result;
}

iot_status_t tr50_connect(
	iot_t *lib,
	struct tr50_data *data,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_bool_t is_reconnect )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	const char *reason = "connect";
	if ( is_reconnect != IOT_FALSE )
		reason = "reconnect";
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", reason );

	if ( data )
	{
		const char *app_token = NULL;
		const char *ca_bundle = NULL;
		iot_mqtt_connect_options_t con_opts = IOT_MQTT_CONNECT_OPTIONS_INIT;
		const char *host = NULL;
		const char *proxy_type = NULL;
		iot_int64_t port = 0;
		iot_mqtt_ssl_t ssl_conf;
		iot_mqtt_proxy_t proxy_conf;
		iot_mqtt_proxy_t *proxy_conf_p = NULL;
		iot_bool_t validate_cert = IOT_FALSE;

		iot_config_get( lib, "cloud.host", IOT_FALSE,
			IOT_TYPE_STRING, &host );
		iot_config_get( lib, "cloud.port", IOT_FALSE,
			IOT_TYPE_INT64, &port );
		iot_config_get( lib, "cloud.token", IOT_FALSE,
			IOT_TYPE_STRING, &app_token );
		iot_config_get( lib, "ca_bundle_file", IOT_FALSE,
			IOT_TYPE_STRING, &ca_bundle );
		if ( !ca_bundle )
			ca_bundle = IOT_DEFAULT_CERT_PATH;
		iot_config_get( lib, "validate_cloud_cert", IOT_FALSE,
			IOT_TYPE_BOOL, &validate_cert );

		os_memzero( &ssl_conf, sizeof( iot_mqtt_ssl_t ) );
		ssl_conf.ca_path = ca_bundle;
		ssl_conf.insecure = !validate_cert;

		os_memzero( &proxy_conf, sizeof( iot_mqtt_proxy_t ) );
		if ( iot_config_get( lib, "proxy.type", IOT_FALSE,
			IOT_TYPE_STRING, &proxy_type )
				== IOT_STATUS_SUCCESS )
		{
			proxy_conf_p = &proxy_conf;
			iot_config_get( lib, "proxy.host", IOT_FALSE,
				IOT_TYPE_STRING, &proxy_conf.host );
			iot_config_get( lib, "proxy.port", IOT_FALSE,
				IOT_TYPE_INT64, &proxy_conf.port );
			iot_config_get( lib, "proxy.username", IOT_FALSE,
				IOT_TYPE_STRING, &proxy_conf.username );
			iot_config_get( lib, "proxy.password", IOT_FALSE,
				IOT_TYPE_STRING, &proxy_conf.password );
			if ( os_strcmp( proxy_type, "SOCKS5" ) == 0 )
				proxy_conf.type = IOT_PROXY_SOCKS5;
			else if ( os_strcmp( proxy_type, "HTTP" ) == 0 )
				proxy_conf.type = IOT_PROXY_HTTP;
			else
				proxy_conf.type = IOT_PROXY_UNKNOWN;
			os_memcpy( &data->proxy, &proxy_conf,
				sizeof( struct iot_proxy ) );
		}

		if ( app_token == NULL )
			IOT_LOG( lib, IOT_LOG_ERROR, "tr50 %s: %s",
				reason, "no application token provided" );

		os_snprintf( data->thing_key, TR50_THING_KEY_MAX_LEN,
			"%s-%s", lib->device_id, iot_id( lib ) );
		data->thing_key[ TR50_THING_KEY_MAX_LEN ] = '\0';

		con_opts.client_id = iot_id( lib );
		con_opts.host = host;
		con_opts.port = (iot_uint16_t)port;
		con_opts.keep_alive = TR50_MQTT_KEEP_ALIVE;
		con_opts.proxy_conf = proxy_conf_p;
		con_opts.ssl_conf = &ssl_conf;
		con_opts.username = data->thing_key;
		con_opts.password = app_token;
		con_opts.version = IOT_MQTT_VERSION_3_1_1;
		if ( is_reconnect == IOT_FALSE )
		{
			data->mqtt = iot_mqtt_connect( &con_opts, max_time_out );
			if ( data->mqtt )
				result = IOT_STATUS_SUCCESS;
		}
		else
		{
			result = iot_mqtt_reconnect( data->mqtt, &con_opts,
				max_time_out );
		}

		data->time_last_msg_received = iot_timestamp_now();
		data->time_last_mailbox_check = 0;
		data->ping_miss_count = 0u;
		if ( data->mqtt && result == IOT_STATUS_SUCCESS )
		{
			data->reconnect_count = 1u;
			data->connection_lost_msg_count = 1u;
			iot_mqtt_set_user_data( data->mqtt, data );
			iot_mqtt_set_message_callback( data->mqtt,
				tr50_on_message );
			iot_mqtt_subscribe( data->mqtt, "reply/#", TR50_MQTT_QOS );
			IOT_LOG( lib, IOT_LOG_INFO, "tr50 %s: %s",
				reason, "successfully" );
			result = tr50_check_mailbox( data, txn );
		}
		else if ( is_reconnect == IOT_FALSE ) /* show on connect only */
		{
			IOT_LOG( lib, IOT_LOG_ERROR,
				"tr50: failed to %s", reason );
		}
	}
	return result;
}

iot_status_t tr50_connect_check(
	iot_t *lib,
	struct tr50_data *data,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lib && data && data->mqtt )
	{
		iot_bool_t connected = IOT_TRUE;
		iot_timestamp_t time_stamp_changed = 0u;
		 iot_timestamp_t time_stamp_diff;

		/* obtain the current connection status */
		result = iot_mqtt_connection_status( data->mqtt,
			&connected, &time_stamp_changed );

		/* calculate the time since the last connection changed */
		time_stamp_diff  = iot_timestamp_now() - time_stamp_changed;

		if ( result == IOT_STATUS_SUCCESS && connected == IOT_FALSE )
		{
			result = IOT_STATUS_FAILURE; /* not connected */

			/* attempt to reconnect, if time out condition is met */
			if ( data->reconnect_count > 0u && time_stamp_diff >=
			     ( data->reconnect_count * TR50_TIMEOUT_RECONNECT_MS ) )
			{
				++data->reconnect_count;

				/* default to 1 second */
				if( max_time_out == 0u )
					max_time_out = IOT_MILLISECONDS_IN_SECOND;

				result = tr50_connect( lib, data, txn,
					max_time_out, IOT_TRUE );
				if ( result == IOT_STATUS_SUCCESS ||
				   ( time_stamp_diff >=
				     data->connection_lost_msg_count *
					TR50_TIMEOUT_CONNECTION_LOSS_MSG_MS ) )
				{
					++data->connection_lost_msg_count;
					IOT_LOG( lib, IOT_LOG_INFO,
						"tr50 connection loss for %u seconds",
						(unsigned int)(time_stamp_diff / IOT_MILLISECONDS_IN_SECOND) );
				}
			}
		}
	}
	return result;
}

iot_status_t tr50_disconnect(
	iot_t *lib,
	struct tr50_data *data )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "disconnect" );
	if ( data )
	{
		data->reconnect_count = 0u; /* don't reconnect */
		result = iot_mqtt_disconnect( data->mqtt );
	}
	return result;
}

iot_status_t tr50_disable(
	iot_t *lib,
	void *UNUSED(plugin_data),
	iot_bool_t UNUSED(force) )
{
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "disable" );
	/** @todo check if we are connected and call tr50_disconnect */
	/*
	if ( iot_connected( lib ) )
		tr50_disconnect( (struct tr50_data *)plugin_data );
	*/
	return IOT_STATUS_SUCCESS;
}

iot_status_t tr50_enable(
	iot_t *lib,
	void *UNUSED(plugin_data) )
{
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "enable" );
	/** @todo check if we are suppose to be connected an call: tr50_connect */
	/*
	if ( iot_connected( lib ) )
		tr50_connect( (struct tr50_data *)plugin_data );
	*/
	return IOT_STATUS_SUCCESS;
}

iot_status_t tr50_event_publish(
	struct tr50_data *data,
	const char *message,
	const iot_transaction_t *txn,
	const iot_options_t *options )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data && message )
	{
		iot_json_encoder_t *json;
#ifdef IOT_STACK_ONLY
		char buffer[1024u];
		json = iot_json_encode_initialize( buffer, 1024u, 0 );
#else
		json = iot_json_encode_initialize(
			NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
#endif
		result = IOT_STATUS_NO_MEMORY;
		if ( json )
		{
			char id[11u];
			iot_int64_t level;
			const char *msg;

			if ( txn )
				os_snprintf( id, sizeof(id), "%u", (unsigned int)(*txn) );
			else
				os_snprintf( id, sizeof(id), "cmd" );
			iot_json_encode_object_start( json, id );
			iot_json_encode_string( json, "command", "log.publish" );
			iot_json_encode_object_start( json, "params" );
			iot_json_encode_string( json, "thingKey",
				data->thing_key );
			iot_json_encode_string( json, "msg",
				message );

			/* publish optional arguments */
			tr50_optional( json, "ts", options, "time_stamp",
				IOT_TYPE_NULL );
			tr50_optional( json, "global", options, "global",
				IOT_TYPE_BOOL );

			if ( iot_options_get_integer( options,
				"level", IOT_TRUE, &level ) == IOT_STATUS_SUCCESS )
			{
				if ( level <= IOT_LOG_ERROR )
					level = 0;
				else if ( level < IOT_LOG_INFO )
					level = 1;
				else
					level = 2; /* default */
				iot_json_encode_integer( json, "level", level );
			}

			iot_json_encode_object_end( json );
			iot_json_encode_object_end( json );

			msg = iot_json_encode_dump( json );
			result = tr50_mqtt_publish(
				data, "api", msg, os_strlen( msg ), txn );
			iot_json_encode_terminate( json );
		}
	}
	return result;
}

iot_status_t tr50_execute(
	iot_t *lib,
	void* plugin_data,
	iot_operation_t op,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_step_t *step,
	const void *item,
	const void *value,
	const iot_options_t *options )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	struct tr50_data *const data = plugin_data;
	if ( op != IOT_OPERATION_ITERATION )
		IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s %d.%d",
			"execute", (int)op, (int)*step );
	else
		tr50_connect_check( lib, data, txn, max_time_out );

	if ( txn )
		tr50_transaction_status_set( data, (iot_uint8_t)(*txn),
			TR50_TRANSACTION_INVOKED );
	if ( *step == IOT_STEP_DURING )
	{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif /* ifdef __clang__ */
		switch( op )
		{
			case IOT_OPERATION_CLIENT_CONNECT:
				result = tr50_connect( lib, data, txn,
					max_time_out, IOT_FALSE );
				break;
			case IOT_OPERATION_CLIENT_DISCONNECT:
				result = tr50_disconnect( lib, data );
				break;
			case IOT_OPERATION_FILE_DOWNLOAD:
			case IOT_OPERATION_FILE_UPLOAD:
				result = tr50_file_request_send( data, op,
					(const iot_file_transfer_t*)item,
					txn, options );
				break;
			case IOT_OPERATION_TELEMETRY_PUBLISH:
				result = tr50_telemetry_publish( data,
					(const iot_telemetry_t*)item,
					(const struct iot_data*)value,
					txn, options );
				break;
			case IOT_OPERATION_ITERATION:
				if ( data )
					iot_mqtt_loop( data->mqtt, max_time_out );
				tr50_ping( lib, data, txn, max_time_out );
				tr50_file_queue_check( data );
				break;
			case IOT_OPERATION_ACTION_CHECK:
				result = tr50_check_mailbox( data, NULL );
				break;
			case IOT_OPERATION_ACTION_COMPLETE:
				result = tr50_action_complete( data,
					(const iot_action_t*)item,
					(const iot_action_request_t*)value,
					txn, options );
				break;
			case IOT_OPERATION_ALARM_PUBLISH:
				result = tr50_alarm_publish( data,
					(const iot_alarm_t*)item,
					(const iot_alarm_data_t*)value,
					txn, options );
				break;
			case IOT_OPERATION_ATTRIBUTE_PUBLISH:
				result = tr50_attribute_publish( data,
					(const char *)item,
					(const char *)value,
					txn, options );
				break;
			case IOT_OPERATION_EVENT_PUBLISH:
				result = tr50_event_publish( data,
					(const char *)value, txn, options );
				break;
			case IOT_OPERATION_TRANSACTION_STATUS:
				result = tr50_transaction_status( data,
					(const iot_transaction_t*)item,
					options );
				break;
			default:
				/* unhandled operations */
				break;
		}
#ifdef __clang__
#pragma clang diagnostic pop
#endif /* ifdef __clang__ */
	}
	return result;
}

iot_status_t tr50_file_request_send(
	struct tr50_data *data,
	iot_operation_t op,
	const iot_file_transfer_t *file_transfer,
	const iot_transaction_t *UNUSED(txn),
	const iot_options_t *options )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data && file_transfer )
	{
		result = IOT_STATUS_FULL;
		if ( data->file_transfer_count < TR50_FILE_TRANSFER_MAX )
		{
			char buf[ 512u ];
			const char *msg;
			struct tr50_file_transfer transfer;

			iot_json_encoder_t *json =
				iot_json_encode_initialize( buf, sizeof( buf ), 0u);

			os_memzero( &transfer, sizeof( struct tr50_file_transfer ) );
			os_strncpy( transfer.name, file_transfer->name, PATH_MAX );
			os_strncpy( transfer.path, file_transfer->path, PATH_MAX );
			transfer.callback = file_transfer->callback;
			transfer.user_data = file_transfer->user_data;
			transfer.op = op;
			transfer.use_global_store = IOT_FALSE;
			iot_options_get_bool( options, "global", IOT_FALSE,
				&transfer.use_global_store );

			result = IOT_STATUS_FAILURE;
			if ( json )
			{
				char id[11u];
				char global_name[PATH_MAX];

				/* create json string request for file.get/file.put */
				os_snprintf( id, sizeof(id), "%u",
					(data->file_transfer_count +
					TR50_FILE_REQUEST_ID_OFFSET) );

				iot_json_encode_object_start( json, id );
				iot_json_encode_string( json, "command",
					(transfer.op == IOT_OPERATION_FILE_UPLOAD)?
						"file.put" : "file.get" );

				iot_json_encode_object_start( json, "params" );

				/* Use the global file store if true */

				iot_json_encode_bool( json, "global",
					transfer.use_global_store);

				/* prepend a thing key if this is
				 * global, but strip any path information in the file.  It is not
				 * valid to upload a file with a path name */
				if ( transfer.op == IOT_OPERATION_FILE_UPLOAD &&
					transfer.use_global_store == IOT_TRUE )
				{
					os_snprintf( global_name, PATH_MAX,
						"%s_%s", data->thing_key, transfer.name);
					iot_json_encode_string( json, "fileName", global_name );
				}
				else
					iot_json_encode_string( json, "fileName", transfer.name );

				iot_json_encode_string( json, "thingKey", data->thing_key );

				if ( transfer.op == IOT_OPERATION_FILE_UPLOAD )
					iot_json_encode_bool( json, "public", IOT_FALSE );

				iot_json_encode_object_end( json );
				iot_json_encode_object_end( json );

				msg = iot_json_encode_dump( json );

				/* publish */
				result = iot_mqtt_publish( data->mqtt, "api",
					msg, os_strlen( msg ), TR50_MQTT_QOS,
					IOT_FALSE, NULL );
				if ( result == IOT_STATUS_SUCCESS )
				{
					/* add it to the queue */
					os_memcpy(
						&data->file_transfer_queue[ data->file_transfer_count ],
						&transfer, sizeof( struct tr50_file_transfer ) );
					data->file_transfer_queue[ data->file_transfer_count ].plugin_data =
						(void*)data;
					++data->file_transfer_count;
				}
				else
					IOT_LOG( data->lib, IOT_LOG_ERROR, "%s",
						"Failed send file request" );

				iot_json_encode_terminate( json );
			}
			else
				IOT_LOG( data->lib, IOT_LOG_ERROR, "%s",
					"Failed to encode json" );
		}
		else
			IOT_LOG( data->lib, IOT_LOG_ERROR, "%s",
				"Maximum file transfer reached" );
	}
	return result;
}

#ifdef IOT_THREAD_SUPPORT
OS_THREAD_DECL tr50_file_transfer(
	void* arg )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	iot_bool_t remove_from_queue = IOT_FALSE;
	struct tr50_file_transfer *const transfer =
		(struct tr50_file_transfer*)arg;
	if ( transfer )
	{
		const struct tr50_data *const data =
			(const struct tr50_data * )transfer->plugin_data;
		char file_path[ PATH_MAX +1u ];
		transfer->lib_curl = curl_easy_init();
		if ( transfer->lib_curl && data )
		{
			const char *ca_bundle_file = NULL;
			CURLcode curl_result = CURLE_FAILED_INIT;
			os_file_t file_handle;
			iot_bool_t validate_cert = IOT_FALSE;
			int retry = 0;
			iot_bool_t append_mode = IOT_FALSE;

			if ( transfer->op == IOT_OPERATION_FILE_UPLOAD )
				os_strncpy( file_path, transfer->path, PATH_MAX );
			else
				os_snprintf( file_path, PATH_MAX, "%s%s",
					transfer->path, TR50_DOWNLOAD_EXTENSION );

			if ( os_file_exists( file_path ) &&  transfer->op == IOT_OPERATION_FILE_DOWNLOAD)
				append_mode = IOT_TRUE;

			file_handle = os_file_open( file_path,
				(transfer->op == IOT_OPERATION_FILE_UPLOAD)? OS_READ : OS_READ_WRITE |
				( (append_mode == IOT_TRUE)? OS_APPEND: OS_CREATE) );

			if ( file_handle )
			{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif /* ifdef __clang__ */
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_URL, transfer->url );
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_VERBOSE, 1L );
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_NOSIGNAL, 1L );
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_FAILONERROR, 1L);
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_ACCEPT_ENCODING, "" );
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_NOPROGRESS, 0L );
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_PROGRESSFUNCTION,
					tr50_file_progress_old );
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_PROGRESSDATA, transfer );
#if LIBCURL_VERSION_NUM >= 0x072000
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_XFERINFOFUNCTION,
					tr50_file_progress );
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_XFERINFODATA, transfer );
#endif /* LIBCURL_VERSION_NUM >= 0x072000 */
				iot_config_get( data->lib,
					"ca_bundle_file", IOT_FALSE,
					IOT_TYPE_STRING, &ca_bundle_file );
				iot_config_get( data->lib,
					"validate_cloud_cert", IOT_FALSE,
					IOT_TYPE_BOOL, &validate_cert );
				if ( !ca_bundle_file )
					ca_bundle_file = IOT_DEFAULT_CERT_PATH;
				curl_easy_setopt( transfer->lib_curl,
					CURLOPT_CAINFO, ca_bundle_file );

				/* SSL verification */
				if ( validate_cert != IOT_FALSE )
				{
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_SSL_VERIFYHOST,
						TR50_DEFAULT_SSL_VERIFY_HOST );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_SSL_VERIFYPEER,
						TR50_DEFAULT_SSL_VERIFY_PEER );

					/* In some OSs libcurl cannot access the default CAs
					 * and it has to be added in the fs */

				}
				else
				{
					curl_easy_setopt(transfer->lib_curl, CURLOPT_SSL_VERIFYHOST, 0L);
					curl_easy_setopt(transfer->lib_curl, CURLOPT_SSL_VERIFYPEER, 0L);
				}

				/* Proxy settings */
				if ( data->proxy.type != IOT_PROXY_UNKNOWN &&
				     data->proxy.host && *data->proxy.host != '\0' )
				{
					long proxy_type = CURLPROXY_HTTP;
					if ( data->proxy.type == IOT_PROXY_SOCKS5 )
						proxy_type = CURLPROXY_SOCKS5_HOSTNAME;

					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_PROXY, data->proxy.host );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_PROXYPORT, data->proxy.port );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_PROXYTYPE, proxy_type );

					if ( data->proxy.username && data->proxy.username[0] != '\0' )
						curl_easy_setopt( transfer->lib_curl,
							CURLOPT_PROXYUSERNAME,
							data->proxy.username );
					if ( data->proxy.password && data->proxy.password[0] != '\0' )
						curl_easy_setopt( transfer->lib_curl,
							CURLOPT_PROXYPASSWORD,
							data->proxy.password );
				}

				if ( transfer->op == IOT_OPERATION_FILE_UPLOAD )
				{
					transfer->size =
						os_file_size( transfer->path );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_POST, 1L );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_READDATA, file_handle );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_READFUNCTION, os_file_read );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_POSTFIELDSIZE,
						transfer->size );
				}
				else
				{

					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_WRITEFUNCTION, os_file_write );
					curl_easy_setopt( transfer->lib_curl,
						CURLOPT_WRITEDATA, file_handle );
				}
				IOT_LOG( data->lib, IOT_LOG_DEBUG, "Maximum number of retries: %ld",
					transfer->max_retries);

				/* max_retries defined in iot_defs.h */
				for ( retry = 0; (retry <= transfer->max_retries ||
					transfer->max_retries< 0 ) && curl_result != CURLE_OK; retry++)
				{
					IOT_LOG( data->lib, IOT_LOG_TRACE, "retry count=%d", retry);
					if ( os_file_exists( file_path ) )
					{
						long resume_from = 0;

						/* Force a timeout when speed is less than the low speed limit
						 * for certain period of time so libcurl will stop trying for nothing
						 * and wait until network gets better */
						curl_easy_setopt( transfer->lib_curl,
							CURLOPT_LOW_SPEED_LIMIT, IOT_TRANSFER_LOW_SPEED_LIMIT );     /* bytes/second */
						curl_easy_setopt( transfer->lib_curl,
							CURLOPT_LOW_SPEED_TIME, IOT_TRANSFER_LOW_SPEED_TIMEOUT );      /* low speed timeout */

						/* set resume from amount for download */
						if ( transfer->op == IOT_OPERATION_FILE_DOWNLOAD)
						{
							resume_from = (long)os_file_size_handle( file_handle );
							IOT_LOG( data->lib, IOT_LOG_DEBUG,
								"File exists %s, resume xfer from %ld bytes",
								file_path, resume_from);
							curl_easy_setopt( transfer->lib_curl,
								CURLOPT_RESUME_FROM, resume_from );
						}
						else
						{
							curl_easy_setopt( transfer->lib_curl,
								CURLOPT_APPEND, 1L);
						}
						curl_easy_setopt( transfer->lib_curl,
							CURLOPT_FRESH_CONNECT, 1L);                 /* fresh connect ctx */
						curl_easy_setopt( transfer->lib_curl,
								CURLOPT_DNS_CACHE_TIMEOUT, 0L);     /* no dns cache */
					}
					curl_result = curl_easy_perform( transfer->lib_curl );

					/* need to handle errors 400 * without retrying */
					if ( curl_result == CURLE_HTTP_RETURNED_ERROR ||
					     curl_result == CURLE_SSL_CACERT )
					{
						IOT_LOG( data->lib, IOT_LOG_ERROR,
							"File transfer not recoverable(%d) exiting.\nReason: %s",
							curl_result, curl_easy_strerror( curl_result ) );
						break;
					}
					else
						IOT_LOG( data->lib, IOT_LOG_TRACE, "curl result %d", curl_result);

					/* add a delay before immediately trying again */
					if ( curl_result != CURLE_OK )
						os_time_sleep(10000, IOT_FALSE);
				}

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* ifdef __clang__ */

				if ( curl_result == CURLE_OK )
					result = IOT_STATUS_SUCCESS;
				else
					IOT_LOG( data->lib, IOT_LOG_ERROR,
						"File transfer failed: %s",
						curl_easy_strerror( curl_result ) );

				os_file_close( file_handle );
			}
			else
				IOT_LOG( data->lib, IOT_LOG_ERROR,
					"Failed to open %s", transfer->path );

			curl_easy_cleanup( transfer->lib_curl );
		}
		else
			IOT_LOG( data->lib, IOT_LOG_ERROR, "%s",
				"Failed to initialize libcurl" );


		/* final checks and cleanup */
		if ( result == IOT_STATUS_SUCCESS )
		{
			if ( transfer->op == IOT_OPERATION_FILE_DOWNLOAD )
			{
				os_file_t file_handle = os_file_open(
					file_path, OS_READ );
				if ( file_handle )
				{
					iot_uint64_t crc32 = 0u;

					result = iot_checksum_file_get(
						data->lib, file_handle,
						IOT_CHECKSUM_TYPE_CRC32, &crc32 );
					if ( result == IOT_STATUS_SUCCESS &&
						crc32 != transfer->crc32 )
					{
						IOT_LOG( data->lib, IOT_LOG_ERROR,
							"Checksum for %s does not match. "
							"Expected: 0x%lX, calculated: 0x%lX",
							transfer->path, transfer->crc32, crc32);
						os_file_delete( file_path );
						result = IOT_STATUS_FAILURE;
					}
					os_file_close( file_handle );

					if ( result == IOT_STATUS_SUCCESS )
						os_file_move( file_path, transfer->path );
				}
			}
			else
			{
				if ( os_strlen( transfer->path ) > 4u  &&
					os_strncmp(
						transfer->path +
						os_strlen( transfer->path ) - 4u,
						".tar", 4u ) == 0 )
					os_file_delete( transfer->path );
			}

			remove_from_queue = IOT_TRUE;
		}
		else
			remove_from_queue = IOT_TRUE;

		if ( transfer->callback )
		{
			iot_file_progress_t transfer_progress;
			os_memzero( &transfer_progress, sizeof(transfer_progress) );
			transfer_progress.percentage = (result == IOT_STATUS_SUCCESS)?
				100.0 : (iot_float32_t)(100.0 * transfer->prev_byte / transfer->size);
			transfer_progress.status = result;
			transfer_progress.completed = IOT_TRUE;

			transfer->callback( &transfer_progress, transfer->user_data );
		}

		if ( remove_from_queue )
		{
			struct tr50_data *const tr50 =
				(struct tr50_data *)transfer->plugin_data;

			if ( tr50 )
			{
				/* remove the transfer from the queue */
				iot_uint8_t i = (iot_uint8_t)(
					(unsigned long)((const unsigned char *)(transfer) - (const unsigned char*)&tr50->file_transfer_queue[0]) / (unsigned long)sizeof(struct tr50_file_transfer));
				os_memmove(
					&tr50->file_transfer_queue[i],
					&tr50->file_transfer_queue[i+1u],
					( tr50->file_transfer_count - i -1u ) *
					sizeof( iot_file_transfer_t ) );
				os_memzero(
					&tr50->file_transfer_queue[ tr50->file_transfer_count - 1u ],
					sizeof( iot_file_transfer_t ) );
				--tr50->file_transfer_count;
				result = IOT_STATUS_SUCCESS;
			}
			else
			{
				IOT_LOG( data->lib, IOT_LOG_ERROR, "%s",
					"Cannot find plugin data" );
				result = IOT_STATUS_FAILURE;
			}
		}
	}
	return (OS_THREAD_RETURN)result;
}

int tr50_file_progress( void *user_data,
	curl_off_t UNUSED(down_total), curl_off_t down_now,
	curl_off_t up_total, curl_off_t up_now )
{
	int result = 0;
	struct tr50_file_transfer *const transfer =
		(struct tr50_file_transfer*)user_data;

	if ( transfer )
	{
		if ( transfer->cancel )
			result = 1;
		else
		{
			/* Get the total transfer time and compare it to the last time the
			 * progress is updated. This is done to minimize printing in the logs.
			 * The end of the transfer will still be printed to show some
			 * progress for small files. */
			double cur_time = 0, int_time = 0;
			long now = 0, total = 0;
			const char *transfer_type = NULL;

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif /* ifdef __clang__ */
			curl_easy_getinfo(
				transfer->lib_curl, CURLINFO_TOTAL_TIME, &cur_time);
#ifdef __clang__
#pragma clang diagnostic pop
#endif /* ifdef __clang__ */
			int_time = cur_time - transfer->last_update_time;

			if ( transfer->op == IOT_OPERATION_FILE_UPLOAD )
			{
				now = (long)up_now + transfer->prev_byte;
				total = (long)up_total + transfer->prev_byte;
				transfer_type = "Upload";
			}
			else
			{
				/* For larger files, cloud does not specify total
				 * size, so use the file size given in the beginning */
				now = (long)down_now + transfer->prev_byte;
				total = (long)transfer->size;
				transfer_type = "Download";
			}

			if ( total > 0 && (now == total ||
				int_time > TR50_FILE_TRANSFER_PROGRESS_INTERVAL) )
			{
				double progress = (double)( 100.0 * now / total );
				transfer->last_update_time = cur_time;

				if ( transfer->callback )
				{
					iot_file_progress_t transfer_progress;
					os_memzero( &transfer_progress, sizeof(transfer_progress) );
					transfer_progress.percentage = (iot_float32_t)progress;
					transfer_progress.status = IOT_STATUS_INVOKED;
					transfer_progress.completed = IOT_FALSE;

					transfer->callback( &transfer_progress, transfer->user_data );
				}
				else
				{
					struct tr50_data *data =
						transfer->plugin_data;
					iot_t *lib = NULL;
					if ( data )
						lib = data->lib;
					IOT_LOG( lib, IOT_LOG_TRACE,
						"%sing %s: %.1f%% (%ld/%ld bytes)\n",
						transfer_type, transfer->path,
						progress, now, total );
				}
			}
		}
	}
	return result;
}

int tr50_file_progress_old(
	void *user_data,
	double down_total, double down_now,
	double up_total, double up_now )
{
	return tr50_file_progress( user_data,
		(curl_off_t)down_total, (curl_off_t)down_now,
		(curl_off_t)up_total, (curl_off_t)up_now );
}
#endif /* ifdef IOT_THREAD_SUPPORT */

void tr50_file_queue_check(
	struct tr50_data *data )
{
	if ( data )
	{
		iot_timestamp_t now = iot_timestamp_now();
		if ( data->file_queue_last_checked == 0u ||
			now - data->file_queue_last_checked >=
			TR50_FILE_QUEUE_CHECK_INTERVAL )
		{
#ifdef IOT_THREAD_SUPPORT
			iot_uint8_t i = 0u;
			for ( i= 0u;
				i < data->file_transfer_count;
				i++ )
			{
				struct tr50_file_transfer *transfer =
					&data->file_transfer_queue[i];
				if ( transfer &&
					transfer->retry_time != 0u &&
					transfer->retry_time <= now )
				{
					os_thread_t thread;
					size_t stack_size = 0u;

					/* Create a thread to do the file transfer */
#if defined( __VXWORKS__ )
					stack_size = deviceCloudStackSizeGet();
#endif /* if defined( __VXWORKS__ ) */

					if ( os_thread_create( &thread,
						tr50_file_transfer,
						transfer, stack_size ) == 0 )
						transfer->retry_time = 0u;
				}
			}
#endif /* ifdef IOT_THREAD_SUPPORT */
			data->file_queue_last_checked = now;
		}
	}
}

iot_status_t tr50_initialize(
	iot_t *lib,
	void **plugin_data )
{
	iot_status_t result = IOT_STATUS_NO_MEMORY;
	struct tr50_data *const data = os_malloc( sizeof( struct tr50_data ) );
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "initialize" );
	if ( data )
	{
		os_memzero( data, sizeof( struct tr50_data ) );
		data->lib = lib;
		*plugin_data = data;
#ifdef IOT_THREAD_SUPPORT
		os_thread_mutex_create( &data->mail_check_mutex ) ;
#endif /* IOT_THREAD_SUPPORT */
		curl_global_init( CURL_GLOBAL_ALL );
		result = iot_mqtt_initialize();
	}
	return result;
}

iot_status_t tr50_mqtt_publish(
	struct tr50_data *data,
	const char *topic,
	const void *payload,
	size_t payload_len,
	const iot_transaction_t *txn )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( data && topic && payload )
	{
		IOT_LOG( data->lib, IOT_LOG_DEBUG,
			"tr50: sent (%u bytes on %s): %.*s",
				(unsigned int)payload_len, topic,
				(int)payload_len, (const char*)payload );
		result = iot_mqtt_publish( data->mqtt, topic,
			payload, payload_len, TR50_MQTT_QOS, IOT_FALSE, NULL );
		if ( result != IOT_STATUS_SUCCESS && txn )
			tr50_transaction_status_set( data, (iot_uint8_t)(*txn),
				TR50_TRANSACTION_FAILURE );
	}
	return result;
}

void tr50_on_message(
	void *user_data,
	const char *topic,
	void *payload,
	size_t payload_len,
	int UNUSED(qos),
	iot_bool_t UNUSED(retain) )
{
#ifdef IOT_STACK_ONLY
	char buf[TR50_IN_BUFFER_SIZE];
#endif
	struct tr50_data *const data = (struct tr50_data *)(user_data);
	iot_json_decoder_t *json;
	const iot_json_item_t *root;

	if ( data )
	{
		IOT_LOG( data->lib, IOT_LOG_DEBUG,
			"tr50: received (%u bytes on %s): %.*s",
			(unsigned int)payload_len, topic,
			(int)payload_len, (const char *)payload );
		data->time_last_msg_received = iot_timestamp_now();
	}

#ifdef IOT_STACK_ONLY
	json = iot_json_decode_initialize( buf, TR50_IN_BUFFER_SIZE, 0u );
#else
	json = iot_json_decode_initialize( NULL, 0u, IOT_JSON_FLAG_DYNAMIC );
#endif
	if ( data && json &&
		iot_json_decode_parse( json, payload, payload_len, &root,
			NULL, 0u ) == IOT_STATUS_SUCCESS )
	{

		if ( os_strcmp( topic, "notify/mailbox_activity" ) == 0 )
		{
			iot_json_type_t type;
			const iot_json_item_t *const j_thing_key =
				iot_json_decode_object_find( json, root,
					"thingKey" );
			type = iot_json_decode_type( json, j_thing_key );

			if ( type == IOT_JSON_TYPE_STRING )
			{
				const char *v = NULL;
				size_t v_len = 0u;
				iot_json_decode_string( json, j_thing_key,
					&v, &v_len );

				/* check if message is for us */
				if ( os_strncmp( v, data->thing_key, v_len ) == 0 )
					tr50_check_mailbox( data, NULL );
			}
		}
		else if ( os_strcmp( topic, "reply" ) == 0 )
		{
			const iot_json_object_iterator_t *root_iter =
				iot_json_decode_object_iterator( json, root );
			if ( root_iter )
			{
				char name[ IOT_NAME_MAX_LEN + 1u ];
				const char *v = NULL;
				size_t v_len = 0u;
				const iot_json_item_t *j_obj = NULL;
				int msg_id = 0;

				iot_json_decode_object_iterator_key(
					json, root, root_iter,
					&v, &v_len );
				os_snprintf( name, IOT_NAME_MAX_LEN, "%.*s", (int)v_len, v );
				msg_id = os_atoi( name );
				iot_json_decode_object_iterator_value(
					json, root, root_iter, &j_obj );

				/* clear pending mailbox check */
				if ( os_strncmp( name, "check", 5 ) == 0 )
					data->time_last_mailbox_check = 0;

				if ( os_strncmp( name, "ping", 4 ) == 0 &&
					data->ping_miss_count > 0u )
					--data->ping_miss_count;
				else if ( j_obj )
				{
					const iot_json_item_t *j_success;
					iot_bool_t is_success;

					j_success = iot_json_decode_object_find( json,
						j_obj, "success" );
					if ( j_success )
					{
						enum tr50_transaction_status s = TR50_TRANSACTION_FAILURE;
						iot_json_decode_bool( json, j_success, &is_success );

						/* update transaction status */
						if ( msg_id > 0 && msg_id < 256 )
						{
							if ( is_success )
								s = TR50_TRANSACTION_SUCCESS;
							tr50_transaction_status_set(
								data, (iot_uint8_t)msg_id, s );
						}

						if ( is_success )
						{
							const iot_json_item_t *j_params;
							const iot_json_item_t *j_messages;
							j_params = iot_json_decode_object_find(
								json, j_obj, "params" );

							j_messages = iot_json_decode_object_find( json,
								j_params, "messages" );

							/* actions (aka methods) parsing */
							if ( j_messages && iot_json_decode_type( json, j_messages )
								== IOT_JSON_TYPE_ARRAY )
							{
								size_t i;
								const size_t msg_count =
									iot_json_decode_array_size( json, j_messages );

								for ( i = 0u; i < msg_count; ++i )
								{
									const iot_json_item_t *j_cmd_item;
									if ( iot_json_decode_array_at( json,
										j_messages, i, &j_cmd_item ) == IOT_STATUS_SUCCESS )
									{
										const iot_json_item_t *j_id;
										j_id = iot_json_decode_object_find(
											json, j_cmd_item, "id" );
										if ( !j_id )
											IOT_LOG( data->lib, IOT_LOG_WARNING,
												"\"%s\" not found!", "id" );

										j_params = iot_json_decode_object_find(
											json, j_cmd_item, "params" );
										if ( !j_params )
											IOT_LOG( data->lib, IOT_LOG_WARNING,
												"\"%s\" not found!", "params" );

										if ( j_id && j_params )
										{
											const iot_json_item_t *j_method;
											const iot_json_object_iterator_t *iter;
											iot_action_request_t *req = NULL;

											j_method = iot_json_decode_object_find(
												json, j_params, "method" );
											if ( j_method )
											{
												char id[ IOT_ID_MAX_LEN + 1u ];
												*id = '\0';

												iot_json_decode_string( json, j_id, &v, &v_len );
												os_snprintf( id, IOT_ID_MAX_LEN, "%.*s", (int)v_len, v );
												id[ IOT_ID_MAX_LEN ] = '\0';

												iot_json_decode_string( json, j_method, &v, &v_len );
												os_snprintf( name, IOT_NAME_MAX_LEN, "%.*s", (int)v_len, v );
												name[ IOT_NAME_MAX_LEN ] = '\0';
												req = iot_action_request_allocate( data->lib, name, "tr50" );

												/* check mailbox for more actions if queue is available */
												tr50_check_mailbox( data, NULL );
												if ( req )
													iot_action_request_option_set( req, "id", IOT_TYPE_STRING, id );
												else
												{
													/* send response that message can't be handled */
													const char *out_msg;
													char out_msg_id[6u];
													char out_msg_buf[ 512u ];
													iot_json_encoder_t *out_json;
													out_json = iot_json_encode_initialize( out_msg_buf, 512u, 0 );
													os_snprintf( out_msg_id, sizeof(out_msg_id), "cmd" );
													iot_json_encode_object_start( out_json, out_msg_id );
													iot_json_encode_string( out_json, "command", "mailbox.ack" );
													iot_json_encode_object_start( out_json, "params" );
													iot_json_encode_string( out_json, "id", id );
													iot_json_encode_integer( out_json, "errorCode", (int)IOT_STATUS_FULL );
													iot_json_encode_string( out_json, "errorMessage", "maximum inbound requests reached" );
													iot_json_encode_object_end( out_json );
													iot_json_encode_object_end( out_json );

													out_msg = iot_json_encode_dump( out_json );
													tr50_mqtt_publish(
														data, "api", out_msg,
														os_strlen( out_msg ), NULL );
													iot_json_encode_terminate( out_json );
												}
											}

											/* for each parameter */
											j_params = iot_json_decode_object_find(
												json, j_params, "params" );
											iter = iot_json_decode_object_iterator(
												json, j_params );
											while ( iter )
											{
												const iot_json_item_t *j_value = NULL;
												iot_json_decode_object_iterator_key(
													json, j_params, iter,
													&v, &v_len );
												iot_json_decode_object_iterator_value(
													json, j_params, iter,
													&j_value );
												os_snprintf( name, IOT_NAME_MAX_LEN, "%.*s", (int)v_len, v );
												name[ IOT_NAME_MAX_LEN ] = '\0';
												iter = iot_json_decode_object_iterator_next(
													json, j_params, iter );
												switch ( iot_json_decode_type( json,
													j_value ) )
												{
												case IOT_JSON_TYPE_BOOL:
													{
													iot_bool_t value;
													iot_json_decode_bool( json, j_value, &value );
													iot_action_request_parameter_set( req, name, IOT_TYPE_BOOL, value );
													}
													break;
												case IOT_JSON_TYPE_INTEGER:
													{
													iot_int64_t value;
													iot_json_decode_integer( json, j_value, &value );
													iot_action_request_parameter_set( req, name, IOT_TYPE_INT64, value );
													}
													break;
												case IOT_JSON_TYPE_REAL:
													{
													iot_float64_t value;
													iot_json_decode_real( json, j_value, &value );
													iot_action_request_parameter_set( req, name, IOT_TYPE_FLOAT64, value );
													}
													break;
												case IOT_JSON_TYPE_STRING:
													{
													char *value;
													iot_json_decode_string( json, j_value, &v, &v_len );
													value = os_malloc( v_len + 1u );
													if( value )
													{
														size_t j;
														char *p = value;
														for ( j = 0u; j < v_len; ++j )
														{
															if ( *v != '\\' || *(v+1) != '"' )
																*p++ = *v;
															++v;
														}
														*p = '\0';
														iot_action_request_parameter_set( req, name, IOT_TYPE_STRING, value );
														os_free( value );
													}
													}
												case IOT_JSON_TYPE_ARRAY:
												case IOT_JSON_TYPE_OBJECT:
												case IOT_JSON_TYPE_NULL:
												default:
													break;
												}
											}

											if ( req )
												iot_action_request_execute( req, 0u );
										}
									}
								}
							}
							else
							{
								j_obj = iot_json_decode_object_find( json,
									j_params, "fileId" );
								if ( j_obj && iot_json_decode_type( json, j_obj )
									== IOT_JSON_TYPE_STRING )
								{
									/* file transfer request parsing */
									iot_bool_t found_transfer = IOT_FALSE;
									struct tr50_file_transfer *transfer = NULL;
									iot_int64_t crc32 = 0u;
									iot_int64_t fileSize = 0u;

									/* obtain the fileId */
									iot_json_decode_string( json, j_obj, &v, &v_len );

									j_obj = iot_json_decode_object_find( json,
										j_params, "crc32" );
									if ( j_obj && iot_json_decode_type( json, j_obj )
										== IOT_JSON_TYPE_INTEGER )
										iot_json_decode_integer( json, j_obj, &crc32 );

									j_obj = iot_json_decode_object_find( json,
										j_params, "fileSize" );
									if ( j_obj && iot_json_decode_type( json, j_obj )
										== IOT_JSON_TYPE_INTEGER )
										iot_json_decode_integer( json, j_obj, &fileSize );

									if ( msg_id > 0 && (unsigned int)msg_id >= TR50_FILE_REQUEST_ID_OFFSET &&
										(unsigned int)msg_id - TR50_FILE_REQUEST_ID_OFFSET < TR50_FILE_TRANSFER_MAX )
									{
										transfer = &data->file_transfer_queue[(unsigned int)msg_id - TR50_FILE_REQUEST_ID_OFFSET];
										if ( transfer->path[0] )
										{
											/* determine host name from config file */
											const char *host = NULL;
											iot_config_get( data->lib,
												"cloud.host", IOT_FALSE,
												IOT_TYPE_STRING, &host );
											os_snprintf( transfer->url, PATH_MAX,
												"https://%s/file/%.*s", host, (int)v_len, v );
											transfer->crc32 = (iot_uint64_t)crc32;
											transfer->size = (iot_uint64_t)fileSize;
											transfer->retry_time = 0u;
											transfer->expiry_time =
												iot_timestamp_now() +
												TR50_FILE_TRANSFER_EXPIRY_TIME;
											transfer->max_retries =
												IOT_TRANSFER_MAX_RETRIES;
											found_transfer = IOT_TRUE;
										}
									}

									if ( found_transfer )
									{
#if defined( IOT_THREAD_SUPPORT )
										os_thread_t thread;
										size_t stack_size = 0u;

#if defined( __VXWORKS__ )
										stack_size = deviceCloudStackSizeGet();
#endif /* if defined( __VXWORKS__ ) */

										/* Create a thread to do the file transfer */
										if ( os_thread_create( &thread, tr50_file_transfer, transfer, stack_size ) )
											IOT_LOG( data->lib, IOT_LOG_ERROR,
												"Failed to create a thread to transfer "
												"file for message #%u", (unsigned int)msg_id );
#endif /* if defined( IOT_THREAD_SUPPORT ) */
									}
								}
							}
						}
					}
				}
			}
		}
		else
			IOT_LOG( data->lib, IOT_LOG_TRACE, "tr50: %s",
				"message received on unknown topic" );
		iot_json_decode_terminate( json );
	}
	else if ( data )
		IOT_LOG( data->lib, IOT_LOG_ERROR, "tr50: %s",
			"failed to parse incoming message" );
}

void tr50_optional(
	iot_json_encoder_t *json,
	const char *json_key,
	const iot_options_t *options,
	const char *options_key,
	iot_type_t type )
{
	if ( json && options && options_key && *options_key != '\0' )
	{
		struct iot_data data;
		switch( type )
		{
		case IOT_TYPE_BOOL:
		{
			data.value.boolean = IOT_FALSE;
			if ( iot_options_get_bool( options, options_key, IOT_FALSE,
				&data.value.boolean ) == IOT_STATUS_SUCCESS )
			{
				iot_json_encode_bool( json, json_key,
					data.value.boolean );
			}
			break;
		}
		case IOT_TYPE_FLOAT32:
		case IOT_TYPE_FLOAT64:
		{
			data.value.float64 = 0.0;
			if ( iot_options_get_real( options, options_key, IOT_FALSE,
				&data.value.float64 ) == IOT_STATUS_SUCCESS )
			{
				iot_json_encode_real( json, json_key,
					data.value.float64 );
			}
			break;
		}
		case IOT_TYPE_INT8:
		case IOT_TYPE_INT16:
		case IOT_TYPE_INT32:
		case IOT_TYPE_INT64:
		case IOT_TYPE_UINT8:
		case IOT_TYPE_UINT16:
		case IOT_TYPE_UINT32:
		case IOT_TYPE_UINT64:
		{
			data.value.int64 = 0;
			if ( iot_options_get_integer( options, options_key, IOT_FALSE,
				&data.value.int64 ) == IOT_STATUS_SUCCESS )
			{
				iot_json_encode_integer( json, json_key,
					data.value.int64 );
			}
			break;
		}
		case IOT_TYPE_LOCATION:
		{
			data.value.location = NULL;
			if ( iot_options_get_location( options, options_key,
				IOT_FALSE, &data.value.location ) == IOT_STATUS_SUCCESS &&
				data.value.location )
			{
				if ( json_key )
					iot_json_encode_object_start( json, json_key );
				iot_json_encode_real( json, "lat",
					data.value.location->latitude );
				iot_json_encode_real( json, "lng",
					data.value.location->longitude );
				if ( json_key )
					iot_json_encode_object_end( json );
			}
			break;
		}
		case IOT_TYPE_RAW:
			data.value.raw.ptr = NULL;
			if ( iot_options_get_raw( options, options_key,
				IOT_FALSE, &data.value.raw.length,
				&data.value.raw.ptr ) == IOT_STATUS_SUCCESS &&
				data.value.raw.ptr )
			{
				iot_json_encode_string( json, json_key,
					data.value.raw.ptr );
			}
			break;
		case IOT_TYPE_STRING:
			data.value.string = NULL;
			if ( iot_options_get_string( options, options_key,
				IOT_FALSE, &data.value.string ) == IOT_STATUS_SUCCESS &&
				data.value.string )
			{
				iot_json_encode_string( json, json_key,
					data.value.string );
			}
			break;
		case IOT_TYPE_NULL: /* special case: time stamp */
		{
			if ( iot_options_get_integer( options, options_key,
				IOT_FALSE, &data.value.int64 ) == IOT_STATUS_SUCCESS )
			{
				char ts_str[32u];
				tr50_strtime(
					(iot_timestamp_t)data.value.int64,
					ts_str, 25u );
				iot_json_encode_string( json, json_key, ts_str );
			}
			break;
		}
		}
	}
}

void tr50_ping(
	iot_t *lib,
	struct tr50_data *data,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out )
{
	if ( data )
	{
		iot_timestamp_t now = iot_timestamp_now();
		if ( data->time_last_msg_received > 0u &&
			now - data->time_last_msg_received >= TR50_PING_INTERVAL )
		{
			if ( data->ping_miss_count > TR50_PING_MISS_ALLOWED )
			{
				/* try to reconnect */
				tr50_connect( lib, data, txn,
					max_time_out, IOT_TRUE );
				data->ping_miss_count = 0u;
			}
			else
			{
				/* encode ping message */
				const char *out_msg;
				char out_msg_id[6u];
				char out_msg_buf[ 512u ];
				iot_json_encoder_t *out_json;
				out_json = iot_json_encode_initialize( out_msg_buf, 512u, 0 );
				os_snprintf( out_msg_id, sizeof(out_msg_id), "ping" );
				iot_json_encode_object_start( out_json, out_msg_id );
				iot_json_encode_string( out_json, "command", "diag.ping" );
				iot_json_encode_object_end( out_json );

				out_msg = iot_json_encode_dump( out_json );
				tr50_mqtt_publish(
					data, "api", out_msg,
					os_strlen( out_msg ), NULL );
				iot_json_encode_terminate( out_json );

				/* update receive time, so another ping isn't sent */
				data->time_last_msg_received = now;
				++data->ping_miss_count;
			}
		}
	}
}

char *tr50_strtime( iot_timestamp_t ts,
	char *out, size_t len )
{
	size_t out_len;

	/* TR50 format: "YYYY-MM-DDTHH:MM:SS.mmZ" */
	out_len = os_time_format( out, len,
		"%Y-%m-%dT%H:%M:%S", ts, OS_FALSE );
	if ( out_len > 0u )
	{
		/* add milliseconds, if there are any remaining */
		ts %= 1000u;
		if ( ts > 0u )
		{
			size_t out_req;
			out_req = (size_t)os_snprintf( &out[out_len],
				len - out_len, ".%03u", (unsigned int)ts );

			if ( out_req < len - out_len )
				out_len += out_req;
			else
				out_len = 0u;
		}

		/* add Zulu time indicator */
		if ( out_len > 0u && out_len < len - 1u )
			out[out_len++] = 'Z';
		else /* buffer too small */
			out_len = 0u;
	}

	/* ensure null-terminated */
	if ( out && len )
		out[out_len] = '\0';
	return out;
}

iot_status_t tr50_telemetry_publish(
	struct tr50_data *data,
	const iot_telemetry_t *t,
	const struct iot_data *d,
	const iot_transaction_t *txn,
	const iot_options_t *UNUSED(options) )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	if ( d->has_value )
	{
		const char *cmd;
		char id[11u];
		const char *msg;
		const char *const value_key = "value";
		iot_json_encoder_t *const json =
			iot_json_encode_initialize( NULL, 0u, 0u);

		if ( d->type == IOT_TYPE_LOCATION )
			cmd = "location.publish";
		else if ( d->type == IOT_TYPE_STRING ||
			d->type == IOT_TYPE_RAW )
			cmd = "attribute.publish";
		else
			cmd = "property.publish";

		/* convert id to string */
		if ( txn )
			os_snprintf( id, sizeof(id), "%u", (unsigned int)(*txn) );
		else
			os_snprintf( id, sizeof(id), "cmd" );
		iot_json_encode_object_start( json, id );
		iot_json_encode_string( json, "command", cmd );
		iot_json_encode_object_start( json, "params" );
		iot_json_encode_string( json, "thingKey",
			data->thing_key );
		iot_json_encode_string( json, "key",
			iot_telemetry_name_get( t ) );
		switch ( d->type )
		{
		case IOT_TYPE_BOOL:
			iot_json_encode_real( json, value_key,
				(double)d->value.boolean );
			break;
		case IOT_TYPE_FLOAT32:
			iot_json_encode_real( json, value_key,
				(double)d->value.float32 );
			break;
		case IOT_TYPE_FLOAT64:
			iot_json_encode_real( json, value_key,
				(double)d->value.float64 );
			break;
		case IOT_TYPE_INT8:
			iot_json_encode_real( json, value_key,
				(double)d->value.int8 );
			break;
		case IOT_TYPE_INT16:
			iot_json_encode_real( json, value_key,
				(double)d->value.int16 );
			break;
		case IOT_TYPE_INT32:
			iot_json_encode_real( json, value_key,
				(double)d->value.int32 );
			break;
		case IOT_TYPE_INT64:
			iot_json_encode_real( json, value_key,
				(double)d->value.int64 );
			break;
		case IOT_TYPE_UINT8:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint8 );
			break;
		case IOT_TYPE_UINT16:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint16 );
			break;
		case IOT_TYPE_UINT32:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint32 );
			break;
		case IOT_TYPE_UINT64:
			iot_json_encode_real( json, value_key,
				(double)d->value.uint64 );
			break;
		case IOT_TYPE_RAW:
			tr50_append_value_raw( json, value_key,
				d->value.raw.ptr, d->value.raw.length );
			break;
		case IOT_TYPE_STRING:
			tr50_append_value_raw( json, value_key,
				d->value.string, (size_t)-1 );
			break;
		case IOT_TYPE_LOCATION:
			tr50_append_location( json, NULL, d->value.location );
			break;
		case IOT_TYPE_NULL:
		default:
			break;
		}

		if ( t->time_stamp > 0u )
		{
			char ts_str[32u];
			tr50_strtime( t->time_stamp, ts_str, 25u );
			iot_json_encode_string( json, "ts", ts_str );
		}
		iot_json_encode_object_end( json );
		iot_json_encode_object_end( json );

		msg = iot_json_encode_dump( json );
		result = tr50_mqtt_publish(
			data, "api", msg, os_strlen( msg ), txn );
		iot_json_encode_terminate( json );
	}
	return result;
}

iot_status_t tr50_terminate(
	iot_t *lib,
	void *plugin_data )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	struct tr50_data *data = plugin_data;
	IOT_LOG( lib, IOT_LOG_TRACE, "tr50: %s", "terminate" );
#ifdef IOT_THREAD_SUPPORT
	os_thread_mutex_destroy( &data->mail_check_mutex );
#endif /* IOT_THREAD_SUPPORT */
	os_free_null( (void**)&data );
	iot_mqtt_terminate();
	curl_global_cleanup();
	return result;
}

iot_status_t tr50_transaction_status(
	const struct tr50_data *data,
	const iot_transaction_t *txn,
	const iot_options_t *UNUSED(options) )
{
	/* size of a "chunk" in bits */
	const unsigned int chunk_size_bits = sizeof(unsigned int)*8u;
	/* number of bits required to data for each item */
	const unsigned int data_bits = 2u;
	/* number of data blocks in a "chunk" */
	const unsigned int data_blks = chunk_size_bits/data_bits;
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( data && txn )
	{
		const iot_uint8_t txn_id = (iot_uint8_t)(*txn);
		const unsigned int b = (txn_id / data_blks);
		enum tr50_transaction_status tx_status;

		/* obtain the bits within a chunk that hold the desired data */
		tx_status = (data->transactions[b] >> (((txn_id%data_blks)*data_bits)) & ((1u<<data_bits)-1u));

		/* convert tr50 transaction status to result */
		switch (tx_status)
		{
		case TR50_TRANSACTION_SUCCESS:
			result = IOT_STATUS_SUCCESS;
			break;
		case TR50_TRANSACTION_INVOKED:
			result = IOT_STATUS_INVOKED;
			break;
		case TR50_TRANSACTION_FAILURE:
			result = IOT_STATUS_EXECUTION_ERROR;
			break;
		case TR50_TRANSACTION_UNKNOWN:
		default:
			result = IOT_STATUS_NOT_FOUND;
		}
	}
	return result;
}

void tr50_transaction_status_set(
	struct tr50_data *data,
	iot_uint8_t txn_id,
	enum tr50_transaction_status tx_status )
{
	/* size of a "chunk" in bits */
	const unsigned int chunk_size_bits = sizeof(iot_uint32_t)*8u;
	/* number of bits required to data for each item */
	const unsigned int data_bits = 2u;
	/* number of data blocks in a "chunk" */
	const unsigned int data_blks = chunk_size_bits/data_bits;

	if ( data )
	{
		const unsigned int b = (txn_id / data_blks);
		const iot_uint32_t mask = ((1u<<data_bits)-1u) << ((txn_id%data_blks)*data_bits);
		const iot_uint32_t value = tx_status << ((txn_id%data_blks)*data_bits);

		if ( b < sizeof(data->transactions) / sizeof(iot_uint32_t) )
			data->transactions[b] = (data->transactions[b] & ~mask) | (value & mask);
	}
}

IOT_PLUGIN( tr50, 10, iot_version_encode(1,0,0,0),
	iot_version_encode(2,3,0,0), 0 )

