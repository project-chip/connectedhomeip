#!/bin/bash

if [[ ! -d $K32W061_SDK_ROOT ]]; then
    echo "K32W061_SDK_ROOT is not set"
    exit 1
fi

cp ./third_party/k32w_sdk/sdk_fixes/controller_interface.h "$K32W061_SDK_ROOT"/middleware/wireless/ble_controller/interface/
cp ./third_party/k32w_sdk/sdk_fixes/Messaging.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/Messaging/Interface/
cp ./third_party/k32w_sdk/sdk_fixes/gpio_pins.h "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm/
cp ./third_party/k32w_sdk/sdk_fixes/pin_mux.c "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/enablement/

echo "K32W SDK Master Release 3 was patched!"
exit 0
