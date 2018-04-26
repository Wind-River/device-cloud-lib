/**
 * @file
 * @brief Source code for mocking the operating system abstraction layer
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

#include "api/public/iot.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <os.h>
#include <string.h>
#include <test_support.h>

/* mock definitions */
void *__wrap_os_calloc( size_t nmemb, size_t size );
os_status_t __wrap_os_directory_current( char *buffer, size_t size );
os_status_t __wrap_os_directory_create( const char *path, os_millisecond_t timeout );
os_bool_t __wrap_os_directory_exists( const char *dir_path );
size_t __wrap_os_env_expand( char *dest, size_t len );
size_t __wrap_os_env_get( const char *env, char *dest, size_t len );
os_status_t __wrap_os_file_chown( const char *path, const char *user );
os_status_t __wrap_os_file_close( os_file_t handle );
os_bool_t __wrap_os_file_eof( os_file_t stream );
os_bool_t __wrap_os_file_exists( const char *file_path );
os_file_t __wrap_os_file_open( const char *file_path, int flags );
size_t __wrap_os_file_read( void *ptr, size_t size, size_t nmemb, os_file_t stream );
size_t __wrap_os_file_write( const void *ptr, size_t size, size_t nmemb, os_file_t stream );
os_bool_t __wrap_os_flush( os_file_t stream );
int __wrap_os_fprintf( os_file_t stream, const char *format, ... )
	__attribute__((format(printf,2,3)));
void __wrap_os_free( void *ptr );
void __wrap_os_free_null( void **ptr );
os_status_t __wrap_os_make_path( char *path, size_t len, ... );
void *__wrap_os_malloc( size_t size );
void __wrap_os_memcpy( void *dest, const void *src, size_t len );
void __wrap_os_memmove( void *dest, const void *src, size_t len );
void __wrap_os_memset( void *dest, int c, size_t len );
void __wrap_os_memzero( void *dest, size_t len, os_bool_t secure );
os_bool_t __wrap_os_path_is_absolute( const char *path );
os_status_t __wrap_os_path_executable( char *path, size_t size );
int __wrap_os_printf( const char *format, ... )
	__attribute__((format(printf,1,2)));
void *__wrap_os_realloc( void *ptr, size_t size );
int __wrap_os_snprintf( char *str, size_t size, const char *format, ... )
	__attribute__((format(printf,3,4)));
os_status_t __wrap_os_socket_initialize( void );
os_status_t __wrap_os_socket_terminate( void );
int __wrap_os_strcasecmp( const char *s1, const char *s2 );
char *__wrap_os_strchr( const char *s, char c );
int __wrap_os_strcmp( const char *s1, const char *s2 );
size_t __wrap_os_strlen( const char *s );
int __wrap_os_strncasecmp( const char *s1, const char *s2, size_t len );
int __wrap_os_strncmp( const char *s1, const char *s2, size_t len );
char *__wrap_os_strncpy( char *destination, const char *source, size_t num );
char *__wrap_os_strpbrk( const char *str1, const char *str2 );
char *__wrap_os_strrchr( const char *s, int c );
char *__wrap_os_strstr( const char *str1, const char *str2 );
double __wrap_os_strtod( const char *str, char **endptr );
char* __wrap_os_strtok( char* s, const char* delm );
long __wrap_os_strtol( const char *str, char **endptr );
unsigned long __wrap_os_strtoul( const char *str, char **endptr );
int __wrap_os_system_error_last( void );
const char *__wrap_os_system_error_string( int error_number );
os_uint32_t __wrap_os_system_pid( void );
os_status_t __wrap_os_system_run(
	const char *command,
	int *exit_status,
	os_bool_t privileged,
	int priority,
	size_t stack_size,
	os_file_t pipe_files[2u] );
os_status_t __wrap_os_system_run_wait(
	const char *command,
	int *exit_status,
	os_bool_t privileged,
	int priority,
	size_t stack_size,
	char *out_buf[2u],
	size_t out_len[2u],
	os_millisecond_t max_time_out );
