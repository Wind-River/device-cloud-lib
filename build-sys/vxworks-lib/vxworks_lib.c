/**
 * @file
 * @brief VxWorks specific functions
 *
 * @copyright Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied."
 */

#include "vxworks_lib.h"

#include <limits.h>
#include <stdlib.h>

static char CONFIG_DIR[PATH_MAX] = "/bd0:1/etc/iot";
static char RUNTIME_DIR[PATH_MAX] = "/bd0:1/var/lib/iot";
static char BIN_DIR[PATH_MAX] = "/bd0:1/bin";
static int PRIORITY = 100;
static int STACK_SIZE = 0x10000;

void deviceCloudConfigDirSet ( const char *str )
{
	if ((str != NULL) && (str != '\0'))
	{
		strncpy(CONFIG_DIR, str, PATH_MAX);
		CONFIG_DIR[PATH_MAX - 1u] = '\0';
	}
}

void deviceCloudRuntimeDirSet ( const char *str )
{
	if ((str != NULL) && (str != '\0'))
	{
		strncpy(RUNTIME_DIR, str, PATH_MAX);
		RUNTIME_DIR[PATH_MAX - 1u] = '\0';
	}
}

void deviceCloudBinDirSet ( const char *str )
{
	if ((str != NULL) && (str != '\0'))
	{
		strncpy(BIN_DIR, str, PATH_MAX);
		BIN_DIR[PATH_MAX - 1u] = '\0';
	}
}

void deviceCloudPrioritySet ( const char *str )
{
	if ((str != NULL) && (str != '\0'))
		PRIORITY = atoi(str);
}

void deviceCloudStackSizeSet ( const char *str )
{
	if ((str != NULL) && (str != '\0'))
		STACK_SIZE = atoi(str);
}

const char *deviceCloudConfigDirGet ( void )
{
	return CONFIG_DIR;
}

const char *deviceCloudRuntimeDirGet ( void )
{
	return RUNTIME_DIR;
}

const char *deviceCloudBinDirGet ( void )
{
	return BIN_DIR;
}

unsigned int deviceCloudPriorityGet ( void )
{
	return PRIORITY;
}

unsigned int deviceCloudStackSizeGet ( void )
{
	return STACK_SIZE;
}

