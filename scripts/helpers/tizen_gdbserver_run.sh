#!/bin/bash

GDBSERVER_DEFAULT_PORT=9999
RESULT_MODE="debug"

function help() {

    cat <<EOF
Usage:
$0 --app-name APP_NAME [--gdbserver-port PORT] [--target TARGET_DEVICE] [-- APP_ARGUMENTS]

Options:
    --app-name APP_NAME - name of app to debug
    --gdbserver-port PORT - gdbserver port, if not specified, defaults to $GDBSERVER_DEFAULT_PORT
    --target DEVICE - device to debug app, if not specified and one device are conneted it will be used
    APP_ARGUMENTS - arguments to pass to debugged app
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
            echo "Unknown option $1"
            help
            exit 1
            ;;
    esac
done

if [ -z "$APP_NAME" ]; then
    echo "Missing app name"
    help
    exit 1
fi

if [ -z "$GDBSERVER_PORT" ]; then
    GDBSERVER_PORT=$GDBSERVER_DEFAULT_PORT
fi

if [ "$(sdb devices | wc -l)" -le 1 ]; then
    echo "No device connected"
    exit 1
fi

if [ -n "$TARGET_DEVICE" ]; then
    TARGET_FLAG="-s $TARGET_DEVICE"
fi

echo sdb "$TARGET_FLAG" shell "/usr/bin/app_launcher -s \"$APP_NAME\"  \
    __AUL_SDK__ $RESULT_MODE \
    __DLP_DEBUG_ARG__ :$GDBSERVER_PORT \
    __DLP_GDBSERVER_PATH__ $SDK_TOOLS_PATH/gdbserver/gdbserver \
    $APP_ARGS \
    "
