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

#include <platform/ESP32/ESP32Utils.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>

#include "esp_event.h"
#include "esp_wifi.h"

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

CHIP_ERROR ESP32Utils::IsAPEnabled(bool & apEnabled)
{
    CHIP_ERROR err;
    wifi_mode_t curWiFiMode;

    err = esp_wifi_get_mode(&curWiFiMode);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_mode() failed: %s", chip::ErrorStr(err));
        return err;
    }

    apEnabled = (curWiFiMode == WIFI_MODE_AP || curWiFiMode == WIFI_MODE_APSTA);

    return CHIP_NO_ERROR;
}

bool ESP32Utils::IsStationProvisioned(void)
{
    wifi_config_t stationConfig;
    return (esp_wifi_get_config(ESP_IF_WIFI_STA, &stationConfig) == ERR_OK && stationConfig.sta.ssid[0] != 0);
}

CHIP_ERROR ESP32Utils::IsStationConnected(bool & connected)
{
    wifi_ap_record_t apInfo;
    connected = (esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Utils::StartWiFiLayer(void)
{
    CHIP_ERROR err;
    int8_t ignored;
    bool wifiStarted;

    // There appears to be no direct way to ask the ESP WiFi layer if esp_wifi_start()
    // has been called.  So use the ESP_ERR_WIFI_NOT_STARTED error returned by
    // esp_wifi_get_max_tx_power() to detect this.
    err = esp_wifi_get_max_tx_power(&ignored);
    switch (err)
    {
    case ESP_OK:
        wifiStarted = true;
        break;
    case ESP_ERR_WIFI_NOT_STARTED:
        wifiStarted = false;
        err         = ESP_OK;
        break;
    default:
        ExitNow();
    }

    if (!wifiStarted)
    {
        ChipLogProgress(DeviceLayer, "Starting ESP WiFi layer");

        err = esp_wifi_start();
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_wifi_start() failed: %s", chip::ErrorStr(err));
        }
    }

exit:
    return err;
}

CHIP_ERROR ESP32Utils::EnableStationMode(void)
{
    CHIP_ERROR err;
    wifi_mode_t curWiFiMode;

    // Get the current ESP WiFI mode.
    err = esp_wifi_get_mode(&curWiFiMode);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_mode() failed: %s", chip::ErrorStr(err));
    }
    SuccessOrExit(err);

    // If station mode is not already enabled (implying the current mode is WIFI_MODE_AP), change
    // the mode to WIFI_MODE_APSTA.
    if (curWiFiMode == WIFI_MODE_AP)
    {
        ChipLogProgress(DeviceLayer, "Changing ESP WiFi mode: %s -> %s", WiFiModeToStr(WIFI_MODE_AP),
                        WiFiModeToStr(WIFI_MODE_APSTA));

        err = esp_wifi_set_mode(WIFI_MODE_APSTA);
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_wifi_set_mode() failed: %s", chip::ErrorStr(err));
        }
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR ESP32Utils::SetAPMode(bool enabled)
{
    CHIP_ERROR err;
    wifi_mode_t curWiFiMode, targetWiFiMode;

    targetWiFiMode = (enabled) ? WIFI_MODE_APSTA : WIFI_MODE_STA;

    // Get the current ESP WiFI mode.
    err = esp_wifi_get_mode(&curWiFiMode);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_get_mode() failed: %s", chip::ErrorStr(err));
    }
    SuccessOrExit(err);

    // If station mode is not already enabled (implying the current mode is WIFI_MODE_AP), change
    // the mode to WIFI_MODE_APSTA.
    if (curWiFiMode != targetWiFiMode)
    {
        ChipLogProgress(DeviceLayer, "Changing ESP WiFi mode: %s -> %s", WiFiModeToStr(curWiFiMode), WiFiModeToStr(targetWiFiMode));

        err = esp_wifi_set_mode(targetWiFiMode);
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "esp_wifi_set_mode() failed: %s", chip::ErrorStr(err));
        }
        SuccessOrExit(err);
    }

exit:
    return err;
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

