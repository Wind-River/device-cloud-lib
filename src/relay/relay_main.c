/**
 * @file
 * @brief Main source file for the Wind River IoT relay application
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

#include "relay_main.h"

#include "app_arg.h"                   /* for struct app_arg & functions */
#include "app_config.h"                /* for proxy configuration */
#include "app_path.h"                  /* for path support functions */
#include "iot.h"                       /* for iot_bool_t type */
#include "iot_build.h"                 /* for IOT_NAME_FULL */
#include "iot_mqtt.h"
#include "shared/iot_types.h"          /* for proxy struct */
#include "app_websocket.h"             /* for websocket operation */

#include <os.h>                        /* for os_* functions */
#include <stdarg.h>                    /* for va_list */

/* defines */
/** @brief Key used to initialize a client connection */
#define RELAY_CONNECTION_KEY       "CONNECTED-129812"

/** @brief Default host to use for connections */
#define RELAY_DEFAULT_HOST         "127.0.0.1"

/** @brief Websocket receive buffer size */
#define RELAY_BUFFER_SIZE          10240u

/** @brief Maximum address length */
#define RELAY_MAX_ADDRESS_LEN      256u

/** @brief Log prefix for debugging */
#define LOG_PREFIX                 "RELAY CLIENT: "

/** @brief Log timestamp max length */
#define RELAY_LOG_TIMESTAMP_LEN    16u

/* structures */
/** @brief relay state */
enum relay_state
{
	RELAY_STATE_CONNECT = 0,    /**< @brief socket not connected */
	RELAY_STATE_BIND,           /**< @brief socket needs binding */
	RELAY_STATE_CONNECTED,      /**< @brief socket connected */
	RELAY_STATE_BOUND           /**< @brief socket bound */
};

/** @brief Structure that contains information for forwarding data */
struct relay_data
{
	app_websocket_t *ws;        /**< @brief websocket for connections */
	os_socket_t *socket;        /**< @brief socket for connections */
	enum relay_state state;     /**< @brief connection state */
	unsigned char *tx_buffer;   /**< @brief buffer for data to forward */
	size_t tx_buffer_size;      /**< @brief transmit buffer size */
	size_t tx_buffer_len;       /**< @brief amount of data on buffer */
	iot_bool_t udp;             /**< @brief UDP packets instead of TCP */
	iot_bool_t verbose;         /**< @brief whether in verbose mode */
};

/** @brief Structure containing the textual representations of log levels */
static const char *const RELAY_LOG_LEVEL_TEXT[] =
{
	"FATAL",
	"ALERT",
	"CRITICAL",
	"ERROR",
	"WARNING",
	"NOTICE",
	"INFO",
	"DEBUG",
	"TRACE",
	"ALL"
};

/* TODO: ideally we get rid of this global declaration but we need it for
 * backwards compatibility with websocket libraries */
#if (defined( IOT_WEBSOCKET_CIVETWEB ) && (CIVETWEB_VERSION_MAJOR < 1 || \
	CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR < 10)) \
    || (!defined( IOT_WEBSOCKET_CIVETWEB ) && ( LWS_LIBRARY_VERSION_MAJOR < 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 4 ) ))
static struct relay_data APP_DATA;
#endif

/* function definitions */
/**
 * @brief Contains main code for the client
 *
 * @param[in]      url                 url to connect to via websocket
 * @param[in]      host                host to open to port on
 * @param[in]      port                port to open
 * @param[in]      udp                 whether to accept udp or tcp packets
 * @param[in]      bind                whether to bind to opened port
 * @param[in]      config_file         configuration file to use
 * @param[in]      insecure            accept self-signed certificates
 * @param[in]      verbose             output in verbose mode on stdout
 * @param[in]      notification_file   file to write connection status in
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
static int relay_client( const char *url,
	const char *host, os_uint16_t port, iot_bool_t udp, iot_bool_t bind,
	const char *config_file, iot_bool_t insecure, iot_bool_t verbose,
	const char * notification_file );

/**
 * @brief Callback function to handle receiving of data
 *
 * @param[in,out]  user_data           pointer to internal application data
 * @param[in]      data                incoming data
 * @param[in]      data_len            size of incoming data
 *
 * @retval 0       data successfully handled
 * @retval -1      error handling data
 */
