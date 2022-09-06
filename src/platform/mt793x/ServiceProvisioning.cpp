/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

#include <platform/mt793x/MT793XConfig.h>

#include "ServiceProvisioning.h"

using namespace ::chip::DeviceLayer;

CHIP_ERROR SetWiFiStationProvisioning(const char * ssid, const char * key)
{
    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);

    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t security;

    security = WIFI_AUTH_MODE_WPA2_PSK;

    /* Save into internal Keys */
    (void) Internal::MT793XConfig::WriteConfigValueStr(Internal::MT793XConfig::kConfigKey_WiFiSSID, (char *) ssid);
    (void) Internal::MT793XConfig::WriteConfigValueStr(Internal::MT793XConfig::kConfigKey_WiFiPSK, key);
    (void) Internal::MT793XConfig::WriteConfigValueBin(Internal::MT793XConfig::kConfigKey_WiFiSEC, &security, sizeof(security));
    ChipLogProgress(DeviceLayer, "SP WiFi STA provision set (SSID: %s)", ssid);

    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);
    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);

    return err;
}
