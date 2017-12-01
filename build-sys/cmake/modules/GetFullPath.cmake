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

# get header file full path
function( GET_FULL_PATH OUT_VAR )
	set( SRCS )
	foreach ( SRC_FILE ${ARGN} )
		if ( IS_ABSOLUTE "${SRC_FILE}" )
			list( APPEND SRCS "${SRC_FILE}" )
		else()
			list( APPEND SRCS "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_FILE}" )
		endif()
	endforeach( SRC_FILE )
	set( ${OUT_VAR} ${SRCS} PARENT_SCOPE )
endfunction( GET_FULL_PATH OUT_VAR )