os_bool_t __wrap_os_terminal_vt100_support( os_file_t stream );
#ifdef IOT_THREAD_SUPPORT
os_status_t __wrap_os_thread_condition_broadcast( os_thread_condition_t *cond );
os_status_t __wrap_os_thread_condition_create( os_thread_condition_t *cond );
os_status_t __wrap_os_thread_condition_destroy( os_thread_condition_t *cond );
os_status_t __wrap_os_thread_condition_signal(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock );
os_status_t __wrap_os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock );
os_status_t __wrap_os_thread_create(
	os_thread_t *thread, os_thread_main_t main, void *arg, size_t stack_size );
os_status_t __wrap_os_thread_destroy( os_thread_t *thread );
os_status_t __wrap_os_thread_mutex_create( os_thread_mutex_t *lock );
os_status_t __wrap_os_thread_mutex_destroy( os_thread_mutex_t *lock );
os_status_t __wrap_os_thread_mutex_lock( os_thread_mutex_t *lock );
os_status_t __wrap_os_thread_mutex_unlock( os_thread_mutex_t *lock );
os_status_t __wrap_os_thread_rwlock_create( os_thread_rwlock_t *lock );
os_status_t __wrap_os_thread_rwlock_destroy( os_thread_rwlock_t *lock );
os_status_t __wrap_os_thread_rwlock_read_lock( os_thread_rwlock_t *lock );
os_status_t __wrap_os_thread_rwlock_read_unlock( os_thread_rwlock_t *lock );
os_status_t __wrap_os_thread_rwlock_write_lock( os_thread_rwlock_t *lock );
os_status_t __wrap_os_thread_rwlock_write_unlock( os_thread_rwlock_t *lock );
os_status_t __wrap_os_thread_wait( os_thread_t *thread );
#endif /* ifdef IOT_THREAD_SUPPORT */
os_status_t __wrap_os_time( os_timestamp_t *time_stamp, os_bool_t *up_time );
os_status_t __wrap_os_time_sleep( os_millisecond_t ms, os_bool_t allow_interrupts );
int __wrap_os_vfprintf( os_file_t stream, const char *format, va_list args )
	__attribute__((format(printf,2,0)));
int __wrap_os_vsnprintf( char *str, size_t size, const char *format, va_list args )
	__attribute__((format(printf,3,0)));
os_status_t __wrap_os_uuid_generate( os_uuid_t *uuid );
os_status_t __wrap_os_uuid_to_string_lower( os_uuid_t *uuid, char *dest, size_t len );

/* mock functions */
void *__wrap_os_calloc( size_t nmemb, size_t size )
{
	void* result = mock_ptr_type( void* );
	if ( result )
		result = test_calloc( nmemb, size );
	return result;
}

os_status_t __wrap_os_directory_create( const char *path, os_millisecond_t timeout )
{
	return OS_STATUS_SUCCESS;
}

os_status_t __wrap_os_directory_current( char *buffer, size_t size )
{
	const char *source = mock_ptr_type( const char * );
	if ( source )
		strncpy( buffer, source, size );
	return ( source == NULL ? OS_STATUS_FAILURE : OS_STATUS_SUCCESS );
}

os_bool_t __wrap_os_directory_exists( const char *dir_path )
{
	return mock_type( os_bool_t );
}

size_t __wrap_os_env_expand( char *dest, size_t len )
{
	size_t result = 0u;
	if ( dest )
		result = strlen( dest );
	return result;
}

size_t __wrap_os_env_get( const char *env, char *dest, size_t len )
{
	const char *source = mock_ptr_type( const char * );
	if ( source )
		strncpy( dest, source, len );
	else if ( dest )
		*dest = '\0';
	return ( source == NULL ? OS_STATUS_FAILURE : OS_STATUS_SUCCESS );
}

