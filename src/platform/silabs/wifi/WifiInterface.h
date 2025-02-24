/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#pragma once

#include <app/icd/server/ICDServerConfig.h>
#include <array>
#include <cmsis_os2.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Span.h>
#include <platform/silabs/wifi/wfx_msgs.h>
#include <sl_cmsis_os2_common.h>

#include "sl_status.h"
#include <stdbool.h>

#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#include "rsi_common_apis.h"
#include "sl_si91x_types.h"
#include "sl_wifi_constants.h"
#include "sl_wifi_device.h"
#endif // (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)

/* Updated constants */

constexpr size_t kWifiMacAddressLength = 6;

/* Defines to update */

// TODO: Not sure why the pass key max length differs for the 917 SoC & NCP
#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
// MAX PASSKEY LENGTH including NULL character
#define WFX_MAX_PASSKEY_LENGTH (SL_WIFI_MAX_PSK_LENGTH)
#else
// MAX PASSKEY LENGTH including NULL character
#define WFX_MAX_PASSKEY_LENGTH (64)
#endif // (SLI_SI91X_MCU_INTERFACE  | EXP_BOARD)

// MAX SSID LENGTH excluding NULL character
#define WFX_MAX_SSID_LENGTH (32)
#define MAX_JOIN_RETRIES_COUNT (5)

/* Updated types */

using MacAddress = std::array<uint8_t, kWifiMacAddressLength>;

enum class WifiEvent : uint8_t
{
    kStartUp      = 0,
    kConnect      = 1,
    kDisconnect   = 2,
    kScanComplete = 3,
    kGotIPv4      = 4,
    kGotIPv6      = 5,
    kLostIP       = 6,
};

enum class WifiState : uint16_t
{
    kStationInit        = (1 << 0),
    kAPReady            = (1 << 1),
    kStationProvisioned = (1 << 2),
    kStationConnecting  = (1 << 3),
    kStationConnected   = (1 << 4),
    kStationDhcpDone    = (1 << 6), /* Requested to do DHCP after conn */
    kStationMode        = (1 << 7), /* Enable Station Mode */
    kAPMode             = (1 << 8), /* Enable AP Mode */
    kStationReady       = (kStationConnected | kStationDhcpDone),
    kStationStarted     = (1 << 9),
    kScanStarted        = (1 << 10), /* Scan Started */
};

enum class WifiDisconnectionReasons : uint16_t // using uint16 to match current structure during the transition
{
    kUnknownError      = 1, // Disconnation due to an internal error
    kAccessPointLost   = 2, // Device did not receive AP beacon too many times
    kAccessPoint       = 3, // AP disconnected the device
    kApplication       = 4, // Application requested disconnection
    kWPACouterMeasures = 5, // WPA contermeasures triggered a disconnection
};

/* Enums to update */

/* Note that these are same as RSI_security */
typedef enum
{
    WFX_SEC_UNSPECIFIED    = 0,
    WFX_SEC_NONE           = 1,
    WFX_SEC_WEP            = 2,
    WFX_SEC_WPA            = 3,
    WFX_SEC_WPA2           = 4,
    WFX_SEC_WPA3           = 5,
    WFX_SEC_WPA_WPA2_MIXED = 6,
} wfx_sec_t;

typedef struct wfx_wifi_scan_result
{
    uint8_t ssid[WFX_MAX_SSID_LENGTH]; // excludes null-character
    size_t ssid_length;
    wfx_sec_t security;
    uint8_t bssid[kWifiMacAddressLength];
    uint8_t chan;
    int16_t rssi; /* I suspect this is in dBm - so signed */
} wfx_wifi_scan_result_t;
using ScanCallback = void (*)(wfx_wifi_scan_result_t *);

typedef struct wfx_wifi_scan_ext
{
    uint32_t beacon_lost_count;
    uint32_t beacon_rx_count;
    uint32_t mcast_rx_count;
    uint32_t mcast_tx_count;
    uint32_t ucast_rx_count;
    uint32_t ucast_tx_count;
    uint32_t overrun_count;
} wfx_wifi_scan_ext_t;

#ifdef RS911X_WIFI
/*
 * This Sh%t is here to support WFXUtils - and the Matter stuff that uses it
 * We took it from the SDK (for WF200)
 */
