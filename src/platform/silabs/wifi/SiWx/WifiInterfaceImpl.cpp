/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/*
 * This file implements the interface to the wifi sdk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (SL_MATTER_GN_BUILD == 0)
#include "sl_matter_wifi_config.h"
#endif // SL_MATTER_GN_BUILD

#include "FreeRTOS.h"
#include "ble_config.h"
#include "event_groups.h"
#include "sl_status.h"
#include "sl_wifi_device.h"
#include "task.h"
#include <app/icd/server/ICDServerConfig.h>
#include <inet/IPAddress.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/silabs/wifi/WifiInterface.h>
#include <platform/silabs/wifi/wiseconnect-interface/WiseconnectWifiInterface.h>

extern "C" {
#include "sl_si91x_driver.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_types.h"
#include "sl_wifi.h"
#include "sl_wifi_callback_framework.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_types.h"
#if SL_MBEDTLS_USE_TINYCRYPT
#include "sl_si91x_constants.h"
#include "sl_si91x_trng.h"
#endif // SL_MBEDTLS_USE_TINYCRYPT

#include <sl_net.h>
#include <sl_net_constants.h>
#include <sl_net_for_lwip.h>
#include <sl_net_wifi_types.h>
}

#if SLI_SI91X_MCU_INTERFACE
#include "rsi_wisemcu_hardware_setup.h"
#endif // SLI_SI91X_MCU_INTERFACE

#if (EXP_BOARD)
#include "rsi_bt_common_apis.h"
#include <platform/silabs/wifi/SiWx/ncp/sl_board_configuration.h>
#endif

#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
#include "rsi_rom_power_save.h"
#include "sl_gpio_board.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_power_manager.h"
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE

// TODO : Temporary work-around for wifi-init failure in 917NCP and 917SOC ACX module boards.
// Can be removed after Wiseconnect fixes region code for all ACX module boards.
#if (SL_SI91X_ACX_MODULE == 1) || defined(EXP_BOARD)
#define REGION_CODE IGNORE_REGION
#else
#define REGION_CODE US
#endif

WfxRsi_t wfx_rsi;
extern osSemaphoreId_t sl_rs_ble_init_sem;

namespace {

#if CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE
// TODO: should be removed once we are getting the press interrupt for button 0 with sleep
bool btn0_pressed = false;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && SLI_SI91X_MCU_INTERFACE

bool hasNotifiedWifiConnectivity = false;
bool hasNotifiedIPV6             = false;
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
bool hasNotifiedIPV4 = false;
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

wfx_wifi_scan_ext_t temp_reset;

osSemaphoreId_t sScanCompleteSemaphore;
osSemaphoreId_t sScanInProgressSemaphore;
osTimerId_t sDHCPTimer;
osMessageQueueId_t sWifiEventQueue = nullptr;

sl_net_wifi_lwip_context_t wifi_client_context;
sl_wifi_security_t security = SL_WIFI_SECURITY_UNKNOWN;

const sl_wifi_device_configuration_t config = {
    .boot_option = LOAD_NWP_FW,
    .mac_address = NULL,
    .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
    .region_code = REGION_CODE,
    .boot_config = { .oper_mode = SL_SI91X_CLIENT_MODE,
                     .coex_mode = SL_SI91X_WLAN_BLE_MODE,
                     .feature_bit_map =
#ifdef SLI_SI91X_MCU_INTERFACE
                         (SL_SI91X_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_WPS_DISABLE),
#else
                         (SL_SI91X_FEAT_SECURITY_OPEN | SL_SI91X_FEAT_AGGREGATION),
#endif
                     .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT | SL_SI91X_TCP_IP_FEAT_DNS_CLIENT |
                                                SL_SI91X_TCP_IP_FEAT_SSL | SL_SI91X_TCP_IP_FEAT_BYPASS
#ifdef ipv6_FEATURE_REQUIRED
                                                | SL_SI91X_TCP_IP_FEAT_DHCPV6_CLIENT | SL_SI91X_TCP_IP_FEAT_IPV6
#endif
                                                | SL_SI91X_TCP_IP_FEAT_ICMP | SL_SI91X_TCP_IP_FEAT_EXTENSION_VALID),
                     .custom_feature_bit_map     = (SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID | RSI_CUSTOM_FEATURE_BIT_MAP),
                     .ext_custom_feature_bit_map = (RSI_EXT_CUSTOM_FEATURE_BIT_MAP | (SL_SI91X_EXT_FEAT_BT_CUSTOM_FEAT_ENABLE)
#if (defined A2DP_POWER_SAVE_ENABLE)
                                                    | SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(2)
#endif
                                                        ),
                     .bt_feature_bit_map = (RSI_BT_FEATURE_BITMAP
#if (RSI_BT_GATT_ON_CLASSIC)
                                            | SL_SI91X_BT_ATT_OVER_CLASSIC_ACL /* to support att over classic acl link */
