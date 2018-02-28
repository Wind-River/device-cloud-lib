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

ifeq ($(SPACE), user)
#EXE = iot-update
ADDED_LIBS += -liot -losal -lnet -larchive -lHASH -lOPENSSL -lmosquitto -ljson -lzlib -lcurl -lunix
else
LIB_BASE_NAME = deviceCloudUpdate
endif

BUILD_ALL_OBJS = TRUE

DEVICE_CLOUD_LIB_DIR = $(VSB_DIR)/3pp/HDC_DEVICE_CLOUD_LIB/device_cloud_lib_repo

SRC_DIRS = \
        $(DEVICE_CLOUD_LIB_DIR)/src/software-update

SRC_FILES = \
        $(DEVICE_CLOUD_LIB_DIR)/src/software-update/iot_update.c \
        $(DEVICE_CLOUD_LIB_DIR)/src/software-update/iot_update_main.c

OBJS = $(addsuffix .o, $(notdir $(basename $(SRC_FILES))))

EXTRA_INCLUDE += -I$(DEVICE_CLOUD_LIB_DIR)/src \
                 -I$(DEVICE_CLOUD_LIB_DIR)/src/software-update

ADDED_CFLAGS += $(C_COMPILER_FLAGS_C99)

EXTRA_DEFINE += -DOSAL_STATIC=1 -DOSAL_WRAP=1 -DIOT_THREAD_SUPPORT=1 \
                -DOPENSSL=1

VPATH = $(realpath $(SRC_DIRS))

ifeq ($(SPACE), user)
include $(WIND_USR_MK)/rules.rtp.mk
else
include $(WIND_KRNL_MK)/rules.library.mk
endif

#
# This code is 3rd party. Suppress all compiler warnings.
#
CC_WARNINGS = $(CC_WARNINGS_NONE)
