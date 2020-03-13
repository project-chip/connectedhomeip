/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an generic implementation of ConnectivityManager features
 *          for use on platforms that support BLE.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_IPP
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_IPP

#include <Weave/DeviceLayer/internal/GenericConnectivityManagerImpl_BLE.h>


namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the template class in whatever compilation unit includes this file.
template class GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>;

template<class ImplClass>
const char * GenericConnectivityManagerImpl_BLE<ImplClass>::_WoBLEServiceModeToStr(ConnectivityManager::WoBLEServiceMode mode)
{
    switch (mode)
    {
    case ConnectivityManager::kWoBLEServiceMode_NotSupported:
        return "NotSupported";
    case ConnectivityManager::kWoBLEServiceMode_Enabled:
        return "Disabled";
    case ConnectivityManager::kWoBLEServiceMode_Disabled:
        return "Enabled";
    default:
        return "(unknown)";
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl


#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_IPP
