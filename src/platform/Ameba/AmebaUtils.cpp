/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          General utility methods for the Ameba platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <chip_porting.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Ameba/AmebaConfig.h>
#include <platform/Ameba/AmebaUtils.h>
#include <platform/Ameba/ConfigurationManagerImpl.h>

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
} // namespace

CHIP_ERROR AmebaUtils::StartWiFi(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Ensure that the WiFi layer is started.
    wifi_on(RTW_MODE_STA);
    return err;
}

CHIP_ERROR AmebaUtils::IsStationEnabled(bool & staEnabled)
{
    staEnabled = (wifi_mode == RTW_MODE_STA || wifi_mode == RTW_MODE_STA_AP);
    return CHIP_NO_ERROR;
}

bool AmebaUtils::IsStationProvisioned(void)
{
    rtw_wifi_config_t WiFiConfig = { 0 };
    return ((GetWiFiConfig(&WiFiConfig) == CHIP_NO_ERROR) && (WiFiConfig.ssid[0] != 0));
}

CHIP_ERROR AmebaUtils::IsStationConnected(bool & connected)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    connected      = (wifi_is_connected_to_ap() == RTW_SUCCESS) ? 1 : 0;
    return err;
}

CHIP_ERROR AmebaUtils::EnableStationMode(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Ensure that station mode is enabled in the WiFi layer.
    wifi_set_mode(RTW_MODE_STA);
    return err;
}

CHIP_ERROR AmebaUtils::SetWiFiConfig(rtw_wifi_config_t * config)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    /* Store Wi-Fi Configurations in Storage */
    err = PersistedStorage::KeyValueStoreMgr().Put(kWiFiSSIDKeyName, config->ssid, sizeof(config->ssid));
    SuccessOrExit(err);

    err = PersistedStorage::KeyValueStoreMgr().Put(kWiFiCredentialsKeyName, config->password, sizeof(config->password));
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR AmebaUtils::GetWiFiConfig(rtw_wifi_config_t * config)
{
    CHIP_ERROR err        = CHIP_NO_ERROR;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    /* Retrieve Wi-Fi Configurations from Storage */
    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, config->ssid, sizeof(config->ssid), &ssidLen);
    SuccessOrExit(err);

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, config->password, sizeof(config->password),
                                                   &credentialsLen);
    SuccessOrExit(err);

    config->ssid_len     = ssidLen;
    config->password_len = credentialsLen;

exit:
    return err;
}

CHIP_ERROR AmebaUtils::ClearWiFiConfig()
{
    // Clear Ameba WiFi station config
    CHIP_ERROR err = CHIP_NO_ERROR;
    rtw_wifi_config_t wifiConfig;
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    err = SetWiFiConfig(&wifiConfig);
    return err;
}

CHIP_ERROR AmebaUtils::WiFiDisconnect(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "wifi_disconnect");
    err = (wifi_disconnect() == RTW_SUCCESS) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    return err;
}

CHIP_ERROR AmebaUtils::WiFiConnect(void)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    rtw_wifi_config_t * config = (rtw_wifi_config_t *) pvPortMalloc(sizeof(rtw_wifi_config_t));
    memset(config, 0, sizeof(rtw_wifi_config_t));
    GetWiFiConfig(config);
    ChipLogProgress(DeviceLayer, "Connecting to AP : [%s]", (char *) config->ssid);
    int ameba_err = wifi_connect((char *) config->ssid, RTW_SECURITY_WPA_WPA2_MIXED, (char *) config->password,
                                 strlen((const char *) config->ssid), strlen((const char *) config->password), 0, NULL);

    vPortFree(config);
    err = (ameba_err == RTW_SUCCESS) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    return err;
}
