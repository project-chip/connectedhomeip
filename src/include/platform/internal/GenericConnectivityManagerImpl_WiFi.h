/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          Provides an generic implementation of ConnectivityManager features
 *          for use on platforms that support WiFi.
 */

#pragma once

#include <app/icd/server/ICDServerConfig.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/DeviceNetworkInfo.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of WiFi-specific ConnectivityManager features for
 * platforms that don't support WiFi.
 *
 * This class is intended to be inherited (directly or indirectly) by the ConnectivityManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 * The members of this class are all inlined methods that do nothing, and return static return
 * values.  This allows the compiler to optimize away dead code without the use of \#ifdef's.
 * For example:
 *
 * ```
 * if (ConnectivityMgr().GetWiFiStationMode() != ConnectivityManager::kWiFiStationMode_NotSupported)
 * {
 *     // ... do something on devices that support WiFi ...
 * }
 * ```
 */
template <class ImplClass>
class GenericConnectivityManagerImpl_WiFi
{
public:
    // ===== Methods that implement the ConnectivityManager abstract interface.

    ConnectivityManager::WiFiStationMode _GetWiFiStationMode();
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    bool _IsWiFiStationEnabled();
    bool _IsWiFiStationApplicationControlled();
    System::Clock::Timeout _GetWiFiStationReconnectInterval();
    CHIP_ERROR _SetWiFiStationReconnectInterval(System::Clock::Timeout val);
    bool _IsWiFiStationProvisioned();
    void _ClearWiFiStationProvision();
    ConnectivityManager::WiFiAPMode _GetWiFiAPMode();
    CHIP_ERROR _SetWiFiAPMode(ConnectivityManager::WiFiAPMode val);
    bool _IsWiFiAPActive();
    bool _IsWiFiAPApplicationControlled();
    void _DemandStartWiFiAP();
    void _StopOnDemandWiFiAP();
    void _MaintainOnDemandWiFiAP();
    System::Clock::Timeout _GetWiFiAPIdleTimeout();
    void _SetWiFiAPIdleTimeout(System::Clock::Timeout val);
    CHIP_ERROR _GetAndLogWiFiStatsCounters();
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();
// TODO ICD rework: ambiguous declaration of _SetPollingInterval when thread and wifi are both build together
#if CHIP_CONFIG_ENABLE_ICD_SERVER && !CHIP_DEVICE_CONFIG_ENABLE_THREAD
    CHIP_ERROR _SetPollingInterval(System::Clock::Milliseconds32 pollingInterval);
#endif
    static const char * _WiFiStationModeToStr(ConnectivityManager::WiFiStationMode mode);
    static const char * _WiFiAPModeToStr(ConnectivityManager::WiFiAPMode mode);
    static const char * _WiFiStationStateToStr(ConnectivityManager::WiFiStationState state);
    static const char * _WiFiAPStateToStr(ConnectivityManager::WiFiAPState state);

protected:
    enum class ConnectivityFlags : uint16_t
    {
        kHaveIPv4InternetConnectivity = 0x0001,
        kHaveIPv6InternetConnectivity = 0x0002,
        kAwaitingConnectivity         = 0x0010,
    };

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline System::Clock::Timeout GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetWiFiStationReconnectInterval()
{
    return System::Clock::kZero;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetWiFiStationReconnectInterval(System::Clock::Timeout val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiStationProvisioned()
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_ClearWiFiStationProvision()
{}

template <class ImplClass>
inline ConnectivityManager::WiFiAPMode GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetWiFiAPMode()
{
    return ConnectivityManager::kWiFiAPMode_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetWiFiAPMode(ConnectivityManager::WiFiAPMode val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiAPActive()
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiAPApplicationControlled()
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_DemandStartWiFiAP()
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_StopOnDemandWiFiAP()
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_MaintainOnDemandWiFiAP()
{}

template <class ImplClass>
inline System::Clock::Timeout GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetWiFiAPIdleTimeout()
{
    return System::Clock::kZero;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetWiFiAPIdleTimeout(System::Clock::Timeout val)
{}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetAndLogWiFiStatsCounters()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_CanStartWiFiScan()
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_OnWiFiScanDone()
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_OnWiFiStationProvisionChange()
{}

#if CHIP_CONFIG_ENABLE_ICD_SERVER && !CHIP_DEVICE_CONFIG_ENABLE_THREAD
template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetPollingInterval(System::Clock::Milliseconds32 pollingInterval)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
