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

#include <stdarg.h>
#include "relay_main.h"

#pragma warning(push, 0)
#include <libwebsockets.h>             /* for libwebsockets header files */
#pragma warning(pop)

#include "iot_build.h"                 /* for IOT_NAME_FULL */
#include <os.h>                        /* for os_* functions */
#include "iot.h"                       /* for iot_bool_t type */
#include "shared/iot_types.h"          /* for proxy struct */
#include "app_arg.h"                   /* for struct app_arg & functions */
/*#include "app-common/app_config.h"     *//* for proxy configuration */
/*#include "app-common/app_path.h"       *//* for path support functions */

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
#define LOG_PREFIX "RELAY CLIENT: "

/** @brief Log timestamp max length */
#define RELAY_LOG_TIMESTAMP_LEN    16u

/* libwebsockets backwards support defines */
#if LWS_LIBRARY_VERSION_MAJOR < 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 )
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
#if LWS_LIBRARY_VERSION_MAJOR < 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 7 )
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
static int lws_parse_uri( char *p, const char **prot, const char **ads,
	int *port, const char **path )
{
	int result = EXIT_FAILURE;
	if ( p )
	{
		static const char *slash = "/";
		char *start = NULL;
		const char *port_str = NULL;

		start = p;

		while ( p[0] && ( p[0] != ':' || p[1] != '/' || p[2] != '/' ) )
			++p;

		if ( *p != '\0' ) {
			*p = '\0';
			p += 3;
			if ( prot )
				*prot = start;
		} else {
			if ( prot )
				*prot = p;
			p = start;
		}

		if ( ads )
			*ads = p;

		while ( *p != '\0' && *p != ':' && *p != '/' )
			++p;

		if ( *p == ':' )
		{
			*p = '\0';
			++p;
			port_str = p;
		}

		while ( *p != '\0' && *p != '/' )
			++p;

		if ( *p != '\0' )
		{
			*p = '\0';
			++p;
			if ( path )
				*path = p;
		}
		else if ( path )
			*path = slash;

		if ( port )
		{
			*port = 0;
			if ( prot )
			{
				if ( os_strcmp( *prot, "http" ) == 0 ||
					os_strcmp( *prot, "ws" ) == 0 )
					*port = 80;
				else if ( os_strcmp( *prot, "https" ) == 0 ||
					os_strcmp( *prot, "wss" ) == 0 )
					*port = 443;
			}
			if ( port_str )
				*port = os_atoi( port_str );
		}

		result = EXIT_SUCCESS;
	}
	return result;
}
#endif /* libwebsockets < 1.7.0 */

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
	os_socket_t *socket;    /**< @brief socket for connections */
	enum relay_state state;     /**< @brief connection state */
	unsigned char *tx_buffer;   /**< @brief buffer for data to forward */
	size_t tx_buffer_size;      /**< @brief transmit buffer size */
	size_t tx_buffer_len;       /**< @brief amount of data on buffer */
	iot_bool_t udp;             /**< @brief UDP packets instead of TCP */
	iot_bool_t verbose;         /**< @brief whether in verbose mode */
};

#if LWS_LIBRARY_VERSION_MAJOR < 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 4 )
static struct relay_data WSD;
#endif

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
 * @brief Callback called by libwebsockets
 *
 * @param[in,out]  wsi                 pointer to libwebsocket interface
 * @param[in]      reason              reason callback was triggered
 * @param[in,out]  user                pointer to user allocated space
 * @param[in]      in                  incoming data
 * @param[in]      len                 size of incoming data
 *
 * @retval 0       callback succesfully handled
 */
static int relay_service_callback( struct lws *wsi,
	enum lws_callback_reasons reason, void *user, void *in, size_t len );

