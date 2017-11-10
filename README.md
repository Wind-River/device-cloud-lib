Wind River's Internet of Things Solution
========================================

This is the base source code tree for Wind River's Internet of Things (IoT)
solution.  It contains several improvements over previous versions for of the
solution.

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
  * Ubuntu:
```sh
sudo apt-get install build-essential git cmake libssl-dev libwebsockets-dev  \
               uuid-dev libcurl4-openssl-dev libarchive-dev
```

Note: regaring repository access, device-cloud-osal is not public.  Make sure to
enable ssh clone access in your github account (e.g. add ssh key), or
the build-deps.sh script will fail to clone the device-cloud-osal repo.

```sh
./build-deps.sh
# if you have build issues, it is typically caused by missing deps
cd build
make
```

Running
-------
From the directory you ran "make in, do the following to test an
application.  Note: you will need credentials e.g. an app token to
connect.  The "iot-control" tool will create the correct
iot-connect.cfg file in /etc/iot.

```sh
# make some dirs for the config and device_id
mkdir -p /etc/iot
mkdir -p /var/lib/iot

# run the configure tool answer the questions.
bin/iot-control

# run an app
bin/iot-app-complete
```
