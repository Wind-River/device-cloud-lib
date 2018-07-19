#!/usr/bin/env python

'''
    Copyright (c) 2016-2018 Wind River Systems, Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at:
    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software  distributed
    under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
    OR CONDITIONS OF ANY KIND, either express or implied.
'''

"""
A script that executes an application which publishes known values to the Cloud
with the purpose of validating the abilities of the host to run the device_cloud agent.
In the Cloud there must be a thing definition validate_thing_def.cfg.
"""

import getpass
import json
import os
import requests
import subprocess
import sys
import time
import platform
import uuid
import argparse

session_id = None
thing_key = None

from datetime import datetime

if sys.version_info.major == 2:
    input = raw_input

app_file = ""
cloud = ""
validate_app = None
default_thing_def = "hdc_validate_def"
default_app_name = "hdc_validate_app"
pycommand = "python"
runtime_dir = ""
config_file = ""

if sys.version_info.major == 3 and sys.platform.startswith("linux"):
    pycommand = "python3"

def _send(data, session_id=None):
    headers = None
    if session_id:
        headers = {"sessionId":session_id}
    datastr = json.dumps(data)
    r = requests.post("https://"+cloud+"/api", headers=headers, data=datastr)
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
    return {"success":False, "content":r.content,"status_code":r.status_code}


def get_alarm(session_id, thing_key, alarm_name):
    """
    Retrieve the last value of a sent alarm
    """

    data_params = {"thingKey":thing_key,"key":alarm_name}
    data = {"cmd":{"command":"alarm.current","params":data_params}}
    return _send(data, session_id)


def get_app(session_id, name):
    """
    Retreive a list of applications and their tokens
    """

    data_params = {"name":name}
    data = {"cmd":{"command":"app.find", "params":data_params}}
    return _send(data, session_id)


def get_attribute(session_id, thing_key, attr_name):
    """
    Retrieve the last value of a sent attribute
    """

    data_params = {"thingKey":thing_key,"key":attr_name}
    data = {"cmd":{"command":"attribute.current","params":data_params}}
    return _send(data, session_id)


def get_files(session_id, thing_key):
    """
    Retrieve a list of a specified thing's files
    """

    data_params = {"thingKey":thing_key}
    data = {"cmd":{"command":"file.list","params":data_params}}
    return _send(data, session_id)


def get_location(session_id, thing_key):
    """
    Retrieve last location of a specified thing
    """

    data_params = {"thingKey":thing_key}
    data = {"cmd":{"command":"location.current","params":data_params}}
    return _send(data, session_id)


def get_logs(session_id, thing_key, start=None):
    """
    Retrive logs from the Cloud
    """

    data_params = {"thingKey":thing_key}
    if start:
        data_params.update({"start":start})
    data = {"cmd":{"command":"log.list","params":data_params}}
    return _send(data, session_id)


def get_property(session_id, thing_key, prop_name):
    """
    Retrieve the last value of a sent property
    """

    data_params = {"thingKey":thing_key,"key":prop_name}
    data = {"cmd":{"command":"property.current","params":data_params}}
    return _send(data, session_id)


def get_session(username, password):
    """
    Get session for future communications with Cloud
    """

    data_params = {"username":username,"password":password}
    data = {"auth":{"command":"api.authenticate","params":data_params}}
    return _send(data)


def get_thing(session_id, thing_key):
    """
    Get information about a specific thing
    """

    data_params = {"key":thing_key}
    data = {"cmd":{"command":"thing.find","params":data_params}}
    return _send(data, session_id)

def delete_thing(session_id, thing_key):
    """
    delete the test thing
    """

    print("Deleting thing key {} for this test".format(thing_key))
    data_params = {"key":thing_key}
    data = {"cmd":{"command":"thing.delete","params":data_params}}
    return _send(data, session_id)

def method_exec(session_id, thing_key, method_name, params=None):
    """
    Execute Method
    """

    data_params = {"thingKey":thing_key,"method":method_name,"ackTimeout":30}
    if params:
        data_params["params"] = params
    data = {"cmd":{"command":"method.exec","params":data_params}}
    return _send(data, session_id)