#endif
                                            ),
#ifdef RSI_PROCESS_MAX_RX_DATA
                     .ext_tcp_ip_feature_bit_map =
                         (RSI_EXT_TCPIP_FEATURE_BITMAP | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID | SL_SI91X_EXT_TCP_MAX_RECV_LENGTH),
#else
                     .ext_tcp_ip_feature_bit_map = (RSI_EXT_TCPIP_FEATURE_BITMAP | SL_SI91X_CONFIG_FEAT_EXTENTION_VALID),
#endif
                     //! ENABLE_BLE_PROTOCOL in bt_feature_bit_map
                     .ble_feature_bit_map =
                         ((SL_SI91X_BLE_MAX_NBR_PERIPHERALS(RSI_BLE_MAX_NBR_PERIPHERALS) |
                           SL_SI91X_BLE_MAX_NBR_CENTRALS(RSI_BLE_MAX_NBR_CENTRALS) |
                           SL_SI91X_BLE_MAX_NBR_ATT_SERV(RSI_BLE_MAX_NBR_ATT_SERV) |
                           SL_SI91X_BLE_MAX_NBR_ATT_REC(RSI_BLE_MAX_NBR_ATT_REC)) |
                          SL_SI91X_FEAT_BLE_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_BLE_PWR_INX(RSI_BLE_PWR_INX) |
                          SL_SI91X_BLE_PWR_SAVE_OPTIONS(RSI_BLE_PWR_SAVE_OPTIONS) | SL_SI91X_916_BLE_COMPATIBLE_FEAT_ENABLE
#if RSI_BLE_GATT_ASYNC_ENABLE
                          | SL_SI91X_BLE_GATT_ASYNC_ENABLE
#endif
                          ),

                     .ble_ext_feature_bit_map = ((SL_SI91X_BLE_NUM_CONN_EVENTS(RSI_BLE_NUM_CONN_EVENTS) |
                                                  SL_SI91X_BLE_NUM_REC_BYTES(RSI_BLE_NUM_REC_BYTES))
#if RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST
                                                 | SL_SI91X_BLE_INDICATE_CONFIRMATION_FROM_HOST // indication response from app
#endif
#if RSI_BLE_MTU_EXCHANGE_FROM_HOST
                                                 | SL_SI91X_BLE_MTU_EXCHANGE_FROM_HOST // MTU Exchange request initiation from app
#endif
#if RSI_BLE_SET_SCAN_RESP_DATA_FROM_HOST
                                                 | (SL_SI91X_BLE_SET_SCAN_RESP_DATA_FROM_HOST) // Set SCAN Resp Data from app
#endif
#if RSI_BLE_DISABLE_CODED_PHY_FROM_HOST
                                                 | (SL_SI91X_BLE_DISABLE_CODED_PHY_FROM_HOST) // Disable Coded PHY from app
#endif
#if BLE_SIMPLE_GATT
                                                 | SL_SI91X_BLE_GATT_INIT
#endif
                                                 ),
                     .config_feature_bit_map = (SL_SI91X_FEAT_SLEEP_GPIO_SEL_BITMAP | RSI_CONFIG_FEATURE_BITMAP) }
};

constexpr int8_t kAdvScanThreshold           = -40;
constexpr uint8_t kAdvRssiToleranceThreshold = 5;
constexpr uint8_t kAdvActiveScanDuration     = 15;
constexpr uint8_t kAdvPassiveScanDuration    = 20;
constexpr uint8_t kAdvMultiProbe             = 1;
constexpr uint8_t kAdvScanPeriodicity        = 10;
constexpr uint8_t kAdvEnableInstantbgScan    = 1;

// TODO: Confirm that this value works for size and timing
constexpr uint8_t kWfxQueueSize = 10;

// TODO: Figure out why we actually need this, we are already handling failure and retries somewhere else.
constexpr uint16_t kWifiScanTimeoutTicks = 10000;

/**
 * @brief Network Scan callback when the device receive a scan operation from the controller.
 *        This callback is used whe the Network Commission Driver send a ScanNetworks command.
 *
 *        If the scan network was requested for a specific SSID - wfx_rsi.scan_ssid had a valid value,
 *        the callback will only forward that specific networks information.
 *        If no ssid is provided, wfx_rsi.scan_ssid is a nullptr, we return the information of all scanned networks.
 */