typedef enum
{
    SL_WFX_STA_INTERFACE    = 0, ///< Interface 0, linked to the station
    SL_WFX_SOFTAP_INTERFACE = 1, ///< Interface 1, linked to the softap
} sl_wfx_interface_t;
#endif

// TODO: Figure out if we need this structure. We have different strcutures for the same use
struct WifiCredentials
{
    std::array<uint8_t, WFX_MAX_SSID_LENGTH> ssid       = { 0 };
    size_t ssidLength                                   = 0;
    std::array<uint8_t, WFX_MAX_PASSKEY_LENGTH> passkey = { 0 };
    size_t passkeyLength                                = 0;
    wfx_sec_t security                                  = WFX_SEC_UNSPECIFIED;

    WifiCredentials & operator=(const WifiCredentials & other)
    {
        if (this != &other)
        {
            ssid          = other.ssid;
            ssidLength    = other.ssidLength;
            passkey       = other.passkey;
            passkeyLength = other.passkeyLength;
            security      = other.security;
        }
        return *this;
    }

    void Clear()
    {
        ssid.fill(0);
        ssidLength = 0;
        passkey.fill(0);
        passkeyLength = 0;
        security      = WFX_SEC_UNSPECIFIED;
    }
};

typedef struct wfx_rsi_s
{
    chip::BitFlags<WifiState> dev_state;
    uint16_t ap_chan; /* The chan our STA is using	*/
    WifiCredentials credentials;
    ScanCallback scan_cb;
    uint8_t * scan_ssid; /* Which one are we scanning for */
    size_t scan_ssid_length;
#ifdef SL_WFX_CONFIG_SOFTAP
    MacAddress softap_mac;
#endif
    MacAddress sta_mac;
    MacAddress ap_mac;   /* To which our STA is connected */
    MacAddress ap_bssid; /* To which our STA is connected */
    uint8_t ip4_addr[4]; /* Not sure if this is enough */
} WfxRsi_t;

/* Updated functions */

/**
 * @brief Function initalizes the WiFi module before starting WiFi task.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR, if the initialization succeeded
 *                    CHIP_ERROR_INTERNAL, if sequence failed due to internal API error
 *                    CHIP_ERROR_NO_MEMORY, if sequence failed due to unavaliablility of memory
 */

CHIP_ERROR InitWiFiStack(void);

/**
 * @brief Function notifies the PlatformManager that an IPv6 event occured on the WiFi interface.
 *
 * @param gotIPv6Addr true, got an IPv6 address
 *                    false, lost or wasn't able to get an IPv6 address
 */
void NotifyIPv6Change(bool gotIPv6Addr);

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/**
 * @brief Function notifies the PlatformManager that an IPv4 event occured on the WiFi interface.
 *
 * @param gotIPv4Addr true, got an IPv4 address
 *                    false, lost or wasn't able to get an IPv4 address
 */
void NotifyIPv4Change(bool gotIPv4Addr);
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/**
 * @brief Function notifies the PlatformManager that a disconnection event occurred
 *
 * @param reason reason for the disconnection
 */
void NotifyDisconnection(WifiDisconnectionReasons reason);

/**
 * @brief Function notifies the PlatformManager that a connection event occurred
 *
 * @param[in] ap pointer to the structure that contains the MAC address of the AP
 */
void NotifyConnection(const MacAddress & ap);

/**
 * @brief Returns the IPv6 notification state
 *
 * @note This function is necessary because the class inheritance structure has not been done as of yet.
 *       Once the inheritance is done, sub-classes will have access to the member directly wihtout needing to use an extra guetter.
 *
 * @return true, IPv6 change has been notified
           false, otherwise
 */
bool HasNotifiedIPv6Change();

/**
 * @brief Returns the IPv4 notification state
 *
 * @note This function is necessary because the class inheritance structure has not been done as of yet.
 *       Once the inheritance is done, sub-classes will have access to the member directly wihtout needing to use an extra guetter.
 *
 * @return true, IPv4 change has been notified
           false, otherwise
 */
bool HasNotifiedIPv4Change();

/**
 * @brief Function resets the IP notification states
 *
 * * @note This function is necessary because the class inheritance structure has not been done as of yet.
 *       Once the inheritance is done, sub-classes will have access to the member directly wihtout needing to use an extra guetter.
 */
