/**
 * @file
 * @brief source file defining common mqtt function implementations
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

#include "public/iot_mqtt.h"

#include "shared/iot_defs.h"
#include "shared/iot_types.h"

#ifdef IOT_MQTT_MOSQUITTO
#	include <mosquitto.h>
#else /* ifdef IOT_MQTT_MOSQUITTO */
#	ifdef IOT_THREAD_SUPPORT
#		include <MQTTAsync.h>
#	else /* ifdef IOT_THREAD_SUPPORT */
#		include <MQTTClient.h>
#	endif /* else ifdef IOT_THREAD_SUPPORT */
#endif /* else ifdef IOT_MQTT_MOSQUITTO */

/** @brief Defualt MQTT port for non-SSL connections */
#define IOT_MQTT_PORT                  1883
/** @brief Default MQTT port for SSL connections */
#define IOT_MQTT_PORT_SSL              8883

/** @brief count of the number of times that MQTT initalize has been called */
static unsigned int MQTT_INIT_COUNT = 0u;

#ifdef IOT_MQTT_MOSQUITTO
/**
 * @brief callback called when a connection is established
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      rc                  the reason for the connection
 */
static IOT_SECTION void iot_mqtt_on_connect(
	struct mosquitto *mosq,
	void *user_data,
	int rc );
/**
 * @brief callback called when a connection is terminated
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      rc                  the reason for the disconnection
 */
static IOT_SECTION void iot_mqtt_on_disconnect(
	struct mosquitto *mosq,
	void *user_data,
	int rc );
/**
 * @brief callback called when a message is delivered
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      msg_id              id of the message delivered
 */
static IOT_SECTION void iot_mqtt_on_delivery(
	struct mosquitto *mosq,
	void *user_data,
	int msg_id );
/**
 * @brief callback called when a message is received
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      message             message being received
 */
static IOT_SECTION void iot_mqtt_on_message(
	struct mosquitto *mosq,
	void *user_data,
	const struct mosquitto_message *message );
/**
 * @brief callback called on a subscription
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      msg_id              id of the message delivered
 * @param[in]      qos_count           the number of granted subscriptions
 *                                     (size of granted_qos).
 * @param[in]      granted_qos         an array of integers indicating the
 *                                     granted QoS for each of the subscriptions.
 */
static IOT_SECTION void iot_mqtt_on_subscribe(
	struct mosquitto *mosq,
	void *obj,
	int msg_id,
	int qos_count,
	const int *granted_qos );
/**
 * @brief callback called on a subscription
 *
 * @param[in]      mosq                mosquitto instance calling the callback
 * @param[in]      user_data           user data provided in <mosquitto_new>
 * @param[in]      level               the log message level from the values:
 *                                     MOSQ_LOG_INFO, MOSQ_LOG_NOTICE,
 *                                     MOSQ_LOG_WARNING, MOSQ_LOG_ERR,
 *                                     MOSQ_LOG_DEBUG
 * @param[in]      str                 the message string
 */
void iot_mqtt_on_log(
	struct mosquitto *mosq,
	void *obj,
	int level,
	const char *str );
#else /* ifdef IOT_MQTT_MOSQUITTO */
/**
 * @def PAHO_OBJ
 * @brief Macro to replace the paho functions & objects with the correct prefix
 *
 * This macro allows for easily changing between PAHO's Asynchronous &
 * Synchronous C libraries with calling functions or using object types.
 * This macro allows for a quick replace of the prefix for the return type in
 * the library (either MQTTAsync or MQTTClient).  Using a macro allows for less
 * copying for similar code, when switching between the two library types.
 *
 * @param x        name of the function to call
 *
 * @see PAHO_RES
 */
/**
 * @def PAHO_OBJ
 * @brief Macro to replace the paho return codes with the correct prefix
 *
 * This macro allows for easily changing between PAHO's Asynchronous &
 * Synchronous C libraries with determining return codes from the library.
 * This macro allows for a quick replace of the prefix for the return type in
 * the library (either MQTTAsync or MQTTClient).  Using a macro allows for less
 * copying for similar code, when switching between the two library types.
 *
 * @param x        name of the function to call
 *
 * @see PAHO_OBJ
 */
#ifdef IOT_THREAD_SUPPORT
#define PAHO_OBJ(x)        MQTTAsync ## x
#define PAHO_RES(x)        MQTTASYNC ## x

