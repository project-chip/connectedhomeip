/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <platform/ConnectivityManager.h>

#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Inet {
class IPAddress;
} // namespace Inet
} // namespace chip

namespace chip {
namespace DeviceLayer {

class ConnectivityManagerImplWiFi
{
    friend class ConnectivityManager;

protected:
    CHIP_ERROR InitWiFi();

private:
    // Wi-Fi station
    ConnectivityManager::WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationApplicationControlled(void);
    bool _IsWiFiStationConnected(void);
    System::Clock::Timeout _GetWiFiStationReconnectInterval(void);
    CHIP_ERROR _SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    CHIP_ERROR _GetAndLogWiFiStatsCounters(void);
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();

    // Wi-Fi access point - not supported
    ConnectivityManager::WiFiAPMode _GetWiFiAPMode(void);
    CHIP_ERROR _SetWiFiAPMode(ConnectivityManager::WiFiAPMode val);
    bool _IsWiFiAPActive(void);
    bool _IsWiFiAPApplicationControlled(void);
    void _DemandStartWiFiAP(void);
    void _StopOnDemandWiFiAP(void);
    void _MaintainOnDemandWiFiAP(void);
    System::Clock::Timeout _GetWiFiAPIdleTimeout(void);
    void _SetWiFiAPIdleTimeout(System::Clock::Timeout val);

    ConnectivityManager::WiFiStationMode mStationMode{ ConnectivityManager::WiFiStationMode::kWiFiStationMode_Disabled };
    ConnectivityManager::WiFiStationState mStationState{ ConnectivityManager::WiFiStationState::kWiFiStationState_NotConnected };
    System::Clock::Timeout mWiFiStationReconnectInterval{};

    static const char * _WiFiStationModeToStr(ConnectivityManager::WiFiStationMode mode);
    static const char * _WiFiAPModeToStr(ConnectivityManager::WiFiAPMode mode);
    static const char * _WiFiStationStateToStr(ConnectivityManager::WiFiStationState state);
    static const char * _WiFiAPStateToStr(ConnectivityManager::WiFiAPState state);
};

} // namespace DeviceLayer
} // namespace chip
