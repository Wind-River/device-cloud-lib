Admin Tools
===========

This directory contains scripts that are suitable for setting up an
org or updating an org.  The following things are supported:
  * thing defs
  * applications

Thing Definitions
-----------------
Thing defs are defined in one of the cfg files in thing_defs/.  Each
file contains a json string that was copied from the cloud thing
definition from the "advanced view".  One change must be made to the
thing def json string that was copied:

  * remove the "id" and "ownerOrgId".  These are dynamically
  generated.

Each thing def cfg file can contain multiple thing defs (comma
separated).  "thing_definitions" is a json array.  The json must be valid.

Applications
------------
Applications are defined in app_defs/.  Each file contains a json
string describing the app.  Multiple apps can be defined in one file.
It is a json array.  "app_defintions" is a json array.  The json must
be valid.

Note:
Applications need to have a thing defintion associated.  E.g:
In the app definition, the autoRegThingDefId must point to a thing
definition key from one of the thing_def config files.

```
"app_definitions":[
    {
        "name":"hdc_dm_default_app",
        "desc":"hdc_dm_default_app",
        "token":"",
        "autoRegThingDefId":"hdc_dm_default_def",
        "autoRegTags":["", ""],
        "autoRegSecTags":["", ""],
        "roles":"",
        "isSuperAdmin":false,
        "isSuperOps":false,
        "isOrgAdmin":true,
        "license":""
    }
    ]
```

Usage
-----
The default apps and thing defs are maintain in the app_defs/ and
thing_defs/ directories.  The script will detect if the thing def or
app is new or existing, and then create or update the definition.
```sh
# put your credentials in the env
HDCADDRESS=core-api.hdcstg.net
HDCUSERNAME=(your login user name to the clouid)
HDCPASSWORD=(your login password to the cloud)
HDCORG=(optional: different org to setup)
export HDCADDRESS HDCUSERNAME HDCPASSWORD HDCORG

# alternatively, the script will prompt for credentials

./setup-cloud-deps.py
Cloud: core-api.hdcstg.net
Session ID: 59b947156c313133794e4c52 - OK
Confirmed user has admin access
Processing thing definitions
Processing thing_defs/demo_defs.cfg
Updating thing def hdc_simple_location_def
Updating thing def hdc_simple_telemetry_def
Updating thing def hdc_simple_actions_def
Processing thing_defs/validate_thing_def.cfg
Updating thing def hdc_validate_def
Processing thing_defs/hdc_device_manager_def.cfg
Updating thing def hdc_dm_default_def
Processing application definitions
Processing app_defs/demo_apps.cfg
Updating app def hdc_simple_location_app
Found thing def id 59b83de47902896e3b54e60d for thing def hdc_simple_location_def
Updating app def hdc_simple_telmetry_app
Found thing def id 59b83de57902896e3b54e614 for thing def hdc_simple_telemetry_def
Updating app def hdc_simple_actions_app
Found thing def id 59b83de77902896e3b54e617 for thing def hdc_simple_actions_def
Processing app_defs/validate_app.cfg
Updating app def hdc_validate_app
Found thing def id 59b0599c7902896e3b3bb673 for thing def hdc_validate_def
Processing app_defs/hdc_device_manager_app.cfg
Updating app def hdc_dm_default_app
Found thing def id 5989cd1774e9366084696f35 for thing def hdc_dm_default_def
```

Now, verify that the definitions and apps are correct in the cloud.
