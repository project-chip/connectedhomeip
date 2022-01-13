
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

./out/debug/chip-ota-provider-app -f tmp/ota.txt > tmp/provider-log.txt  &
provider_pid=$!

echo  "Commissioning Provider "

./out/chip-tool pairing onnetwork 1 "$ARG1" > tmp/chip-tool-1.txt 

stdbuf -o0 ./out/debug/chip-ota-requestor-app -u "$ARG3" -d "$ARG2" > tmp/requestor-log.txt & 
requestor_pid=$!

echo  "Commissioning Requestor "

./out/chip-tool pairing onnetwork-long 2 "$ARG1" "$ARG2"  > tmp/chip-tool-2.txt 

echo  "Sending announce-ota-provider "

./out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0  > tmp/chip-tool-3.txt 

echo  "Sleeping for 20 seconds "
sleep 20

echo "Exiting, logs are in tmp/"

kill $provider_pid
kill $requestor_pid

if grep "OTA image downloaded to test.txt" tmp/requestor-log.txt;
then echo Test passed && exit 0;
else echo Test failed && exit 1;
fi
