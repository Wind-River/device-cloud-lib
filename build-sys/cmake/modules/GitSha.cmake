#
# Copyright (C) 2016-2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
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
