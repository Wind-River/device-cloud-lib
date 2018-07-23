#!/bin/bash

#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
#

REQUIRED_BUILD_PACKAGES_UB1604="\
	bison libxml2-utils gperf curl libstdc++6:i386 zlib1g:amd64 zlib1g:i386 flex adb"

REQUIRED_BUILD_PACKAGES_UB1404="\
	git-core gnupg flex bison gperf build-essential \
	zip curl zlib1g-dev:amd64 gcc-multilib g++-multilib libc6-dev-i386 \
	lib32ncurses5-dev x11proto-core-dev libx11-dev:amd64 lib32z1-dev \
	ccache libgl1-mesa-dev libxml2-utils xsltproc unzip gperf \
	ccache lib32ncurses5-dev g++-multilib \
	zlib1g:amd64 zlib1g:i386 \
	android-tools-adb android-tools-fastboot \
"
ANDROID_CURRENT="/opt/Android/CURRENT"
ODROID_CURRENT="/opt/Android/odroidxu-5.1.1_CURRENT"
ODROID_UBOOT="/opt/Android/odroidxu4_sd_install.img"

# All boolean variables must be initialized
DI_DEBUG=0
DI_ERROR_EXIT=1
DI_UPDATE_ADB=0

# DI_DEBUG=1
#debug
if [ "${DI_DEBUG}" == "1" ]
then
	set -x
fi

#===============================================================================
# Functions
#===============================================================================
function show_info
{
	echo " INFO: " "$@"
}

function show_warning
{
	echo -ne "\e[1;32m"; echo -n "WARNING: " "$@"; echo -ne "\e[0m\n"
}

function show_error
{
	echo -ne "\e[1;31m"; echo; echo -n "ERROR: " "$@"; echo -ne "\e[0m\n"
	[ "$DI_ERROR_EXIT" = "1" ] && exit 1
}

function show_debug
{
	[ "$DI_DEBUG" = "1" ] && echo "DEBUG: " "$@"
}

function show_debug_ne
{
	[ "$DI_DEBUG" = "1" ] && echo -ne "$@"
}

function exit_error
{
	show_error "$@"
	echo

	exit 1
}

function echo_bold
{
	echo -e "\e[1m${1}\e[21m"
}

function usage
{
	echo "Missing script args."
	echo "Usage - $0 build_dir"
	echo " such as:"
	echo "  $0 ."
}

# build emulator and image
# $1: job number 
# $2: target name
# current directory is android source repo
#
function build_emulator_sdcard_image
{
	job_num=${1}
	target_name=${2}
	if [ -f ./build/envsetup.sh ]
	then
		source ./build/envsetup.sh
		lunch ${target_name} 
		[ $? -ne 0 ] && exit_error "Failed: lunch ${target_name}"
		make update-api
		show_debug "make -j${job_num}"
		make -j${job_num}
		[ $? -ne 0 ] && exit_error "Failed: make -j${job_num}"

		# build mksdcard tool
		mmm sdk/emulator/mksdcard
		[ $? -ne 0 ] && show_error "mmm sdk/emulator/mksdcard "
		# make sd image
		mksdcard -l hdcsd 64M hdcsd.img
		[ $? -ne 0 ] && show_error "mksdcard -l hdcsd 64M hdcsd.img"
	else
		exit_error "$(pwd) doesn't have android source code"
	fi
	show_info "Build is completed."
	show_info "sdcard image: $(pwd)/hdcsd.img"
	show_info "Pick a MAC address for your emulated Android device and start the emulator"
	show_info "  emulator -selinux permissive -sdcard hdcsd.img -qemu -net nic,vlan=0,macaddr=xx:yy:zz:tt:uu:vv -net user,vlan=0"
}

