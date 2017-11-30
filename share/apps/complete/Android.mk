#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../src/api/public
LOCAL_SHARED_LIBRARIES := libiot
LOCAL_MODULE := iot-app-complete
LOCAL_SRC_FILES := complete.c
LOCAL_LDLIBS := -lm
include $(BUILD_EXECUTABLE)

