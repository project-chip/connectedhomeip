#!/bin/bash

SDB="$TIZEN_SDK_ROOT/tools/sdb"
PORT=9999
RESULT_MODE="debug"

function help() {
    cat <<EOF
./$(basename "$0") --app-name=APP_NAME -- [APP_ARGUMENTS]
EOF
    exit
}

POSITIONAL_ARGS=()
APP_ARGS=""
while [[ $# -gt 0 ]]; do
    case $1 in
        --app-name)
            APP_NAME="$2"
            shift
            shift
            ;;
        --help)
            help
            shift
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
    help
fi

if [ "$("$SDB" devices | wc -l)" -le 1 ]; then
    echo "No device connected"
    exit 1
fi

"$SDB" shell "/usr/bin/app_launcher -s \"$APP_NAME\" \
    __AUL_SDK__ $RESULT_MODE \
    __DLP_DEBUG_ARG__ :$PORT \
    __DLP_GDBSERVER_PATH__ $SDK_TOOLS_PATH/gdbserver/gdbserver \
    $APP_ARGS \
    "
