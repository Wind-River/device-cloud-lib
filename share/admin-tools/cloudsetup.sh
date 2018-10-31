#!/bin/sh

export HDCADDRESS=engr-api.devicewise.com
export HDCUSERNAME=maya.nahas@telit.com
export HDCPASSWORD=<change cloudsetup.sh file to have your password>
export HDCORG=HDC_TEST
#export HDCDMTOKEN=
#export HDCDEBUG=1

python3 $1
