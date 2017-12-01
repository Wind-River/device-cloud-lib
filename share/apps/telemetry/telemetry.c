/**
 * @brief Source file for the pseudo-telemetry app.
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
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

#include <iot.h>
#include <ctype.h>   /* for alnum */
#include <signal.h>  /* for SIGINT */
#include <stdlib.h>  /* for malloc, free, EXIT_SUCCESS & EXIT_FAILURE */
#include <string.h>  /* for strrchr, strlen */
#include <stdio.h>   /* for printf */
#include <time.h>    /* for time call */
#ifdef _WIN32
#	include <Windows.h>  /* for SleepEx */
#else
#	include <unistd.h> /* for sleep */
#endif

/** @brief time in seconds to wait in main loop */
#define POLL_INTERVAL_SEC 10u
/** @brief maximum number of telemetry samples to send */
#define MAX_LOOP_ITERATIONS 360

#ifdef _WRS_KERNEL
	/**
	 * @brief Redefine main function to prevent name collision in global
	 * scope
	 */
#	define main      app_telemetry_main
#endif

/** @brief Whether the application is still running */
static iot_bool_t running = IOT_TRUE;
/** @brief Whether or not currently sending telemetry */
static iot_bool_t send_telemetry = IOT_TRUE;

/** @brief Pointer to the boolean telemetry object */
static iot_telemetry_t *telemetry_boolean = NULL;
/** @brief Pointer to the 8-bit integer telemetry object */
static iot_telemetry_t *telemetry_int8 = NULL;
/** @brief Pointer to the 16-bit integer telemetry object */
static iot_telemetry_t *telemetry_int16 = NULL;
/** @brief Pointer to the 32-bit integer telemetry object */
static iot_telemetry_t *telemetry_int32 = NULL;
/** @brief Pointer to the 64-bit integer telemetry object */
static iot_telemetry_t *telemetry_int64 = NULL;
/** @brief Pointer to the 8-bit unsigned integer telemetry object */
static iot_telemetry_t *telemetry_uint8 = NULL;
/** @brief Pointer to the 16-bit unsigned integer telemetry object */
static iot_telemetry_t *telemetry_uint16 = NULL;
/** @brief Pointer to the 32-bit unsigned integer telemetry object */
static iot_telemetry_t *telemetry_uint32 = NULL;
/** @brief Pointer to the 64-bit unsigned integer telemetry object */
static iot_telemetry_t *telemetry_uint64 = NULL;

/** @brief Action to enable/disable transmission of telemetry */
static iot_action_t *enable_action = NULL;

/**
 * @brief sleep handler.
 *
 * @param[in]      delay         Length of time in seconds to sleep.
 */
static void do_sleep( unsigned int delay );

/**
 * @brief Send telemetry data to the agent
 *
 * @return a handle to the iot library
 */
static iot_t *initialize( void );

/**
 * @brief Function called to handle log messages generated from the iot library
 *
 * @param[in]      log_level           message level
 * @param[in]      log_source          source that generated the message
 * @param[in]      message             message that was generated
 * @param[in]      user_data           user specific data
 */
static void log_handler( iot_log_level_t log_level,
	const iot_log_source_t *log_source,
	const char *message, void *user_data );

/**
 * @brief Callback function for enabling & disabling sending telemetery
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          always
 */
static iot_status_t on_enable_disable_telemetry(
	iot_action_request_t* request, void* user_data );

/**
 * @brief Random number generator
 *
 * @param[in]      min                 minimum number to generate
 * @param[in]      max                 maximum number to generate
 *
 * @return a randomly generated number
 */
static double random_num( double min, double max );

/**
 * @brief Send telemetry data to the agent.
 *
 * @param[in]      iot_lib             iot library handle
 */
static void send_telemetry_sample( iot_t *iot_lib );

/**
 * @brief Handles terminatation signal and tears down gracefully
 *
 * @param[in]      signo                         Signal to trap.
 */
static void sig_handler( int signo );

