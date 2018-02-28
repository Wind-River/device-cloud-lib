#!/bin/bash
#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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

# create iot-connect.schema.json.h and iot-connect.schema.json.c
# under "src/control/" based on upon the content from 
# "src/control/iot-connect.schema.json"
# refer to "build-sys/cmake/scripts/json_schema_to_c.cmake"
# for more information
#

SRC_DIR="$( cd "$(dirname "$0")/.." ; pwd -P )"
DEST_DIR="."
CONFIG_DIR="src/control"
CONFIG_FILE="iot-connect.schema.json"
HEADER_FILE="iot-connect.schema.json.h"
SOURCE_FILE="iot-connect.schema.json.c"

COPYRIGHT_START="2017"

add_file_header() {
	COPYRIGHT_END=`date +"%Y"`
	if [ "${COPYRIGHT_END}" -ne "${COPYRIGHT_START}" ]; then
		COPYRIGHT_RANGE="${COPYRIGHT_START}-${COPYRIGHT_END}"
	else
		COPYRIGHT_RANGE="${COPYRIGHT_START}"
	fi
	cat > "$1" <<- EOM
/**
 * @file
 * @brief JSON Schema file converted to C
 *
 * @copyright Copyright (C) ${COPYRIGHT_RANGE} Wind River Systems, Inc. All Rights Reserved.
 *
 * @license Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

EOM
}


if [ -f "${SRC_DIR}/${CONFIG_DIR}/${CONFIG_FILE}" ]; then
	add_file_header "${DEST_DIR}/${CONFIG_DIR}/${HEADER_FILE}"
	echo "#ifndef IOT_CONNECT_SCHEMA_JSON_H" >> "${DEST_DIR}/${CONFIG_DIR}/${HEADER_FILE}"
	echo "	extern const char *const IOT_CONNECT_SCHEMA;" >> "${DEST_DIR}/${CONFIG_DIR}/${HEADER_FILE}"
	echo "#endif /* ifndef IOT_CONNECT_SCHEMA_JSON_H */" >> "${DEST_DIR}/${CONFIG_DIR}/${HEADER_FILE}"

	add_file_header "${DEST_DIR}/${CONFIG_DIR}/${SOURCE_FILE}"
	echo "#include \"${HEADER_FILE}\"" >> "${DEST_DIR}/${CONFIG_DIR}/${SOURCE_FILE}"
	echo "" >> "${DEST_DIR}/${CONFIG_DIR}/${SOURCE_FILE}"
	echo "const char *const IOT_CONNECT_SCHEMA =" >> "${DEST_DIR}/${CONFIG_DIR}/${SOURCE_FILE}"
	IFS=''
	while read -r line
	do
		tabs=$( echo "${line}" | sed "s/[^\t]*//g" )
		echo -n "${tabs}" >> "${DEST_DIR}/${CONFIG_DIR}/${SOURCE_FILE}"
		line=$( echo "${line}" | sed "s/^[ \t]*//g" )
		new_line=$( echo ${line} | sed "s/\"/\\\\\"/g" )
		echo "\"${new_line}\"" >> "${DEST_DIR}/${CONFIG_DIR}/${SOURCE_FILE}"
	done < "${SRC_DIR}/${CONFIG_DIR}/${CONFIG_FILE}"
	echo ";" >> "${DEST_DIR}/${CONFIG_DIR}/${SOURCE_FILE}"
else
	echo "Configration file doesn't exist!!!"
	exit 1
fi

exit 0
