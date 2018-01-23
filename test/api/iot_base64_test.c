/**
 * @file
 * @brief unit testing for IoT library (base source file)
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

#include "api/public/iot.h"
#include "api/shared/iot_base64.h"
#include "api/shared/iot_types.h"
#include "iot_build.h"

#include <stdlib.h>
#include <string.h>

/* iot_base64_decode */
static void test_iot_base64_decode_bad_string( void **state )
{
	const char *test_in = "====";
	const char *expect_out =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	ssize_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_decode( test_out, out_length, (const uint8_t *)test_in, strlen( test_in ) );
	assert_int_equal( result, -1 );
	assert_string_equal( test_out, "" );

	free( test_out );
}

static void test_iot_base64_decode_in_null( void **state )
{
	const char *test_in =
	    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIG"
	    "J1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxz"
	    "LCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZX"
	    "ZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0"
	    "aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG"
	    "9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
	const char *expect_out =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	ssize_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_decode( test_out, out_length, NULL, strlen( test_in ) );
	assert_int_equal( result, -1 );
	assert_string_equal( test_out, "" );

	free( test_out );
}

static void test_iot_base64_decode_invalid_char( void **state )
{
	const char *test_in = "abc~";
	const char *expect_out =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	ssize_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_decode( test_out, out_length, (const uint8_t *)test_in, strlen( test_in ) );
	assert_int_equal( result, -1 );
	assert_string_equal( test_out, "" );

	free( test_out );
}

static void test_iot_base64_decode_out_3X_length( void **state )
{
	const char *test_in =
	    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIG"
	    "J1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxz"
	    "LCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZX"
	    "ZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0"
	    "aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG"
	    "9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3Vy";
	const char *expect_out =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasur";
	ssize_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_decode( test_out, out_length, (const uint8_t *)test_in, strlen( test_in ) );
	assert_int_equal( result, strlen( expect_out ) );
	assert_string_equal( test_out, expect_out );

	free( test_out );
}

static void test_iot_base64_decode_out_3Xplus2_length( void **state )
{
	const char *test_in =
	    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIG"
	    "J1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxz"
	    "LCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZX"
	    "ZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0"
	    "aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG"
	    "9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
	const char *expect_out =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	ssize_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_decode( test_out, out_length, (const uint8_t *)test_in, strlen( test_in ) );
	assert_int_equal( result, strlen( expect_out ) );
	assert_string_equal( test_out, expect_out );

	free( test_out );
}

/* iot_base64_decode_size */
static void test_iot_base64_decode_size( void **state )
{
	size_t input_size, output_size;
	size_t result;

	input_size = 100u;
	output_size = ( input_size / 4u ) * 3u;
	result = iot_base64_decode_size( input_size );
	/* function returns the ESTIMATED size */
	assert_in_range( result, output_size, output_size + 3u );
}

static void test_iot_base64_decode_size_in_less_than_4( void **state )
{
	size_t input_size;
	size_t result;

	input_size = 1u;
	result = iot_base64_decode_size( input_size );
	/* function returns the ESTIMATED size */
	assert_int_equal( result, 3 );
}

static void test_iot_base64_decode_size_in_zero_length( void **state )
{
	size_t result;

	result = iot_base64_decode_size( 0u );
	assert_int_equal( result, 0u );
}

/* iot_base64_encode */
static void test_iot_base64_encode( void **state )
{
	const char *expect_out =
	    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIG"
	    "J1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxz"
	    "LCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZX"
	    "ZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0"
	    "aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG"
	    "9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
	const char *test_in =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	size_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_encode( test_out, out_length, (const uint8_t *)test_in, strlen( test_in ) );
	assert_int_equal( result, out_length );
	assert_string_equal( test_out, expect_out );

	free( test_out );
}

