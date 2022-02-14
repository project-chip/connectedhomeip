#!/bin/bash

if [[ ! -d $MW320_SDK_ROOT ]]; then
    echo "MW320_SDK_ROOT is not set"
    exit 1
fi

unzip ./third_party/mw320_sdk/sdk_fixes/mw320_matter_flash.zip -d "$MW320_SDK_ROOT"/
echo "export MW320_SDK_ROOT=\"$MW320_SDK_ROOT\"" >./third_party/mw320_sdk/sdk_fixes/set_env.sh
chmod +x+w ./third_party/mw320_sdk/sdk_fixes/set_env.sh

cp ./third_party/mw320_sdk/sdk_fixes/mw_img_conv "$MW320_SDK_ROOT"/tools/mw_img_conv/src/
cp ./third_party/mw320_sdk/sdk_fixes/lwipopts.h "$MW320_SDK_ROOT"/boards/rdmw320_r0/wifi_examples/mw_wifi_cli/
cp ./third_party/mw320_sdk/sdk_fixes/pin_mux.c "$MW320_SDK_ROOT"/boards/rdmw320_r0/wifi_examples/mw_wifi_cli/
cp ./third_party/mw320_sdk/sdk_fixes/pin_mux.h "$MW320_SDK_ROOT"/boards/rdmw320_r0/wifi_examples/mw_wifi_cli/
cp ./third_party/mw320_sdk/sdk_fixes/wifi_config.h "$MW320_SDK_ROOT"/boards/rdmw320_r0/wifi_examples/mw_wifi_cli/
cp ./third_party/mw320_sdk/sdk_fixes/88MW320.h "$MW320_SDK_ROOT"/devices/88MW320/
cp ./third_party/mw320_sdk/sdk_fixes/lwiperf.c "$MW320_SDK_ROOT"/middleware/lwip/src/apps/lwiperf/
cp ./third_party/mw320_sdk/sdk_fixes/errno.h "$MW320_SDK_ROOT"/middleware/lwip/src/include/lwip/
cp ./third_party/mw320_sdk/sdk_fixes/ip_addr.h "$MW320_SDK_ROOT"/middleware/lwip/src/include/lwip/
cp ./third_party/mw320_sdk/sdk_fixes/wm_net.h "$MW320_SDK_ROOT"/middleware/wifi/incl/port/lwip/
cp ./third_party/mw320_sdk/sdk_fixes/wifi.h "$MW320_SDK_ROOT"/middleware/wifi/incl/wifidriver/
cp ./third_party/mw320_sdk/sdk_fixes/wifi_events.h "$MW320_SDK_ROOT"/middleware/wifi/incl/wifidriver/
cp ./third_party/mw320_sdk/sdk_fixes/wlan.h "$MW320_SDK_ROOT"/middleware/wifi/incl/wlcmgr/
cp ./third_party/mw320_sdk/sdk_fixes/iperf.c "$MW320_SDK_ROOT"/middleware/wifi/nw_utils/
cp ./third_party/mw320_sdk/sdk_fixes/net.c "$MW320_SDK_ROOT"/middleware/wifi/port/lwip/
cp ./third_party/mw320_sdk/sdk_fixes/netif_decl.h "$MW320_SDK_ROOT"/middleware/wifi/port/lwip/
cp ./third_party/mw320_sdk/sdk_fixes/wifi_netif.c "$MW320_SDK_ROOT"/middleware/wifi/port/lwip/
cp ./third_party/mw320_sdk/sdk_fixes/wifi.c "$MW320_SDK_ROOT"/middleware/wifi/wifidriver/
cp ./third_party/mw320_sdk/sdk_fixes/wifi-sdio.c "$MW320_SDK_ROOT"/middleware/wifi/wifidriver/
cp ./third_party/mw320_sdk/sdk_fixes/wifi-internal.h "$MW320_SDK_ROOT"/middleware/wifi/wifidriver/
cp ./third_party/mw320_sdk/sdk_fixes/wlan.c "$MW320_SDK_ROOT"/middleware/wifi/wlcmgr/
cp ./third_party/mw320_sdk/sdk_fixes/wlan_tests.c "$MW320_SDK_ROOT"/middleware/wifi/wlcmgr/

echo "MW320 SDK and Flsahing tool was installed to $MW320_SDK_ROOT !"
exit 0
