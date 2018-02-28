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

iot_c_includes := $(LOCAL_PATH) \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/public \
	$(LOCAL_PATH)/share \
	external/e2fsprogs/lib

# build plugin
define build_plugin_util
__plugin_module_name:= $1
__plugin_module_src:= $2
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(iot_c_includes) external/hdc/curl/include
LOCAL_CFLAGS += -DIOT_PLUGIN_BUILTIN=1 -DOPENSSL ${EXTRA_CFLAGS}
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/public
LOCAL_MODULE := $$(__plugin_module_name)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $$(__plugin_module_src)
LOCAL_STATIC_LIBRARIES := libosal libandroidifaddrs
include $(BUILD_STATIC_LIBRARY)
endef

$(eval $(call build_plugin_util, libtr50, ./plugin/tr50/tr50.c ) )

# build libiot
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(iot_c_includes) external/hdc/libarchive/contrib/android/include
LOCAL_CFLAGS += -DIOT_PLUGIN_SUPPORT=1 -DOPENSSL -DJSMN_PARENT_LINKS -DJSMN_STRICT ${EXTRA_CFLAGS}
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/public
LOCAL_SHARED_LIBRARIES := libcutils libdl libjansson libmosquitto libext2_uuid libcrypto libssl libcurl
LOCAL_STATIC_LIBRARIES := libosal libandroidifaddrs libtr50 libpaho-mqtt3as libiotjsmn libarchive

LOCAL_MODULE := libiot
LOCAL_SRC_FILES := \
	./iot_action.c \
	./iot_alarm.c \
	./iot_attribute.c \
	./iot_base.c \
	./iot_base64.c \
	./iot_common.c \
	./iot_event.c \
	./iot_file.c \
	./iot_location.c \
	./iot_mqtt.c \
	./iot_option.c \
	./iot_plugin.c \
	./iot_telemetry.c \
	./checksum/iot_checksum.c \
	./checksum/iot_checksum_crc32.c \
	./json/iot_json_decode.c \
	./json/iot_json_encode.c \
	./json/iot_json_schema.c \
	./json/iot_json_base.c \
	./plugin/iot_plugin_builtin.c
include $(BUILD_SHARED_LIBRARY)

# install certificates
include $(CLEAR_VARS)
LOCAL_MODULE := ca-certificates.crt
LOCAL_SRC_FILES := ca-certificates.crt
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/security/cacerts
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := buidinfo_hdc_android
LOCAL_SRC_FILES := ../../build-sys/android/buildinfo_hdc_android
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/iot
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := config_hdc_android.sh
LOCAL_SRC_FILES := ../../build-sys/android/config_hdc_android.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