int __wrap_os_fprintf( os_file_t stream, const char *format, ... )
{
	va_list args;
	int result = -1;
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( format );

	va_start( args, format );
	result = __wrap_os_vfprintf( stream, format, args );
	va_end( args );
	return result;
}

os_status_t __wrap_os_file_chown( const char *path, const char *user )
{
	return OS_STATUS_SUCCESS;
}

os_status_t __wrap_os_file_close( os_file_t handle )
{
	return OS_STATUS_SUCCESS;
}

os_bool_t __wrap_os_file_eof( os_file_t stream )
{
	return mock_type( os_bool_t );
}

os_bool_t __wrap_os_file_exists( const char *file_path )
{
	return mock_type( os_bool_t );
}

os_file_t __wrap_os_file_open( const char *file_path, int flags )
{
	return mock_type( os_file_t );
}

size_t __wrap_os_file_read( void *ptr, size_t size, size_t nmemb, os_file_t stream )
{
	size_t result = mock_type( size_t );
	if ( result != 0u )
	{
		result = size * nmemb;
		test_generate_random_string( ptr, result );
	}
	return result;
}

size_t __wrap_os_file_write( const void *ptr, size_t size, size_t nmemb, os_file_t stream )
{
	return size * nmemb;
}

os_bool_t __wrap_os_flush( os_file_t stream )
{
	return OS_TRUE;
}

void __wrap_os_free( void *ptr )
{
	/* in cmocka 0.3.2, test_free( NULL ) will generate an error.  So let's
	 * ensure that ptr is valid, in case use is using a new version of
	 * cmocka they will also see this error.  Although, according to the
	 * man pages of "free", passing NULL is valid. But we never know if all
	 * operating systems behave 100% correctly so it is a good idea to
	 * explicitly check. */
	assert_non_null( ptr );
	test_free( ptr );
}

void __wrap_os_free_null( void **ptr )
{
	/* ensure this function is called meeting pre-requirements */
	if ( ptr && *ptr )
	{
		test_free( *ptr );
		*ptr = NULL;
	}
}

void *__wrap_os_realloc( void *ptr, size_t size )
{
	void* result = mock_ptr_type( void* );
	if ( result )
	{
		/* ptr is allowed to be NULL for this function */
		result = test_realloc( ptr, size );
	}
	return result;
}

os_status_t __wrap_os_make_path( char *path, size_t len, ... )
{
	strncpy( path, mock_ptr_type( char * ), len );
	return OS_STATUS_SUCCESS;
}

void *__wrap_os_malloc( size_t size )
{
	void* result = mock_ptr_type( void* );
	if ( result )
	{
		result = test_malloc( size );
		assert_non_null( result );
	}
	return result;
}

void __wrap_os_memcpy( void *dest, const void *src, size_t len )
{
	char *c_dest = (char *)dest;
	const char *c_src = (const char *)src;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( dest );
	assert_non_null( src );
	assert_ptr_not_equal( dest, src );

	/* perform memcpy */
	while ( len > 0u )
	{
		*c_dest = *c_src;
		++c_dest;
		++c_src;
		--len;
	}
}

void __wrap_os_memmove( void *dest, const void *src, size_t len )
{
	char *c_dest = (char *)dest;
	const char *c_src = (const char *)src;
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( dest );
	assert_non_null( src );
	assert_ptr_not_equal( dest, src );

	/* perform memmove */
	if ( src < dest )
	{
		/* copy backwards */
		c_dest += len - 1;
		c_src += len - 1;
		while ( len > 0u )
		{
			*c_dest = *c_src;
			--c_dest;
			--c_src;
			--len;
		}
	}
	else
	{
		/* copy forwards */
		while ( len > 0u )
		{
			*c_dest = *c_src;
			++c_dest;
			++c_src;
			--len;
		}
	}
}

void __wrap_os_memset( void *dest, int c, size_t len )
{
	char *c_dest = (char *)dest;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( dest );

	/* perform memset */
	while ( len > 0u )
	{
		*c_dest = (char)c;
		++c_dest;
		--len;
	}
}