/**
 * @brief callback called on failure of a subscribe, unsubscribe or
 * sending of a message
 *
 * @param[in]      user_data           context user data
 * @param[in]      response            response data
 */
static IOT_SECTION void iot_mqtt_on_failure(
	void *user_data,
	MQTTAsync_failureData *response
);

/**
 * @brief callback called on success of a subscribe, unsubscribe or
 * sending of a message
 *
 * @param[in]      user_data           context user data
 * @param[in]      response            response data
 */
static IOT_SECTION void iot_mqtt_on_success(
	void *user_data,
	MQTTAsync_successData *response
);
#else /* ifdef IOT_THREAD_SUPPORT */
#define PAHO_OBJ(x)        MQTTClient ## x
#define PAHO_RES(x)        MQTTCLIENT ## x
#endif /* else ifdef IOT_THREAD_SUPPORT */

/**
 * @brief callback called when a connection is terminated
 *
 * @param[in]      user_data           context user data
 * @param[in]      cause               string indicating the reason
 */
static IOT_SECTION void iot_mqtt_on_disconnect(
	void *user_data,
	char *cause );

/**
 * @brief callback called when a message is delivered
 *
 * @param[in]      user_data           context user data
 * @param[in]      token               delievery token for the message
 */
static IOT_SECTION void iot_mqtt_on_delivery(
	void *user_data,
#ifdef IOT_THREAD_SUPPORT
	MQTTAsync_token token
#else
	MQTTClient_deliveryToken token
#endif
	);
/**
 * @brief callback called when a message is received
 *
 * @param[in]      user_data           context user data
 * @param[in]      topic               topic message received on
 * @param[in]      topic_len           length of the topic string
 * @param[in]      message             message being received
 */
static IOT_SECTION int iot_mqtt_on_message(
	void *user_data,
	char *topic,
	int topic_len,
	PAHO_OBJ( _message ) *message );
#endif /* else ifdef IOT_MQTT_MOSQUITTO */

/**
 * @brief implementation for connecting to an MQTT broker
 *
 * @param[in,out]  mqtt                MQTT object structure to connect with
 * @param[in]      opts                connection options
 * @param[in]      max_time_out        maximum time to wait
 *                                     (0 = wait indefinitely)
 * @param[in]      reconnect           whether this is a reconnection or an
 *                                     original connection
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed to the function
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 *
 * @see iot_mqtt_connect
 * @see iot_mqtt_reconnect
 */
static IOT_SECTION iot_status_t iot_mqtt_connect_impl(
	iot_mqtt_t *mqtt,
	const iot_mqtt_connect_options_t *opts,
	iot_millisecond_t max_time_out,
	iot_bool_t reconnect );

/** @brief number of seconds before sending a keep alive message */
#define IOT_MQTT_KEEP_ALIVE            60u
/** @brief maximum length for an mqtt connection url */
#define IOT_MQTT_URL_MAX               64u

/** @brief internal object containing information for managing the connection */
struct iot_mqtt
{
#ifdef IOT_THREAD_SUPPORT
	/** @brief Mutex to protect signal condition variable */
	os_thread_mutex_t                notification_mutex;
	/** @brief Signal for waking another thread waiting for notification */
	os_thread_condition_t            notification_signal;
#endif /* ifdef IOT_THREAD_SUPPORT */

#ifdef IOT_MQTT_MOSQUITTO
	/** @brief pointer to the mosquitto client instance */
	struct mosquitto                 *mosq;
#else /* ifdef IOT_MQTT_MOSQUITTO */
#ifdef IOT_THREAD_SUPPORT
	/** @brief paho asynchronous client instance */
	MQTTAsync                        client;
	/** @brief Current message identifier, increments each message */
	MQTTAsync_token                  msg_id;
#else /* ifdef IOT_THREAD_SUPPORT */
	/** @brief paho synchronous client instance */
	MQTTClient                       client;
#endif /* else ifdef IOT_THREAD_SUPPORT */
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
	/** @brief whether the client is expected to be connected */
	iot_bool_t                       is_connected;
	/** @brief whether the client cloud connection is changed */
	iot_bool_t                       connection_changed;
	/** @brief timestamp when the client cloud connection is changed */
	iot_timestamp_t                  time_stamp_changed;
	/** @brief the client cloud reconnect counter */
	iot_uint32_t                     reconnect_count;
	/** @brief callback to call when a disconnection is detected */
	iot_mqtt_disconnect_callback_t   on_disconnect;
	/** @brief callback to call when a message is delivered */
	iot_mqtt_delivery_callback_t     on_delivery;
	/** @brief callback to call when a message is received */
	iot_mqtt_message_callback_t      on_message;
	/** @brief user specified data to pass to callbacks */
	void * user_data;
};

