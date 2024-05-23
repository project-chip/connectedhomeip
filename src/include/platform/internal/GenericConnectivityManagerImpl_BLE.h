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

#pragma once

#include <platform/internal/BLEManager.h>

namespace chip {
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
template <class ImplClass>
class GenericConnectivityManagerImpl_BLE
{
public:
    // ===== Methods that implement the ConnectivityManager abstract interface.

    Ble::BleLayer * _GetBleLayer();
    bool _IsBLEAdvertisingEnabled();
    CHIP_ERROR _SetBLEAdvertisingEnabled(bool val);
    bool _IsBLEAdvertising();
    CHIP_ERROR _SetBLEAdvertisingMode(ConnectivityManager::BLEAdvertisingMode mode);
    CHIP_ERROR _GetBLEDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetBLEDeviceName(const char * deviceName);
    uint16_t _NumBLEConnections();
    static const char * _CHIPoBLEServiceModeToStr(ConnectivityManager::CHIPoBLEServiceMode mode);

private:
    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

// Instruct the compiler to instantiate the template only when explicitly told to do so.
extern template class GenericConnectivityManagerImpl_BLE<ConnectivityManagerImpl>;

template <class ImplClass>
inline Ble::BleLayer * GenericConnectivityManagerImpl_BLE<ImplClass>::_GetBleLayer()
{
    return BLEMgr().GetBleLayer();
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_BLE<ImplClass>::_IsBLEAdvertisingEnabled()
{
    return BLEMgr().IsAdvertisingEnabled();
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_SetBLEAdvertisingEnabled(bool val)
{
    return BLEMgr().SetAdvertisingEnabled(val);
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_BLE<ImplClass>::_IsBLEAdvertising()
{
    return BLEMgr().IsAdvertising();
}

template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_BLE<ImplClass>::_SetBLEAdvertisingMode(ConnectivityManager::BLEAdvertisingMode mode)
{
    return BLEMgr().SetAdvertisingMode(mode);
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_GetBLEDeviceName(char * buf, size_t bufSize)
{
    return BLEMgr().GetDeviceName(buf, bufSize);
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_BLE<ImplClass>::_SetBLEDeviceName(const char * deviceName)
{
    return BLEMgr().SetDeviceName(deviceName);
}

template <class ImplClass>
inline uint16_t GenericConnectivityManagerImpl_BLE<ImplClass>::_NumBLEConnections()
{
    return BLEMgr().NumConnections();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
