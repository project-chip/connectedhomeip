#!/bin/bash

service dbus start
sleep 1
service avahi-daemon start
sleep infinity
