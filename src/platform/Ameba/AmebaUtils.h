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

#pragma once

#include "platform/internal/DeviceNetworkInfo.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class AmebaUtils
{
public:
    static CHIP_ERROR StartWiFi(void);
    static CHIP_ERROR IsStationEnabled(bool & staEnabled);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);
    static CHIP_ERROR EnableStationMode(void);
    static CHIP_ERROR SetWiFiConfig(rtw_wifi_config_t * config);
    static CHIP_ERROR GetWiFiConfig(rtw_wifi_config_t * config);
    static CHIP_ERROR ClearWiFiConfig(void);
    static CHIP_ERROR WiFiDisconnect(void);
    static CHIP_ERROR WiFiConnectProvisionedNetwork(void);
    static CHIP_ERROR WiFiConnect(const char * ssid, const char * password);
    static CHIP_ERROR SetCurrentProvisionedNetwork(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
