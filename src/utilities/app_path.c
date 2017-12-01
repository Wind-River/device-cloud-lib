/**
 * @file
 * @brief source file for path helper operations for applications
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

#include "app_path.h"

#include "os.h"           /* for os_* functions */
#include "iot_build.h"    /* for IOT_BIN_DIR */

/** @brief Maximum path length for app_path_create */
#define APP_PATH_CREATE_MAX_LEN 128u

iot_status_t app_path_create(
	const char *path_in,
	unsigned int timeout )
{
	char path[APP_PATH_CREATE_MAX_LEN + 1u];
	char *token;
	char directory[ APP_PATH_CREATE_MAX_LEN + 1u ];
	iot_status_t result = IOT_STATUS_SUCCESS;

	os_strncpy( path, path_in, APP_PATH_CREATE_MAX_LEN );

	if(path[0] == OS_DIR_SEP)
	{
		directory[0] = OS_DIR_SEP;
		directory[1] = '\0';
	}
	else
		directory[0] = '\0';

	token = os_strtok( path, "/\\" );

	while( token != NULL && result == IOT_STATUS_SUCCESS )
	{
		size_t offset = os_strlen( directory );
		os_snprintf( directory + offset,
			APP_PATH_CREATE_MAX_LEN - offset,
			"%s%c", token, OS_DIR_SEP );
		directory[APP_PATH_CREATE_MAX_LEN] = '\0';

		token = os_strtok( NULL, "/\\" );

		if( token != NULL && *token != '\0' &&
			os_directory_exists( directory ) == IOT_FALSE )
		{
			if ( os_directory_create( directory, timeout ) !=
				OS_STATUS_SUCCESS )
				result = IOT_STATUS_FAILURE;
		}
	}
	return result;
}

size_t app_path_make_absolute( char *path, size_t path_max,
	iot_bool_t relative_to_install )
{
	size_t result = 0u;
	if ( path && *path != '\0' )
	{
		/* convert any environment variables in the path */
		result = os_env_expand( path, path_max );
		if ( result < path_max )
		{
			result = os_strlen( path );

			/* if not an absolute path, then we must prepend the
			 * proper directory
			 */
			if ( os_path_is_absolute( path ) == IOT_FALSE )
			{
				char prepend_path[ PATH_MAX + 1u ];
				size_t prepend_len = 0u;
				if ( relative_to_install == IOT_FALSE )
				{
					if ( os_directory_current(
						prepend_path, PATH_MAX )
						== OS_STATUS_SUCCESS )
						prepend_len =
							os_strlen( prepend_path );
					else
						result = 0u;
				}
				else
				{
					if ( os_path_executable(
						prepend_path, PATH_MAX )
							== OS_STATUS_SUCCESS )
					{
						const size_t bin_len =
							os_strlen( IOT_BIN_DIR );
						prepend_len =
							os_strlen( prepend_path );
						/* remove executable name */
						while ( prepend_len > 0u &&
							prepend_path[prepend_len]
								!= OS_DIR_SEP )
							--prepend_len;
						prepend_path[prepend_len] = '\0';

						/* calculate length without
						 * "bin" directory from
						 * application path */
						if ( prepend_len >= bin_len + 1u
							&& os_strncmp(
							&prepend_path[prepend_len-bin_len],
							IOT_BIN_DIR, bin_len ) == 0 &&
							prepend_path[prepend_len-bin_len-1] ==
							OS_DIR_SEP )
						{
							prepend_len -= bin_len;
							prepend_path[prepend_len] = '\0';
						}
					}
					else
						result = 0u;
				}

				/* prepend path */
				if ( result > 0u && prepend_len > 0u )
				{
					/* add directory seperator */
					if ( prepend_path[prepend_len - 1u]
						!= OS_DIR_SEP )
					{
						prepend_path[prepend_len]
							= OS_DIR_SEP;
						++prepend_len;
					}

					/* prepend path to result */
					if ( result + prepend_len < path_max )
					{
						os_memmove(
							&path[prepend_len],
							&path[0], result );
						os_memcpy( &path[0],
							prepend_path,
							prepend_len );

						/* add null-terminator */
						path[result + prepend_len] = '\0';
					}
					result += prepend_len;
				}
			}
		}
	}
	return result;
}

