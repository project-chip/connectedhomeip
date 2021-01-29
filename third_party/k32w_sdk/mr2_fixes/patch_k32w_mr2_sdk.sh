#!/bin/bash

if [[ ! -v K32W061_SDK_ROOT ]]; then
    echo "K32W061_SDK_ROOT is not set"
    exit 1
fi

cp -r ./third_party/k32w_sdk/mr2_fixes/chip "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/
cp ./third_party/k32w_sdk/mr2_fixes/PWR.c "$K32W061_SDK_ROOT"/middleware/wireless/framework/LowPower/Source/k32w061dk6/
cp ./third_party/k32w_sdk/mr2_fixes/controller_interface.h "$K32W061_SDK_ROOT"/middleware/wireless/ble_controller/interface/
cp ./third_party/k32w_sdk/mr2_fixes/Messaging.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/Messaging/Interface/
cp ./third_party/k32w_sdk/mr2_fixes/ble_conn_manager.h "$K32W061_SDK_ROOT"/middleware/wireless/bluetooth/application/common/
cp ./third_party/k32w_sdk/mr2_fixes/GenericList.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/Lists/
cp ./third_party/k32w_sdk/mr2_fixes/pin_mux.c "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/enablement/

echo "K32W MR2 patched"
exit 0
