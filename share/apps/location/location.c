/**
 * @brief Source file for a pseudo location app.
 *
 * @copyright Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */
#include <iot.h>

#include <ctype.h>             /* for alnum */
#include <signal.h>            /* for SIGINT */
#include <stdio.h>             /* for printf() */
#include <stdlib.h>            /* for EXIT_SUCCESS & EXIT_FAILURE */
#include <string.h>            /* for string processing */
#include <time.h>              /* for time() */
#ifdef _WIN32
#	include <Windows.h>    /* for SleepEx */
#else
#	include <unistd.h>     /* for sleep */
#endif

/** @brief Number of milliseconds in a second */
#define MILLISECONDS_IN_SECOND         1000u
/** @brief Interval between samples */
#define POLL_INTERVAL_MSEC             2000u
/** @brief Tag max length */
#define TAG_MAX_LEN                    128u

#ifdef _WRS_KERNEL
	/**
	 * @brief Redefine main function to prevent name collision in global
	 * scope
	 */
#	define main      app_location_main
#endif

/** @brief Variable used to hold the agent library pointer. */
static iot_t *iot_lib_hdl = NULL;
/** @brief Variable used to track running status. */
static iot_bool_t running = IOT_TRUE;
/** @brief Variable used to send location telemetry */
static iot_bool_t send_telemetry = IOT_FALSE;
/** @brief Variable used for location telemetry */
static iot_telemetry_t *telemetry_location = NULL;

/**
 * @brief Text sample used for random string generation.
 */
static const char text_buffer[] = "Arma virumque cano, Troiae qui primus ab oris"
	"Italiam, fato profugus, Laviniaque venit"
	"litora, multum ille et terris iactatus et alto"
	"vi superum saevae memorem Iunonis ob iram;"
	"multa quoque et bello passus, dum conderet urbem,"
	"inferretque deos Latio, genus unde Latinum,"
	"Albanique patres, atque altae moenia Romae.";

/**
 * @brief Debug log wrapper for printf, used for callbacks.
 *
 * @param[in]      log_level           log level to print out
 * @param[in]      source              source information
 * @param[in]      msg                 message string to print out
 * @param[in]      user_data           unused
 */
static void debug_log( iot_log_level_t log_level,
	const iot_log_source_t *source, const char* msg, void* user_data );

/**
 * @brief Sleep handler
 *
 * @param[in]      delay              Length of time in seconds to delay.
 */
static void do_sleep( unsigned int delay );

/**
 * @brief Generate a random number between a minimum and maximum
 *
 * @param[in]     min                  minimum value
 * @param[in]     max                  maximum value
 *
 * @return a random number between the minimum and maximum specified
 */
static double generate_random_number( double  min, double  max );

/**
 * @brief Generate a random string
 *
 * @param[in]      dest                Buffer to store random string.
 * @param[in]      length              Size of the random string to generate.
 * @param[in]      max                 Maximum random sample range.
 * @param[in]      min                 Minimum random sample range.
 */
static void generate_random_string( char *dest, size_t length,
	int max, int min );

/**
 * @brief App initialization
 *
 * @retval IOT_TRUE                    on success
 * @retval IOT_FALSE                   on failure
 */
static iot_bool_t initialize( void );

/**
 * @brief Send telemetry data to the agent.
 */
static void send_telemetry_sample( void );

/**
 * @brief Handles terminatation signal and tears down gracefully
 *
 * @param[in]      signo                         Signal to trap.
 */
static void sig_handler( int signo );

/**
 * @brief Sleeps for the specified time in milliseconds
 *
 * @param[in]      ms                  number of milliseconds to sleep for
 *
 * @retval IOT_STATUS_FAILURE          on system failure or system interruption
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t sleep_ms( iot_millisecond_t ms );

void debug_log( iot_log_level_t log_level,
	const iot_log_source_t *source,
	const char* msg,
	void *user_data )
{
	unsigned int i;
	const char *const prefix[] =
		{ "Fatal", "Alert", "Critical", "Error", "Warning", "Notice",
		  "Info", "Debug", "Trace" };
	(void)user_data;

	/* ensure log level is a valid enumeration value */
	if ( (unsigned int)log_level <= IOT_LOG_TRACE )
	{
		i = (unsigned int)log_level;
		printf( "%s:%s():%u %s: %s\n", source->file_name,
			source->function_name,source->line_number, prefix[i],
			msg );
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

double generate_random_number(
	double  min,
	double  max )
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

void generate_random_string( char *dest, size_t length,
	int max, int min )
{
	while ( length-- > 0 )
	{
		double index = generate_random_number( min, max );
		*dest++ = text_buffer[(size_t)index];
	}
	*dest = '\0';
}

static iot_bool_t initialize( )
{
	iot_bool_t result = IOT_FALSE;
	iot_status_t status = IOT_STATUS_FAILURE;

	iot_lib_hdl = iot_initialize( "location-app", NULL, 0 );
	iot_log_callback_set( iot_lib_hdl, &debug_log, NULL );
	status = iot_connect( iot_lib_hdl, 0u );
	if ( status == IOT_STATUS_SUCCESS )
	{
		IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "%s", "Connected" );

		/* Allocate telemetry items */
		telemetry_location = iot_telemetry_allocate( iot_lib_hdl,
			"location", IOT_TYPE_LOCATION );

		/* Register telemetry items */
		IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "Registering telemetry: %s",
			"location" );
		iot_telemetry_register( telemetry_location, NULL, 0u );
	}
	else
		IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR, "%s",
			"Failed to connect" );
	if ( status == IOT_STATUS_SUCCESS )
		result = IOT_TRUE;
	return result;
}