static int relay_on_receive( void *user_data,
	void *data, size_t data_len );

/**
 * @brief Callback function to write data to websocket
 *
 * @param[in,out]  user_data           pointer to internal application data
 *
 * @retval 0       when the connection has been closed
 * @retval -1      on error
 * @retavl  >0     number of bytes written on success
 */
static int relay_on_writeable( void *user_data );

/**
 * @brief Callback function to be called when websocket closes
 *
 * @param[in,out]  user_data           pointer to internal application data
 */
static void relay_on_close( void *user_data );

/**
 * @brief Signal handler called when a signal occurs on the process
 *
 * @param[in]      signum              signal number that triggered handler
 */
static void relay_signal_handler( int signum );

/**
 * @brief Redirect output to a file for logging purposes
 *
 * @param    path                      path to the log file to write to
 *
 * @retval   OS_STATUS_SUCCESS        log file opened correctly
 * @retval   OS_STATUS_FAILURE        log file could not be opened
 * @retval   OS_STATUS_BAD_PARAMETER  an invalid path was specified
 */
os_status_t relay_setup_file_log( const char *path );

/**
 * @brief Log data to stderr with prepended timestamp
 *
 * @param[in]    level     log level associated with the message
 * @param[in]    format    printf format string to use for the log
 * @param[in]    ...       items to replace based on @p format
 */
static void relay_log( iot_log_level_t level, const char *format, ... )
	__attribute__ ((format(printf,2,3)));

/**
 * @brief Log handler for libwebsockets
 *
 * @param[in]    level     severity of the log line
 * @param[in]    line      log text to print
 */
void relay_lws_log( int level, const char* line );

/* globals */
extern iot_bool_t TO_QUIT;
/** @brief Flag indicating signal for quitting received */
iot_bool_t TO_QUIT = IOT_FALSE;

/** @brief File/stream to use for logging */
static os_file_t LOG_FILE = NULL;


/* Write a status file once the connectivity has been confirmed so
 * that the device manager and return a status to the cloud.  This
 * will help debug connectivity issues */
int relay_client( const char *url,
	const char *host, os_uint16_t port, iot_bool_t udp, iot_bool_t bind,
	const char *config_file, iot_bool_t insecure, iot_bool_t verbose,
	const char *notification_file )
{
	os_socket_t *socket = NULL;
	os_socket_t *socket_accept = NULL;
	int packet_type = SOCK_STREAM;
	int result = EXIT_FAILURE;
	app_websocket_t *ws = NULL;


	/* TODO: remove global APP_DATA */
#if (defined( IOT_WEBSOCKET_CIVETWEB ) && (CIVETWEB_VERSION_MAJOR > 1 || \
	CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR > 10)) \
    || (!defined( IOT_WEBSOCKET_CIVETWEB ) && ( LWS_LIBRARY_VERSION_MAJOR > 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 4 ) ))
	struct relay_data APP_DATA;
