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
 *          Provides a generic implementation of ConnectivityManager features
 *          for use on platforms that do NOT support Thread.
 */

#pragma once

#include <app/AttributeAccessInterface.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Provides a generic implementation of WiFi-specific ConnectivityManager features for
 * use on platforms that do NOT support Thread.
 *
 * This class is intended to be inherited (directly or indirectly) by the ConnectivityManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 */
template <class ImplClass>
class GenericConnectivityManagerImpl_NoThread
{
protected:
    // ===== Methods that implement the ConnectivityManager abstract interface.

    ConnectivityManager::ThreadMode _GetThreadMode(void);
    CHIP_ERROR _SetThreadMode(ConnectivityManager::ThreadMode val);
    bool _IsThreadEnabled(void);
    bool _IsThreadApplicationControlled(void);
    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType(void);
    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);
    bool _IsThreadAttached(void);
    bool _IsThreadProvisioned(void);
    void _ErasePersistentInfo(void);
    void _ResetThreadNetworkDiagnosticsCounts(void);
    CHIP_ERROR _WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder);

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline ConnectivityManager::ThreadMode GenericConnectivityManagerImpl_NoThread<ImplClass>::_GetThreadMode(void)
{
    return ConnectivityManager::kThreadMode_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoThread<ImplClass>::_SetThreadMode(ConnectivityManager::ThreadMode val)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadEnabled(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadApplicationControlled(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadAttached(void)
{
    return false;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_NoThread<ImplClass>::_IsThreadProvisioned(void)
{
    return false;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoThread<ImplClass>::_ErasePersistentInfo(void)
{}

template <class ImplClass>
inline ConnectivityManager::ThreadDeviceType GenericConnectivityManagerImpl_NoThread<ImplClass>::_GetThreadDeviceType(void)
{
    return ConnectivityManager::kThreadDeviceType_NotSupported;
}

template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_NoThread<ImplClass>::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_NoThread<ImplClass>::_ResetThreadNetworkDiagnosticsCounts()
{}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_NoThread<ImplClass>::_WriteThreadNetworkDiagnosticAttributeToTlv(
    AttributeId attributeId, app::AttributeValueEncoder & encoder)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