#if LWS_LIBRARY_VERSION_MAJOR < 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 )
/**
 * @brief Callback called by older versions of libwebsockets
 *
 * @param[in]      context             libwebsocket initialization context
 * @param[in,out]  wsi                 pointer to libwebsocket interface
 * @param[in]      reason              reason callback was triggered
 * @param[in,out]  user                pointer to user allocated space
 * @param[in]      in                  incoming data
 * @param[in]      len                 size of incoming data
 *
 * @retval 0       callback succesfully handled
 */
static int relay_service_callback_old( struct lws_context *context,
	struct lws *wsi, enum lws_callback_reasons reason, void *user,
	void *in, size_t len );
#endif /* libwebsockets < 1.6.0 */

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
void relay_log( iot_log_level_t level, const char *format, ... ) __attribute__ ((format(printf,2,3)));

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

#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 7 )
/**
 * @brief Libwebsocket extensions, required for SSL connections to
 * prevent crash
 */
static const struct lws_extension EXTS[] = {
	{
		"permessage-deflate",
		lws_extension_callback_pm_deflate,
		"permessage-deflate; client_max_window_bits"
	},
	{
		"deflate-frame",
		lws_extension_callback_pm_deflate,
		"deflate_frame"
	},
	{ NULL, NULL, NULL /* terminator */ }
};
#endif /* libwebsockets >= 1.7.0 */

/* function implementations */
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

#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 4 )
	struct relay_data WSD;
#endif
	struct relay_data *const wsd = &WSD;

	/*FIXME*/
	(void)config_file;

	/* print client configuration */
	relay_log(IOT_LOG_INFO, "%s url:      %s", LOG_PREFIX, url );
	relay_log(IOT_LOG_INFO, "%s host:     %s", LOG_PREFIX, host );
	relay_log(IOT_LOG_INFO, "%s port:     %u", LOG_PREFIX, port );
	relay_log(IOT_LOG_INFO, "%s bind:     %s", LOG_PREFIX,
		( bind == IOT_FALSE ? "false" : "true" ) );
	relay_log(IOT_LOG_INFO, "%s protocol: %s", LOG_PREFIX,
		( udp == IOT_FALSE ? "tcp" : "udp" ) );
	relay_log(IOT_LOG_INFO, "%s insecure: %s", LOG_PREFIX,
		( insecure == IOT_FALSE ? "false" : "true" ) );
	relay_log(IOT_LOG_INFO, "%s verbose:  %s", LOG_PREFIX,
		( verbose == IOT_FALSE ? "false" : "true" ) );
	relay_log(IOT_LOG_INFO, "%s notification_file:  %s", LOG_PREFIX,
		notification_file );

	/* support UDP packets */
	if ( udp != IOT_FALSE )
		packet_type = SOCK_DGRAM;

	/* setup socket */
	os_memzero( wsd, sizeof( struct relay_data ) );
	wsd->udp = udp;
	wsd->verbose = verbose;
	if ( verbose != IOT_FALSE )
		lws_set_log_level( LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO |
				   LLL_DEBUG | LLL_CLIENT, &relay_lws_log );
	else
		lws_set_log_level( LLL_ERR | LLL_WARN | LLL_NOTICE, &relay_lws_log );
	if ( os_socket_open( &socket, host, (os_uint16_t)port, packet_type, 0, 0u )
		== OS_STATUS_SUCCESS )
	{
		if ( verbose != IOT_FALSE )
			relay_log(IOT_LOG_DEBUG, 
				"%s socket opened successfully", LOG_PREFIX );

		if ( bind != IOT_FALSE )
		{
			os_status_t retval;
			wsd->state = RELAY_STATE_BIND;
			/* setup socket as a server */
			retval = os_socket_bind( socket, 1u );
			if ( retval == OS_STATUS_SUCCESS )
			{
				/* wait for an incoming connection */
				if ( os_socket_accept( socket,
					&socket_accept, 0u ) == OS_STATUS_SUCCESS )
				{
					result = EXIT_SUCCESS;
					wsd->socket = socket_accept;
					wsd->state = RELAY_STATE_BOUND;
				}
				else
					relay_log( IOT_LOG_FATAL,
						"Failed to accept incoming connection.  Reason: %s",
						iot_error(IOT_STATUS_FAILURE) );
			}
			else
				relay_log( IOT_LOG_FATAL, "Failed to bind to socket; Reason: %s",
					iot_error(IOT_STATUS_FAILURE) );
		}
		else
		{
			wsd->socket = socket;
			wsd->state = RELAY_STATE_CONNECT;
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
		char cert_path[ PATH_MAX + 1u ];
		/*struct app_config* conf;*/
		struct lws_context *context = NULL;
		struct lws_context_creation_info context_ci;
		struct lws_protocols protocols[ 2u ];
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 3 )
		/*struct iot_proxy proxy_info;*/
#endif /* libwebsockets >= 1.3.0 */
		char web_url[ PATH_MAX + 1u ];
		const char *web_address = NULL;
		const char *web_path = NULL;
		const char *web_protocol = NULL;
		int web_port = 0;

		wsd->tx_buffer = (unsigned char *)os_malloc(
			sizeof( char ) * (
			LWS_SEND_BUFFER_PRE_PADDING + RELAY_BUFFER_SIZE +
			LWS_SEND_BUFFER_POST_PADDING ) );
		if ( !wsd->tx_buffer )
		{
			relay_log( IOT_LOG_FATAL, "Failed to allocate transmission buffer" );
			result = EXIT_FAILURE;
		}

		if ( result == EXIT_SUCCESS )
		{
			wsd->tx_buffer_size = RELAY_BUFFER_SIZE;

			/* protocol */
			os_memzero( &protocols[0u], sizeof( struct lws_protocols ) * 2u );
			protocols[0].name = "relay";
			protocols[0].per_session_data_size = 0;
			protocols[0].rx_buffer_size = RELAY_BUFFER_SIZE;
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 6 )
			protocols[0].callback = relay_service_callback;
#else /* libwebsockets < 1.6.0 */
			protocols[0].callback = relay_service_callback_old;
#endif
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 4 )
			protocols[0].user = wsd;