#endif /* if civetweb > 1.10 || libwebsockets >= 1.4 */
	struct relay_data *const app_data = &APP_DATA;
	struct app_websocket_protocol protocol;

	(void)config_file;

	/* print client configuration */
	relay_log(IOT_LOG_INFO, "host:     %s", host );
	relay_log(IOT_LOG_INFO, "port:     %u", port );
	relay_log(IOT_LOG_INFO, "bind:     %s",
		( bind == IOT_FALSE ? "false" : "true" ) );
	relay_log(IOT_LOG_INFO, "protocol: %s",
		( udp == IOT_FALSE ? "tcp" : "udp" ) );
	relay_log(IOT_LOG_INFO, "insecure: %s",
		( insecure == IOT_FALSE ? "false" : "true" ) );
	relay_log(IOT_LOG_INFO, "verbose:  %s",
		( verbose == IOT_FALSE ? "false" : "true" ) );
	relay_log(IOT_LOG_INFO, "notification_file:  %s",
		notification_file );

	/* support UDP packets */
	if ( udp != IOT_FALSE )
		packet_type = SOCK_DGRAM;

	/* setup socket */
	os_memzero( app_data, sizeof( struct relay_data ) );
	app_data->udp = udp;
	app_data->verbose = verbose;

	app_websocket_set_log_level_with_logger( verbose, &relay_lws_log );

	if ( os_socket_open( &socket, host, (os_uint16_t)port, packet_type, 0, 0u )
		== OS_STATUS_SUCCESS )
	{
		if ( verbose != IOT_FALSE )
			relay_log( IOT_LOG_DEBUG, "%s",
				"socket opened successfully" );

		if ( bind != IOT_FALSE )
		{
			os_status_t retval;
			app_data->state = RELAY_STATE_BIND;
			/* setup socket as a server */
			retval = os_socket_bind( socket, 1u );
			if ( retval == OS_STATUS_SUCCESS )
			{
				/* wait for an incoming connection */
				if ( os_socket_accept( socket,
					&socket_accept, 0u ) == OS_STATUS_SUCCESS )
				{
					result = EXIT_SUCCESS;
					app_data->socket = socket_accept;
					app_data->state = RELAY_STATE_BOUND;
				}
				else
					relay_log( IOT_LOG_FATAL,
						"Failed to accept incoming connection. "
						"Reason: %s",
						os_system_error_string(
							os_system_error_last() ) );
			}
			else
				relay_log( IOT_LOG_FATAL, "Failed to bind to socket; "
					"Reason: %s", os_system_error_string(
						os_system_error_last() ) );
		}
		else
		{
			app_data->socket = socket;
			app_data->state = RELAY_STATE_CONNECT;
			result = EXIT_SUCCESS;
		}
	}
	else
	{
		relay_log( IOT_LOG_FATAL, "Failed to create socket!" );
		result = EXIT_FAILURE;
	}

	if ( result == EXIT_SUCCESS )
	{
		char web_url[ PATH_MAX + 1u ];
		const char *web_address = NULL;
		const char *web_path = NULL;
		const char *web_protocol = NULL;
		int web_port = 0;

		/* allocate transmission buffer */
		app_data->tx_buffer = (unsigned char *)os_malloc(
			sizeof( char ) * (
			SEND_BUFFER_PRE_PADDING + RELAY_BUFFER_SIZE +
			SEND_BUFFER_POST_PADDING ) );
		if ( !app_data->tx_buffer )
		{
			relay_log( IOT_LOG_FATAL,
				"Failed to allocate transmission buffer" );
			result = EXIT_FAILURE;
		}
		app_data->tx_buffer_size = RELAY_BUFFER_SIZE;

		/* set websocket protocol, namely the callbacks */
		protocol.name = "relay";
		protocol.rx_buffer_size = 10240;
		protocol.on_receive = relay_on_receive;
		protocol.on_writeable = relay_on_writeable;
		protocol.on_close = relay_on_close;

		/* initialize websocket with provided protocol */
		ws = app_websocket_initialize( &protocol );
		if ( ws != NULL )
		{
			app_data->ws = ws;
			app_websocket_set_user_data( app_data->ws, app_data );
			/* client connection info */
			os_strncpy( web_url, url, PATH_MAX );
			result = app_websocket_parse_uri( web_url, &web_protocol,
				&web_address, &web_port, &web_path );
		}
		else
		{
			relay_log( IOT_LOG_FATAL, "Failed to initialize WebSocket!" );
			result = EXIT_FAILURE;
		}

		if ( result == EXIT_SUCCESS )
		{
			struct app_websocket_ci connect_in;
			char *web_path_heap = NULL;
			const size_t web_path_len = os_strlen( web_path );

			/* enable ssl support */
			if ( web_protocol &&
				( web_port == 443 ||
				os_strncmp( web_protocol, "wss", 3 ) == 0 ||
				os_strncmp( web_protocol, "https", 5 ) == 0 ) )
			{
				if ( app_data->verbose )
					relay_log( IOT_LOG_DEBUG, "%s",
						"Setting SSL connection options" );
			}

			/* ensure web path begins with a forward slash ('/') */
			web_path_heap = (char*)os_malloc( web_path_len + 2u );
			if ( web_path_heap )
			{
				*web_path_heap = '/';
				os_strncpy( &web_path_heap[1], web_path,
					web_path_len );
				web_path_heap[ web_path_len + 1u ] = '\0';
			}

			if ( app_data->verbose != IOT_FALSE )
			{
				relay_log( IOT_LOG_DEBUG, "protocol: %s", web_protocol );
				relay_log( IOT_LOG_DEBUG, "address:  %s", web_address );
				relay_log( IOT_LOG_DEBUG, "path:     %s", web_path_heap );
				relay_log( IOT_LOG_DEBUG, "port:     %d", web_port );
			}

			connect_in.web_addr = web_address;
			connect_in.port = web_port;
			connect_in.origin_addr = web_address;
			connect_in.path = web_path_heap;
			connect_in.is_secure = !insecure;
			result = app_websocket_connect( ws, &connect_in );

			if ( result == EXIT_SUCCESS )
			{
				/* wait here for the callback states to complete.
				 * Both local socket and relay sides need to be
				 * CONNECTED */
				int wait = 0;
				while ( wait == 0 && app_data->state !=
					RELAY_STATE_CONNECTED )
				{
					if ( TO_QUIT != IOT_FALSE )
					{
						relay_log( IOT_LOG_FATAL,
							"Connection failure, state=%d",
							app_data->state );
						wait = 1;
						result = EXIT_FAILURE;
					}
					app_websocket_poll( ws, 50u );
				}

				relay_log( IOT_LOG_INFO,
					"Connected status %d", result );

				while ( result == EXIT_SUCCESS && TO_QUIT == IOT_FALSE )
				{
					if ( app_data->state == RELAY_STATE_CONNECTED ||
						app_data->state == RELAY_STATE_BOUND )
					{
						size_t rx_len = 0u;
						const os_status_t rx_result = os_socket_read(
							app_data->socket,
							&app_data->tx_buffer[ SEND_BUFFER_PRE_PADDING + app_data->tx_buffer_len ],
							app_data->tx_buffer_size - app_data->tx_buffer_len,
							&rx_len,
							20u );
						if ( rx_result == OS_STATUS_SUCCESS && rx_len > 0u )
						{
							if ( app_data->verbose )
							{
								relay_log( IOT_LOG_DEBUG, "%s Rx: %lu\n",
									(udp == IOT_FALSE ? "TCP" : "UDP"),
									(unsigned long)rx_len );
							}
							app_data->tx_buffer_len += (size_t)rx_len;
						}
						else if ( rx_result == OS_STATUS_TRY_AGAIN )
						{
							relay_log( IOT_LOG_FATAL,
								"Failed to read from socket" );
							TO_QUIT = IOT_TRUE;
						}
					}

					if ( app_data->tx_buffer_len > 0u )
					{
						app_websocket_callback_on_writeable( ws );
						app_websocket_poll( ws, 50u );
					}
					else
						app_websocket_poll( ws, 50u );
				}
			}
			else
			{
				relay_log( IOT_LOG_FATAL, "%s",
					"Failed to connect to client" );
			}

			os_free_null( (void**)&web_path_heap );
			app_websocket_destroy( ws );
		}
		else
			relay_log( IOT_LOG_FATAL, "Failed to parse url: %s", url );

		app_data->tx_buffer_len = app_data->tx_buffer_size = 0;
		os_free_null( (void **)&app_data->tx_buffer );
	}
	os_socket_close( socket_accept );
	os_socket_close( socket );
	return result;
}

