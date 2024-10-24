#!/bin/bash

# Default paths
DEFAULT_SYNC_CHOICES=(
    "./fabric-sync"
    "out/debug/standalone/fabric-sync"
    "out/linux-x64-fabric-sync/fabric-sync"
)

FABRIC_SYNC_PATH=""

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
SPECIFIED_SYNC_PATH=""

for arg in "$@"; do
    case $arg in
        --sync-path=*)
            SPECIFIED_SYNC_PATH="${arg#*=}"
            ;;
    esac
done

# Use specified paths if provided
if [[ -n "$SPECIFIED_SYNC_PATH" ]]; then
    if [[ -e "$SPECIFIED_SYNC_PATH" ]]; then
        FABRIC_SYNC_PATH="$SPECIFIED_SYNC_PATH"
    else
        echo >&2 "Specified fabric sync path does not exist: $SPECIFIED_SYNC_PATH"
        exit 1
    fi
else
    FABRIC_SYNC_PATH=$(find_binary "${DEFAULT_SYNC_CHOICES[@]}")
    if [[ $? -ne 0 ]]; then
        echo >&2 "Could not find the fabric-sync binary"
        exit 1
    fi
fi

echo "Fabric Sync path: $FABRIC_SYNC_PATH"

# Remove /tmp/chip_* files and directories
rm -rf /tmp/chip_*
echo "Removed /tmp/chip_* files and directories."

# Start fabric-sync
"$FABRIC_SYNC_PATH"
