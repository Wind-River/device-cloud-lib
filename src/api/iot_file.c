/**
 * @file
 * @brief source file containing file transfer implementation
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "public/iot.h"

#include "iot_build.h"             /* for IOT_RUNTIME_DIR_DEFAULT */
#include "iot_common.h"            /* for iot_common_arg_set */
#include "iot_plugin.h"            /* for plug-in support */

#include "shared/iot_types.h"      /* for struct iot */

#include <os.h>                    /* operating system abstraction */
#include <archive.h>               /* for archiving functions */
#include <archive_entry.h>         /* for adding files to an archive */

/**
 * @brief Default download subdirectory
 */
#define IOT_DEFAULT_DIR_DOWNLOAD       "download"

/**
 * @brief Default update subdirectory
 */
#define IOT_DEFAULT_DIR_UPLOAD         "upload"

/**
 * @brief Transfer a file/directory to/from cloud
 *
 * @param[in]      lib                 library handle
 * @param[out]     txn                 transaction status (optional)
 * @param[in]      options             options for the file transfer (optional)
 * @param[in]      file_name           cloud's file name (optional)
 *                                     if file name is not given, local file
 *                                     name will be used, if it is a directory,
 *                                     the full path will be used where all
 *                                     the separators are replaced with dashes
 * @param[in]      file_path           location of the local file.
 *                                     if path is not absolute, it will
 *                                     be relative to default directory.
 *                                     if it is a directory instead of a file,
 *                                     all files within that directory will be
 *                                     bundled together.
 * @param[in]      func                callback function to give
 *                                     progress update (optional)
 *                                     if none is given, progress will
 *                                     be printed in the logs at
 *                                     a regular interval
 * @param[in]      user_data           user's specific data for progress
 *                                     callback (optional)
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
static iot_status_t iot_file_transfer(
	iot_t *lib,
	iot_transaction_t *txn,
	const iot_options_t *options,
	iot_operation_t op,
	const char * const file_name,
	const char * const file_path,
	iot_file_progress_callback_t *func,
	void *user_data );

/**
 * @brief Build an archive from a directory for the cloud
 *
 * @param[in]      archive_path        name of the archive file to produce
 * @param[in]      path                path to the directory to archive
 *
 * @retval IOT_STATUS_BAD_PARAMETER    invalid parameter passed
 * @retval IOT_STATUS_FAILURE          internal system failure
 * @retval IOT_STATUS_SUCCESS          operation successful
 */
static iot_status_t iot_file_archive_directory(
	const char *archive_path,
	const char *path );


iot_status_t iot_file_archive_directory(
	const char *archive_path,
	const char *path )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( archive_path && path )
	{
		os_dir_t *dir = NULL;
		struct archive *archive;

		/* compress all files in upload directories */
		archive = archive_write_new();
		archive_write_add_filter_none( archive );
		archive_write_set_format_pax_restricted( archive );
		archive_write_open_filename( archive, archive_path );

		if ( ( dir = os_directory_open( path ) ) != NULL )
		{
			char file_name[ PATH_MAX + 1u ];
			iot_bool_t walk_directory = IOT_TRUE;

			while ( walk_directory )
			{
				char file_path[ PATH_MAX + 1u ];
				os_file_t input_file;
				struct stat file_stat;

				os_directory_next( dir, IOT_TRUE, file_name, PATH_MAX );

				os_snprintf( file_path, PATH_MAX, "%s%c%s", path, OS_DIR_SEP, file_name );

				stat( file_name, &file_stat );
				input_file = os_file_open( file_name, OS_READ );
				if ( input_file )
				{
					/* create an archive of all files */
					char buff[1024];
					size_t buff_len;
					struct archive_entry *entry;

					entry = archive_entry_new();

					/* Note: set the file details individually.  Calling archive_entry_copy_stat(
					 * entry, &file_stat ) is easier but it corrupts archives on 32b architectures,
					 * e.g. quark. */
					archive_entry_set_size(entry, file_stat.st_size );
					archive_entry_set_filetype(entry, AE_IFREG );
					archive_entry_set_perm(entry, 0644 );
					archive_entry_set_pathname( entry, file_name );

					/* stat struct does not store
					 * the birthtime.  That is part of the file system */
					archive_entry_set_atime( entry, file_stat.st_atime, 0 );
					archive_entry_set_birthtime( entry, file_stat.st_ctime, 0 );
					archive_entry_set_ctime( entry, file_stat.st_ctime, 0 );
					archive_entry_set_mtime( entry, file_stat.st_mtime, 0 );

					archive_write_header( archive, entry );
					while ( ( buff_len = os_file_read( buff, 1, sizeof( buff ), input_file ) ) )
						archive_write_data( archive, buff, buff_len );

					archive_entry_free( entry );
					os_file_close( input_file );
					result = IOT_STATUS_SUCCESS;
				}
				else
					walk_directory = IOT_FALSE;
			}
			os_directory_close( dir );
		}

		archive_write_close( archive );
		archive_write_free( archive );
	}
	return result;
}