sl_status_t BackgroundScanCallback(sl_wifi_event_t event, sl_wifi_scan_result_t * result, uint32_t result_length, void * arg)
{
    VerifyOrReturnError(result != nullptr, SL_STATUS_NULL_POINTER);
    VerifyOrReturnError(wfx_rsi.scan_cb != nullptr, SL_STATUS_INVALID_HANDLE);

    uint32_t nbreResults = result->scan_count;
    chip::ByteSpan requestedSsidSpan(wfx_rsi.scan_ssid, wfx_rsi.scan_ssid_length);

    for (uint32_t i = 0; i < nbreResults; i++)
    {
        wfx_wifi_scan_result_t currentScanResult = { 0 };

        // Lenght excludes null-character
        size_t scannedSsidLenght = strnlen(reinterpret_cast<char *>(result->scan_info[i].ssid), WFX_MAX_SSID_LENGTH);
        chip::ByteSpan scannedSsidSpan(result->scan_info[i].ssid, scannedSsidLenght);

        // Copy the scanned SSID to the current scan ssid buffer that will be forwarded to the callback
        chip::MutableByteSpan currentScanSsid(currentScanResult.ssid, WFX_MAX_SSID_LENGTH);
        chip::CopySpanToMutableSpan(scannedSsidSpan, currentScanSsid);
        currentScanResult.ssid_length = currentScanSsid.size();

        chip::ByteSpan inBssid(result->scan_info[i].bssid, kWifiMacAddressLength);
        chip::MutableByteSpan outBssid(currentScanResult.bssid, kWifiMacAddressLength);
        chip::CopySpanToMutableSpan(inBssid, outBssid);

        // TODO: We should revisit this to make sure we are setting the correct values
        currentScanResult.security = static_cast<wfx_sec_t>(result->scan_info[i].security_mode);
        currentScanResult.rssi     = (-1) * result->scan_info[i].rssi_val; // The returned value is positive - we need to flip it
        currentScanResult.chan     = result->scan_info[i].rf_channel;

        // if user has provided ssid, check if the current scan result ssid matches the user provided ssid
        if (!requestedSsidSpan.empty())
        {
            if (requestedSsidSpan.data_equal(currentScanSsid))
            {
                wfx_rsi.scan_cb(&currentScanResult);
            }
        }
        else // No ssid was provide - forward all results
        {
            wfx_rsi.scan_cb(&currentScanResult);
        }
    }

    // cleanup and return
    wfx_rsi.dev_state.Clear(WifiState::kScanStarted);
    wfx_rsi.scan_cb(nullptr);
    wfx_rsi.scan_cb = nullptr;
    if (wfx_rsi.scan_ssid)
    {
        chip::Platform::MemoryFree(wfx_rsi.scan_ssid);
        wfx_rsi.scan_ssid = nullptr;
    }
    osSemaphoreRelease(sScanCompleteSemaphore);

    return SL_STATUS_OK;
}

void DHCPTimerEventHandler(void * arg)
{
    WifiPlatformEvent event = WifiPlatformEvent::kStationDhcpPoll;
    sl_matter_wifi_post_event(event);
}

void CancelDHCPTimer(void)
{
    VerifyOrReturn(osTimerIsRunning(sDHCPTimer), ChipLogDetail(DeviceLayer, "CancelDHCPTimer: timer not running"));
    VerifyOrReturn(osTimerStop(sDHCPTimer) == osOK, ChipLogError(DeviceLayer, "CancelDHCPTimer: failed to stop timer"));
}

void StartDHCPTimer(uint32_t timeout)
{
    // Cancel timer if already started
    CancelDHCPTimer();

    VerifyOrReturn(osTimerStart(sDHCPTimer, pdMS_TO_TICKS(timeout)) == osOK,
                   ChipLogError(DeviceLayer, "StartDHCPTimer: failed to start timer"));
}

sl_status_t sl_wifi_siwx917_init(void)
{
    sl_status_t status = SL_STATUS_OK;

#ifdef SLI_SI91X_MCU_INTERFACE
    // SoC Configurations
    uint8_t xtal_enable = 1;
    status              = sl_si91x_m4_ta_secure_handshake(SL_SI91X_ENABLE_XTAL, 1, &xtal_enable, 0, nullptr);
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_si91x_m4_ta_secure_handshake failed: 0x%lx", static_cast<uint32_t>(status)));

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#ifdef ENABLE_CHIP_SHELL
    // While using the matter shell with a Low Power Build, GPIO 1 is used to check the UULP PIN 1 status
    // since UART doesn't act as a wakeup source in the UULP mode.

    // Configuring the NPS GPIO 1
    RSI_NPSSGPIO_SetPinMux(RTE_UULP_GPIO_1_PIN, 0);

    // Configure the NPSS GPIO direction to input
    RSI_NPSSGPIO_SetDir(RTE_UULP_GPIO_1_PIN, 1);

    // Enable the REN
    RSI_NPSSGPIO_InputBufferEn(RTE_UULP_GPIO_1_PIN, 1);
#endif // ENABLE_CHIP_SHELL
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#else
    // NCP Configurations
    status = sl_matter_wifi_platform_init();
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_matter_wifi_platform_init failed: 0x%lx", static_cast<uint32_t>(status)));
#endif // SLI_SI91X_MCU_INTERFACE

    sl_wifi_firmware_version_t version = { 0 };
    status                             = sl_wifi_get_firmware_version(&version);
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_wifi_get_firmware_version failed: 0x%lx", static_cast<uint32_t>(status)));

    ChipLogDetail(DeviceLayer, "Firmware version is: %x%x.%d.%d.%d.%d.%d.%d", version.chip_id, version.rom_id, version.major,
                  version.minor, version.security_version, version.patch_num, version.customer_id, version.build_num);

    status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, reinterpret_cast<sl_mac_address_t *>(wfx_rsi.sta_mac.data()));
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_wifi_get_mac_address failed: 0x%lx", static_cast<uint32_t>(status)));

