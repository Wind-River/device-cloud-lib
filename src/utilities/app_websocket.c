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

#include "app_websocket.h"
#include "app_log.h"

/**
 * configures the SSL connection flags to be used
 * by websocket libraries before connecting
 *
 * @param[in] 	is_secure        boolean for enabling ssl
 *
 * @note: a separate patch is required for civetweb support on
 * civetweb <= 1.10
 */
static int configure_ssl( int is_secure );

#if !defined( IOT_WEBSOCKET_CIVETWEB ) && ( LWS_LIBRARY_VERSION_MAJOR > 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 7 ) )
/**
 * @brief Libwebsocket extensions, required for SSL connections to
 * prevent crash for libwebsockets >= 1.7.0
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

#if !defined( IOT_WEBSOCKET_CIVETWEB )

/**
 * @brief libwebsocket callback to be called on receiving data
 *
 * @param wsi	        websocket interface
 * @param reason        callback reason
 * @param user          user specified data
 * @param in            incoming websocket data
 * @param len           length of incoming websocket data
 *
 * @note: the return values will tell the websocket libraries to close
 * the websocket on a return value of 1
 *
 * @retval 0			callback handled succesfully
 * @retval > 0			close the socket
 *
 */
static int app_websocket_on_receive(
	struct lws *wsi,
	enum lws_callback_reasons reason, void *user,
	void *in, size_t len );

#if LWS_LIBRARY_VERSION_MAJOR < 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 )


/**
 * @brief deprecated libwebsocket callback to be called on receiving data
 *
 * @param context       lws context
 * @param wsi	        websocket connection interface
 * @param reason        callback reason
 * @param user          user specified data
 * @param in            incoming websocket data
 * @param len           length of incoming websocket data
 *
 * @retval 0			callback handled succesfully
 * @retval > 0			close the socket
 *
 * @note: for use with lws < 1.7.0
 */

static int app_websocket_on_receive_old(
	struct lws_context *UNUSED(context),
	struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len );
#endif /* libwebsockets < 1.7.0 */

#elif defined( IOT_WEBSOCKET_CIVETWEB )

/**
 * @brief civetweb callback to be called on receiving data
 *
 * @param wsi           websocket connection interface
 * @param flags         flags indicating reason for callback
 * @param data          incoming websocket data
 * @param data_len      length of incoming websocket data
 * @param user_data     user specified data
 *
 * @retval 0			callback handled succesfully
 * @retval > 0			close the socket
 */
static int app_websocket_on_receive(
	struct mg_connection *wsi,
	int flags,
	char *data,
	size_t data_len,
	void *user_data );

/**
 * @brief civetweb callback to be called on websocket closure
 *
 * @param wsi           websocket connection interface
 * @param user_data		user specified data
 */
static void app_websocket_on_close(
	const struct mg_connection *wsi,
	void *user_data );

#endif /* civetweb */


#if !defined( IOT_WEBSOCKET_CIVETWEB )
int app_websocket_on_receive(
	struct lws *wsi,
	enum lws_callback_reasons reason, void * user,
	void *in, size_t len )
{
	/* socket closes on result = 1 */
	int result = 0;
	app_websocket_t * ws = NULL;
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 4 )
	/* libwebsockets >= 1.4.0 */
	const struct lws_protocols* const protocols = lws_get_protocol( wsi );
	(void)user; /* unused warning */
	if ( protocols )
		ws = (app_websocket_t *)protocols[0].user;
#else /* libwebsockets < 1.4.0 */
	(void)wsi; /* unused warning */
	ws = (app_websocket_t *)user;
