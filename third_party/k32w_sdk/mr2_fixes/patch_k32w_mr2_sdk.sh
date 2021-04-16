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
cp ./third_party/k32w_sdk/mr2_fixes/fsl_debug_console.c "$K32W061_SDK_ROOT"/devices/K32W061/utilities/debug_console/
cp ./third_party/k32w_sdk/mr2_fixes/RNG_Interface.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/RNG/Interface/
cp ./third_party/k32w_sdk/mr2_fixes/gpio_pins.h "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm/
cp ./third_party/k32w_sdk/mr2_fixes/pin_mux.c "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/enablement/

echo "K32W SDK Master Release 2 was patched!"
exit 0
