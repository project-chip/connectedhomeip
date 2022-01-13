#!/bin/bash

rm -r /tmp/chip_*

ARG1=${1:-20202021}
ARG2=${2:-42}
ARG3=${3:-5560}

CHIP_ROOT="../.."

pkill chip-ota-provider-app
pkill chip-ota-requestor-app 

scripts/examples/gn_build_example.sh examples/chip-tool out/
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux out/debug chip_config_network_layer_ble=false
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux out/debug chip_config_network_layer_ble=false

$CHIP_ROOT/scripts/examples/gn_build_example.sh $CHIP_ROOT/examples/chip-tool out/
$CHIP_ROOT/scripts/examples/gn_build_example.sh $CHIP_ROOT/examples/ota-provider-app/linux $CHIP_ROOT/out/debug chip_config_network_layer_ble=false
$CHIP_ROOT/scripts/examples/gn_build_example.sh $CHIP_ROOT/examples/ota-requestor-app/linux $CHIP_ROOT/out/debug chip_config_network_layer_ble=false

./$CHIP_ROOT/out/debug/chip-ota-provider-app -f $CHIP_ROOT/tmp/ota.txt > $CHIP_ROOT/tmp/provider-log.txt  &
provider_pid=$!

echo  "Commissioning Provider "

./$CHIP_ROOT/out/chip-tool pairing onnetwork 1 "$ARG1" > $CHIP_ROOT/tmp/chip-tool-1.txt 

stdbuf -o0 ./$CHIP_ROOT/out/debug/chip-ota-requestor-app -u "$ARG3" -d "$ARG2" > $CHIP_ROOT/tmp/requestor-log.txt & 
requestor_pid=$!

echo  "Commissioning Requestor "

./$CHIP_ROOT/out/chip-tool pairing onnetwork-long 2 "$ARG1" "$ARG2"  > $CHIP_ROOT/tmp/chip-tool-2.txt 

echo  "Sending announce-ota-provider "

./$CHIP_ROOT/out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0  > $CHIP_ROOT/tmp/chip-tool-3.txt 

echo  "Sleeping for 20 seconds "

sleep 10

echo "Exiting, logs are in $CHIP_ROOT/tmp/"

if grep "OTA image downloaded to test.txt" $CHIP_ROOT/tmp/requestor-log.txt; then echo Test passed; else echo Test failed; fi

kill $provider_pid
kill $requestor_pid


