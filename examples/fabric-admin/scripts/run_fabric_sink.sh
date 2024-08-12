#!/bin/bash

# Default paths
DEFAULT_CHOICES=(
    "./fabric-admin"
    "out/debug/standalone/fabric-admin"
    "out/linux-x64-fabric-admin/fabric-admin"
    "out/darwin-arm64-fabric-admin/fabric-admin"
)
FABRIC_ADMIN_LOG="/tmp/fabric_admin.log"
FABRIC_ADMIN_PATH=""

# Function to find fabric-admin binary
find_fabric_admin() {
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
SPECIFIED_PATH=""

for arg in "$@"; do
    case $arg in
        --verbose)
            VERBOSE=true
            ;;
        --path=*)
            SPECIFIED_PATH="${arg#*=}"
            ;;
    esac
done

# Use specified path if provided
if [[ -n "$SPECIFIED_PATH" ]]; then
    if [[ -e "$SPECIFIED_PATH" ]]; then
        FABRIC_ADMIN_PATH="$SPECIFIED_PATH"
    else
        echo >&2 "Specified path does not exist: $SPECIFIED_PATH"
        exit 1
    fi
else
    FABRIC_ADMIN_PATH=$(find_fabric_admin "${DEFAULT_CHOICES[@]}")
    if [[ $? -ne 0 ]]; then
        echo >&2 "Could not find the fabric-admin binary"
        exit 1
    fi
fi

echo "PATH IS: $FABRIC_ADMIN_PATH"

# Kill fabric-admin if it is running
echo "Checking for running fabric-admin process..."
fabric_admin_pid=$(pgrep -f "$FABRIC_ADMIN_PATH")
if [[ -n "$fabric_admin_pid" ]]; then
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