def stop_app(proc):
    """
    Stop application by sending a newline to stdin
    """

    if proc:
        if proc.poll() is None:
            out = proc.communicate(b"\n")
            proc.wait()


def error_quit(*args):
    """
    Print error messages, stop application, and exit with error code 1
    """

    for arg in args:
        if arg.__class__.__name__ == "str":
            print(arg)
        elif arg.__class__.__name__ == "Popen":
            stop_app(arg)
    sys.exit(1)


def strtotime(string):
    """
    Convert string to a datetime object
    """

    return datetime.strptime(string, "%Y-%m-%dT%H:%M:%S.%fZ")


def timetostr(dtime):
    """
    Convert datetime into a string
    """

    return dtime.strftime("%Y-%m-%dT%H:%M:%S.%fZ")

def check_for_match(haystack, needle):
    found = False
    for x in range(len(haystack)):
        if needle in haystack[x]['msg']:
            print("Log: \"{}\" - OK".format(haystack[x]['msg']))
            found = True
    return found

def rest_exec_upload(session_id, thing_key):
    """
    Test REST calls by uploading a file
    """
    loop_done = False
    tries = 10
    upload_stat = method_exec(session_id, thing_key, "file_upload",
                              {"file_name":"validate_exec.txt",
                              "file_path":"validate_download.txt"})
    print upload_stat
    if upload_stat.get("success") is True:
        print("upload success: True - OK")
    else:
        print("upload failed to complete successfully - FAIL")

    # Check that a file was successfully uploaded to the Cloud
    while tries > 0 and loop_done == False:
        tries -= 1
        time.sleep(0.5)

        # Check for success status for method_exec command
        if upload_stat.get("success") == True:
            files_info = get_files(session_id, thing_key)

            # Check that the file was actually uploaded
            if files_info.get("success") is True:

                # there may be more than one file returned
                for file in files_info["params"]["result"]:
                    if file["fileName"]  == "validate_exec.txt":
                        print("-->File uploaded with method_exec: validate_exec.txt - OK")
                        loop_done = True

            # check for the file
            upload_stat = method_exec(session_id, thing_key, "file_upload",
                                      {"file_name":"validate_exec.txt"})
    if tries == 0:
        print("method_exec failied for upload - FAIL")
        fails.append("method_exec upload")

def rest_exec_download(session_id, thing_key):
    """
    Test REST calls by downloading a file
    """
    download_stat = method_exec(session_id, thing_key, "file_download",
                                {"file_name":"validate_exec.txt",
                                "file_path":"validate_exec.txt"})
    if download_stat.get("success") is True:
        print("upload success: True - OK")
    else:
        print("upload failed to complete successfully - FAIL")

    # Check that a file was successfully downloaded from the Cloud
    tries = 10
    while tries > 0:
        tries -= 1
        time.sleep(0.5)
        # Check for success status for method_exec command
        if download_stat.get("success") == True:
            loop_done = False
            if os.path.isfile(os.path.abspath("validate_exec.txt")):
                print("File downloaded with method_exec: validate_exec.txt - OK")
                os.remove(os.path.abspath("validate_exec.txt"))
                loop_done = True
                break
            else:
                # Try again
                download_stat = method_exec(session_id, thing_key, "file_download",
                                            {"file_name":"validate_exec.txt"})
                if tries == 0:
                    print("method_exec was successful, but file not found - FAIL")
                    fails.append("Finding file on device (method_exec)")
                    break
            if loop_done == True:
                break
        else:
            download_stat = method_exec(session_id, thing_key, "file_download",
                                        {"file_name":"validate_exec.txt"})
            if tries == 0:
                print("method_exec failied for download - FAIL")
                fails.append("method_exec download")

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

# precision is an issue between C and python depending on how the
# number is rounded.  So, get close enough with a tolerance of
# 0.0.0.0.1
def is_float_close(a, b, rel_tol=1e-09, abs_tol=0.0001):
    ret = abs(a-b) <= max(rel_tol * max(abs(a), abs(b)), abs_tol)
    if ret:
        print("{} is close enough to {}".format(a,b))
    else:
        print("Error: {} is NOT close enough to {}".format(a,b))
    return ret

