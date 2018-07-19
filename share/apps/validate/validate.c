/**
 * @brief Source file for the actions app
 *
 * @copyright Copyright (C) 2016-2018 Wind River Systems, Inc. All Rights Reserved.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>  /* for SIGINT */
#include <stdlib.h>  /* for malloc, free, EXIT_SUCCESS & EXIT_FAILURE */
#include <string.h>  /* for strrchr, strlen */
#include <stdio.h>   /* for printf */
#include <time.h>    /* for nanosleep */
#include <unistd.h>   /* for readlink */
#include <errno.h>    /* for errno */

/** @brief time in milliseconds to wait in main loop */
#define POLL_INTERVAL_MSEC 2000u

/** @brief name of file parameter */
#define PARAM_NAME_FILE                "file_name"
/** @brief name of string parameter */
#define PARAM_NAME_STR                 "param"
/** @brief name of string parameter */
#define MATCH_PARAM_VALUE_STR           "value"
/** @brief name of path parameter */
#define PARAM_NAME_PATH                "file_path"

#ifdef _WRS_KERNEL
	/**
	 * @brief Redefine main function to prevent name collision in global
	 * scope
	 */
#	define main      app_action_main
#endif

/** @brief Whether the application is still running */
static iot_bool_t running = IOT_TRUE;

/** @brief Pointer to the pass action */
static iot_action_t *pass_action = NULL;

/** @brief Pointer to the fail action */
static iot_action_t *fail_action = NULL;

/** @brief Pointer to the test file upload action */
static iot_action_t *file_upload_action = NULL;

/** @brief Pointer to the test file download action */
static iot_action_t *file_download_action = NULL;

/** @brief Pointer to the quit action */
static iot_action_t *quit_action = NULL;

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
 * @brief Callback function to an action with parameters
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_NOT_SUPPORTED    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
static iot_status_t on_action_pass_action(
	iot_action_request_t* request,
	void* user_data );


/**
 * @brief Callback function that returns failure
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_NOT_SUPPORTED    invalid parameter passed to function
 */
static iot_status_t on_action_fail_action(
	iot_action_request_t* request,
	void* user_data );



/**
 * @brief Function called to quit the application
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_SUCCESS          always
 */
static iot_status_t on_action_quit(
	iot_action_request_t* request, void* user_data );

/**
 * @brief Callback function that uploads a file
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_NOT_SUPPORTED    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t on_action_file_upload(
	iot_action_request_t* request,
	void* user_data );

/**
 * @brief Callback function that downloads a file
 *
 * @param[in,out]  request             request invoked by the cloud
 * @param[in]      user_data           not used
 *
 * @retval IOT_STATUS_NOT_SUPPORTED    invalid parameter passed to function
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t on_action_file_download(
	iot_action_request_t* request,
	void* user_data );

/**
 * @brief Handles terminatation signal and tears down gracefully
 *
 * @param[in]      signo               signal to trap
 */
static void sig_handler( int signo );

iot_status_t on_action_file_download(
	iot_action_request_t* request,
	void* user_data )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	iot_status_t status = IOT_STATUS_NOT_SUPPORTED;
	const char *param_file = NULL;
	const char *param_path = NULL;
	iot_t *iot_lib = (iot_t *)user_data;
	char file_write[2048];


	printf("==========================================%s\n", __func__);

	/* file name on cloud */
	status = iot_action_parameter_get( request, PARAM_NAME_FILE,
		IOT_FALSE, IOT_TYPE_STRING, &param_file );
	if ( status != IOT_STATUS_SUCCESS )
		printf( "Parameter: %s is empty (using default)...\n",
			PARAM_NAME_FILE );
	else
		printf( "Value for parameter: %s = %s\n", PARAM_NAME_FILE,
			param_file );

	/* get path + filename to download */
	status = iot_action_parameter_get( request, PARAM_NAME_PATH,
		IOT_FALSE, IOT_TYPE_STRING, &param_path );
	if ( status != IOT_STATUS_SUCCESS )
	{
		result = IOT_STATUS_BAD_PARAMETER;
		printf( "Parameter: %s is empty, ignoring...\n",
			PARAM_NAME_PATH );
	}

	if ( result == IOT_STATUS_SUCCESS )
	{
		char cwd[1024];
		size_t len = 0;

		/* download the file we just uploaded to the cwd */
		if ( getcwd(cwd, sizeof(cwd)) != NULL )
			printf("CWD %s\n", cwd);

		/* allow for the / and null bytes */
		len = strlen(cwd) + strlen(param_path) +4;
		snprintf(file_write, len, "%s/%s",cwd, param_path );

		result = iot_file_download( iot_lib,  /* lib handle */
				NULL,                 /* transaction id */
				NULL,                 /* options */
				param_file,           /* file name to rename on cloud */
				file_write,           /* path to send */
				NULL,                 /* callback func */
				NULL );               /* user data */
		if ( result != IOT_STATUS_SUCCESS )
		{
			result = IOT_STATUS_BAD_PARAMETER;
			printf( "Failed to download file: %s\n", param_file );
		}
		else
			printf( "File %s downloaded successfully\n", param_file );
	}

	return result;
}

