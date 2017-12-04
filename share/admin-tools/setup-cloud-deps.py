#!/usr/bin/env python

'''
    Copyright (c) 2017-2018 Wind River Systems, Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at:
    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software  distributed
    under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
    OR CONDITIONS OF ANY KIND, either express or implied.
'''

"""
This is an administrator script, i.e. someone with admin credentials
in the cloud.  This script sets up the thing definitions based on the
config files in the thing_defs (relative) directory.  If any file is
put in this directory with the suffix ".cfg" it will be processed as a
thing definition to create or update.
"""

import getpass
import json
import os
import requests
import subprocess
import sys
import time
import platform

from datetime import datetime

if sys.version_info.major == 2:
    input = raw_input

app_file = "validate_app.py"
cloud = ""
validate_app = None
default_thing_def = "hdc_validate_def"
default_app_name = "validation_app"

def update_thing_def(session_id, thing_def, action):
    """
    Create or update a thing definition
    """
    data = {"cmd":{"command":action, "params":thing_def}}
    result = _send(data, session_id)
    ret = False
    if result.get("success") is True:
        ret = True
    #print(json.dumps(result, indent=2, sort_keys=True))

    return ret

def check_thing_def( session_id, thing_def_key ):
    """
    check to see if thing def exists
    """
    data_params = {"key":thing_def_key}
    data = {"cmd":{"command":"thing_def.find", "params":data_params}}
    thing_def_exists = _send(data, session_id)
    ret = False
    if thing_def_exists.get("success") is True:
        ret = True
    return ret

def get_thing_def_id( session_id, thing_def_key ):
    """
    check to see if thing def exists
    """
    data_params = {"key":thing_def_key}
    data = {"cmd":{"command":"thing_def.find", "params":data_params}}
    result = _send(data, session_id)
    ret = "Undef"
    if result.get("success") is True:
        ret = result["params"]["id"]
    #print(json.dumps(result, indent=2, sort_keys=True))
    return ret

def update_app_def(session_id, app_def, action):
    """
    Create or update a app definition
    """
    data = {"cmd":{"command":action, "params":app_def}}
    result = _send(data, session_id)
    ret = False
    if result.get("success") is True:
        ret = True
    #print(json.dumps(result, indent=2, sort_keys=True))

    return ret


def check_app_def( session_id, app_name ):
    """
    check to see if app exists
    """
    data_params = {"name":app_name}
    data = {"cmd":{"command":"app.find", "params":data_params}}
    app_def_exists = _send(data, session_id)
    ret = False
    if app_def_exists.get("success") is True:
        ret = True
    return ret

def _send(data, session_id=None):
    headers = None
    if session_id:
        headers = {"sessionId":session_id}
    datastr = json.dumps(data)
    r = requests.post("https://"+cloud+"/api", headers=headers, data=datastr)
    ret =  {"success":False, "content":r.content,"status_code":r.status_code}
    if r.status_code == 200:
        try:
            rjson = r.json()
            if "auth" in rjson:
                ret = rjson["auth"]
            elif "cmd" in rjson:
                ret = rjson["cmd"]
            return ret
        except Exception as error:
            print(error)
    return ret

def get_app(session_id, name):
    """
    Retreive a list of applications and their tokens
    """

    data_params = {"name":name}
    data = {"cmd":{"command":"app.find", "params":data_params}}
    return _send(data, session_id)

def get_session(username, password):
    """
    Get session for future communications with Cloud
    """

    data_params = {"username":username,"password":password}
    data = {"auth":{"command":"api.authenticate","params":data_params}}
    return _send(data)

def user_is_org_admin( session_id, username ):
    """
    Get info on username
    """
    ret = False
    data_params = {"username":username}
    data = {"cmd":{"command":"user.find","params":data_params}}
    result = _send(data, session_id)
    #print(json.dumps(result, indent=2, sort_keys=True))

    # now get the user ID from this
    if result.get("success") is True:
        user_id = result['params']['id']
        org_id = result['params']['defaultOrgId']

        data_params = {"userId":user_id,"orgId":org_id}
        data = {"cmd":{"command":"user.org.find","params":data_params}}
        result = _send(data, session_id)
        #print(json.dumps(result, indent=2, sort_keys=True))
        if result.get("success") == True:
            if result['params']['isOrgAdmin'] == True:
                ret = True
    return ret

