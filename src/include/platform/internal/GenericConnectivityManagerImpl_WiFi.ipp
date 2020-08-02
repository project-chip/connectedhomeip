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

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_IPP
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_IPP

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.h>
#include <support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericConnectivityManagerImpl_WiFi<ConnectivityManagerImpl>;

template <class ImplClass>
const char *
GenericConnectivityManagerImpl_WiFi<ImplClass>::_WiFiStationModeToStr(ConnectivityManager::WiFiStationMode mode)
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
bool GenericConnectivityManagerImpl_WiFi<ImplClass>::_IsWiFiStationEnabled(void)
{
    return Impl()->GetWiFiStationMode() == ConnectivityManager::kWiFiStationMode_Enabled;
}

template <class ImplClass>
CHIP_ERROR GenericConnectivityManagerImpl_WiFi<ImplClass>::_SetWiFiStationMode(ConnectivityManager::WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiStationMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", Impl()->WiFiStationModeToStr(mWiFiStationMode),
                        Impl()->WiFiStationModeToStr(val));
    }

    mWiFiStationMode = val;
exit:
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_WIFI_IPP
