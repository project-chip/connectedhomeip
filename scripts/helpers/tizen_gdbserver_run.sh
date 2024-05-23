#!/bin/bash

set -e

GDBSERVER_DEFAULT_PORT=9999
GDBSERVER_TARGET_PATH="/opt/usr/home/owner/share/tmp/sdk_tools/gdbserver"
RESULT_MODE="debug"
USAGE_INFO_MSG="See: $0 --help"

function help() {

    cat <<EOF
Usage:
$0 --app-name APP_NAME [--help] [--gdbserver-port PORT] [--target TARGET_DEVICE] [-- APP_ARGUMENTS]

Options:
    --app-name APP_NAME - name of app to debug
    --gdbserver-port PORT - gdbserver port, if not specified, defaults to $GDBSERVER_DEFAULT_PORT
    --target DEVICE - device to debug the app, if not specified and one device is connected, it will be used
    --help - print help
    APP_ARGUMENTS - arguments to pass to debugged app

Requirements:
    gdbserver has to be installed on the target device in the $GDBSERVER_TARGET_PATH path.
    If it is installed on the device by RPM it is necessary to create a link by:
    'mkdir -p $GDBSERVER_TARGET_PATH && ln -sf /usr/bin/gdbserver $GDBSERVER_TARGET_PATH/gdbserver'.
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

SDB_DEVICES=$(sdb devices | grep -cv 'List of devices attached')

if [ "$SDB_DEVICES" -eq 0 ]; then
    echo "ERROR: No device connected"
    exit 1
elif [ "$SDB_DEVICES" -gt 1 ]; then
    if [ -z "$TARGET_DEVICE" ]; then
        echo "ERROR: More than one device connected, please specify target device."
        echo "$USAGE_INFO_MSG"
        exit 1
    fi
fi

if [ -z "$TARGET_DEVICE" ]; then
    sdb -s "$TARGET_DEVICE" root on
    GDBSERVER_FOUND=$(sdb shell "[ -e \"$GDBSERVER_TARGET_PATH/gdbserver\" ] || echo -n \"gdb not found\"")
else
    sdb -s "$TARGET_DEVICE" root on
    GDBSERVER_FOUND=$(sdb -s "$TARGET_DEVICE" shell "[ -e \"$GDBSERVER_TARGET_PATH/gdbserver\" ] || echo -n \"gdb not found\"")
fi

if [ "$GDBSERVER_FOUND" == "gdb not found" ]; then
    echo "ERROR: Gdbserver not found on the target device in path $GDBSERVER_TARGET_PATH"
    echo "$USAGE_INFO_MSG"
    exit 1
fi

CMD="/usr/bin/app_launcher -s \"$APP_NAME\"  \
__AUL_SDK__ $RESULT_MODE \
__DLP_DEBUG_ARG__ :$GDBSERVER_PORT \
$APP_ARGS \
"

if [ -z "$TARGET_DEVICE" ]; then
    sdb shell "$CMD"
    sdb forward tcp:"$GDBSERVER_PORT" tcp:"$GDBSERVER_PORT"
else
    sdb -s "$TARGET_DEVICE" shell "$CMD"
    sdb -s "$TARGET_DEVICE" forward tcp:"$GDBSERVER_PORT" tcp:"$GDBSERVER_PORT"
fi
