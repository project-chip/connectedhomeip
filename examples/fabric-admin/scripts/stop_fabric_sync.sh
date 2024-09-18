#!/bin/bash

FABRIC_ADMIN_PATH="fabric-admin"
FABRIC_BRIDGE_APP_PATH="fabric-bridge"

# Kill fabric-admin if it is running
fabric_admin_pid=$(pgrep "$FABRIC_ADMIN_PATH")
echo "Found fabric-admin PID: $fabric_admin_pid"
if [ ! -z "$fabric_admin_pid" ]; then
    kill -9 "$fabric_admin_pid"
    echo "Killed fabric-admin with PID $fabric_admin_pid"
fi

# Kill fabric-bridge-app if it is running
fabric_bridge_app_pid=$(pgrep "$FABRIC_BRIDGE_APP_PATH")
echo "Found fabric-bridge-app PID: $fabric_bridge_app_pid"
if [ ! -z "$fabric_bridge_app_pid" ]; then
    kill -9 "$fabric_bridge_app_pid"
    echo "Killed fabric-bridge-app with PID $fabric_bridge_app_pid"
fi

# Remove /tmp/chip_* files and directories
rm -rf /tmp/chip_*
echo "Removed /tmp/chip_* files and directories."
rm /tmp/fabric_admin.log
rm /tmp/fabric_bridge_app.log
echo "Removed fabric sync log files."
