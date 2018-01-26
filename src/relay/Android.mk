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

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../api/ \
	external/hdc/libwebsockets/lib \
	external/hdc/curl/include \
	external/openssl/include \

LOCAL_CFLAGS += ${EXTRA_CFLAGS}
LOCAL_SHARED_LIBRARIES := libiot libcrypto libssl libz libdl libext2_uuid
LOCAL_STATIC_LIBRARIES := libiotutils libosal libandroidifaddrs libwebsockets

LOCAL_MODULE := iot-relay

LOCAL_SRC_FILES := \
	./relay.c \
	./relay_main.c \

include $(BUILD_EXECUTABLE)

