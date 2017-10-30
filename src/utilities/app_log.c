/**
 * @file
 * @brief Implementation of common log functionality for applications
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#include "app_log.h"
#include "os.h"

#ifdef IOT_LOG_TIMESTAMP
/** @brief Length of the formatted time stamp */
#define APP_LOG_TIMESTAMP_LENGTH  16
#endif /* ifdef IOT_LOG_TIMESTAMP */
/** @brief Maximum number of times to repeat log message */
#define LOG_MESSAGE_REPEAT_MAX    4294967295u

void app_log( iot_log_level_t log_level, const iot_log_source_t *source,
	const char *message, void *UNUSED(user_data) )
{
	const char *file_name = "";
	int fg_colour;
	unsigned int i = 0u;
#ifndef _WRS_KERNEL
	unsigned long hash = 5381u;
	static unsigned int  last_msg_count = 0u;
	static unsigned long last_msg_hash = 0u;
#endif /* ifndef _WRS_KERNEL */
	unsigned int line_number = 0u;
	const char *const prefix[] =
		{ "Fatal", "Alert", "Critical", "Error", "Warning", "Notice",
		  "Info", "Debug", "Trace" };

#ifndef _WRS_KERNEL
	/* create hash of message (to detect if this message is a repeat) */
	{
		const char *str = message;
		char c;

		while ( (c = *str++) != '\0' )
			/* hash * 33 + c */
			hash = ((hash << 5) + hash) + (unsigned long)c;
	}

	if ( hash == last_msg_hash && last_msg_count < LOG_MESSAGE_REPEAT_MAX )
		++last_msg_count;
	else
	{

#ifdef IOT_LOG_TIMESTAMP
		char timestamp[APP_LOG_TIMESTAMP_LENGTH  + 1u];
#endif /* ifdef IOT_LOG_TIMESTAMP */

		/* print if last message has been repeated many times */
		if ( last_msg_count > 0u )
		{
			os_fprintf( OS_STDERR, "[repeated %u times]\n",
				last_msg_count );
			last_msg_count = 0u;
		}
#ifdef IOT_LOG_TIMESTAMP
		/* Print time stamp */
		os_time_format( timestamp, APP_LOG_TIMESTAMP_LENGTH  );
		os_fprintf( OS_STDERR, "%s ", timestamp );
#endif

#endif /* ifndef _WRS_KERNEL */
		/* ensure function name points to a string */
		if ( source && source->file_name )
		{
			file_name = os_strrchr(
				source->file_name, OS_DIR_SEP );
			if ( file_name )
				++file_name;
			else
				file_name = source->file_name;
			line_number = source->line_number;
		}

		/* ensure log level is a valid enumeration value */
		if ( (unsigned int)log_level <= IOT_LOG_TRACE )
			i = (unsigned int)log_level;

		if ( log_level <= IOT_LOG_ERROR ) /* FATAL, ALERT, CRITICAL, ERROR */
			fg_colour = 1; /* red */
		else if ( log_level <= IOT_LOG_NOTICE ) /* WARNING, NOTICE */
			fg_colour = 3; /* yellow */
		else if ( log_level <= IOT_LOG_INFO ) /* INFO */
			fg_colour = 2; /* green */
		else if ( log_level <= IOT_LOG_DEBUG ) /* DEBUG */
			fg_colour = 4; /* blue */
		else /* TRACE, NONE */
			fg_colour = 6; /* cyan */

		if ( os_terminal_vt100_support( OS_STDERR ) != IOT_FALSE )
			os_fprintf( OS_STDERR, "\033[1;3%dm%s\033[0m ",
				fg_colour, prefix[i] );
		else
			os_fprintf( OS_STDERR, "%s ", prefix[i] );
		os_fprintf( OS_STDERR, "- %s:%u - %s%s", file_name,
			line_number, message, OS_FILE_LINE_BREAK );
		os_flush( OS_STDOUT );
		os_flush( OS_STDERR );
#ifndef _WRS_KERNEL
		last_msg_hash = hash;
	}
#endif /* ifndef _WRS_KERNEL */
}