int relay_on_receive( void *user_data,
	void *data, size_t data_len )
{
	int result = 0;
	struct relay_data *app_data = (struct relay_data *)user_data;
#if (defined( IOT_WEBSOCKET_CIVETWEB ) && (CIVETWEB_VERSION_MAJOR < 1 || \
	CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR <= 10)) \
    || (!defined( IOT_WEBSOCKET_CIVETWEB ) && ( LWS_LIBRARY_VERSION_MAJOR < 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 4 ) ))
	app_data = &APP_DATA;
#endif
	if ( app_data )
	{
		if ( app_data->verbose )
		{
			relay_log( IOT_LOG_DEBUG, "WS  Rx: %u",
				(unsigned int)data_len );
		}
		if ( data && data_len > 0u )
		{
			static size_t connection_key_len = 0u;
			os_status_t retval;
			if ( connection_key_len == 0u )
				connection_key_len =
					os_strlen( RELAY_CONNECTION_KEY );

			/* if the relay-client is not connected, then connect */
			/* ie. connect as a "fake client" to a tcp socket */
			if ( app_data->state == RELAY_STATE_CONNECT )
			{
				retval = os_socket_connect( app_data->socket );
				if ( retval == OS_STATUS_SUCCESS &&
					app_data->socket )
					app_data->state = RELAY_STATE_CONNECTED;
				else
				{
					relay_log( IOT_LOG_FATAL,
						"Failed to connect to socket. "
						"Reason: %s",
						os_system_error_string(
							os_system_error_last() ) );
					result = -1;
					TO_QUIT = IOT_TRUE;
				}
			}

			/* pass data that is not a key */
			if ( app_data->socket && ( data_len != connection_key_len ||
				os_strncmp( (const char *)data,
					RELAY_CONNECTION_KEY,
					connection_key_len ) != 0) )
			{
				size_t bytes_written = 0u;
				retval = os_socket_write( app_data->socket, data,
					data_len, &bytes_written, 0u );
				if (bytes_written > 0u && app_data->verbose)
				{
					relay_log( IOT_LOG_DEBUG, "%s Tx: %u",
						(app_data->udp == IOT_FALSE ? "TCP" : "UDP"),
						(unsigned int)data_len );
				}
			}
		}
	}
	return result;
}

