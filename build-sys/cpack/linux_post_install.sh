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

# Creates a group and user for the system
IOT_USER=@IOT_USER@
IOT_GROUP=@IOT_GROUP@

groupadd "${IOT_GROUP}" --force --system
IOT_GROUP_ID="$(getent group '$IOT_GROUP' | cut -d: -f3)"

useradd "$IOT_USER" -g "${IOT_GROUP_ID}" -M -r -s /sbin/nologin -c

# Change ownership run-time directories
chown -R "$IOT_USER:$IOT_GROUP" "@IOT_DEFAULT_DIR_RUNTIME@"
