#
# Compiler flags
#
# Common spot to define compiler flags for various compilers
#
#
# Copyright (C) 2016 Wind River Systems, Inc. All Rights Reserved.
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

include( CheckCCompilerFlag )

if ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -pedantic" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic-errors -Wall -Werror" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wmissing-prototypes" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdeclaration-after-statement" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wunused -Wfloat-equal" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Wwrite-strings" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat-security" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-format-attribute " )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-qual -Wextra" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unknown-pragmas -Wno-padded" )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-nonliteral -fPIC" )

	# Clang static analysis options
	if ( CMAKE_C_COMPILER_ID MATCHES "Clang" )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-documentation-unknown-command" )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-covered-switch-default" )
	endif ( CMAKE_C_COMPILER_ID MATCHES "Clang" )

	check_c_compiler_flag( "-Weverything" WITH_EVERYTHING ) # Clang option
	if ( WITH_EVERYTHING )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Weverything" )
	endif ( WITH_EVERYTHING )

	check_c_compiler_flag( "-D_FORTIFY_SOURCE=2" WITH_FORTIFY_SOURCE )
	if ( WITH_FORTIFY_SOURCE )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORITFY_SOURCE=2" )
	endif( WITH_FORTIFY_SOURCE )

	check_c_compiler_flag( "-D_GNU_SOURCE" WITH_GNU_SOURCE )
	if ( WITH_GNU_SOURCE )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE" )
	endif( WITH_GNU_SOURCE )

	check_c_compiler_flag( "-Wno-format-truncation" WITH_NO_FORMAT_TRUNCATION )
	if ( WITH_NO_FORMAT_TRUNCATION )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-truncation" )
	endif( WITH_NO_FORMAT_TRUNCATION )

	execute_process( COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION )
	if ( GCC_VERSION VERSION_LESS 4.7 )
		set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format" )
	endif ( GCC_VERSION VERSION_LESS 4.7 )

endif ( CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)" )

if ( MSVC )
	# Microsoft visual studio pre-2015 doesn't support fully support C99 and
	# the "inline" keyword.  Instead use the __inline keyword.
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Dinline=__inline" )
	# Treat all warnings as errors
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /WX" )
	# Increase warning level but disable warning about padding on structures
	# set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Wall /wd4820" )
endif( MSVC )

