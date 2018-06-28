/**
 * @file
 * @brief Header file for WebSocket operations within the IoT library
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
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

#ifndef APP_WEBSOCKET_H
#define APP_WEBSOCKET_H

#include <iot.h>
#include <os.h>


#if defined( IOT_WEBSOCKET_CIVETWEB )
#	include <civetweb.h>                  /* for civetweb functions */
#	include <openssl/ssl.h>               /* for SSL support */

	/** @brief send buffer pre-padding */
#	define SEND_BUFFER_PRE_PADDING        0u
	/** @brief send buffer post-padding */
#	define SEND_BUFFER_POST_PADDING       0u

#else /* if !defined( IOT_WEBSOCKET_CIVETWEB ) */
#	if defined( __DCC__ )
#		include <libwebsockets.h>     /* for libwebsockets functions */
#	else /* if defined( __DCC__ ) */
#		pragma warning(push, 0)
#		include <libwebsockets.h>     /* for libwebsockets functions */
#		pragma warning(pop)
#	endif /* else if defined( __DCC__ ) */

	/** @brief send buffer pre-padding */
#	define SEND_BUFFER_PRE_PADDING        LWS_SEND_BUFFER_PRE_PADDING
	/** @brief send buffer post-padding */
#	define SEND_BUFFER_POST_PADDING       LWS_SEND_BUFFER_POST_PADDING
#endif /* else if defined( IOT_WEBSOCKET_CIVETWEB ) */

#include "iot_build.h"                 /* for IOT_NAME_FULL */

/* libwebsockets backwards support defines */
#if !defined( IOT_WEBSOCKET_CIVETWEB) && \
	( LWS_LIBRARY_VERSION_MAJOR < 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 ) )
/** @{ */
/** @brief Callbacks to support old naming of functions in libwebsockets */
#	define lws_callback_reasons          libwebsocket_callback_reasons
#	define lws_client_connect            libwebsocket_client_connect
#	define lws_context                   libwebsocket_context
#	define lws_create_context            libwebsocket_create_context
#	define lws_callback_on_writable      libwebsocket_callback_on_writable
#	define lws_service                   libwebsocket_service
#	define lws_context_destroy           libwebsocket_context_destroy
#	define lws_protocols                 libwebsocket_protocols
#	define lws_extension                 libwebsocket_extension
#	define lws_write                     libwebsocket_write
#	define lws                           libwebsocket
/** @} */
#endif /* libwebsockets  < 1.6.0 */

/**	@brief signature of function to be called when data is received */
typedef int (*app_websocket_on_receive_t)(
	void *user_data,
	void *data,
	size_t data_len );

/**	@brief signature of function to be called when websocket is closed */
typedef void (*app_websocket_on_close_t)(
	void *user_data );

/**	@brief signature of function to be called when websocket is
 * 	available for writing */
typedef int (*app_websocket_on_writeable_t)(
	void *user_data );

/** @brief base structure for a WebSocket */
typedef struct app_websocket
{
#if !defined( IOT_WEBSOCKET_CIVETWEB )
	/** @brief libwebsocket connection handle */
	struct lws *connection;
	/** @brief libwebsocket context */
	struct lws_context *context;
#elif defined( IOT_WEBSOCKET_CIVETWEB )
	/** @brief civwetweb connection handle */
	struct mg_connection *connection;
	/** @brief civwetweb error buffer */
	char error_buf[256u];
#endif
	/** @brief user defined connection protocol */
	struct app_websocket_protocol *protocol;
	/** @brief user defined connection related data*/
	void* user_data;
} app_websocket_t;

/** @brief structure used for setting up a WebSocket connection */
struct app_websocket_ci
{
	/** @brief remote address to connect to*/
	const char * web_addr;
	/** @brief content of host header */
	const char * host;
	/** @brief remote port to connect to */
	int port;
	/** @brief content of origin header */
	const char * origin_addr;
	/** @brief boolean for secure connection (SSL) */
	int is_secure;
	/** @brief uri path of websocket */
	const char * path;
};