#endif /* libwebsockets >= 1.4.0 */

			/* set ca certificate directory */
			os_strncpy( cert_path, IOT_DEFAULT_CERT_PATH, PATH_MAX );
			/*FIXME*/
			/*conf = app_config_open( NULL, config_file );*/
			/*if ( conf )*/
			/*{*/
			/*iot_bool_t ssl_validation = !insecure;*/
			/*app_config_read_boolean( conf, NULL, "ssl_validation",*/
			/*&ssl_validation );*/
			/*insecure = !ssl_validation;*/

			/*app_config_read_string( conf, NULL, "cert_path",*/
			/*cert_path, PATH_MAX );*/
			/*}*/

			/*if ( conf )*/
			/*app_config_close( conf );*/

			/* context creation info */
			os_memzero( &context_ci, sizeof( struct lws_context_creation_info ) );
			context_ci.port = CONTEXT_PORT_NO_LISTEN;
			context_ci.iface = NULL;
			context_ci.protocols = &protocols[0];
			context_ci.extensions = NULL;

			/*FIXME*/
			/*app_path_make_absolute( cert_path, PATH_MAX, IOT_TRUE );*/
			if ( verbose != IOT_FALSE && ( *cert_path ) )
				relay_log( IOT_LOG_DEBUG, "%s CA certificate path: %s\n",
					LOG_PREFIX, cert_path );
			context_ci.ssl_ca_filepath = cert_path;
			context_ci.ssl_cert_filepath = NULL;
			context_ci.ssl_private_key_filepath = NULL;
			context_ci.gid = -1;
			context_ci.uid = -1;
#if LWS_LIBRARY_VERSION_MAJOR > 2 || ( LWS_LIBRARY_VERSION_MAJOR == 2 && LWS_LIBRARY_VERSION_MINOR >= 1 )
			context_ci.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif /* libwebsockets >= 2.1.0 */