static iot_t* initialize( void )
{
	iot_t* iot_lib;
	iot_status_t status = IOT_STATUS_FAILURE;

	iot_lib = iot_initialize( "telemetry-app", NULL, 0 );
	iot_log_callback_set( iot_lib, &log_handler, NULL );
	status = iot_connect( iot_lib, 0u );
	if ( status == IOT_STATUS_SUCCESS )
	{

		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Connected" );

		/* Allocate telemetry items */
		telemetry_boolean = iot_telemetry_allocate( iot_lib,
			"bool", IOT_TYPE_BOOL );
		telemetry_int8 = iot_telemetry_allocate( iot_lib,
			"int8", IOT_TYPE_INT8 );
		telemetry_int16 = iot_telemetry_allocate( iot_lib,
			"int16", IOT_TYPE_INT16 );
		telemetry_int32 = iot_telemetry_allocate( iot_lib,
			"int32", IOT_TYPE_INT32 );
		telemetry_int64 = iot_telemetry_allocate( iot_lib,
			"int64", IOT_TYPE_INT64 );
		telemetry_uint8 = iot_telemetry_allocate( iot_lib,
			"uint8", IOT_TYPE_UINT8 );
		telemetry_uint16 = iot_telemetry_allocate( iot_lib,
			"uint16", IOT_TYPE_UINT16 );
		telemetry_uint32 = iot_telemetry_allocate( iot_lib,
			"uint32", IOT_TYPE_UINT32 );
		telemetry_uint64 = iot_telemetry_allocate( iot_lib,
			"uint64", IOT_TYPE_UINT64 );

		/* Register telemetry items */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"bool" );
		iot_telemetry_register( telemetry_boolean, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int8" );
		iot_telemetry_register( telemetry_int8, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int16" );
		iot_telemetry_register( telemetry_int16, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int32" );
		iot_telemetry_register( telemetry_int32, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"int64" );
		iot_telemetry_register( telemetry_int64, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint8" );
		iot_telemetry_register( telemetry_uint8, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint16" );
		iot_telemetry_register( telemetry_uint16, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint32" );
		iot_telemetry_register( telemetry_uint32, NULL, 0u );

		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering telemetry: %s",
			"uint64" );
		iot_telemetry_register( telemetry_uint64, NULL, 0u );

		/* Allocate actions */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "registering action: %s",
			"telemetry_ON_OFF" );
		enable_action = iot_action_allocate( iot_lib,
			"telemetry_ON_OFF" );
		status = iot_action_register_callback( enable_action,
			&on_enable_disable_telemetry, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register action. Reason: %s",
				iot_error( status ) );
		}
	}
	else
		IOT_LOG( iot_lib, IOT_LOG_ERROR, "%s", "Failed to connect" );
	if ( status != IOT_STATUS_SUCCESS )
	{
		iot_terminate( iot_lib, 0u );
		iot_lib = NULL;
	}
	return iot_lib;
}

void log_handler( iot_log_level_t log_level, const iot_log_source_t *log_source,
	const char *message, void *user_data )
{
	(void)log_level;
	(void)log_source;
	(void)user_data;
	printf( "%s\n", message );
}

iot_status_t on_enable_disable_telemetry(
	iot_action_request_t* request,
	void* user_data )
{
	(void)(request);
	(void)(user_data);
	if ( send_telemetry == IOT_FALSE )
	{
		printf( "Sending telemetry...\n" );
		send_telemetry = IOT_TRUE;
	}
	else
	{
		printf( "Disabling telemetry...\n" );
		send_telemetry = IOT_FALSE;
	}
	return IOT_STATUS_SUCCESS;
}

double random_num( double min, double max )
{
	static int rand_init = 0;

	/* take a time seed to get better randomness */
	if (!rand_init )
	{
		srand( (unsigned int)time( NULL ) );
		rand_init = 1;
	}
	return min + (rand() / (double)RAND_MAX) * ( max - min );
}

void send_telemetry_sample( iot_t *iot_lib )
{
	static iot_bool_t bool_test = IOT_FALSE;
	iot_int64_t int_test;
	double random_value;
	(void)iot_lib;

	random_value = random_num( -3000000000.0, 3000000000.0 );
	int_test = (iot_int64_t)random_value;

	IOT_LOG( iot_lib, IOT_LOG_INFO,"%s",
		"+--------------------------------------------------------+");
	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending bool  : %s",
		(bool_test == IOT_FALSE ? "false" : "true") );
	iot_telemetry_publish( telemetry_boolean, NULL, 0,
		IOT_TYPE_BOOL, bool_test );
	
	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int8  : %hhd", (signed char) int_test );
	iot_telemetry_publish( telemetry_int8, NULL, 0,
		IOT_TYPE_INT8, (iot_int8_t)int_test );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int16 : %hd", (short) int_test );
	iot_telemetry_publish( telemetry_int16, NULL, 0,
		IOT_TYPE_INT16, (iot_int16_t)int_test );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int32 : %ld", (long int) int_test );
	iot_telemetry_publish( telemetry_int32, NULL, 0,
		IOT_TYPE_INT32, (iot_int32_t)int_test );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending int64 : %lld", (long long int) int_test );
	iot_telemetry_publish( telemetry_int64, NULL, 0,
		IOT_TYPE_INT64, (iot_int64_t)int_test );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint8 : %hhu", (unsigned char) int_test );
	iot_telemetry_publish( telemetry_uint8, NULL, 0,
		IOT_TYPE_UINT8, (iot_uint8_t)int_test );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint16: %hu", (unsigned short) int_test );
	iot_telemetry_publish( telemetry_uint16, NULL, 0,
		IOT_TYPE_UINT16, (iot_uint16_t)int_test );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint32: %lu",
		(unsigned long int) int_test );
	iot_telemetry_publish( telemetry_uint32, NULL, 0,
		IOT_TYPE_UINT32, (iot_uint32_t)int_test );

	IOT_LOG( iot_lib, IOT_LOG_INFO, "Sending uint64: %llu",
		(unsigned long long int) int_test );
	iot_telemetry_publish( telemetry_uint64, NULL, 0,
		IOT_TYPE_UINT64, (iot_uint64_t)int_test );

	/* toggle the boolean value for next sample */
	if ( bool_test == IOT_FALSE )
		bool_test = IOT_TRUE;
	else if ( bool_test == IOT_TRUE )
		bool_test = IOT_FALSE;
}

