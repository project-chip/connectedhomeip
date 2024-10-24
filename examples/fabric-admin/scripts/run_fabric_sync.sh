#!/bin/bash

# Get the path to the current script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Default paths
DEFAULT_ADMIN_CHOICES=(
    "./fabric-admin"
    "out/debug/standalone/fabric-admin"
    "out/linux-x64-fabric-admin-rpc/fabric-admin"
    "out/darwin-arm64-fabric-admin-rpc/fabric-admin"
)
DEFAULT_BRIDGE_CHOICES=(
    "./fabric-bridge-app"
    "out/debug/standalone/fabric-bridge-app"
    "out/linux-x64-fabric-bridge-rpc/fabric-bridge-app"
    "out/linux-x64-fabric-bridge-rpc-no-ble/fabric-bridge-app"
    "out/darwin-arm64-fabric-bridge-rpc/fabric-bridge-app"
    "out/darwin-arm64-fabric-bridge-rpc-no-ble/fabric-bridge-app"
)
FABRIC_ADMIN_LOG="/tmp/fabric_admin.log"
FABRIC_BRIDGE_APP_LOG="/tmp/fabric_bridge_app.log"
FABRIC_ADMIN_PATH=""
FABRIC_BRIDGE_APP_PATH=""

# Function to find a binary
find_binary() {
    local choices=("$@")
    for path in "${choices[@]}"; do
        if [[ -e "$path" ]]; then
            echo "$path"
            return 0
        fi
    done
    return 1
}

# Parse arguments
VERBOSE=false
SPECIFIED_ADMIN_PATH=""
SPECIFIED_BRIDGE_PATH=""

for arg in "$@"; do
    case $arg in
        --verbose)
            VERBOSE=true
            ;;
        --admin-path=*)
            SPECIFIED_ADMIN_PATH="${arg#*=}"
            ;;
        --bridge-path=*)
            SPECIFIED_BRIDGE_PATH="${arg#*=}"
            ;;
    esac
done

# Use specified paths if provided
if [[ -n "$SPECIFIED_ADMIN_PATH" ]]; then
    if [[ -e "$SPECIFIED_ADMIN_PATH" ]]; then
        FABRIC_ADMIN_PATH="$SPECIFIED_ADMIN_PATH"
    else
        echo >&2 "Specified admin path does not exist: $SPECIFIED_ADMIN_PATH"
        exit 1
    fi
else
    FABRIC_ADMIN_PATH=$(find_binary "${DEFAULT_ADMIN_CHOICES[@]}")
    if [[ $? -ne 0 ]]; then
        echo >&2 "Could not find the fabric-admin binary"
        exit 1
    fi
fi

if [[ -n "$SPECIFIED_BRIDGE_PATH" ]]; then
    if [[ -e "$SPECIFIED_BRIDGE_PATH" ]]; then
        FABRIC_BRIDGE_APP_PATH="$SPECIFIED_BRIDGE_PATH"
    else
        echo >&2 "Specified bridge path does not exist: $SPECIFIED_BRIDGE_PATH"
        exit 1
    fi
else
    FABRIC_BRIDGE_APP_PATH=$(find_binary "${DEFAULT_BRIDGE_CHOICES[@]}")
    if [[ $? -ne 0 ]]; then
        echo >&2 "Could not find the fabric-bridge-app binary"
        exit 1
    fi
fi

echo "Admin path: $FABRIC_ADMIN_PATH"
echo "Bridge path: $FABRIC_BRIDGE_APP_PATH"

# Determine the path to stop_fabric_sync.sh based on the location of run_fabric_sync.sh
RUN_FABRIC_SOURCE_PATH=$(find_binary "$SCRIPT_DIR/run_fabric_sync.sh")
if [[ $? -ne 0 ]]; then
    echo >&2 "Could not find the run_fabric_sync.sh script"
    exit 1
fi
STOP_FABRIC_SYNC_PATH="${RUN_FABRIC_SOURCE_PATH/run_fabric_sync/stop_fabric_sync}"

# Stop any running instances and clean up
if [[ -e "$STOP_FABRIC_SYNC_PATH" ]]; then
    "$STOP_FABRIC_SYNC_PATH"
else
    echo >&2 "Could not find the stop_fabric_sync.sh script"
    exit 1
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
