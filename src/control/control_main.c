/**
 * @file
 * @brief Main source file for the Wind River IoT control application
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "control_main.h"

#include "control_config.h"
#include "api/shared/iot_types.h"   /* for iot_host structure */
#include "utilities/app_arg.h"
#include "iot_build.h"

#include <iot_json.h>
#include <os.h>
#include <stdlib.h>                    /* for EXIT_SUCCESS, EXIT_FAILURE */

/**
 * @brief Report the application build information
 */
static IOT_SECTION void control_build_information( void );
static IOT_SECTION int control_device_decommission( void );

int control_device_shutdown( iot_bool_t shutdown, unsigned int delay )
{
	return ( os_system_shutdown( shutdown, delay ) ==
		OS_STATUS_INVOKED ? EXIT_SUCCESS : EXIT_FAILURE );
}

int control_device_decommission( void )
{
	char file_path[ PATH_MAX ];
	iot_status_t result = IOT_STATUS_NO_PERMISSION;

	/* #1 config dir: remove the iot-connect.cfg file, reboot. */
	os_snprintf( file_path,
		PATH_MAX, "%s%c%s",
		IOT_DEFAULT_DIR_CONFIG, OS_DIR_SEP,
		IOT_DEFAULT_FILE_CONNECT);

	if ( os_file_exists( file_path ) )
	{
		os_printf("Found %s\n", file_path );
		if ( os_file_delete( file_path ) != OS_STATUS_SUCCESS )
			os_printf("Error: unable to remove %s\n", file_path );
		else
			result = IOT_STATUS_SUCCESS;
	}

	/* #2: runtime dir: the API uses config dir and runtime dir.  Check the runtime
	 * dir as well */
	os_snprintf( file_path,
		PATH_MAX, "%s%c%s",
		IOT_DEFAULT_DIR_RUNTIME, OS_DIR_SEP,
		IOT_DEFAULT_FILE_CONNECT);

	if ( os_file_exists( file_path ) )
	{
		os_printf("Found %s\n", file_path );
		if ( os_file_delete( file_path ) != OS_STATUS_SUCCESS )
		{
			result = IOT_STATUS_NO_PERMISSION;
			os_printf("Error: unable to remove %s\n", file_path );
		}
		else
			result = IOT_STATUS_SUCCESS;
	}

	/* #3: remove the device id */
	os_snprintf( file_path,
		PATH_MAX, "%s%c%s",
		IOT_DEFAULT_DIR_RUNTIME, OS_DIR_SEP,
		IOT_DEFAULT_FILE_DEVICE_ID);
	if ( os_file_exists( file_path ) )
	{
		os_printf("Found %s\n", file_path );
		if ( os_file_delete( file_path ) != OS_STATUS_SUCCESS )
		{
			result = IOT_STATUS_NO_PERMISSION;
			os_printf("Error: unable to remove %s\n", file_path );
		}
		else
			result = IOT_STATUS_SUCCESS;
	}

	if ( result == IOT_STATUS_SUCCESS )
	{
		os_printf("Rebooting system\n");
		if ( control_device_shutdown(IOT_TRUE, IOT_REBOOT_DELAY) != EXIT_SUCCESS )
		{
			os_printf("Error rebooting system\n");
			result = IOT_STATUS_BAD_REQUEST;
		}
	}
	return result;
}

int control_main( int argc, char *argv[] )
{
	int result = EXIT_SUCCESS;

	struct app_arg args[] = {
		{ 'h', "help"          , 0u, NULL, NULL, "display help menu"  , 0u },
		{ 0u , "reboot"        , 0u, NULL, NULL, "reboot the device"  , 0u },
		{ 0u , "shutdown"      , 0u, NULL, NULL, "shutdown the device", 0u },
		{ 'v', "version"       , 0u, NULL, NULL, "display version"    , 0u },
		{ 'd' , "decommission"  , 0u, NULL, NULL, "decommission device", 0u },
		{ 0, NULL, 0, NULL, NULL, NULL, 0u }
	};

	result = app_arg_parse( args, argc, argv, NULL );
	if ( result == EXIT_FAILURE || app_arg_count( args, 'h', NULL ) )
		app_arg_usage( args, 36u, argv[0], IOT_PRODUCT, NULL, NULL );
	else if ( result == EXIT_SUCCESS )
	{
		/* Prompt user if no argument is provided
		 * Generate iot connection configuration file
		 */
		if ( result == EXIT_SUCCESS && argc <= 1 )
			control_config_generate();
		if ( app_arg_count( args, 'v', NULL ) )
			control_build_information();
		if ( result == EXIT_SUCCESS &&
			app_arg_count( args, 0u, "reboot" ) )
		{
			result = control_device_shutdown(IOT_TRUE, IOT_REBOOT_DELAY);
			os_printf("reboot device delay=%d seconds\n", IOT_REBOOT_DELAY);
		}
		if ( result == EXIT_SUCCESS &&
			app_arg_count( args, 0u, "shutdown" ) )
		{
			result = control_device_shutdown( IOT_FALSE, IOT_REBOOT_DELAY);
			os_printf("shutdown device delay=%d seconds\n", IOT_REBOOT_DELAY);
		}
                if ( result == EXIT_SUCCESS &&
                        app_arg_count( args, 0u, "decommission" ) )
		{
			result = control_device_decommission();
			os_printf("decommissioning device\n");
		}
	}
	return result;
}

void control_build_information( void )
{
	os_fprintf( OS_STDERR, "SHA: %s\n", IOT_GIT_SHA );
}
