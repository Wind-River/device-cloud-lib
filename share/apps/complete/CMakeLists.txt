#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.

add_sample_app( complete
	DISPLAY_NAME "Complete example"
	DESCRIPTION "Application demonstrating advanced usage of the api"
	README README.md
	SCRIPTS_UNIX test_script.sh
	SCRIPTS_WIN32 test_script.cmd
	SRCS complete.c
	LDFLAGS_GCC -lm
	TARGET iot-app-complete
)