def get_org_id(session_id, username):
    """
    Get org id
    """
    ret = False
    data_params = {"username":username}
    data = {"cmd":{"command":"user.find","params":data_params}}
    result = _send(data, session_id)
    #print(json.dumps(result, indent=2, sort_keys=True))

    # now get the user ID from this
    if result.get("success") is True:
        user_id = result['params']['id']
        org_id = result['params']['defaultOrgId']
    return org_id

def get_thing(session_id, thing_key):
    """
    Get information about a specific thing
    """

    data_params = {"key":thing_key}
    data = {"cmd":{"command":"thing.find","params":data_params}}
    return _send(data, session_id)

def check_for_match(haystack, needle):
    found = False
    for x in range(len(haystack)):
        if needle in haystack[x]['msg']:
            print("Log: \"{}\" - OK".format(haystack[x]['msg']))
            found = True
    return found

def error_quit(*args):
    """
    Print error messages, stop application, and exit with error code 1
    """

    for arg in args:
        if arg.__class__.__name__ == "str":
            print(arg)
    sys.exit(1)

def change_session_org(session_id, org):
    """
    Change org
    """
    ret = False
    print("Changing org to %s" % org)
    data_params = {"key":org}
    data = {"cmd":{"command":"session.org.switch","params":data_params}}
    result = _send(data, session_id)
    if result.get("success") == True:
        ret = True
    return ret

def get_role_id(session_id, role_key):
    """
    check for and return a role id
    """
    data_params = {"key":role_key}
    data = {"cmd":{"command":"role.find", "params":data_params}}
    result = _send(data, session_id)
    ret = "Undef"
    if result.get("success") is True:
        ret = result["params"]["id"]
    #print(json.dumps(result, indent=2, sort_keys=True))
    return ret

def find_role( session_id, role_key):
    """
    Check to see if a role exists
    """
    data_params = {"key":role_key}
    data = {"cmd":{"command":"role.find", "params":data_params}}
    role_exists = _send(data, session_id)
    if role_exists.get("success") is True:
        return True
    else:
        return False

def update_role(session_id, role, role_key, role_perms, action):
    """
    Create or update a role
    """
    data_params = {"key":role_key, "name":role, "perms":role_perms}
    data = {"cmd":{"command":action, "params":data_params}}
    result = _send(data, session_id)
    #print(json.dumps(result, indent=2, sort_keys=True))
    if result.get("success") is True:
        ret = True
    else:
        return False