struct netif * ESP32Utils::GetStationNetif(void)
{
    return GetNetif(TCPIP_ADAPTER_IF_STA);
}

struct netif * ESP32Utils::GetNetif(tcpip_adapter_if_t intfId)
{
    struct netif * netif;
    return (tcpip_adapter_get_netif(intfId, (void **) &netif) == ESP_OK) ? netif : NULL;
}

bool ESP32Utils::IsInterfaceUp(tcpip_adapter_if_t intfId)
{
    struct netif * netif = GetNetif(intfId);
    return netif != NULL && netif_is_up(netif);
}

const char * ESP32Utils::InterfaceIdToName(tcpip_adapter_if_t intfId)
{
    switch (intfId)
    {
    case TCPIP_ADAPTER_IF_STA:
        return "WiFi station";
    case TCPIP_ADAPTER_IF_AP:
        return "WiFi AP";
    case TCPIP_ADAPTER_IF_ETH:
        return "Ethernet";
    default:
        return "(unknown)";
    }
}

bool ESP32Utils::HasIPv6LinkLocalAddress(tcpip_adapter_if_t intfId)
{
    ip6_addr_t unused;
    return tcpip_adapter_get_ip6_linklocal(intfId, &unused) == ESP_OK;
}

CHIP_ERROR ESP32Utils::GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t stationConfig;

    err = esp_wifi_get_config(ESP_IF_WIFI_STA, &stationConfig);
    SuccessOrExit(err);

    VerifyOrExit(stationConfig.sta.ssid[0] != 0, err = CHIP_ERROR_INCORRECT_STATE);

    netInfo.NetworkId              = kWiFiStationNetworkId;
    netInfo.FieldPresent.NetworkId = true;
    memcpy(netInfo.WiFiSSID, stationConfig.sta.ssid,
           min(strlen(reinterpret_cast<char *>(stationConfig.sta.ssid)) + 1, sizeof(netInfo.WiFiSSID)));

    // Enforce that netInfo wifiSSID is null terminated
    netInfo.WiFiSSID[kMaxWiFiSSIDLength] = '\0';

    if (includeCredentials)
    {
        netInfo.WiFiKeyLen = min(strlen((char *) stationConfig.sta.password), sizeof(netInfo.WiFiKey));
        memcpy(netInfo.WiFiKey, stationConfig.sta.password, netInfo.WiFiKeyLen);
    }

exit:
    return err;
}

CHIP_ERROR ESP32Utils::SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t wifiConfig;

    char wifiSSID[kMaxWiFiSSIDLength + 1];
    size_t netInfoSSIDLen = strlen(netInfo.WiFiSSID);

    // Ensure that ESP station mode is enabled.  This is required before esp_wifi_set_config(ESP_IF_WIFI_STA,...)
    // can be called.
    err = ESP32Utils::EnableStationMode();
    SuccessOrExit(err);

    // Enforce that wifiSSID is null terminated before copying it
    memcpy(wifiSSID, netInfo.WiFiSSID, min(netInfoSSIDLen + 1, sizeof(wifiSSID)));
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
    memcpy(wifiConfig.sta.ssid, wifiSSID, min(strlen(wifiSSID) + 1, sizeof(wifiConfig.sta.ssid)));
    memcpy(wifiConfig.sta.password, netInfo.WiFiKey, min((size_t) netInfo.WiFiKeyLen, sizeof(wifiConfig.sta.password)));
    wifiConfig.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifiConfig.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;

    // Configure the ESP WiFi interface.
    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig);
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_set_config() failed: %s", chip::ErrorStr(err));
    }
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "WiFi station provision set (SSID: %s)", netInfo.WiFiSSID);

exit:
    return err;
}

CHIP_ERROR ESP32Utils::ClearWiFiStationProvision(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wifi_config_t stationConfig;

    // Clear the ESP WiFi station configuration.
    memset(&stationConfig, 0, sizeof(stationConfig));
    esp_wifi_set_config(ESP_IF_WIFI_STA, &stationConfig);

    return err;
}
