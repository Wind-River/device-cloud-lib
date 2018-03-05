/**
 * @file
 * @brief unit testing for common functions (path conversion functions)
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

#include "test_support.h"

#include "iot_build.h"
#include "utilities/app_path.h"

#include <stdlib.h>
#include <string.h>

static void test_app_path_make_absolute_cur_dir_buff_too_small( void **state )
{
	size_t result;
	char path[ 64u ];
	const char *expected_path;
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
#ifdef _WIN32
	will_return( __wrap_os_directory_current, "C:\\" IOT_BIN_DIR );
	expected_path = "C:\\" IOT_BIN_DIR "\\file_to_find.c";
#else
	will_return( __wrap_os_directory_current, "/usr/" IOT_BIN_DIR );
	expected_path = "/usr/" IOT_BIN_DIR "/file_to_find.c";
#endif
	/* too small by 1 character (null-terminator) */
	result = app_path_make_absolute( path, strlen( expected_path ),
		IOT_FALSE );

	assert_int_equal( result, strlen( expected_path ) );

	/* should not change the buffer */
	assert_string_equal( path, "file_to_find.c" );
}

static void test_app_path_make_absolute_cur_dir_failed( void **state )
{
	size_t result;
	char path[ 64u ];
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
	will_return( __wrap_os_directory_current, NULL );
	result = app_path_make_absolute( path, 64u, IOT_FALSE );

	assert_int_equal( result, 0u );
	assert_string_equal( path, "file_to_find.c" );
}

static void test_app_path_make_absolute_cur_dir_valid_with_sep( void **state )
{
	size_t result;
	char path[ 64u ];
	const char *expected_path;
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
#ifdef _WIN32
	will_return( __wrap_os_directory_current, "C:\\" IOT_BIN_DIR "\\" );
	expected_path = "C:\\" IOT_BIN_DIR "\\file_to_find.c";
#else
	will_return( __wrap_os_directory_current, "/usr/" IOT_BIN_DIR "/" );
	expected_path = "/usr/" IOT_BIN_DIR "/file_to_find.c";
#endif
	result = app_path_make_absolute( path, 64u, IOT_FALSE );

	assert_int_equal( result, strlen( expected_path ) );
	assert_string_equal( path, expected_path );
}

static void test_app_path_make_absolute_cur_dir_valid_without_sep( void **state )
{
	size_t result;
	char path[ 64u ];
	const char *expected_path;
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
	will_return( __wrap_os_directory_current, IOT_BIN_DIR );
#ifdef _WIN32
	expected_path = IOT_BIN_DIR "\\file_to_find.c";
#else
	expected_path = IOT_BIN_DIR "/file_to_find.c";
#endif
	result = app_path_make_absolute( path, 64u, IOT_FALSE );

	assert_int_equal( result, strlen( expected_path ) );
	assert_string_equal( path, expected_path );
}

static void test_app_path_make_absolute_null_path( void **state )
{
	size_t result;
	result = app_path_make_absolute( NULL, 0u, IOT_FALSE );
	assert_int_equal( result, 0u );
	result = app_path_make_absolute( NULL, 0u, IOT_TRUE );
	assert_int_equal( result, 0u );
}

static void test_app_path_make_absolute_pass_absolute_path( void **state )
{
	char path[ 64u ];
	const char *expected_path;
	size_t result;
#ifdef _WIN32
	strncpy( path, "C:\\Program Files (x86)\\file_to_find.c", 64u );
	expected_path = "C:\\Program Files (x86)\\file_to_find.c";
#else
	strncpy( path, "/usr/bin/file_to_find.c", 64u );
	expected_path = "/usr/bin/file_to_find.c";
#endif
	expect_string( __wrap_os_path_is_absolute, path, expected_path );
	will_return( __wrap_os_path_is_absolute, IOT_TRUE );
	result = app_path_make_absolute( path, 64u, IOT_FALSE );

	assert_int_equal( result, strlen( expected_path ) );
	assert_string_equal( path, expected_path );
}