def main():
    """
    Main function to validate abilites of host
    """
    global cloud

    # relative path to the defs
    thing_def_dir = "thing_defs"
    app_def_dir = "app_defs"
    role_dir = "role_defs"

    # credentials in env
    cloud = os.environ.get("HDCADDRESS")
    username = os.environ.get("HDCUSERNAME")
    password = os.environ.get("HDCPASSWORD")
    switch_org = os.environ.get("HDCORG")

    # ask for Cloud credentials
    if not cloud:
        cloud = input("Cloud Address: ")
    if not username:
        username = input("Username: ")
    if not password:
        password = getpass.getpass("Password: ")
    if not switch_org:
        switch_org = input("Org Key(Optional): ")

    # Ensure Cloud address is formatted correctly for later use
    cloud = cloud.split("://")[-1]
    cloud = cloud.split("/")[0]
    print("Cloud: {}".format(cloud))

    # Start a session with the Cloud
    session_id = ""
    session_info = get_session(username, password)
    if session_info.get("success") is True:
        session_id = session_info["params"].get("sessionId")
    if session_id:
        print("Session ID: {} - OK".format(session_id))
    else:
        error_quit("Failed to get session id.")

    # if the org was specified, switch to it here
    if switch_org:
        print("Org ID before switch=%s" % get_org_id(session_id, username))
        if change_session_org(session_id, switch_org) == False:
            error_quit("Failed to switch org.")
        else:
            print("Org ID after switch=%s" % get_org_id(session_id, username))

    # check to see if this user is an org admin
    if user_is_org_admin( session_id, username ) == False:
        error_quit("Error: user %s is not an org admin." % username)
    else:
        print("Confirmed user has admin access")

    # thing definition creation
    print("Processing thing definitions")
    for path, dirs, files in os.walk( thing_def_dir ):
        for fh in files:
            if fh.endswith(".cfg"):
                def_file = os.path.join( path, fh )
                print( "Processing %s" % def_file )
                with open( def_file ) as cloud_file:
                    data = json.load( cloud_file )

                # check the cloud for this def, if it exists, update it
                # with this one.  There may be more than one def in this
                # file.
                thing_definitions = data["thing_definitions"]
                for thing_def in thing_definitions:

                    thing_key = thing_def["key"]
                    if check_thing_def( session_id, thing_key ) == False:
                        print( "Creating thing def %s" % thing_key )
                        action = "thing_def.create"
                    else:
                        print( "Updating thing def %s" % thing_key )
                        action = "thing_def.update"

                    if update_thing_def( session_id, thing_def, action ) == False:
                        error_quit("Failed to update thing def %s with action %s." % (thing_def, action))

    # role creation
    print("Processing related roles")
    for path, dirs, files in os.walk( role_dir ):
        for fh in files:
            if fh.endswith(".cfg"):
                def_file = os.path.join( path, fh )
                print( "Processing %s" % def_file )
                with open( def_file ) as cloud_file:
                    data = json.load( cloud_file )

                # check the cloud for this role, if it exists, update it
                # with this one.  There may be more than one role def in this
                # file.
                role_definitions = data["role_definitions"]
                for role in role_definitions:
                    role_name = role["name"]
                    role_key = role["key"]
                    role_perms = role["perms"]
                    if find_role( session_id, role_key ) == False:
                        print( "Creating role %s" % role_name )
                        action = "role.create"
                    else:
                        print( "Updating role %s" % role_name )
                        action = "role.update"

                    if update_role( session_id, role_name, role_key, role_perms, action ) == False:
                        print ("WARNING: Failed to update role %s with action %s." % (role-name, action))


    # application creation
    print("Processing application definitions")
    for path, dirs, files in os.walk( app_def_dir ):
        for fh in files:
            if fh.endswith(".cfg"):
                def_file = os.path.join( path, fh )
                print( "Processing %s" % def_file )
                with open( def_file ) as cloud_file:
                    data = json.load( cloud_file )
                app_definitions = data["app_definitions"]
                for app_def in app_definitions:
                    app_name = app_def["name"]
                    if check_app_def( session_id, app_name ) == False:
                        print( "Creating app def %s" % app_name )
                        action = "app.create"
                    else:
                        print( "Updating app def %s" % app_name )
                        action = "app.update"

                    # Note: the role and autoRegThingDefId must be the id,
                    # not the name.  The cfg will contain the name, so
                    # look it up here and update the json data
                    role_keys = app_def.get("roles")
                    role_list = []
                    for role_key in role_keys:
                        role_id = get_role_id(session_id, role_key)
                        if role_id == "Undef":
                            error_quit("Error: Role to set as default does not"
                            "exist for %s." % app_def)
                        print("Found role id %s for role %s" % (role_id, role_name))
                        role_list.append(role_id)
                    thing_def_name = app_def.get("autoRegThingDefId")
                    thing_def_id = get_thing_def_id(session_id, thing_def_name )
                    if thing_def_id == "Undef":
                        error_quit("Error: thing def to set as default does not"
                            "exist for %s." % app_def)
                    print("Found thing def id %s for thing def %s" % (thing_def_id, thing_def_name))
                    app_def["roles"] = role_list
                    app_def["autoRegThingDefId"] = thing_def_id
                    if update_app_def( session_id, app_def, action ) == False:
                        error_quit("Failed to update app def %s with action %s." % (app_def, action))

if __name__ == "__main__":
   main()
   sys.exit(0)
