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

/** @brief number of seconds before sending a keep alive message */
#define IOT_MQTT_KEEP_ALIVE            60u
/** @brief maximum length for an mqtt connection url */
#define IOT_MQTT_URL_MAX               64u

/** @brief internal object containing information for managing the connection */
struct iot_mqtt
{
#ifdef IOT_THREAD_SUPPORT
	/** @brief Mutex to protect signal condition variable */
	os_thread_mutex_t           notification_mutex;
	/** @brief Signal for waking another thread waiting for notification */
	os_thread_condition_t       notification_signal;
#endif /* ifdef IOT_THREAD_SUPPORT */

#ifdef IOT_MQTT_MOSQUITTO
	/** @brief pointer to the mosquitto client instance */
	struct mosquitto *mosq;
#else /* ifdef IOT_MQTT_MOSQUITTO */
#ifdef IOT_THREAD_SUPPORT
	/** @brief paho asynchronous client instance */
	MQTTAsync client;
	/** @brief Current message identifier, increments each message */
	MQTTAsync_token msg_id;
#else /* ifdef IOT_THREAD_SUPPORT */
	/** @brief paho synchronous client instance */
	MQTTClient client;
#endif /* else ifdef IOT_THREAD_SUPPORT */
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
	/** @brief whether the client is expected to be connected */
	iot_bool_t connected;
	/** @brief whether the client cloud connection is changed */
	iot_bool_t connection_changed;
	/** @brief timestamp when the client cloud connection is changed */
	iot_timestamp_t time_stamp_connection_changed;
	/** @brief the client cloud reconnect counter */
	iot_uint32_t reconnect_count;
	/** @brief callback to call when a disconnection is detected */
	iot_mqtt_disconnect_callback_t on_disconnect;
	/** @brief callback to call when a message is delivered */
	iot_mqtt_delivery_callback_t   on_delivery;
	/** @brief callback to call when a message is received */
	iot_mqtt_message_callback_t on_message;
	/** @brief user specified data to pass to callbacks */
	void * user_data;
};

