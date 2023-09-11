/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