static void test_app_path_make_absolute_relative_buff_too_small( void **state )
{
	size_t result;
	char path[ 64u ];
	const char *expected_path;
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
#ifdef _WIN32
	will_return( __wrap_os_path_executable, "C:\\" IOT_BIN_DIR "\\my_app" );
	/* note: IOT_BIN_DIR (is removed) */
	expected_path = "C:\\file_to_find.c";
#else
	will_return( __wrap_os_path_executable, "/usr/" IOT_BIN_DIR "/my_app" );
	/* note: IOT_BIN_DIR (is removed) */
	expected_path = "/usr/file_to_find.c";
#endif
	/* too small by 1 character (null-terminator) */
	result = app_path_make_absolute( path, strlen( expected_path ),
		IOT_TRUE );

	assert_int_equal( result, strlen( expected_path ) );

	/* should not change the buffer */
	assert_string_equal( path, "file_to_find.c" );
}

static void test_app_path_make_absolute_relative_failed( void **state )
{
	size_t result;
	char path[ 64u ];
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
	will_return( __wrap_os_path_executable, NULL );
	result = app_path_make_absolute( path, 64u, IOT_TRUE );

	assert_int_equal( result, 0u );
	assert_string_equal( path, "file_to_find.c" );
}

static void test_app_path_make_absolute_relative_valid_with_sep( void **state )
{
	size_t result;
	char path[ 64u ];
	const char *expected_path;
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
#ifdef _WIN32
	will_return( __wrap_os_path_executable, "C:\\" IOT_BIN_DIR "\\my_app" );
	/* note: IOT_BIN_DIR (is removed) */
	expected_path = "C:\\file_to_find.c";
#else
	will_return( __wrap_os_path_executable, "/usr/" IOT_BIN_DIR "/my_app" );
	/* note: IOT_BIN_DIR (is removed) */
	expected_path = "/usr/file_to_find.c";
#endif
	result = app_path_make_absolute( path, 64u, IOT_TRUE );

	assert_int_equal( result, strlen( expected_path ) );
	assert_string_equal( path, expected_path );
}

static void test_app_path_make_absolute_relative_valid_without_sep( void **state )
{
	char path[ 64u ];
	const char *expected_path;
	size_t result;
	strncpy( path, "file_to_find.c", 64u );
	expect_string( __wrap_os_path_is_absolute, path, "file_to_find.c" );
	will_return( __wrap_os_path_is_absolute, IOT_FALSE );
	will_return( __wrap_os_path_executable, "my_app" );
	expected_path = "file_to_find.c";
	result = app_path_make_absolute( path, 64u, IOT_TRUE );

	assert_int_equal( result, strlen( expected_path ) );
	assert_string_equal( path, expected_path );
}

static void test_app_path_which_null_file( void **state )
{
	size_t result;
	char path[ 123u ];

	*path = '\0';
	result = app_path_which( path, 123u, NULL, NULL );
	assert_int_equal( result, 0u );
	assert_string_equal( path, "" );
}

static void test_app_path_which_null_path_exists( void **state )
{
	size_t result;

	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_TRUE );
#ifdef _WIN32
	will_return( __wrap_os_env_get, "C:\\Windows;C:\\Windows\\bin" );
#else
	will_return( __wrap_os_env_get, "/usr/bin:/usr/local/bin" );
#endif
	will_return( __wrap_os_env_get, "" );
	result = app_path_which( NULL, 0u, NULL, "test_file" );
	assert_int_equal( result, 11u );
}

static void test_app_path_which_null_path_no_env_vars( void **state )
{
	size_t result;

	will_return( __wrap_os_env_get, NULL );
	will_return( __wrap_os_env_get, NULL );
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	result = app_path_which( NULL, 0u, NULL, "test_file" );
	assert_int_equal( result, 0u );
}

