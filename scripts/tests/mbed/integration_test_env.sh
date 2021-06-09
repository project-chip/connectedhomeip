#!/usr/bin/env bash

set -e

if [ $# -lt 1 ]; then
    echo "Illegal number of parameters. Please define action (start/stop)"
    exit 1
fi

#####################################################################
### Default parameters
true ${CHIP_DIR:=CHIP}

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

    if [ $# -ge 2 ]; then
        CHIP_DIR=$2
    fi

    bash $CHIP_DIR/scripts/tests/mbed/wlan_ap.sh start --ap_gateway $AP_GATEWAY --ap_ssid $AP_SSID --ap_pswd $AP_PASSWORD
    bash $CHIP_DIR/scripts/tests/mbed/echo_server.sh

    # Build CHIP main
    pwd=$PWD
    cd $CHIP_DIR
    bash scripts/build/default.sh
    cd $pwd

    # Install Python Chip Device Controller
    pip install $CHIP_DIR/out/default/controller/python/chip*.whl
    pip install -r $CHIP_DIR/src/test_driver/mbed-functional/requirements.txt

    source $CHIP_DIR/scripts/tests/mbed/common.sh
    mount_mbed_device
fi

#####################################################################
### Handle stop command

if [[ "$COMMAND" == *"stop"* ]]; then
    bash $CHIP_DIR/scripts/tests/mbed/wlan_ap.sh stop
fi
