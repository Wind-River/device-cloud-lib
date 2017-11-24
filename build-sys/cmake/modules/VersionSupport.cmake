#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

# Project version (based on latest commit date)
include( GitSha )
# Support function for extracting a version number from a string, if the
# index is not specified it will return a value of "0" for that version part
# Arguments:
# - _out           output variable name
# - _list_name     list containing version seperated
# - _index         index to extract
function( VERSION_PART_FROM_LIST _out _list_name _index )
	set( VERSION "0" )
	list( LENGTH ${_list_name} VERSION_ITEMS )
	if( ${VERSION_ITEMS} GREATER ${_index} )
		list( GET ${_list_name} ${_index} VERSION )
	endif( ${VERSION_ITEMS} GREATER ${_index} )
	set( "${_out}" ${VERSION} PARENT_SCOPE )
endfunction( VERSION_PART_FROM_LIST )

# Extracts the various version parts (major, minor, patch, tweak) from a
# version string The values will be saved as:
# - ${_out}_COUNT: number of version parts set
# - ${_out}_MAJOR: major (1st) version number
# - ${_out}_MINOR: minor (2nd) version number
# - ${_out}_PATCH: patch (3rd) version number
# - ${_out}_TWEAK: tweak (4th) version number
#
# Arugments:
# - _out           output variable name
# - _ver ...       version to split
function( VERSION_SPLIT _out _ver )
	set( _ver ${_ver} ${ARGN} )
	string( REPLACE "." ";" VERSION_SPLIT "${_ver}" )
	list( LENGTH VERSION_SPLIT "VERSION_COUNT" )
	message( "VERSION: ${VERSION_SPLIT}" )

	version_part_from_list( VERSION_MAJOR VERSION_SPLIT 0 )
	version_part_from_list( VERSION_MINOR VERSION_SPLIT 1 )
	version_part_from_list( VERSION_PATCH VERSION_SPLIT 2 )
	version_part_from_list( VERSION_TWEAK VERSION_SPLIT 3 )

	set( "${_out}_COUNT" "${VERSION_COUNT}" PARENT_SCOPE )
	set( "${_out}_MAJOR" "${VERSION_MAJOR}" PARENT_SCOPE )
	set( "${_out}_MINOR" "${VERSION_MINOR}" PARENT_SCOPE )
	set( "${_out}_PATCH" "${VERSION_PATCH}" PARENT_SCOPE )
	set( "${_out}_TWEAK" "${VERSION_TWEAK}" PARENT_SCOPE )
endfunction( VERSION_SPLIT )

