#!/usr/bin/env bash

PASSCODE=${1:-20202021}
DISCRIMINATOR=${2:-42}
UDP_PORT=${3:-5560}
OTA_DOWNLOAD_PATH=${4:-"/tmp/test.bin"}

FIRMWARE_BIN="my-firmware.bin"
FIRMWARE_OTA="my-firmware.ota"

OTA_PROVIDER_APP="chip-ota-provider-app"
OTA_PROVIDER_FOLDER="out/ota_provider_debug"
OTA_REQUESTOR_APP="chip-ota-requestor-app"
OTA_REQUESTOR_FOLDER="out/ota_requestor_debug"
CHIP_TOOL_APP="chip-tool"
CHIP_TOOL_FOLDER="out"

killall -e "$OTA_PROVIDER_APP" "$OTA_REQUESTOR_APP"
rm -f "$FIRMWARE_OTA" "$FIRMWARE_BIN" "$OTA_DOWNLOAD_PATH"

set -e

scripts/examples/gn_build_example.sh examples/chip-tool "$CHIP_TOOL_FOLDER"
scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux "$OTA_REQUESTOR_FOLDER" chip_config_network_layer_ble=false
scripts/examples/gn_build_example.sh examples/ota-provider-app/linux "$OTA_PROVIDER_FOLDER" chip_config_network_layer_ble=false

echo "Test" >"$FIRMWARE_BIN"

rm -f /tmp/chip_*

./src/app/ota_image_tool.py create -v 0xDEAD -p 0xBEEF -vn 10 -vs "10.0" -da sha256 "$FIRMWARE_BIN" "$FIRMWARE_OTA"

if [ ! -f "$FIRMWARE_OTA" ]; then
    exit 1
fi

./"$OTA_PROVIDER_FOLDER"/"$OTA_PROVIDER_APP" -f "$FIRMWARE_OTA" | tee /tmp/ota/provider-log.txt &

echo "Commissioning Provider"

./"$CHIP_TOOL_FOLDER"/"$CHIP_TOOL_APP" pairing onnetwork 1 "$PASSCODE" | tee /tmp/ota/chip-tool-commission-provider.txt
if grep "Device commissioning completed with success" /tmp/ota/chip-tool-commission-provider.txt; then
    echo Provider Commissioned
else
    echo Provider not commissioned properly
fi

./"$CHIP_TOOL_FOLDER"/"$CHIP_TOOL_APP" accesscontrol write acl '[{"fabricIndex": 1, "privilege": 5, "authMode": 2, "subjects": [112233], "targets": null}, {"fabricIndex": 1, "privilege": 3, "authMode": 2, "subjects": null, "targets": null}]' 1 0

stdbuf -o0 ./"$OTA_REQUESTOR_FOLDER"/"$OTA_REQUESTOR_APP" --discriminator "$DISCRIMINATOR" --secured-device-port "$UDP_PORT" --KVS /tmp/chip_kvs_requestor --otaDownloadPath "$OTA_DOWNLOAD_PATH" | tee /tmp/ota/requestor-log.txt &

echo "Commissioning Requestor"

./"$CHIP_TOOL_FOLDER"/"$CHIP_TOOL_APP" pairing onnetwork-long 2 "$PASSCODE" "$DISCRIMINATOR" | tee /tmp/ota/chip-tool-commission-requestor.txt

if grep "Device commissioning completed with success" /tmp/ota/chip-tool-commission-requestor.txt; then
    echo Requestor Commissioned
else
    echo Requestor not commissioned properly
fi

echo "Sending announce-ota-provider"

./"$CHIP_TOOL_FOLDER"/"$CHIP_TOOL_APP" otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0 | tee /tmp/ota/chip-tool-announce-ota.txt

timeout 30 grep -q "OTA image downloaded to" <(tail -n0 -f /tmp/ota/requestor-log.txt)

echo "Exiting, logs are in tmp/ota/"

if cmp "$OTA_DOWNLOAD_PATH" "$FIRMWARE_BIN"; then
    TEST_RESULT="Test passed"
    RETURN_VALUE=0
else
    TEST_RESULT="Test failed"
    RETURN_VALUE=1
fi

killall -s SIGKILL -e "$OTA_PROVIDER_APP" "$OTA_REQUESTOR_APP"
rm -f "$FIRMWARE_OTA" "$FIRMWARE_BIN" "$OTA_DOWNLOAD_PATH"

echo "$TEST_RESULT"
exit "$RETURN_VALUE"
