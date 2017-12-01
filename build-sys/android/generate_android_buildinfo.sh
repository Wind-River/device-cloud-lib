#!/bin/bash
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#

# collect hdc build infomation
pushd external/hdc

BUILD_INFO_FILE="wr-iot-lib/build-sys/android/buildinfo_hdc_android"
touch ${BUILD_INFO_FILE}
BUILD_INFO_FILE_ABS_PATH=$( readlink -e ${BUILD_INFO_FILE} )

if [ ! -z ${BUILD_INFO_FILE_ABS_PATH} ]
then
	echo "USER: ${USER}" > ${BUILD_INFO_FILE_ABS_PATH}
	echo "BUILD_SERVER: $(uname -a)" >> ${BUILD_INFO_FILE_ABS_PATH}
	echo "BUILD_TIME: $(date -d now +'%F %H-%M-%S %A week %W %Z %::z')" >> ${BUILD_INFO_FILE_ABS_PATH}
	for i in *
	do
		echo "------------------" >> ${BUILD_INFO_FILE_ABS_PATH}
		echo "repo: ${i}" >> ${BUILD_INFO_FILE_ABS_PATH}
		pushd ${i}
		echo "git describe --tags --abbrev=0" >> ${BUILD_INFO_FILE_ABS_PATH}
		git describe --tags --abbrev=0 >> ${BUILD_INFO_FILE_ABS_PATH}
		echo "git log -1" >> ${BUILD_INFO_FILE_ABS_PATH}
		git log -1 >> ${BUILD_INFO_FILE_ABS_PATH}
		echo "git status" >> ${BUILD_INFO_FILE_ABS_PATH}
		git status >> ${BUILD_INFO_FILE_ABS_PATH}
		popd
		echo "" >> ${BUILD_INFO_FILE_ABS_PATH}
	done
fi
popd # external/hdc

exit 0
