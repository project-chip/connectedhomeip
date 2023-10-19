/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/ErrorStr.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ASR/ASRConfig.h>
#include <platform/ASR/ASRUtils.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "lega_wlan_api.h"
void lega_wlan_clear_pmk(void);
#ifdef __cplusplus
}
#endif

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

namespace {
constexpr uint8_t kWiFiMaxNetworks = 15;
uint8_t NumAP                      = 0;
lega_wlan_wifi_conf wifi_conf;
lega_wlan_scan_ap_record_t scan_result[kWiFiMaxNetworks];
} // namespace
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_AP
CHIP_ERROR ASRUtils::IsAPEnabled(bool & apEnabled)
{
    if (lega_wlan_get_wifi_mode() == SOFTAP)
    {
        apEnabled = true;
    }
    else
    {
        apEnabled = false;
    }
    return CHIP_NO_ERROR;
}
#endif
CHIP_ERROR ASRUtils::IsStationEnabled(bool & staEnabled)
{
    if (lega_wlan_get_wifi_mode() == STA)
    {
        staEnabled = true;
    }
    else
    {
        staEnabled = false;
    }
    return CHIP_NO_ERROR;
}

bool ASRUtils::IsStationProvisioned(void)
{
    lega_wlan_wifi_conf stationConfig;
    bool stationConnected;
    Internal::ASRUtils::IsStationConnected(stationConnected);
    return (asr_wifi_get_config(&stationConfig) == CHIP_NO_ERROR && stationConfig.ssid_len != 0) || stationConnected;
}

static bool is_sta_connected = false;