static void test_app_path_which_null_path_not_found( void **state )
{
	size_t result;

#ifdef _WIN32
	will_return( __wrap_os_env_get, "C:\\Windows;C:\\Windows\\bin" );
#else
	will_return( __wrap_os_env_get, "/usr/local:/usr/local/bin" );
#endif
	will_return( __wrap_os_env_get, "" );
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "/usr/local/test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "/usr/local/bin/test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	result = app_path_which( NULL, 0u, NULL, "test_file" );
	assert_int_equal( result, 0u );
}

static void test_app_path_which_null_path_only_exts( void **state )
{
	size_t result;

	will_return( __wrap_os_env_get, "" );
#ifdef _WIN32
	will_return( __wrap_os_env_get, ".bat;com;.exe" );
#else
	will_return( __wrap_os_env_get, ".bat:com:.exe" );
#endif
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "./test_file.bat" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "./test_file.com" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "./test_file.exe" );
	will_return( __wrap_os_file_exists, IOT_TRUE );
	result = app_path_which( NULL, 0u, NULL, "test_file" );
	assert_int_equal( result, 15u );
}

static void test_app_path_which_null_path_only_paths( void **state )
{
	size_t result;

#ifdef _WIN32
	will_return( __wrap_os_env_get, "C:\\Windows;C:\\Windows\\bin" );
#else
	will_return( __wrap_os_env_get, "/usr/local:/usr/local/bin" );
#endif
	will_return( __wrap_os_env_get, "" );
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "/usr/local/test_file" );
	will_return( __wrap_os_file_exists, IOT_TRUE );
	result = app_path_which( NULL, 0u, NULL, "test_file" );
	assert_int_equal( result, 20u );
}

static void test_app_path_which_out_path_exists( void **state )
{
	size_t result;
	char out[ 25u ];

	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_TRUE );
#ifdef _WIN32
	will_return( __wrap_os_env_get, "/usr/bin;/usr/local/bin" );
#else
	will_return( __wrap_os_env_get, "/usr/bin:/usr/local/bin" );
#endif
	will_return( __wrap_os_env_get, "" );
	result = app_path_which( out, 25u, NULL, "test_file" );
	assert_int_equal( result, 11u );
	assert_string_equal( out, "./test_file" );
}

static void test_app_path_which_out_path_no_env_vars( void **state )
{
	size_t result;
	char out[ 25u ];

	will_return( __wrap_os_env_get, NULL );
	will_return( __wrap_os_env_get, NULL );
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	result = app_path_which( out, 25u, NULL, "test_file" );
	assert_int_equal( result, 0u );
	assert_string_equal( out, "" );
}

static void test_app_path_which_out_path_not_found( void **state )
{
	size_t result;
	char out[ 25u ];

#ifdef _WIN32
	will_return( __wrap_os_env_get, "C:\\Windows;C:\\Windows\\System32" );
#else
	will_return( __wrap_os_env_get, "/usr/local:/usr/local/bin" );
#endif
	will_return( __wrap_os_env_get, "" );
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "/usr/local/test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "/usr/local/bin/test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	result = app_path_which( out, 25u, NULL, "test_file" );
	assert_int_equal( result, 0u );
	assert_string_equal( out, "" );
}

static void test_app_path_which_out_path_only_exts( void **state )
{
	size_t result;
	char out[ 25u ];

	will_return( __wrap_os_env_get, "" );
#ifdef _WIN32
	will_return( __wrap_os_env_get, ".bat;com;.exe" );
#else
	will_return( __wrap_os_env_get, ".bat:com:.exe" );
#endif
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "./test_file.bat" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "./test_file.com" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
	will_return( __wrap_os_make_path, "./test_file.exe" );
	will_return( __wrap_os_file_exists, IOT_TRUE );
	result = app_path_which( out, 25u, NULL, "test_file" );
	assert_int_equal( result, 15u );
	assert_string_equal( out, "./test_file.exe");
}

