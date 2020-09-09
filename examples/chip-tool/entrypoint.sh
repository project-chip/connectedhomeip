#!/bin/bash

service dbus start
sleep 1
/usr/sbin/otbr-agent -I wpan0 spinel+hdlc+uart:///dev/ttyUSB0 &
sleep 1
ot-ctl panid 0x1234
ot-ctl ifconfig up
ot-ctl thread start

if [ "$1" == "server" ]; then
    chip-tool-server
elif [ "$1" == "tool" ]; then
    sleep infinity
fi