void __wrap_os_memzero( void *dest, size_t len, os_bool_t secure )
{
	volatile char *c_dest = (volatile char *)dest;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( dest );

	/* perform memzero */
	while ( len > 0u )
	{
		*c_dest = '\0';
		++c_dest;
		--len;
	}
}

os_bool_t __wrap_os_path_is_absolute( const char *path )
{
	check_expected( path );
	return mock_type( os_bool_t );
}

os_status_t __wrap_os_path_executable( char *path, size_t size )
{
	const char *exe_path = mock_ptr_type( const char * );
	if ( exe_path && path )
		strncpy( path, exe_path, size );
	return ( exe_path == NULL ? OS_STATUS_FAILURE : OS_STATUS_SUCCESS );
}

os_status_t __wrap_os_socket_initialize( void )
{
	return OS_STATUS_SUCCESS;
}

os_status_t __wrap_os_socket_terminate( void )
{
	return OS_STATUS_SUCCESS;
}

int __wrap_os_strcasecmp( const char *s1, const char *s2 )
{
	const char offset = 'a' - 'A';
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( s1 );
	assert_non_null( s2 );

	/* perform strcasecmp */
	while( ( *s1 ) && ( *s1 == *s2 || *s1 + offset == *s2 || *s1 == *s2 + offset ) )
	{
		s1++;
		s2++;
	}
	return *( const unsigned char* )s1 - *( const unsigned char* )s2;
}

char *__wrap_os_strchr( const char *s, char c )
{
	const char *result = NULL;
	assert_non_null( s );

	while ( *s != '\0' && *s != c )
		++s;
	if ( *s == c )
		result = s;
#ifdef __GNUC__
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	return (char *)result;
#ifdef __GNUC__
#	pragma GCC diagnostic pop
#endif
}

int __wrap_os_strcmp( const char *s1, const char *s2 )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( s1 );
	assert_non_null( s2 );

	/* perform strcmp */
	while( ( *s1 ) && ( *s1 == *s2 ) )
	{
		s1++;
		s2++;
	}
	return *( const unsigned char* )s1 - *( const unsigned char* )s2;
}

size_t __wrap_os_strlen( const char *s )
{
	size_t result = 0u;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( s );

	while ( *s != '\0' )
	{
		++result;
		++s;
	}
	return result;
}

int __wrap_os_strncasecmp( const char *s1, const char *s2, size_t len )
{
	size_t i = 0u;
	const char offset = 'a' - 'A';
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( s1 );
	assert_non_null( s2 );

	/* perform strcasecmp */
	while( ( *s1 ) && ( *s1 == *s2 || *s1 + offset == *s2 || *s1 == *s2 + offset ) && ( i < len ) )
	{
		s1++;
		s2++;
		i++;
	}

	if ( i == len )
		return 0;
	else
		return *( const unsigned char* )s1 - *( const unsigned char* )s2;
}

int __wrap_os_strncmp( const char *s1, const char *s2, size_t len )
{
	size_t i = 0;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( s1 );
	assert_non_null( s2 );

	/* perform strncmp */
	while( ( *s1 ) && ( *s1 == *s2 ) && ( i < len ) )
	{
		s1++;
		s2++;
		i++;
	}

	if ( i == len )
		return 0;
	else
		return *( const unsigned char* )s1 - *( const unsigned char* )s2;
}

char *__wrap_os_strncpy( char *destination, const char *source, size_t num )
{
	size_t i = 0;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( destination );
	assert_non_null( source );

	/* perform strncpy */
	for ( ; i < num && *source != '\0'; ++i, ++destination, ++source )
		*destination = *source;
	if ( i < num )
		*destination = '\0';
	return destination;
}

