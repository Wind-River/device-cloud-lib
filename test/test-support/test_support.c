/**
 * @file
 * @brief Source file for common test support functions
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

#include "test_support.h"

#include <stdlib.h> /* for srand, fileno */
#include <time.h>   /* for time() */

/**
 * @brief Global variable whether mock low-level system functionality is enabled
 */
int MOCK_SYSTEM_ENABLED = 0;

void test_generate_random_string( char *dest, size_t len )
{
	static const char *random_chars =
	    "abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "0123456789,.-#'?!";
	if ( dest && len > 1u )
	{
		char *cur_pos;
		const char *random_char;
		size_t i;
		size_t random_chars_len = 0u;

		/* (platform independent) strlen */
		random_char = random_chars;
		while ( *random_char != '\0' )
		{
			++random_chars_len;
			++random_char;
		}

		/* obtain random character */
		cur_pos = dest;
		srand( (unsigned int)( ( size_t )( time( NULL ) ) * len ) );
		for ( i = 0; i < len - 1u; ++i )
		{
			*cur_pos = random_chars[(size_t)rand() % random_chars_len];
			++cur_pos;
		}
		dest[len - 1u] = '\0'; /* ensure null-terminated */
	}
}