#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 3 )
			/* setup proxy connection, if defined */
			/*os_memzero( &proxy_info, sizeof( struct iot_proxy ) );*/
			/*FIXME*/
			/*if ( app_config_read_proxy_file( &proxy_info ) ==*/
			/*OS_STATUS_SUCCESS )*/
			/*{*/
			/*char address[ RELAY_MAX_ADDRESS_LEN + 1u ];*/
			/*size_t address_len = 0;*/
			/**address = '\0';*/
			/*address[ RELAY_MAX_ADDRESS_LEN ] = '\0';*/
			/*if ( *proxy_info.username != '\0' )*/
			/*{*/
			/*os_strncpy( address,*/
			/*proxy_info.username,*/
			/*RELAY_MAX_ADDRESS_LEN - address_len );*/
			/*address_len = os_strlen( address );*/
			/*if ( *proxy_info.password != '\0' )*/
			/*{*/
			/*os_snprintf( &address[address_len],*/
			/*RELAY_MAX_ADDRESS_LEN - address_len,*/
			/*":%s", proxy_info.password );*/
			/*address_len = os_strlen( address );*/
			/*}*/
			/*os_strncpy( &address[address_len], "@",*/
			/*RELAY_MAX_ADDRESS_LEN - address_len );*/
			/*++address_len;*/
			/*}*/
			/*os_strncpy( &address[address_len],*/
			/*proxy_info.host,*/
			/*RELAY_MAX_ADDRESS_LEN - address_len );*/

			/*if ( os_strncmp( proxy_info.type, "HTTP",*/
			/*IOT_PROXY_TYPE_MAX_LEN ) == 0 )*/
			/*{*/
			/*context_ci.http_proxy_address = address;*/
			/*context_ci.http_proxy_port = proxy_info.port;*/
			/*}*/
			/*#if LWS_LIBRARY_VERSION_MAJOR > 2 || ( LWS_LIBRARY_VERSION_MAJOR == 2 && LWS_LIBRARY_VERSION_MINOR > 2 )*/
			/*else if ( os_strncmp( proxy_info.type, "SOCKS5",*/
			/*IOT_PROXY_TYPE_MAX_LEN ) == 0 )*/
			/*{*/
			/*#if defined(LWS_WITH_SOCKS5)*/
			/*context_ci.socks_proxy_address = address;*/
			/*context_ci.socks_proxy_port = proxy_info.port;*/
			/*#else*/
			/*relay_log( IOT_LOG_WARNING,*/
			/*"The libwebsockets doesn't have SOCKS5"*/
			/*" proxy support built in" );*/
			/*#endif *//* if defined(LWS_WITH_SOCKS5) */
			/*}*/
			/*#endif *//* libwebsockets > 2.2.0 */

			/*if ( wsd->verbose != IOT_FALSE && *address != '\0' )*/
			/*relay_log( IOT_LOG_DEBUG,*/
			/*"%s Proxy set to: %s:%ld\n",*/
			/*LOG_PREFIX, address, proxy_info.port );*/
			/*}*/
#endif /* libwebsockets >= 1.3.0 */

			context = lws_create_context( &context_ci );
			if ( context == NULL )
			{
				relay_log( IOT_LOG_FATAL, "Failed to create context" );
				result = EXIT_FAILURE;
			}
			else
			{

				/* client connection info */
				os_strncpy( web_url, url, PATH_MAX );
				result = lws_parse_uri( web_url, &web_protocol,
					&web_address, &web_port, &web_path );
			}
		}

		if ( context != NULL && result == EXIT_SUCCESS )
		{
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 7 )
			struct lws_client_connect_info client_ci;
