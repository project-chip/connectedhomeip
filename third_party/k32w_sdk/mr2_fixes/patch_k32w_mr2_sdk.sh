#!/bin/bash

if [[ ! -d $K32W061_SDK_ROOT ]]; then
    echo "K32W061_SDK_ROOT is not set"
    exit 1
fi

cp ./third_party/k32w_sdk/mr2_fixes/PWR.c "$K32W061_SDK_ROOT"/middleware/wireless/framework/LowPower/Source/k32w061dk6/
cp ./third_party/k32w_sdk/mr2_fixes/controller_interface.h "$K32W061_SDK_ROOT"/middleware/wireless/ble_controller/interface/
cp ./third_party/k32w_sdk/mr2_fixes/Messaging.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/Messaging/Interface/
cp ./third_party/k32w_sdk/mr2_fixes/ble_conn_manager.h "$K32W061_SDK_ROOT"/middleware/wireless/bluetooth/application/common/
cp ./third_party/k32w_sdk/mr2_fixes/GenericList.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/Lists/

echo "K32W MR2 patched"
exit 0
