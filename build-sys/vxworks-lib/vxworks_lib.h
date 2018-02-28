/**
 * @file
 * @brief Function definitions for global VxWorks specific settings
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
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

#if !defined(VXWORKS_LIB_H)
#define VXWORKS_LIB_H 1

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

#define remainder fmod

#if !defined(_WRS_KERNEL)
/**
 * @brief Sets the configuration directory for the device cloud
 *
 * This is the location where configuration files for the device cloud are
 * found on the system
 *
 * @param[in]      str                 system path to set
 *
 * @see deviceCloudConfigDirGet
 */
extern void deviceCloudConfigDirSet( const char *str );
/**
 * @brief Sets the runtime directory for the device cloud
 *
 * This is the location where runtime files required for the device cloud are
 * stored on the system
 *
 * @param[in]      str                 system path to set
 *
 * @see deviceCloudRuntimeDirGet
 */
extern void deviceCloudRuntimeDirSet( const char *str );
/**
 * @brief Sets the binary (executable) directory for the device cloud
 *
 * This is the location where binary (executable) files required for the device
 * cloud are stored on the system
 *
 * @param[in]      str                 system path to set
 *
 * @see deviceCloudBinDirGet
 */
extern void deviceCloudBinDirSet( const char *str );
/**
 * @brief Sets the task priority for the device cloud
 *
 * Sets the task priority for new tasks for the device cloud
 *
 * @param[in]      str                 priority to set
 *
 * @see deviceCloudPriorityGet
 */
extern void deviceCloudPrioritySet( const char *str );
/**
 * @brief Sets the task stack size for the device cloud
 *
 * Sets the task stack size for new tasks for the device cloud
 *
 * @param[in]      str                 stack size to set
 *
 * @see deviceCloudStackSizeGet
 */
extern void deviceCloudStackSizeSet( const char *str );
#endif /* _WRS_KERNEL */

/**
 * @brief Retrieves the configuration directory for the device cloud
 *
 * @return the location where configuration files for the device cloud are
 * found on the system
 *
 * @see deviceCloudConfigDirSet
 */
extern const char *deviceCloudConfigDirGet( void );
/**
 * @brief Retrieves the runtime directory for the device cloud
 *
 * @return the location where runtime files required for the device cloud are
 * stored on the system
 *
 * @see deviceCloudRuntimeDirSet
 */
extern const char *deviceCloudRuntimeDirGet( void );
/**
 * @brief Retrieves the binary (executable) directory for the device cloud
 *
 * @return the location where binary (executable) files required for the device
 * cloud are stored on the system
 *
 * @see deviceCloudBinDirSet
 */
extern const char *deviceCloudBinDirGet( void );
/**
 * @brief Retrieves the task priority for the device cloud
 *
 * @return the task priority for new tasks for the device cloud
 *
 * @see deviceCloudPrioritySet
 */
extern unsigned int deviceCloudPriorityGet( void );
/**
 * @brief Retrieves the task stack size for the device cloud
 *
 * @return the task stack size for new tasks for the device cloud
 *
 * @see deviceCloudStackSizeSet
 */
extern unsigned int deviceCloudStackSizeGet( void );

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* if !defined(VXWORKS_LIB_H) */
