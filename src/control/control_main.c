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

int control_device_shutdown( iot_bool_t shutdown, unsigned int delay )
{
	return ( os_system_shutdown( shutdown, delay ) ==
		OS_STATUS_INVOKED ? EXIT_SUCCESS : EXIT_FAILURE );
}

int control_main( int argc, char *argv[] )
{
	int result = EXIT_SUCCESS;

	struct app_arg args[] = {
		{ 'h', "help"          , 0u, NULL, NULL, "display help menu"  , 0u },
		{ 0u , "reboot"        , 0u, NULL, NULL, "reboot the device"  , 0u },
		{ 0u , "shutdown"      , 0u, NULL, NULL, "shutdown the device", 0u },
		{ 'v', "version"       , 0u, NULL, NULL, "display version"    , 0u },
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
	}
	return result;
}

void control_build_information( void )
{
	os_fprintf( OS_STDERR, "SHA: %s\n", IOT_GIT_SHA );
}
