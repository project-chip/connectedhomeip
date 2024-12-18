/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *          General utility methods for the ESP32 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/ErrorStr.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/ESP32Utils.h>

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "esp_wifi.h"
#include "nvs.h"

#ifdef CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
#include "esp_heap_caps.h"
#include <lib/support/CHIPMemString.h>
#include <string.h>
#include <system/SystemTimer.h>
#include <tracing/macros.h>
#include <tracing/metric_event.h>

using namespace chip::DeviceLayer;
using namespace chip::Tracing;

// Heap Diagnostics (internal)
constexpr MetricKey kMetricHeapInternalFree         = "internal_free";
constexpr MetricKey kMetricHeapInternalMinFree      = "internal_min_free";
constexpr MetricKey kMetricHeapInternalLargestBlock = "internal_largest_free";

// Heap Diagnostics (external)
constexpr MetricKey kMetricHeapExternalFree         = "external_free";
constexpr MetricKey kMetricHeapExternalMinFree      = "external_min_free";
constexpr MetricKey kMetricHeapExternalLargestBlock = "external_largest_block";

// Task runtime
constexpr MetricKey kMetricTaskName = "runtime";
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

#ifndef CONFIG_HEAP_LOG_INTERVAL
#define CONFIG_HEAP_LOG_INTERVAL 300000
#endif // CONFIG_HEAP_LOG_INTERVAL

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR ESP32Utils::IsAPEnabled(bool & apEnabled)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    wifi_mode_t curWiFiMode;

    esp_err_t err = esp_wifi_get_mode(&curWiFiMode);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_mode() failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    apEnabled = (curWiFiMode == WIFI_MODE_AP || curWiFiMode == WIFI_MODE_APSTA);

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
}

CHIP_ERROR ESP32Utils::IsStationEnabled(bool & staEnabled)
{
    wifi_mode_t curWiFiMode;
    esp_err_t err = esp_wifi_get_mode(&curWiFiMode);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_mode() failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    staEnabled = (curWiFiMode == WIFI_MODE_STA || curWiFiMode == WIFI_MODE_APSTA);

    return CHIP_NO_ERROR;
}

bool ESP32Utils::IsStationProvisioned(void)
{
    wifi_config_t stationConfig;
    return (esp_wifi_get_config(WIFI_IF_STA, &stationConfig) == ERR_OK && stationConfig.sta.ssid[0] != 0);
}

