/**
 * @file
 * @brief Header file declaring common unit test functionality
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
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

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

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
 * @brief Generates a random string for testing
 * @note The returned string is null terminated
 * @param[out] dest       destination to put the generated string
 * @param[in]  len        length of the string to be generated
 */
void test_generate_random_string( char *dest, size_t len );

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

#endif /* ifndef UNIT_TEST_H */
