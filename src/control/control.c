/**
 * @file
 * @brief Main source file for the Wind River IoT control application
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

#include "control_main.h"

/**
 * @brief Main entry-point for the application
 *
 * The body of this function is actually in another function, this is to allow
 * for full unit testing of the application.  The unit test would not work if
 * there were multiple "main" functions; i.e. if there was one for the entry
 * point and another for the unit test entry point.  Thus, this function is
 * just a wrapper around a starting block named other than "main".
 *
 * @param[in]      argc                          number of arguments passed to
 *                                               the application
 * @param[in]      argv                          array of arguments passed to
 *                                               the application
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
int main( int argc, char* argv[] )
{
	return control_main( argc, argv );
}

