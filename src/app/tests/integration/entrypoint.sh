#!/usr/bin/env bash

service dbus start
systemctl is-active --quiet dbus && echo dbus is running
service avahi-daemon start
/usr/sbin/otbr-agent -I wpan0 spinel+hdlc+uart:///dev/ttyUSB0 &
sleep 1
ot-ctl panid 0x1234
ot-ctl ifconfig up
ot-ctl thread start

if [ "$1" = "responder" ]; then
    chip-im-responder
else
    sleep infinity
fi
