#!/bin/bash

if [[ ! -d $NXP_K32W061_SDK_ROOT ]]; then
    echo "NXP_K32W061_SDK_ROOT is not set"
    exit 1
fi

convert_to_dos() {

    [[ $(file -b - <$1) != *"CRLF"* ]] && unix2dos "$1"
}

SOURCE=${BASH_SOURCE[0]}
SOURCE_DIR=$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)

convert_to_dos "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm/gpio_pins.h
patch --binary -d "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm -p1 <"$SOURCE_DIR/gpio_pins_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common/app_dual_mode_low_power.h
patch --binary -d "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common -p1 <"$SOURCE_DIR/app_dual_mode_low_power_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common/app_dual_mode_switch.h
patch --binary -d "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common -p1 <"$SOURCE_DIR/app_dual_mode_switch_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/SecLib/SecLib.h
patch --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/SecLib -p1 <"$SOURCE_DIR/SecLib_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Source/OtaUtils.c
patch --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Source -p1 <"$SOURCE_DIR/OtaUtils_c.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/bluetooth/host/interface/ble_utils.h
patch --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/bluetooth/host/interface -p1 <"$SOURCE_DIR/ble_utils_h.patch"

echo "K32W SDK MR3 QP1 was patched!"
exit 0