/** @brief structure used for setting up a WebSocket protocol */
struct app_websocket_protocol
{
	/** @brief identifier for the protocol */
	const char *name;
	/** @brief buffer size for receiving data */
	size_t rx_buffer_size;
	/** @brief user defined callback on receiving data */
	app_websocket_on_receive_t on_receive;
	/** @brief user defined callback on WebSocket closure */
	app_websocket_on_close_t on_close;
	/** @brief user defined callback when WebSocket is available to be written */
	app_websocket_on_writeable_t on_writeable;
};

/**
 * @brief initializes a WebSocket interface
 *
 * @param[in]      protocol            preconfigured protocol used for initialization
 *
 * @retval         NULL                failed to initialize a WebSocket instance
 * @retval         !NULL               successfully initialized a WebSocket instance
 */
app_websocket_t *app_websocket_initialize( struct app_websocket_protocol* protocol  );

/**
 * @brief connects to a WebSocket server
 *
 * @param[in]      ws                  WebSocket instance to connect
 * @param[in]      connect_in          connection options
 *
 * @retval IOT_STATUS_FAILURE          operation failed
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
iot_status_t app_websocket_connect( app_websocket_t *ws,
	struct app_websocket_ci *connect_in  );

/**
 * @brief set the log level and optional logger function
 *
 * @param[in]      is_verbose          boolean for logging verbosity
 * @param[in]      log_emit_function   (optional) user function for logging
 */
void app_websocket_set_log_level_with_logger( int is_verbose,
	void (*log_emit_function)(int level, const char *line) );

/**
 * @brief service specified WebSocket or poll for actions
 *
 * @param[in]      ws                  WebSocket instance
 * @param[in]      timeout             milliseconds for timeout
 */
void app_websocket_poll( app_websocket_t *ws, os_millisecond_t timeout );

/**
 * @brief sets user data to be passed when a callback is called
 *
 * @param[in]      ws                  WebSocket instance
 * @param[in]      user_data           user data to pass to callback
 *
 * @retval IOT_STATUS_NOT_INITIALIZED  non-initialized websocket instance
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
iot_status_t app_websocket_set_user_data( app_websocket_t *ws, void *user_data );

/**
 * @brief write to a specified WebSocket
 *
 * @note this should only be called in an on_writeable callback
 *
 * @param[in]      ws                  WebSocket instance
 * @param[in]      buffer              buffer to be written
 * @param[in]      buffer_len          number of bytes to write
 *
 * @retval         > 0                 number of bytes succesfully written
 * @retval         = 0                 no bytes written (no error)
 * @retval         < 0                 error occurred
 */
int app_websocket_write( app_websocket_t *ws, void * buffer,
	size_t buffer_len );


/**
 * @brief request a writeable callback to be called when the WebSocket
 * is no longer blocked and the connection can accept more data
 *
 * @note this is an lws-specific feature. For other libraries, ws may attempt
 * to write to a non-accepting connection, depending on the defined callback
 *
 * @param[in]      ws                  WebSocket instance
 */
void app_websocket_callback_on_writeable( app_websocket_t *ws );

/**
 * @brief destroy a websocket context and free associated memory
 *
 * @param[in]      ws                  WebSocket instance
 */
void app_websocket_destroy( app_websocket_t *ws );

/* Helper Methods */
/**
 * splits the portions of the uri passed in, into parts
 *
 * @note this is done by dropping '\0' into input string, thus the leading / on
 * the path is consequently lost
 *
 * @param[in]      p                   incoming uri string.. will get written to
 * @param[out]     prot                result pointer for protocol part (https://)
 * @param[out]     ads                 result pointer for address part
 * @param[out]     port                result pointer for port part
 * @param[out]     path                result pointer for path part
 */
 iot_status_t app_websocket_parse_uri( char *p, const char **prot, const char **ads,
	int *port, const char **path );


#endif /* ifndef APP_WEBSOCKET_H */
