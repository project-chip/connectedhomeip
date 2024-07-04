#!/bin/bash

# Kill fabric-admin if it is running
fabric_admin_pid=$(pgrep -f fabric-admin)
if [ ! -z "$fabric_admin_pid" ]; then
    kill -9 "$fabric_admin_pid"
    echo "Killed fabric-admin with PID $fabric_admin_pid"
fi

# Kill fabric-bridge-app if it is running
fabric_bridge_app_pid=$(pgrep -f fabric-bridge-app)
if [ ! -z "$fabric_bridge_app_pid" ]; then
    kill -9 "$fabric_bridge_app_pid"
    echo "Killed fabric-bridge-app with PID $fabric_bridge_app_pid"
fi

# Remove /tmp/chip_* files and directories
sudo rm -rf /tmp/chip_*
echo "Removed /tmp/chip_* files and directories"

# Start fabric-bridge-app and redirect its output to /dev/null
out/debug/standalone/fabric-bridge-app >/dev/null 2>&1 &
echo "Started fabric-bridge-app"

# Start fabric-admin
out/debug/standalone/fabric-admin