char *__wrap_os_strpbrk( const char *str1, const char *str2 )
{
	const char *result = NULL;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( str1 );
	assert_non_null( str2 );

	/* perform strpbrk */
	while ( *str1 != '\0' && !result )
	{
		const char *c = str2;
		while ( *c != '\0' && !result )
		{
			if ( *(c++) == *str1 )
			{
				result = str1;
			}
		}
		++str1;
	}
#ifdef __GNUC__
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	return (char *)result;
#ifdef __GNUC__
#	pragma GCC diagnostic pop
#endif
}

char *__wrap_os_strrchr( const char *s, int c )
{
	const char *result = NULL;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( s );

	/* perform strrchr */
	while ( *s )
	{
		if ( (int)*s == c )
			result = s;
		s++;
	}

#ifdef __GNUC__
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	return (char *)result;
#ifdef __GNUC__
#	pragma GCC diagnostic pop
#endif
}

char *__wrap_os_strstr( const char *str1, const char *str2 )
{
	const char *result = NULL;

	assert_non_null( str1 );
	assert_non_null( str2 );

	while ( *str1 && result == NULL )
	{
		const char *str1p = str1;
		const char *str2p = str2;
		while ( *str2p && *str1p == *str2p )
		{
			str1p++;
			str2p++;
		}
		if ( !( *str2p ) )
			result = str1;
		str1++;
	}
#ifdef __GNUC__
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wcast-qual"
#endif
	return (char *)result;
#ifdef __GNUC__
#	pragma GCC diagnostic pop
#endif
}

double __wrap_os_strtod( const char *str, char **endptr )
{
	double result;

	assert_non_null( str );
	result = mock_type( double );
	return result;
}

char* __wrap_os_strtok( char* s, const char* delm )
{
	static char *str = NULL;
	char *rv = NULL;

	if ( s )
		str = s;

	if ( str && delm )
	{
		while ( *str && rv == NULL )
		{
			const char *d = delm;
			while ( *d && rv == NULL )
			{
				if ( *str == *d )
					rv = str;
				++d;
			}
			++str;
		}
	}
	return rv;
}

long __wrap_os_strtol( const char *str, char **endptr )
{
	long result;

	assert_non_null( str );
	result = mock_type( long );
	return result;
}

unsigned long __wrap_os_strtoul( const char *str, char **endptr )
{
	unsigned long result;

	assert_non_null( str );
	result = mock_type( unsigned long );
	return result;
}

int __wrap_os_system_error_last( void )
{
	return 123;
}

const char *__wrap_os_system_error_string( int error_number )
{
	return "internal error";
}

os_uint32_t __wrap_os_system_pid( void )
{
	return (os_uint32_t)mock();
}

os_status_t __wrap_os_system_run(
	const char *command,
	int *exit_status,
	os_bool_t privileged,
	int priority,
	size_t stack_size,
	os_file_t pipe_files[2u] )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( command );
	check_expected( command );
	assert_non_null( exit_status );

	if ( exit_status )
		*exit_status = mock_type( int );
	return mock_type( os_status_t );
}

os_status_t __wrap_os_system_run_wait(
	const char *command,
	int *exit_status,
	os_bool_t privileged,
	int priority,
	size_t stack_size,
	char *out_buf[2u],
	size_t out_len[2u],
	os_millisecond_t max_time_out )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( command );
	check_expected( command );
	assert_non_null( exit_status );
	assert_non_null( out_buf );
	assert_non_null( out_len );

	if ( exit_status )
		*exit_status = mock_type( int );
	if ( out_buf[0u] )
		strncpy( out_buf[0u], mock_type( char * ), out_len[0u] );
	if ( out_buf[1u] )
		strncpy( out_buf[1u], mock_type( char * ), out_len[1u] );
	return mock_type( os_status_t );
}

os_bool_t __wrap_os_terminal_vt100_support( os_file_t stream )
{
	return mock_type( os_bool_t );
}

