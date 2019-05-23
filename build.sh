#!/bin/bash -x
#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
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
# This script downloads, builds and installs project dependencies for the
# device-cloud-lib into sub-directory called "deps".  Once the script completes,
# there will be a directory called "build".  Setup cmake in that directory and
# run "make" in the build directory.  For more information See README.md.
#
# Usage:
#  $ ./build.sh
# --------------------------------------------------------------------

# exit on error
set -e

# Required system dependencies:
#    See README.md for list of dependencies from the build host
#
SCRIPT_PATH="$( cd "$(dirname "$0")" ; pwd -P )"
export DEPS_DIR=`pwd`/deps
mkdir -p "$DEPS_DIR"
mkdir -p "$DEPS_DIR/include"
mkdir -p "$DEPS_DIR/lib"

CMAKE_ARGS="-v"

# Get the name of current branch
if [ ! -z "$TRAVIS_PULL_REQUEST_BRANCH" ]; then
	BRANCH_NAME="$TRAVIS_PULL_REQUEST_BRANCH"
elif [ ! -z "$TRAVIS_BRANCH" ]; then
	BRANCH_NAME="$TRAVIS_BRANCH"
else
	CWD=`pwd`
	cd "$SCRIPT_PATH"
	BRANCH_NAME=`git symbolic-ref --short HEAD || true`
	cd "$CWD"
fi

# operating system abstraction layer
git clone https://github.com/Wind-River/device-cloud-osal.git device-cloud-osal
cd device-cloud-osal
# test if branch exists on osal layer matching current branch, if so, change to
if [ ! -z "$BRANCH_NAME" ]; then
	git checkout "$BRANCH_NAME" 2>/dev/null || true
fi
if [ ! -e cmake_build ]; then
	mkdir cmake_build
fi
cd cmake_build
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH="$DEPS_DIR" -DOSAL_THREAD_SUPPORT:BOOL=$THREAD_SUPPORT -DOSAL_WRAP:BOOL=ON ..
make
make install
cd ../..
rm -rf device-cloud-osal

if [ "$USE_CMOCKA" != "" ]; then
	# cmocka
	wget https://cmocka.org/files/1.1/cmocka-1.1.1.tar.xz
	tar -xvf cmocka-1.1.1.tar.xz
	if [ -e cmocka-1.1.1 ]; then
		cd cmocka-1.1.1
		if [ ! -e cmake_build ]; then
			mkdir cmake_build
		fi
		cd cmake_build
		cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH="$DEPS_DIR" ..
		make
		make install
		cd ../..
		rm -rf cmocka-1.1.1
	fi
fi

if [ "$USE_JANSSON" != "" ]; then
	# jansson
	CMAKE_ARGS="${CMAKE_ARGS} -DIOT_JSON_LIBRARY:STRING=jansson"
else
	# jsmn
	git clone https://github.com/zserge/jsmn.git jsmn
	cd jsmn
        git checkout v1.0.0
	make clean
	make "CFLAGS=-DJSMN_PARENT_LINKS=1 -DJSMN_STRICT=1 -fPIC"
	make test
	cmake -E copy "jsmn.h" "$DEPS_DIR/include/"
	cmake -E copy "libjsmn.a" "$DEPS_DIR/lib/"
	cd ..
	rm -rf jsmn
fi

if [ "$USE_CIVETWEB" != "" ]; then
	# civetweb
	export CIVETWEB_GIT_TAG="f04a9f7411731f69751255b1e1a97157fe3cc812"
	#export CIVETWEB_GIT_TAG="tags/v1.11"  # Version 1.11 doesn't exist yet
	git clone https://github.com/civetweb/civetweb.git
	cd civetweb
	git checkout -b build_version $CIVETWEB_GIT_TAG
	if [ ! -e cmake_build ]; then
		mkdir cmake_build
	fi
	cd cmake_build
	cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH="$DEPS_DIR" -DCIVETWEB_ENABLE_WEBSOCKETS:BOOL=ON -DCIVETWEB_ENABLE_IPV6:BOOL=ON -DCIVETWEB_ENABLE_SSL:BOOL=ON -DCIVETWEB_SSL_OPENSSL_API_1_1:BOOL=ON ..
	make
	make install
	cd ../..
	rm -rf civetweb
	CMAKE_ARGS="${CMAKE_ARGS} -DIOT_WEBSOCKET_LIBRARY:STRING=civetweb"
else
	# libwebsockets
	export LWS_GIT_TAG="tags/v2.3.0"
	git clone https://github.com/warmcat/libwebsockets.git libwebsockets
	cd libwebsockets
	git checkout -b build_version $LWS_GIT_TAG
	if [ ! -e cmake_build ]; then
		mkdir cmake_build
	fi
	cd cmake_build
	cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH="$DEPS_DIR" ..
	make
	make install
	cd ../..
	rm -rf libwebsockets
fi

if [ "${USE_MOSQUITTO}" != "" ]; then
	# mosquitto
	export MOSQUITTO_GIT_TAG="tags/v1.4.14"
	git clone https://github.com/eclipse/mosquitto.git
	cd mosquitto
	git checkout -b build_version $MOSQUITTO_GIT_TAG
	find . -name CMakeLists.txt | xargs sed -i 's/ldconfig/ldconfig ARGS -N/'
	sed -i 's/add_subdirectory(man)//' CMakeLists.txt
	if [ ! -e cmake_build ]; then
		mkdir cmake_build
	fi
	cd cmake_build
	cmake -DWITH_SRV:BOOL=NO -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH="$DEPS_DIR" ..
	make
	make install
	cd ../..
	rm -fr mosquitto
	CMAKE_ARGS="${CMAKE_ARGS} -DIOT_MQTT_LIBRARY:STRING=mosquitto"
else
	# paho
	export PAHO_GIT_TAG="tags/v1.2.0"
	git clone https://github.com/eclipse/paho.mqtt.c.git paho
	cd paho
	git checkout -b build_version $PAHO_GIT_TAG
	if [ ! -e cmake_build ]; then
		mkdir cmake_build
	fi
	cd cmake_build
	cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH="$DEPS_DIR" -DPAHO_WITH_SSL:BOOL=TRUE -DPAHO_BUILD_STATIC:BOOL=TRUE -DCMAKE_C_FLAGS:STRING=-fPIC ..
	make
	make install
	cd ../..
	rm -rf paho
fi

if [ "${DWTUNSVR_ENABLED}" != "" ]; then
	#export DWUTIL_GIT_TAG="tags/"	
	git clone https://github.com/deviceWISE/dwtunsvr.git dwtunsvr
	cd dwtunsvr
	sh build.sh
	#git checkout -b build_version $DWUTIL_GIT_TAG 
	if [ ! -e cmake_build ]; then
		mkdir cmake_build
	fi
	cd cmake_build
	cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH="$DEPS_DIR" ..
	make
	make install
	cd ../../
	rm -fr dwtunsvr
fi

cmake  $CMAKE_ARGS -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DIOT_DWTUNSVR_ENABLED:BOOL=${DWTUNSVR_ENABLED} -DIOT_THREAD_SUPPORT:BOOL=$THREAD_SUPPORT -DIOT_STACK_ONLY:BOOL=$STACK_ONLY -DDEPENDS_ROOT_DIR:PATH="$DEPS_DIR" "$SCRIPT_PATH"

echo
echo "device-cloud-lib is ready to build."
echo
