/**
 * @file
 * @brief Main source file for the Wind River IoT control configuration files
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
#ifndef CONTROL_CONFIG_H

#include "utilities/app_json_schema.h"
#include "utilities/app_path.h"

/**
 * @brief Generates a configuration file for the product by prompting for
 *        user input
 *
 * @param[in]      file_name           file name to generate (optional)
 *
 * @retval IOT_STATUS_FAILURE          operating system operation failed
 * @retval IOT_STATUS_NO_MEMORY        not enough memory available
 * @retval IOT_STATUS_SUCCESS          on success
 */
iot_status_t control_config_generate( const char *file_name );

#endif /* ifndef CONTROL_CONFIG_H */
