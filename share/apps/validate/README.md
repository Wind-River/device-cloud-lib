Validate App
============
This app is designed to be called on a Linux system during CI, e.g.
on github.com by the travis service when validating a pull request.

Usage
-----
This validate_script.py expects the following directories to exist:
  * /var/lib/iot (writes device_id here)
  * /etc/iot     (writes the validate-app.cfg here)
It expects to run from the $workspace/bin directory.

Build the code
--------------
```sh
./build.sh
make
```

Running from travis
-------------------
The device-cloud-python CI will spawn 8 builds concurrently, and uses
the AUTO_PREFIX environmental variable to prevent collisions.  See
.travis.yml for details.

Travis calls the following for each build instance:
```sh
cp share/apps/validate/*.py bin
cd bin
./validate_script.py -a iot-app-validate  -r /var/lib/iot -c /etc/iot/validate-app.cfg
```

