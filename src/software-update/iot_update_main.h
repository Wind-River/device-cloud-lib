/**
 * @file
 * @brief Main source file for the Wind River IoT software update application
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

#ifndef IOT_UPDATE_MAIN_H
#define IOT_UPDATE_MAIN_H

/**
 * @brief Main entry-point for the application
 *
 * @param[in]      argc                number of arguments passed to the
 *                                     application
 * @param[in]      argv                array of arguments passed to the
 *                                     application
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
int iot_update_main( int argc, char* argv[] );

#endif /* ifndef IOT_UPDATE_MAIN_H */