/**
 * @def EXT_LIST_MAX
 * @brief Maximum length for the environment variable containing extensions
 */
#define EXT_LIST_MAX 63u
/**
 * @def FILE_NAME_MAX
 * @brief Maximum length for the file name to find
 */
#define FILE_NAME_MAX 63u

size_t app_path_which( char *path, size_t path_max, const char *cur_dir,
	const char *file_name )
{
	size_t result = 0u;

	if ( file_name )
	{
		char exts[ EXT_LIST_MAX + 1u ];
		char exts_copy[ EXT_LIST_MAX + 1u ];
		char *exts_cur;
		const char *dir;
		char dirs[ PATH_MAX + 1u ];
		char *dirs_cur;
		iot_bool_t match = IOT_FALSE;
		char test_path[ PATH_MAX + 1u ];

		if ( !cur_dir )
			cur_dir = ".";

		os_env_get( "PATH", dirs, PATH_MAX );
		os_env_get( "PATHEXT", exts, EXT_LIST_MAX );

		dir = cur_dir;
		dirs_cur = dirs;
		do
		{
			const char *ext = "";
			/* we want to work in a copy because we will
			 * modify the the array with '\0' as it is
			 * walked */
			os_strncpy( exts_copy, exts, EXT_LIST_MAX );
			exts_cur = exts_copy;
			do
			{
				char test_file_name[FILE_NAME_MAX + 1u];
				if ( *ext != '\0' && *ext != '.' )
					os_snprintf( test_file_name,
						FILE_NAME_MAX,
						"%s.%s", file_name, ext );
				else
					os_snprintf( test_file_name,
						FILE_NAME_MAX,
						"%s%s", file_name, ext );
				test_file_name[FILE_NAME_MAX] = '\0';

				/* test built path */
				os_make_path( test_path, PATH_MAX,
					dir, test_file_name, NULL );
				test_path[ PATH_MAX ] = '\0';
				match = os_file_exists( test_path );

				/* move to next extension */
				ext = exts_cur;
				while( *exts_cur != '\0' &&
					*exts_cur != OS_ENV_SPLIT )
					++exts_cur;

				/* if not last item, add null-terminator */
				if ( *exts_cur != '\0' )
				{
					*exts_cur = '\0';
					++exts_cur;
				}
			} while ( *ext != '\0' && match == IOT_FALSE );

			/* move to next directory */
			dir = dirs_cur;
			while( *dirs_cur != '\0' &&
				*dirs_cur != OS_ENV_SPLIT )
				++dirs_cur;

			/* if not last item, add null-terminator */
			if ( *dirs_cur != '\0' )
			{
				*dirs_cur = '\0';
				++dirs_cur;
			}
		} while ( *dir != '\0' && match == IOT_FALSE );

		if ( match == IOT_FALSE )
			test_path[0] = '\0';

		/* copy result */
		result = os_strlen( test_path );
		if ( path )
		{
			os_strncpy( path, test_path, path_max );
			test_path[ path_max - 1u ] = '\0';
		}
	}
	return result;
}

iot_status_t app_path_executable_directory_get(
	char *path, const size_t size )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( path )
	{
		char exe_path[ PATH_MAX + 1u ];
		os_memzero( exe_path, PATH_MAX + 1u );
		os_memzero( path, size );
		result = IOT_STATUS_FAILURE;
		if ( os_path_executable( exe_path, PATH_MAX ) ==
			OS_STATUS_SUCCESS )
		{
			char *dir_sep = NULL;
			dir_sep = os_strrchr( exe_path, OS_DIR_SEP );
			if ( dir_sep )
			{
				size_t dir_size = (size_t)(dir_sep - exe_path);
				if ( dir_size < size )
				{
					os_strncpy( path, exe_path, dir_size );
					path[ dir_size ] = '\0';
					result = IOT_STATUS_SUCCESS;
				}
			}
		}
	}
	return result;
}