int relay_on_writeable( void *user_data )
{
	int result = 0;
	struct relay_data *app_data = (struct relay_data *)user_data;
	app_websocket_t* ws = NULL;

#if (defined( IOT_WEBSOCKET_CIVETWEB ) && (CIVETWEB_VERSION_MAJOR < 1 || \
	CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR <= 10)) \
    || (!defined( IOT_WEBSOCKET_CIVETWEB ) && ( LWS_LIBRARY_VERSION_MAJOR  < 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 4 ) ))
	app_data = &APP_DATA;
#endif
	ws = app_data->ws;
	if ( app_data && app_data->tx_buffer_len )
	{
		result = app_websocket_write( ws,
			&app_data->tx_buffer[ SEND_BUFFER_PRE_PADDING ],
			app_data->tx_buffer_len );
		if ( result > 0 )
		{
			if ( app_data->verbose )
				relay_log( IOT_LOG_DEBUG, "WS  Tx: %u",
					(unsigned int)result );

			/* if complete buffer not written
			 * then move the remaining up */
			if ( (size_t)result != app_data->tx_buffer_len )
				os_memmove(
					&app_data->tx_buffer[SEND_BUFFER_PRE_PADDING],
					&app_data->tx_buffer[SEND_BUFFER_PRE_PADDING + (unsigned int)result],
					app_data->tx_buffer_len - (size_t)result
				);
			app_data->tx_buffer_len -= (size_t)result;
		}
	}
	else if ( app_data->tx_buffer_len == 0 )
	{
		/* lws may generate its own LWS_CALLBACK_CLIENT_WRITEABLE events */
		/* on its own, so we can't assume that just because the writeable callback */
		/* came we actually have data to send - see LWS docs for more info */
		result = 1;
	}

	return result;
}

