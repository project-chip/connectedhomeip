#!/usr/bin/env bash

set -e

if [ $# -lt 1 ]; then
    echo "Illegal number of parameters. Please define action (start/stop)"
    exit 1
fi

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

    if [ $# -lt 2 ]; then
        echo "Illegal number of parameters for start command. Please define CHIP source directory"
        exit 1
    fi

    CHIP_DIR=$2

    bash $CHIP_DIR/scripts/tests/mbed/wlan_ap.sh start --ap_gateway $AP_GATEWAY --ap_ssid $AP_SSID --ap_pswd $AP_PASSWORD
    bash $CHIP_DIR/scripts/tests/mbed/echo_server.sh

    pip install mbed-flasher pytest

    source $CHIP_DIR/scripts/tests/mbed/common.sh
    mount_mbed_device
    mbedls -j >devices.json
fi

#####################################################################
### Handle stop command

if [[ "$COMMAND" == *"stop"* ]]; then
    if [ $# -lt 2 ]; then
        echo "Illegal number of parameters for stop command. Please define CHIP source directory"
        exit 1
    fi

    bash $2/scripts/tests/mbed/wlan_ap.sh stop
fi
