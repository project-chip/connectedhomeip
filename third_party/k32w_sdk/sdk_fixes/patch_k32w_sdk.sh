#!/bin/bash

if [[ ! -d $K32W061_SDK_ROOT ]]; then
    echo "K32W061_SDK_ROOT is not set"
    exit 1
fi

cp ./third_party/k32w_sdk/sdk_fixes/gpio_pins.h "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm/
cp ./third_party/k32w_sdk/sdk_fixes/pin_mux.c "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/enablement/

#TODO internal: https://jira.sw.nxp.com/browse/MCB-2678
cp ./third_party/k32w_sdk/sdk_fixes/SecLib.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/SecLib/

# TODO internal: https://jira.sw.nxp.com/browse/MCB-2675
cp -r ./third_party/k32w_sdk/sdk_fixes/app_dual_mode_low_power.h "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common/
cp -r ./third_party/k32w_sdk/sdk_fixes/app_dual_mode_switch.h "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common/

echo "K32W SDK MR3 QP1 was patched!"
exit 0
