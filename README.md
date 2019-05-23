[![Coverity Scan Build Status](https://scan.coverity.com/projects/15034/badge.svg)](https://scan.coverity.com/projects/15034)

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

### Building for Linux ###
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

```sh
rm -rf build
mkdir build
cd build
export BUILD_TYPE=Release #for additional debug logging, use 'BUILD_TYPE=Debug'
export DWTUNSVR_ENABLED=1 #to use devicewise tunneling feature
../build.sh
# if you have build issues, it is typically caused by missing build dependencies
make


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

### Building for Android ###
From the source tree root
```
cd build-sys/wr-hdc-buildinfo/
mkdir build
./android_build_wr-iot-lib.sh build num_of_jobs git_tag branch_name target_name [platform | all]
```
This builds and android image from source packages and builds the device cloud library on top of it.
This will take up ~160GB of disk space. (note: this will recursively checkout this repository)

Unit Testing & Coverage
-------
Required system depndencies:
  * CMocka: [git://git.cryptomilk.org/projects/cmocka.git](git://git.cryptomilk.org/projects/cmocka.git)
  * lcov:	https://github.com/linux-test-project/lcov

Make sure you don't have any preinstalled CMocka - install from the repository above.
Older versions will cause the tests to fail.

In the folder you ran `build.sh` in, you can run:

`make check` (requires CMocka)
This will build and run unit tests on the code base

`make coverage` (requires lcov)
This will generate coverage reports for code base.
Requires you to edit the `CMAKE_BUILD_TYPE:STRING=""` line to `CMAKE_BUILD_TYPE:STRING=Coverage`



Running
-------
From the directory you ran "make" in, do the following to test an
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
