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
 *          for use on platforms that support Thread.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_H
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_H

#include <platform/ThreadStackManager.h>

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

    void _Init(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    ConnectivityManager::ThreadMode _GetThreadMode(void);
    CHIP_ERROR _SetThreadMode(ConnectivityManager::ThreadMode val);
    bool _IsThreadEnabled(void);
    bool _IsThreadApplicationControlled(void);
    ConnectivityManager::ThreadDeviceType _GetThreadDeviceType(void);
    CHIP_ERROR _SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType);
    void _GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig);
    CHIP_ERROR _SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig);
    bool _IsThreadAttached(void);
    bool _IsThreadProvisioned(void);
    void _ErasePersistentInfo(void);
    bool _HaveServiceConnectivityViaThread(void);

    // ===== Members for use by the implementation subclass.

    void UpdateServiceConnectivity(void);

private:
    // ===== Private members reserved for use by this class only.

    enum Flags
    {
        kFlag_HaveServiceConnectivity = 0x01,
        kFlag_IsApplicationControlled = 0x02
    };

    uint8_t mFlags;

    ImplClass * Impl() { return static_cast<ImplClass *>(this); }
};

template <class ImplClass>
inline void GenericConnectivityManagerImpl_Thread<ImplClass>::_Init(void)
{
    mFlags = 0;
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadEnabled(void)
{
    return ThreadStackMgrImpl().IsThreadEnabled();
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadApplicationControlled(void)
{
    return GetFlag(mFlags, kFlag_IsApplicationControlled);
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadAttached(void)
{
    return ThreadStackMgrImpl().IsThreadAttached();
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_IsThreadProvisioned(void)
{
    return ThreadStackMgrImpl().IsThreadProvisioned();
}

template <class ImplClass>
inline void GenericConnectivityManagerImpl_Thread<ImplClass>::_ErasePersistentInfo(void)
{
    ThreadStackMgrImpl().ErasePersistentInfo();
}

template <class ImplClass>
inline ConnectivityManager::ThreadDeviceType GenericConnectivityManagerImpl_Thread<ImplClass>::_GetThreadDeviceType(void)
{
    return ThreadStackMgrImpl().GetThreadDeviceType();
}

template <class ImplClass>
inline CHIP_ERROR
GenericConnectivityManagerImpl_Thread<ImplClass>::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    return ThreadStackMgrImpl().SetThreadDeviceType(deviceType);
}

template <class ImplClass>
inline void
GenericConnectivityManagerImpl_Thread<ImplClass>::_GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    ThreadStackMgrImpl().GetThreadPollingConfig(pollingConfig);
}

template <class ImplClass>
inline CHIP_ERROR GenericConnectivityManagerImpl_Thread<ImplClass>::_SetThreadPollingConfig(
    const ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    return ThreadStackMgrImpl().SetThreadPollingConfig(pollingConfig);
}

template <class ImplClass>
inline bool GenericConnectivityManagerImpl_Thread<ImplClass>::_HaveServiceConnectivityViaThread(void)
{
    return GetFlag(mFlags, kFlag_HaveServiceConnectivity);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_H
