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

/**
 *    @file
 *          Provides an generic implementation of ConnectivityManager features
 *          for use on platforms that support WiFi.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_H
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_H

#include <platform/ConnectivityManager.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <support/logging/CHIPLogging.h>

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
 * values.  This allows the compiler to optimize away dead code without the use of #ifdef's.
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

    ConnectivityManager::WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationApplicationControlled(void);
    uint32_t _GetWiFiStationReconnectIntervalMS(void);
    CHIP_ERROR _SetWiFiStationReconnectIntervalMS(uint32_t val);
    bool _IsWiFiStationProvisioned(void);
    void _ClearWiFiStationProvision(void);
    ConnectivityManager::WiFiAPMode _GetWiFiAPMode(void);
    CHIP_ERROR _SetWiFiAPMode(ConnectivityManager::WiFiAPMode val);
    bool _IsWiFiAPActive(void);
    bool _IsWiFiAPApplicationControlled(void);
    void _DemandStartWiFiAP(void);
    void _StopOnDemandWiFiAP(void);
    void _MaintainOnDemandWiFiAP(void);
    uint32_t _GetWiFiAPIdleTimeoutMS(void);
    void _SetWiFiAPIdleTimeoutMS(uint32_t val);
    CHIP_ERROR _GetAndLogWifiStatsCounters(void);
    bool _CanStartWiFiScan();
    void _OnWiFiScanDone();
    void _OnWiFiStationProvisionChange();
    static const char * _WiFiStationModeToStr(ConnectivityManager::WiFiStationMode mode);
    static const char * _WiFiAPModeToStr(ConnectivityManager::WiFiAPMode mode);

protected:
    enum Flags
    {
        kFlag_HaveIPv4InternetConnectivity = 0x0001,
        kFlag_HaveIPv6InternetConnectivity = 0x0002,
        kFlag_AwaitingConnectivity         = 0x0010,
    };

    DeviceNetworkInfo mWiFiNetworkInfo;
    ConnectivityManager::WiFiStationMode mWiFiStationMode;
    bool mWiFiProvisioned;
    bool mWiFiScanPending;

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline ConnectivityManager::WiFiStationMode GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetWiFiStationMode(void)
{
    return mWiFiStationMode;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiStationApplicationControlled(void)
{
    return mWiFiStationMode == ConnectivityManager::kWiFiStationMode_ApplicationControlled;
}

template <class ImplClass>
inline uint32_t GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetWiFiStationReconnectIntervalMS(void)
{
    return 0;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetWiFiStationReconnectIntervalMS(uint32_t val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiStationProvisioned(void)
{
    return mWiFiProvisioned;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_ClearWiFiStationProvision(void)
{
    mWiFiProvisioned = false;
}

template <class ImplClass>
inline ConnectivityManager::WiFiAPMode GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetWiFiAPMode(void)
{
    return ConnectivityManager::kWiFiAPMode_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetWiFiAPMode(ConnectivityManager::WiFiAPMode val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiAPActive(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiAPApplicationControlled(void)
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_DemandStartWiFiAP(void)
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_StopOnDemandWiFiAP(void)
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_MaintainOnDemandWiFiAP(void)
{}

template <class ImplClass>
inline uint32_t GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetWiFiAPIdleTimeoutMS(void)
{
    return 0;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetWiFiAPIdleTimeoutMS(uint32_t val)
{}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_GetAndLogWifiStatsCounters(void)
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
{
    mWiFiScanPending = false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_WiFi<ImplClass>::_OnWiFiStationProvisionChange()
{}

template <class ImplClass>
inline const char * GenericConnectivityManagerImpl_WiFi<ImplClass>::_WiFiAPModeToStr(ConnectivityManager::WiFiAPMode mode)
{
    return NULL;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_H
