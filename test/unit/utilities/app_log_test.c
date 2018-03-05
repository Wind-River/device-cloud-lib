/**
 * @file
 * @brief unit testing for logging utilities
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

#include "test_support.h"

#include "iot_build.h"
#include "utilities/app_log.h"

#include <stdlib.h>
#include <string.h>

static void test_app_log( void** state )
{
	iot_log_level_t log_level;
	iot_log_source_t source;
	const char *message_1 = "test message 1";
	const char *message_2 = "test message 2";
	const char *message_3 = "test message 3";
	const char *message_4 = "test message 4";
	const char *message_5 = "test message 5";

	log_level = IOT_LOG_CRITICAL;
	source.file_name = "test_file";
	source.function_name = "test_file";
	source.line_number = 100u;

	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_1, NULL );
	app_log( log_level, &source, message_1, NULL );
	app_log( log_level, &source, message_1, NULL );

	log_level = IOT_LOG_NOTICE;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_2, NULL );
	log_level = IOT_LOG_INFO;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_3, NULL );
	log_level = IOT_LOG_DEBUG;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_4, NULL );
	log_level = IOT_LOG_TRACE;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_5, NULL );
}

static void test_app_log_filename_with_path( void** state )
{
	iot_log_level_t log_level;
	iot_log_source_t source;
	const char *message_1 = "test message 1";
	const char *message_2 = "test message 2";
	const char *message_3 = "test message 3";
	const char *message_4 = "test message 4";
	const char *message_5 = "test message 5";

	log_level = IOT_LOG_CRITICAL;
	source.file_name = "/a/b/c/test_file";
	source.function_name = "test_file";
	source.line_number = 100u;

	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_1, NULL );
	app_log( log_level, &source, message_1, NULL );
	app_log( log_level, &source, message_1, NULL );

	log_level = IOT_LOG_NOTICE;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_2, NULL );
	log_level = IOT_LOG_INFO;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_3, NULL );
	log_level = IOT_LOG_DEBUG;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_4, NULL );
	log_level = IOT_LOG_TRACE;
	will_return( __wrap_os_terminal_vt100_support, IOT_TRUE );
	app_log( log_level, &source, message_5, NULL );
}

static void test_app_log_no_vt100( void** state )
{
	iot_log_level_t log_level;
	iot_log_source_t source;
	const char *message_1 = "test message 1";
	const char *message_2 = "test message 2";
	const char *message_3 = "test message 3";
	const char *message_4 = "test message 4";
	const char *message_5 = "test message 5";

	log_level = IOT_LOG_CRITICAL;
	source.file_name = "test_file";
	source.function_name = "test_file";
	source.line_number = 100u;

	will_return( __wrap_os_terminal_vt100_support, IOT_FALSE );
	app_log( log_level, &source, message_1, NULL );
	app_log( log_level, &source, message_1, NULL );
	app_log( log_level, &source, message_1, NULL );

	log_level = IOT_LOG_NOTICE;
	will_return( __wrap_os_terminal_vt100_support, IOT_FALSE );
	app_log( log_level, &source, message_2, NULL );
	log_level = IOT_LOG_INFO;
	will_return( __wrap_os_terminal_vt100_support, IOT_FALSE );
	app_log( log_level, &source, message_3, NULL );
	log_level = IOT_LOG_DEBUG;
	will_return( __wrap_os_terminal_vt100_support, IOT_FALSE );
	app_log( log_level, &source, message_4, NULL );
	log_level = IOT_LOG_TRACE;
	will_return( __wrap_os_terminal_vt100_support, IOT_FALSE );
	app_log( log_level, &source, message_5, NULL );
}

/* main */
int main( int argc, char* argv[] )
{
	int result;
	const struct CMUnitTest tests[] =
	{
		cmocka_unit_test( test_app_log ),
		cmocka_unit_test( test_app_log_filename_with_path ),
		cmocka_unit_test( test_app_log_no_vt100 )
	};
	MOCK_SYSTEM_ENABLED = 1;
	result = cmocka_run_group_tests( tests, NULL, NULL );
	MOCK_SYSTEM_ENABLED = 0;
	return result;
}

