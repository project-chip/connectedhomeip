#!/usr/bin/env bash

set -e

if [ $# -lt 1 ]; then
    echo "Illegal number of parameters. Please define CHIP directory path"
    exit 1
fi

CHIP_DIR=$1

pip install mbed-flasher pytest
source $CHIP_DIR/scripts/tests/mbed/common.sh
mount_mbed_device
mbedls -j >devices.json
