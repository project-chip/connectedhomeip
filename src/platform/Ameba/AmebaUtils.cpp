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
#include <lib/core/ErrorStr.h>
#include <lib/support/CodeUtils.h>
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
    bool staEnabled;
    int32_t error;
    CHIP_ERROR err;

    if (wifi_mode == RTW_MODE_AP)
    {
        ChipLogError(DeviceLayer, "StartWiFi(): Does not support RTW_MODE_AP, change to RTW_MODE_STA_AP");
        error = matter_wifi_set_mode(RTW_MODE_STA_AP);
        err   = MapError(error, AmebaErrorType::kWiFiError);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "matter_wifi_on(RTW_MODE_STA_AP) failed");
            return err;
        }
    }

    if (!(IsStationInterfaceUp()))
    {
        ChipLogError(DeviceLayer, "StartWiFi() setting Wi-Fi interface");
        error = matter_wifi_set_mode(RTW_MODE_STA);
        err   = MapError(error, AmebaErrorType::kWiFiError);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "matter_wifi_set_mode(RTW_MODE_STA) failed");
            return err;
        }
    }
exit:
    return CHIP_NO_ERROR; // will fail if wifi is already initialized, let it pass
}

bool AmebaUtils::IsStationInterfaceUp(void)
{
    /* Station mode will only be setup in Interface 0 in both station only and concurrent mode
     * Thus, only check Interface 0 for station mode
     */
    return matter_wifi_is_up(RTW_STA_INTERFACE);
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

CHIP_ERROR AmebaUtils::IsStationIPLinked(bool & linked)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    linked         = (matter_wifi_is_ready_to_transceive(RTW_STA_INTERFACE) == RTW_SUCCESS) ? 1 : 0;
    return err;
}

bool AmebaUtils::IsStationOpenSecurity(void)
{
    bool is_open_security = (matter_wifi_is_open_security()) ? 1 : 0;
    return is_open_security;
}

CHIP_ERROR AmebaUtils::IsStationConnected(bool & connected)
{
    int32_t error  = matter_wifi_is_connected_to_ap();
    CHIP_ERROR err = MapError(error, AmebaErrorType::kWiFiError);
    connected      = (err == CHIP_NO_ERROR) ? true : false;
    return err;
}

CHIP_ERROR AmebaUtils::EnableStationMode(void)
{
    // Ensure that station mode is enabled in the WiFi layer.
    int32_t error  = matter_wifi_set_mode(RTW_MODE_STA);
    CHIP_ERROR err = MapError(error, AmebaErrorType::kWiFiError);
    return err;
}

CHIP_ERROR AmebaUtils::SetWiFiConfig(rtw_wifi_config_t * config)
{
    CHIP_ERROR err;
    // don't store if ssid is null
    if (config->ssid[0] == 0)
    {
        return CHIP_NO_ERROR;
    }

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
    CHIP_ERROR err;
    size_t ssidLen        = 0;
    size_t credentialsLen = 0;

    /* Retrieve Wi-Fi Configurations from Storage */
    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiSSIDKeyName, config->ssid, sizeof(config->ssid), &ssidLen);
    SuccessOrExit(err);

    err = PersistedStorage::KeyValueStoreMgr().Get(kWiFiCredentialsKeyName, config->password, sizeof(config->password),
                                                   &credentialsLen);
    SuccessOrExit(err);

    config->ssid_len     = strlen((const char *) config->ssid);
    config->password_len = strlen((const char *) config->password);

exit:
    return err;
}

CHIP_ERROR AmebaUtils::ClearWiFiConfig()
{
    /* Clear Wi-Fi Configurations in Storage */
    CHIP_ERROR err;
    err = PersistedStorage::KeyValueStoreMgr().Delete(kWiFiSSIDKeyName);
    SuccessOrExit(err);

    err = PersistedStorage::KeyValueStoreMgr().Delete(kWiFiCredentialsKeyName);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR AmebaUtils::WiFiDisconnect(void)
{
    ChipLogProgress(DeviceLayer, "Disconnecting WiFi");
    int32_t error  = matter_wifi_disconnect();
    CHIP_ERROR err = MapError(error, AmebaErrorType::kWiFiError);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "matter_lwip_releaseip");
        matter_lwip_releaseip();
    }
    return err;
}

