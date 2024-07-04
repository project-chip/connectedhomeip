#!/bin/bash

# Check if --verbose option is specified
VERBOSE=false
for arg in "$@"; do
    if [ "$arg" == "--verbose" ]; then
        VERBOSE=true
        break
    fi
done

# Kill fabric-admin if it is running
fabric_admin_pid=$(pgrep -f fabric-admin)
if [ ! -z "$fabric_admin_pid" ]; then
    kill -9 "$fabric_admin_pid"
    echo "Killed fabric-admin with PID $fabric_admin_pid"
fi

# Remove /tmp/chip_* files and directories
sudo rm -rf /tmp/chip_*
echo "Removed /tmp/chip_* files and directories"

# Start fabric-admin with or without log file path based on --verbose option
if [ "$VERBOSE" = true ]; then
    out/debug/standalone/fabric-admin
else
    out/debug/standalone/fabric-admin --log-file-path /tmp/fabric_admin.log
fi