void ResetIPNotificationStates();

/**
 * @brief Returns the provide interfaces MAC address
 *        Valid buffer large enough for the MAC address must be provided to the function
 *
 * @param[in] interface SL_WFX_STA_INTERFACE or SL_WFX_SOFTAP_INTERFACE.
 *                      If soft AP is not enabled, the interface is ignored and the function always returns the Station MAC
 *                      address
 * @param[out] addr     Interface MAC addres
 *
 * @return CHIP_ERROR CHIP_NO_ERROR on success
 *                    CHIP_ERROR_BUFFER_TOO_SMALL if the provided ByteSpan size is too small
 *
 */
CHIP_ERROR GetMacAddress(sl_wfx_interface_t interface, chip::MutableByteSpan & addr);

/**
 * @brief Triggers a network scan
 *        The function is asynchronous and the result is provided via the callback.
 *
 * @param ssid The SSID to scan for. If empty, all networks are scanned
 * @param callback The callback to be called when the scan is complete. Cannot be nullptr.
 *                 The callback is called asynchrounously.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR if the network scan was successfully started
 *                    CHIP_INVALID_ARGUMENT if the callback is nullptr
 *                    CHIP_ERROR_IN_PROGRESS, if there is already a network scan in progress
 *                    CHIP_ERROR_INVALID_STRING_LENGTH, if there SSID length exceeds handled limit
 *                    other, if there is a platform error when starting the scan
 */
CHIP_ERROR StartNetworkScan(chip::ByteSpan ssid, ScanCallback callback);

/**
 * @brief Creates and starts the WiFi task that processes Wifi events and operations
 *
 * @return CHIP_ERROR CHIP_NO_ERROR if the task was successfully started and initialized
 *         CHIP_ERROR_NO_MEMORY if the task failed to be created
 *         CHIP_ERROR_INTERNAL if software or hardware initialization failed
 */
CHIP_ERROR StartWifiTask();

/**
 * @brief Configures the Wi-Fi devices as a Wi-Fi station
 */
void ConfigureStationMode();

/**
 * @brief Returns the state of the Wi-Fi connection
 *
 * @return true, if the Wi-Fi device is connected to an AP
 *         false, otherwise
 */
bool IsStationConnected();

/**
 * @brief Returns the state of the Wi-Fi Station configuration of the Wi-Fi device
 *
 * @return true, if the Wi-Fi Station mode is enabled
 *         false, otherwise
 */
bool IsStationModeEnabled();

/**
 * @brief Returns the state of the Wi-Fi station initialization
 *
 * @return true, if the initialization was successful
 *         false, otherwise
 */
bool IsStationReady();

/**
 * @brief Triggers the device to disconnect from the connected Wi-Fi network
 *
 * @note The disconnection is not immediate. It can take a certain amount of time for the device to be in a disconnected state once
 *       the function is called. When the function returns, the device might not have yet disconnected from the Wi-Fi network.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR, disconnection request was succesfully triggered
 *         otherwise, CHIP_ERROR_INTERNAL
 */
CHIP_ERROR TriggerDisconnection();

/**
 * @brief Gets the connected access point information.
 *        See @wfx_wifi_scan_result_t for the information that is returned by the function.
 *
 * @param[out] info AP information
 *
 * @return CHIP_ERROR CHIP_NO_ERROR, device has succesfully pulled all the AP information
 *                    CHIP_ERROR_INTERNAL, otherwise. If the function returns an error, the data in ap cannot be used.
 */
CHIP_ERROR GetAccessPointInfo(wfx_wifi_scan_result_t & info);

/**
 * @brief Gets the connected access point extended information.
 *        See @wfx_wifi_scan_ext_t for the information that is returned by the information
 *
 * @param[out] info AP extended information
 *
 * @return CHIP_ERROR CHIP_NO_ERROR, device has succesfully pulled all the AP information
 *                    CHIP_ERROR_INTERNAL, otherwise. If the function returns an error, the data in ap cannot be used.
 */
CHIP_ERROR GetAccessPointExtendedInfo(wfx_wifi_scan_ext_t & info);

