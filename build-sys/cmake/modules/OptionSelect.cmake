#
# Copyright (C) 2015-2018 Wind River Systems, Inc. All Rights Reserved.
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

include( CMakeParseArguments )

# Creates a drop-down list of options for a user to select in the cmake-gui
# Arguments:
#   OUTPUT_VAR - name of the output variable
#   DESCRIPTION ... - description for the option (OPTIONAL)
#   DEAFULT ... - default value for the list
#   ... - list of options to put in the drop down list
function( OPTION_SELECT OUTPUT_VAR )
	cmake_parse_arguments( DROPDOWN "" "DESCRIPTION;DEFAULT" "" ${ARGN} )
	set( DROPDOWN_OPTIONS ${DROPDOWN_UNPARSED_ARGUMENTS} )
	if ( DROPDOWN_DEFAULT )
		if ( NOT ";${DROPDOWN_OPTIONS};" MATCHES ";${DROPDOWN_DEFAULT};" )
			set( DROPDOWN_OPTIONS ${DROPDOWN_OPTIONS} ${DROPDOWN_DEFAULT} )
		endif ( NOT ";${DROPDOWN_OPTIONS};" MATCHES ";${DROPDOWN_DEFAULT};" )
	endif ( DROPDOWN_DEFAULT )
	if ( NOT DROPDOWN_DESCRIPTION )
		set( DROPDOWN_DESCRIPTION "Value for ${OUPUT_VAR}" )
	endif ( NOT DROPDOWN_DESCRIPTION )

	if ( NOT ${OUTPUT_VAR} )
		set( ${OUTPUT_VAR} "${DROPDOWN_DEFAULT}" CACHE STRING "${DROPDOWN_DESCRIPTION}" FORCE )
	endif ( NOT ${OUTPUT_VAR} )
	set_property( CACHE ${OUTPUT_VAR} PROPERTY STRINGS ${DROPDOWN_OPTIONS} )

	string( TOUPPER "${DROPDOWN_OPTIONS}" DROPDOWN_OPTIONS_UPPER )
	string( TOUPPER "${${OUTPUT_VAR}}" CURRENT_OPTION_UPPER )
	if ( NOT ";${DROPDOWN_OPTIONS_UPPER};" MATCHES ";${CURRENT_OPTION_UPPER};" )
		string( REPLACE ";" ", " VALID_OPTIONS "${DROPDOWN_OPTIONS}" )
		message( FATAL_ERROR
			"The value of \"${${OUTPUT_VAR}}\" for \"${OUTPUT_VAR}\" is not valid. "
			"Valid values are: ${VALID_OPTIONS}" )
	endif ( NOT ";${DROPDOWN_OPTIONS_UPPER};" MATCHES ";${CURRENT_OPTION_UPPER};" )
endfunction( OPTION_SELECT )

# Similiar to the cmake "option" command, except that it ensures that the
# given variable has a value defined and is not defined to "blank"
# Arguments:
#   VAR_NAME - name of the variable
#   VAR_DESCRIPTION - description of the variable
#   ... - initial value for th eoption (ON or OFF)
function( OPTION_ENSURE_SET VAR_NAME VAR_DESCRIPTION )
	if ( DEFINED ${VAR_NAME} AND "x${${VAR_NAME}}" STREQUAL "x" )
		unset( ${VAR_NAME} CACHE )
	endif ( DEFINED ${VAR_NAME} AND "x${${VAR_NAME}}" STREQUAL "x" )
	option( ${ARGV} )
endfunction( OPTION_ENSURE_SET )
