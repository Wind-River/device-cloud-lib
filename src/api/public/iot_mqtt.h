/**
 * @file
 * @brief header file declaring common mqtt function implementations
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
#ifndef IOT_MQTT_H
#define IOT_MQTT_H

#include <iot.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @brief Possible types for iot proxy
 */
typedef enum iot_proxy_type
{
	/** @brief UNKNOWN proxy type */
	IOT_PROXY_UNKNOWN,
	/** @brief SOCKS5 proxy type */
	IOT_PROXY_SOCKS5,
	/** @brief HTTP proxy type */
	IOT_PROXY_HTTP,
} iot_proxy_type_t;

/** @brief Structure containing information about proxy server used */
typedef struct iot_proxy
{
	/** @brief Proxy to use */
	const char *host;
	/** @brief Port number the proxy server listens to */
	const iot_int64_t port;
	/** @brief Proxy protocol type to use */
	iot_proxy_type_t type;
	/** @brief User name to use for proxy authentication */
	const char *username;
	/** @brief Password to use with proxy authentication */
	const char *password;
} iot_mqtt_proxy_t;

/**
 * @brief Structure for passing in mqtt SSL configuration information
 */
typedef struct iot_mqtt_ssl
{
	/** @brief client certificate file path */
	const char *cert_file;
	/** @brief client key file path */
	const char *key_file;
	/** @brief client certificate authority file or directory path */
	const char *ca_path;
	/** @brief if true, allow connections to privately signed certificates */
	iot_bool_t insecure;
} iot_mqtt_ssl_t;

/**
 * @brief Structure containing options for connecting to an MQTT broker
 */
typedef struct iot_mqtt_connect_options
{
	/** @brief id of the client */
	const char *client_id;
	/** @brief host server to connect to */
	const char *host;
	/** @brief port to connect on (if 0, defaults: to 1883 or 8883) */
	iot_uint16_t port;
	/** @brief proxy information (optional) */
	iot_mqtt_proxy_t *proxy_conf;
	/** @brief secure connection information (optional) */
	iot_mqtt_ssl_t *ssl_conf;
	/**
	 * @brief user name to connect with (optional)
	 *
	 * @note This field is only applicable to clients that connect using
	 * MQTT version 3.1.1 protocol or higher.
	 */
	const char *username;
	/**
	 * @brief password to connect with (optional)
	 *
	 * @note This field is only applicable to clients that connect using
	 * MQTT version 3.1.1 protocol or higher.
	 */
	const char *password;
} iot_mqtt_connect_options_t;

/**
 * @brief Initializes the @p iot_mqtt_connection_options_t structure
 */
#define IOT_MQTT_CONNECT_OPTIONS_INIT \
	{ NULL, NULL, 0u, NULL, NULL, NULL, NULL }

/**
 * @brief internal MQTT structure
 */
typedef struct iot_mqtt iot_mqtt_t;

/** @brief signature of function to be called when a connection is detected */
typedef void (*iot_mqtt_connect_callback_t)(
	void *user_data,
	iot_bool_t unexpected );

/** @brief signature of function to be called when a disconnection is detected */
typedef void (*iot_mqtt_disconnect_callback_t)(
	void *user_data,
	iot_bool_t unexpected );

/** @brief signature of function to be called when a message is delivered */
typedef void (*iot_mqtt_delivery_callback_t)(
	void *user_data,
	int msg_id );

/**
 * @brief signature of function to be called when a message is received
 */
typedef void (*iot_mqtt_message_callback_t)(
	void *user_data,
	const char *topic,
	void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain );

/**
 * @brief connects to an MQTT server
 *
 * @note The @p username & @p password fields are only applicable to clients
 * that connect using MQTT version 3.1.1 protocol or higher.
 *
 * @param[in]      opts                connection options
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 *
 * @retval         NULL                failed to connect
 * @retval         !NULL               successful connection established
 *
 * @see iot_mqtt_disconnect
 * @see iot_mqtt_reconnect
 */
IOT_API IOT_SECTION iot_mqtt_t* iot_mqtt_connect(
	const iot_mqtt_connect_options_t *opts,
	iot_millisecond_t max_time_out );

