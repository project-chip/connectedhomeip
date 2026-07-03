/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "SlNetConfig.h"
#include "ble_config.h"

#include <lib/support/CodeUtils.h>

extern "C" {
#include "sl_si91x_types.h"
#include "sl_wifi.h"
}

sl_wifi_device_configuration_t SLNetGetDefaultDeviceConfiguration(void)
{
    return { .boot_option = LOAD_NWP_FW,
             .mac_address = nullptr,
             .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
             .region_code = REGION_CODE_BITMAP,
             .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                              .coex_mode = SL_WIFI_SYSTEM_WLAN_ONLY_MODE,
                              .feature_bit_map =
                                  SL_SI91X_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_AGGREGATION | SL_SI91X_FEAT_WPS_DISABLE,
                              .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_BYPASS | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                              .custom_feature_bit_map     = SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
                              .ext_custom_feature_bit_map = SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_CLK |
                                  SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV | FRONT_END_SWITCH_CTRL | SL_SI91X_EXT_FEAT_IEEE_80211W,
                              .bt_feature_bit_map         = 0,
                              .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                              .ble_feature_bit_map        = 0,
                              .ble_ext_feature_bit_map    = 0,
                              .config_feature_bit_map     = SL_SI91X_ENABLE_ENHANCED_MAX_PSP } };
}

void SLWiFiApplyDeviceConfiguration(sl_wifi_device_configuration_t * configuration)
{
    VerifyOrReturn(configuration != nullptr);
#ifndef SLI_SI91X_MCU_INTERFACE
    configuration->boot_config.feature_bit_map |= SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1 | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE;
    configuration->boot_config.config_feature_bit_map |= SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP;
#endif // SLI_SI91X_MCU_INTERFACE

#ifdef ipv6_FEATURE_REQUIRED
    configuration->boot_config.tcp_ip_feature_bit_map |= (SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6);
#endif // ipv6_FEATURE_REQUIRED

#ifdef RSI_PROCESS_MAX_RX_DATA
    configuration->boot_config.ext_tcp_ip_feature_bit_map |= SL_SI91X_EXT_TCP_MAX_RECV_LENGTH;
#endif // RSI_PROCESS_MAX_RX_DATA
}

void SLBLEApplyDeviceConfiguration(sl_wifi_device_configuration_t * configuration)
{
#if defined(SLI_SI91X_ENABLE_BLE) && SLI_SI91X_ENABLE_BLE

    VerifyOrReturn(configuration != nullptr);
    configuration->boot_config.coex_mode = SL_SI91X_WLAN_BLE_MODE;
    configuration->boot_config.ext_custom_feature_bit_map |= SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE;
    configuration->boot_config.bt_feature_bit_map = RSI_BT_FEATURE_BITMAP;
    configuration->boot_config.ble_feature_bit_map =
        ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS) | SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS) |
          SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV) | SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)) |
         SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX) |
         SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
         | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif // RSI_BLE_GATT_ASYNC_ENABLE
        );
    configuration->boot_config.ble_ext_feature_bit_map =
        (SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS) | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES));

#endif // SLI_SI91X_ENABLE_BLE
}