iot_status_t iot_file_download(
	iot_t *lib,
	iot_transaction_t *txn,
	const iot_options_t *options,
	const char *file_name,
	const char *file_path,
	iot_file_progress_callback_t *func,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lib && file_path )
		result = iot_file_transfer( lib, txn, options,
			IOT_OPERATION_FILE_DOWNLOAD, file_name, file_path,
			func, user_data );
	return result;
}

iot_status_t iot_file_transfer(
	iot_t *lib,
	iot_transaction_t *txn,
	const iot_options_t *options,
	iot_operation_t op,
	const char * const file_name,
	const char * const file_path,
	iot_file_progress_callback_t *func,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lib )
	{
		char *heap_name = NULL;
		char *heap_path = NULL;
		iot_file_transfer_t transfer;

		result = IOT_STATUS_FAILURE;
		os_memzero( &transfer, sizeof( iot_file_transfer_t ) );
		transfer.callback = func;
		transfer.user_data = user_data;

		/* Use default directory if the path provided is not absolute */
		/** @todo update this to be a real absolute path check */
		if ( file_path && file_path[0] == OS_DIR_SEP )
			transfer.path = file_path;
		else
		{
			const char *subdir = IOT_DEFAULT_DIR_DOWNLOAD;
			size_t heap_len = 0u;

			if ( op == IOT_OPERATION_FILE_UPLOAD )
				subdir = IOT_DEFAULT_DIR_UPLOAD;

			heap_len += iot_directory_name_get(
				IOT_DIR_RUNTIME, NULL, 0u );
			/* +1 for directory seperator */
			heap_len += os_strlen( subdir ) + 1u;
			/* +1 for directory seperator */
			/* if no file path was specified, use the
			 * default path, but don't strlen it */
			if ( file_path )
				heap_len += os_strlen( file_path ) + 1u;

			heap_path = os_malloc( heap_len + 1u );
			if ( heap_path )
			{
				size_t dir_len;
				dir_len = iot_directory_name_get(
					IOT_DIR_RUNTIME, heap_path,
					heap_len );
				os_snprintf( &heap_path[dir_len],
					heap_len - dir_len + 1,
					"%c%s%c%s",
					OS_DIR_SEP, subdir,
					OS_DIR_SEP, file_path );
				heap_path[heap_len] = '\0';
				transfer.path = heap_path;
			}
		}

		/* Use the file_name to rename it on the cloud */
		if ( file_name && file_name[0] != '\0' )
			transfer.name = file_name;
		else
		{
			/* if it's a directory, use the path's name
			 * with dashes and tar extension */
			if ( os_directory_exists( transfer.path ) )
			{
				const char *const ext = ".tar";
				size_t path_len = os_strlen(
					transfer.path );
				const size_t heap_len = path_len +
					os_strlen( ext );

				heap_name = os_malloc( heap_len + 1u );
				if ( heap_name )
				{
					char *pos;

					os_strncpy( heap_name, transfer.path,
						heap_len );

					/* remove ending slashes */
					while ( path_len > 0u &&
						heap_name[path_len - 1u] == OS_DIR_SEP )
					{
						heap_name[path_len - 1u] = '\0';
						--path_len;
					}

					/* add extension to end of name */
					os_strncpy( &heap_name[path_len],
						ext, heap_len - path_len + 1u);
					pos = heap_name;

#ifdef _WIN32
					/* if string starts with "C:" */
					if ( heap_len > 1u &&
						pos[1] == ':' )
						pos += 2;
#endif /* ifdef _WIN32 */

					/* remove root directory seperators */
					while ( pos && *pos == OS_DIR_SEP )
						++pos;

					/* convert any directory seperators to
					   dashes */
					transfer.name = pos;
					while( pos && *pos != '\0' )
					{
						if ( *pos == OS_DIR_SEP )
							*pos = '-';
						++pos;
					}
				}
			}
			else
				transfer.name = os_strrchr( transfer.path,
					OS_DIR_SEP ) + 1u;
		}
		if ( op == IOT_OPERATION_FILE_UPLOAD )
		{
			if ( os_directory_exists( transfer.path ) )
			{
				size_t archive_len = 0u;
				char *archive_path = NULL;
				const char *const ext = ".tar";
				const char *const file_name_format =
					"fileXXXXXX";

				/* get runtime directory name */
				archive_len = iot_directory_name_get(
					IOT_DIR_RUNTIME, NULL, 0u );

				/* +1 for directory seperator */
				archive_len += os_strlen( file_name_format ) +
					os_strlen( ext ) + 1u;

				/* get a temporary file name */
				archive_path = os_malloc( archive_len + 1u );

				result = IOT_STATUS_NO_MEMORY;
				if ( archive_path )
				{
					const size_t dir_len =
						iot_directory_name_get(
							IOT_DIR_RUNTIME,
							archive_path,
							archive_len );
					os_snprintf( &archive_path[dir_len],
						archive_len - dir_len + 1u,
						"%c%s%s", OS_DIR_SEP,
						file_name_format, ext );
					archive_path[ archive_len ] = '\0';
					os_file_temp( archive_path,
						os_strlen(ext) );

					result = iot_file_archive_directory(
						archive_path, transfer.path );
					if ( result == IOT_STATUS_SUCCESS )
					{
						transfer.path = archive_path;
						os_free( heap_path );
						heap_path = archive_path;
					}
					else
						os_free( archive_path );
				}
			}
			else if ( os_file_exists( transfer.path ) )
				result = IOT_STATUS_SUCCESS;
			else
				IOT_LOG( lib, IOT_LOG_ERROR,
					"File does not exist: %s",
					transfer.path );
		}
		else
		{
			/* Check if the directory exists and writable */
			char dir[ PATH_MAX ];
			os_strncpy( dir, transfer.path, PATH_MAX );
			*os_strrchr( dir, OS_DIR_SEP ) = '\0';

			result = IOT_STATUS_SUCCESS;
			if ( os_file_exists( dir ) == OS_FALSE )
			{
				/* Create it if it doesn't exists */
				IOT_LOG( lib, IOT_LOG_INFO,
					"Creating directory: %s", dir );
				if ( os_directory_create_nowait( dir ) !=
					OS_STATUS_SUCCESS )
				{
					IOT_LOG( lib, IOT_LOG_ERROR,
						"Failed to create directory: %s",
						dir );
					result = IOT_STATUS_FAILURE;
				}
			}
		}

		/* send the request */
		if ( result == IOT_STATUS_SUCCESS )
			result = iot_plugin_perform( lib,
				txn, NULL, op, &transfer, NULL, options );

		/* clean up memory on the heap */
		if ( heap_name )
			os_free( heap_name );
		if ( heap_path )
			os_free( heap_path );
	}
	return result;
}

iot_status_t iot_file_progress_get(
	const iot_file_progress_t *progress,
	iot_status_t *status,
	iot_float32_t *percentage,
	iot_bool_t *is_completed )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;
	if ( progress )
	{
		if ( status )
			*status = progress->status;
		if ( percentage )
			*percentage = progress->percentage;
		if ( is_completed )
			*is_completed = progress->completed;
		result = IOT_STATUS_SUCCESS;
	}
	return result;
}

iot_status_t iot_file_upload(
	iot_t *lib,
	iot_transaction_t *txn,
	const iot_options_t *options,
	const char *file_name,
	const char *file_path,
	iot_file_progress_callback_t *func,
	void *user_data )
{
	iot_status_t result = IOT_STATUS_BAD_PARAMETER;

	if ( lib  )
		result = iot_file_transfer( lib, txn, options,
			IOT_OPERATION_FILE_UPLOAD, file_name, file_path,
			func, user_data );
	return result;
}

