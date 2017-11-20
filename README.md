Wind River's Internet of Things Solution
========================================

This is the base source code tree for Wind River's Internet of Things (IoT)
solution, code named: Device Cloud. It contains several improvements over
previous versions for of the solution.

Features Included are:
- Portability (ported to multiple operating systems)
- API is operating system independent
- API is can be either threaded or non-threaded
- Dynamic plug-in architecture allowing for add-ons within the library
- Easier device API (reduction in api, makes easier to use for developers)
- Agent support is provided all within a library

Building
--------
Required system dependencies:
  * Fedora (22+):
```sh
sudo dnf install git gcc cmake openssl-devel libcurl-devel libarchive-devel
```
  * Ubuntu:
```sh
sudo apt-get install build-essential git cmake libssl-dev \
               libcurl4-openssl-dev libarchive-dev
```

Note: regaring repository access, device-cloud-osal is not public.  Make sure to
enable ssh clone access in your github account (e.g. add ssh key), or
the `build.sh` script will fail to clone the device-cloud-osal repository.

```sh
rm -rf build
mkdir build
cd build
set BUILD_TYPE=Release
../build.sh
# if you have build issues, it is typically caused by missing build dependencies
make

# Optional: make check
This will build and run unit tests on the code base

# Optional: make package
This will build a package for installing the code base

# Optional: install
sudo make install
or
make install
```
Note: if you use DESTDIR, make sure the libiot.so can be found, e.g.
update ld.so.conf.d,  LD_LIBRARY_PATH etc.  Run 'sudo ldconfig' if the
ld.so.conf.d was changed.


Running
-------
From the directory you ran "make in, do the following to test an
application.  Note: you will need credentials e.g. an app token to
connect.  The "iot-control" tool will create the correct
iot-connect.cfg file in /etc/iot.

```sh
# Recommended way:
sudo make install
iot-control (answer questions)
iot-app-complete

# Alternatively: make some dirs for the config and device_id
mkdir -p /etc/iot
mkdir -p /var/lib/iot

# run the configure tool answer the questions.
bin/iot-control

# run an app
bin/iot-app-complete
```