static void test_iot_base64_encode_in_zero_length( void **state )
{
	const char *expect_out =
	    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIG"
	    "J1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxz"
	    "LCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZX"
	    "ZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0"
	    "aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG"
	    "9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
	const char *test_in =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	size_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_encode( test_out, out_length, (const uint8_t *)test_in, 0 );
	assert_int_equal( result, 0 );
	assert_string_equal( test_out, "" );

	free( test_out );
}

static void test_iot_base64_encode_in_null( void **state )
{
	const char *expect_out =
	    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIG"
	    "J1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxz"
	    "LCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZX"
	    "ZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0"
	    "aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG"
	    "9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
	const char *test_in =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	size_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_encode( test_out, out_length, NULL, strlen( test_in ) );
	assert_int_equal( result, 0 );
	assert_string_equal( test_out, "" );

	free( test_out );
}

static void test_iot_base64_encode_out_zero_length( void **state )
{
	const char *expect_out =
	    "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIG"
	    "J1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxz"
	    "LCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZX"
	    "ZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0"
	    "aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG"
	    "9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";
	const char *test_in =
	    "Man is distinguished, not only by his reason, but by this "
	    "singular passion from other animals, which is a lust of the "
	    "mind, that by a perseverance of delight in the continued and "
	    "indefatigable generation of knowledge, exceeds the short "
	    "vehemence of any carnal pleasure.";
	size_t result;
	size_t out_length;
	uint8_t *test_out;

	out_length = strlen( expect_out );
	test_out = malloc( out_length + 1 );
	assert_non_null( test_out );
	memset( test_out, 0, out_length + 1 );

	result = iot_base64_encode( test_out, 0, (const uint8_t *)test_in, strlen( test_in ) );
	assert_int_equal( result, 0 );
	assert_string_equal( test_out, "" );

	free( test_out );
}

/* iot_base64_encode_size */
static void test_iot_base64_encode_size( void **state )
{
	size_t input_size, output_size;
	size_t result;

	input_size = 100u;
	output_size = ( input_size / 3u ) * 4u;
	result = iot_base64_encode_size( input_size );
	/* function returns the ESTIMATED size */
	assert_in_range( result, output_size, output_size + 4u );
}

static void test_iot_base64_encode_size_in_less_than_3( void **state )
{
	size_t input_size;
	size_t result;

	input_size = 1u;
	result = iot_base64_encode_size( input_size );
	assert_int_equal( result, 4u );
}

static void test_iot_base64_encode_size_in_zero_length( void **state )
{
	size_t result;

	result = iot_base64_encode_size( 0u );
	assert_int_equal( result, 0u );
}

/* main */
int main( int argc, char *argv[] )
{
	int result;
	const struct CMUnitTest tests[] = {
		cmocka_unit_test( test_iot_base64_decode_bad_string ),
		cmocka_unit_test( test_iot_base64_decode_in_null ),
		cmocka_unit_test( test_iot_base64_decode_invalid_char ),
		cmocka_unit_test( test_iot_base64_decode_out_3X_length ),
		cmocka_unit_test( test_iot_base64_decode_out_3Xplus2_length ),
		cmocka_unit_test( test_iot_base64_decode_size ),
		cmocka_unit_test( test_iot_base64_decode_size_in_less_than_4 ),
		cmocka_unit_test( test_iot_base64_decode_size_in_zero_length ),
		cmocka_unit_test( test_iot_base64_encode ),
		cmocka_unit_test( test_iot_base64_encode_in_null ),
		cmocka_unit_test( test_iot_base64_encode_in_zero_length ),
		cmocka_unit_test( test_iot_base64_encode_out_zero_length ),
		cmocka_unit_test( test_iot_base64_encode_size ),
		cmocka_unit_test( test_iot_base64_encode_size_in_less_than_3 ),
		cmocka_unit_test( test_iot_base64_encode_size_in_zero_length )
	};
	test_initialize( argc, argv );
	result = cmocka_run_group_tests( tests, NULL, NULL );
	test_finalize( argc, argv );
	return result;
}
