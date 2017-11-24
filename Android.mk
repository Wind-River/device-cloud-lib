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

LOCAL_PATH := $(call my-dir)
# generated iot_build.h and iot_plugin_builtin.c
$( info ($(shell ( cd ${LOCAL_PATH}; ./build-sys/generate_header.sh && sed -E "s:etc/([a-zA-Z0-9_-]+):data/\1/etc:g" -i iot_build.h && sed -E "s:var/lib/([a-zA-Z0-9_-]+):data/\1/etc:g" -i iot_build.h && sed -e "s:etc/ssl/certs:system/etc/security/cacerts:g" -i iot_build.h ))))
# generate build_info
$( info ($(shell ${LOCAL_PATH}/build-sys/android/generate_android_buildinfo.sh )))

LOCAL_CFLAGS := -DIOT_THREAD_SUPPORT=1
include $(call all-subdir-makefiles)

