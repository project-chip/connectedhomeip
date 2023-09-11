/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "platform/internal/DeviceNetworkInfo.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

enum class AmebaErrorType
{
    kDctError,
    kFlashError,
    kWiFiError,
};

class AmebaUtils
{
public:
    static CHIP_ERROR StartWiFi(void);
    static CHIP_ERROR IsStationEnabled(bool & staEnabled);
    static bool IsStationInterfaceUp(void);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);
    static CHIP_ERROR IsStationIPLinked(bool & linked);
    static bool IsStationOpenSecurity(void);
    static CHIP_ERROR EnableStationMode(void);
    static CHIP_ERROR SetWiFiConfig(rtw_wifi_config_t * config);
    static CHIP_ERROR GetWiFiConfig(rtw_wifi_config_t * config);
    static CHIP_ERROR ClearWiFiConfig(void);
    static CHIP_ERROR WiFiDisconnect(void);
    static CHIP_ERROR WiFiConnectProvisionedNetwork(void);
    static CHIP_ERROR WiFiConnect(const char * ssid, const char * password);
    static CHIP_ERROR SetCurrentProvisionedNetwork(void);
    static CHIP_ERROR WiFiConnect(void);

    static CHIP_ERROR MapError(int32_t error, AmebaErrorType type);

private:
    static CHIP_ERROR MapDctError(int32_t error);
    static CHIP_ERROR MapFlashError(int32_t error);
    static CHIP_ERROR MapWiFiError(int32_t error);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
