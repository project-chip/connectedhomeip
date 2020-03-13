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
 *          for use on platforms that don't have BLE.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_BLE_H
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_BLE_H

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of BLE-specific ConnectivityManager features for
 * platforms that don't support BLE.
 *
 * This class is intended to be inherited (directly or indirectly) by the ConnectivityManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 * The members of this class are all inlined methods that do nothing, and return static return
 * values.  This allows the compiler to optimize away dead code without the use of #ifdef's.
 * For example:
 *
 * ```
 * if (ConnectivityMgr().GetWoBLEServiceMode() != ConnectivityManager::kWoBLEServiceMode_NotSupported)
 * {
 *     // ... do something on devices that support WoBLE ...
 * }
 * ```
 */
template<class ImplClass>
class GenericConnectivityManagerImpl_NoBLE
{
public:

    // ===== Methods that implement the ConnectivityManager abstract interface.

    ConnectivityManager::WoBLEServiceMode _GetWoBLEServiceMode(void);
    WEAVE_ERROR _SetWoBLEServiceMode(ConnectivityManager::WoBLEServiceMode val);
    bool _IsBLEAdvertisingEnabled(void);
    WEAVE_ERROR _SetBLEAdvertisingEnabled(bool val);
    bool _IsBLEFastAdvertisingEnabled(void);
    WEAVE_ERROR _SetBLEFastAdvertisingEnabled(bool val);
    WEAVE_ERROR _GetBLEDeviceName(char * buf, size_t bufSize);
    WEAVE_ERROR _SetBLEDeviceName(const char * deviceName);
    uint16_t _NumBLEConnections(void);
    static const char * _WoBLEServiceModeToStr(ConnectivityManager::WoBLEServiceMode mode);

private:

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template<class ImplClass>
inline ConnectivityManager::WoBLEServiceMode GenericConnectivityManagerImpl_NoBLE<ImplClass>::_GetWoBLEServiceMode(void)
{
    return ConnectivityManager::kWoBLEServiceMode_NotSupported;
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_NoBLE<ImplClass>::_SetWoBLEServiceMode(ConnectivityManager::WoBLEServiceMode val)
{
    return WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE;
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_NoBLE<ImplClass>::_IsBLEAdvertisingEnabled(void)
{
    return false;
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_NoBLE<ImplClass>::_SetBLEAdvertisingEnabled(bool val)
{
    return WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE;
}

template<class ImplClass>
inline bool GenericConnectivityManagerImpl_NoBLE<ImplClass>::_IsBLEFastAdvertisingEnabled(void)
{
    return false;
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_NoBLE<ImplClass>::_SetBLEFastAdvertisingEnabled(bool val)
{
    return WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE;
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_NoBLE<ImplClass>::_GetBLEDeviceName(char * buf, size_t bufSize)
{
    return WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE;
}

template<class ImplClass>
inline WEAVE_ERROR GenericConnectivityManagerImpl_NoBLE<ImplClass>::_SetBLEDeviceName(const char * deviceName)
{
    return WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE;
}

template<class ImplClass>
inline uint16_t GenericConnectivityManagerImpl_NoBLE<ImplClass>::_NumBLEConnections(void)
{
    return false;
}

template<class ImplClass>
inline const char * GenericConnectivityManagerImpl_NoBLE<ImplClass>::_WoBLEServiceModeToStr(ConnectivityManager::WoBLEServiceMode mode)
{
    return NULL;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_NO_BLE_H
