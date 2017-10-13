#!/bin/bash
# --------------------------------------------------------------------
# Description:
# This script downloads, builds and installs into CWD/deps with
# the dependencies for the hdc-lib API and applications.  Once the
# script completes, there will be a directory called "build".  Run
# "make" in the build directory.
#
# Usage:
#  $ ./build-deps.sh
# Installs into CWD/deps
# --------------------------------------------------------------------

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
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR
make
make install
cd ..
rm -rf libwebsockets

# operating system abstraction layer
git clone ssh://git@github.com/Wind-River/hdc-osal.git hdc-osal
cd hdc-osal
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR -DSOAL_THREAD_SUPPORT:BOOL=ON -DOSAL_WRAP:BOOL=ON .
make
make install
cd ..
rm -rf hdc-osal

# paho
export PAHO_GIT_TAG=v1.2.0
git clone https://github.com/eclipse/paho.mqtt.c.git paho
cd paho
git checkout tags/$PAHO_GIT_TAG
cmake -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR -DCMAKE_INSTALL_LIBDIR:PATH=lib -DPAHO_WITH_SSL:BOOL=TRUE -DPAHO_BUILD_STATIC:BOOL=TRUE -DCMAKE_C_FLAGS:STRING=-fPIC
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
cmake -DWITH_SRV:BOOL=NO -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR
make
make install
cd ..
rm -fr mosquitto

# libarchive
export LIBARCHIVE_GIT_TAG=master
git clone https://github.com/libarchive/libarchive.git
cd libarchive
git checkout tags/$LIBARCHIVE_GIT_TAG
cmake  -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX:PATH=$DEPS_DIR
make
make install
cd ..
rm -fr libarchive


# build hdc lib, if you want to use mosquitto instead of paho
# uncomment the following
#USE_MOSQUITTO=-DIOT_MQTT_LIBRARY:STRING=mosquitto

rm -fr build
mkdir build
cd build
cmake  $USE_MOSQUITTO -DCMAKE_BUILD_TYPE:STRING=$BUILD_TYPE -DDEPENDS_ROOT_DIR:PATH=$DEPS_DIR ..

echo
echo "hdc-lib is ready to build."
echo "cd build && make"
echo
