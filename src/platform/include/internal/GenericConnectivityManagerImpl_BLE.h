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

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_H
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_H

#include <Weave/DeviceLayer/internal/BLEManager.h>

namespace nl {
namespace Weave {
namespace DeviceLayer {

class ConnectivityManagerImpl;

namespace Internal {


/**
 * Provides a generic implementation of BLE-specific ConnectivityManager features for
 * platforms where BLE functionality is implemented by the BLEManager class.
 *
 * This class is intended to be inherited (directly or indirectly) by the ConnectivityManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 * The majority of methods on this class simply forward calls to similarly-named methods on
 * the BLEManager class.  This arrangement, where the ConnectivityManager implementation delegates
 * BLE support to the BLEManager class, is standard on platforms that support BLE, and helps to
 * limit the complexity of the ConnectivityManagerImpl class.
 */
template<class ImplClass>
class GenericConnectivityManagerImpl_BLE
{
public:

    // ===== Methods that implement the ConnectivityManager abstract interface.

    ConnectivityManager::WoBLEServiceMode _GetWoBLEServiceMode(void);
    WEAVE_ERROR _SetWoBLEServiceMode(ConnectivityManager::WoBLEServiceMode val);
    bool _IsBLEAdvertisingEnabled(void);
    WEAVE_ERROR _SetBLEAdvertisingEnabled(bool val);
    bool _IsBLEFastAdvertisingEnabled(void);
    WEAVE_ERROR _SetBLEFastAdvertisingEnabled(bool val);
    bool _IsBLEAdvertising(void);
    WEAVE_ERROR _GetBLEDeviceName(char * buf, size_t bufSize);
    WEAVE_ERROR _SetBLEDeviceName(const char * deviceName);
    uint16_t _NumBLEConnections(void);
    static const char * _WoBLEServiceModeToStr(ConnectivityManager::WoBLEServiceMode mode);

private:

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>;

template<class ImplClass>
inline ConnectivityManager::WoBLEServiceMode GenericConnectivityManagerImpl_BLE<ImplClass>::_GetWoBLEServiceMode(void)
{
    return BLEMgr().GetWoBLEServiceMode();
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_SetWoBLEServiceMode(ConnectivityManager::WoBLEServiceMode val)
{
    return BLEMgr().SetWoBLEServiceMode(val);
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_BLE<ImplClass>::_IsBLEAdvertisingEnabled(void)
{
    return BLEMgr().IsAdvertisingEnabled();
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_SetBLEAdvertisingEnabled(bool val)
{
    return BLEMgr().SetAdvertisingEnabled(val);
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_BLE<ImplClass>::_IsBLEFastAdvertisingEnabled(void)
{
    return BLEMgr().IsFastAdvertisingEnabled();
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_SetBLEFastAdvertisingEnabled(bool val)
{
    return BLEMgr().SetFastAdvertisingEnabled(val);
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_BLE<ImplClass>::_IsBLEAdvertising(void)
{
    return BLEMgr().IsAdvertising();
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_GetBLEDeviceName(char * buf, size_t bufSize)
{
    return BLEMgr().GetDeviceName(buf, bufSize);
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_SetBLEDeviceName(const char * deviceName)
{
    return BLEMgr().SetDeviceName(deviceName);
}

template<class ImplClass>
inline uint16_t GenericConnectivityManagerImpl_BLE<ImplClass>::_NumBLEConnections(void)
{
    return BLEMgr().NumConnections();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_BLE_H