#endif
	if ( ws && ws->protocol && ws->protocol->on_receive )
	{
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch-enum"
#endif /* ifdef __clang__ */
		switch ( reason )
		{
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			app_log( IOT_LOG_FATAL, NULL,
				"websocket: Failed to connect to client", NULL );
			result = 1;
			break;
#if LWS_LIBRARY_VERSION_MAJOR > 1 || ( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 3 )
	 	/* libwebsockets >= 1.3.0 */
		case LWS_CALLBACK_WSI_DESTROY:
#endif
		case LWS_CALLBACK_CLOSED:
			ws->protocol->on_close( ws->user_data );
			break;
		case LWS_CALLBACK_CLIENT_RECEIVE:
			result = ws->protocol->on_receive( ws->user_data, in, len );
			break;
		case LWS_CALLBACK_CLIENT_WRITEABLE:
			if ( ws->protocol->on_writeable( ws->user_data ) )
				result = 0;
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

#if LWS_LIBRARY_VERSION_MAJOR < 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 )
	/* libwebsockets < 1.6.0 */
int app_websocket_on_receive_old(
	struct lws_context *UNUSED(context),
	struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len )
{
	return app_websocket_on_receive( wsi, reason, user, in, len );
}
#endif

#elif defined( IOT_WEBSOCKET_CIVETWEB )
int app_websocket_on_receive(
	struct mg_connection *UNUSED(wsi),
	int flags,
	char *data,
	size_t data_len,
	void *user_data )
{
	const int opcode = flags & 0xF;
	/* close socket on result = 0 */
	int result = 0;

	app_websocket_t * const ws = (app_websocket_t *)user_data;

	if( ws && ws->protocol
		&& ws->protocol->on_close
		&& ws->protocol->on_receive )
	{
		if( opcode == 0x0 || opcode == 0x1 || opcode == 0x2 )
		{
			/* continue websocket */
			result = !ws->protocol->on_receive( ws->user_data, data, data_len );
		}
		else if ( opcode == 0x8 )
			ws->protocol->on_close( ws->user_data );
		else if ( opcode == 0x9 ) /* websocket ping */
		{
#if CIVETWEB_VERSION_MAJOR > 1 || CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR >= 10
			/* civetweb  >= 1.10 */
			/* build a "PONG" websocket frame */
			result = mg_websocket_client_write( ws->connection,
				MG_WEBSOCKET_OPCODE_PONG, data, data_len );
#else
			/* civetweb < 1.10 */
			/* build a "PONG" websocket frame */
			result = mg_websocket_client_write( ws->connection,
				WEBSOCKET_OPCODE_PONG, data, data_len );
#endif
		}
	}
	return result;
}

void app_websocket_on_close(
	const struct mg_connection *UNUSED(wsi),
	void *user_data )
{
	app_websocket_t * const ws = (app_websocket_t *)user_data;
	if( ws && ws->protocol->on_close )
		ws->protocol->on_close( ws->user_data );
}

#endif

app_websocket_t *app_websocket_initialize( struct app_websocket_protocol *protocol )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	app_websocket_t * ws = NULL;
#if !defined( IOT_WEBSOCKET_CIVETWEB )
	/* libwebsocket declarations */
	/* create libwebsocket protocol struct */
	char cert_path[ PATH_MAX + 1u ];
	struct lws_context *context = NULL;
	struct lws_context_creation_info *context_ci = NULL;
	struct lws_protocols * lws_protocols = NULL;
#endif
	/* check for valid protocol */
	if ( protocol && protocol->on_close
			&& protocol->on_receive
			&& protocol->on_writeable )
	{
		ws = os_malloc( sizeof( struct app_websocket ) );
		if ( ws == NULL )
			result = IOT_STATUS_NO_MEMORY;
		else
		{
			os_memzero( ws, sizeof( struct app_websocket ) );
			ws->protocol = protocol;
			result = IOT_STATUS_SUCCESS;
		}
	}
	else
	{
		app_log( IOT_LOG_FATAL, NULL,
			"websocket: websocket could not be initialized: "
			"invalid protocol specified",
			NULL );
		result = IOT_STATUS_BAD_PARAMETER ;
	}
	/* library initializations */
#if defined( IOT_WEBSOCKET_CIVETWEB )
	/* check for websocket support */
#	if CIVETWEB_VERSION_MAJOR > 1 || \
		CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR > 10
#		define IOT_USE_SSL         MG_FEATURES_TLS
#		define IOT_USE_WEBSOCKETS  MG_FEATURES_WEBSOCKET
#	else /* if civetweb > 1.10 */
#		define IOT_USE_SSL         0x02 /* 2 */
#		define IOT_USE_WEBSOCKETS  0x10 /* 16 */
#	endif /* else if civetweb < 1.10 */
	if ( result == IOT_STATUS_SUCCESS &&
		(mg_check_feature( IOT_USE_WEBSOCKETS ) == 0))
	{
		app_log( IOT_LOG_FATAL, NULL,
			"websocket: civetweb is not compiled with websocket "
			"support", NULL );
		result = IOT_STATUS_FAILURE;
	}

	/* initialize civetlib library (with openssl support) */
	if ( result == IOT_STATUS_SUCCESS &&
		( mg_init_library( IOT_USE_SSL ) == 0))
	{
		app_log( IOT_LOG_FATAL, NULL,
			"websocket: failed to initialize civet library, ensure "
			"that civet was compiled with the correct "
			"flags. (for example set: "
			"CIVETWEB_SSL_OPENSSL_API_1_1 to true, if "
			"compiling against openssl version 1.1 or "
			"greater", NULL );
		result = IOT_STATUS_FAILURE;
	}
#elif !defined( IOT_WEBSOCKET_CIVETWEB )
	if ( result == IOT_STATUS_SUCCESS )
	{
		/* lws requires an array of lws_protocols but we only need one */
		lws_protocols = os_malloc( sizeof( struct lws_protocols ) * 2u );
		if ( lws_protocols == NULL )
			result = IOT_STATUS_NO_MEMORY;
		else
		{
			os_memzero( lws_protocols, sizeof( struct lws_protocols ) * 2u );
			lws_protocols[0].name = protocol->name;
			lws_protocols->per_session_data_size = 0;
			lws_protocols[0].rx_buffer_size = protocol->rx_buffer_size;
#if LWS_LIBRARY_VERSION_MAJOR > 1 || \
			( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 6 )
			lws_protocols[0].callback = app_websocket_on_receive;
#else /* libwebsockets < 1.6.0 */
			lws_protocols[0].callback = app_websocket_on_receive_old;
#endif
#if LWS_LIBRARY_VERSION_MAJOR > 1 || \
			( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 4 )
			lws_protocols[0].user = ws;
#endif /* libwebsockets >= 1.4.0 */
		}
	}
	/* initialize system sockets */
	if ( result == IOT_STATUS_SUCCESS )
	{
		if ( os_socket_initialize() != OS_STATUS_SUCCESS )
			result = IOT_STATUS_FAILURE;
	}

	/* get ca cert path */
	if ( result == IOT_STATUS_SUCCESS )
	{
		/* TODO: make this configurable */
		os_strncpy( cert_path, IOT_DEFAULT_CERT_PATH, PATH_MAX );
	}

	/* create lws context */
	if ( result == IOT_STATUS_SUCCESS )
	{
		context_ci = os_malloc( sizeof( struct lws_context_creation_info ) );
		if ( context_ci == NULL )
			result = IOT_STATUS_NO_MEMORY;
		else
		{
			os_memzero( context_ci, sizeof( struct lws_context_creation_info ) );
			context_ci->port = CONTEXT_PORT_NO_LISTEN;
			context_ci->iface = NULL;
			context_ci->protocols = lws_protocols;
			context_ci->extensions = NULL;

			/* ssl related */
			context_ci->ssl_ca_filepath = cert_path;
			context_ci->ssl_cert_filepath = NULL;
			context_ci->ssl_private_key_filepath = NULL;

			/* id related, to change after socket connection */
			context_ci->gid = -1;
			context_ci->uid = -1;
#if LWS_LIBRARY_VERSION_MAJOR > 2 || \
		( LWS_LIBRARY_VERSION_MAJOR == 2 && LWS_LIBRARY_VERSION_MINOR >= 1 )
			context_ci->options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif /* libwebsockets >= 2.1.0 */
			context = lws_create_context( context_ci );
			if ( context == NULL )
			{
				app_log( IOT_LOG_FATAL, NULL, "websocket: Could not create LWS context.", NULL );
				result = IOT_STATUS_FAILURE;
			}
		}
	}

	if ( result == IOT_STATUS_SUCCESS )
	{
		ws->context = context;
	}
	/* TODO: add proxy support (for LWS >= 1.3.0 ) */
#endif /* if !defined( IOT_WEBSOCKET_CIVETWEB ) */
	if ( result != IOT_STATUS_SUCCESS )
	{
		/* free any allocated memory */
		if ( ws )
			os_free( ws );
		ws = NULL;
	}
	return ws;
}

/* set the log level */
void app_websocket_set_log_level_with_logger( int is_verbose,
	void ( *log_emit_function )(int level, const char *line) )
{
	(void) log_emit_function; /* unused for CIVETWEB */
	(void) is_verbose; /* unused for CIVETWEB */
#if !defined( IOT_WEBSOCKET_CIVETWEB )
	if ( is_verbose )
		lws_set_log_level( LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO |
				   LLL_DEBUG | LLL_CLIENT, log_emit_function );
	else
		lws_set_log_level( LLL_ERR | LLL_WARN | LLL_NOTICE, log_emit_function );
#endif /* if !defined( IOT_WEBSOCKET_CIVETWEB ) */
}

void app_websocket_poll( app_websocket_t * websocket, os_millisecond_t timeout_ms )
{
#if defined( IOT_WEBSOCKET_CIVETWEB )
	(void)websocket; /* unused */
	os_time_sleep( timeout_ms, IOT_TRUE );
#elif !defined( IOT_WEBSOCKET_CIVETWEB )
	/*
	 * If libwebsockets sockets are all we
	 * care about, you can use this api
	 * which takes care of the poll() and
	 * looping through finding who needed
	 * service.
	 *
	 * If no socket needs service, it'll
	 * return anyway after the number of
	 * ms in the second argument.
	 */
	lws_service( websocket->context, (int) timeout_ms );
#endif
	return;
}

iot_status_t app_websocket_set_user_data( app_websocket_t *ws, void *user_data )
{
	iot_status_t result = IOT_STATUS_NOT_INITIALIZED;
	if( ws )
	{
		ws->user_data = user_data;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

void app_websocket_callback_on_writeable( app_websocket_t *ws )
{
#if !defined( IOT_WEBSOCKET_CIVETWEB )
	if ( ws && ws->context && ws->connection )
	{
#	if LWS_LIBRARY_VERSION_MAJOR < 1 || \
			( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR < 6 )
			/* libwebsockets <= 1.6.0 */
		lws_callback_on_writable( ws->context, ws->connection );
#	else /* libwebsockets > 1.6.0 */
		lws_callback_on_writable( ws->connection );
#	endif
	}

#elif defined( IOT_WEBSOCKET_CIVETWEB )
	if ( ws && ws->connection )
		ws->protocol->on_writeable( ws->user_data );
#endif
	else
	{
		app_log( IOT_LOG_FATAL, NULL,
			"websocket: invalid write - websocket not initialized or connected", NULL );
	}
}

iot_status_t app_websocket_connect( app_websocket_t* ws,
	struct app_websocket_ci *connect_in )
{
	iot_status_t result = IOT_STATUS_FAILURE;
	int ssl_connection = configure_ssl( connect_in->is_secure );

#if !defined( IOT_WEBSOCKET_CIVETWEB )
	/* libwebsockets */
#if LWS_LIBRARY_VERSION_MAJOR > 1 || \
	( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 7 )
	/* libwebsockets >= 1.7.0 (use client_ci )*/
	struct lws_client_connect_info client_ci;
#endif
#endif

#if !defined( IOT_WEBSOCKET_CIVETWEB )
#	if LWS_LIBRARY_VERSION_MAJOR > 1 || \
( LWS_LIBRARY_VERSION_MAJOR == 1 && LWS_LIBRARY_VERSION_MINOR >= 7 )
/* libwebsockets >= 1.7.0 (use client_ci) */
	os_memzero( &client_ci, sizeof( struct lws_client_connect_info ) );
	client_ci.context = ws->context;
	client_ci.address = connect_in->web_addr;
	client_ci.origin = connect_in->web_addr;
	client_ci.path = connect_in->path;
	client_ci.port = connect_in->port;
	client_ci.protocol = ws->protocol->name;
	client_ci.ssl_connection = ssl_connection;
	client_ci.host = connect_in->web_addr;
	client_ci.ietf_version_or_minus_one = -1;
	client_ci.client_exts = EXTS;
	ws->connection = lws_client_connect_via_info( &client_ci );
#else /* libwebsockets < 1.7.0 (no client_ci support)*/
	ws->connection = lws_client_connect(
		ws->context,
		connect_in->web_addr,
		connect_in->port,
		ssl_connection,
		connect_in->path,
		connect_in->host,
		connect_in->origin_addr,
		ws->protocol->name,
		-1
	);
#endif
#elif defined( IOT_WEBSOCKET_CIVETWEB )
	ws->connection = mg_connect_websocket_client(
		connect_in->web_addr,
		connect_in->port,
		ssl_connection,
		ws->error_buf,
		sizeof( ws->error_buf ),
		connect_in->path,
		connect_in->origin_addr,
		app_websocket_on_receive,
		app_websocket_on_close,
		ws
	);
#endif
	if( ws->connection != NULL )
		result = IOT_STATUS_SUCCESS;
	else
	{
#if defined( IOT_WEBSOCKET_CIVETWEB )
		ws->error_buf[sizeof(ws->error_buf) -1] = '\0';
		printf("Failed to connect to client: %s", ws->error_buf);
#else
		printf("Failed to connect to client");
#endif
		/* clean-up after connection failure */
		app_websocket_destroy( ws );
	}
	return result;
}

int app_websocket_write( app_websocket_t *ws, void * buffer, size_t len )
{
	int result = 0;
	if ( buffer )
	{
#if defined( IOT_WEBSOCKET_CIVETWEB )
#if CIVETWEB_VERSION_MAJOR > 1 || CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR >= 10
/* if civetweb >= 1.10 */
		result = mg_websocket_client_write(
			ws->connection,
			MG_WEBSOCKET_OPCODE_BINARY,
			(const char *)buffer,
			len );
#else
/* else if civetweb < 1.10 */
		result = mg_websocket_client_write(
			ws->connection,
			WEBSOCKET_OPCODE_BINARY,
			(const char *)buffer,
			len );
#endif
#else /* libwebsockets */
		result = lws_write( ws->connection,
			buffer,
			len, LWS_WRITE_BINARY );
#endif /* else if defined( IOT_WEBSOCKET_CIVETWEB ) */
	}
	return result;
}

void app_websocket_destroy( app_websocket_t *ws )
{
	if( ws )
	{
#if !defined( IOT_WEBSOCKET_CIVETWEB )
		lws_context_destroy( ws->context );
#elif defined( IOT_WEBSOCKET_CIVETWEB )
		mg_exit_library();
#endif
		os_free( ws );
	}
}


/* Helper Methods */
int configure_ssl( int is_secure )
{
	/* configure ssl connection options */
	int ssl_connection = 0;
#if defined( IOT_WEBSOCKET_CIVETWEB )
#	if CIVETWEB_VERSION_MAJOR < 1 || \
		CIVETWEB_VERSION_MAJOR == 1 && CIVETWEB_VERSION_MINOR <= 10
	/* Civetweb <= 1.10 */
	(void)is_secure; /* unused warning */
#if defined( __VXWORKS__ )
	/* in VxWorks a patch is applied to the civetweb
	 * code base to initialize SSL support.
	 */
	ssl_connection = 1;
#else /* if defined( __VXWORKS__ ) */
	ssl_connection = 0;
	app_log( IOT_LOG_WARNING, NULL,
		"websocket: SSL is not supported on civetweb <= 1.10", NULL );
#endif /* else if defined( __VXWORKS__ ) */
#else
	/* Civetweb > 1.10 */
	ssl_connection = 1;
	if ( is_secure == IOT_FALSE )
		app_log( IOT_LOG_WARNING, NULL,
			"websocket: Insecure SSL (private certs) option not "
			"supported on civetweb, using secure", NULL );
#endif /* else if civetweb <= 1.10 */
#else /* if defined( IOT_WEBSOCKET_CIVETWEB ) */


#	if LWS_LIBRARY_VERSION_MAJOR > 2 || \
( LWS_LIBRARY_VERSION_MAJOR == 2 && \
LWS_LIBRARY_VERSION_MINOR >= 1 )
	ssl_connection |= LCCSCF_USE_SSL;
	if ( is_secure == IOT_FALSE )
		ssl_connection |= LCCSCF_ALLOW_SELFSIGNED |
#		if LWS_LIBRARY_VERSION_MAJOR > 2 || \
( LWS_LIBRARY_VERSION_MAJOR == 2 && \
  LWS_LIBRARY_VERSION_MINOR >= 2 )
			LCCSCF_ALLOW_EXPIRED |
#		endif /* libwebsockets >= 2.2.0 */
			LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
#	else /* libwebsockets < 2.1.0 */
	if ( is_secure == IOT_FALSE )
		ssl_connection = 2;
	else
		ssl_connection = 1;
#	endif /* libwebsocket < 2.1.0 */
#endif /* if defined( IOT_WEBSOCKET_CIVETWEB ) */
	return ssl_connection;
}

iot_status_t app_websocket_parse_uri( char *p, const char **prot, const char **ads,
	int *port, const char **path )
{
	iot_status_t result = IOT_STATUS_FAILURE;
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

		result = IOT_STATUS_SUCCESS;
	}
	return result;
}
