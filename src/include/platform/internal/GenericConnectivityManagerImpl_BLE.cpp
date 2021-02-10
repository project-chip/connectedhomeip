/* See Project CHIP LICENSE file for licensing information. */

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

// Fully instantiate the template class in whatever compilation unit includes this file.
template class GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>;

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

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_CPP