#ifdef SL_MBEDTLS_USE_TINYCRYPT
    constexpr uint32_t trngKey[TRNG_KEY_SIZE] = { 0x16157E2B, 0xA6D2AE28, 0x8815F7AB, 0x3C4FCF09 };

    // To check the Entropy of TRNG and verify TRNG functioning.
    status = sl_si91x_trng_entropy();
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_si91x_trng_entropy failed: 0x%lx", static_cast<uint32_t>(status)));

    // Initiate and program the key required for TRNG hardware engine
    status = sl_si91x_trng_program_key((uint32_t *) trngKey, TRNG_KEY_SIZE);
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_si91x_trng_program_key failed: 0x%lx", static_cast<uint32_t>(status)));
#endif // SL_MBEDTLS_USE_TINYCRYPT

    wfx_rsi.dev_state.Set(WifiState::kStationInit);
    osSemaphoreRelease(sl_rs_ble_init_sem);
    return status;
}

// TODO: this changes will be reverted back after the Silabs WiFi SDK team fix the scan API
#ifndef EXP_BOARD
sl_status_t ScanCallback(sl_wifi_event_t event, sl_wifi_scan_result_t * scan_result, uint32_t result_length, void * arg)
{
    sl_status_t status = SL_STATUS_OK;
    if (SL_WIFI_CHECK_IF_EVENT_FAILED(event))
    {
        ChipLogError(DeviceLayer, "Scan Netwrok Failed: 0x%lx", *reinterpret_cast<sl_status_t *>(status));
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        security = SL_WIFI_WPA3;
#else
        security = SL_WIFI_WPA_WPA2_MIXED;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */

        status = SL_STATUS_FAIL;
    }
    else
    {
        security        = static_cast<sl_wifi_security_t>(scan_result->scan_info[0].security_mode);
        wfx_rsi.ap_chan = scan_result->scan_info[0].rf_channel;
        memcpy(wfx_rsi.ap_mac.data(), scan_result->scan_info[0].bssid, kWifiMacAddressLength);
    }

    osSemaphoreRelease(sScanCompleteSemaphore);
    return status;
}
#endif

sl_status_t InitiateScan()
{
    sl_status_t status = SL_STATUS_OK;

// TODO: this changes will be reverted back after the Silabs WiFi SDK team fix the scan API
#ifndef EXP_BOARD
    sl_wifi_ssid_t ssid = { 0 };

    // TODO: this changes will be reverted back after the Silabs WiFi SDK team fix the scan API
    sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;

    ssid.length = wfx_rsi.sec.ssid_length;

    // TODO: workaround because the string management with the null termination is flawed
    chip::Platform::CopyString((char *) &ssid.value[0], ssid.length + 1, wfx_rsi.sec.ssid);
    sl_wifi_set_scan_callback(ScanCallback, NULL);

    osSemaphoreAcquire(sScanInProgressSemaphore, osWaitForever);

    // This is an odd success code?
    status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, &ssid, &wifi_scan_configuration);
    if (status == SL_STATUS_IN_PROGRESS)
    {
        osSemaphoreAcquire(sScanCompleteSemaphore, kWifiScanTimeoutTicks);
        status = SL_STATUS_OK;
    }

    osSemaphoreRelease(sScanInProgressSemaphore);
#endif

    return status;
}

sl_status_t SetWifiConfigurations()
{
    sl_status_t status = SL_STATUS_OK;

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Setting the listen interval to 0 which will set it to DTIM interval
    sl_wifi_listen_interval_t sleep_interval = { .listen_interval = 0 };
    status                                   = sl_wifi_set_listen_interval(SL_WIFI_CLIENT_INTERFACE, sleep_interval);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    sl_wifi_advanced_client_configuration_t client_config = { .max_retry_attempts = 5 };
    status = sl_wifi_set_advanced_client_configuration(SL_WIFI_CLIENT_INTERFACE, &client_config);
    VerifyOrReturnError(status == SL_STATUS_OK, status);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID, SL_NET_WIFI_PSK, &wfx_rsi.sec.passkey[0],
                                   wfx_rsi.sec.passkey_length);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

    sl_net_wifi_client_profile_t profile = {
        .config = {
            .ssid = {
                //static cast because the types dont match
                .length = static_cast<uint8_t>(wfx_rsi.sec.ssid_length),
            },
            .channel = {
                .channel = SL_WIFI_AUTO_CHANNEL,
                .band = SL_WIFI_AUTO_BAND,
                .bandwidth = SL_WIFI_AUTO_BANDWIDTH
            },
            .bssid = {{0}},
            .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
            .security = security,
            .encryption = SL_WIFI_NO_ENCRYPTION,
            .client_options = SL_WIFI_JOIN_WITH_SCAN,
            .credential_id = SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID,
        },
        .ip = {
            .mode = SL_IP_MANAGEMENT_DHCP,
            .type = SL_IPV6,
            .host_name = NULL,
            .ip = {{{0}}},
        }
    };
    // TODO: memcpy for now since the types dont match
    memcpy((char *) &profile.config.ssid.value, wfx_rsi.sec.ssid, wfx_rsi.sec.ssid_length);

    status = sl_net_set_profile((sl_net_interface_t) SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
    VerifyOrReturnError(status == SL_STATUS_OK, status, ChipLogError(DeviceLayer, "sl_net_set_profile Failed"));

    return status;
}

