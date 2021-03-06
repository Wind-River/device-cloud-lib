/**
 * @file
 * @brief Source file for common test support functions
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

#include <stdlib.h> /* for srand, fileno */
#include <string.h> /* for strncmp */
#include <time.h>   /* for time() */

/**
 * @brief Helper function for generating a random string for testing
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
 * @param[in]      random_chars        string of input characters that can be
 *                                     used to generate the output
 */
static void test_generate_random_string_internal( char *dest, size_t len,
	const char *random_chars );

/**
 * @brief Global variable whether mock low-level system functionality is enabled
 */
int MOCK_SYSTEM_ENABLED = 0;

void test_finalize( int argc, char **argv )
{
	/* disable mocking system */
	(void)argc;
	(void)argv;
	MOCK_SYSTEM_ENABLED = 0;
}

void test_generate_random_string( char *dest, size_t len )
{
	static const char *random_chars =
	    "abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "0123456789,.-#'?!";
	test_generate_random_string_internal( dest, len, random_chars );
}

static void test_generate_random_string_internal( char *dest, size_t len,
	const char *random_chars )
{
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
		for ( i = 0; i < len - 1u; ++i )
		{
			*cur_pos = random_chars[(size_t)rand() % random_chars_len];
			++cur_pos;
		}
		dest[len - 1u] = '\0'; /* ensure null-terminated */
	}
}

void test_generate_random_uuid( char *dest, size_t len, int to_upper )
{
	static const char *random_chars = "abcdef0123456789";
	test_generate_random_string_internal( dest, len, random_chars );
	if ( len > 8u )
		dest[ 8u ] = '-';
	if ( len > 13u )
		dest[ 13u ] = '-';
	if ( len > 18u )
		dest[ 18u ] = '-';
	if ( len > 23u )
		dest[ 23u ] = '-';

	/* uuid have max length */
	if ( len > 36u )
		dest[ 36u ] = '\0';

	/* support generating upper characters */
	if ( to_upper && len > 0u )
	{
		while ( *dest )
		{
			if ( *dest >= 'a' && *dest <= 'z' )
				*dest = (*dest - 'a' + 'A');
			++dest;
		}
	}
}

void test_initialize( int argc, char **argv )
{
	const char *value = NULL;
	unsigned int random_seed;

	random_seed = (unsigned int)time( NULL );

	test_parse_arg( argc, argv, "seed", 's', 0u, &value );
	if ( value )
		random_seed = (unsigned int)atoi( value );

	printf( "TEST SEED: %u\n", (unsigned int)random_seed );
	srand( random_seed );

	/* enable mocking system */
	MOCK_SYSTEM_ENABLED = 1;
}

int test_parse_arg(
	int argc,
	char **argv,
	const char *name,
	const char abbrev,
	unsigned int idx,
	const char **value )
{
/** @brief character used to prepend an argument name '-n' or '--name' */
#define TEST_ARG_CHAR '-'
/** @brief character used to split an argument from a value */
#define TEST_ARG_SPLIT '='

	int i;
	size_t name_len = 0u;
	int retval = -1;
	unsigned int match_count = 0u;

	if ( name )
		name_len = strlen( name );

	for ( i = 0u; retval == -1 && i < argc; ++i )
	{
		if ( abbrev )
		{
			if ( ( argv[i][0u] == TEST_ARG_CHAR &&
				argv[i][1u] == abbrev ) &&
				( argv[i][2u] == TEST_ARG_SPLIT || argv[i][2u] == '\0' ) )
			{
				retval = 0;
			}
		}
		if ( name_len )
		{
			if ( ( argv[i][0u] == TEST_ARG_CHAR &&
				argv[i][1u] == TEST_ARG_CHAR &&
				strncmp( &argv[i][2u], name, name_len ) == 0 ) &&
				( argv[i][name_len + 2u] == TEST_ARG_SPLIT ||
				  argv[i][name_len + 2u] == '\0' ) )
			{
				retval = 0;
			}
		}

		/* match found */
		if ( retval == 0 )
		{
			if ( match_count < idx )
			{
				++match_count;
				retval = -1;
			}
			else
			{
				const char *v = strchr( argv[i], TEST_ARG_SPLIT );
				if ( v )
					++v;
				else if ( i+1 < argc &&
					argv[i+1][0u] != TEST_ARG_CHAR )
					v = argv[i+1];

				if ( value && v )
					*value = v;
				else if ( value )
					retval = -2; /* no value found */
			}
		}
	}
	return retval;
}

