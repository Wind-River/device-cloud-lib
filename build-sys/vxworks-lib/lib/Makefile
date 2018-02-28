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


include $(WIND_KRNL_MK)/defs.ver.mk

PKG_NAME := device_cloud_lib_repo
PKG_URL = https://github.com/Wind-River/device-cloud-lib.git $(PKG_NAME); cd $(PKG_NAME) && git checkout -b vxworks origin/master && git reset HEAD --hard && build-sys/generate_header.sh && build-sys/generate_schema.sh && mv iot_build.h src/iot_build.h

PKG_FILE_TYPE = git

PKG_BUILD_TARGETS := src
PKG_BUILD_DIR := $(PKG_NAME)/src
PKG_PATCH_DIR := $(PKG_NAME)

PKG_EXTRA_SRC :=

KERNEL_PUBLIC_H_FILES = api/public/iot.h \
                        api/public/iot_checksum.h \
                        api/public/iot_json.h \
                        api/public/iot_mqtt.h \
                        api/public/iot_plugin.h
USER_PUBLIC_H_FILES =   api/public/iot.h \
                        api/public/iot_checksum.h \
                        api/public/iot_json.h \
                        api/public/iot_mqtt.h \
                        api/public/iot_plugin.h

include $(WIND_KRNL_MK)/rules.unix.ports.mk
