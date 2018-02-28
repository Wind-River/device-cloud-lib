/* deviceCloudActionRtpCfg.c - Device Cloud configlette */
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

#include <ioLib.h>
#include <taskLib.h>
#include <rtpLib.h>

#define DEVICE_CLOUD_ACTION_RTP_NAME "iot-app-simple-actions"

/**
 * @brief spawns the RTP.
 *
 * This function spawns the RTP after a delay.
 */
static void deviceCloudActionRtpDelay( void )
{
	int fd;
	const char *args[2];

	(void)sleep (DEVICE_CLOUD_AGENT_APP_DELAY);

	if (chdir (DEVICE_CLOUD_AGENT_BIN_DIR) != OK)
	{
		(void)fprintf (stderr, "bin directory %s chdir failed.\n",
			DEVICE_CLOUD_AGENT_BIN_DIR );
		return;
	}

	if ((fd = open (DEVICE_CLOUD_ACTION_RTP_NAME, O_RDONLY, 0)) == -1)
	{
		(void)fprintf (stderr, "Open RTP file %s failed.\n",
			DEVICE_CLOUD_ACTION_RTP_NAME );
		return;
	}
	(void)close (fd);

	args[0] = "";
	args[1] = NULL;

	if (rtpSpawn (DEVICE_CLOUD_ACTION_RTP_NAME, args, NULL,
		DEVICE_CLOUD_AGENT_PRIORITY,
		DEVICE_CLOUD_AGENT_STACK_SIZE,
		RTP_LOADED_WAIT, VX_FP_TASK) == RTP_ID_ERROR)
	{
		(void)fprintf (stderr, "RTP spawn %s error.\n"
			 DEVICE_CLOUD_ACTION_RTP_NAME );
		}
}

/**
 * @brief spawns a task
 *
 * This function spawns a task that will spawn the RTP after a delay.
 */
void deviceCloudActionRtp( void )
{
	if (taskSpawn ("tActionRtpDelay",
		DEVICE_CLOUD_AGENT_PRIORITY, 0,
		DEVICE_CLOUD_AGENT_STACK_SIZE,
		(FUNCPTR) deviceCloudActionRtpDelay,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == TASK_ID_ERROR)
	{
		(void)fprintf (stderr, "Task spawn error.\n");
	}
}
