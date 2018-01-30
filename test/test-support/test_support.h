/**
 * @file
 * @brief Header file declaring common unit test functionality
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

#ifndef TEST_SUPPORT_H
#define TEST_SUPPORT_H

/* header files to include before cmocka */
/* clang-format off */
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
/* clang-format on */

#include <stdio.h> /* for snprintf */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

#ifndef __GNUC__
#define __attribute__( x ) /* gcc specific */
#endif                     /* ifndef __GNUC__ */

/* In Windows some signatures of functions are slightly different */
#if defined( _WIN32 ) && !defined( snprintf )
#define snprintf _snprintf
#endif /* if defined( _WIN32 ) && !defined( snprintf ) */

/* functions */
/**
 * @brief Called to destory test support system
 *
 * @param[in]      argc                number of command-line arguments
 * @param[in]      argv                array of command-line arguments
 */
void test_finalize( int argc, char **argv );

/**
 * @brief Generates a random string for testing
 *
 * @note This function uses a pseudo-random generator to provide
 *       reproductability between test runs, if given the same seed
 *
 * @note The returned string is null terminated
 *
 * @param[out]     dest                destination to put the generated string
 * @param[in]      len                 size of the destination buffer, returned
 *                                     string is null-terminated (thus random
 *                                     characters are: len - 1u)
 */
void test_generate_random_string( char *dest, size_t len );

/**
 * @brief Generates a random universally unique identifer (UUID) for testing
 *
 * @note This function uses a pseudo-random generator to provide
 *       reproductability between test runs, if given the same seed
 *
 * @note The returned string is null terminated
 *
 * @param[out]     dest                destination to put the generated string
 * @param[in]      len                 size of the destination buffer, returned
 *                                     string is null-terminated (thus random
 *                                     characters are: len - 1u).  Note that
 *                                     only the first 36 character (37 +
 *                                     null-terminator) are encoded
 * @param[in]      to_upper            convert the hexidecimal characters to
 *                                     upper-case
 */
void test_generate_random_uuid( char *dest, size_t len, int to_upper );

/**
 * @brief Called to initialize test support system
 *
 * @param[in]      argc                number of command-line arguments
 * @param[in]      argv                array of command-line arguments
 */
void test_initialize( int argc, char **argv );

/**
 * @brief Checks to see if an argument was passed on the command line
 *
 * @param[in]      argc                number of command-line arguments
 * @param[in]      argv                array of command-line arguments
 * @param[in]      name                name of argument (NULL if none)
 * @param[in]      abbrev              abbreviation character ('\0' if none)
 * @param[in]      idx                 matching index if argument specified
 *                                     multiple times (0 for first match)
 * @param[out]     value               value set (NULL if no output value)
 *
 *
 * @retval 0  argument found
 * @retval -1 argument not found
 * @retval -2 argument found, but no value found
 */
int test_parse_arg(
	int argc,
	char **argv,
	const char *name,
	const char abbrev,
	unsigned int idx,
	const char **value );

/* macros */
/**
 * @def FUNCTION_NAME
 * @brief Macro that will hold the name of the current function
 */
#ifndef FUNCTION_NAME
#if defined __func__
#define FUNCTION_NAME __func__
#elif defined __PRETTY_FUNCTION__
#define FUNCTION_NAME __PRETTY_FUNCTION__
#elif defined __FUNCTION__
#define FUNCTION_NAME __FUNCTION__
#else
#define FUNCTION_NAME ""
#endif
#endif

/**
 * @brief Macro test displays the name of the test case
 * @param[in]  x   name of the test case
 */
#define test_case( x ) test_case_out( FUNCTION_NAME, x )
/**
 * @brief Macro that displays the name of the test case
 * @param[in] x    name of the test case in printf format
 * @param[in] ...  printf format flags for the test case name
 */
#define test_case_printf( x, ... ) test_case_out( FUNCTION_NAME, x, __VA_ARGS__ )

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

/**
 * @brief Whether low-level system function mocking is currently enabled
 */
extern int MOCK_SYSTEM_ENABLED;

#endif /* ifndef TEST_SUPPORT_H */
