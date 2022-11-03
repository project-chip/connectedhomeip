/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
    static CHIP_ERROR WiFiConnect(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