iot_mqtt_t* iot_mqtt_connect(
	const iot_mqtt_connect_options_t *opts,
	iot_millisecond_t max_time_out )
{
	iot_mqtt_t *result = NULL;
	if ( opts && opts->host && opts->client_id )
	{
		result = (iot_mqtt_t*)os_malloc( sizeof( struct iot_mqtt ) );
		if ( result )
		{
			iot_status_t connect_result = IOT_STATUS_FAILURE;
#ifndef IOT_MQTT_MOSQUITTO
			iot_uint16_t port = opts->port;
			char url[IOT_MQTT_URL_MAX + 1u];
#endif /* ifndef IOT_MQTT_MOSQUITTO */

			os_memzero( result, sizeof( struct iot_mqtt ) );

#ifdef IOT_THREAD_SUPPORT
			os_thread_mutex_create(
				&result->notification_mutex );
			os_thread_condition_create(
				&result->notification_signal );
#endif /* ifdef IOT_THREAD_SUPPORT */

#ifdef IOT_MQTT_MOSQUITTO
			result->mosq = mosquitto_new( opts->client_id, true,
				result );
			if ( result->mosq )
			{
#else /* ifdef IOT_MQTT_MOSQUITTO */
			if ( port == 0u )
			{
				if ( opts->ssl_conf )
					port = IOT_MQTT_PORT_SSL;
				else
					port = IOT_MQTT_PORT;
			}

			if ( opts->ssl_conf && port != IOT_MQTT_PORT )
				os_snprintf( url, IOT_MQTT_URL_MAX,
					"ssl://%s:%d", opts->host, port );
			else
				os_snprintf( url, IOT_MQTT_URL_MAX,
					"tcp://%s:%d", opts->host, port );
			url[ IOT_MQTT_URL_MAX ] = '\0';

			if ( PAHO_OBJ( _create )( &result->client, url,
				opts->client_id, MQTTCLIENT_PERSISTENCE_NONE,
				NULL ) == PAHO_RES( _SUCCESS ) )
			{
#endif /* else ifdef IOT_MQTT_MOSQUITTO */

				/* try to connect */
				connect_result = iot_mqtt_connect_impl(
					result, opts, max_time_out, IOT_FALSE );
			}

			/* failed to connect, so let's clean up */
			if ( connect_result != IOT_STATUS_SUCCESS )
			{
#ifdef IOT_MQTT_MOSQUITTO
				if ( result->mosq )
					mosquitto_destroy( result->mosq );
#else /* ifdef IOT_MQTT_MOSQUITTO */
				PAHO_OBJ( _destroy )( &result->client );
#endif /* else ifdef IOT_MQTT_MOSQUITTO */

#ifdef IOT_THREAD_SUPPORT
				os_thread_condition_destroy(
					&result->notification_signal );
				os_thread_mutex_destroy(
					&result->notification_mutex );
#endif /* ifdef IOT_THREAD_SUPPORT */

				os_free( result );
				result = NULL;
			}
		}
	}
	return result;
}

iot_status_t iot_mqtt_connect_impl(
	iot_mqtt_t *mqtt,
	const iot_mqtt_connect_options_t *opts,
	iot_millisecond_t max_time_out,
	iot_bool_t reconnect )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt && opts && opts->host && opts->client_id )
	{
#ifdef IOT_MQTT_MOSQUITTO
		int mosq_res;
#else /* ifdef IOT_MQTT_MOSQUITTO */
		PAHO_OBJ( _connectOptions ) conn_opts =
			PAHO_OBJ( _connectOptions_initializer );
		PAHO_OBJ( _SSLOptions ) ssl_opts =
			PAHO_OBJ( _SSLOptions_initializer );
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
		iot_uint16_t port = opts->port;
		mqtt->is_connected = IOT_FALSE;

		result = IOT_STATUS_FAILURE;
		if ( port == 0u )
		{
			if ( opts->ssl_conf )
				port = IOT_MQTT_PORT_SSL;
			else
				port = IOT_MQTT_PORT;
		}

#ifdef IOT_MQTT_MOSQUITTO
		mosquitto_connect_callback_set( mqtt->mosq,
			iot_mqtt_on_connect );
		mosquitto_disconnect_callback_set( mqtt->mosq,
			iot_mqtt_on_disconnect );
		mosquitto_message_callback_set( mqtt->mosq,
			iot_mqtt_on_message );
		mosquitto_publish_callback_set( mqtt->mosq,
			iot_mqtt_on_delivery );
		mosquitto_subscribe_callback_set( mqtt->mosq,
			iot_mqtt_on_subscribe );
		mosquitto_log_callback_set( mqtt->mosq,
			iot_mqtt_on_log );
		if ( opts->username && opts->password )
			mosquitto_username_pw_set( mqtt->mosq,
				opts->username, opts->password );

		if ( opts->proxy_conf )
		{
			if ( opts->proxy_conf->type == IOT_PROXY_SOCKS5 )
				mosquitto_socks5_set(
					mqtt->mosq,
					opts->proxy_conf->host,
					(int)opts->proxy_conf->port,
					opts->proxy_conf->username,
					opts->proxy_conf->password );
			else
				os_fprintf(OS_STDERR,
					"unsuppored proxy setting: "
					"host port %d proxy_type %d !\n",
					(int)opts->proxy_conf->port,
					(int)opts->proxy_conf->type );
		}

		if ( opts->ssl_conf && port != IOT_MQTT_PORT )
		{
			mosquitto_tls_set( mqtt->mosq,
				opts->ssl_conf->ca_path, NULL,
				opts->ssl_conf->cert_file,
				opts->ssl_conf->key_file, NULL );
			mosquitto_tls_insecure_set(
				mqtt->mosq,
				opts->ssl_conf->insecure );
		}

		switch ( opts->version )
		{
			case IOT_MQTT_VERSION_3_1:
			{
				int ver = MQTT_PROTOCOL_V31;
				mosquitto_opts_set(
					mqtt->mosq,
					MOSQ_OPT_PROTOCOL_VERSION,
					&ver );
				break;
			}
			case IOT_MQTT_VERSION_3_1_1:
			{
				int ver = MQTT_PROTOCOL_V31;
				mosquitto_opts_set(
					mqtt->mosq,
					MOSQ_OPT_PROTOCOL_VERSION,
					&ver );
				break;
			}
			case IOT_MQTT_VERSION_DEFAULT:
			default:
				break;
		}

#ifdef IOT_THREAD_SUPPORT
		if ( reconnect == IOT_FALSE )
			mosq_res = mosquitto_connect_async( mqtt->mosq,
				opts->host, opts->port, IOT_MQTT_KEEP_ALIVE );
		else
			mosq_res = mosquitto_reconnect_async( mqtt->mosq );

		if ( mosq_res == MOSQ_ERR_SUCCESS )
		{
			mosquitto_loop_start( mqtt->mosq );

			/* wait here until connected or timed out! */
			if ( max_time_out > 0u )
				os_thread_condition_timed_wait(
					&mqtt->notification_signal,
					&mqtt->notification_mutex,
					max_time_out );
			else
				os_thread_condition_wait(
					&mqtt->notification_signal,
					&mqtt->notification_mutex );
		}
#else /* ifdef IOT_THREAD_SUPPORT */
		if ( reconnect == IOT_FALSE )
			mosq_res = mosquitto_connect( mqtt->mosq,
				opts->host, opts->port, IOT_MQTT_KEEP_ALIVE );
		else
			mosq_res = mosquitto_reconnect( mqtt->mosq );

		if ( mosq_res == MOSQ_ERR_SUCCESS )
		{
			os_timestamp_t ts = 0u;

			/* default is to wait for 1 day */
			iot_millisecond_t wait_time =
				IOT_MILLISECONDS_IN_SECOND *
				IOT_SECONDS_IN_MINUTE *
				IOT_MINUTES_IN_HOUR * IOT_HOURS_IN_DAY;

			if ( max_time_out > 0u )
				wait_time = max_time_out;

			/* loop until connected or time out */
			os_time( &ts, NULL );
			while ( mosq_res == MOSQ_ERR_SUCCESS &&
				mqtt->is_connected == IOT_FALSE &&
				wait_time > 0u )
			{
				mosq_res = mosquitto_loop(
					mqtt->mosq,
					wait_time, 1 );

				if ( max_time_out > 0u )
					os_time_remaining( &ts,
						max_time_out,
						&wait_time );
			}
		}
#endif /* ifdef IOT_THREAD_SUPPORT */
#else /* ifdef IOT_MQTT_MOSQUITTO */
		if ( opts->proxy_conf )
			os_fprintf(OS_STDERR,
				"unsuppored proxy setting: "
				"host port %d proxy_type %d !\n",
				(int)opts->proxy_conf->port,
				(int)opts->proxy_conf->type );

		/* this uses PAHO in asynchronous mode, this
		 * mode is only safe in single-thread mode */
		PAHO_OBJ( _setCallbacks )( mqtt->client,
			mqtt,
			iot_mqtt_on_disconnect,
			iot_mqtt_on_message,
			iot_mqtt_on_delivery );
		conn_opts.keepAliveInterval = IOT_MQTT_KEEP_ALIVE;
		conn_opts.cleansession = !reconnect;
		if ( opts->version != IOT_MQTT_VERSION_3_1 )
		{
			conn_opts.username = opts->username;
			conn_opts.password = opts->password;
		}

		switch ( opts->version )
		{
			case IOT_MQTT_VERSION_3_1:
				conn_opts.MQTTVersion = MQTTVERSION_3_1;
				break;
			case IOT_MQTT_VERSION_3_1_1:
				conn_opts.MQTTVersion = MQTTVERSION_3_1_1;
				break;
			case IOT_MQTT_VERSION_DEFAULT:
			default:
				break;
		}

#ifdef IOT_THREAD_SUPPORT
		conn_opts.onSuccess = iot_mqtt_on_success;
		conn_opts.onFailure = iot_mqtt_on_failure;
		conn_opts.context = mqtt;
#endif /* ifdef IOT_THREAD_SUPPORT */

		if ( opts->ssl_conf && port != IOT_MQTT_PORT )
		{ /* ssl */
			/* ssl args */
			ssl_opts.trustStore = opts->ssl_conf->ca_path;
			ssl_opts.enableServerCertAuth =
				!opts->ssl_conf->insecure;

			/* client cert and key */
			ssl_opts.keyStore = opts->ssl_conf->cert_file;
			ssl_opts.privateKey = opts->ssl_conf->key_file;

			conn_opts.ssl = &ssl_opts;
		}

		if ( max_time_out > 0u )
			conn_opts.connectTimeout =
				(max_time_out /
				IOT_MILLISECONDS_IN_SECOND) + 1u;

		if ( PAHO_OBJ( _connect )( mqtt->client,
				&conn_opts ) == PAHO_RES( _SUCCESS ) )
		{
#ifdef IOT_THREAD_SUPPORT
			/* wait here until connected or timed out! */
			if ( max_time_out > 0u )
				os_thread_condition_timed_wait(
					&mqtt->notification_signal,
					&mqtt->notification_mutex,
					max_time_out );
			else
				os_thread_condition_wait(
					&mqtt->notification_signal,
					&mqtt->notification_mutex );
#else /* ifdef IOT_THREAD_SUPPORT */
			mqtt->is_connected = IOT_TRUE;
#endif /* else ifdef IOT_THREAD_SUPPORT */
		}
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
		/* if we connected then success */
		if ( mqtt->is_connected != IOT_FALSE )
			result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_disconnect(
	iot_mqtt_t* mqtt )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		result = IOT_STATUS_FAILURE;
#ifdef IOT_MQTT_MOSQUITTO
		if ( mqtt->is_connected != IOT_FALSE &&
			mosquitto_disconnect( mqtt->mosq ) == MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#ifdef IOT_THREAD_SUPPORT
		mosquitto_loop_stop( mqtt->mosq, true );
#endif /* ifdef IOT_THREAD_SUPPORT */
		mosquitto_destroy( mqtt->mosq );
		mqtt->mosq = NULL;
#else /* ifdef IOT_MQTT_MOSQUITTO */
#ifdef IOT_THREAD_SUPPORT
		{
			MQTTAsync_disconnectOptions opts =
				MQTTAsync_disconnectOptions_initializer;
			opts.timeout = IOT_MQTT_KEEP_ALIVE;
			if ( MQTTAsync_disconnect( mqtt->client, &opts )
				== MQTTASYNC_SUCCESS )
				result = IOT_STATUS_SUCCESS;
			MQTTAsync_destroy( &mqtt->client );
		}
#else /* ifdef IOT_THREAD_SUPPORT */
		if ( MQTTClient_disconnect( mqtt->client, IOT_MQTT_KEEP_ALIVE )
			== MQTTCLIENT_SUCCESS )
			result = IOT_STATUS_SUCCESS;
		MQTTClient_destroy( &mqtt->client );
#endif /* else ifdef IOT_THREAD_SUPPORT */
#endif /* else ifdef IOT_MQTT_MOSQUITTO */

#ifdef IOT_THREAD_SUPPORT
		os_thread_condition_destroy( &mqtt->notification_signal );
		os_thread_mutex_destroy( &mqtt->notification_mutex );
#endif /* ifdef IOT_THREAD_SUPPORT */

		mqtt->is_connected = IOT_FALSE;
		os_free( mqtt );
	}
	return result;
}