CHIP_ERROR AmebaUtils::WiFiConnectProvisionedNetwork(void)
{
    rtw_wifi_config_t * config = (rtw_wifi_config_t *) pvPortMalloc(sizeof(rtw_wifi_config_t));
    memset(config, 0, sizeof(rtw_wifi_config_t));
    GetWiFiConfig(config);
    ChipLogProgress(DeviceLayer, "Connecting to AP : [%s]", (char *) config->ssid);
    int32_t error  = matter_wifi_connect((char *) config->ssid, RTW_SECURITY_WPA_WPA2_MIXED, (char *) config->password,
                                         strlen((const char *) config->ssid), strlen((const char *) config->password), 0, nullptr);
    CHIP_ERROR err = MapError(error, AmebaErrorType::kWiFiError);

    vPortFree(config);
    return err;
}

CHIP_ERROR AmebaUtils::WiFiConnect(const char * ssid, const char * password)
{
    ChipLogProgress(DeviceLayer, "Connecting to AP : [%s]", (char *) ssid);
    int32_t error  = matter_wifi_connect((char *) ssid, RTW_SECURITY_WPA_WPA2_MIXED, (char *) password, strlen(ssid),
                                         strlen(password), 0, nullptr);
    CHIP_ERROR err = MapError(error, AmebaErrorType::kWiFiError);
    return err;
}

CHIP_ERROR AmebaUtils::SetCurrentProvisionedNetwork()
{
    rtw_wifi_setting_t pSetting;
    int32_t error  = matter_get_sta_wifi_info(&pSetting);
    CHIP_ERROR err = MapError(error, AmebaErrorType::kWiFiError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "STA No Wi-Fi Info");
        goto exit;
    }
    else
    {
        rtw_wifi_config_t config = { 0 };
        GetWiFiConfig(&config);
        if (!memcmp(config.ssid, pSetting.ssid, strlen((const char *) pSetting.ssid) + 1))
        {
            ChipLogProgress(DeviceLayer, "STA Wi-Fi Info exist, do nothing");
            matter_set_autoreconnect(0);
            goto exit;
        }
        else
        {
            ChipLogProgress(DeviceLayer, "STA Wi-Fi Info ");

            memcpy(config.ssid, pSetting.ssid, strlen((const char *) pSetting.ssid) + 1);
            memcpy(config.password, pSetting.password, strlen((const char *) pSetting.password) + 1);
            err = SetWiFiConfig(&config);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "SetWiFiConfig() failed");
                matter_set_autoreconnect(0);
                goto exit;
            }
        }
    }
exit:
    return err;
}

CHIP_ERROR AmebaUtils::MapError(int32_t error, AmebaErrorType type)
{
    if (type == AmebaErrorType::kDctError)
    {
        return MapDctError(error);
    }
    if (type == AmebaErrorType::kFlashError)
    {
        return MapFlashError(error);
    }
    if (type == AmebaErrorType::kWiFiError)
    {
        return MapWiFiError(error);
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR AmebaUtils::MapDctError(int32_t error)
{
    if (error == DCT_SUCCESS)
        return CHIP_NO_ERROR;
    if (error == DCT_ERR_NO_MEMORY)
        return CHIP_ERROR_NO_MEMORY;
    if (error == DCT_ERR_NOT_FIND)
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    if (error == DCT_ERR_SIZE_OVER)
        return CHIP_ERROR_INVALID_ARGUMENT;
    if (error == DCT_ERR_MODULE_BUSY)
        return CHIP_ERROR_BUSY;

    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR AmebaUtils::MapFlashError(int32_t error)
{
    if (error == 1)
        return CHIP_NO_ERROR;

    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR AmebaUtils::MapWiFiError(int32_t error)
{
    if (error == RTW_SUCCESS)
        return CHIP_NO_ERROR;

    return CHIP_ERROR_INTERNAL;
}