sl_status_t JoinWifiNetwork(void)
{
    VerifyOrReturnError(!wfx_rsi.dev_state.HasAny(WifiState::kStationConnecting, WifiState::kStationConnected),
                        SL_STATUS_IN_PROGRESS);
    sl_status_t status = SL_STATUS_OK;

    // Start Join Network
    wfx_rsi.dev_state.Set(WifiState::kStationConnecting);

    status = SetWifiConfigurations();
    VerifyOrReturnError(status == SL_STATUS_OK, status, ChipLogError(DeviceLayer, "Failure to set the Wifi Configurations!"));

    status = sl_net_up((sl_net_interface_t) SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);

    if (status == SL_STATUS_OK || status == SL_STATUS_IN_PROGRESS)
    {
        WifiPlatformEvent event = WifiPlatformEvent::kStationConnect;
        sl_matter_wifi_post_event(event);
        return status;
    }

    // failure only happens when the firmware returns an error
    ChipLogError(DeviceLayer, "sl_wifi_connect failed: 0x%lx", static_cast<uint32_t>(status));
    VerifyOrReturnError((wfx_rsi.join_retries <= MAX_JOIN_RETRIES_COUNT), status);

    wfx_rsi.dev_state.Clear(WifiState::kStationConnecting).Clear(WifiState::kStationConnected);

    ChipLogProgress(DeviceLayer, "Connection retry attempt %d", wfx_rsi.join_retries);
    wfx_retry_connection(++wfx_rsi.join_retries);

    WifiPlatformEvent event = WifiPlatformEvent::kStationStartJoin;
    sl_matter_wifi_post_event(event);

    return status;
}

} // namespace

/**
 * @brief Wifi initialization called from app main
 *
 * @return sl_status_t Returns underlying Wi-Fi initialization error
 */
sl_status_t sl_matter_wifi_platform_init(void)
{
    sl_status_t status = SL_STATUS_OK;

    status = sl_net_init((sl_net_interface_t) SL_NET_WIFI_CLIENT_INTERFACE, &config, &wifi_client_context, nullptr);
    VerifyOrReturnError(status == SL_STATUS_OK, status, ChipLogError(DeviceLayer, "sl_net_init failed: %lx", status));

    // Create Sempaphore for scan completion
    sScanCompleteSemaphore = osSemaphoreNew(1, 0, nullptr);
    VerifyOrReturnError(sScanCompleteSemaphore != nullptr, SL_STATUS_ALLOCATION_FAILED);

    // Create Semaphore for scan in-progress protection
    sScanInProgressSemaphore = osSemaphoreNew(1, 1, nullptr);
    VerifyOrReturnError(sScanCompleteSemaphore != nullptr, SL_STATUS_ALLOCATION_FAILED);

    // Create the message queue
    sWifiEventQueue = osMessageQueueNew(kWfxQueueSize, sizeof(WifiPlatformEvent), nullptr);
    VerifyOrReturnError(sWifiEventQueue != nullptr, SL_STATUS_ALLOCATION_FAILED);

    // Create timer for DHCP polling
    // TODO: Use LWIP timer instead of creating a new one here
    sDHCPTimer = osTimerNew(DHCPTimerEventHandler, osTimerPeriodic, nullptr, nullptr);
    VerifyOrReturnError(sDHCPTimer != nullptr, SL_STATUS_ALLOCATION_FAILED);

    return status;
}