IOT_API IOT_SECTION iot_status_t iot_mqtt_get_connection_status(
	const iot_mqtt_t* mqtt,
	iot_bool_t *connected,
	iot_bool_t *connection_changed,
	iot_timestamp_t *time_stamp_connection_changed )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		if( connected )
			*connected = mqtt->is_connected;
		if( connection_changed )
			*connection_changed = mqtt->connection_changed;
		if ( time_stamp_connection_changed )
			*time_stamp_connection_changed =
				mqtt->time_stamp_changed;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_initialize( void )
{
	if ( MQTT_INIT_COUNT == 0u )
	{
#ifdef IOT_MQTT_MOSQUITTO
		mosquitto_lib_init();
#endif /* ifdef IOT_MQTT_MOSQUITTO */
	}
	++MQTT_INIT_COUNT;
	return IOT_STATUS_SUCCESS;
}

iot_status_t iot_mqtt_loop( iot_mqtt_t *mqtt,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
#ifdef IOT_MQTT_MOSQUITTO
#ifdef IOT_THREAD_SUPPORT
		(void)max_time_out;
		result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_THREAD_SUPPORT */
		if ( mosquitto_loop( mqtt->mosq, max_time_out, 1 )
			== MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#endif /* else ifdef IOT_THREAD_SUPPORT */
#else /* ifdef IOT_MQTT_MOSQUITTO */
		(void)max_time_out;
		result = IOT_STATUS_SUCCESS;
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
	}
	return result;
}

