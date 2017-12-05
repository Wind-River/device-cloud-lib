#!/bin/bash
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
# --------------------------------------------------------------------
# Description:
# This script downloads, builds and installs into CWD/deps with
# the dependencies for the device-cloud-lib API and applications.  Once the
# script completes, there will be a directory called "build".  Run
# "make" in the build directory.
#
# Usage:
#  $ ./build-deps.sh
# Installs into CWD/deps
# --------------------------------------------------------------------

# Required system dependencies:
#    Ubuntu:
#	$ sudo apt-get install build-essential git cmake libssl-dev libwebsockets-dev  \
#		uuid-dev libcurl4-openssl-dev libarchive-dev
#
# Repository access.  If device-cloud-osal is not public, make sure to
# enable ssh clone access in your github account (e.g. add ssh key)
#
export DEPS_DIR=`pwd`/deps
mkdir -p $DEPS_DIR

# cmocka
wget https://cmocka.org/files/1.1/cmocka-1.1.1.tar.xz
tar -xvf cmocka-1.1.1.tar.xz
cd cmocka-1.1.1
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR ..
make
make install
cd ../..
rm -rf cmocka-1.1.1

# jsmn
git clone https://github.com/zserge/jsmn.git jsmn
cd jsmn
make clean
make "CFLAGS=-DJSMN_PARENT_LINKS=1 -DJSMN_STRICT=1 -fPIC"
make test
cmake -E copy "libjsmn.a" "$DEPS_DIR/lib"
cmake -E copy "jsmn.h" "$DEPS_DIR/include"
cd ..
rm -rf jsmn

# libwebsockets
export LWS_GIT_TAG=v2.3.0
git clone https://github.com/warmcat/libwebsockets.git libwebsockets
cd libwebsockets
git checkout tags/$LWS_GIT_TAG
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR .
make
make install
cd ..
rm -rf libwebsockets

# operating system abstraction layer
git clone ssh://git@github.com/Wind-River/device-cloud-osal.git device-cloud-osal
cd device-cloud-osal
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR -DSOAL_THREAD_SUPPORT:BOOL=ON -DOSAL_WRAP:BOOL=ON .
make
make install
cd ..
rm -rf device-cloud-osal

# paho
export PAHO_GIT_TAG=v1.2.0
git clone https://github.com/eclipse/paho.mqtt.c.git paho
cd paho
git checkout tags/$PAHO_GIT_TAG
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR -DCMAKE_INSTALL_LIBDIR:PATH=lib -DPAHO_WITH_SSL:BOOL=TRUE -DPAHO_BUILD_STATIC:BOOL=TRUE -DCMAKE_C_FLAGS:STRING=-fPIC .
make
make install
cd ..
rm -rf paho

# mosquitto
export MOSQUITTO_GIT_TAG=v1.4.14
git clone https://github.com/eclipse/mosquitto.git
cd mosquitto
git checkout tags/$MOSQUITTO_GIT_TAG
find . -name CMakeLists.txt | xargs sed -i 's/ldconfig/ldconfig ARGS -N/'
sed -i 's/add_subdirectory(man)//' CMakeLists.txt
cmake -DWITH_SRV:BOOL=NO -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR .
make
make install
cd ..
rm -fr mosquitto

# build device-cloud lib, if you want to use mosquitto instead of paho
# uncomment the following
#USE_MOSQUITTO=-DIOT_MQTT_LIBRARY:STRING=mosquitto

rm -fr build
mkdir build
cd build
cmake  $USE_MOSQUITTO -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DDEPENDS_ROOT_DIR:PATH=$DEPS_DIR ..

echo
echo "device-cloud-lib is ready to build."
echo "cd build && make"
echo
