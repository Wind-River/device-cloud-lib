#!/system/bin/sh
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
RUNNING_MODE="PRODUCT"

# wait for iot-device-manager completing telnetd configuration first 
sleep 10

if [ "${RUNNING_MODE}" == "TEST" ]
then
	echo "configuring hdc in automation test mode"

	if [ 0 -eq $( getprop | grep adb | grep -c 5555 ) ]
	then
		echo "enable adb over network "
		setprop service.adb.tcp.port 5555
		stop adbd
		start adbd
	fi

	if [ 0 -eq $( getprop | grep init.svc.console | grep -c running ) ]
	then
		echo "enable console over serial port"
		start console
	fi

	if [ 0 -eq $( getprop | grep init.svc.su_daemon | grep -c running ) ]
	then
		echo "enable su_daemon"
		setprop init.svc.su_daemon running
	fi

	if [ 0 -eq $( getprop | grep persist.sys.root_access | grep -c 3 ) ]
	then
		echo "enable system root access"
		setprop persist.sys.root_access 3
	fi

	which busybox
	if [ 0 -eq $? ]
	then
		if [ 1 -eq $( netstat | grep "23" | grep "127" | grep -c "LISTEN" ) ]
		then
			echo "stop the local port only telnetd if it exist"
			for pid in $( ps | grep busybox | grep -v grep | busybox awk '{print $2}' )
			do
				kill -9 ${pid}
			done
		fi

		if [ 0 -eq $( netstat | grep "23" | grep -c "LISTEN" ) ]
		then
			echo "enable telnetd listening to all ports"
			busybox telnetd -l /system/bin/sh
		fi
	fi
else
	which busybox
	if [ 0 -eq $? ]
	then
		if [ 0 -eq $( netstat | grep "23" | grep -c "LISTEN" ) ]
		then
			echo "enable telnetd listening to localhost telnet port"
			busybox telnetd -l /system/bin/sh -b 127.0.0.1:23
		fi
	fi
fi

if [ 0 -eq $( logcat -g | grep -ci 2Mb ) ]
then
	echo "set logcat buffer to 2MB on the device"
	logcat -G 2M
fi

echo "--- system status ----"

echo "telnet:"
netstat | grep 23 | grep LISTE

echo "logcat:"
logcat -g

echo "adb:"
getprop | grep adb

echo "root access:"
getprop | grep persist.sys.root_access
getprop | grep init.svc.su_daemon

echo "console:"
getprop | grep init.svc.console

echo "configuration for hdc is completed"

exit 0
