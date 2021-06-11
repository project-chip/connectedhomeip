#!/bin/bash

if [[ ! -d $K32W061_SDK_ROOT ]]; then
    echo "K32W061_SDK_ROOT is not set"
    exit 1
fi

cp ./third_party/k32w_sdk/sdk_fixes/controller_interface.h "$K32W061_SDK_ROOT"/middleware/wireless/ble_controller/interface/
cp ./third_party/k32w_sdk/sdk_fixes/Messaging.h "$K32W061_SDK_ROOT"/middleware/wireless/framework/Messaging/Interface/
cp ./third_party/k32w_sdk/sdk_fixes/gpio_pins.h "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/reed/bm/
cp ./third_party/k32w_sdk/sdk_fixes/pin_mux.c "$K32W061_SDK_ROOT"/boards/k32w061dk6/wireless_examples/openthread/enablement/
cp ./third_party/k32w_sdk/sdk_fixes/ntag_driver.h "$K32W061_SDK_ROOT"/middleware/ntag_i2c_plus/HAL_NTAG/inc/
cp ./third_party/k32w_sdk/sdk_fixes/HAL_I2C_jn_fsl.h "$K32W061_SDK_ROOT"/middleware/ntag_i2c_plus/HAL_I2C/inc/
cp ./third_party/k32w_sdk/sdk_fixes/SerialManager.c "$K32W061_SDK_ROOT"/middleware/wireless/framework/SerialManager/Source/
cp ./third_party/k32w_sdk/sdk_fixes/Eeprom_MX25R8035F.c "$K32W061_SDK_ROOT"/middleware/wireless/framework/Flash/External/Source
cp ./third_party/k32w_sdk/sdk_fixes/system.c ./third_party/openthread/ot-nxp/src/k32w/platform/
cp ./third_party/k32w_sdk/sdk_fixes/openthread-core-k32w061-config.h ./third_party/openthread/ot-nxp/src/k32w/k32w061

echo "K32W SDK Master Release 3 was patched!"
exit 0