/**
 * @brief Function resets the BeaconLostCount, BeaconRxCount, PacketMulticastRxCount, PacketMulticastTxCount, PacketUnicastRxCount,
 *        PacketUnicastTxCount back to 0
 *
 * @return CHIP_ERROR CHIP_NO_ERROR, the counters were succesfully reset to 0.
 *                    CHIP_ERROR_INTERNAL, if there was an error when resetting the counter values
 */
CHIP_ERROR ResetCounters();

/**
 * @brief Configures the broadcast filter.
 *
 * @param[in] enableBroadcastFilter Boolean to enable or disable the broadcast filter.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR, the counters were succesfully reset to 0.
 *                    CHIP_ERROR_INTERNAL, if there was an error when configuring the broadcast filter
 */
CHIP_ERROR ConfigureBroadcastFilter(bool enableBroadcastFilter);

/**
 * @brief Clears the stored Wi-Fi crendetials stored in RAM only
 */
void ClearWifiCredentials();

/**
 * @brief Stores the Wi-Fi credentials
 *
 * @note Function does not validate if the device already has Wi-Fi credentials.
 *       It is the responsibility of the caller to ensuret that.
 *       The function will overwrite any existing Wi-Fi credentials.
 *
 * @param[in] credentials
 */
void SetWifiCredentials(const WifiCredentials & credentials);

/**
 * @brief Returns the configured Wi-Fi credentials
 *
 * @param[out] credentials stored wifi crendetials
 *
 * @return CHIP_ERROR CHIP_ERROR_INCORRECT_STATE, if the device does not have any set credentials
 *                    CHIP_NO_ERROR, otherwise
 */
CHIP_ERROR GetWifiCredentials(WifiCredentials & credentials);

/**
 * @brief Returns the state of the Wi-Fi network provisionning
 *        Does the device has Wi-Fi credentials or not
 *
 * @return true, the device has Wi-Fi credentials
 *         false, otherwise
 */
bool IsWifiProvisioned();

/**
 * @brief Triggers a connection attempt the Access Point who's crendetials match the ones store with the SetWifiCredentials API.
 *        The function triggers an async connection attempt. The upper layers are notified trought a platform event if the
 *        connection attempt was successful or not.
 *
 *        The returned error code only indicates if the connection attempt was triggered or not.
 *
 * @return CHIP_ERROR CHIP_NO_ERROR, the connection attempt was succesfully triggered
 *                    CHIP_ERROR_INCORRECT_STATE, the Wi-Fi station does not have any Wi-Fi credentials
 *                    CHIP_ERROR_INVALID_ARGUMENT, the provisionned crendetials do not match the Wi-Fi station requirements
 *                    CHIP_ERROR_INTERNAL, otherwise
 */
CHIP_ERROR ConnectToAccessPoint(void);

/* Function to update */

// TODO: Harmonize the Power Save function inputs for all platforms
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
CHIP_ERROR ConfigurePowerSave(rsi_power_save_profile_mode_t sl_si91x_ble_state, sl_si91x_performance_profile_t sl_si91x_wifi_state,
                              uint32_t listenInterval);
#else
CHIP_ERROR ConfigurePowerSave();
#endif /* (SLI_SI91X_MCU_INTERFACE | EXP_BOARD) */
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if CHIP_DEVICE_CONFIG_ENABLE_IPV4
/**
 * @brief Returns IP assignment status
 *

 * @return true, Wi-Fi station has an IPv4 address
 *         false, otherwise
 */
bool HasAnIPv4Address();
#endif /* CHIP_DEVICE_CONFIG_ENABLE_IPV4 */

/**
 * @brief Returns IP assignment status
 *

 * @return true, Wi-Fi station has an IPv6 address
 *         false, otherwise
 */
bool HasAnIPv6Address();

/**
 * @brief Cancels the on-going network scan operation.
 *        If one isn't in-progress, function doesn't do anything
 */
void CancelScanNetworks();

/**
 * @brief Notifies upper-layers that Wi-Fi initialization has succesfully completed
 */
void NotifyWifiTaskInitialized(void);

/**
 * @brief Function schedules a reconnection attempt with the Access Point
 *
 * @note The retry interval increases exponentially with each attempt, starting from a minimum value and doubling each time,
 *       up to a maximum value. For example, if the initial retry interval is 1 second, the subsequent intervals will be 2 seconds,
 *       4 seconds, 8 seconds, and so on, until the maximum retry interval is reached.
 */
void ScheduleConnectionAttempt();