void relay_on_close( void *UNUSED(user_data) )
{
	TO_QUIT = IOT_TRUE;
}


void relay_signal_handler( int UNUSED(signum) )
{
	relay_log( IOT_LOG_NOTICE, "Received signal, Quitting..." );
	TO_QUIT = IOT_TRUE;
}

void relay_lws_log( int level, const char* line )
{
#if !defined( IOT_WEBSOCKET_CIVETWEB )
	char line_out[RELAY_BUFFER_SIZE + 1u];
	char *newline;
	iot_log_level_t iot_level = IOT_LOG_ALL;

	switch( level )
	{
		case LLL_INFO:
			iot_level = IOT_LOG_INFO;
			break;
		case LLL_DEBUG:
			iot_level = IOT_LOG_DEBUG;
			break;
		case LLL_NOTICE:
			iot_level = IOT_LOG_NOTICE;
			break;
		case LLL_WARN:
			iot_level = IOT_LOG_WARNING;
			break;
		case LLL_ERR:
			iot_level = IOT_LOG_ERROR;
			break;
	}

	os_strncpy( line_out, line, RELAY_BUFFER_SIZE );
	line_out[RELAY_BUFFER_SIZE] = '\0';

	newline = os_strchr( line, '\n' );
	if( newline )
		*newline = '\0';

	relay_log( iot_level, "libwebsockets: %s", line );
#elif defined( IOT_WEBSOCKET_CIVETWEB )
	relay_log( (iot_log_level_t) level, "civetweb: %s", line );
#endif
}

void relay_log( iot_log_level_t level, const char *format, ... )
{
	char timestamp[ RELAY_LOG_TIMESTAMP_LEN ];
	va_list args;
	os_timestamp_t t;

	os_time( &t, NULL );
	os_time_format( timestamp, RELAY_LOG_TIMESTAMP_LEN,
		"%Y-%m-%dT%H:%M:%S", t, IOT_FALSE );

	os_fprintf( LOG_FILE, "%s - [%s] %s",
		timestamp, RELAY_LOG_LEVEL_TEXT[level], LOG_PREFIX );

	va_start( args, format );
	os_vfprintf( LOG_FILE, format, args );
	va_end( args );
	os_fprintf( LOG_FILE, OS_FILE_LINE_BREAK );

}

os_status_t relay_setup_file_log( const char *path )
{

	(void) path;
	/*os_file_t log_file;*/
	/*int flags = OS_WRITE;*/
	/*os_status_t result = OS_STATUS_FAILURE;*/

	/*result = app_path_create( path, RELAY_DIR_CREATE_TIMEOUT );*/

	/*if( result != OS_STATUS_SUCCESS )*/
	/*relay_log( IOT_LOG_ERROR, "Failed to create path %s", path );*/
	/*else*/
	/*{*/
	/*if( os_file_exists( path ) == IOT_FALSE )*/
	/*flags = flags | OS_CREATE;*/

	/*log_file = os_file_open( path, flags );*/

	/*if( log_file == NULL )*/
	/*{*/
	/*relay_log( IOT_LOG_ERROR, "%s failed to open!", path );*/
	/*result = OS_STATUS_FAILURE;*/
	/*}*/
	/*else*/
	/*LOG_FILE = log_file;*/
	/*}*/

	/*return result;*/
	return OS_STATUS_SUCCESS;
}