#ifdef IOT_MQTT_MOSQUITTO
void iot_mqtt_on_connect(
	struct mosquitto *UNUSED(mosq),
	void *user_data,
	int UNUSED(rc) )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->is_connected == IOT_FALSE )
	{
		mqtt->is_connected = IOT_TRUE;
		mqtt->connection_changed = IOT_TRUE;
		mqtt->time_stamp_changed = iot_timestamp_now();

#ifdef IOT_THREAD_SUPPORT
		/* notify iot_mqtt_connect, that we've estalished a connection */
		os_thread_condition_signal( &mqtt->notification_signal,
			&mqtt->notification_mutex );
#endif /* ifdef IOT_THREAD_SUPPORT */
	}
}

void iot_mqtt_on_disconnect(
	struct mosquitto *UNUSED(mosq),
	void *user_data,
	int rc )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt )
	{
		const iot_bool_t unexpected = rc ? IOT_TRUE : IOT_FALSE;
		mqtt->is_connected = IOT_FALSE;
		mqtt->connection_changed = IOT_TRUE;
		mqtt->time_stamp_changed = iot_timestamp_now();
		mqtt->reconnect_count = 0u;
		if ( mqtt->on_disconnect )
			mqtt->on_disconnect( mqtt->user_data, unexpected );
	}
}

