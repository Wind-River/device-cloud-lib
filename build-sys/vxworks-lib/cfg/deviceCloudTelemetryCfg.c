/* deviceCloudTelemetryCfg.c - Device Cloud configlette */
/*
 * Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include <taskLib.h>

extern int app_telemetry_main ( int argc, char *argv[] );

/**
 *
 * @brief spawns the application task
 *
 * This function spawns the application task.
 */
void deviceCloudTelemetryDelay( void )
{
	static const char *argv[] = { "" };

	(void)sleep (DEVICE_CLOUD_AGENT_APP_DELAY);

	if (taskSpawn ("tTelemetry",
		DEVICE_CLOUD_AGENT_PRIORITY, 0,
		DEVICE_CLOUD_AGENT_STACK_SIZE,
		(FUNCPTR) app_telemetry_main,
		(_Vx_usr_arg_t) 1, (_Vx_usr_arg_t) argv,
		0, 0, 0, 0, 0, 0, 0, 0 ) == TASK_ID_ERROR)
	{
		(void)fprintf (stderr, "Task spawn error.\n");
	}
}

/**
 * @brief spawns a task
 *
 * This function spawns a task that will spawn the application after a delay.
 */
void deviceCloudTelemetryStart( void )
{
	if (taskSpawn ("tTelemetryDelay",
		DEVICE_CLOUD_AGENT_PRIORITY, 0,
		DEVICE_CLOUD_AGENT_STACK_SIZE,
		(FUNCPTR) deviceCloudTelemetryDelay,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
	{
		(void)fprintf (stderr, "Task spawn error.\n");
	}
}
