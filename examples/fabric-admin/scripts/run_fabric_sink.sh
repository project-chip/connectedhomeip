#!/bin/bash

# Default paths
FABRIC_ADMIN_PATH="./fabric-admin"
FABRIC_ADMIN_LOG="/tmp/fabric_admin.log"

# Detect platform and set default paths
if [[ $(uname -m) == "x86_64" ]]; then
    FABRIC_ADMIN_PATH="out/debug/standalone/fabric-admin"
fi

# Check if --verbose option is specified
VERBOSE=false
for arg in "$@"; do
    if [ "$arg" == "--verbose" ]; then
        VERBOSE=true
        break
    fi
done

# Kill fabric-admin if it is running
echo "Checking for running fabric-admin process..."
fabric_admin_pid=$(pgrep -f "$FABRIC_ADMIN_PATH")
if [ ! -z "$fabric_admin_pid" ]; then
    echo "Found fabric-admin with PID $fabric_admin_pid, attempting to kill..."
    kill -9 "$fabric_admin_pid"
    echo "Killed fabric-admin with PID $fabric_admin_pid"
fi

# Remove /tmp/chip_* files and directories
echo "Removing /tmp/chip_* files and directories..."
sudo rm -rf /tmp/chip_*
echo "Removed /tmp/chip_* files and directories"

# Start fabric-admin with or without log file path based on --verbose option
echo "Starting fabric-admin..."
if [ "$VERBOSE" = true ]; then
    "$FABRIC_ADMIN_PATH"
else
    "$FABRIC_ADMIN_PATH" --log-file-path "$FABRIC_ADMIN_LOG"
fi