iot_status_t on_action_file_upload(
	iot_action_request_t* request,
	void* user_data )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	iot_status_t status = IOT_STATUS_NOT_SUPPORTED;
	const char *param_file = NULL;
	const char *param_path = NULL;
	iot_t *iot_lib = (iot_t *)user_data;
	FILE *fh = NULL;
	char file_write[2048];


	printf("==================================================%s\n", __func__);

	/* file name on cloud */
	status = iot_action_parameter_get( request, PARAM_NAME_FILE,
		IOT_FALSE, IOT_TYPE_STRING, &param_file );
	if ( status != IOT_STATUS_SUCCESS )
		printf( "Parameter: %s is empty (using default)...\n",
			PARAM_NAME_FILE );
	else
		printf( "Value for parameter: %s = %s\n", PARAM_NAME_FILE,
			param_file );

	/* get path + filename to upload */
	status = iot_action_parameter_get( request, PARAM_NAME_PATH,
		IOT_FALSE, IOT_TYPE_STRING, &param_path );
	if ( status != IOT_STATUS_SUCCESS )
	{
		result = IOT_STATUS_BAD_PARAMETER;
		printf( "Parameter: %s is empty, ignoring...\n",
			PARAM_NAME_PATH );
	}
	else
	{
		char cwd[1024];
		size_t len = 0;

		if ( getcwd(cwd, sizeof(cwd)) != NULL )
			printf("Writing to the CWD %s\n", cwd);

		/* allow for the / and null bytes */
		len = strlen(cwd) + strlen(param_path) +4;
		snprintf(file_write, len, "%s/%s",cwd, param_path );

		/* create a dummy file in the cwd to upload */
		printf( "Writing test file %s to path %s\n", param_path, cwd);
		fh = fopen(param_path, "w");
		if (fh)
			fprintf(fh, "%s", "Example file for upload, download");
		fclose(fh);
	}

	if ( result == IOT_STATUS_SUCCESS )
	{
		result = iot_file_upload( iot_lib,    /* lib handle */
				NULL,                 /* transaction id */
				NULL,                 /* options */
				param_file,           /* file name to rename on cloud */
				file_write,           /* path to send */
				NULL,                 /* callback func */
				NULL );               /* user data */
		if ( result != IOT_STATUS_SUCCESS )
		{
			result = IOT_STATUS_BAD_PARAMETER;
			printf( "Failed to upload file: %s\n", param_file );
		}
		else
			printf( "File %s uploaded successfully\n", param_file );
	}

	return result;
}

iot_status_t on_action_pass_action(
	iot_action_request_t* request,
	void* user_data )
{
	iot_status_t result = IOT_STATUS_SUCCESS;
	iot_status_t status = IOT_STATUS_FAILURE;
	const char *param_str = NULL;
	(void)(user_data);

	/*success path test each parameter.*/
	/*1. string */
	status = iot_action_parameter_get( request, PARAM_NAME_STR,
		IOT_FALSE, IOT_TYPE_STRING, &param_str );
	if ( status != IOT_STATUS_SUCCESS )
	{
		result = IOT_STATUS_NOT_SUPPORTED;
		printf( "Failed to get value for parameter: %s\n",
			PARAM_NAME_STR );
	}
	else
		printf( "Value for parameter: %s = %s\n", PARAM_NAME_STR,
			param_str );

	if (strncmp(param_str, MATCH_PARAM_VALUE_STR, strlen(MATCH_PARAM_VALUE_STR)) == 0)
	{
		printf("Success: Parameter received %s matches %s\n", param_str,MATCH_PARAM_VALUE_STR );
		result = IOT_STATUS_SUCCESS;
	}
	else
	{
		printf("Error: Parameter received %s matches %s\n", param_str,MATCH_PARAM_VALUE_STR );
		result = IOT_STATUS_NOT_SUPPORTED;
	}
	return result;
}