static void send_telemetry_sample( void )
{
	iot_status_t status = IOT_STATUS_FAILURE;

	/* A location sample */
	iot_location_t *sample;

	/* Accuracy of latitude & longitude in metres */
	iot_float64_t accuracy = 0.0;
	/* Altitude in metres */
	iot_float64_t altitude = 0.0;
	/* Range of the altitude in metres */
	iot_float64_t altitude_accuracy = 0.0;
	/* Direction heading */
	iot_float64_t heading = 0.0;
	/* Latitude in degrees */
	iot_float64_t latitude = 0.0;
	/* Longitude in degrees */
	iot_float64_t longitude = 0.0;
	/* Location source type */
	iot_location_source_t source = IOT_LOCATION_SOURCE_UNKNOWN;
	/* Speed being currently travelled in metres/second */
	iot_float64_t speed = 0.0;
	/* Location tag */
	char tag[TAG_MAX_LEN];

	double random_value;
	iot_uint32_t tag_size;
#define LOG_FORMAT "Location:\n" \
	"\tlatitude         :%f\n" \
	"\tlongitude        :%f\n" \
	"\taccuracy         :%f\n" \
	"\taltitude         :%f\n" \
	"\taltitude_accuracy:%f\n" \
	"\theading          :%f\n" \
	"\tspeed            :%f\n" \
	"\tsource           :%u\n" \
	"\ttag              :%s\n"

	/* Random location values */
	latitude = generate_random_number( -90.0, 90.0 );
	longitude = generate_random_number( 180.0, 180.0 );
	accuracy = generate_random_number( 0.0, 1000.0 );
	altitude = generate_random_number( -15.0, 1000.0 );
	altitude_accuracy = generate_random_number( 0.0, 1000.0 );
	heading = generate_random_number( 0.0, 360.0 );
	speed = generate_random_number( 0.0, 10000.0 );
	random_value = generate_random_number(
		IOT_LOCATION_SOURCE_FIXED, IOT_LOCATION_SOURCE_WIFI );
	source = (iot_location_source_t)random_value;

	random_value = generate_random_number( 0, TAG_MAX_LEN );
	tag_size = (iot_uint32_t)random_value;
	generate_random_string( tag, tag_size, 0, TAG_MAX_LEN - 1 );

	/* create a sample with random values */
	sample = iot_location_allocate( 0.0, 0.0 );
	if ( sample )
	{
		/* set values for the sample */
		if ( ( status = iot_location_set( sample, latitude,
			longitude ) ) != IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting latitude and longitude failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_accuracy_set( sample, accuracy ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting coordinate accuracy failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_altitude_set( sample, altitude ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting altitude failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_altitude_accuracy_set( sample,
			altitude_accuracy ) ) != IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting altitude accuracy failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_heading_set( sample, heading ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting heading failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_speed_set( sample, speed ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting speed failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_source_set( sample, source ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting source failed: %s",
				iot_error( status ) );
		if ( ( status = iot_location_tag_set( sample, tag ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Setting tag failed: %s",
				iot_error( status ) );
	
		/* print out the location values */
		IOT_LOG( iot_lib_hdl, IOT_LOG_INFO,
			LOG_FORMAT,
			latitude, longitude, accuracy, altitude,
			altitude_accuracy, heading, speed,
			(iot_uint32_t)source, tag );

		/* publish location telemetry */
		if ( ( status = iot_telemetry_publish( telemetry_location,
			NULL, 0, IOT_TYPE_LOCATION, sample ) )
			!= IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib_hdl, IOT_LOG_WARNING,
				"Sample publish failed: %s",
				iot_error( status ) );
	}
	else
		IOT_LOG( iot_lib_hdl, IOT_LOG_ERROR, "%s",
			"Failed to allocate a location sample.\n" );

	/* free the sample */
	if ( sample )
		iot_location_free( sample );
}

static void sig_handler( int signo )
{
	if ( signo == SIGINT )
	{
		IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "%s",
			"Received termination signal...\n" );
		running = IOT_FALSE;
	}
}

iot_status_t sleep_ms( iot_millisecond_t ms )
{
	do_sleep( ms / MILLISECONDS_IN_SECOND );
	return IOT_STATUS_SUCCESS;
}

int main( )
{
	if ( initialize( ) == IOT_TRUE )
	{
		signal( SIGINT, sig_handler );

		send_telemetry = IOT_TRUE;

		if ( send_telemetry != IOT_FALSE )
			IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "%s",
				"Sending telemetry..." );
		else
			IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "%s",
				"Disabling telemetry..." );
		while ( running != IOT_FALSE )
		{
			if ( send_telemetry != IOT_FALSE )
				send_telemetry_sample();
			sleep_ms( POLL_INTERVAL_MSEC );
		}
	}

	/* Free telemetry resources allocated */
	iot_telemetry_free( telemetry_location, 0 );
	
	/* Terminate */
	iot_terminate( iot_lib_hdl, 0 );
	IOT_LOG( iot_lib_hdl, IOT_LOG_INFO, "%s", "Exiting..." );
	return EXIT_SUCCESS;
}