/******************************************************************
 * @fn   int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t *ap)
 * @brief
 *       Getting the AP details
 * @param[in] ap: access point
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_get_ap_info(wfx_wifi_scan_result_t * ap)
{
    // TODO: Convert this to a int8
    int32_t rssi = 0;
    ap->security = wfx_rsi.sec.security;
    ap->chan     = wfx_rsi.ap_chan;

    chip::MutableByteSpan output(ap->ssid, WFX_MAX_SSID_LENGTH);
    // Cast is a workaround until the wfx_rsi structure is refactored
    chip::ByteSpan ssid(reinterpret_cast<uint8_t *>(wfx_rsi.sec.ssid), wfx_rsi.sec.ssid_length);
    chip::CopySpanToMutableSpan(ssid, output);
    ap->ssid_length = output.size();

    chip::ByteSpan apMacSpan(wfx_rsi.ap_mac.data(), wfx_rsi.ap_mac.size());
    chip::MutableByteSpan bssidSpan(ap->bssid, kWifiMacAddressLength);
    chip::CopySpanToMutableSpan(apMacSpan, bssidSpan);

    // TODO: add error processing
    sl_wifi_get_signal_strength(SL_WIFI_CLIENT_INTERFACE, &(rssi));
    ap->rssi = rssi;

    return SL_STATUS_OK;
}

/******************************************************************
 * @fn   int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t *extra_info)
 * @brief
 *       Getting the AP extra details
 * @param[in] extra info: access point extra information
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_get_ap_ext(wfx_wifi_scan_ext_t * extra_info)
{
    sl_status_t status        = SL_STATUS_OK;
    sl_wifi_statistics_t test = { 0 };
    status                    = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VERIFY_STATUS_AND_RETURN(status);
    extra_info->beacon_lost_count = test.beacon_lost_count - temp_reset.beacon_lost_count;
    extra_info->beacon_rx_count   = test.beacon_rx_count - temp_reset.beacon_rx_count;
    extra_info->mcast_rx_count    = test.mcast_rx_count - temp_reset.mcast_rx_count;
    extra_info->mcast_tx_count    = test.mcast_tx_count - temp_reset.mcast_tx_count;
    extra_info->ucast_rx_count    = test.ucast_rx_count - temp_reset.ucast_rx_count;
    extra_info->ucast_tx_count    = test.ucast_tx_count - temp_reset.ucast_tx_count;
    extra_info->overrun_count     = test.overrun_count - temp_reset.overrun_count;
    return status;
}

/******************************************************************
 * @fn   int32_t wfx_rsi_reset_count(void)
 * @brief
 *       Getting the driver reset count
 * @param[in] None
 * @return
 *        status
 *********************************************************************/
int32_t wfx_rsi_reset_count(void)
{
    sl_wifi_statistics_t test = { 0 };
    sl_status_t status        = SL_STATUS_OK;
    status                    = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VERIFY_STATUS_AND_RETURN(status);
    temp_reset.beacon_lost_count = test.beacon_lost_count;
    temp_reset.beacon_rx_count   = test.beacon_rx_count;
    temp_reset.mcast_rx_count    = test.mcast_rx_count;
    temp_reset.mcast_tx_count    = test.mcast_tx_count;
    temp_reset.ucast_rx_count    = test.ucast_rx_count;
    temp_reset.ucast_tx_count    = test.ucast_tx_count;
    temp_reset.overrun_count     = test.overrun_count;
    return status;
}

/******************************************************************
 * @fn   sl_wifi_platform_disconnect(void)
 * @brief
 *       Getting the driver disconnect status
 * @param[in] None
 * @return
 *        status
 *********************************************************************/
int32_t sl_wifi_platform_disconnect(void)
{
    return sl_net_down((sl_net_interface_t) SL_NET_WIFI_CLIENT_INTERFACE);
}

/// NotifyConnectivity
/// @brief Notify the application about the connectivity status if it has not been notified yet.
///        Helper function for HandleDHCPPolling.
void NotifyConnectivity(void)
{
    VerifyOrReturn(!hasNotifiedWifiConnectivity);
    NotifyConnection(wfx_rsi.ap_mac);
    hasNotifiedWifiConnectivity = true;
}