# build odroidxu sdcard image
# $1: job number 
# $2: target name
# current directory is android source repo
#
function build_odroid_sdcard_image
{
	job_num=${1}
	target_name=${2}
	if [ -f ./build.sh ]
	then
#		if [ ! -f ./wrs_build.sh ]
#		then
#			cp ./build.sh ./wrs_build.sh
#			current_dir=$(pwd);
#			sed -i 's!/tmp!$(pwd)!g' ./wrs_build.sh
#			sed -i "s/CPU_JOB_NUM=.*/CPU_JOB_NUM=${job_num}/" ./wrs_build.sh
#		fi
#		show_info "build for odroid xu4..."
#		show_info ./wrs_build.sh "${target_name}" "${build_option}"
#		./wrs_build.sh "${target_name}" "${build_option}"

		# collect files for building sd_install image
		install_dir="${target_name}_sd_install"
		if [ -f "${ODROID_UBOOT}" ]
		then
			show_info "creating sd install image"
			show_info ""

			show_info "copying sd install image template"
			cp -f "${ODROID_UBOOT}" ./odroidxu4_sd_install.img
			if [ ! -d "${install_dir}" ]
			then
				mkdir -p "${install_dir}"
			fi
			show_info "mount sd install image"
			sudo mount -o loop,offset=90177536 ./odroidxu4_sd_install.img "${install_dir}"
			if [ $? -ne 0 ]
			then
				exit_error "Failed to mount the install image"
			fi
			show_info "copying build files into image"
			if [ -f "out/target/product/${target_name}/ramdisk.img" ]
			then
				sudo cp -f "out/target/product/${target_name}/ramdisk.img" "${install_dir}"
			else
				exit_error "Failed: out/target/product/${target_name}/ramdisk.img"
			fi
			sudo cp -f ${target_name}/update/* "${install_dir}"
			sudo cp -f ${install_dir}/zImage-dtb ${install_dir}/zImage
			sync
			show_info "unmount sd install image"
			sudo umount -f "${install_dir}"
			rm -rf "${install_dir}"
			if [ -f ./odroidxu4_sd_install.img ]
			then
				show_info "gzip ./odroidxu4_sd_install.img"
				gzip -f ./odroidxu4_sd_install.img
			fi
		else
			exit_error "${ODROID_UBOOT} doesn't exit. The uboot files are not available"
		fi
	else
		exit_error "$(pwd) doesn't have android source code"
	fi

	show_info "Build is completed."
	show_info "update package: $(pwd)/${TARGET_NAME}"
	show_info "sd_install image: $(pwd)/odroidxu4_sd_install.img.gz"
}

# build sdcard image
# $2: target name
# current directory is android source repo
#
function build_sdcard_image
{
	job_num=10
	target_name=${1}
	if [ "${target_name}" == "odroidxu3" ]
	then
		build_odroid_sdcard_image ${job_num} ${target_name}
	else
		exit_error "Failed: Only supporting ${target_name} for now"
		#build_emulator_sdcard_image ${job_num} ${target_name}
	fi
}

# check and install required package on the host
# $1 package name
function check_install_package
{
	show_info "check packge ${i}"
	if [ 0 -eq $( dpkg -l | grep -c "ii  ${1} " ) ]
	then
		sudo apt-get install -y $1
		if [ $? -ne 0 ]
		then
			exit_error "Failed in command sudo apt-get install -y $1"
		fi
	fi
}

# check and install required packages on the host
function check_install_packages
{
	on_ubuntu=$( uname -a | grep -c -i ubuntu )
	if [ 0 -eq ${on_ubuntu} ]
	then
		exit_error "Exit: not on ubuntu."
	fi

	ubuntu_1404=$( cat /etc/lsb-release | grep DISTRIB_RELEASE | grep -c "14.04" )
	ubuntu_1604=$( cat /etc/lsb-release | grep DISTRIB_RELEASE | grep -c "16.04" )

	# jdk7-jdk
	if [ 0 -eq $( dpkg -l | grep -c "jdk-7-jdk" ) ]
	then
		sudo add-apt-repository -y ppa:openjdk-r/ppa
		sync
		echo "apt-get update ..."
		sudo apt-get update
		sync
		sudo apt-get install -y openjdk-7-jdk
		if [ $? -ne 0 ]
		then
			exit_error "Failed in command sudo apt-get install -y $1"
		fi
		show_info "please choose version 7"
		sudo update-alternatives --config java
		sudo update-alternatives --config javac
		java -version
	fi

	if [ 1 -eq ${ubuntu_1404} ]
	then
		for i in ${REQUIRED_BUILD_PACKAGES_UB1404}; do
			check_install_package $i; 
		done
		if [ 1 -eq $( adb version | grep -c "1.0.31" ) ]
		then
			show_info "adb verion is 1.0.31."

			if [ "DI_UPDATE_ADB" = "1" ]
			then
				# update adb
				wget -O - https://skia.googlesource.com/skia/+archive/cd048d18e0b81338c1a04b9749a00444597df394/platform_tools/android/bin/linux.tar.gz | tar -zxvf - adb
				sudo mv adb /usr/bin/adb
				sudo chmod a+x /usr/bin/adb
			else
				show_info "to upgrade adb"
				show_info " wget -O - https://skia.googlesource.com/skia/+archive/cd048d18e0b81338c1a04b9749a00444597df394/platform_tools/android/bin/linux.tar.gz | tar -zxvf - adb"
				show_info " sudo mv adb /usr/bin/adb"
				show_info " sudo chmod a+x /usr/bin/adb"
			fi
		fi
	elif [ 1 -eq ${ubuntu_1604} ]
	then

		for i in ${REQUIRED_BUILD_PACKAGES_UB1604}; do
			check_install_package $i; 
		done
	else
		exit_error "Exit: not supported ubuntu version."
	fi
}

# clone android repo into the current directory
# using mirror on yow_iotbuild2 if possible
# otherwise, downloading from google source
# $1 : job number 
# $2 : target name 
function clone_android_source_packages
{
	job_num=${1}
	target_name=${2}

	if [ "${target_name}" == "odroidxu3" ]
	then
		src_dir=${ODROID_CURRENT}
	else
		src_dir=${ANDROID_CURRENT}
	fi

	if [ -d ${src_dir} ]
	then
		if [ ! -f ./build/envsetup.sh ]
		then
			show_info "clone android source code ... ( will take a while )"
			show_debug "cp -aru ${src_dir}/* ."
			date
			cp -aru ${src_dir}/* .
			date
		fi
	else
		# get repo tool 
		if [ ! -x ../bin/repo ]
		then
			mkdir -p ../bin
			curl https://storage.googleapis.com/git-repo-downloads/repo > ../bin/repo
			[ $? -ne 0 ] && show_error "curl https://storage.googleapis.com/git-repo-downloads/repo > ../bin/repo"
			chmod a+x ../bin/repo
		fi
		repo_tool="../bin/repo"
		if [ "${target_name}" == "odroidxu3" ]
		then
			android_url="git://github.com/voodik/android.git"
			android_branch="cm-12.1_5422"
		else
			android_url="https://android.googlesource.com/platform/manifest"
			android_branch="android-5.1.1_r19"
		fi

		if [ ! -f ./build/envsetup.sh ]
		then
			show_info "clone android source code ... ( will take a while )"
			show_debug ${repo_tool} init -u ${android_url} -b ${android_branch}
			${repo_tool} init -u ${android_url} -b ${android_branch}
			[ $? -ne 0 ] && show_error "Failed: ${repo_tool} init -u ${android_url} -b ${android_branch}"
			show_info "clone android repo... this will take a few hours ..."
			show_debug "${repo_tool} sync -j${job_num}"
			${repo_tool} sync -j${job_num}
			[ $? -ne 0 ] && show_error "${repo_tool} sync -j${job_num}"
		fi

		# remove the repo tool
		rm -rf ../bin/repo
		if [ 0 -eq $( ls ../bin | wc -l ) ]; then
			rm -rf ../bin
		fi
	fi
}

# clone git repo into the directory specified in $1 if it doesn't exist
# $1 is the directory which holding the 
# $2 : git tag to checkout
# $3 : branch name
# by default, the source should be under "external/hdc"
#  
function clone_wr_iot_source_packages
{
	mkdir -p $1
	pushd $1

	# wr-iot: use the git sha from command line
	for i in wr-iot; do
		if [ ! -d $i ]
		then
			git clone http://stash.wrs.com/scm/hpr/${i}.git -b $3
			[ $? -ne 0 ] && show_error "git clone http://stash.wrs.com/scm/hpr/${i}.git -b $3"
			cd $i
			show_debug "git checkout $2"
			git checkout $2
			[ $? -ne 0 ] && show_error "git checkout $2"
			cd ..
		fi
	done

	# use HEAD of following git repo for now.
	for i in android-ifaddrs curl wr-iot-ccg jansson libarchive mosquitto; do
		if [ ! -d ../$i ] && [ ! -d ./$i ]
		then
			git clone http://stash.wrs.com/scm/hpr/${i}.git -b android
			[ $? -ne 0 ] && show_error "git clone http://stash.wrs.com/scm/hpr/${i}.git -b android"
			cd $i
			show_debug "git checkout HEAD"
			git checkout HEAD 
			[ $? -ne 0 ] && show_error "git checkout HEAD"
			cd ..
		fi
	done

	# update the header files
	WR_IOT_CMAKE_BUILD_DIR="wr-iot-cmake"
	mkdir -p $WR_IOT_CMAKE_BUILD_DIR
	cd $WR_IOT_CMAKE_BUILD_DIR
	cmake -DIOT_DXL_RUNTIME_DIR:PATH="/sdcard/intel/ccg/bin" ../wr-iot
	# copy over the generated header files
	for i in $(find . |grep "\.h$")
	do
		cp -rf $i ../wr-iot/build-sys/android
	done

	# updated the iot-build.h
	file_name="../wr-iot/build-sys/android/iot_build.h"
	if [ $(grep IOT_RUNTIME_DIR ${file_name} | grep sdcard | wc -l) -eq 0 ]
	then
		sed -i 's!#define IOT_RUNTIME_DIR                \"\(.*\)\"!#define IOT_RUNTIME_DIR                \"/sdcard\1\"!' ${file_name}
	fi

	# updated the remaining protocol related header files
	for i in crouton udmp dxl
	do 
		rm -rf ../wr-iot/build-sys/android/iot_${i}.h
		cp -rf ./src/api/protocol-client/out/iot_ipc.h ../wr-iot/build-sys/android/iot_${i}.h
		sed -i "s/ipc/${i}/g" ../wr-iot/build-sys/android/iot_${i}.h
		upper_case_name=$(echo "$i" | awk '{print toupper($0)}')
		sed -i "s/IPC/${upper_case_name}/g" ../wr-iot/build-sys/android/iot_${i}.h
	done
	cd ..
	rm -rf $WR_IOT_CMAKE_BUILD_DIR

	popd
}

# check and apply patch for HOST_x86_common.mk if necessary
# $1:patch file
function check_apply_patch_for_HOST_x86_common.mk
{
	key_change="(clang_2nd_arch_prefix)HOST_TOOLCHAIN_FOR_CLANG)/x86_64-linux/bin"
	if [ 0 -eq $( head -n 16w build/core/clang/HOST_x86_common.mk | grep -c ${key_change} build/core/clang/HOST_x86_common.mk ) ]; then
		patch -p1 < $1
		[ $? -ne 0 ] && show_error "patch -p1 < $1"
	else
		show_info "skip: HOST_x86_common.mk has already been updated"
	fi
}

# check and apply patch for HOST_x86_common.mk if necessary
# $1:patch file
function check_apply_patch_for_generic_no_telephony.mk
{
	if [ 0 -eq $( grep -c 'iot-ccg \\'  build/target/product/generic_no_telephony.mk ) ]; then
		patch -p1 < $1
		[ $? -ne 0 ] && show_error "patch -p1 < $1"
	else
		show_info "skip: generic_no_telephony.mk has been already updated"
	fi
}

# check and apply patch for init.rc if necessary
# $1:patch file
function check_apply_patch_for_init.rc
{
	if [ 0 -eq $( grep -c 'init.hdc.rc'  system/core/rootdir/init.rc ) ]; then
		patch -p1 < $1
		[ $? -ne 0 ] && show_error "patch -p1 < $1"
	else
		show_info "skip: generic_no_telephony.mk has been already updated"
	fi
}

# create patch file for HOST_x86_common.mk which is specified in $1
# $1: patch file for HOST_x86_common.mk
function create_patch_for_HOST_x86_common.mk
{
	echo 'diff --git a/build/core/clang/HOST_x86_common.mk b/build/core/clang/HOST_x86_common.mk
index 0241cb6..f215485 100644
--- a/build/core/clang/HOST_x86_common.mk
+++ b/build/core/clang/HOST_x86_common.mk
@@ -8,7 +8,8 @@ ifeq ($(HOST_OS),linux)
 CLANG_CONFIG_x86_LINUX_HOST_EXTRA_ASFLAGS := \
   --gcc-toolchain=$($(clang_2nd_arch_prefix)HOST_TOOLCHAIN_FOR_CLANG) \
   --sysroot=$($(clang_2nd_arch_prefix)HOST_TOOLCHAIN_FOR_CLANG)/sysroot \
-  -no-integrated-as
+  -no-integrated-as \
+  -B$($(clang_2nd_arch_prefix)HOST_TOOLCHAIN_FOR_CLANG)/x86_64-linux/bin
 
 CLANG_CONFIG_x86_LINUX_HOST_EXTRA_CFLAGS := \
   --gcc-toolchain=$($(clang_2nd_arch_prefix)HOST_TOOLCHAIN_FOR_CLANG) \' > $1
}

# create patch file for generic_no_telephony.mk which is specified in $1
# $1: patch file for generic_no_telephony.mk
function create_patch_for_generic_no_telephony.mk
{
	echo 'index 7af62ce..e59f127 100644
--- a/build/target/product/generic_no_telephony.mk
+++ b/build/target/product/generic_no_telephony.mk
@@ -52,7 +52,28 @@ PRODUCT_PACKAGES += \
 PRODUCT_PACKAGES += \
     local_time.default
 
+PRODUCT_PACKAGES += \
+    libandroidifaddrs \
+    libjansson \
+    libarchive \
+    libcurl \
+    iot-ccg \
+    libmosquitto \
+    mosquitto \
+    mosquitto_sub \
+    mosquitto_pub \
+    iot-device-manager \
+    iot-service \
+    libiot \
+    iot-app-simple-telemetry \
+    iot-app-simple-actions \
+    actions_script_android.sh \
+    wr_decommission_device_android.sh \
+    wr_software_updates_android.sh \
+    wr_restore_factory_images_android.sh \
+
 PRODUCT_COPY_FILES := \
+        external/hdc/wr-iot/build-sys/android/init.hdc.rc:root/init.hdc.rc \
         frameworks/av/media/libeffects/data/audio_effects.conf:system/etc/audio_effects.conf
 
 PRODUCT_PROPERTY_OVERRIDES += \' > $1
}

# create patch file for init.rc which is specified in $1
# $1: patch file for init.rc
function create_patch_for_init.rc_emulator
{
	echo 'index cbcb842..0f20fd7 100644
--- a/system/core/rootdir/init.rc
+++ b/system/core/rootdir/init.rc
@@ -9,6 +9,7 @@ import /init.usb.rc
 import /init.${ro.hardware}.rc
 import /init.${ro.zygote}.rc
 import /init.trace.rc
+import /init.hdc.rc
 
 on early-init' > $1
}

# create patch file for init.rc which is specified in $1
# $1: patch file for init.rc
function create_patch_for_init.rc_odroidxu
{
	echo "index cbcb842..0f20fd7 100644
--- a/system/core/rootdir/init.rc
+++ b/system/core/rootdir/init.rc
@@ -13,4 +13,5 @@ # Include CM's extra init file
 import /init.cm.rc
+import /init.hdc.rc
 
 
 on early-init" > $1
}

# create patch file for init.rc which is specified in $1
# $1: patch file for init.rc
# $2: target name
function create_patch_for_init.rc
{
	patch_file_name="$1"
	target_name="$2"
	if [ "${target_name}" == "odroidxu3" ]
	then
		create_patch_for_init.rc_odroidxu "${patch_file_name}"
	else
		create_patch_for_init.rc_emulator "${patch_file_name}"
	fi
}

#function check_apply_patch_for_generic_no_telephony.mk
# $1: build_dir 
# $2: target name 
function main
{
	build_dir=${1}
	target_name=${2}

	if [ ! -d $1 ]
	then
		exit_error "Failed: ${build_dir} doesn't exist."
		mkdir -p $1
	fi
	pushd $1
	build_sdcard_image ${target_name}
	popd
}

#

# Check that we have the expected args
if [ $# -lt 1 ]; then
	usage
	exit 1
fi

BUILD_DIR=${1}
TARGET_NAME="odroidxu3"

main ${BUILD_DIR} ${TARGET_NAME}