/**
 * @brief disconnects from an MQTT server
 *
 * @param[in]      mqtt                MQTT object to disconnect
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_connect
 * @see iot_mqtt_reconnect
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_disconnect(
	iot_mqtt_t* mqtt
	);

/**
 * @brief gets MQTT server connection status
 *
 * @param[in]      mqtt                  MQTT object to disconnect
 * @param[out]     connected             server is connected
 * @param[out]     connection_changed    server connection is changed
 * @param[out]     time_stamp_connection_changed
 *                                       time stamp when server connection was
 *                                       changed
 *
 * @retval IOT_STATUS_BAD_PARAMETER      invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE            operation failed
 * @retval IOT_STATUS_SUCCESS            operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_get_connection_status(
	const iot_mqtt_t* mqtt,
	iot_bool_t *connected,
	iot_bool_t *connection_changed,
	iot_timestamp_t *time_stamp_connection_changed );

/**
 * @brief initializes MQTT functionality
 *
 * @retval IOT_STATUS_SUCCESS                    always
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_initialize( void );

/**
 * @brief Called every loop iteration
 *
 * @param[in]      mqtt                MQTT object to subscribe to
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_loop( iot_mqtt_t *mqtt,
	iot_millisecond_t max_time_out );

/**
 * @brief publishes a message to the mqtt connection to a specified topic
 *
 * @param[in]      mqtt                MQTT object to transmit to
 * @param[in]      topic               topic to transmit on
 * @param[in]      payload             message to transmit
 * @param[in]      payload_len         size of message
 * @param[in]      qos                 MQTT QOS level to use (0, 1 or 3)
 * @param[in]      retain              retain the message
 * @param[out]     msg_id              message id assigned to the message
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_publish(
	iot_mqtt_t *mqtt,
	const char *topic,
	const void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain,
	int *msg_id );

/**
 * @brief sets the callback for notification of a disconnection
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      cb                  call back to be set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_delivery_callback
 * @see iot_mqtt_set_message_callback
 * @see iot_mqtt_set_user_data
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_disconnect_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_disconnect_callback_t cb );

/**
 * @brief sets the callback for notification of a message delivery
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      cb                  call back to be set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_disconnect_callback
 * @see iot_mqtt_set_message_callback
 * @see iot_mqtt_set_user_data
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_delivery_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_delivery_callback_t cb );

/**
 * @brief sets the callback for receiving incoming messages
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      cb                  call back to be set
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_disconnect_callback
 * @see iot_mqtt_set_delivery_callback
 * @see iot_mqtt_set_user_data
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_message_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_message_callback_t cb );

/**
 * @brief sets the user data to pass when a callback is called
 *
 * @param[in]      mqtt                MQTT object to set callback on
 * @param[in]      user_data           user data to pass to callbacks
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_set_disconnect_callback
 * @see iot_mqtt_set_delivery_callback
 * @see iot_mqtt_set_message_callback
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_set_user_data(
	iot_mqtt_t *mqtt,
	void *user_data );

/**
 * @brief reconnects to an MQTT server after a connection
 *
 * @param[in,out]  mqtt                MQTT object to subscribe to
 * @param[in]      opts                connection options
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_connect
 * @see iot_mqtt_disconnect
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_reconnect(
	iot_mqtt_t *mqtt,
	const iot_mqtt_connect_options_t *opts,
	iot_millisecond_t max_time_out );

/**
 * @brief subscribes for messages on an MQTT topic
 *
 * @param[in]      mqtt                MQTT object to subscribe to
 * @param[in]      topic               topic to subscribe for (may include wildcards)
 * @param[in]      qos                 MQTT QOS level to use (0, 1 or 3)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_subscribe(
	iot_mqtt_t *mqtt,
	const char *topic,
	int qos );

/**
 * @brief terminates MQTT functionality
 *
 * @retval IOT_STATUS_SUCCESS                    always
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_terminate( void );

/**
 * @brief unsubscribes from an MQTT topic
 *
 * @param[in]      mqtt                MQTT object to unsubscribe from
 * @param[in]      topic               topic to unsubscribe for (may include wildcards)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
IOT_API IOT_SECTION iot_status_t iot_mqtt_unsubscribe(
	iot_mqtt_t *mqtt,
	const char *topic );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */
#endif /* ifndef IOT_MQTT_H */