void HandleDHCPPolling(void)
{
    WifiPlatformEvent event;

    // TODO: Notify the application that the interface is not set up or Chipdie here because we are in an unkonwn state
    struct netif * sta_netif = &wifi_client_context.netif;
    VerifyOrReturn(sta_netif != nullptr, ChipLogError(DeviceLayer, "HandleDHCPPolling: failed to get STA netif"));

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    uint8_t dhcp_state = dhcpclient_poll(sta_netif);
    if (dhcp_state == DHCP_ADDRESS_ASSIGNED && !hasNotifiedIPV4)
    {
        wfx_dhcp_got_ipv4((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
        hasNotifiedIPV4 = true;
        event           = WifiPlatformEvent::kStationDhcpDone;
        sl_matter_wifi_post_event(event);
        NotifyConnectivity();
    }
    else if (dhcp_state == DHCP_OFF)
    {
        NotifyIPv4Change(false);
        hasNotifiedIPV4 = false;
    }
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
    /* Checks if the assigned IPv6 address is preferred by evaluating
     * the first block of IPv6 address ( block 0)
     */
    if ((ip6_addr_ispreferred(netif_ip6_addr_state(sta_netif, 0))) && !hasNotifiedIPV6)
    {
        char addrStr[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
        VerifyOrReturn(ip6addr_ntoa_r(netif_ip6_addr(sta_netif, 0), addrStr, sizeof(addrStr)) != nullptr);
        ChipLogProgress(DeviceLayer, "SLAAC OK: linklocal addr: %s", addrStr);
        NotifyIPv6Change(true);
        hasNotifiedIPV6 = true;
        event           = WifiPlatformEvent::kStationDhcpDone;
        sl_matter_wifi_post_event(event);
        NotifyConnectivity();
    }
}
void sl_matter_wifi_post_event(WifiPlatformEvent event)
{
    sl_status_t status = osMessageQueuePut(sWifiEventQueue, &event, 0, 0);

    if (status != osOK)
    {
        ChipLogError(DeviceLayer, "sl_matter_wifi_post_event: failed to post event with status: %ld", status);
        // TODO: Handle error, requeue event depending on queue size or notify relevant task,
        // Chipdie, etc.
    }
}
/// ResetDHCPNotificationFlags
/// @brief Reset the flags that are used to notify the application about DHCP connectivity
///        and emits a WifiPlatformEvent::kStationDoDhcp event to trigger DHCP polling checks. Helper function for ProcessEvent.
void ResetDHCPNotificationFlags(void)
{

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    hasNotifiedIPV4 = false;
#endif // CHIP_DEVICE_CONFIG_ENABLE_IPV4
    hasNotifiedIPV6             = false;
    hasNotifiedWifiConnectivity = false;

    WifiPlatformEvent event = WifiPlatformEvent::kStationDoDhcp;
    sl_matter_wifi_post_event(event);
}

void ProcessEvent(WifiPlatformEvent event)
{
    // Process event
    switch (event)
    {
    case WifiPlatformEvent::kStationConnect:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationConnect");
        wfx_rsi.dev_state.Set(WifiState::kStationConnected);
        ResetDHCPNotificationFlags();
        break;

    case WifiPlatformEvent::kStationDisconnect: {
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationDisconnect");
        // TODO: This event is not being posted anywhere, seems to be a dead code or we are missing something

        wfx_rsi.dev_state.Clear(WifiState::kStationReady)
            .Clear(WifiState::kStationConnecting)
            .Clear(WifiState::kStationConnected)
            .Clear(WifiState::kStationDhcpDone);

        /* TODO: Implement disconnect notify */
        ResetDHCPNotificationFlags();
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
        NotifyIPv4Change(false);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
        NotifyIPv6Change(false);
    }
    break;

    case WifiPlatformEvent::kAPStart:
        // TODO: Currently unimplemented
        break;

    case WifiPlatformEvent::kScan:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kScan");
        if (!(wfx_rsi.dev_state.Has(WifiState::kScanStarted)))
        {
            ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kScan");
            sl_status_t status = SL_STATUS_OK;

            sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;
            if (wfx_rsi.dev_state.Has(WifiState::kStationConnected))
            {
                /* Terminate with end of scan which is no ap sent back */
                wifi_scan_configuration.type                   = SL_WIFI_SCAN_TYPE_ADV_SCAN;
                wifi_scan_configuration.periodic_scan_interval = kAdvScanPeriodicity;
            }

            sl_wifi_advanced_scan_configuration_t advanced_scan_configuration = {
                .trigger_level        = kAdvScanThreshold,
                .trigger_level_change = kAdvRssiToleranceThreshold,
                .active_channel_time  = kAdvActiveScanDuration,
                .passive_channel_time = kAdvPassiveScanDuration,
                .enable_instant_scan  = kAdvEnableInstantbgScan,
                .enable_multi_probe   = kAdvMultiProbe,
            };

            status = sl_wifi_set_advanced_scan_configuration(&advanced_scan_configuration);

            // TODO: Seems like Chipdie should be called here, the device should be initialized here
            VerifyOrReturn(
                status == SL_STATUS_OK,
                ChipLogError(DeviceLayer, "sl_wifi_set_advanced_scan_configuration failed: 0x%lx", static_cast<uint32_t>(status)));

            sl_wifi_set_scan_callback(BackgroundScanCallback, nullptr);
            wfx_rsi.dev_state.Set(WifiState::kScanStarted);

            // If an ssid was not provided, we need to call the scan API with nullptr to scan all Wi-Fi networks
            sl_wifi_ssid_t ssid      = { 0 };
            sl_wifi_ssid_t * ssidPtr = nullptr;

            if (wfx_rsi.scan_ssid != nullptr)
            {
                chip::ByteSpan requestedSsid(wfx_rsi.scan_ssid, wfx_rsi.scan_ssid_length);
                chip::MutableByteSpan ouputSsid(ssid.value, sizeof(ssid.value));
                chip::CopySpanToMutableSpan(requestedSsid, ouputSsid);

                ssid.length = ouputSsid.size();
                ssidPtr     = &ssid;
            }

            osSemaphoreAcquire(sScanInProgressSemaphore, osWaitForever);
            status = sl_wifi_start_scan(SL_WIFI_CLIENT_2_4GHZ_INTERFACE, ssidPtr, &wifi_scan_configuration);
            if (SL_STATUS_IN_PROGRESS == status)
            {
                osSemaphoreAcquire(sScanCompleteSemaphore, kWifiScanTimeoutTicks);
            }

            osSemaphoreRelease(sScanInProgressSemaphore);
        }
        break;

    case WifiPlatformEvent::kStationStartJoin:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationStartJoin");

        InitiateScan();
        JoinWifiNetwork();
        break;

    case WifiPlatformEvent::kStationDoDhcp:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationDoDhcp");
        StartDHCPTimer(WFX_RSI_DHCP_POLL_INTERVAL);
        break;

    case WifiPlatformEvent::kStationDhcpDone:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationDhcpDone");
        CancelDHCPTimer();
        break;

    case WifiPlatformEvent::kStationDhcpPoll:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationDhcpPoll");
        HandleDHCPPolling();
        break;

    default:
        break;
    }
}

/*********************************************************************************
 * @fn  void sl_matter_wifi_task(void *arg)
 * @brief
 * The main WLAN task - started by StartWifiTask() that interfaces with RSI.
 * The rest of RSI stuff come in call-backs.
 * The initialization has been already done.
 * @param[in] arg:
 * @return
 *       None
 **********************************************************************************/
/* ARGSUSED */
void sl_matter_wifi_task(void * arg)
{
    (void) arg;
    WifiPlatformEvent event;
    sl_status_t status = SL_STATUS_OK;

    status = sl_wifi_siwx917_init();
    VerifyOrReturn(
        status == SL_STATUS_OK,
        ChipLogError(DeviceLayer, "sl_matter_wifi_task: sl_wifi_siwx917_init failed: 0x%lx", static_cast<uint32_t>(status)));

    sl_matter_wifi_task_started();

    ChipLogDetail(DeviceLayer, "sl_matter_wifi_task: starting event loop");
    for (;;)
    {
        if (osMessageQueueGet(sWifiEventQueue, &event, nullptr, osWaitForever) == osOK)
        {
            ProcessEvent(event);
        }
        else
        {
            ChipLogError(DeviceLayer, "sl_matter_wifi_task: get event failed: 0x%lx", static_cast<uint32_t>(status));
        }
    }
}

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/********************************************************************************************
 * @fn   void wfx_dhcp_got_ipv4(uint32_t ip)
 * @brief
 *        Acquire the new ip address
 * @param[in] ip: internet protocol
 * @return
 *        None
 **********************************************************************************************/
void wfx_dhcp_got_ipv4(uint32_t ip)
{
    /*
     * Acquire the new IP address
     */
    wfx_rsi.ip4_addr[0] = (ip) &0xFF;
    wfx_rsi.ip4_addr[1] = (ip >> 8) & 0xFF;
    wfx_rsi.ip4_addr[2] = (ip >> 16) & 0xFF;
    wfx_rsi.ip4_addr[3] = (ip >> 24) & 0xFF;
    ChipLogDetail(DeviceLayer, "DHCP OK: IP=%d.%d.%d.%d", wfx_rsi.ip4_addr[0], wfx_rsi.ip4_addr[1], wfx_rsi.ip4_addr[2],
                  wfx_rsi.ip4_addr[3]);
    /* Notify the Connectivity Manager - via the app */
    wfx_rsi.dev_state.Set(WifiState::kStationDhcpDone).Set(WifiState::kStationReady);
    NotifyIPv4Change(true);
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

#if SL_ICD_ENABLED
/*********************************************************************
 * @fn  sl_status_t wfx_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t
 sl_si91x_wifi_state)
 * @brief
 *      Implements the power save in sleepy application
 * @param[in]  sl_si91x_ble_state : State to set for the BLE
               sl_si91x_wifi_state : State to set for the WiFi
 * @return  SL_STATUS_OK if successful,
 *          SL_STATUS_FAIL otherwise
 ***********************************************************************/
sl_status_t wfx_power_save(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state)
{
    int32_t error = rsi_bt_power_save_profile(sl_si91x_ble_state, 0);
    if (error != RSI_SUCCESS)
    {
        ChipLogError(DeviceLayer, "rsi_bt_power_save_profile failed: %ld", error);
        return SL_STATUS_FAIL;
    }

    sl_wifi_performance_profile_t wifi_profile = { .profile = sl_si91x_wifi_state };
    sl_status_t status                         = sl_wifi_set_performance_profile(&wifi_profile);
    if (status != SL_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "sl_wifi_set_performance_profile failed: 0x%lx", static_cast<uint32_t>(status));
        return status;
    }

    return SL_STATUS_OK;
}
#endif