/* introduced in cmocka 1.0.0 */
#if defined(NEED_TEST_REALLOC)
#define MALLOC_GUARD_SIZE 16
struct ListNode {
	const void *value;
	int refcount;
	struct ListNode *next;
	struct ListNode *prev;
};

struct MallocBlockInfo {
	void* block;
	size_t allocated_size;
	size_t size;
	SourceLocation location;
	struct ListNode node;
};

void *_test_realloc(void *ptr,
	const size_t size,
	const char *file,
	const int line)
{
	void *rv = NULL;

	if ( size == 0 )
		_test_free(ptr, file, line);
	else
		rv = _test_malloc(size, file, line);

	if ( ptr && rv )
	{
		size_t cpy_size = size;
		struct MallocBlockInfo *block_info =
			(struct MallocBlockInfo*)(void *)
			((char*)ptr - (MALLOC_GUARD_SIZE + sizeof(struct MallocBlockInfo)));

		if (block_info->size < size)
			cpy_size = block_info->size;

		memcpy(rv, ptr, cpy_size);
		_test_free(ptr, file, line);
	}

	return rv;
}
#endif /* if defined(NEED_TEST_REALLOC) */

#if defined(NEED_CMOCKA_RUN_GROUP_TESTS)
int _cmocka_run_group_tests(const char *group_name,
	const struct CMUnitTest *const tests,
	const size_t num_tests,
	CMFixtureFunction group_setup,
	CMFixtureFunction group_teardown)
{
	int rv = 0;
	void *group_state = NULL;
	size_t i;
	unsigned int tests_executed = 0u, tests_failed = 0u;
	const char **failed_names = NULL;

	/* group setup */
	if ( group_setup )
	{
		rv = (*group_setup)(&group_state);
		if ( rv != 0 )
			print_error( "[  ERROR   ] %s - Group setup failed\n",
				tests->name );
	}

	/* run tests */
	if ( rv == 0 )
	{
		const struct CMUnitTest *test = tests;
		for ( i = 0u; i < num_tests; ++i, ++test )
		{
			if ( test->test_func )
			{
				const int test_res =_run_test( test->name,
					test->test_func, group_state,
					UNIT_TEST_FUNCTION_TYPE_TEST, NULL );
				if ( test_res != 0 )
				{
					const char **fn = realloc(failed_names,
						sizeof(char *) * (tests_failed + 1u));
					if ( fn )
					{
						fn[tests_failed] = test->name;
						failed_names = fn;
						++tests_failed;
					}
					++rv;
				}
				++tests_executed;
			}
		}
	}

	/* group teardown */
	if ( group_teardown && (*group_teardown)(&group_state) != 0 )
		++rv;

	/* print summary */
	print_message("[==========] %u test(s) run.\n",
		tests_executed);
	print_error("[  PASSED  ] %u test(s).\n",
		tests_executed - tests_failed);

	if (tests_failed > 0u)
	{
		print_error("[  FAILED  ] %u test(s), listed below:\n",
			tests_failed);
		for (i = 0u; i < (size_t)tests_failed; ++i)
			print_error("[  FAILED  ] %s\n", failed_names[i]);
		free(failed_names);
	} else
		print_error("\n %u FAILED TEST(S)\n", tests_failed);

	return rv;
}
#endif /* if defined(NEED_CMOCKA_RUN_GROUP_TESTS) */

/* introduced in cmocka 0.4.0 */
#if defined(NEED_ASSERT_RETURN_CODE)
void _assert_return_code(const LargestIntegralType result,
	size_t rlen,
	const LargestIntegralType error,
	const char * const expression,
	const char * const file,
	const int line)
{
	LargestIntegralType valmax;

	switch (rlen) {
	case 1:
		valmax = 255;
		break;
	case 2:
		valmax = 32767;
		break;
	case 4:
		valmax = 2147483647;
		break;
	case 8:
	default:
		if (rlen > sizeof(valmax)) {
			valmax = 2147483647;
		} else {
			valmax = 9223372036854775807L;
		}
		break;
	}

	if (result > valmax - 1) {
		if (error > 0) {
			print_error("%s < 0, errno(%llu): %s\n",
				expression, error, strerror((int)error));
		} else {
			print_error("%s < 0\n", expression);
		}
		_fail(file, line);
	}
}
#endif /* if defined(need_assert_return_code) */
