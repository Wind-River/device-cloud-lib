/**
 * @file
 * @brief header file for argument parsing functionality for an application
 *
 * @copyright Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
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

#ifndef APP_ARG_H
#define APP_ARG_H

#include <stdlib.h> /* for size_t */
#include "os.h"     /* osal functions */

/** @brief Structure defining an arugment for an application */
struct app_arg
{
	/** @brief Argument single character token (i.e. specified with "-") */
	char ch;
	/** @brief Argument multi character token (i.e. specified with "--") */
	const char *name;
	/** @brief Whether the argument is required */
	int req;
	/** @brief Parameter id used in display */
	const char *param;
	/** @brief Location set pointer if found */
	const char **param_result;
	/** @brief Argument description */
	const char *desc;
	/** @brief Number of times argument was found */
	unsigned int hit;
};

/**
 * @brief Returns the number of times an argument was specified
 *
 * @param[in]      args                pointer to the application arguments
 *                                     array
 * @param[in]      ch                  single character token (optional)
 * @param[in]      name                multiple character token (optional)
 *
 * @return the number of times that the argument was specified
 *
 * @see app_arg_parse
 * @see app_arg_usage
 */
unsigned int app_arg_count( const struct app_arg *args, char ch,
	const char *name );

/**
 * @brief Parses arguments passed to the application
 *
 * @param[in,out]  args                array specifying arguments expected by
 *                                     the application
 * @param[in]      argc                number of arguments passed to the
 *                                     application
 * @param[in]      argv                array of arguments passed to the
 *                                     application
 * @param[in,out]  pos                 index of first positional argument
 *                                     (optional)
 *
 * @note This function doesn't check for optional or required positional
 * arguments
 *
 * @note If no positional arguments are found then @p pos will return a value
 * of 0
 *
 * @retval EXIT_FAILURE                application encountered an error
 * @retval EXIT_SUCCESS                application completed successfully
 *
 * @see app_arg_count
 * @see app_arg_usage
 */
int app_arg_parse( struct app_arg *args, int argc, char **argv,
	int *pos );

/**
 * @brief Prints to stdout which arguments will be handled by the application
 *
 * @param[in]      args                array specifying arguments expected by
 *                                     the application
 * @param[in]      col                 column index at which to align argument
 *                                     descriptions
 * @param[in]      app                 name of the application (optional)
 * @param[in]      desc                description of the application (optional)
 * @param[in]      pos                 id name to use for positional arguments
 *                                     (optional)
 * @param[in]      pos_desc            description for positional arguments
 *                                     (optional)
 *
 * @note @b pos can be of the format "id", "[id]", "id+" or "[id]+", where 'id'
 * is the name to use for the argument. Enclosing the id in brackets '[]'
 * indicates that positional argument(s) are optional.  A plus '+' at the end
 * indicates that multiple positional arguments are able to be handled.
 *
 * @see app_arg_count
 * @see app_arg_parse
 */
void app_arg_usage( const struct app_arg *args, size_t col,
	const char *app, const char *desc, const char *pos,
	const char *pos_desc );

#endif /* ifndef APP_ARG_H */

