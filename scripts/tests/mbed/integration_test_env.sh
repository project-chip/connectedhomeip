#!/usr/bin/env bash

#####################################################################
### Default parameters
true ${CHIP_DIR:=CHIP}

#####################################################################
### Set enviroment variables
export AP_SSID=CHIPnet
export AP_PASSWORD=CHIPnet123
export AP_GATEWAY=192.168.4.1
export ECHO_SERVER_PORT=7

####################################################################
### Declare ap_stop function
function env_stop() {
    sudo bash $CHIP_DIR/scripts/tests/mbed/wlan_ap.sh stop
}

#####################################################################
### Declare ap_start function
function env_start() {
    sudo bash $CHIP_DIR/scripts/tests/mbed/wlan_ap.sh start --ap_gateway $AP_GATEWAY --ap_ssid $AP_SSID --ap_pswd $AP_PASSWORD
    sudo bash $CHIP_DIR/scripts/tests/mbed/echo_server.sh

    # Build CHIP main
    ./$CHIP_DIR/scripts/build/default.sh
    export CHIP_TOOLS_DIR=$CHIP_DIR/out/default

    # Install Python Chip Device Controller
    pip install $CHIP_TOOLS_DIR/controller/python/chip*.whl
    pip install -r $CHIP_DIR/src/test_driver/mbed-functional/requirements.txt

    source $CHIP_DIR/scripts/tests/mbed/common.sh
    mount_mbed_device
}

#####################################################################
### Check if using supported commands [start/stop]

SUPPORTED_COMMAND=(start stop)
COMMAND=$1

if [[ ! " ${SUPPORTED_COMMAND[@]} " =~ " ${COMMAND} " ]]; then
    echo "ERROR: Command $COMMAND not supported"
    exit 1
fi

#####################################################################
### Handle start command

if [[ "$COMMAND" == *"start"* ]]; then
    ### Parse start command arguments
    # Remove $1 from argument list
    shift

    for i in "$@"; do
        case $i in
        --chip_dir=*)
            CHIP_DIR="${i#*=}"
            shift
            ;;
        *)
            # unknown option
            ;;
        esac
    done
    env_start
fi

#####################################################################
### Handle stop command

if [[ "$COMMAND" == *"stop"* ]]; then
    env_stop
fi
