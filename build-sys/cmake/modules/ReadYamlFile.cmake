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

# This is a basic YAML parser, as more features are required, this parser
# is expected to grow over time.  Currently, the parser just parses simple
# lists.

include( CMakeParseArguments )

# Converts YAML types to equivilent cmake type
function( YAML_CONVERT OUT )
	set( "${OUT}" "${ARGN}" PARENT_SCOPE )

	# Forced string (example: "yes", in quotes)
	if ( "${ARGN}" MATCHES "^['\"](.*)['\"]$" )
		set( "${OUT}" "${CMAKE_MATCH_1}" PARENT_SCOPE )
	# False type
	elseif ( "${ARGN}" MATCHES "^(n|N|no|No|NO|false|False|FALSE|off|Off|OFF)$" )
		set( "${OUT}" "0" PARENT_SCOPE )
	# True type
	elseif ( "${ARGN}" MATCHES "^(y|Y|yes|Yes|YES|true|True|TRUE|on|On|ON)$" )
		set( "${OUT}" "1" PARENT_SCOPE )
	# Null type
	elseif ( "${ARGN}" MATCHES "^(~|null|Null|NULL)$" )
		set( "${OUT}" "" PARENT_SCOPE )
	endif()
endfunction( YAML_CONVERT )

function( YAML_PARSE_LINE UPDATED_VARS )
	set( LINE "${ARGN}" )
	# Blank line (or "..." or "---", used to indicate the end of a sequence)
	if( NOT LINE OR LINE MATCHES "^\\.\\.\\." OR LINE MATCHES "^---" )
		string( REGEX REPLACE "^(\\.\\.\\.|---)[ \t]*" "" LINE "${LINE}" )
		string( REGEX REPLACE "[ \t]*#.*$" "" LINE "${LINE}" )
		set( YAML_SEPERATOR " " PARENT_SCOPE )
		if ( LINE MATCHES "^\\|" )
			set( YAML_SEPERATOR "\n" PARENT_SCOPE )
		endif()
		set( YAML_IS_SEQUENCE FALSE PARENT_SCOPE )
		set( YAML_LAST_INDENT 0 PARENT_SCOPE )
		set( YAML_KEYS "" PARENT_SCOPE )
	else()
		# Strip trailing comments (and whitespace)
		string( REGEX REPLACE "[ \t]*#.*$" "" LINE "${LINE}" )

		# Line contains more than just comments
		if ( LINE )
			# Calculate indentation
			string( LENGTH "${LINE}" LEN1 )
			string( STRIP "${LINE}" LINE )
			string( LENGTH "${LINE}" LEN2 )
			math( EXPR YAML_INDENT "${LEN1} - ${LEN2}" )

			# If we changed scope then reset seperator
			if ( ${YAML_INDENT} LESS ${YAML_LAST_INDENT} )
				set( YAML_SEPERATOR "\n" )
			endif()

			# Determine if in a sequence (list)
			if ( "${LINE}" MATCHES "^-" )
				string( REGEX REPLACE "^-[ \t]*" "" LINE "${LINE}" )
				set( YAML_SEPERATOR ";" )
				set( YAML_IS_SEQUENCE TRUE )
			endif()

			# Determine map key
			if ( LINE MATCHES ":" )
				string( REGEX REPLACE "[ \t]*:.*" "" YAML_KEY "${LINE}" )
				string( REGEX REPLACE "^[^:]+:[ \t]*" "" YAML_VALUE "${LINE}" )
			else()
				string( STRIP "${LINE}" YAML_VALUE )
			endif()

			yaml_convert( YAML_VALUE "${YAML_VALUE}" )
			if( YAML_VALUE MATCHES "^>" )
				set( YAML_SEPERATOR " " )
				set( YAML_VALUE "" )
			elseif( YAML_VALUE MATCHES "^\\|" )
				set( YAML_SEPERATOR "\n" )
				set( YAML_VALUE "" )
			endif()

			# if we are moving up or the same with a new key
			# then remove the last key from list
			if ( YAML_KEY AND YAML_KEYS )
				if ( (${YAML_INDENT} LESS ${YAML_LAST_IDENT}) OR
					(${YAML_INDENT} EQUAL ${YAML_LAST_INDENT}) )
					list( REMOVE_AT YAML_KEYS -1 )
				endif()
			endif()

			# Determine full key for key seperated by "_"
			set( YAML_FULL_KEY "${YAML_PREFIX}" )
			foreach( KEY ${YAML_KEYS} )
				set( YAML_FULL_KEY "${YAML_FULL_KEY}${KEY}_" )
			endforeach( KEY )
			string( REGEX REPLACE "_$" "" YAML_FULL_KEY "${YAML_FULL_KEY}" )

			if ( YAML_KEY )
				if ( YAML_FULL_KEY )
					if ( ${YAML_FULL_KEY} )
						set( "${YAML_FULL_KEY}" "${${YAML_FULL_KEY}}${YAML_SEPERATOR}${YAML_KEY}" )
					else()
						set( "${YAML_FULL_KEY}" "${YAML_KEY}" )
					endif()
					set( "${YAML_FULL_KEY}_${YAML_KEY}" "${YAML_VALUE}" )
					set( OUT_VARS "${YAML_FULL_KEY}" "${YAML_FULL_KEY}_${YAML_KEY}" )
				else()
					set( "${YAML_KEY}" "${YAML_VALUE}" )
					set( OUT_VARS "${YAML_KEY}" )
				endif()
			else()
				if ( NOT YAML_FULL_KEY )
					set( YAML_FULL_KEY "${YAML_PREFIX}BASE" )
				endif( NOT YAML_FULL_KEY )
				if ( ${YAML_FULL_KEY} )
					set( "${YAML_FULL_KEY}" "${${YAML_FULL_KEY}}${YAML_SEPERATOR}${YAML_VALUE}" )
				else()
					set( "${YAML_FULL_KEY}" "${YAML_VALUE}" )
				endif()
				set( OUT_VARS "${YAML_FULL_KEY}" )
			endif()

			if( YAML_KEY )
				if ( (${YAML_INDENT} GREATER ${YAML_LAST_INDENT}) OR
					(${YAML_INDENT} EQUAL ${YAML_LAST_INDENT}) )
					list( APPEND YAML_KEYS "${YAML_KEY}" )
				endif()
			endif()

			# Output the list of variables updated by this line
			foreach ( OUT_VAR ${OUT_VARS} )
				set( "${OUT_VAR}" ${${OUT_VAR}} PARENT_SCOPE )
			endforeach( OUT_VAR )
			set( "${UPDATED_VARS}" ${OUT_VARS} PARENT_SCOPE )

			set( YAML_SEPERATOR "${YAML_SEPERATOR}" PARENT_SCOPE )
			set( YAML_KEYS ${YAML_KEYS} PARENT_SCOPE )
			set( YAML_LAST_INDENT ${YAML_INDENT} PARENT_SCOPE )
		endif()
	endif()