iot_mqtt_t* iot_mqtt_connect(
	const char *client_id,
	const char *host,
	iot_uint16_t port,
	iot_mqtt_ssl_t *ssl_conf,
	iot_mqtt_proxy_t *proxy_conf,
	const char *username,
	const char *password,
	iot_millisecond_t max_time_out )
{
	iot_mqtt_t *result = NULL;
	if ( host && client_id )
	{
		if ( port == 0u )
		{
			if ( ssl_conf )
				port = IOT_MQTT_PORT_SSL;
			else
				port = IOT_MQTT_PORT;
		}

		result = (iot_mqtt_t*)os_malloc( sizeof( struct iot_mqtt ) );
		if ( result )
		{
#ifdef IOT_MQTT_MOSQUITTO
			(void)max_time_out;
			os_memzero( result, sizeof( struct iot_mqtt ) );
			result->mosq = mosquitto_new( client_id, true, result );
			if ( result->mosq )
			{
				mosquitto_connect_callback_set( result->mosq,
					iot_mqtt_on_connect );
				mosquitto_disconnect_callback_set( result->mosq,
					iot_mqtt_on_disconnect );
				mosquitto_message_callback_set( result->mosq,
					iot_mqtt_on_message );
				mosquitto_publish_callback_set( result->mosq,
					iot_mqtt_on_delivery );
				mosquitto_subscribe_callback_set( result->mosq,
					iot_mqtt_on_subscribe );
				mosquitto_log_callback_set( result->mosq,
					iot_mqtt_on_log );
				if ( username && password )
					mosquitto_username_pw_set( result->mosq,
						username, password );

				if ( proxy_conf )
				{
					if ( proxy_conf->type == IOT_PROXY_SOCKS5 )
						mosquitto_socks5_set(
							result->mosq,
							proxy_conf->host,
							(int)proxy_conf->port,
							proxy_conf->username,
							proxy_conf->password );
					else
						os_fprintf(OS_STDERR,
							"unsuppored proxy setting: "
							"host port %d proxy_type %d !\n",
							(int)port,
							(int)proxy_conf->type );
				}

				if ( ssl_conf && port != 1883u )
				{
					mosquitto_tls_set( result->mosq,
						ssl_conf->ca_path, NULL,
						ssl_conf->cert_file,
						ssl_conf->key_file, NULL );
					mosquitto_tls_insecure_set(
						result->mosq,
						ssl_conf->insecure );
				}

				if ( mosquitto_connect( result->mosq,
					host, port, IOT_MQTT_KEEP_ALIVE ) == MOSQ_ERR_SUCCESS )
				{
					mosquitto_loop_start( result->mosq );
					result->connected = IOT_TRUE;
					result->connection_changed = IOT_FALSE;
				}
				else
					os_free_null( (void**)&result );
			}
			else
				os_free_null( (void**)&result );
#else /* ifdef IOT_MQTT_MOSQUITTO */
			char url[IOT_MQTT_URL_MAX + 1u];
			if ( ssl_conf && port != 1883u )
				os_snprintf( url, IOT_MQTT_URL_MAX,
					"ssl://%s:%d", host, port );
			else
				os_snprintf( url, IOT_MQTT_URL_MAX,
					"tcp://%s:%d", host, port );
			url[ IOT_MQTT_URL_MAX ] = '\0';
			os_memzero( result, sizeof( struct iot_mqtt ) );

			if ( proxy_conf )
				os_fprintf(OS_STDERR,
					"unsuppored proxy setting: "
					"host port %d proxy_type %d !\n",
					(int)port,
					(int)proxy_conf->type );

			if ( MQTTClient_create( &result->client, url,
				client_id, MQTTCLIENT_PERSISTENCE_NONE,
				NULL ) == MQTTCLIENT_SUCCESS )
			{
				MQTTClient_connectOptions conn_opts =
					MQTTClient_connectOptions_initializer;

				/* ssl */
				MQTTClient_SSLOptions ssl_opts =
					MQTTClient_SSLOptions_initializer;

				MQTTClient_setCallbacks( result->client, result,
					iot_mqtt_on_disconnect, iot_mqtt_on_message,
					iot_mqtt_on_delivery );
				conn_opts.keepAliveInterval = IOT_MQTT_KEEP_ALIVE;
				conn_opts.cleansession = 1;
				conn_opts.username = username;
				conn_opts.password = password;

				if ( ssl_conf && port != 1883u )
				{ /* ssl */
					/* ssl args */
					ssl_opts.trustStore = ssl_conf->ca_path;
					ssl_opts.enableServerCertAuth =
						!ssl_conf->insecure;

					/* client cert and key */
					ssl_opts.keyStore = ssl_conf->cert_file;
					ssl_opts.privateKey = ssl_conf->key_file;

					conn_opts.ssl = &ssl_opts;
				}
				if ( max_time_out > 0u )
					conn_opts.connectTimeout =
						(max_time_out /
						IOT_MILLISECONDS_IN_SECOND) + 1u;
				if ( MQTTClient_connect( result->client,
						&conn_opts ) == MQTTCLIENT_SUCCESS )
					result->connected = IOT_TRUE;
				else
				{
					MQTTClient_destroy( &result->client );
					os_free_null( (void**)&result );
				}
			}
			else
				os_free_null( (void**)&result );
#endif /* else IOT_MQTT_MOSQUITTO */
		}
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
		if ( mosquitto_disconnect( mqtt->mosq ) == MOSQ_ERR_SUCCESS )
			result = IOT_STATUS_SUCCESS;
		mosquitto_loop_stop( mqtt->mosq, true );
		mosquitto_destroy( mqtt->mosq );
		mqtt->mosq = NULL;
#else /* ifdef IOT_MQTT_MOSQUITTO */
		mqtt->connected = IOT_FALSE;
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
		os_thread_condition_destroy( &mqtt->notification_signal );
		os_thread_mutex_destroy( &mqtt->notification_mutex );
#else /* ifdef IOT_THREAD_SUPPORT */
		if ( MQTTClient_disconnect( mqtt->client, IOT_MQTT_KEEP_ALIVE )
			== MQTTCLIENT_SUCCESS )
			result = IOT_STATUS_SUCCESS;
		MQTTClient_destroy( &mqtt->client );
#endif /* else ifdef IOT_THREAD_SUPPORT */
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
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
			*connected = mqtt->connected;
		if( connection_changed )
			*connection_changed = mqtt->connection_changed;
		if ( time_stamp_connection_changed )
			*time_stamp_connection_changed =
				mqtt->time_stamp_connection_changed;
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
	if ( mqtt && mqtt->connected == IOT_FALSE )
	{
		mqtt->connected = IOT_TRUE;
		mqtt->connection_changed = IOT_TRUE;
		mqtt->time_stamp_connection_changed = iot_timestamp_now();

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
		mqtt->connected = IOT_FALSE;
		mqtt->connection_changed = IOT_TRUE;
		mqtt->time_stamp_connection_changed = iot_timestamp_now();
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
		const iot_bool_t unexpected = mqtt->connected;
		mqtt->connected = IOT_FALSE;
		mqtt->connection_changed = IOT_TRUE;
		mqtt->time_stamp_connection_changed = iot_timestamp_now();
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
		mqtt->connected = IOT_FALSE;
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
		mqtt->connected = IOT_TRUE;
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
	const char *client_id,
	const char *host,
	iot_uint16_t port,
	iot_mqtt_ssl_t *ssl_conf,
	const char *username,
	const char *password,
	iot_millisecond_t max_time_out )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
#ifdef IOT_MQTT_MOSQUITTO
	(void)username;
	(void)password;
	(void)max_time_out;
	if ( host && client_id && mqtt )
#else /* ifdef IOT_MQTT_MOSQUITTO */
	if ( host && client_id && mqtt && mqtt->client)
#endif /* else ifdef IOT_MQTT_MOSQUITTO */
	{
		result = IOT_STATUS_FAILURE;
		if ( port == 0u )
		{
			if ( ssl_conf )
				port = IOT_MQTT_PORT_SSL;
			else
				port = IOT_MQTT_PORT;
		}

#ifdef IOT_MQTT_MOSQUITTO
		if ( mqtt->connection_changed == IOT_TRUE &&
		     mqtt->connected == IOT_TRUE )
		{
			mqtt->connection_changed = IOT_FALSE;
			result = IOT_STATUS_SUCCESS;
		}
#else /* ifdef IOT_MQTT_MOSQUITTO */
		if ( mqtt->connected == IOT_FALSE )
		{
			iot_timestamp_t time_stamp_current =
				iot_timestamp_now();
			iot_timestamp_t time_stamp_diff =
				time_stamp_current -
				mqtt->time_stamp_connection_changed;
			if ( time_stamp_diff >
				( mqtt->reconnect_count + 0u ) *
				IOT_MQTT_KEEP_ALIVE *
				IOT_MILLISECONDS_IN_SECOND )
			{ /* attempt to re-connect */
				char url[IOT_MQTT_URL_MAX + 1u];
				PAHO_OBJ( _connectOptions ) conn_opts =
					PAHO_OBJ( _connectOptions_initializer );
				/* ssl */
				PAHO_OBJ( _SSLOptions ) ssl_opts =
					PAHO_OBJ( _SSLOptions_initializer );

				if ( ssl_conf && port != 1883u )
					os_snprintf( url, IOT_MQTT_URL_MAX,
						"ssl://%s:%d", host, port );
				else
					os_snprintf( url, IOT_MQTT_URL_MAX,
						"tcp://%s:%d", host, port );
				url[ IOT_MQTT_URL_MAX ] = '\0';

				conn_opts.keepAliveInterval = IOT_MQTT_KEEP_ALIVE;
				conn_opts.cleansession = 1;
				conn_opts.username = username;
				conn_opts.password = password;

				if ( ssl_conf && port != 1883u )
				{ /* ssl */
					/* ssl args */
					ssl_opts.trustStore = ssl_conf->ca_path;
					ssl_opts.enableServerCertAuth =
						!ssl_conf->insecure;

					/* client cert and key */
					ssl_opts.keyStore = ssl_conf->cert_file;
					ssl_opts.privateKey = ssl_conf->key_file;

					conn_opts.ssl = &ssl_opts;
				}

				if ( max_time_out > 0u )
					conn_opts.connectTimeout =
						(max_time_out /
						IOT_MILLISECONDS_IN_SECOND) + 1u;

				if ( PAHO_OBJ( _connect )( mqtt->client, &conn_opts )
					== PAHO_RES( _SUCCESS ) )
				{
					mqtt->connected = IOT_TRUE;
					mqtt->connection_changed = IOT_FALSE;
					mqtt->time_stamp_connection_changed =
						iot_timestamp_now();
					mqtt->reconnect_count = 0u;
					result = IOT_STATUS_SUCCESS;
				}
				else
					++mqtt->reconnect_count;
			}
		}
#endif /* else IOT_MQTT_MOSQUITTO */
		if( result != IOT_STATUS_SUCCESS )
			os_time_sleep( IOT_MILLISECONDS_IN_SECOND, OS_TRUE );
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

