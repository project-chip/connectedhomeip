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

#if CHIP_DEVICE_CONFIG_ENABLE_SED
    CHIP_ERROR _GetSEDIntervalsConfig(ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _SetSEDIntervalsConfig(const ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _RequestSEDActiveMode(bool onOff, bool delayIdle = false);
#endif

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
