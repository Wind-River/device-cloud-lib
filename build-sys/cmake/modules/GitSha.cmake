#
# Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
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

find_program( GIT_EXECUTABLE git NO_CMAKE_FIND_ROOT_PATH )
mark_as_advanced( GIT_EXECUTABLE )

# Get SHA of last commit
if( GIT_EXECUTABLE )
	execute_process( WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE PROJECT_GIT_SHA COMMAND ${GIT_EXECUTABLE} log -1 --format="%H" )
	string( REPLACE "\n" "" PROJECT_GIT_SHA "${PROJECT_GIT_SHA}" )
	string( REPLACE "\"" "" PROJECT_GIT_SHA "${PROJECT_GIT_SHA}" )
endif( GIT_EXECUTABLE )

# Get date of last commit in short format (2017-11-24)
if ( GIT_EXECUTABLE )
	execute_process( WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE PROJECT_COMMIT_DATE COMMAND ${GIT_EXECUTABLE} log -1 --format="%cd" --date=short )
	string( REPLACE "\n" "" PROJECT_COMMIT_DATE "${PROJECT_COMMIT_DATE}" )
	string( REPLACE "\"" "" PROJECT_COMMIT_DATE "${PROJECT_COMMIT_DATE}" )
endif( GIT_EXECUTABLE )
