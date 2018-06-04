/**
 * @file
 * @brief Header file for using the Internet of Things library
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
#ifndef IOT_PLUGIN_H
#define IOT_PLUGIN_H

#include <iot.h>

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/** @brief indicates with plug-in step in a plug-in task is currently being executed */
enum iot_step
{
	/** @brief step before executing the plug-in task (used or security validation) */
	IOT_STEP_BEFORE = 0,
	/** @brief step performing the plug-in task */
	IOT_STEP_DURING,
	/** @brief step called after performing the plug-in task */
	IOT_STEP_AFTER
};
/** @brief type defining the plug-in step */
typedef enum iot_step iot_step_t;

/** @brief Current operation being performed */
enum iot_operation
{
	/** @brief unknown/unsupported operation */
	IOT_OPERATION_UNKNOWN = 0,
	/** @brief ( up ) check if actions are available */
	IOT_OPERATION_ACTION_CHECK,
	/** @brief ( up ) action completed */
	IOT_OPERATION_ACTION_COMPLETE,
	/** @brief ( up ) action deregistration */
	IOT_OPERATION_ACTION_DEREGISTER,
	/** @brief ( up ) action registration */
	IOT_OPERATION_ACTION_REGISTER,
	/** @brief ( up ) publication of an attribute */
	IOT_OPERATION_ATTRIBUTE_PUBLISH,
	/** @brief ( up ) publication of an alarm event */
	IOT_OPERATION_ALARM_PUBLISH,
	/** @brief ( up ) client connect to cloud */
	IOT_OPERATION_CLIENT_CONNECT,
	/** @brief ( up ) client disconnecting from cloud */
	IOT_OPERATION_CLIENT_DISCONNECT,
	/** @brief ( up ) check if client still exists */
	IOT_OPERATION_CLIENT_HEARTBEAT,
	/** @brief ( up ) client-2-client message */
	IOT_OPERATION_CLIENT_MESSAGE,
	/** @brief ( up ) publication of an event log */
	IOT_OPERATION_EVENT_PUBLISH,
	/** @brief (up) get url to download a file from */
	IOT_OPERATION_FILE_DOWNLOAD,
	/** @brief (up) get url to upload a file to */
	IOT_OPERATION_FILE_UPLOAD,
	/** @brief ( up ) iteration */
	IOT_OPERATION_ITERATION,
	/** @brief ( up ) property publish */
	IOT_OPERATION_PROPERTY_PUBLISH,
	/** @brief ( up ) telemetry deregistration */
	IOT_OPERATION_TELEMETRY_DEREGISTER,
	/** @brief ( up ) publication of a telemetry sample(s) */
	IOT_OPERATION_TELEMETRY_PUBLISH,
	/** @brief ( up ) telemetry registration */
	IOT_OPERATION_TELEMETRY_REGISTER,
	/** @brief ( up ) obtain the transaction status */
	IOT_OPERATION_TRANSACTION_STATUS,
};

/** @brief current operation being performed */
typedef enum iot_operation iot_operation_t;

/** @brief typedef to simply function signatures */
typedef struct iot_plugin iot_plugin_t;

/** @brief defines the signature for the disable function in an IoT plug-in */
typedef iot_status_t (*iot_plugin_disable_fptr)(
	iot_t *lib,
	void* plugin_data,
	iot_bool_t force );
/** @brief defines the signature for the enable function in an IoT plug-in */
typedef iot_status_t (*iot_plugin_enable_fptr)(
	iot_t *lib,
	void* plugin_data );
/** @brief defines the signature for the main callback in an IoT plug-in */
typedef iot_status_t (*iot_plugin_execute_fptr)(
	iot_t *lib,
	void* plugin_data,
	iot_operation_t op,
	const iot_transaction_t *txn,
	iot_millisecond_t max_time_out,
	iot_step_t *step,
	const void *item,
	const void *value,
	const iot_options_t *options );
/** @brief defines the signature for the terminate function in an IoT plug-in */
typedef iot_status_t (*iot_plugin_terminate_fptr)(
	iot_t *lib,
	void *plugin_data );
/** @brief defines the signature for the initialize fuction in an IoT plug-in */
typedef iot_status_t (*iot_plugin_initialize_fptr)(
	iot_t *lib,
	void **plugin_data );