void iot_mqtt_on_delivery(
	struct mosquitto *UNUSED(mosq),
	void *user_data,
	int msg_id )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_delivery )
		mqtt->on_delivery( mqtt->user_data, msg_id );
}

void iot_mqtt_on_message(
	struct mosquitto *UNUSED(mosq),
	void *user_data,
	const struct mosquitto_message *message )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_message )
		mqtt->on_message( mqtt->user_data, message->topic,
		message->payload, (size_t)message->payloadlen, message->qos,
		message->retain );
}

void iot_mqtt_on_subscribe(
	struct mosquitto *UNUSED(mosq),
	void *UNUSED(obj),
	int UNUSED(msg_id),
	int UNUSED(qos_count),
	const int *UNUSED(granted_qos) )
{
}

void iot_mqtt_on_log(
	struct mosquitto *UNUSED(mosq),
	void *UNUSED(obj),
	int UNUSED(level),
	const char *UNUSED(str) )
{
}

#else /* ifdef IOT_MQTT_MOSQUITTO */
void iot_mqtt_on_disconnect(
	void *user_data,
	char *UNUSED(cause) )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt )
	{
		/* this function is called after a keep-alive timeout fails */
		const iot_bool_t unexpected = mqtt->is_connected;
		mqtt->is_connected = IOT_FALSE;
		mqtt->connection_changed = IOT_TRUE;
		mqtt->time_stamp_changed = iot_timestamp_now();
		mqtt->reconnect_count = 0u;
		if( mqtt->on_disconnect )
			mqtt->on_disconnect( mqtt->user_data, unexpected );
	}
}

