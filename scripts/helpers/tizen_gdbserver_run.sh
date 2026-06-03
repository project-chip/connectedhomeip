#!/bin/bash

set -e

GDBSERVER_DEFAULT_PORT=9999
GDBSERVER_TARGET_PATH="/opt/usr/home/owner/share/tmp/sdk_tools/gdbserver/gdbserver"
USAGE_INFO_MSG="See: $0 --help"

function help() {
    cat <<EOF
Usage:
$0 --app-name APP_NAME [--help] [--gdbserver-port PORT] [--target SDB_ID] [-- APP_ARGUMENTS]

Options:
    --app-name APP_NAME   - name of app to debug
    --gdbserver-port PORT - gdbserver port, if not specified, defaults to $GDBSERVER_DEFAULT_PORT
    --target SDB_ID       - SDB identifier (e.g. 192.168.0.118:26101), if not specified first connected will be used
    --help                - print help
    APP_ARGUMENTS         - arguments to pass to debugged app
EOF
}

APP_ARGS=""
while [[ $# -gt 0 ]]; do
    case $1 in
        --app-name)
            APP_NAME="$2"
            shift 2
            ;;
        --gdbserver-port)
            GDBSERVER_PORT="$2"
            shift 2
            ;;
        --target)
            TARGET_DEVICE="$2"
            shift 2
            ;;
        --help)
            help
            exit 0
            ;;
        --)
            shift
            APP_ARGS=$@
            break
            ;;
        *)
            echo "ERROR: Unknown option $1"
            echo "$USAGE_INFO_MSG"
            exit 1
            ;;
    esac
done

if [ -z "$APP_NAME" ]; then
    echo "ERROR: Missing app name"
    echo "$USAGE_INFO_MSG"
    exit 1
fi

if [ -z "$GDBSERVER_PORT" ]; then
    GDBSERVER_PORT=$GDBSERVER_DEFAULT_PORT
fi

# Select SDB target flags correctly (requires exact SDB ID like IP:Port)
SDB_CMD="sdb"
if [ -n "$TARGET_DEVICE" ]; then
    SDB_CMD="sdb -s $TARGET_DEVICE"
fi

$SDB_CMD root on

# 1. Launch the application in native Tizen suspension debug mode
echo "Launching $APP_NAME in suspended debug mode..."
LAUNCH_CMD="app_launcher --debug --start $APP_NAME"
if [ -n "$APP_ARGS" ]; then
    LAUNCH_CMD="$LAUNCH_CMD -- $APP_ARGS"
fi
LAUNCH_OUT=$($SDB_CMD shell "$LAUNCH_CMD")
echo "$LAUNCH_OUT"

# 2. Parse the PID from the app_launcher output
PID=$(echo "$LAUNCH_OUT" | sed -n 's/.*pid = \([0-9][0-9]*\).*/\1/p')

if [ -z "$PID" ]; then
    echo "ERROR: Failed to capture application PID from app_launcher output."
    exit 1
fi

echo "Captured App PID: $PID"

# 3. Setup port forwarding from host to target device
echo "Setting up SDB port forward (TCP $GDBSERVER_PORT)..."
$SDB_CMD forward tcp:"$GDBSERVER_PORT" tcp:"$GDBSERVER_PORT"

# 4. Attach gdbserver manually to the frozen process
echo "Attaching gdbserver to PID $PID on port $GDBSERVER_PORT..."
$SDB_CMD shell "$GDBSERVER_TARGET_PATH :$GDBSERVER_PORT --attach $PID"