def main():
    """
    Main function to validate abilites of host
    """

    global cloud, session_id, thing_key
    start_time = datetime.utcnow()
    global fails
    fails = []

    # travis sets an env, use it for a unique think key that will not
    # be impacted by someone running the script manually.
    # Note: use default_device_id with the default device id.  If the device_id
    # file does not exist, use the default.
    default_device_id = ""
    default_device_id = os.environ.get("AUTO_PREFIX")
    if default_device_id:
        py_ver = platform.python_version()
        default_device_id += py_ver.replace(".","")
    else:
        # if there is no AUTO_PREFIX generate a default id
        # this will only be used if there is no device_id file
        default_device_id = str(uuid.uuid4())

    if not os.path.isfile(app_file):
        error_quit("Could not find app file {}.".format(app_file))

    cloud = os.environ.get("HDCADDRESS")
    username = os.environ.get("HDCUSERNAME")
    password = os.environ.get("HDCPASSWORD")
    token = os.environ.get("HDCAPPTOKEN")
    org = os.environ.get("HDCORG")

    # Get Cloud credentials
    if not cloud:
        cloud = input("Cloud Address: ")
    if not username:
        username = input("Username: ")
    if not password:
        password = getpass.getpass("Password: ")
    if not org:
        org = input("Org Key(Optional): ")

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

    # now handle a switch org if needed
    if org:
        print("Org ID before switch=%s" % get_org_id(session_id, username))
        if change_session_org(session_id, org) == False:
            error_quit("Failed to switch org.")
        else:
            print("Org ID after switch=%s" % get_org_id(session_id, username))


    # Look for the app token created for this validation test.
    # This token is looked for by name, so as long as the cloud has a validation
    # app set up, the token does not need to be retrieved manually.
    # if HDCAPPTOKEN was in the env use it.  Otherwise, query it.
    if not token:
        validateapps = []
        app_info = get_app(session_id, default_app_name)
        #print(json.dumps(app_info, indent=2, sort_keys=True))
        if app_info.get("success") is True:
            app_list = app_info.get("params")
            token = app_list["token"]
            print("Token: {} - OK".format(token))
        else:
            error_quit("Either app does not exist or user does not have\n"
                       "permission to query the token.\n"
                       " * Please export HDCAPPTOKEN=<app token> and run again")

    # Generate config for app with retrieved token
    generate = subprocess.Popen("{} generate_config.py -f {} -c {} -p 8883 -t {}".format(
                                pycommand, config_file, cloud, token),
                                shell=True,
                                stdout=subprocess.PIPE, stdin=subprocess.PIPE)
    ret = generate.wait()
    if ret != 0:
        error_quit("Failed to generate connection config for validation app.")

    # Remove the downloaded file if it exists from a previous validation
    if os.path.isfile(os.path.abspath("validate_download.txt")):
        os.remove(os.path.abspath("validate_download.txt"))

    # delete the thing in the cloud so that we don't have 100s of new
    # instances of test apps.  Write the device_id here and then check
    # the cloud.  Subsequent code will use the device_id.  This test
    # would normally be run in a docker instance with a new device_id each time.
    if os.path.isfile(runtime_dir + "/device_id"):
        print("file device_id exists, using it")
        with open(runtime_dir + "/device_id", "r") as did_file:
            device_id = did_file.read().strip()
    else:
        device_id_path = runtime_dir + "/device_id"

        print ("Writing device_id {} to {}".format(default_device_id,device_id_path ))
        with open(device_id_path, "w") as did_file:
            did_file.write(default_device_id)
        device_id = default_device_id
    thing_key = device_id + "-validate-app"

    thing_info = delete_thing(session_id, thing_key)
    #print(json.dumps(thing_info, indent=2, sort_keys=True))
    time.sleep(2)

    # Start app
    os.system("{}/{} &".format(runtime_dir,app_file))

    # Check to make sure thing is connected in Cloud
    thing = None
    connected = False
    print("Checking thing key %s for connection" % thing_key)
    for i in range(20):
        thing_info = get_thing(session_id, thing_key)
        print(json.dumps(thing_info, indent=2, sort_keys=True))
        if thing_info.get("success") is True:
            thing = thing_info.get("params")
            connected = thing.get("connected")
            break
        time.sleep(2)

    if connected is True:
       print("Connected - OK")
    else:
       error_quit("Thing not connected - FAIL", validate_app)

    # Check that the expected property value was published to the Cloud
    prop = None
    for i in range(10):
        prop_info = get_property(session_id, thing_key, "property")
        #print(json.dumps(prop_info, indent=2, sort_keys=True))
        if prop_info.get("success") is True:
           prop = prop_info.get("params")
           if prop:
               if (strtotime(prop["ts"]) > start_time and
                   strtotime(prop["ts"]) < datetime.utcnow()):
                   if prop["value"] == 12.34:
                       print("Property: {} - OK".format(prop["value"]))
                       break
                   else:
                       print("Wrong property value: {} != 12.34 "
                        "- FAIL".format(prop["value"]))
                       fails.append("Property value")
               else:
                   print("Property timestamp out of range of application - FAIL")
                   fails.append("Property time")
           else:
               print("Property not found in Cloud - FAIL")
               fails.append("Property retrieval")

    # Check that the expected attribute value was published to the Cloud
    attr = None
    for i in range(10):
        attr_info = get_attribute(session_id, thing_key, "attribute")
        #print(json.dumps(attr_info, indent=2, sort_keys=True))
        if attr_info.get("success") is True:
            attr = attr_info.get("params")
        if attr:
            if (strtotime(attr["ts"]) > start_time and
                    strtotime(attr["ts"]) < datetime.utcnow()):
                if attr["value"] == "text and such":
                    print("Attribute: \"{}\" - OK".format(attr["value"]))
                    break
                else:
                    print("Wrong attribute value: {} != \"text and such\" - FAIL".format(attr["value"]))
                    fails.append("Attribute value")
            else:
                print("Attribute timestamp out of range of application - FAIL")
                fails.append("Attribute time")
        else:
            print("Attribute not found in Cloud - FAIL")
            fails.append("Attribute retrieval")

    # Check that the expected location was published to the Cloud
    loc = None
    for i in range(10):
        loc_info = get_location(session_id, thing_key)
        #print(json.dumps(loc_info, indent=2, sort_keys=True))
        if loc_info.get("success") is True:
            loc = loc_info.get("params")
        if loc:
            lat_exp = 45.351603
            lng_exp = -75.918713
            hed_exp = 12.34
            alt_exp = 1.0
            spd_exp = 2.0
            acc_exp = 3.0
            fix_exp = "fixed"

            errors = []
            if not is_float_close(loc["lat"], lat_exp):
                errors.append("lat: {} != {}".format(loc["lat"], lat_exp))
            if not is_float_close(loc["lng"], lng_exp):
                errors.append("lng: {} != {}".format(loc["lng"], lng_exp))
            if not is_float_close(loc["heading"], hed_exp):
                errors.append("heading: {} != {}".format(loc["heading"], hed_exp))
            if not is_float_close(loc["altitude"], alt_exp):
                errors.append("altitude: {} != {}".format(loc["altitude"], alt_exp))
            if not is_float_close(loc["speed"], spd_exp):
                errors.append("speed: {} != {}".format(loc["speed"], spd_exp))
            if not is_float_close(loc["fixAcc"], acc_exp):
                errors.append("fixAcc: {} != {}".format(loc["fixAcc"], acc_exp))
            if loc["fixType"] != fix_exp:
                errors.append("fix_type: {} != {}".format(loc["fixType"], fix_exp))
            if errors:
                print("Wrong location: {} - FAIL".format(", ".join(errors)))
                fails.append("Location")
            else:
                print("Location: lat:{}, lng:{}, etc... - OK".format(loc["lat"], loc["lng"]))
                break
        else:
            print("Location not found in Cloud - FAIL")
            fails.append("Location retrieval")

    # Check that the expected log was published to the Cloud. 
    # load the json obj and walk it to make sure
    # recheck for logs if at first it fails
    logs = None
    found = False
    for i in range(10):
        logs_info = get_logs(session_id, thing_key, start=timetostr(start_time))
        found = check_for_match(logs_info['params']['result'], "logs and such")
        if found == True:
            break
        time.sleep(1)

    if found == False:
        print("No logs for this thing in the specified time frame - FAIL")
        fails.append("Log retrieval")

    # Check that the pass action executes and returns successfully
    pass_act_info = method_exec(session_id, thing_key, "pass_action",
                                {"param":"value"})
    #print(json.dumps(pass_act_info, indent=2, sort_keys=True))
    if pass_act_info.get("success") is True:
        print("Pass action success: True - OK")
    else:
        print("Pass action failed to complete successfully - FAIL")
        fails.append("Action successful execution")

    # Check that the fail action executes and returns a failure successfully
    fail_act_info = method_exec(session_id, thing_key, "fail_action")
    #print(json.dumps(fail_act_info, indent=2, sort_keys=True))
    if fail_act_info.get("success") is False:
        err_exp = "not supported"
        errmsgs = fail_act_info.get("errorMessages")
        if len(errmsgs) == 1 and errmsgs[0] == err_exp:
            print("Fail action error message: {} - OK".format(err_exp))
        else:
            print("Fail action error message: {} != {} - "
                  "FAIL".format(errmsgs[0], err_exp))
            fails.append("Action error message")
    else:
        print("Fail action did not return failure - FAIL")
        fails.append("Action failure")

    # Check that a file was successfully uploaded to the Cloud
    tries = 50
    while tries > 0:
        tries -= 1
        time.sleep(0.5)
        files_info = get_files(session_id, thing_key)
        loop_done = False
        if files_info.get("success") is True:
            # there may be more than one file returned
            file_results = files_info["params"]["result"]
            if file_results:
                for file in file_results:
                    if file["fileName"]  == "validate_upload.txt":
                        print("File uploaded: validate_upload.txt - OK")
                        loop_done = True
                        break
            if loop_done == True:
                break
        else:
            if tries == 0:
                print("File list could not be retrieved - FAIL")
                fails.append("File list retrieval")

    # Check that a file was successfully downloaded from the Cloud
    tries = 50
    while tries > 0:
        tries -= 1
        time.sleep(0.5)
        if os.path.isfile(os.path.abspath("validate_download.txt")):
            print("File downloaded: validate_download.txt - OK")
            os.remove(os.path.abspath("validate_download.txt"))
            break
        else:
            if tries == 0:
                print("Could not find downloaded file - FAIL")
                fails.append("Download file")

    rest_exec_upload(session_id, thing_key)
    rest_exec_download(session_id, thing_key)

    method_exec(session_id, thing_key, "quit_app")
    time.sleep(2)
    delete_thing(session_id, thing_key)
    return fails

if __name__ == "__main__":
    fails = []
    parser = argparse.ArgumentParser(description="Validation script")
    parser.add_argument("-a", "--app", help="Name of validation app to run")
    parser.add_argument("-r", "--runtime_dir", help="Runtime directory directory")
    parser.add_argument("-c", "--config_file", help="Configuration directory directory")
    args = parser.parse_args(sys.argv[1:])
    if args.app:
        app_file = args.app
    else:
        # default to the python app
        app_file = "validate_app.py"

    if args.runtime_dir:
        runtime_dir = args.runtime_dir
    else:
        # default to the python app
        runtime_dir = "."

    if args.config_file:
        config_file = args.config_file
    else:
        # default to the python app
        config_file = "validate.cfg"
        

    fails = main()
    if not fails:
        print("\n\nAll passed! Success.")
        sys.exit(0)
    else:
        print("\n\nFailed on the following:")
        for fail in fails:
            print(fail)
        sys.exit(1)


