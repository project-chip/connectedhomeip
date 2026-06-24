/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "ble_config.h"
#include "wifi_config.h"

#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/WifiInterface.h>
#include <platform/silabs/wifi/wfx_msgs.h>

extern "C" {
#include "sl_si91x_driver.h"
#include "sl_si91x_types.h"
#include "sl_wifi.h"
}

#if SLI_SI91X_MCU_INTERFACE
#include "rsi_wisemcu_hardware_setup.h"
#endif // SLI_SI91X_MCU_INTERFACE

#if SL_MBEDTLS_USE_TINYCRYPT
#include "sl_si91x_trng.h"
#endif // SL_MBEDTLS_USE_TINYCRYPT

extern WfxRsi_t wfx_rsi;

namespace {
sl_wifi_device_configuration_t sWifiDeviceConfiguration;
bool sWifiDeviceConfigurationSet = false;
} // namespace

sl_wifi_device_configuration_t MatterWifiGetBaseDeviceConfiguration(void)
{
    return {
        .boot_option = LOAD_NWP_FW,
        .mac_address = nullptr,
        .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
        .region_code = REGION_CODE_BITMAP,
        .boot_config = { .oper_mode                  = SL_SI91X_CLIENT_MODE,
                         .coex_mode                  = 0,
                         .feature_bit_map            = SL_SI91X_FEAT_SECURITY_OPEN,
                         .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT |
                                                        SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_BYPASS |
                                                        SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                         .custom_feature_bit_map     = SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID,
                         .ext_custom_feature_bit_map = RSI_EXT_CUSTOM_FEATURE_BIT_MAP,
                         .bt_feature_bit_map         = 0,
                         .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                         .ble_feature_bit_map        = 0,
                         .ble_ext_feature_bit_map    = 0,
                         .config_feature_bit_map     = SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP }
    };
}

void MatterWifiApplyOptionalDeviceConfiguration(sl_wifi_device_configuration_t * configuration)
{
    if (configuration == nullptr)
    {
        return;
    }

#ifdef SLI_SI91X_MCU_INTERFACE
    configuration->boot_config.feature_bit_map |= SL_SI91X_FEAT_WPS_DISABLE;
#else
    configuration->boot_config.feature_bit_map |=
        (SL_SI91X_FEAT_AGGREGATION | SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1);
#endif

#ifdef ipv6_FEATURE_REQUIRED
    configuration->boot_config.tcp_ip_feature_bit_map |=
        (SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6);
#endif

#if defined(SLI_SI91X_ENABLE_BLE) && SLI_SI91X_ENABLE_BLE
    configuration->boot_config.coex_mode = SL_SI91X_WLAN_BLE_MODE;
    configuration->boot_config.ext_custom_feature_bit_map |= SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE;
    configuration->boot_config.bt_feature_bit_map = RSI_BT_FEATURE_BITMAP;
#if (RSI_BT_GATT_ON_CLASSIC)
    configuration->boot_config.bt_feature_bit_map |= SL_SI91X_BT_ATT_OVER_CLASSIC_ACL;
#endif
    configuration->boot_config.ble_feature_bit_map =
        ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS) |
          SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS) | SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV) |
          SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)) |
         SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX) |
         SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
         | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
        );
    configuration->boot_config.ble_ext_feature_bit_map =
        (SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS) | SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES))
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
        | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
        | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
        | SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
        | SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST
#endif
#if BLE_SIMPLE_GATT
        | SL_SI91X_BLE_GATT_INIT
#endif
        ;
#endif // SLI_SI91X_ENABLE_BLE

#if (defined A2DP_POWER_SAVE_ENABLE)
    configuration->boot_config.ext_custom_feature_bit_map |= SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(2);
#endif

#ifdef RSI_PROCESS_MAX_RX_DATA
    configuration->boot_config.ext_tcp_ip_feature_bit_map |= SL_SI91X_EXT_TCP_MAX_RECV_LENGTH;
#endif
}

sl_wifi_device_configuration_t MatterWifiGetDefaultDeviceConfiguration(void)
{
    sl_wifi_device_configuration_t configuration = MatterWifiGetBaseDeviceConfiguration();
    MatterWifiApplyOptionalDeviceConfiguration(&configuration);
    return configuration;
}

void MatterWifiSetDeviceConfiguration(const sl_wifi_device_configuration_t * configuration)
{
    if (configuration == nullptr)
    {
        return;
    }

    sWifiDeviceConfiguration    = *configuration;
    sWifiDeviceConfigurationSet = true;
}

const sl_wifi_device_configuration_t * MatterWifiGetDeviceConfiguration(void)
{
    return sWifiDeviceConfigurationSet ? &sWifiDeviceConfiguration : nullptr;
}

sl_status_t SiWxPlatformInit(const sl_wifi_device_configuration_t * configuration)
{
    sl_status_t status = SL_STATUS_OK;

    MatterWifiSetDeviceConfiguration(configuration);

#ifdef SLI_SI91X_MCU_INTERFACE
    uint8_t xtal_enable = 1;
    status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, nullptr);
    if (status != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "sl_si91x_m4_ta_secure_handshake failed: 0x%lx", static_cast<uint32_t>(status));
        return status;
    }
#endif // SLI_SI91X_MCU_INTERFACE

    sl_wifi_firmware_version_t version = { 0 };
    status                             = sl_wifi_get_firmware_version(&version);
    if (status != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "sl_wifi_get_firmware_version failed: 0x%lx", static_cast<uint32_t>(status));
        return status;
    }

    ChipLogDetail(DeviceLayer, "Firmware version is: %x%x.%d.%d.%d.%d.%d.%d", version.chip_id, version.rom_id, version.major,
                  version.minor, version.security_version, version.patch_num, version.customer_id, version.build_num);

    status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, reinterpret_cast<sl_mac_address_t *>(wfx_rsi.sta_mac.data()));
    if (status != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "sl_wifi_get_mac_address failed: 0x%lx", static_cast<uint32_t>(status));
        return status;
    }

#ifdef SL_MBEDTLS_USE_TINYCRYPT
    constexpr uint32_t trngKey[TRNG_KEY_SIZE] = { 0x16157E2B, 0xA6D2AE28, 0x8815F7AB, 0x3C4FCF09 };

    status = sl_si91x_trng_entropy();
    if (status != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "sl_si91x_trng_entropy failed: 0x%lx", static_cast<uint32_t>(status));
        return status;
    }

    status = sl_si91x_trng_program_key((uint32_t *) trngKey, TRNG_KEY_SIZE);
    if (status != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "sl_si91x_trng_program_key failed: 0x%lx", static_cast<uint32_t>(status));
        return status;
    }
#endif // SL_MBEDTLS_USE_TINYCRYPT

    wfx_rsi.dev_state.Set(chip::DeviceLayer::Silabs::WifiInterface::WifiState::kStationInit);
    return status;
}