#endif /* libwebsockets >= 1.7.0 */
			struct lws *lws;
			int ssl_connection = 0;
			char *web_path_heap = NULL;
			const size_t web_path_len = os_strlen( web_path );

			/* ensure web path begins with a forward slash ('/') */
			web_path_heap = (char*)os_malloc( web_path_len + 2u );
			if ( web_path_heap )
			{
				*web_path_heap = '/';
				os_strncpy( &web_path_heap[1], web_path,
					web_path_len );
				web_path_heap[ web_path_len + 1u ] = '\0';
			}

			if ( wsd->verbose != IOT_FALSE )
			{
				relay_log( IOT_LOG_DEBUG, "%s protocol: %s", LOG_PREFIX, web_protocol );
				relay_log( IOT_LOG_DEBUG, "%s address:  %s", LOG_PREFIX, web_address );
				relay_log( IOT_LOG_DEBUG, "%s path:     %s", LOG_PREFIX, web_path_heap );
				relay_log( IOT_LOG_DEBUG, "%s port:     %d", LOG_PREFIX, web_port );
			}

			/* enable ssl support */
			if ( web_protocol &&
				( web_port == 443 ||
				os_strncmp( web_protocol, "wss", 3 ) == 0 ||
				os_strncmp( web_protocol, "https", 5 ) == 0 ) )
			{
				if ( wsd->verbose )
					relay_log( IOT_LOG_DEBUG, "%s Setting SSL connection options",
						LOG_PREFIX );
#if LWS_LIBRARY_VERSION_MAJOR > 2 || ( LWS_LIBRARY_VERSION_MAJOR == 2 && LWS_LIBRARY_VERSION_MINOR >= 1 )
				ssl_connection |= LCCSCF_USE_SSL;
				if ( insecure )
					ssl_connection |= LCCSCF_ALLOW_SELFSIGNED |
#if LWS_LIBRARY_VERSION_MAJOR > 2 || ( LWS_LIBRARY_VERSION_MAJOR == 2 && LWS_LIBRARY_VERSION_MINOR >= 2 )
						LCCSCF_ALLOW_EXPIRED |
#endif /* libwebsockets >= 2.2.0 */
						LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
#else /* libwebsockets < 2.1.0 */
				if ( insecure )
					ssl_connection = 2;
				else
					ssl_connection = 1;
#endif
			}

#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 7 )
			os_memzero( &client_ci,
				sizeof( struct lws_client_connect_info ) );

			/* build host address */
			client_ci.context = context;
			client_ci.address = web_address; /* address: 127.0.0.1 */
			client_ci.origin = web_address; /* origin: 127.0.0.1 */
			client_ci.protocol = protocols[0].name; /* protocol */
			client_ci.path = web_path_heap; /* path: /ws-relay/123456789 */
			client_ci.port = web_port; /* port: 9000 */
			client_ci.ssl_connection = ssl_connection;
			client_ci.host = web_address; /* host: 127.0.0.1 */
			client_ci.ietf_version_or_minus_one = -1;
			client_ci.client_exts = EXTS;
			lws = lws_client_connect_via_info( &client_ci );
#else /* libwebsockets < 1.7.0 */
			lws = lws_client_connect( context,
				web_address, /* address: 127.0.0.1 */
				web_port, /* port: 9000 */
				ssl_connection, /* ssl */
				web_path_heap, /* path: /ws-relay/123456789 */
				web_address, /* host: 127.0.0.1 */
				web_address, /* origin: 127.0.0.1 */
				protocols[0].name, /* protocol */
				-1 /* ietf_version_or_minus_one */
			);