/* main entry point function */
int relay_main( int argc, char *argv[] )
{
	int result;
	const char *config_file = NULL;
	const char *port_str = NULL;
	const char *host = NULL;
	const char *log_file_path = NULL;
	const char *notification_file = NULL;
	int url_pos = 0;

	struct app_arg args[] = {
		{ 'p', "port", APP_ARG_FLAG_REQUIRED,
			"port", &port_str, "port to connect to", 0u },
		{ 'b', "bind", APP_ARG_FLAG_OPTIONAL, NULL, NULL,
			"bind to the specified socket", 0u },
		{ 'c', "configure", APP_ARG_FLAG_OPTIONAL,
			"file", &config_file, "configuration file", 0u },
		{ 'h', "help", APP_ARG_FLAG_OPTIONAL, NULL, NULL,
			"display help menu", 0u },
		{ 'i', "insecure", APP_ARG_FLAG_OPTIONAL, NULL, NULL,
			"disable certificate validation", 0u },
		{ 'n', "notification", APP_ARG_FLAG_OPTIONAL,
			"file", &notification_file, "notification file", 0u },
		{ 'o', "host", APP_ARG_FLAG_OPTIONAL, "host", &host,
			"host for socket connection", 0u },
		{ 'u', "udp", APP_ARG_FLAG_OPTIONAL, NULL, NULL,
			"UDP packets instead of TCP", 0u },
		{ 'v', "verbose", APP_ARG_FLAG_OPTIONAL, NULL, NULL,
			"verbose output", 0u },
		{ 'f', "log-file", APP_ARG_FLAG_OPTIONAL, "file",
			&log_file_path, "log to the file specified", 0u },
		{ 0, NULL, 0, NULL, NULL, NULL, 0u }
	};

	LOG_FILE = OS_STDERR;

	result = app_arg_parse( args, argc, argv, &url_pos );

	if ( result == EXIT_FAILURE || argv[url_pos] == NULL ||
		app_arg_count( args, 'h', NULL ) )
		app_arg_usage( args, 36u, argv[0], IOT_PRODUCT, "url",
			"remote relay address" );
	else if ( result == EXIT_SUCCESS )
	{
		os_uint16_t port;
		const char *const url = argv[url_pos];
		char host_resolved[ RELAY_MAX_ADDRESS_LEN + 1u ];

		if ( host == NULL || *host == '\0' )
			host = RELAY_DEFAULT_HOST;

		port = (os_uint16_t)os_atoi( port_str );

		if( log_file_path )
		{
			if( relay_setup_file_log( log_file_path ) != OS_STATUS_SUCCESS )
				result = EXIT_FAILURE;
			else
				result = EXIT_SUCCESS;
		}


		if( result == EXIT_SUCCESS )
		{
			/* setup signal handler */
			os_terminate_handler( relay_signal_handler );

			/* initialize websockets */
			os_socket_initialize();

			if ( os_get_host_address ( host, port_str, host_resolved,
				RELAY_MAX_ADDRESS_LEN, AF_INET ) == 0 )
			{
				host_resolved[ RELAY_MAX_ADDRESS_LEN ] = '\0';

				result = relay_client( url, host_resolved, port,
					(iot_bool_t)app_arg_count( args, 'u', NULL ),
					(iot_bool_t)app_arg_count( args, 'b', NULL ),
					config_file,
					(iot_bool_t)app_arg_count( args, 'i', NULL ),
					(iot_bool_t)app_arg_count( args, 'v', NULL ),
					notification_file );
			}
			else
			{
				relay_log( IOT_LOG_FATAL, "Could not resolve host %s", host );
				result = EXIT_FAILURE;
			}
		}

	}

	/* terminate websockets */
	os_socket_terminate();

	/* close log file */
	os_file_close( LOG_FILE );

	return result;
}

