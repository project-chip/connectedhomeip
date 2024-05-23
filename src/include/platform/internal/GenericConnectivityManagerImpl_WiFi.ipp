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

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_CPP
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_CPP

#include <lib/support/CodeUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
const char * GenericConnectivityManagerImpl_WiFi<ImplClass>::_WiFiStationModeToStr(ConnectivityManager::WiFiStationMode mode)
{
    switch (mode)
    {
    case ConnectivityManager::kWiFiStationMode_NotSupported:
        return "NotSupported";
    case ConnectivityManager::kWiFiStationMode_ApplicationControlled:
        return "AppControlled";
    case ConnectivityManager::kWiFiStationMode_Enabled:
        return "Enabled";
    case ConnectivityManager::kWiFiStationMode_Disabled:
        return "Disabled";
    default:
        return "(unknown)";
    }
}

template <class ImplClass>
const char * GenericConnectivityManagerImpl_WiFi<ImplClass>::_WiFiAPModeToStr(ConnectivityManager::WiFiAPMode mode)
{
    switch (mode)
    {
    case ConnectivityManager::kWiFiAPMode_NotSupported:
        return "NotSupported";
    case ConnectivityManager::kWiFiAPMode_ApplicationControlled:
        return "AppControlled";
    case ConnectivityManager::kWiFiAPMode_Disabled:
        return "Disabled";
    case ConnectivityManager::kWiFiAPMode_Enabled:
        return "Enabled";
    case ConnectivityManager::kWiFiAPMode_OnDemand:
        return "OnDemand";
    case ConnectivityManager::kWiFiAPMode_OnDemand_NoStationProvision:
        return "OnDemand_NoStationProvision";
    default:
        return "(unknown)";
    }
}

template <class ImplClass>
const char * GenericConnectivityManagerImpl_WiFi<ImplClass>::_WiFiStationStateToStr(ConnectivityManager::WiFiStationState state)
{
    switch (state)
    {
    case ConnectivityManager::kWiFiStationState_NotConnected:
        return "NotConnected";
    case ConnectivityManager::kWiFiStationState_Connecting:
        return "Connecting";
    case ConnectivityManager::kWiFiStationState_Connecting_Succeeded:
        return "Connecting_Succeeded";
    case ConnectivityManager::kWiFiStationState_Connecting_Failed:
        return "Connecting_Failed";
    case ConnectivityManager::kWiFiStationState_Connected:
        return "Connected";
    case ConnectivityManager::kWiFiStationState_Disconnecting:
        return "Disconnecting";
    default:
        return "(unknown)";
    }
}

template <class ImplClass>
const char * GenericConnectivityManagerImpl_WiFi<ImplClass>::_WiFiAPStateToStr(ConnectivityManager::WiFiAPState state)
{
    switch (state)
    {
    case ConnectivityManager::kWiFiAPState_NotActive:
        return "NotActive";
    case ConnectivityManager::kWiFiAPState_Activating:
        return "Activating";
    case ConnectivityManager::kWiFiAPState_Active:
        return "Active";
    case ConnectivityManager::kWiFiAPState_Deactivating:
        return "Deactivating";
    default:
        return "(unknown)";
    }
}

template <class ImplClass>
bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiStationEnabled()
{
    return Impl()->GetWiFiStationMode() == ConnectivityManager::kWiFiStationMode_Enabled;
}

template <class ImplClass>
bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiStationApplicationControlled()
{
    return Impl()->GetWiFiStationMode() == ConnectivityManager::kWiFiStationMode_ApplicationControlled;
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_CPP