#endif
			if ( lws )
			{
				/* wait here for the callback states to complete.
				 * Both local socket and relay sides need to be
				 * CONNECTED */
				int wait = 0;
				while ( wait == 0 && wsd->state !=
					RELAY_STATE_CONNECTED)
				{
					if ( TO_QUIT != IOT_FALSE )
					{
						relay_log( IOT_LOG_FATAL, "%s Connection failure, state=%d\n",
							LOG_PREFIX, wsd->state );
						wait = 1;
						result = EXIT_FAILURE;
					}
					lws_service( context, 50 );
				}

				relay_log( IOT_LOG_INFO, "%s relay connected status %d\n",
					LOG_PREFIX, result );

				while ( result >= EXIT_SUCCESS && TO_QUIT == IOT_FALSE )
				{
					if ( wsd->state == RELAY_STATE_CONNECTED ||
						wsd->state == RELAY_STATE_BOUND )
					{
						size_t rx_len = 0u;
						const os_status_t rx_result = os_socket_read(
							wsd->socket,
							&wsd->tx_buffer[ LWS_SEND_BUFFER_PRE_PADDING + wsd->tx_buffer_len ],
							wsd->tx_buffer_size - wsd->tx_buffer_len,
							&rx_len,
							20u );
						if ( rx_result == OS_STATUS_SUCCESS && rx_len > 0u )
						{
							if ( wsd->verbose )
							{
								relay_log( IOT_LOG_DEBUG, "%sd %s Rx: %lu\n",
									LOG_PREFIX,
									(udp == IOT_FALSE ? "TCP" : "UDP"),
									(unsigned long)rx_len );
							}
							wsd->tx_buffer_len += (size_t)rx_len;
						}
						else if ( rx_result == OS_STATUS_TRY_AGAIN )
						{
							relay_log( IOT_LOG_FATAL, "Failed to read from socket" );
							TO_QUIT = IOT_TRUE;
						}
					}

					/*
					 * If libwebsockets sockets are all we care
					 * about, you can use this api which takes care
					 * of the poll() and looping through finding who
					 * needed service.
					 *
					 * If no socket needs service, it'll return
					 * anyway after the number of ms in the second
					 * argument.
					 */
					if ( wsd->tx_buffer_len > 0u )
#if LWS_LIBRARY_VERSION_MAJOR < 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 )
						lws_callback_on_writable( context, lws );
#else /* libwebsockets >= 1.6.0 */
						lws_callback_on_writable( lws );
#endif
					lws_service( context, 50 );
				}
			}
			else
				relay_log( IOT_LOG_FATAL, "Failed to connect to client url: %s", url );

			os_free_null( (void**)&web_path_heap );
			lws_context_destroy( context );
		}
		else
			relay_log( IOT_LOG_FATAL, "Failed to parse url: %s", url );

		wsd->tx_buffer_len = wsd->tx_buffer_size = 0;
		os_free_null( (void **)&wsd->tx_buffer );
	}

	os_socket_close( socket_accept );
	os_socket_close( socket );
	return result;
}

int relay_service_callback( struct lws *wsi,
	enum lws_callback_reasons reason, void *UNUSED(user),
	void *in, size_t len )
{
	int result = 0;
	struct relay_data *wsd = NULL;
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 4 )
	const struct lws_protocols* const protocols = lws_get_protocol( wsi );
	if ( protocols )
		wsd = (struct relay_data *)protocols[0].user;
#else /* libwebsockets < 1.4.0 */
	wsd = &WSD;
