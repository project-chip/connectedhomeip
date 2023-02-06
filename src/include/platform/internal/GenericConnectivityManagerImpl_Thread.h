/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for use on platforms that support Thread.
 */

#pragma once

#include <app/AttributeAccessInterface.h>
#include <lib/support/BitFlags.h>
#include <platform/ThreadStackManager.h>

#include <cstdint>

namespace chip {
namespace DeviceLayer {

class ConnectivityManagerImpl;

namespace Internal {

/**
 * Provides a generic implementation of WiFi-specific ConnectivityManager features for
 * use on platforms that support Thread.
 *
 * This class is intended to be inherited (directly or indirectly) by the ConnectivityManagerImpl
 * class, which also appears as the template's ImplClass parameter.
 *
 * The GenericConnectivityManagerImpl_Thread<> class is designed to be independent of the particular
 * Thread stack in use, implying, for example, that the code does not make direct use of any OpenThread
 * APIs.  This is achieved by delegating all stack-specific operations to the ThreadStackManager class.
 *
 */
template <class ImplClass>
class GenericConnectivityManagerImpl_Thread
{
protected:
    // ===== Methods that implement the ConnectivityManager abstract interface.

    void _Init();
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    ConnectivityManager::ThreadMode _GetThreadMode();
    CHIP_ERROR _SetThreadMode(ConnectivityManager::ThreadMode val);
    bool _IsThreadEnabled();
    bool _IsThreadApplicationControlled();
    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType();
    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);
#if CHIP_DEVICE_CONFIG_ENABLE_SED
    CHIP_ERROR _GetSEDIntervalsConfig(ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _SetSEDIntervalsConfig(const ConnectivityManager::SEDIntervalsConfig & intervalsConfig);
    CHIP_ERROR _RequestSEDActiveMode(bool onOff, bool delayIdle = false);
#endif
    bool _IsThreadAttached();
    bool _IsThreadProvisioned();
    void _ErasePersistentInfo();
    void _ResetThreadNetworkDiagnosticsCounts();
    CHIP_ERROR _WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId, app::AttributeValueEncoder & encoder);

    // ===== Members for use by the implementation subclass.

    void UpdateServiceConnectivity();

private:
    // ===== Private members reserved for use by this class only.

    enum class Flags : uint8_t
    {
        kHaveServiceConnectivity = 0x01,
        kIsApplicationControlled = 0x02
    };

    BitFlags<Flags> mFlags;

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline void GenericConnectivityManagerImpl_Thread<ImplClass>::_Init()
{
    mFlags.ClearAll();
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadEnabled()
{
    return ThreadStackMgrImpl().IsThreadEnabled();
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadApplicationControlled()
{
    return mFlags.Has(Flags::kIsApplicationControlled);
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadAttached()
{
    return ThreadStackMgrImpl().IsThreadAttached();
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadProvisioned()
{
    return ThreadStackMgrImpl().IsThreadProvisioned();
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_Thread<ImplClass>::_ErasePersistentInfo()
{
    ThreadStackMgrImpl().ErasePersistentInfo();
}

template <class ImplClass>
inline ConnectivityManager::ThreadDeviceType GenericConnectivityManagerImpl_Thread<ImplClass>::_GetThreadDeviceType()
{
    return ThreadStackMgrImpl().GetThreadDeviceType();
}

template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_Thread<ImplClass>::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    return ThreadStackMgrImpl().SetThreadDeviceType(deviceType);
}

#if CHIP_DEVICE_CONFIG_ENABLE_SED
template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_Thread<ImplClass>::_GetSEDIntervalsConfig(ConnectivityManager::SEDIntervalsConfig & intervalsConfig)
{
    return ThreadStackMgrImpl().GetSEDIntervalsConfig(intervalsConfig);
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_Thread<ImplClass>::_SetSEDIntervalsConfig(
    const ConnectivityManager::SEDIntervalsConfig & intervalsConfig)
{
    return ThreadStackMgrImpl().SetSEDIntervalsConfig(intervalsConfig);
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_Thread<ImplClass>::_RequestSEDActiveMode(bool onOff, bool delayIdle)
{
    return ThreadStackMgrImpl().RequestSEDActiveMode(onOff, delayIdle);
}
#endif

template <class ImplClass>
inline void GenericConnectivityManagerImpl_Thread<ImplClass>::_ResetThreadNetworkDiagnosticsCounts()
{
    ThreadStackMgrImpl().ResetThreadNetworkDiagnosticsCounts();
}

template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_Thread<ImplClass>::_WriteThreadNetworkDiagnosticAttributeToTlv(AttributeId attributeId,
                                                                                              app::AttributeValueEncoder & encoder)
{
    return ThreadStackMgrImpl().WriteThreadNetworkDiagnosticAttributeToTlv(attributeId, encoder);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