void iot_mqtt_on_delivery(
	void *user_data,
#ifdef IOT_THREAD_SUPPORT
	MQTTAsync_token token
#else /* ifdef IOT_THREAD_SUPPORT */
	MQTTClient_deliveryToken token
#endif /* else ifdef IOT_THREAD_SUPPORT */
	)
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_delivery )
		mqtt->on_delivery( mqtt->user_data, (int)token );
}

#ifdef IOT_THREAD_SUPPORT
void iot_mqtt_on_failure(
	void *user_data,
	MQTTAsync_failureData *response )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && response && response->token == 0u )
	{
		if ( mqtt->is_connected != IOT_FALSE )
		{
			mqtt->is_connected = IOT_FALSE;
			mqtt->time_stamp_changed = iot_timestamp_now();
		}
		os_thread_condition_signal( &mqtt->notification_signal,
			&mqtt->notification_mutex );
	}
}
#endif /* ifdef IOT_THREAD_SUPPORT */

int iot_mqtt_on_message(
	void *user_data,
	char *topic,
	int UNUSED(topic_len),
	PAHO_OBJ( _message ) *message )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && mqtt->on_message )
		mqtt->on_message( mqtt->user_data, topic, message->payload,
		(size_t)message->payloadlen, message->qos, message->retained );

	/* message succesfully handled */
	PAHO_OBJ(_freeMessage) ( &message );
	PAHO_OBJ(_free)( topic );
	return 1; /* true */
}

#ifdef IOT_THREAD_SUPPORT
void iot_mqtt_on_success(
	void *user_data,
	MQTTAsync_successData *response )
{
	iot_mqtt_t *const mqtt = (iot_mqtt_t *)user_data;
	if ( mqtt && response && response->token == 0u )
	{
		/* called on a connection */
		if ( mqtt->is_connected == IOT_FALSE )
		{
			mqtt->is_connected = IOT_TRUE;
			mqtt->time_stamp_changed = iot_timestamp_now();
		}
		os_thread_condition_signal( &mqtt->notification_signal,
			&mqtt->notification_mutex );
	}
}
#endif /* ifdef IOT_THREAD_SUPPORT */
#endif /* else ifdef IOT_MQTT_MOSQUITTO */

iot_status_t iot_mqtt_publish(
	iot_mqtt_t *mqtt,
	const char *topic,
	const void *payload,
	size_t payload_len,
	int qos,
	iot_bool_t retain,
	int *msg_id )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	int mid = 0;
	qos = 1;
	if ( mqtt )
	{
#ifdef IOT_MQTT_MOSQUITTO
		result = IOT_STATUS_FAILURE;
		if ( mosquitto_publish( mqtt->mosq, &mid, topic, payload_len,
			payload, qos, retain ) == MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_MQTT_MOSQUITTO */
		void *pl = os_malloc( payload_len );
		result = IOT_STATUS_NO_MEMORY;
		if ( pl )
		{
#ifdef IOT_THREAD_SUPPORT
			int rs;
			const MQTTAsync_token token = mqtt->msg_id++;
			MQTTAsync_responseOptions opts =
				MQTTAsync_responseOptions_initializer;
			opts.context = mqtt;
			opts.token = token;
			opts.onFailure = iot_mqtt_on_failure;
			opts.onSuccess = iot_mqtt_on_success;

			os_memcpy( pl, payload, payload_len );
			result = IOT_STATUS_FAILURE;
			rs = MQTTAsync_send( mqtt->client, topic,
				(int)payload_len, pl, qos, retain, &opts );
			if ( rs == MQTTASYNC_SUCCESS )
#else /* ifdef IOT_THREAD_SUPPORT */
			MQTTClient_deliveryToken token;
			os_memcpy( pl, payload, payload_len );
			result = IOT_STATUS_FAILURE;
			if ( MQTTClient_publish( mqtt->client, topic,
				(int)payload_len, pl, qos, retain, &token )
				== MQTTCLIENT_SUCCESS )
#endif /* ifdef IOT_THREAD_SUPPORT */
			{
				mid = (int)token;
				result = IOT_STATUS_SUCCESS;
			}
			os_free( pl );
		}
#endif /* else IOT_MQTT_MOSQUITTO */
	}

	if ( msg_id )
		*msg_id = mid;
	return result;
}

