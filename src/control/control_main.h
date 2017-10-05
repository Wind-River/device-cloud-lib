/**
 * @file
 * @brief Main source file for the Wind River IoT control application
 *
 * @copyright Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license The right to copy, distribute or otherwise make use of this software
 * may be licensed only pursuant to the terms of an applicable Wind River
 * license agreement.  No license to Wind River intellectual property rights is
 * granted herein.  All rights not licensed by Wind River are reserved by Wind
 * River.
 */

#ifndef CONTROL_MAIN_H
#define CONTROL_MAIN_H

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
#include "iot.h"                      /* for iot types */

/**
 * @brief Main entry-point for the application
 *
 * @param[in]      argc                          number of arguments passed to
 *                                               the application
 * @param[in]      argv                          array of arguments passed to
 *                                               the application
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
int control_main( int argc, char* argv[] );

/**
 * @brief Shutdown function
 *
 * @param[in]      shutdown                      boolean where true is
 *                                               reboot and false is shutdown
 * @param[in]      delay                         delay in seconds
 *
 * @retval EXIT_SUCCESS      application completed successfully
 * @retval EXIT_FAILURE      application encountered an error
 */
int control_device_shutdown( iot_bool_t shutdown, unsigned int delay);

#endif /* ifndef CONTROL_MAIN_H */

