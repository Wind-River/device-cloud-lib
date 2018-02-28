#
# Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#

LIB_BASE_NAME = iot

BUILD_ALL_OBJS = TRUE

DEVICE_CLOUD_LIB_DIR = $(VSB_DIR)/3pp/HDC_DEVICE_CLOUD_LIB/device_cloud_lib_repo

SRC_DIRS = \
        $(DEVICE_CLOUD_LIB_DIR) \
        $(DEVICE_CLOUD_LIB_DIR)/src/api \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/checksum \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/plugin \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/plugin/tr50 \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities \
        $(DEVICE_CLOUD_LIB_DIR)/build-sys/vxworks-lib

SRC_FILES = \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_action.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_alarm.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_attribute.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_base64.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_base.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_common.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_event.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_file.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_location.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_mqtt.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_option.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_plugin.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/iot_telemetry.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_base.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_decode.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_encode.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/json/iot_json_schema.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/checksum/iot_checksum.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/checksum/iot_checksum_crc32.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/plugin/iot_plugin_builtin.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/api/plugin/tr50/tr50.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities/app_arg.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities/app_log.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/utilities/app_path.c \
        $(DEVICE_CLOUD_LIB_DIR)/build-sys/vxworks-lib/vxworks_lib.c

OBJS = $(addsuffix .o, $(notdir $(basename $(SRC_FILES))))

EXTRA_INCLUDE += -I$(DEVICE_CLOUD_LIB_DIR) \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src/api \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src/api/public \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src/utilities

ADDED_CFLAGS += $(C_COMPILER_FLAGS_C99)

EXTRA_DEFINE += -DOSAL_STATIC=1 -DOSAL_WRAP=1 -DIOT_THREAD_SUPPORT=1 \
                -DOPENSSL=1 \
                -DIOT_PLUGIN_BUILTIN=1 \
                -DIOT_MQTT_MOSQUITTO=1 \
                -DIOT_JSON_JANSSON=1

VPATH = $(realpath $(SRC_DIRS))

ifeq ($(SPACE), user)
include $(WIND_USR_MK)/rules.library.mk
else
include $(WIND_KRNL_MK)/rules.library.mk
endif

#
# This code is 3rd party. Suppress all compiler warnings.
#
CC_WARNINGS = $(CC_WARNINGS_NONE)