#endif
	if ( wsd )
	{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif /* ifdef __clang__ */
		switch ( reason )
		{
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			relay_log( IOT_LOG_FATAL, "Failed to connect to client" );
			if ( in )
				relay_log( IOT_LOG_FATAL, "%s", (const char *)in );
			result = 1;
			break;
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 3 )
		case LWS_CALLBACK_WSI_DESTROY:
#endif /* libwebsockets >= 1.3.0 */
		case LWS_CALLBACK_CLOSED:
			TO_QUIT = IOT_TRUE;
			break;
		case LWS_CALLBACK_CLIENT_RECEIVE:
			if ( wsd->verbose )
			{
				relay_log( IOT_LOG_DEBUG, "WS  Rx: %u",
					(unsigned int)len );
			}
			if ( in && len > 0u )
			{
				static size_t connection_key_len = 0u;
				os_status_t retval;
				if ( connection_key_len == 0u )
					connection_key_len =
						os_strlen( RELAY_CONNECTION_KEY );

				/* if client and not connected, connect here! */
				/* connect as client to tcp socket */
				if ( wsd->state == RELAY_STATE_CONNECT )
				{
					retval = os_socket_connect( wsd->socket );
					if ( retval == OS_STATUS_SUCCESS)
						wsd->state = RELAY_STATE_CONNECTED;
					else
					{
						relay_log( IOT_LOG_FATAL,
							"Failed to connect to socket.  Reason: %s",
							iot_error( IOT_STATUS_FAILURE ) );
						result = -1;
						TO_QUIT = IOT_TRUE;
					}
				}

				/* pass data that is not a key */
				if ( wsd->socket && ( len != connection_key_len ||
					os_strncmp( (const char *)in,
						RELAY_CONNECTION_KEY,
						connection_key_len ) != 0) )
				{
					size_t bytes_written = 0u;
					retval = os_socket_write( wsd->socket, in,
						len, &bytes_written, 0u );
					if (bytes_written > 0u && wsd->verbose)
					{
						relay_log( IOT_LOG_DEBUG, "%s Tx: %u",
							(wsd->udp == IOT_FALSE ? "TCP" : "UDP"),
							(unsigned int)len );
					}
				}
			}
			break;
		case LWS_CALLBACK_CLIENT_WRITEABLE:
			if ( wsd->tx_buffer_len )
			{
				const int write_result =
					lws_write( wsi,
						&wsd->tx_buffer[ LWS_SEND_BUFFER_PRE_PADDING ],
						wsd->tx_buffer_len, LWS_WRITE_BINARY );
				if ( write_result > 0 )
				{
					if ( wsd->verbose )
						relay_log( IOT_LOG_DEBUG, "%s WS  Tx: %u\n",
							 LOG_PREFIX,
							(unsigned int)write_result );

					/* if complete buffer not written
					 * then move the remaining up */
					if ( (size_t)write_result != wsd->tx_buffer_len )
						os_memmove(
							&wsd->tx_buffer[ LWS_SEND_BUFFER_PRE_PADDING ],
							&wsd->tx_buffer[ LWS_SEND_BUFFER_PRE_PADDING + write_result ],
							wsd->tx_buffer_len - (size_t)write_result
						);
					wsd->tx_buffer_len -= (size_t)write_result;
				}
			}
			break;
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			result = 0;
			break;
		default:
			result = 0;
			break;
		}
#ifdef __clang__
#pragma clang diagnostic pop
#endif /* ifdef __clang__ */
	}
	return result;
}

#if LWS_LIBRARY_VERSION_MAJOR < 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 )
int relay_service_callback_old( struct lws_context *UNUSED(context),
	struct lws *wsi, enum lws_callback_reasons reason, void *user,
	void *in, size_t len )
{
	return relay_service_callback( wsi, reason, user, in, len );
}
#endif /* libwebsockets < 1.6.0 */

void relay_signal_handler( int UNUSED(signum) )
{
	relay_log( IOT_LOG_NOTICE, "Received signal, Quitting..." );
	TO_QUIT = IOT_TRUE;
}

void relay_lws_log( int level, const char* line )
{
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
}

void relay_log( iot_log_level_t level, const char *format, ... )
{
	char timestamp[ RELAY_LOG_TIMESTAMP_LEN ];
	va_list args;
	os_timestamp_t t;

	os_time(&t, NULL );
	os_time_format( timestamp, RELAY_LOG_TIMESTAMP_LEN,
		"%Y-%m-%dT%H:%M:%S", t, IOT_FALSE );

	os_fprintf( LOG_FILE, "%s - [%s] ", timestamp, RELAY_LOG_LEVEL_TEXT[level] );

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
	if ( result == EXIT_FAILURE || url_pos == 0 ||
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

