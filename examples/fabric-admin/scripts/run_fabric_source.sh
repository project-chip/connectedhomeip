#!/bin/bash

# Default paths
FABRIC_ADMIN_PATH="./fabric-admin"
FABRIC_BRIDGE_APP_PATH="./fabric-bridge-app"
FABRIC_ADMIN_LOG="/tmp/fabric_admin.log"
FABRIC_BRIDGE_APP_LOG="/tmp/fabric_bridge_app.log"

# Detect platform and set default paths
PLATFORM=$(uname -m)
if [[ "$PLATFORM" == "x86_64" ]]; then
    FABRIC_ADMIN_PATH="out/debug/standalone/fabric-admin"
    FABRIC_BRIDGE_APP_PATH="out/debug/standalone/fabric-bridge-app"
fi

# Check if --verbose option is specified
VERBOSE=false
for arg in "$@"; do
    if [ "$arg" == "--verbose" ]; then
        VERBOSE=true
        break
    fi
done

# Stop any running instances and clean up

if [[ "$PLATFORM" == "x86_64" ]]; then
    ./examples/fabric-admin/scripts/stop_fabric_source.sh
elif [[ "$PLATFORM" == "aarch64" ]]; then
    ./stop_fabric_source.sh
fi

# Start fabric-bridge-app if available and redirect its output to /dev/null
if [ -f "$FABRIC_BRIDGE_APP_PATH" ]; then
    "$FABRIC_BRIDGE_APP_PATH" >"$FABRIC_BRIDGE_APP_LOG" 2>&1 &
    echo "Started fabric-bridge-app"
fi

# Start fabric-admin with or without log file path based on --verbose option
if [ "$VERBOSE" = true ]; then
    "$FABRIC_ADMIN_PATH"
else
    "$FABRIC_ADMIN_PATH" --log-file-path "$FABRIC_ADMIN_LOG"
fi
