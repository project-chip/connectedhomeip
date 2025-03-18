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

#if (SL_MATTER_GN_BUILD == 0)
#include "sl_matter_wifi_config.h"
#endif // SL_MATTER_GN_BUILD

#include "ble_config.h"
#include "sl_status.h"
#include "sl_wifi_device.h"
#include <app/icd/server/ICDServerConfig.h>
#include <inet/IPAddress.h>
#include <lib/support/CHIPMem.h>
#include <platform/silabs/wifi/SiWx/WifiInterfaceImpl.h>

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

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDConfigurationData.h> // nogncheck
#include <platform/silabs/wifi/icd/WifiSleepManager.h>

#if SLI_SI91X_MCU_INTERFACE // SoC Only
#include "rsi_rom_power_save.h"
#include "sl_gpio_board.h"
#include "sl_si91x_driver_gpio.h"
#include "sl_si91x_power_manager.h"
#endif // SLI_SI91X_MCU_INTERFACE
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

using namespace chip::DeviceLayer::Silabs;

// TODO : Temporary work-around for wifi-init failure in 917NCP ACX module boards.
// Can be removed after Wiseconnect fixes region code for all ACX module boards.
#if defined(EXP_BOARD)
#define REGION_CODE IGNORE_REGION
#else
#define REGION_CODE US
#endif

// TODO: This needs to be refactored so we don't need the global object
WfxRsi_t wfx_rsi;

namespace {

#if CHIP_CONFIG_ENABLE_ICD_SERVER
constexpr uint32_t kTimeToFullBeaconReception = 5000; // 5 seconds
#endif                                                // CHIP_CONFIG_ENABLE_ICD_SERVER

wfx_wifi_scan_ext_t temp_reset;

osSemaphoreId_t sScanCompleteSemaphore;
osSemaphoreId_t sScanInProgressSemaphore;

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
    wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kScanStarted);
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

sl_status_t SiWxPlatformInit(void)
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

    wfx_rsi.dev_state.Set(WifiInterface::WifiState::kStationInit);
    return status;
}

