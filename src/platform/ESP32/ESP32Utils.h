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

#pragma once

#include "platform/internal/DeviceNetworkInfo.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class ESP32Utils
{
public:
    static CHIP_ERROR IsAPEnabled(bool & apEnabled);
    static CHIP_ERROR IsStationEnabled(bool & staEnabled);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);
    static CHIP_ERROR StartWiFiLayer(void);
    static CHIP_ERROR EnableStationMode(void);
    static CHIP_ERROR SetAPMode(bool enabled);
    static int OrderScanResultsByRSSI(const void * _res1, const void * _res2);
    static const char * WiFiModeToStr(wifi_mode_t wifiMode);
    static struct netif * GetNetif(const char * ifKey);
    static struct netif * GetStationNetif(void);
    static bool IsInterfaceUp(const char * ifKey);
    static bool HasIPv6LinkLocalAddress(const char * ifKey);

    static CHIP_ERROR GetWiFiStationProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials);
    static CHIP_ERROR SetWiFiStationProvision(const Internal::DeviceNetworkInfo & netInfo);
    static CHIP_ERROR ClearWiFiStationProvision(void);
    static CHIP_ERROR InitWiFiStack(void);

    static CHIP_ERROR MapError(esp_err_t error);
    static void RegisterESP32ErrorFormatter();
    static bool FormatError(char * buf, uint16_t bufSize, CHIP_ERROR err);
};

#define ReturnMappedErrorOnFailure(expr)                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        esp_err_t __err = (expr);                                                                                                  \
        if (__err != ESP_OK)                                                                                                       \
        {                                                                                                                          \
            return chip::DeviceLayer::Internal::ESP32Utils::MapError(__err);                                                       \
        }                                                                                                                          \
    } while (false)

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