void sig_handler( int signo )
{
	if ( signo == SIGINT )
	{
		printf( "Received termination signal...\n" );
		running = IOT_FALSE;
	}
}

void do_sleep( unsigned int delay )
{

	/* hide the ifdefs in a function for readability */
#ifdef _WIN32
	/* windows sleep is milliseconds */
	Sleep( delay * MILLISECONDS_IN_SECOND );
#else
	sleep( delay );
#endif
}

int main( int argc, char *argv[] )
{
	iot_t *iot_lib = initialize();
	(void)argc;
	(void)argv;
	if ( iot_lib )
	{
		int count = 0;

		signal( SIGINT, sig_handler );

		/* If any arg is passed in, then start telemetry */
		if ( argc > 1 )
			send_telemetry = IOT_TRUE;

		IOT_LOG( iot_lib, IOT_LOG_INFO,
			"Telemetry interval: %d seconds", POLL_INTERVAL_SEC );

		while ( running != IOT_FALSE )
		{
			if ( send_telemetry != IOT_FALSE )
			{
				send_telemetry_sample( iot_lib );
				count++;

				/* Stop sending samples after a max iteration */
				if ( count >= MAX_LOOP_ITERATIONS )
				{
					/* empty place holders to call
					 * on_enable_disable_telemetry */
					iot_action_request_t* request =
						iot_action_request_allocate(
						iot_lib, "telemetry_ON_OFF",
						NULL );
					IOT_LOG( iot_lib, IOT_LOG_INFO,
						"Max poll interval reached %d.  Stopping telemetry",
						POLL_INTERVAL_SEC );
					iot_action_request_execute( request, 0u );
					count = 0;
				}
				/* sleep for POLL_INTERVAL_SEC */
				do_sleep( POLL_INTERVAL_SEC );
			}
			else
				/* sleep 1 sec while waiting for the action to  start */
				do_sleep( 1 );
		}
	}

	/* Terminate */
	IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Exiting..." );
	iot_terminate( iot_lib, 0 );
	return EXIT_SUCCESS;
}

