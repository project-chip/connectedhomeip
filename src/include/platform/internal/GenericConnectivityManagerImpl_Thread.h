/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    CHIP_ERROR _SetPollingInterval(System::Clock::Milliseconds32 pollingInterval);
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER */
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

#if CHIP_CONFIG_ENABLE_ICD_SERVER
template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_Thread<ImplClass>::_SetPollingInterval(System::Clock::Milliseconds32 pollingInterval)
{
    return ThreadStackMgrImpl().SetPollingInterval(pollingInterval);
}
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

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
