#!/usr/bin/env bash

PASSCODE=${1:-20202021}
DISCRIMINATOR=${2:-42}
UDP_PORT=${3:-5560}

pkill chip-ota-provider-app
pkill chip-ota-requestor-app

scripts/examples/gn_build_example.sh examples/chip-tool out/

touch my-firmware.bin

rm /tmp/chip_*

./src/app/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 1 -vs "1.0" -da sha256 my-firmware.bin my-firmware.ota

if [ ! -f "my-firmware.ota" ]; then
    exit 1
fi

./out/ota_provider_debug/chip-ota-provider-app -f my-firmware.ota | tee /tmp/ota/provider-log.txt &
provider_pid=$!

echo "Commissioning Provider"

./out/chip-tool pairing onnetwork 1 "$PASSCODE" | tee /tmp/ota/chip-tool-commission-provider.txt
if grep "Device commissioning completed with success" /tmp/ota/chip-tool-commission-provider.txt; then
    echo Provider Commissioned
else
    echo Provider not commissioned properly
fi

./out/chip-tool accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0

stdbuf -o0 ./out/ota_requestor_debug/chip-ota-requestor-app --discriminator "$DISCRIMINATOR" --secured-device-port "$UDP_PORT" --KVS /tmp/chip_kvs_requestor | tee /tmp/ota/requestor-log.txt &
requestor_pid=$!

echo "Commissioning Requestor"

./out/chip-tool pairing onnetwork-long 2 "$PASSCODE" "$DISCRIMINATOR" | tee /tmp/ota/chip-tool-commission-requestor.txt

if grep "Device commissioning completed with success" /tmp/ota/chip-tool-commission-requestor.txt; then
    echo Requestor Commissioned
else
    echo Requestor not commissioned properly
fi

echo "Sending announce-ota-provider"

./out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0 | tee /tmp/ota/chip-tool-announce-ota.txt

# timeout 30 grep -q "OTA image downloaded to" <(tail -n0 -f /tmp/ota/requestor-log.txt)
sleep 20
echo "Exiting, logs are in tmp/ota/"

kill "$provider_pid"
kill "$requestor_pid"

if grep "OTA image downloaded to" /tmp/ota/requestor-log.txt; then
    echo Test passed && exit 0
else
    echo Test failed && exit 1
fi