CHIP_ERROR ASRUtils::IsStationConnected(bool & connected)
{
    connected = is_sta_connected;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASRUtils::SetStationConnected(bool connected)
{
    is_sta_connected = connected;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASRUtils::EnableStationMode(void)
{
    int curmode = lega_wlan_get_wifi_mode();

    if (curmode == STA)
    {
        ChipLogError(DeviceLayer, "EnableStationMode, STA already");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Changing ASR WiFi mode to STA");
        lega_wlan_init_type_t conf;

        memset(&conf, 0, sizeof(lega_wlan_init_type_t));
        conf.wifi_mode = STA;
        conf.dhcp_mode = WLAN_DHCP_CLIENT;

        // before wlan open with sta mode, make sure the wlan is closed.
        lega_wlan_close();

        if (lega_wlan_open(&conf) != 0)
        {
            ChipLogError(DeviceLayer, "lega_wlan_open() failed");
            return CHIP_ERROR_INTERNAL;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ASRUtils::GetWiFiSSID(char * buf, size_t bufSize)
{
    size_t num = 0;
    return ASRConfig::ReadConfigValueStr(ASRConfig::kConfigKey_WiFiSSID, buf, bufSize, num);
}

CHIP_ERROR ASRUtils::StoreWiFiSSID(char * buf, size_t size)
{
    return ASRConfig::WriteConfigValueStr(ASRConfig::kConfigKey_WiFiSSID, buf, size);
}

CHIP_ERROR ASRUtils::GetWiFiPassword(char * buf, size_t bufSize)
{
    size_t num = 0;
    return ASRConfig::ReadConfigValueStr(ASRConfig::kConfigKey_WiFiPassword, buf, bufSize, num);
}

CHIP_ERROR ASRUtils::StoreWiFiPassword(char * buf, size_t size)
{
    return ASRConfig::WriteConfigValueStr(ASRConfig::kConfigKey_WiFiPassword, buf, size);
}

CHIP_ERROR ASRUtils::GetWiFiSecurityCode(uint32_t & security)
{
    return ASRConfig::ReadConfigValue(ASRConfig::kConfigKey_WiFiSecurity, security);
}

CHIP_ERROR ASRUtils::StoreWiFiSecurityCode(uint32_t security)
{
    return ASRConfig::WriteConfigValue(ASRConfig::kConfigKey_WiFiSecurity, security);
}

CHIP_ERROR ASRUtils::wifi_get_mode(uint32_t & mode)
{
    return ASRConfig::ReadConfigValue(ASRConfig::kConfigKey_WiFiMode, mode);
}

CHIP_ERROR ASRUtils::wifi_set_mode(uint32_t mode)
{
    return ASRConfig::WriteConfigValue(ASRConfig::kConfigKey_WiFiMode, mode);
}

CHIP_ERROR ASRUtils::asr_wifi_set_config(lega_wlan_wifi_conf * conf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Store Wi-Fi Configurations in Storage */
    err = StoreWiFiSSID((char *) conf->wifi_ssid, conf->ssid_len + 1);
    SuccessOrExit(err);

    err = StoreWiFiPassword((char *) conf->wifi_key, conf->key_len + 1);
    SuccessOrExit(err);

    err = StoreWiFiSecurityCode(conf->security);
    SuccessOrExit(err);

    memcpy(&wifi_conf, conf, sizeof(lega_wlan_wifi_conf));

exit:
    return err;
}

CHIP_ERROR ASRUtils::asr_wifi_get_config(lega_wlan_wifi_conf * conf)
{
    uint32_t code  = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (ASRConfig::ConfigValueExists(ASRConfig::kConfigKey_WiFiSSID) &&
        ASRConfig::ConfigValueExists(ASRConfig::kConfigKey_WiFiPassword) &&
        ASRConfig::ConfigValueExists(ASRConfig::kConfigKey_WiFiSecurity))
    {
        /* Retrieve Wi-Fi Configurations from Storage */
        err = GetWiFiSSID((char *) conf->wifi_ssid, sizeof(conf->wifi_ssid));
        SuccessOrExit(err);

        err = GetWiFiPassword((char *) conf->wifi_key, sizeof(conf->wifi_key));
        SuccessOrExit(err);

        err = GetWiFiSecurityCode(code);

        conf->ssid_len = strlen((char *) conf->wifi_ssid);

        conf->key_len = strlen((char *) conf->wifi_key);

        SuccessOrExit(err);
        conf->security = static_cast<lega_wlan_security_e>(code);
    }
    else
    {
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

exit:
    return err;
}

CHIP_ERROR ASRUtils::GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    lega_wlan_wifi_conf stationConfig;

    err = asr_wifi_get_config(&stationConfig);
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "GetWiFiStationProvision");
    VerifyOrExit(strlen((const char *) stationConfig.wifi_ssid) != 0, err = CHIP_ERROR_INCORRECT_STATE);

    netInfo.NetworkId              = kWiFiStationNetworkId;
    netInfo.FieldPresent.NetworkId = true;
    memcpy(netInfo.WiFiSSID, stationConfig.wifi_ssid,
           min(strlen(reinterpret_cast<char *>(stationConfig.wifi_ssid)) + 1, sizeof(netInfo.WiFiSSID)));

    // Enforce that netInfo wifiSSID is null terminated
    netInfo.WiFiSSID[kMaxWiFiSSIDLength] = '\0';

    if (includeCredentials)
    {
        static_assert(sizeof(netInfo.WiFiKey) < 255, "Our min might not fit in netInfo.WiFiKeyLen");
        netInfo.WiFiKeyLen = static_cast<uint8_t>(min(strlen((char *) stationConfig.wifi_key), sizeof(netInfo.WiFiKey)));
        memcpy(netInfo.WiFiKey, stationConfig.wifi_key, netInfo.WiFiKeyLen);
    }

exit:
    return err;
}

CHIP_ERROR ASRUtils::ClearWiFiStationProvision(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    lega_wlan_wifi_conf stationConfig;
    ChipLogProgress(DeviceLayer, "ClearWiFiStationProvision");
    // Clear the ASR WiFi station configuration.
    memset(&stationConfig, 0, sizeof(stationConfig));
    ReturnLogErrorOnFailure(asr_wifi_set_config(&stationConfig));
    return err;
}

CHIP_ERROR ASRUtils::asr_wifi_disconnect(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "asr_wifi_disconnect");

    if (lega_wlan_close() != 0)
    {
        ChipLogError(DeviceLayer, "asr_wifi_disconnect() failed");
        err = CHIP_ERROR_INTERNAL;
    }
    return err;
}

CHIP_ERROR ASRUtils::asr_wifi_connect(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    lega_wlan_wifi_conf stationConfig;
    lega_wlan_init_type_t conf;

    memset(&conf, 0, sizeof(lega_wlan_init_type_t));
    conf.wifi_mode = STA;
    conf.dhcp_mode = WLAN_DHCP_CLIENT;
    asr_wifi_get_config(&stationConfig);

    strncpy((char *) conf.wifi_ssid, (char *) stationConfig.wifi_ssid, stationConfig.ssid_len);

    strncpy((char *) conf.wifi_key, (char *) stationConfig.wifi_key, stationConfig.key_len);
    conf.security = stationConfig.security;

    ChipLogProgress(DeviceLayer, "Connecting to AP : [%s]\r\n", StringOrNullMarker(conf.wifi_ssid));

    lega_wlan_clear_pmk();

    if (lega_wlan_open(&conf) != 0)
    {
        ChipLogError(DeviceLayer, "asr_wifi_connect() failed");
        err = CHIP_ERROR_INTERNAL;
    }

    return err;
}

void ASRUtils::lega_wifi_scan_ind(lega_wlan_scan_result_t * result)
{
    NumAP = 0;
    for (int i = 0; i < result->ap_num; i++)
    {
        if (i < kWiFiMaxNetworks)
        {
            memcpy(&scan_result[i], &result->ap_list[i], sizeof(lega_wlan_scan_ap_record_t));
            NumAP++;
        }
    }
    NetworkCommissioning::ASRWiFiDriver::GetInstance().OnScanWiFiNetworkDone();
}

CHIP_ERROR ASRUtils::StartScanWiFiNetworks(ByteSpan ssid)
{
    if (Internal::ASRUtils::EnableStationMode() != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Start Scan WiFi Networks Failed");
        return CHIP_ERROR_INTERNAL;
    }

    NumAP = 0;
    memset(scan_result, 0, sizeof(scan_result));
    lega_wlan_scan_compeleted_cb_register(Internal::ASRUtils::lega_wifi_scan_ind);

    if (!ssid.empty()) // ssid is given, only scan this network
    {
        char cSsid[DeviceLayer::Internal::kMaxWiFiSSIDLength] = {};
        memcpy(cSsid, ssid.data(), ssid.size());
        lega_wlan_start_scan_active(cSsid, 0);
    }
    else // scan all networks
    {
        lega_wlan_start_scan();
    }
    return CHIP_NO_ERROR;
}

lega_wlan_scan_ap_record_t * ASRUtils::GetScanResults(void)
{
    return scan_result;
}

uint8_t ASRUtils::GetScanApNum(void)
{
    return NumAP;
}
