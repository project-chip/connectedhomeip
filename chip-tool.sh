#!/bin/bash

if [[ "$1" == "ble" ]]; then
        dataset=`cat openthread.dat`
        nodeid=$((1 + $RANDOM % 10000))
        discriminator=${2:-3840}
        echo "NODE: $nodeid, DISC: $discriminator"
        ./out/chip-tool/chip-tool pairing ble-thread $nodeid hex:$dataset 1 73141520 $discriminator
elif [[ "$1" == "oc" ]]; then
        dataset=`cat openthread.dat`
        nodeid=$2
        discriminator=${3:-3840}
        echo "NODE: $nodeid, DISC: $discriminator"
        ./out/chip-tool/chip-tool pairing open-commissioning-window $nodeid 0 100 1000 $discriminator
elif [[ "$1" == "on" ]]; then
        ./out/chip-tool/chip-tool onoff on $2 1
elif [[ "$1" == "off" ]]; then
        ./out/chip-tool/chip-tool onoff off $2 1
elif [[ "$1" == "toggle" ]]; then
        ./out/chip-tool/chip-tool onoff toggle $2 1
fi
