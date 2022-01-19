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

#include "wfx_host_events.h"
#include <platform/EFR32/EFR32Config.h>

#include "ServiceProvisioning.h"

using namespace ::chip::DeviceLayer;

CHIP_ERROR SetWiFiStationProvisioning(const char * ssid, const char * key)
{
    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);

    CHIP_ERROR err = CHIP_NO_ERROR;
    wfx_wifi_provision_t wifiConfig;

    // Set the wifi configuration
    memset(&wifiConfig, 0, sizeof(wifiConfig));
    memcpy(wifiConfig.ssid, ssid, std::min(strlen(ssid) + 1, sizeof(wifiConfig.ssid)));
    memcpy(wifiConfig.passkey, key, std::min(strlen(key) + 1, sizeof(wifiConfig.passkey)));
    wifiConfig.security = WFX_SEC_WPA2; /* Need something better ? Scan? */

    // Configure the WFX WiFi interface.
    wfx_set_wifi_provision(&wifiConfig);
    /* Save into internal Keys */
    (void) Internal::EFR32Config::WriteConfigValueStr(Internal::EFR32Config::kConfigKey_WiFiSSID, (char *) ssid);
    (void) Internal::EFR32Config::WriteConfigValueStr(Internal::EFR32Config::kConfigKey_WiFiPSK, key);
    (void) Internal::EFR32Config::WriteConfigValueBin(Internal::EFR32Config::kConfigKey_WiFiSEC, &wifiConfig.security,
                                                      sizeof(wifiConfig.security));

    ChipLogProgress(DeviceLayer, "SP WiFi STA provision set (SSID: %s)", wifiConfig.ssid);

    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Disabled);
    ConnectivityMgr().SetWiFiStationMode(ConnectivityManager::kWiFiStationMode_Enabled);

    return err;
}
