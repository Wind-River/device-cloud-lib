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

set( TARGET "utilities" )
set( TESTS
	"app_arg"
)

include( "mock_osal" )

set( TEST_APP_ARG_MOCK ${MOCK_OSAL_FUNC} )
set( TEST_APP_ARG_SRCS ${MOCK_OSAL_SRCS} "app_arg_test.c" )
set( TEST_APP_ARG_LIBS ${MOCK_OSAL_LIBS} )
set( TEST_APP_ARG_UNIT "app_arg.c" )

include( TestSupport )
add_tests( ${TARGET} ${TESTS} )

