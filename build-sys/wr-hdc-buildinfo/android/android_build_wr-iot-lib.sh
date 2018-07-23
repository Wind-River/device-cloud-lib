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

# repo information list
declare -A ANDROIDIFADDRS=( [name]="android-ifaddrs" [branch]="android" [sha]="6054f9bd499370e04753c55ad1096459767a13cc" )
declare -A CURL=( [name]="curl" [branch]="android" [sha]="ea62a59546945637373623ea79ee88aedb2437ca" )
declare -A JANSSON=( [name]="jansson" [branch]="android" [sha]="ce6655f248aba4967a3bf6a45d38f9091bb50902" )
declare -A LIBARCHIVE=( [name]="libarchive" [branch]="android" [sha]="b2e4e3a58f51895e4201760b41a5680bf6b15612" )
declare -A LIBWEBSOCKETS=( [name]="libwebsockets" [branch]="master" [sha]="0cbe81bf3ceba5675dd285c568d0bc16e6bc3691" )
declare -A MOSQUITTO=( [name]="mosquitto" [branch]="android" [sha]="278a60c4906733c50f4d56d30522a5897674d0c8" )

declare -a PACKAGES_LIST=( ANDROIDIFADDRS CURL JANSSON LIBARCHIVE LIBWEBSOCKETS MOSQUITTO )
declare -a PACKAGES_LIST_4_IOT_TELIT=( ANDROIDIFADDRS CURL JANSSON LIBARCHIVE LIBWEBSOCKETS MOSQUITTO )

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
	echo "Usage - $0 build_dir num_of_jobs git_tag branch_name target_name [ platform | all ]"
	echo " such as:"
	echo "  $0 build-iot-android 8 HEAD master aosp_arm-eng"
	echo "  $0 build-iot-android 8 HEAD master aosp_mako-eng"
	echo "  $0 build-iot-odroidxu 8 HEAD master odroidxu3"
	echo "  $0 build-iot-odroidxu 8 HEAD master odroidxu3 platform"
	echo "  $0 build-iot-odroidxu 8 HEAD master odroidxu3 all"
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
# $3: build_option
# current directory is android source repo
#
function build_odroid_sdcard_image
{
	job_num=${1}
	target_name=${2}
	build_option=${3}
	if [ -f ./build.sh ]
	then
		if [ ! -f ./wrs_build.sh ]
		then
			cp ./build.sh ./wrs_build.sh
			current_dir=$(pwd);
			sed -i 's!/tmp!$(pwd)!g' ./wrs_build.sh
			sed -i "s/CPU_JOB_NUM=.*/CPU_JOB_NUM=${job_num}/" ./wrs_build.sh
			sed -i "s/-userdebug/-eng/g" ./wrs_build.sh
		fi
		show_info "build for odroid xu4..."
		show_info ./wrs_build.sh "${target_name}" "${build_option}"
		./wrs_build.sh "${target_name}" "${build_option}"
		if [ $? != 0 ]
		then
			exit_error "build failed"
		fi

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
			sudo umount -f "${install_dir}"
			sync
			show_info "unmount sd install image"
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
# $1: job number
# $2: target name
# $3: build_option
# current directory is android source repo
#
function build_sdcard_image
{
	job_num=${1}
	target_name=${2}
	build_option=${3}
	if [ "${target_name}" == "odroidxu3" ]
	then
		build_odroid_sdcard_image ${job_num} ${target_name} ${build_option}
	else
		build_emulator_sdcard_image ${job_num} ${target_name}
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
	# don't use "uname -a" since "it fails in containers/docker etc"
	# on_ubuntu=$( uname -a | grep -c -i ubuntu )
	# using "lsb_release -i" instead.
	on_ubuntu=$( lsb_release -i | grep -c -i ubuntu )
	if [ 0 -eq ${on_ubuntu} ]
	then
		exit_error "Exit: not on ubuntu."
	fi

	# could run "lsb_release -c" instead of "/etc/lsb_release"
	# such as:
	# $ lsb_release -c
	# Codename:	trusty
	# $
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

# clone git repo $1 into the directory specified in $2 if it doesn't exist
# $1 : git repo used by git clone
# $2 is the directory which holding the
# $3 : git tag to checkout
# $4 : branch name
# by default, the source should be under "external/hdc"
#
function clone_git_package
{
	if [ ! -d $2 ]
	then
		show_debug " git clone $1 -b $4 $2 "
		git clone $1 -b $4 $2
	fi

	if [ -d $2 ]
	then
		show_debug "pushd $2; git checkout $3; popd "
		pushd $2
			git checkout $3
		popd
	else
		exit_error "Error $2 doesn't exit "
	fi
}

# clone git repo into the directory specified in $1 if it doesn't exist
# $1 is the directory which holding the
# $2 : git tag to checkout
# $3 : branch name
# by default, the source should be under "external/hdc"
#
# default : paho, wr-iot-lib
# using mk file from .../ws/4gavin
function clone_wr_iot_lib_source_packages
{
	mkdir -p $1
	pushd $1

	# std libraries
	# use HEAD of master following git repo.
	for i in ${PACKAGES_LIST_4_IOT_TELIT[@]}; do
		repo_name=${i}[name]
		repo_branch=${i}[branch]
		repo_sha=${i}[sha]

		if [ ! -d ./${!repo_name} ]
		then
			git clone http://stash.wrs.com/scm/hpr/${!repo_name}.git -b ${!repo_branch}
			[ $? -ne 0 ] && show_error "git clone http://stash.wrs.com/scm/hpr/${!repo_name}.git -b ${!repo_branch}"
			cd ./${!repo_name}
			show_debug "git checkout ${!repo_sha}"
			git checkout ${!repo_sha}
			[ $? -ne 0 ] && show_error "git checkout ${!repo_sha}"
			cd ..
		fi

		# force to using the module from our repo
		if [ -d ../${!repo_name} ]
		then
			show_info "removing ../${!repo_name}"
			rm -rf ../${!repo_name}
		fi
	done


	# paho:
	for i in paho; do
		if [ ! -d $i ]
		then
			# to use HEAD
			# clone_git_package https://github.com/eclipse/paho.mqtt.c.git $i HEAD master
			# sha value having Android support
			clone_git_package https://github.com/eclipse/paho.mqtt.c.git $i 60858cdcc4ed2e56e327ade3cf2f2d7850c9b6e3 master
		fi
		if [ ! -f ${i}/android/Android.mk ]
		then
			exit_error "failed to get paho"
		fi
	done

	# iot jsmn
	for i in iotjsmn; do
		if [ ! -d $i ]
		then
			clone_git_package https://github.com/binli71/jsmn.git $i HEAD master
		fi
		if [ -f ${i}/Android.mk ]
		then
			if [ 0 -eq $(grep -c libiotjsmn ${i}/Android.mk ) ]; then
				sed -i 's!= libjsmn!= libiotjsmn\nLOCAL_CFLAGS    += -DJSMN_PARENT_LINKS -DJSMN_STRICT!g' ${i}/Android.mk
			fi
		else
			exit_error "failed to get jsmn"
		fi
	done

	# osal repo, master / HEAD
	for i in osal; do
		if [ ! -d $i ]
		then
			clone_git_package git@github.com:Wind-River/hdc-osal.git $i HEAD master
		fi
		if [ ! -d $i ]
		then
			exit_error "failed to get osal"
		fi
	done

	# wr-iot-lib
	for i in wr-iot-lib; do
		if [ ! -d $i ]
		then
			clone_git_package git@github.com:Wind-River/hdc-lib.git $i $2 $3
			#clone_git_package http://stash.wrs.com/scm/hpr/wr-iot-lib.git $i $2 $3
		fi
		if [ ! -d $i ]
		then
			exit_error "failed to get hdc-lib"
		fi
	done
	popd
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

	# use HEAD of master following git repo.
	for i in ${PACKAGES_LIST[@]}; do
		repo_name=${i}[name]
		repo_branch=${i}[branch]
		repo_sha=${i}[sha]

		if [ ! -d ./${!repo_name} ]
		then
			git clone http://stash.wrs.com/scm/hpr/${!repo_name}.git -b ${!repo_branch}
			[ $? -ne 0 ] && show_error "git clone http://stash.wrs.com/scm/hpr/${!repo_name}.git -b ${!repo_branch}"
			cd ./${!repo_name}
			show_debug "git checkout ${!repo_sha}"
			git checkout ${!repo_sha}
			[ $? -ne 0 ] && show_error "git checkout ${!repo_sha}"
			cd ..
		fi

		# force to using the module from our repo
		if [ -d ../${!repo_name} ]
		then
			show_info "removing ../${!repo_name}"
			rm -rf ../${!repo_name}
		fi
	done

	popd
}

# check and apply patch for HOST_x86_common.mk if necessary
# $1:patch file
function check_apply_patch_for_HOST_x86_common.mk
{
	key_change="(clang_2nd_arch_prefix)HOST_TOOLCHAIN_FOR_CLANG)/x86_64-linux/bin"
	if [ 0 -eq $( head -n 16 build/core/clang/HOST_x86_common.mk | grep -c "${key_change}" ) ]; then
		patch -p1 < $1
		[ $? -ne 0 ] && show_error "patch -p1 < $1"
	else
		show_info "skip: HOST_x86_common.mk has already been updated"
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


# $1: use our generic_no_telephony.mk
# $2: target name
function create_patch_for_generic_no_telephony.mk_4_wr_iot_lib
{
	pwd
	echo "Updating generic_no_telephony.mk..."
	touch ../wr-hdc-buildinfo/android/generic_no_telephony.mk
	cp ../wr-hdc-buildinfo/android/generic_no_telephony.mk $1
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

# $1: build_dir
# $2: job number
# $3: git sha1 or tag
# $4: branch name
# $5: target name
# $6: build_option
function main
{
	BUILD_DIR=${1}
	JOB_NUM=${2}
	GIT_TAG=${3}
	BRANCH_NANE=${4}
	TARGET_NAME=${5}
	build_option=${6}

	check_install_packages

	if [ ! -d $1 ]
	then
		mkdir -p $1
	fi
	pushd $1

	temp_file="${PWD}/tmp.patch"

	# clone the android source ( will take hours )
	clone_android_source_packages ${JOB_NUM} ${TARGET_NAME}

	rm -rf $temp_file
	# apply patch to build env
	create_patch_for_HOST_x86_common.mk $temp_file
	check_apply_patch_for_HOST_x86_common.mk $temp_file

	rm -rf $temp_file

	clone_wr_iot_lib_source_packages external/hdc ${GIT_TAG} ${BRANCH_NAME}

	echo "Updating generic_no_telephony.mk..."
	cp -f ../wr-hdc-buildinfo/android/generic_no_telephony.mk build/target/product

	create_patch_for_init.rc $temp_file ${TARGET_NAME}
	check_apply_patch_for_init.rc $temp_file

##
# echo exit debug
# exit 1
	build_sdcard_image ${JOB_NUM} ${TARGET_NAME} ${build_option}

}

# Check that we have the expected args
if [ $# -lt 5 ]; then
	usage
	exit 1
fi

BUILD_DIR=${1}
JOB_NUM=${2}
GIT_TAG=${3}
BRANCH_NAME=${4}
TARGET_NAME=${5}
BUILD_OPTION=${6}

main ${BUILD_DIR} ${JOB_NUM} ${GIT_TAG} ${BRANCH_NAME} ${TARGET_NAME} ${BUILD_OPTION}