iot_status_t iot_mqtt_reconnect(
	iot_mqtt_t *mqtt,
	const iot_mqtt_connect_options_t *opts,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
#ifdef IOT_MQTT_MOSQUITTO
	(void)max_time_out;
	if ( opts && opts->host && opts->client_id && mqtt )
#else /* ifdef IOT_MQTT_MOSQUITTO */
	if ( opts && opts->host && opts->client_id && mqtt && mqtt->client)
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
	{
		result = iot_mqtt_connect_impl(
			mqtt, opts, max_time_out, IOT_TRUE );
	}
	return result;
}

iot_status_t iot_mqtt_set_disconnect_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_disconnect_callback_t cb )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->on_disconnect = cb;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_set_delivery_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_delivery_callback_t cb )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->on_delivery = cb;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_set_message_callback(
	iot_mqtt_t *mqtt,
	iot_mqtt_message_callback_t cb )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->on_message = cb;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_set_user_data(
	iot_mqtt_t *mqtt,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
		mqtt->user_data = user_data;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_mqtt_subscribe( iot_mqtt_t *mqtt, const char *topic, int qos )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
#ifdef IOT_MQTT_MOSQUITTO
		result = IOT_STATUS_FAILURE;
		if ( mosquitto_subscribe( mqtt->mosq, NULL, topic, qos )
			== MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_MQTT_MOSQUITTO */
#ifdef IOT_THREAD_SUPPORT
		MQTTAsync_responseOptions opts =
			MQTTAsync_responseOptions_initializer;
		opts.context = mqtt;
		opts.token = mqtt->msg_id++;
		opts.onFailure = iot_mqtt_on_failure;
		opts.onSuccess = iot_mqtt_on_success;

		result = IOT_STATUS_FAILURE;
		if ( MQTTAsync_subscribe( mqtt->client, topic, qos, &opts )
			== MQTTASYNC_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_THREAD_SUPPORT */
		result = IOT_STATUS_FAILURE;
		if ( MQTTClient_subscribe( mqtt->client, topic, qos )
			== MQTTCLIENT_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#endif /* else ifdef IOT_THREAD_SUPPORT */
#endif /* else IOT_MQTT_MOSQUITTO */
	}
	return result;
}

iot_status_t iot_mqtt_terminate( void )
{
	--MQTT_INIT_COUNT;
	if ( MQTT_INIT_COUNT == 0u )
	{
#ifdef IOT_MQTT_MOSQUITTO
		mosquitto_lib_cleanup();
#endif /* ifdef IOT_MQTT_MOSQUITTO */
	}
	return IOT_STATUS_SUCCESS;
}

iot_status_t iot_mqtt_unsubscribe( iot_mqtt_t *mqtt, const char *topic )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( mqtt )
	{
#ifdef IOT_MQTT_MOSQUITTO
		result = IOT_STATUS_FAILURE;
		if ( mosquitto_unsubscribe( mqtt->mosq, NULL, topic )
			== MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_MQTT_MOSQUITTO */
#ifdef IOT_THREAD_SUPPORT
		MQTTAsync_responseOptions opts =
			MQTTAsync_responseOptions_initializer;
		opts.context = mqtt;
		opts.token = mqtt->msg_id++;
		opts.onFailure = iot_mqtt_on_failure;
		opts.onSuccess = iot_mqtt_on_success;

		result = IOT_STATUS_FAILURE;
		if ( MQTTAsync_unsubscribe( mqtt->client, topic, &opts )
			== MQTTASYNC_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#else /* ifdef IOT_THREAD_SUPPORT */
		result = IOT_STATUS_FAILURE;
		if ( MQTTClient_unsubscribe ( mqtt->client, topic )
			== MQTTCLIENT_SUCCESS )
			result = IOT_STATUS_SUCCESS;
#endif /* else ifdef IOT_THREAD_SUPPORT */
#endif /* else IOT_MQTT_MOSQUITTO */
	}
	return result;
}

