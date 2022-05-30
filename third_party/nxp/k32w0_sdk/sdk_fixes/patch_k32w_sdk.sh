#!/bin/bash

if [[ ! -d $NXP_K32W061_SDK_ROOT ]]; then
    echo "NXP_K32W061_SDK_ROOT is not set"
    exit 1
fi

convert_to_dos() {

    [[ $(file -b - <$1) != *"CRLF"* ]] && sed -i 's/$/\r/' "$1"
}

SOURCE=${BASH_SOURCE[0]}
SOURCE_DIR=$(cd "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)

convert_to_dos "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm/gpio_pins.h
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm -p1 <"$SOURCE_DIR/gpio_pins_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common/app_dual_mode_low_power.h
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common -p1 <"$SOURCE_DIR/app_dual_mode_low_power_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common/app_dual_mode_switch.h
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/hybrid/ble_ot/lped_ble_wuart/ble_802_15_4_common -p1 <"$SOURCE_DIR/app_dual_mode_switch_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/SecLib/SecLib.h
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/SecLib -p1 <"$SOURCE_DIR/SecLib_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Source/OtaUtils.c
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Source -p1 <"$SOURCE_DIR/OtaUtils_c.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Source/OtaSupport.c
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/OtaSupport/Source -p1 <"$SOURCE_DIR/OtaSupport_c.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/bluetooth/host/interface/ble_utils.h
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/bluetooth/host/interface -p1 <"$SOURCE_DIR/ble_utils_h.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/Flash/External/Source/Eeprom_MX25R8035F.c
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/Flash/External/Source -p1 <"$SOURCE_DIR/Eeprom_MX25R8035F_c.patch"

convert_to_dos "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/MemManager/Interface/MemManager.h
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/MemManager/Interface -p1 <"$SOURCE_DIR/MemManager_h.patch"

SIGN_FILE_PATH="$NXP_K32W061_SDK_ROOT"/tools/imagetool/sign_images.sh
convert_to_dos "$SIGN_FILE_PATH"
patch -N --binary -d "$NXP_K32W061_SDK_ROOT"/tools/imagetool/ -p1 <"$SOURCE_DIR/sign_images_sh.patch"
sed -i 's/\r$//' "$SIGN_FILE_PATH"

echo "Downloading PDM and BLE libraries from NXP server..."

rm -rf patch_for_K32W061_SDK_2_6_4.zip patch_for_K32W061_SDK_2_6_4
wget https://www.nxp.com/downloads/en/libraries/patch_for_K32W061_SDK_2_6_4.zip
exitCode=$?
if [ "$exitCode" -ne 0 ]; then
    echo "Download error"
    exit
fi

unzip patch_for_K32W061_SDK_2_6_4.zip
cp patch_for_K32W061_SDK_2_6_4/controller_config.c "$NXP_K32W061_SDK_ROOT"/middleware/wireless/ble_controller/config/
cp patch_for_K32W061_SDK_2_6_4/controller_interface.h "$NXP_K32W061_SDK_ROOT"/middleware/wireless/ble_controller/interface/
cp patch_for_K32W061_SDK_2_6_4/lib_ble_controller.a "$NXP_K32W061_SDK_ROOT"/middleware/wireless/ble_controller/lib/
cp patch_for_K32W061_SDK_2_6_4/libPDM_extFlash.a "$NXP_K32W061_SDK_ROOT"/middleware/wireless/framework/PDM/Library/

echo "K32W SDK MR3 QP1 was patched!"
exit 0
