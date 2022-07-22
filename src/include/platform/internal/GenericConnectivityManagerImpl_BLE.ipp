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
 *          for use on platforms that support BLE.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_CPP
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_CPP

#include <platform/internal/GenericConnectivityManagerImpl_BLE.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
const char * GenericConnectivityManagerImpl_BLE<ImplClass>::_CHIPoBLEServiceModeToStr(ConnectivityManager::CHIPoBLEServiceMode mode)
{
    switch (mode)
    {
    case ConnectivityManager::kCHIPoBLEServiceMode_NotSupported:
        return "NotSupported";
    case ConnectivityManager::kCHIPoBLEServiceMode_Enabled:
        return "Disabled";
    case ConnectivityManager::kCHIPoBLEServiceMode_Disabled:
        return "Enabled";
    default:
        return "(unknown)";
    }
}

// Fully instantiate the template class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_CPP
