/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          for use on platforms that don't have WiFi.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_WIFI_H
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_WIFI_H

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
class GenericConnectivityManagerImpl_NoWiFi
{
public:
    // ===== Methods that implement the ConnectivityManager abstract interface.

    ConnectivityManager::WiFiStationMode _GetWiFiStationMode(void);
    CHIP_ERROR _SetWiFiStationMode(ConnectivityManager::WiFiStationMode val);
    bool _IsWiFiStationEnabled(void);
    bool _IsWiFiStationApplicationControlled(void);
    bool _IsWiFiStationConnected(void);
    bool _HaveIPv4InternetConnectivity(void);
    bool _HaveIPv6InternetConnectivity(void);
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

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline ConnectivityManager::WiFiStationMode GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_GetWiFiStationMode(void)
{
    return ConnectivityManager::kWiFiStationMode_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_IsWiFiStationEnabled(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_IsWiFiStationConnected(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_HaveIPv4InternetConnectivity(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_HaveIPv6InternetConnectivity(void)
{
    return false;
}

template <class ImplClass>
inline uint32_t GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_GetWiFiStationReconnectIntervalMS(void)
{
    return 0;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_SetWiFiStationReconnectIntervalMS(uint32_t val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_IsWiFiStationProvisioned(void)
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_ClearWiFiStationProvision(void)
{}

template <class ImplClass>
inline ConnectivityManager::WiFiAPMode GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_GetWiFiAPMode(void)
{
    return ConnectivityManager::kWiFiAPMode_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_SetWiFiAPMode(ConnectivityManager::WiFiAPMode val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_IsWiFiAPActive(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_IsWiFiAPApplicationControlled(void)
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_DemandStartWiFiAP(void)
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_StopOnDemandWiFiAP(void)
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_MaintainOnDemandWiFiAP(void)
{}

template <class ImplClass>
inline uint32_t GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_GetWiFiAPIdleTimeoutMS(void)
{
    return 0;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_SetWiFiAPIdleTimeoutMS(uint32_t val)
{}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_GetAndLogWifiStatsCounters(void)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_CanStartWiFiScan()
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_OnWiFiScanDone()
{}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_OnWiFiStationProvisionChange()
{}

template <class ImplClass>
inline const char *
GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_WiFiStationModeToStr(ConnectivityManager::WiFiStationMode mode)
{
    return NULL;
}

template <class ImplClass>
inline const char * GenericConnectivityManagerImpl_NoWiFi<ImplClass>::_WiFiAPModeToStr(ConnectivityManager::WiFiAPMode mode)
{
    return NULL;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_WIFI_H