sl_status_t ScanCallback(sl_wifi_event_t event, sl_wifi_scan_result_t * scan_result, uint32_t result_length, void * arg)
{
    sl_status_t status = SL_STATUS_OK;
    if (SL_WIFI_CHECK_IF_EVENT_FAILED(event))
    {
        if (scan_result != nullptr)
        {
            status = *reinterpret_cast<sl_status_t *>(scan_result);
            ChipLogError(DeviceLayer, "ScanCallback: failed: 0x%lx", status);
        }

#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION
        security = SL_WIFI_WPA3;
#else
        security = SL_WIFI_WPA_WPA2_MIXED;
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
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

sl_status_t InitiateScan()
{
    sl_status_t status                                   = SL_STATUS_OK;
    sl_wifi_ssid_t ssid                                  = { 0 };
    sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;

    ssid.length = wfx_rsi.credentials.ssidLength;

    chip::ByteSpan requestedSsidSpan(wfx_rsi.credentials.ssid, wfx_rsi.credentials.ssidLength);
    chip::MutableByteSpan ssidSpan(ssid.value, ssid.length);
    chip::CopySpanToMutableSpan(requestedSsidSpan, ssidSpan);

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

    return status;
}

sl_status_t SetWifiConfigurations()
{
    sl_status_t status = SL_STATUS_OK;

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    sl_wifi_listen_interval_t sleep_interval = { .listen_interval =
                                                     chip::ICDConfigurationData::GetInstance().GetSlowPollingInterval().count() };
    status                                   = sl_wifi_set_listen_interval(SL_WIFI_CLIENT_INTERFACE, sleep_interval);
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_wifi_set_listen_interval failed: 0x%lx", status));

    // This is be triggered on the disconnect use case, providing the amount of TA tries
    // Setting the TA retry to 1 and giving the control to the M4 for improved power efficiency
    // When max_retry_attempts is set to 0, TA will retry indefinitely.
    sl_wifi_advanced_client_configuration_t client_config = { .max_retry_attempts = 1 };
    status = sl_wifi_set_advanced_client_configuration(SL_WIFI_CLIENT_INTERFACE, &client_config);
    VerifyOrReturnError(status == SL_STATUS_OK, status,
                        ChipLogError(DeviceLayer, "sl_wifi_set_advanced_client_configuration failed: 0x%lx", status));

    status = sl_si91x_set_join_configuration(
        SL_WIFI_CLIENT_INTERFACE, (SL_SI91X_JOIN_FEAT_LISTEN_INTERVAL_VALID | SL_SI91X_JOIN_FEAT_PS_CMD_LISTEN_INTERVAL_VALID));
    VerifyOrReturnError(status == SL_STATUS_OK, status);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    if (wfx_rsi.credentials.passkeyLength != 0)
    {
        status = sl_net_set_credential(SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID, SL_NET_WIFI_PSK, &wfx_rsi.credentials.passkey[0],
                                       wfx_rsi.credentials.passkeyLength);
        VerifyOrReturnError(status == SL_STATUS_OK, status,
                            ChipLogError(DeviceLayer, "sl_net_set_credential failed: 0x%lx", status));
    }

    sl_net_wifi_client_profile_t profile = {
        .config = {
            .ssid = {
                .value  = { 0 },
                //static cast because the types dont match
                .length = static_cast<uint8_t>(wfx_rsi.credentials.ssidLength),
            },
            .channel = {
                .channel = SL_WIFI_AUTO_CHANNEL,
                .band = SL_WIFI_AUTO_BAND,
                .bandwidth = SL_WIFI_AUTO_BANDWIDTH
            },
            .bssid = {{0}},
            .bss_type = SL_WIFI_BSS_TYPE_INFRASTRUCTURE,
            .security = security,
            .encryption = SL_WIFI_DEFAULT_ENCRYPTION,
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

    chip::MutableByteSpan output(profile.config.ssid.value, WFX_MAX_SSID_LENGTH);
    chip::ByteSpan input(wfx_rsi.credentials.ssid, wfx_rsi.credentials.ssidLength);
    chip::CopySpanToMutableSpan(input, output);

    status = sl_net_set_profile(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID, &profile);
    VerifyOrReturnError(status == SL_STATUS_OK, status, ChipLogError(DeviceLayer, "sl_net_set_profile failed: 0x%lx", status));

    return status;
}

} // namespace

namespace chip {
namespace DeviceLayer {
namespace Silabs {

WifiInterfaceImpl WifiInterfaceImpl::mInstance;

WifiInterface & WifiInterface::GetInstance()
{
    return WifiInterfaceImpl::GetInstance();
}

WiseconnectWifiInterface & WiseconnectWifiInterface::GetInstance()
{
    return WifiInterfaceImpl::GetInstance();
}

void WiseconnectWifiInterface::MatterWifiTask(void * arg)
{
    (void) arg;
    WiseconnectWifiInterface::WifiPlatformEvent event;
    sl_status_t status = SL_STATUS_OK;

    status = SiWxPlatformInit();
    VerifyOrReturn(status == SL_STATUS_OK,
                   ChipLogError(DeviceLayer, "MatterWifiTask: SiWxPlatformInit failed: 0x%lx", static_cast<uint32_t>(status)));

    WifiInterfaceImpl::GetInstance().NotifyWifiTaskInitialized();

    ChipLogDetail(DeviceLayer, "MatterWifiTask: starting event loop");
    for (;;)
    {
        if (osMessageQueueGet(sWifiEventQueue, &event, nullptr, osWaitForever) == osOK)
        {
            WifiInterfaceImpl::GetInstance().ProcessEvent(event);
        }
        else
        {
            ChipLogError(DeviceLayer, "MatterWifiTask: get event failed: 0x%lx", static_cast<uint32_t>(status));
        }
    }
}

CHIP_ERROR WifiInterfaceImpl::InitWiFiStack(void)
{
    sl_status_t status = SL_STATUS_OK;

    status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &config, &wifi_client_context, nullptr);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL, ChipLogError(DeviceLayer, "sl_net_init failed: %lx", status));

    // Create Sempaphore for scan completion
    sScanCompleteSemaphore = osSemaphoreNew(1, 0, nullptr);
    VerifyOrReturnError(sScanCompleteSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);

    // Create Semaphore for scan in-progress protection
    sScanInProgressSemaphore = osSemaphoreNew(1, 1, nullptr);
    VerifyOrReturnError(sScanCompleteSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);

    // Create the message queue
    sWifiEventQueue = osMessageQueueNew(kWfxQueueSize, sizeof(WiseconnectWifiInterface::WifiPlatformEvent), nullptr);
    VerifyOrReturnError(sWifiEventQueue != nullptr, CHIP_ERROR_NO_MEMORY);

    return CHIP_NO_ERROR;
}

void WifiInterfaceImpl::ProcessEvent(WiseconnectWifiInterface::WifiPlatformEvent event)
{
    switch (event)
    {
    case WiseconnectWifiInterface::WifiPlatformEvent::kStationConnect:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationConnect");
        wfx_rsi.dev_state.Set(WifiInterface::WifiState::kStationConnected);
        ResetConnectivityNotificationFlags();
        break;

    case WiseconnectWifiInterface::WifiPlatformEvent::kStationDisconnect: {
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationDisconnect");
        // TODO: This event is not being posted anywhere, seems to be a dead code or we are missing something

        wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationReady)
            .Clear(WifiInterface::WifiState::kStationConnecting)
            .Clear(WifiInterface::WifiState::kStationConnected);

        // TODO: Implement disconnect notify
        ResetConnectivityNotificationFlags();
#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
        NotifyIPv4Change(false);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */
        NotifyIPv6Change(false);
    }
    break;

    case WiseconnectWifiInterface::WifiPlatformEvent::kAPStart:
        // TODO: Currently unimplemented
        break;

    case WiseconnectWifiInterface::WifiPlatformEvent::kScan:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kScan");
        if (!(wfx_rsi.dev_state.Has(WifiInterface::WifiState::kScanStarted)))
        {
            ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kScan");
            sl_status_t status = SL_STATUS_OK;

            sl_wifi_scan_configuration_t wifi_scan_configuration = default_wifi_scan_configuration;
            if (wfx_rsi.dev_state.Has(WifiInterface::WifiState::kStationConnected))
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
            wfx_rsi.dev_state.Set(WifiInterface::WifiState::kScanStarted);

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

    case WiseconnectWifiInterface::WifiPlatformEvent::kStationStartJoin:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kStationStartJoin");

        InitiateScan();
        JoinWifiNetwork();
        break;

    case WiseconnectWifiInterface::WifiPlatformEvent::kConnectionComplete:
        ChipLogDetail(DeviceLayer, "WifiPlatformEvent::kConnectionComplete");
        NotifySuccessfulConnection();

    default:
        break;
    }
}

void WifiInterfaceImpl::NotifySuccessfulConnection(void)
{
    struct netif * sta_netif = &wifi_client_context.netif;
    VerifyOrReturn(sta_netif != nullptr, ChipLogError(DeviceLayer, "NotifySuccessfulConnection: failed to get STA netif"));

#if (CHIP_DEVICE_CONFIG_ENABLE_IPV4)
    GotIPv4Address((uint32_t) sta_netif->ip_addr.u_addr.ip4.addr);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

    char addrStr[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
    VerifyOrReturn(ip6addr_ntoa_r(netif_ip6_addr(sta_netif, 0), addrStr, sizeof(addrStr)) != nullptr);
    ChipLogProgress(DeviceLayer, "SLAAC OK: linklocal addr: %s", addrStr);
    NotifyIPv6Change(true);
    NotifyConnectivity();
}

sl_status_t WifiInterfaceImpl::JoinWifiNetwork(void)
{
    VerifyOrReturnError(
        !wfx_rsi.dev_state.HasAny(WifiInterface::WifiState::kStationConnecting, WifiInterface::WifiState::kStationConnected),
        SL_STATUS_IN_PROGRESS);
    sl_status_t status = SL_STATUS_OK;

    // Start Join Network
    wfx_rsi.dev_state.Set(WifiInterface::WifiState::kStationConnecting);

    status = SetWifiConfigurations();
    VerifyOrReturnError(status == SL_STATUS_OK, status, ChipLogError(DeviceLayer, "Failure to set the Wifi Configurations!"));

    status = sl_wifi_set_join_callback(JoinCallback, nullptr);
    VerifyOrReturnError(status == SL_STATUS_OK, status);

// To avoid IOP issues, it is recommended to enable high-performance mode before joining the network.
// TODO: Remove this once the IOP issue related to power save mode switching is fixed in the Wi-Fi SDK.
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    chip::DeviceLayer::Silabs::WifiSleepManager::GetInstance().RequestHighPerformance();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    status = sl_net_up(SL_NET_WIFI_CLIENT_INTERFACE, SL_NET_DEFAULT_WIFI_CLIENT_PROFILE_ID);

    if (status == SL_STATUS_OK || status == SL_STATUS_IN_PROGRESS)
    {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        // Remove High performance request that might have been added during the connect/retry process
        chip::DeviceLayer::Silabs::WifiSleepManager::GetInstance().RemoveHighPerformanceRequest();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

        WiseconnectWifiInterface::WifiPlatformEvent event = WiseconnectWifiInterface::WifiPlatformEvent::kStationConnect;
        PostWifiPlatformEvent(event);
        return status;
    }

    // failure only happens when the firmware returns an error
    ChipLogError(DeviceLayer, "sl_wifi_connect failed: 0x%lx", static_cast<uint32_t>(status));

    wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationConnecting).Clear(WifiInterface::WifiState::kStationConnected);
    ScheduleConnectionAttempt();

    return status;
}

sl_status_t WifiInterfaceImpl::JoinCallback(sl_wifi_event_t event, char * result, uint32_t resultLenght, void * arg)
{
    sl_status_t status = SL_STATUS_OK;
    wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationConnecting);
    if (SL_WIFI_CHECK_IF_EVENT_FAILED(event))
    {
        status = *reinterpret_cast<sl_status_t *>(result);
        ChipLogError(DeviceLayer, "JoinCallback: failed: 0x%lx", status);
        wfx_rsi.dev_state.Clear(WifiInterface::WifiState::kStationConnected);

        mInstance.ScheduleConnectionAttempt();
    }

    return status;
}

CHIP_ERROR WifiInterfaceImpl::GetAccessPointInfo(wfx_wifi_scan_result_t & info)
{
    // TODO: Convert this to a int8
    int32_t rssi  = 0;
    info.security = wfx_rsi.credentials.security;
    info.chan     = wfx_rsi.ap_chan;

    chip::MutableByteSpan output(info.ssid, WFX_MAX_SSID_LENGTH);
    chip::ByteSpan ssid(wfx_rsi.credentials.ssid, wfx_rsi.credentials.ssidLength);
    chip::CopySpanToMutableSpan(ssid, output);
    info.ssid_length = output.size();

    chip::ByteSpan apMacSpan(wfx_rsi.ap_mac.data(), wfx_rsi.ap_mac.size());
    chip::MutableByteSpan bssidSpan(info.bssid, kWifiMacAddressLength);
    chip::CopySpanToMutableSpan(apMacSpan, bssidSpan);

    // TODO: add error processing
    sl_wifi_get_signal_strength(SL_WIFI_CLIENT_INTERFACE, &(rssi));
    info.rssi = rssi;

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info)
{
    sl_wifi_statistics_t test = { 0 };

    sl_status_t status = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL);

    info.beacon_lost_count = test.beacon_lost_count - temp_reset.beacon_lost_count;
    info.beacon_rx_count   = test.beacon_rx_count - temp_reset.beacon_rx_count;
    info.mcast_rx_count    = test.mcast_rx_count - temp_reset.mcast_rx_count;
    info.mcast_tx_count    = test.mcast_tx_count - temp_reset.mcast_tx_count;
    info.ucast_rx_count    = test.ucast_rx_count - temp_reset.ucast_rx_count;
    info.ucast_tx_count    = test.ucast_tx_count - temp_reset.ucast_tx_count;
    info.overrun_count     = test.overrun_count - temp_reset.overrun_count;

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::ResetCounters()
{
    sl_wifi_statistics_t test = { 0 };

    sl_status_t status = sl_wifi_get_statistics(SL_WIFI_CLIENT_INTERFACE, &test);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL);

    temp_reset.beacon_lost_count = test.beacon_lost_count;
    temp_reset.beacon_rx_count   = test.beacon_rx_count;
    temp_reset.mcast_rx_count    = test.mcast_rx_count;
    temp_reset.mcast_tx_count    = test.mcast_tx_count;
    temp_reset.ucast_rx_count    = test.ucast_rx_count;
    temp_reset.ucast_tx_count    = test.ucast_tx_count;
    temp_reset.overrun_count     = test.overrun_count;

    return CHIP_NO_ERROR;
}

void WifiInterfaceImpl::PostWifiPlatformEvent(WiseconnectWifiInterface::WifiPlatformEvent event)
{
    sl_status_t status = osMessageQueuePut(sWifiEventQueue, &event, 0, 0);

    if (status != osOK)
    {
        ChipLogError(DeviceLayer, "PostWifiPlatformEvent: failed to post event with status: %ld", status);
        // TODO: Handle error, requeue event depending on queue size or notify relevant task,
        // Chipdie, etc.
    }
}

sl_status_t WifiInterfaceImpl::TriggerPlatformWifiDisconnection()
{
    return sl_net_down(SL_NET_WIFI_CLIENT_INTERFACE);
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
CHIP_ERROR WifiInterfaceImpl::ConfigurePowerSave(rsi_power_save_profile_mode_t sl_si91x_ble_state,
                                                 sl_si91x_performance_profile_t sl_si91x_wifi_state, uint32_t listenInterval)
{
    int32_t error = rsi_bt_power_save_profile(sl_si91x_ble_state, RSI_MAX_PSP);
    VerifyOrReturnError(error == RSI_SUCCESS, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "rsi_bt_power_save_profile failed: %ld", error));

    sl_wifi_performance_profile_t wifi_profile = { .profile = sl_si91x_wifi_state,
                                                   // TODO: Performance profile fails if not alligned with DTIM
                                                   .dtim_aligned_type = SL_SI91X_ALIGN_WITH_DTIM_BEACON,
                                                   // TODO: Different types need to be fixed in the Wi-Fi SDK
                                                   .listen_interval = static_cast<uint16_t>(listenInterval) };

    sl_status_t status = sl_wifi_set_performance_profile(&wifi_profile);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "sl_wifi_set_performance_profile failed: 0x%lx", status));

    return CHIP_NO_ERROR;
}

CHIP_ERROR WifiInterfaceImpl::ConfigureBroadcastFilter(bool enableBroadcastFilter)
{
    sl_status_t status = SL_STATUS_OK;

    uint16_t beaconDropThreshold = (enableBroadcastFilter) ? kTimeToFullBeaconReception : 0;
    uint8_t filterBcastInTim     = (enableBroadcastFilter) ? 1 : 0;

    status = sl_wifi_filter_broadcast(beaconDropThreshold, filterBcastInTim, 1 /* valid till next update*/);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "sl_wifi_filter_broadcast failed: 0x%lx", static_cast<uint32_t>(status)));

    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
