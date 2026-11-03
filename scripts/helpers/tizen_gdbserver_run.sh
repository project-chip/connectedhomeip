#!/bin/bash

set -e

GDBSERVER_DEFAULT_PORT=9999
USAGE_INFO_MSG="See: $0 --help"

function help() {
    cat <<EOF
Usage:
$0 --app-name APP_NAME [--help] [--gdbserver-port PORT] [--target SDB_ID] [-- APP_ARGUMENTS]

Options:
    --app-name APP_NAME   - name of app to debug
    --gdbserver-port PORT - gdbserver port, defaults to $GDBSERVER_DEFAULT_PORT
    --target SDB_ID       - SDB identifier, if not specified first connected will be used
    --help                - print help
    APP_ARGUMENTS         - arguments to pass to debugged app

Example:
$0 \\
  --target 192.168.0.118:26101 \\
  --gdbserver-port 9999 \\
  --app-name "org.tizen.matter.example.lighting" \\
  -- \\
  wifi true discriminator 1234 passcode 11223344
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

SDB_CMD=("sdb")
if [ -n "$TARGET_DEVICE" ]; then
    SDB_CMD=("sdb" "-s" "$TARGET_DEVICE")
fi

CNX_STATUS=$("${SDB_CMD[@]}" root on 2>&1 || true)

# --- GDBSERVER DETECTION ---
GDBSERVER_TARGET_PATH="/opt/bin/gdbserver"
KNOWN_TARGET_PATHS=(
    "/opt/bin/gdbserver"
    "/opt/usr/home/owner/share/tmp/sdk_tools/gdbserver/gdbserver"
    "/usr/bin/gdbserver"
)

GDBSERVER_FOUND_ON_DEVICE=false

# 1. Check strict known paths first
for path in "${KNOWN_TARGET_PATHS[@]}"; do
    IS_EXECUTABLE=$("${SDB_CMD[@]}" shell "test -x $path && echo 'OK'" | tr -d '\r' | tr -d ' ')
    if [ "$IS_EXECUTABLE" = "OK" ]; then
        GDBSERVER_TARGET_PATH="$path"
        GDBSERVER_FOUND_ON_DEVICE=true
        echo "Found active gdbserver on device at: $GDBSERVER_TARGET_PATH"
        break
    fi
done

# 2. Fallback: Deploy only if missing from the target
if [ "$GDBSERVER_FOUND_ON_DEVICE" = false ]; then
    echo "gdbserver missing from device. Searching local SDK..."

    if [ -z "$TIZEN_SDK_ROOT" ]; then
        echo "ERROR: TIZEN_SDK_ROOT environment variable is not set."
        exit 1
    fi

    REMOTE_ARCH=$("${SDB_CMD[@]}" shell "uname -m" | tr -d '\r' | tr -d ' ')
    echo "Detected target architecture: $REMOTE_ARCH"

    LOCAL_ARCH_PATTERN=""
    case "$REMOTE_ARCH" in
        aarch64) LOCAL_ARCH_PATTERN="aarch64-linux-gnu" ;;
        arm* | armel) LOCAL_ARCH_PATTERN="arm-linux-gnueabi" ;;
        x86_64) LOCAL_ARCH_PATTERN="x86_64-linux-gnu" ;;
        *)
            echo "ERROR: Unsupported remote architecture: $REMOTE_ARCH"
            exit 1
            ;;
    esac

    LOCAL_GDBSERVER=$(find "$TIZEN_SDK_ROOT" -type f -path "*/bin/*gdbserver*" -name "*$LOCAL_ARCH_PATTERN*" 2>/dev/null | head -n 1)

    if [ -z "$LOCAL_GDBSERVER" ] || [ ! -f "$LOCAL_GDBSERVER" ]; then
        echo "ERROR: Could not find a valid gdbserver binary for $REMOTE_ARCH inside $TIZEN_SDK_ROOT."
        exit 1
    fi

    GDBSERVER_TARGET_PATH="/opt/bin/gdbserver"
    echo "Deploying local SDK binary to target destination: $GDBSERVER_TARGET_PATH..."

    "${SDB_CMD[@]}" shell "mkdir -p /opt/bin"
    "${SDB_CMD[@]}" push "$LOCAL_GDBSERVER" "$GDBSERVER_TARGET_PATH"
    "${SDB_CMD[@]}" shell "chmod +x $GDBSERVER_TARGET_PATH"
    echo "gdbserver successfully deployed."
fi
# ----------------------------------------

# Launch target application in suspended mode
echo "Launching $APP_NAME in suspended debug mode..."
LAUNCH_CMD="app_launcher --debug --start $APP_NAME"
if [ -n "$APP_ARGS" ]; then
    LAUNCH_CMD="$LAUNCH_CMD -- $APP_ARGS"
fi
LAUNCH_OUT=$("${SDB_CMD[@]}" shell "$LAUNCH_CMD")
echo "$LAUNCH_OUT"

PID=$(echo "$LAUNCH_OUT" | sed -n 's/.*pid = \([0-9][0-9]*\).*/\1/p')

if [ -z "$PID" ]; then
    echo "ERROR: Failed to capture application PID from app_launcher output."
    exit 1
fi

echo "Captured App PID: $PID"

# Signal cleanup layout for exit, interrupt, and termination
cleanup_on_error() {
    local exit_code=$?
    if [ -n "$1" ] || [ "$exit_code" -ne 0 ]; then
        echo "Cleaning up suspended PID: $PID from target device..."

        if [ -n "$SDB_PID" ]; then
            kill "$SDB_PID" >/dev/null 2>&1 || true
        fi

        "${SDB_CMD[@]}" shell "kill -9 $PID 2>/dev/null; killall -9 gdbserver 2>/dev/null" >/dev/null 2>&1 || true
        if [ "$1" = "INT" ]; then exit 130; fi
    fi
}

trap 'cleanup_on_error' EXIT
trap 'cleanup_on_error INT' INT
trap 'cleanup_on_error TERM' TERM

echo "Setting up SDB port forward (TCP $GDBSERVER_PORT)..."
"${SDB_CMD[@]}" forward tcp:"$GDBSERVER_PORT" tcp:"$GDBSERVER_PORT"

# Run gdbserver in foreground and capture background job descriptor for trap handling
echo "Attaching gdbserver to PID $PID on port $GDBSERVER_PORT..."
"${SDB_CMD[@]}" shell "$GDBSERVER_TARGET_PATH --once --attach :$GDBSERVER_PORT $PID" &
SDB_PID=$!

set +e
wait "$SDB_PID"
set -e