static void test_app_path_which_out_path_only_paths( void **state )
{
	size_t result;
	char out[ 25u ];

#ifdef _WIN32
	will_return( __wrap_os_env_get, "C:\\Windows;C:\\Windows\\System32" );
#else
	will_return( __wrap_os_env_get, "/usr/local:/usr/local/bin" );
#endif
	will_return( __wrap_os_env_get, "" );
	will_return( __wrap_os_make_path, "./test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
#ifdef _WIN32
	will_return( __wrap_os_make_path, "C:\\Windows\\test_file" );
#else
	will_return( __wrap_os_make_path, "/usr/local/test_file" );
#endif
	will_return( __wrap_os_file_exists, IOT_TRUE );
	result = app_path_which( out, 25u, NULL, "test_file" );
	assert_int_equal( result, 20u );
#ifdef _WIN32
	assert_string_equal( out, "C:\\Windows\\test_file");
#else
	assert_string_equal( out, "/usr/local/test_file");
#endif
}

static void test_app_path_which_out_path_first_dir_env_vars( void **state )
{
	size_t result;
	char out[ 25u ];

#ifdef _WIN32
	will_return( __wrap_os_env_get, "C:\\Windows;C:\\Windows\\System32" );
	will_return( __wrap_os_env_get, ".bat;com;.exe" );
#else
	will_return( __wrap_os_env_get, "/usr/local:/usr/local/bin" );
	will_return( __wrap_os_env_get, ".bat:com:.exe" );
#endif
	will_return( __wrap_os_make_path, "base_dir/test_file" );
	will_return( __wrap_os_file_exists, IOT_FALSE );
#ifdef _WIN32
	will_return( __wrap_os_make_path, "C:\\Windows\\test_file.bat" );
#else
	will_return( __wrap_os_make_path, "/usr/local/test_file.bat" );
#endif
	will_return( __wrap_os_file_exists, IOT_TRUE );
	result = app_path_which( out, 25u, "base_dir", "test_file" );
	assert_int_equal( result, 24u );
#ifdef _WIN32
	assert_string_equal( out, "C:\\Windows\\test_file.bat" );
#else
	assert_string_equal( out, "/usr/local/test_file.bat" );
#endif
}

/* main */
int main( int argc, char* argv[] )
{
	int result;
	const struct CMUnitTest tests[] =
	{
		cmocka_unit_test( test_app_path_make_absolute_cur_dir_buff_too_small ),
		cmocka_unit_test( test_app_path_make_absolute_cur_dir_failed ),
		cmocka_unit_test( test_app_path_make_absolute_cur_dir_valid_with_sep ),
		cmocka_unit_test( test_app_path_make_absolute_cur_dir_valid_without_sep ),
		cmocka_unit_test( test_app_path_make_absolute_null_path ),
		cmocka_unit_test( test_app_path_make_absolute_pass_absolute_path ),
		cmocka_unit_test( test_app_path_make_absolute_relative_buff_too_small ),
		cmocka_unit_test( test_app_path_make_absolute_relative_failed ),
		cmocka_unit_test( test_app_path_make_absolute_relative_valid_with_sep ),
		cmocka_unit_test( test_app_path_make_absolute_relative_valid_without_sep ),
		cmocka_unit_test( test_app_path_which_null_file ),
		cmocka_unit_test( test_app_path_which_null_path_exists ),
		cmocka_unit_test( test_app_path_which_null_path_no_env_vars ),
		cmocka_unit_test( test_app_path_which_null_path_not_found ),
		cmocka_unit_test( test_app_path_which_null_path_only_exts ),
		cmocka_unit_test( test_app_path_which_null_path_only_paths ),
		cmocka_unit_test( test_app_path_which_out_path_exists ),
		cmocka_unit_test( test_app_path_which_out_path_no_env_vars ),
		cmocka_unit_test( test_app_path_which_out_path_not_found ),
		cmocka_unit_test( test_app_path_which_out_path_only_exts ),
		cmocka_unit_test( test_app_path_which_out_path_only_paths ),
		cmocka_unit_test( test_app_path_which_out_path_first_dir_env_vars ),
	};
	MOCK_SYSTEM_ENABLED = 1;
	result = cmocka_run_group_tests( tests, NULL, NULL );
	MOCK_SYSTEM_ENABLED = 0;
	return result;
}