iot_status_t on_action_fail_action(
		iot_action_request_t* request,
		void* user_data )
{
	(void)(request);
	(void)(user_data);

	return IOT_STATUS_NOT_SUPPORTED;
}

iot_status_t on_action_quit(
		iot_action_request_t* request,
		void* user_data )
{
	(void)(request);
	(void)(user_data);
	running = IOT_FALSE;
	return IOT_STATUS_SUCCESS;
}


static iot_t* initialize( void )
{
	iot_t* iot_lib;
	iot_status_t status = IOT_STATUS_FAILURE;

	iot_lib = iot_initialize( "validate-app", NULL, 0 );
	iot_log_level_set(iot_lib, IOT_LOG_INFO);
	iot_log_callback_set( iot_lib, &log_handler, NULL );
	status = iot_connect( iot_lib, 0u );
	if ( status == IOT_STATUS_SUCCESS )
	{

		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Connected" );

		/* pass_action */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering action: %s",
			"pass_action" );
		pass_action = iot_action_allocate( iot_lib, "pass_action" );

		iot_action_parameter_add( pass_action,
			PARAM_NAME_STR, IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		status = iot_action_register_callback( pass_action,
			&on_action_pass_action, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register action. Reason: %s",
				iot_error( status ) );

		/* fail_action */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "Registering action: %s",
			"fail_action" );
		fail_action = iot_action_allocate( iot_lib, "fail_action" );

		iot_action_parameter_add( fail_action,
			PARAM_NAME_STR, IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		status = iot_action_register_callback( fail_action,
			&on_action_fail_action, NULL, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register action. Reason: %s",
				iot_error( status ) );

		/* create the file upload action */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s",
			"Registering action on_action_file_upload" );
		file_upload_action = iot_action_allocate( iot_lib,
			"file_upload" );

		iot_action_parameter_add( file_upload_action,
			PARAM_NAME_FILE, IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		iot_action_parameter_add( file_upload_action,
			PARAM_NAME_PATH, IOT_PARAMETER_IN_REQUIRED, IOT_TYPE_STRING, 0u );

		status = iot_action_register_callback( file_upload_action,
			&on_action_file_upload, iot_lib, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register action. Reason: %s",
				iot_error( status ) );
		}


		/* create the file download action */
		IOT_LOG( iot_lib, IOT_LOG_INFO, "%s",
			"Registering action on_action_file_download" );
		file_download_action = iot_action_allocate( iot_lib,
			"file_download" );

		iot_action_parameter_add( file_download_action,
			PARAM_NAME_FILE, IOT_PARAMETER_IN, IOT_TYPE_STRING, 0u );

		iot_action_parameter_add( file_download_action,
			PARAM_NAME_PATH, IOT_PARAMETER_IN_REQUIRED, IOT_TYPE_STRING, 0u );

		status = iot_action_register_callback( file_download_action,
			&on_action_file_download, iot_lib, NULL, 0u );
		if ( status != IOT_STATUS_SUCCESS )
		{
			IOT_LOG( iot_lib, IOT_LOG_ERROR,
				"Failed to register action. Reason: %s",
				iot_error( status ) );
		}

		/* quit action */
		quit_action = iot_action_allocate( iot_lib, "quit_app" );
		status = iot_action_register_callback( quit_action,
			&on_action_quit, NULL, NULL, 0u );
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


void sig_handler( int signo )
{
	if ( signo == SIGINT )
	{
		printf( "Received termination signal...\n" );
		running = IOT_FALSE;
	}
}

void log_handler( iot_log_level_t log_level, const iot_log_source_t *log_source,
	const char *message, void *user_data )
{
	(void)(log_level);
	(void)(log_source);
	(void)(user_data);
	printf( "%s\n", message );
}

typedef struct location {
	double lat;
	double lng;
	double acc;
	double alt;
	double hed;
	double spd;
	double src;
} location_t;

int main( int argc, char *argv[] )
{
	iot_t *iot_lib = initialize();
	iot_status_t result = IOT_STATUS_FAILURE;
	(void)argc;
	(void)argv;

	if ( iot_lib )
	{
		iot_telemetry_t *telemetry_sample = NULL;
		iot_telemetry_t *telemetry_location = NULL;
		iot_location_t *location_sample = NULL;
		location_t loc;
		char cwd[1024];
		size_t len = 0;
		char file_write[2048];
		FILE *fh = NULL;

		memset(&loc, 0, sizeof(location_t));
	
		signal( SIGINT, sig_handler );

		/* 1. publish telemetry sample */
		telemetry_sample = iot_telemetry_allocate( iot_lib,
			"property", IOT_TYPE_FLOAT32 );
		iot_telemetry_register( telemetry_sample, NULL, 0u );
		printf("Publishing telemetry sample\n");
		iot_telemetry_publish( telemetry_sample, NULL, 0,
			IOT_TYPE_FLOAT32, 12.34 );

		/* 2. publish attribute */
		iot_attribute_publish_string( iot_lib, NULL, NULL, "attribute", "text and such" );

		/* 3. publish location */
		loc.lat = 45.351603;
		loc.lng = -75.918713;
		loc.acc = 3.0;
		loc.alt = 1.0;
		loc.hed = 12.34;
		loc.spd = 2.0;
		loc.src = IOT_LOCATION_SOURCE_FIXED;

		telemetry_location = iot_telemetry_allocate( iot_lib,
			"location", IOT_TYPE_LOCATION );
		iot_telemetry_register( telemetry_location, NULL, 0u );
		location_sample = iot_location_allocate( 0.0, 0.0 );
		iot_location_set( location_sample, loc.lat, loc.lng);
		iot_location_accuracy_set( location_sample, loc.acc);
		iot_location_altitude_set( location_sample, loc.alt);
		iot_location_heading_set( location_sample, loc.hed);
		iot_location_speed_set( location_sample, loc.spd);
		iot_location_source_set( location_sample, IOT_LOCATION_SOURCE_FIXED);
		iot_telemetry_publish( telemetry_location, NULL, 0, IOT_TYPE_LOCATION, location_sample );

		/* 4. event publish */
		iot_event_publish( iot_lib, NULL, NULL, "logs and such");

		/* 5 file upload */
		if ( getcwd(cwd, sizeof(cwd)) != NULL )
		{
			const char *test_file = "test-upload.txt";
			const char *cloud_file_up = "validate_upload.txt";
			const char *cloud_file_down = "validate_download.txt";
			size_t counter = 0;
			iot_bool_t done = IOT_FALSE;

			printf("Writing to the CWD %s\n", cwd);

			/* allow for the / and null bytes */
			len = strlen(cwd) + strlen(test_file) +4;
			snprintf(file_write, len, "%s/%s",cwd, test_file);
			fh = fopen(file_write, "w");
			if (fh)
				fprintf(fh, "%s", "Example file for upload, download");
			fclose(fh);
			result = iot_file_upload( iot_lib,    /* lib handle */
						NULL,         /* transaction id */
						NULL,         /* options */
						cloud_file_up,/* file name to rename on cloud */
						file_write,   /* path to send */
						NULL,         /* callback func */
						NULL );       /* user data */
			if ( result != IOT_STATUS_SUCCESS )
			{
				result = IOT_STATUS_BAD_PARAMETER;
				printf( "Failed to upload file: %s\n", file_write);
			}
			else
				printf( "File %s uploaded successfully\n", file_write);

			/* 6. file download */
			/* download will return immediately. */
			len =  strlen(cwd) + strlen(cloud_file_down) +4;
			snprintf(file_write, len, "%s/%s",cwd, cloud_file_down);

			printf("Ready for testing\n");

			while (done == IOT_FALSE)
			{
				struct stat file_stat;

				iot_file_download( iot_lib, NULL, NULL,
					cloud_file_up,
					file_write,
					NULL, NULL );
				counter++;

				/* check for file */
				stat( file_write, &file_stat );
				if ( S_ISREG( file_stat.st_mode ) || S_ISLNK( file_stat.st_mode ) )
				{
					done = IOT_TRUE;
					result = IOT_STATUS_SUCCESS;
				}
				if (counter >= 60)
				{
					printf("Error: max download time reached\n");
					done = IOT_TRUE;
					result = IOT_STATUS_TIMED_OUT;
				}
				sleep(2);
			}
		}
		else
			printf("Failed to get CWD\n");

		while ( running != IOT_FALSE )
		{
#ifdef _WIN32
			SleepEx( POLL_INTERVAL_MSEC, TRUE );
#else
			struct timespec t;
			t.tv_sec = 0;
			t.tv_nsec = POLL_INTERVAL_MSEC * 1000;
			nanosleep( &t, NULL );
#endif
		}
	}

	IOT_LOG( iot_lib, IOT_LOG_INFO, "%s", "Exiting..." );

	/* Terminate (calls deregister/free for any remaining actions) */
	iot_terminate( iot_lib, 0 );
	return EXIT_SUCCESS;
}