#ifdef IOT_THREAD_SUPPORT
os_status_t __wrap_os_thread_condition_broadcast( os_thread_condition_t *cond )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( cond );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_condition_create( os_thread_condition_t *cond )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( cond );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_condition_destroy( os_thread_condition_t *cond )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( cond );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_condition_signal(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( cond );
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_condition_wait(
	os_thread_condition_t *cond,
	os_thread_mutex_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( cond );
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_create(
	os_thread_t *thread, os_thread_main_t main, void *arg, size_t stack_size )
{
	static unsigned int thread_id = 1u;
	os_status_t result = (os_status_t)mock();
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( thread );
	assert_non_null( main );
	if ( result == OS_STATUS_SUCCESS )
	{
		thread_id++;
		*thread = (os_thread_t)thread_id;
		/* try and call thread's main */
		(*main)( arg );
	}
	return result;
}

os_status_t __wrap_os_thread_destroy( os_thread_t *thread )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( thread );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_mutex_create( os_thread_mutex_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_mutex_destroy( os_thread_mutex_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_mutex_lock( os_thread_mutex_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_mutex_unlock( os_thread_mutex_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_rwlock_create( os_thread_rwlock_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_rwlock_destroy( os_thread_rwlock_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_rwlock_read_lock( os_thread_rwlock_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_rwlock_read_unlock( os_thread_rwlock_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_rwlock_write_lock( os_thread_rwlock_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_rwlock_write_unlock( os_thread_rwlock_t *lock )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( lock );
	return OS_STATUS_FAILURE;
}

os_status_t __wrap_os_thread_wait( os_thread_t *thread )
{
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( thread );
	return OS_STATUS_FAILURE;
}
#endif /* ifdef IOT_THREAD_SUPPORT */

int __wrap_os_printf( const char *format, ... )
{
	va_list args;
	int result;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( format );

	va_start( args, format );
	result = vprintf( format, args );
	va_end( args );
	return result;
}

int __wrap_os_snprintf( char *str, size_t size, const char *format, ... )
{
	va_list args;
	int result;

	/* ensure this function is called meeting pre-requirements */
	assert_non_null( str );
	assert_non_null( format );

	va_start( args, format );
	result = vsnprintf( str, size, format, args );
	va_end( args );
	return result;
}

int __wrap_os_vfprintf( os_file_t stream, const char *format, va_list args )
{
	int result = -1;
#ifdef _WIN32
	char str[1024u];
	char *str_end = NULL;
#endif
	/* ensure this function is called meeting pre-requirements */
	assert_non_null( format );

#ifdef _WIN32
	if ( StringCchVPrintfEx( str, 1024u, &str_end, NULL, 0,
		format, args ) == S_OK )
	{
		DWORD numberOfBytesWritten;
		result = str_end - str;
		if ( !WriteFile( stream, str, (DWORD)result,
			&numberOfBytesWritten, NULL ) )
			result = -1;
	}
#else
	result = vfprintf( stream, format, args );
#endif
	return result;
}

int __wrap_os_vsnprintf( char *str, size_t size, const char *format, va_list args )
{
	/* ensure this function is called meeting pre-requirements */
	if ( size > 0u )
		assert_non_null( str );
	assert_non_null( format );
	return vsnprintf( str, size, format, args );
}

os_status_t __wrap_os_time( os_timestamp_t *time_stamp, os_bool_t *up_time )
{
	*time_stamp = 1234567u;
	return OS_STATUS_SUCCESS;
}

os_status_t __wrap_os_time_sleep( os_millisecond_t ms, os_bool_t allow_interrupts )
{
	return OS_STATUS_SUCCESS;
}

os_status_t __wrap_os_uuid_generate( os_uuid_t *uuid )
{
	return OS_STATUS_SUCCESS;
}

os_status_t __wrap_os_uuid_to_string_lower( os_uuid_t *uuid, char *dest, size_t len )
{
	if ( dest )
		strncpy( dest, "12345678-abcd-dcba-abcd-567812345678", len );
	return OS_STATUS_SUCCESS;
}