endfunction( YAML_PARSE_LINE )

# Arguments: files to read
# Optional: PREFIX
function( READ_YAML_FILE )
	cmake_parse_arguments( "YAML" "" "PREFIX" "IN" ${ARGN} )
	set( YAML_IN ${YAML_IN} ${YAML_UNPARSED_ARGUMENTS} )

	if( YAML_PREFIX )
		set( YAML_PREFIX "${YAML_PREFIX}_" )
	endif()

	foreach( IN_FILE ${YAML_IN} )
		file( STRINGS "${IN_FILE}" IN_LINES NO_HEX_CONVERSION )
		string( REPLACE ";" "_;" IN_LINES "${IN_LINES}" )

		# Reset initial variable values
		yaml_parse_line( "" "---" )
		foreach( LINE ${IN_LINES} )
			string( REGEX REPLACE "_$" "" LINE "${LINE}" )
			yaml_parse_line( UPDATED_VARS "${LINE}" )

			# Update variable in parent scope
			foreach( UPDATED_VAR ${UPDATED_VARS} )
				set( "${UPDATED_VAR}" "${${UPDATED_VAR}}" PARENT_SCOPE )
			endforeach( UPDATED_VAR )
		endforeach( LINE )
	endforeach( IN_FILE )
endfunction( READ_YAML_FILE )