CHIP_ERROR ESP32Utils::IsStationConnected(bool & connected)
{
    wifi_ap_record_t apInfo;
    connected = (esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK && apInfo.ssid[0] != 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Utils::StartWiFiLayer(void)
{
    int8_t ignored;
    bool wifiStarted;

    // There appears to be no direct way to ask the ESP WiFi layer if esp_wifi_start()
    // has been called.  So use the ESP_ERR_WIFI_NOT_STARTED error returned by
    // esp_wifi_get_max_tx_power() to detect this.
    esp_err_t err = esp_wifi_get_max_tx_power(&ignored);
    switch (err)
    {
    case ESP_OK:
        wifiStarted = true;
        break;
    case ESP_ERR_WIFI_NOT_STARTED:
        wifiStarted = false;
        break;
    default:
        return ESP32Utils::MapError(err);
    }

    if (!wifiStarted)
    {
        ChipLogProgress(DeviceLayer, "Starting ESP WiFi layer");

        err = esp_wifi_start();
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_wifi_start() failed: %s", esp_err_to_name(err));
            return ESP32Utils::MapError(err);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Utils::EnableStationMode(void)
{
    wifi_mode_t curWiFiMode;

    // Get the current ESP WiFI mode.
    esp_err_t err = esp_wifi_get_mode(&curWiFiMode);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_mode() failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    // If station mode is not already enabled (implying the current mode is WIFI_MODE_AP), change
    // the mode to WIFI_MODE_APSTA.
    if (curWiFiMode == WIFI_MODE_AP)
    {
        ChipLogProgress(DeviceLayer, "Changing ESP WiFi mode: %s -> %s", WiFiModeToStr(WIFI_MODE_AP),
                        WiFiModeToStr(WIFI_MODE_APSTA));

        err = esp_wifi_set_mode(WIFI_MODE_APSTA);
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_wifi_set_mode() failed: %s", esp_err_to_name(err));
            return ESP32Utils::MapError(err);
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Utils::SetAPMode(bool enabled)
{
    wifi_mode_t curWiFiMode;
    wifi_mode_t targetWiFiMode = WIFI_MODE_STA;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    targetWiFiMode = (enabled) ? WIFI_MODE_APSTA : WIFI_MODE_STA;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

    // Get the current ESP WiFI mode.
    esp_err_t err = esp_wifi_get_mode(&curWiFiMode);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_mode() failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    // If station mode is not already enabled (implying the current mode is WIFI_MODE_AP), change
    // the mode to WIFI_MODE_APSTA.
    if (curWiFiMode != targetWiFiMode)
    {
        ChipLogProgress(DeviceLayer, "Changing ESP WiFi mode: %s -> %s", WiFiModeToStr(curWiFiMode), WiFiModeToStr(targetWiFiMode));

        err = esp_wifi_set_mode(targetWiFiMode);
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_wifi_set_mode() failed: %s", esp_err_to_name(err));
            return ESP32Utils::MapError(err);
        }
    }

    return CHIP_NO_ERROR;
}

int ESP32Utils::OrderScanResultsByRSSI(const void * _res1, const void * _res2)
{
    const wifi_ap_record_t * res1 = (const wifi_ap_record_t *) _res1;
    const wifi_ap_record_t * res2 = (const wifi_ap_record_t *) _res2;

    if (res1->rssi > res2->rssi)
    {
        return -1;
    }
    if (res1->rssi < res2->rssi)
    {
        return 1;
    }
    return 0;
}

const char * ESP32Utils::WiFiModeToStr(wifi_mode_t wifiMode)
{
    switch (wifiMode)
    {
    case WIFI_MODE_NULL:
        return "NULL";
    case WIFI_MODE_STA:
        return "STA";
    case WIFI_MODE_AP:
        return "AP";
    case WIFI_MODE_APSTA:
        return "STA+AP";
    default:
        return "(unknown)";
    }
}

const char * ESP32Utils::WiFiDisconnectReasonToStr(uint16_t reason)
{
    switch (reason)
    {
    case WIFI_REASON_ASSOC_TOOMANY:
        return "WIFI_REASON_ASSOC_TOOMANY";
    case WIFI_REASON_NOT_ASSOCED:
        return "WIFI_REASON_NOT_ASSOCED";
    case WIFI_REASON_ASSOC_NOT_AUTHED:
        return "WIFI_REASON_ASSOC_NOT_AUTHED";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_GROUP_CIPHER_INVALID:
        return "WIFI_REASON_GROUP_CIPHER_INVALID";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
        return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_AKMP_INVALID:
        return "WIFI_REASON_AKMP_INVALID";
    case WIFI_REASON_CIPHER_SUITE_REJECTED:
        return "WIFI_REASON_CIPHER_SUITE_REJECTED";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
        return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
    case WIFI_REASON_NOT_AUTHED:
        return "WIFI_REASON_NOT_AUTHED";
    case WIFI_REASON_MIC_FAILURE:
        return "WIFI_REASON_MIC_FAILURE";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS:
        return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_INVALID_RSN_IE_CAP:
        return "WIFI_REASON_INVALID_RSN_IE_CAP";
    case WIFI_REASON_INVALID_PMKID:
        return "WIFI_REASON_INVALID_PMKID";
    case WIFI_REASON_802_1X_AUTH_FAILED:
        return "WIFI_REASON_802_1X_AUTH_FAILED";
    case WIFI_REASON_NO_AP_FOUND:
        return "WIFI_REASON_NO_AP_FOUND";
    case WIFI_REASON_BEACON_TIMEOUT:
        return "WIFI_REASON_BEACON_TIMEOUT";
    case WIFI_REASON_AUTH_EXPIRE:
        return "WIFI_REASON_AUTH_EXPIRE";
    case WIFI_REASON_AUTH_LEAVE:
        return "WIFI_REASON_AUTH_LEAVE";
    case WIFI_REASON_ASSOC_LEAVE:
        return "WIFI_REASON_ASSOC_LEAVE";
    case WIFI_REASON_ASSOC_EXPIRE:
        return "WIFI_REASON_ASSOC_EXPIRE";
    default:
        return "Unknown Reason";
    }
}

struct netif * ESP32Utils::GetStationNetif(void)
{
    return GetNetif(kDefaultWiFiStationNetifKey);
}

CHIP_ERROR ESP32Utils::GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
    wifi_config_t stationConfig;

    esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &stationConfig);
    if (err != ESP_OK)
    {
        return ESP32Utils::MapError(err);
    }

    VerifyOrReturnError(stationConfig.sta.ssid[0] != 0, CHIP_ERROR_INCORRECT_STATE);

    netInfo.NetworkId              = kWiFiStationNetworkId;
    netInfo.FieldPresent.NetworkId = true;
    memcpy(netInfo.WiFiSSID, stationConfig.sta.ssid,
           std::min(strlen(reinterpret_cast<char *>(stationConfig.sta.ssid)) + 1, sizeof(netInfo.WiFiSSID)));

    // Enforce that netInfo wifiSSID is null terminated
    netInfo.WiFiSSID[kMaxWiFiSSIDLength] = '\0';

    if (includeCredentials)
    {
        static_assert(sizeof(netInfo.WiFiKey) < 255, "Our min might not fit in netInfo.WiFiKeyLen");
        netInfo.WiFiKeyLen = static_cast<uint8_t>(std::min(strlen((char *) stationConfig.sta.password), sizeof(netInfo.WiFiKey)));
        memcpy(netInfo.WiFiKey, stationConfig.sta.password, netInfo.WiFiKeyLen);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Utils::SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo)
{
    wifi_config_t wifiConfig;

    char wifiSSID[kMaxWiFiSSIDLength + 1];
    size_t netInfoSSIDLen = strlen(netInfo.WiFiSSID);

    // Ensure that ESP station mode is enabled.  This is required before esp_wifi_set_config(ESP_IF_WIFI_STA,...)
    // can be called.
    ReturnErrorOnFailure(ESP32Utils::EnableStationMode());

    // Enforce that wifiSSID is null terminated before copying it
    memcpy(wifiSSID, netInfo.WiFiSSID, std::min(netInfoSSIDLen + 1, sizeof(wifiSSID)));
    if (netInfoSSIDLen + 1 < sizeof(wifiSSID))
    {
        wifiSSID[netInfoSSIDLen] = '\0';
    }
    else
    {
        wifiSSID[kMaxWiFiSSIDLength] = '\0';
    }

    // Initialize an ESP wifi_config_t structure based on the new provision information.
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.sta.ssid, wifiSSID, std::min(strlen(wifiSSID) + 1, sizeof(wifiConfig.sta.ssid)));
    memcpy(wifiConfig.sta.password, netInfo.WiFiKey, std::min((size_t) netInfo.WiFiKeyLen, sizeof(wifiConfig.sta.password)));
    wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;

    // Configure the ESP WiFi interface.
    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", esp_err_to_name(err));
        return ESP32Utils::MapError(err);
    }

    ChipLogProgress(DeviceLayer, "WiFi station provision set (SSID: %s)", netInfo.WiFiSSID);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Utils::ClearWiFiStationProvision(void)
{
    wifi_config_t stationConfig;

    // Clear the ESP WiFi station configuration.
    memset(&stationConfig, 0, sizeof(stationConfig));
    esp_wifi_set_config(WIFI_IF_STA, &stationConfig);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Utils::InitWiFiStack(void)
{
    wifi_init_config_t cfg;
    uint8_t ap_mac[6];
    wifi_mode_t mode;
    esp_err_t err = esp_netif_init();
    if (err != ESP_OK)
    {
        return ESP32Utils::MapError(err);
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
    // Lets not create a default AP interface if already present
    if (!esp_netif_get_handle_from_ifkey(kDefaultWiFiAPNetifKey))
    {
        if (!esp_netif_create_default_wifi_ap())
        {
            ChipLogError(DeviceLayer, "Failed to create the WiFi AP netif");
            return CHIP_ERROR_INTERNAL;
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP

    // Lets not create a default station interface if already present
    if (!esp_netif_get_handle_from_ifkey(kDefaultWiFiStationNetifKey))
    {
        if (!esp_netif_create_default_wifi_sta())
        {
            ChipLogError(DeviceLayer, "Failed to create the WiFi STA netif");
            return CHIP_ERROR_INTERNAL;
        }
    }

    // Initialize the ESP WiFi layer.
    cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK)
    {
        return ESP32Utils::MapError(err);
    }

    esp_wifi_get_mode(&mode);
    if ((mode == WIFI_MODE_AP) || (mode == WIFI_MODE_APSTA))
    {
        esp_fill_random(ap_mac, sizeof(ap_mac));
        /* Bit 0 of the first octet of MAC Address should always be 0 */
        ap_mac[0] &= (uint8_t) ~0x01;
        err = esp_wifi_set_mac(WIFI_IF_AP, ap_mac);
        if (err != ESP_OK)
        {
            return ESP32Utils::MapError(err);
        }
    }
    err = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent, NULL);
    if (err != ESP_OK)
    {
        return ESP32Utils::MapError(err);
    }
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#ifdef CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE
void LogHeapDataCallback(chip::System::Layer * systemLayer, void * appState)
{
    // Internal RAM (default heap)
    uint32_t internal_free               = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    uint32_t internal_largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    uint32_t internal_min_free           = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);

    MATTER_LOG_METRIC(kMetricHeapInternalFree, internal_free);
    MATTER_LOG_METRIC(kMetricHeapInternalMinFree, internal_min_free);
    MATTER_LOG_METRIC(kMetricHeapInternalLargestBlock, internal_largest_free_block);

#ifdef CONFIG_SPIRAM
    // External RAM (if PSRAM is enabled)
    uint32_t external_free               = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    uint32_t external_largest_free_block = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    uint32_t external_min_free           = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);

    MATTER_LOG_METRIC(kMetricHeapExternalFree, external_free);
    MATTER_LOG_METRIC(kMetricHeapExternalMinFree, external_min_free);
    MATTER_LOG_METRIC(kMetricHeapExternalLargestBlock, external_largest_free_block);
#endif

    // Reschedule the timer for the next interval
    systemLayer->StartTimer(chip::System::Clock::Milliseconds32(CONFIG_HEAP_LOG_INTERVAL), LogHeapDataCallback, nullptr);
}

void FailedAllocCallback(size_t size, uint32_t caps, const char * function_name)
{
    MATTER_TRACE_COUNTER("Failed_memory_allocations");
    ChipLogError(DeviceLayer, "Memory allocation failed!");
}

// Function to initialize and start periodic heap logging
void ESP32Utils::LogHeapInfo()
{
    esp_err_t err = heap_caps_register_failed_alloc_callback(FailedAllocCallback);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "Failed to register callback. Error: 0x%08x", err);
    }
    LogHeapDataCallback(&SystemLayer(), nullptr);

    // Start the periodic logging using SystemLayer
    SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(CONFIG_HEAP_LOG_INTERVAL), LogHeapDataCallback, nullptr);
}

const char * StateToString(eTaskState state)
{
    switch (state)
    {
    case eRunning:
        return "Running";
    case eReady:
        return "Ready";
    case eBlocked:
        return "Blocked";
    case eSuspended:
        return "Suspended";
    case eDeleted:
        return "Deleted";
    default:
        return "Unknown";
    }
}

CHIP_ERROR ESP32Utils::LogTaskSnapshotInfo()
{
#ifdef CONFIG_FREERTOS_USE_TRACE_FACILITY
    uint32_t arraySize = uxTaskGetNumberOfTasks();

    chip::Platform::ScopedMemoryBuffer<TaskStatus_t> taskStatusArray;
    VerifyOrReturnError(taskStatusArray.Calloc(arraySize), CHIP_ERROR_NO_MEMORY);

    uint32_t dummyRunTimeCounter;
    arraySize = uxTaskGetSystemState(taskStatusArray.Get(), arraySize, &dummyRunTimeCounter);

    auto GenerateMetricName = [&](const char * task_name, const char * suffix) {
        std::string metricName = task_name;
        metricName += "_";
        metricName += suffix;
        return metricName;
    };

    for (int i = 0; i < arraySize; i++)
    {
        TaskStatus_t task = taskStatusArray[i];
        MATTER_TRACE_INSTANT(GenerateMetricName(task.pcTaskName, "state").c_str(), StateToString(task.eCurrentState));
        MATTER_TRACE_INSTANT(GenerateMetricName(task.pcTaskName, "stack_start_address").c_str(),
                             std::to_string(reinterpret_cast<uintptr_t>(task.pxStackBase)).c_str());
#ifdef CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
        MATTER_LOG_METRIC(kMetricTaskName, static_cast<uint32_t>(task.ulRunTimeCounter));
#endif // CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS
    }
#endif // CONFIG_FREERTOS_USE_TRACE_FACILITY
    return CHIP_NO_ERROR;
}
#endif // CONFIG_ENABLE_ESP_DIAGNOSTICS_TRACE

struct netif * ESP32Utils::GetNetif(const char * ifKey)
{
    struct netif * netif       = NULL;
    esp_netif_t * netif_handle = NULL;
    netif_handle               = esp_netif_get_handle_from_ifkey(ifKey);
    netif                      = (struct netif *) esp_netif_get_netif_impl(netif_handle);
    return netif;
}

bool ESP32Utils::IsInterfaceUp(const char * ifKey)
{
    struct netif * netif = GetNetif(ifKey);
    return netif != NULL && netif_is_up(netif);
}

bool ESP32Utils::HasIPv6LinkLocalAddress(const char * ifKey)
{
    struct esp_ip6_addr if_ip6_unused;
    return esp_netif_get_ip6_linklocal(esp_netif_get_handle_from_ifkey(ifKey), &if_ip6_unused) == ESP_OK;
}

CHIP_ERROR ESP32Utils::MapError(esp_err_t error)
{
    if (error == ESP_OK)
    {
        return CHIP_NO_ERROR;
    }
    if (error == ESP_ERR_NVS_NOT_FOUND)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (error == ESP_ERR_NVS_INVALID_LENGTH)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    return CHIP_ERROR(ChipError::Range::kPlatform, error);
}

/**
 * Given a CHIP error value that represents an ESP32 error, returns a
 * human-readable NULL-terminated C string describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not an ESP32 error.
 *
 */
bool ESP32Utils::FormatError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    if (!err.IsRange(ChipError::Range::kPlatform))
    {
        return false;
    }

#if CHIP_CONFIG_SHORT_ERROR_STR
    const char * desc = NULL;
#else  // CHIP_CONFIG_SHORT_ERROR_STR
    const char * desc = esp_err_to_name((esp_err_t) err.GetValue());
#endif // CHIP_CONFIG_SHORT_ERROR_STR

    chip::FormatError(buf, bufSize, "ESP32", err, desc);

    return true;
}

/**
 * Register a text error formatter for ESP32 errors.
 */
void ESP32Utils::RegisterESP32ErrorFormatter()
{
    static ErrorFormatter sErrorFormatter = { ESP32Utils::FormatError, NULL };

    RegisterErrorFormatter(&sErrorFormatter);
}
