#
# Copyright 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH:= $(call my-dir)

iotutils_c_includes := \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/../../ \

iotutils_src_files := \
    app_arg.c \
    app_log.c \
    app_path.c \

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(iotutils_c_includes)
LOCAL_CFLAGS += ${EXTRA_CFLAGS}
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_SHARED_LIBRARIES := libdl libext2_uuid
LOCAL_STATIC_LIBRARIES := libosal libandroidifaddrs
LOCAL_MODULE := libiotutils
LOCAL_SRC_FILES := $(iotutils_src_files)
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(iotutils_c_includes)
LOCAL_CFLAGS += ${EXTRA_CFLAGS}
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_STATIC_LIBRARIES := libosal libandroidifaddrs
LOCAL_MODULE := libiotutils
LOCAL_SRC_FILES := $(iotutils_src_files)
include $(BUILD_STATIC_LIBRARY)