/** @brief defines the signature for the info function in an IoT plug-in */
typedef iot_bool_t (*iot_plugin_info_fptr)(
	const char **name,
	iot_int32_t *order,
	iot_version_t *ver,
	iot_version_t *min,
	iot_version_t *max );
/** @brief defines the signature for the function to load the plugin */
typedef iot_bool_t (*iot_plugin_load_fptr)(
	iot_plugin_t *p );
/**
 * @brief internal structure used to point to various functions within the plug-in
 */
struct iot_plugin
{
	/** @brief function called prior to the plug-in being disabled */
	iot_plugin_disable_fptr disable;
	/** @brief function called prior to the plug-in being enabled */
	iot_plugin_enable_fptr enable;
	/** @brief function called to do work for the plug-in */
	iot_plugin_execute_fptr execute;
	/** @brief function called prior to unloading the plug-in */
	iot_plugin_terminate_fptr terminate;
	/** @brief function called prior to loading the plug-in */
	iot_plugin_initialize_fptr initialize;
	/** @brief function to return the info of the plug-in */
	iot_plugin_info_fptr info;
	/** @brief internal plug-in data */
	void *data;
	/** @brief name of the plug-in */
	const char *name;
#ifdef _WIN32
	/** @brief externally loaded plug-in pointer */
	HANDLE handle;
#else
	/** @brief externally loaded plug-in pointer */
	void *handle;
#endif
};

/**
 * @def IOT_PLUGIN_NAME
 * @brief helper macro for generating function prototypes for loading plug-ins
 * @param[in]      x                   external name of the plug-in
 * @param[in]      x2                  internal name of the plug-in
 * @param[in]      o                   priority order of the plug-in (a lower
 *                                     number will be called first)
 * @param[in]      v                   plug-in version
 * @param[in]      y                   minimum library version
 * @param[in]      z                   maximum library version
 */
#define IOT_PLUGIN_NAME(x,x2,o,v,y,z) \
	IOT_API iot_bool_t x ## _info( const char **name, iot_int32_t *order,\
		iot_version_t *ver, iot_version_t *min, iot_version_t *max ); \
	IOT_API iot_bool_t x ## _load( iot_plugin_t *p ); \
	IOT_API iot_bool_t x ## _info( const char **name, iot_int32_t *order,\
		iot_version_t *ver, iot_version_t *min, iot_version_t *max ) {\
		if (name) *name = (#x2); if (order) *order = (o);\
		if (ver) *ver = (v); if (min) *min = (y); if (max) *max = (z);\
		return IOT_TRUE;\
	}\
	IOT_API iot_bool_t x ## _load( iot_plugin_t *p ) {\
		p->disable = &( x2 ## _disable );\
		p->enable = &( x2 ## _enable );\
		p->execute = &( x2 ## _execute );\
		p->terminate = &( x2 ## _terminate );\
		p->initialize = &( x2 ## _initialize );\
		p->info = &( x ##_info );\
		p->data = NULL;\
		p->name = #x2;\
		p->handle = NULL;\
		return IOT_TRUE;\
	}

/**
 * @def IOT_PLUGIN
 * @brief Macro that implements code to make the plug-in loadable both as a
 *        built-in plug-in or an externally loadable plug-in
 * @param[in]      x                   name of the plug-in
 * @param[in]      o                   priority order of the plug-in (a lower
 *                                     number will be called first)
 * @param[in]      v                   plug-in version
 * @param[in]      y                   minimum library version
 * @param[in]      z                   maximum library version
 */
#if defined(IOT_PLUGIN_BUILTIN)
#define IOT_PLUGIN(x,o,v,y,z) IOT_PLUGIN_NAME(x,x,o,v,y,z)
#else /* if defined(IOT_PLUGIN_BUILTIN) */
#define IOT_PLUGIN(x,o,v,y,z) IOT_PLUGIN_NAME(iot,x,o,v,y,z)
#endif /* else if defined(IOT_PLUGIN_BUILTIN) */

#ifdef __cplusplus
}
#endif /* ifdef __cplusplus */

#endif /* ifndef IOT_PLUGIN_H */

